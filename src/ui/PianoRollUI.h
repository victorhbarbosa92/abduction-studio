#pragma once
#include "imgui.h"
#include "../core/TimelineManager.h"
#include "Commands.h"
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>

#include "../audio/SynthEngine.h"

extern KuroAudio::SynthEngine g_piano_synth;

#include "../core/ClipManager.h"

namespace KuroUI {
    extern CommandManager g_command_manager;

    enum class PianoRollTool { Draw, Paint, Erase, Mute, Slice, Select };

    inline void RenderPianoRoll(bool* open, KuroDSP::TimelineManager& timeline_mgr, int track_idx, float bpm, unsigned long long* current_sample_ptr, bool is_playing, ClipManager& clip_manager, std::vector<KuroDSP::MidiNote>* ghost_notes = nullptr) {
        if (!*open) return;
        static int current_snap_option = 0; // Default to 4/4 (1/4 note)
        static PianoRollTool current_tool = PianoRollTool::Draw;
        static int current_stamp_idx = 0;

        ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
        ImGuiWindowFlags pr_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoFocusOnAppearing;
        if (!ImGui::Begin("Piano Roll", open, pr_flags)) {
            ImGui::End();
            return;
        }

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Acoes")) {
                if (ImGui::MenuItem("Limpar Todas as Notas")) {
                    auto& current_pattern_notes = clip_manager.global_patterns[clip_manager.current_pattern_idx].notes;
                    current_pattern_notes.clear();
                }
                ImGui::EndMenu();
            }
            
            // Seletor de Instrumento MIDI direto na barra de menu
            ImGui::SameLine();
            ImGui::Text("  Instrumento:");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.1f, 0.4f, 1.0f));
            ImGui::SetNextItemWidth(180);
            int current_inst = (int)g_piano_synth.current_instrument;
            if (ImGui::BeginCombo("##instrument_select", KuroAudio::getMidiInstrumentName(g_piano_synth.current_instrument))) {
                for (int i = 0; i < (int)KuroAudio::MidiInstrument::COUNT; i++) {
                    bool is_selected = (current_inst == i);
                    if (ImGui::Selectable(KuroAudio::getMidiInstrumentName((KuroAudio::MidiInstrument)i), is_selected)) {
                        g_piano_synth.setInstrument((KuroAudio::MidiInstrument)i);
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::PopStyleColor(2);
            
            // Seletor de Snap Grid
            ImGui::SameLine();
            ImGui::Text("  Grade (Snap):");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.1f, 0.4f, 1.0f));
            ImGui::SetNextItemWidth(120);
            
            const char* snap_options[] = { "4/4 (1/4)", "8/8 (1/8)", "16/16 (1/16)", "32/32 (1/32)" };
            ImGui::Combo("##snap_select", &current_snap_option, snap_options, IM_ARRAYSIZE(snap_options));
            ImGui::PopStyleColor(2);
            
            ImGui::EndMenuBar();
        }

        // --- Barra de Ferramentas (Toolbar FL Studio) ---
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.5f, 1.0f));
        
        auto draw_tool_btn = [](const char* label, PianoRollTool tool, PianoRollTool& current) {
            bool selected = (current == tool);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.2f, 1.0f)); // Amarelo ativo
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            }
            if (ImGui::Button(label)) current = tool;
            if (selected) ImGui::PopStyleColor(2);
        };

        draw_tool_btn("[ Pencil ]", PianoRollTool::Draw, current_tool); ImGui::SameLine();
        draw_tool_btn("[ Brush ]", PianoRollTool::Paint, current_tool); ImGui::SameLine();
        draw_tool_btn("[ Erase ]", PianoRollTool::Erase, current_tool); ImGui::SameLine();
        draw_tool_btn("[ Mute ]", PianoRollTool::Mute, current_tool); ImGui::SameLine();
        draw_tool_btn("[ Cut ]", PianoRollTool::Slice, current_tool); ImGui::SameLine();
        draw_tool_btn("[ Select ]", PianoRollTool::Select, current_tool); ImGui::SameLine();
        
        ImGui::Text(" | Stamp:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        const char* stamp_options[] = { "None", "Major", "Minor", "7th", "Maj7", "Min7" };
        ImGui::Combo("##stamp_select", &current_stamp_idx, stamp_options, IM_ARRAYSIZE(stamp_options));
        ImGui::PopStyleColor(3);
        
        ImGui::Separator();

        // --- Estado do Piano Roll ---
        static float pan_x = 0.0f;
        static float pan_y = 1765.0f; // Centrado na Oitava 4 (Middle C)
        static float zoom_x = 100.0f; // 100 pixels por segundo (zoom musical ideal)
        static float zoom_y = 45.0f;  // pixels por tecla (aumentado para melhor legibilidade)
        
        static int interacting_note_idx = -1; // Índice da nota sendo modificada
        static int interaction_mode = 0;      // 0=nenhum, 1=movendo, 2=redimensionando, 3=agulha, 4=painting, 5=erasing, 6=slicing, 7=selecting
        static float last_painted_time = -1.0f;
        static float slice_start_x = 0.0f;
        static ImVec2 select_start_pos;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
        canvas_sz.y -= 25.0f; // Deixa espaco para o scrollbar horizontal
        canvas_sz.x -= 20.0f; // Deixa espaco para o scrollbar vertical
        if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
        if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
        
        // Fundo do Canvas
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(30, 30, 35, 255));
        
        // --- Controles de Navegação ---
        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsWindowHovered()) {
            // Zoom X
            if (io.KeyCtrl && io.MouseWheel != 0.0f) {
                zoom_x += io.MouseWheel * 10.0f;
                if (zoom_x < 10.0f) zoom_x = 10.0f;
            }
            // Zoom Y
            if (io.KeyAlt && io.MouseWheel != 0.0f) {
                zoom_y += io.MouseWheel * 2.0f;
                if (zoom_y < 5.0f) zoom_y = 5.0f;
            }
            // Pan (Middle Mouse ou Shift+Drag)
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                pan_x -= io.MouseDelta.x;
                pan_y -= io.MouseDelta.y;
                if (pan_x < 0.0f) pan_x = 0.0f;
                if (pan_y < 0.0f) pan_y = 0.0f;
            }
        }

        float key_width = 90.0f;
        int num_keys = 120; // Expanded to 10 octaves
        int start_pitch = 0;
        
        float grid_start_x = canvas_p0.x + key_width;
        float beat_duration = 60.0f / bpm;
        float divisor = (current_snap_option == 0) ? 1.0f : (current_snap_option == 1) ? 2.0f : (current_snap_option == 2) ? 4.0f : 8.0f;
        float snap_step = beat_duration / divisor;

        // Prevenir pan_y além do número de chaves
        float total_height = num_keys * zoom_y;
        if (pan_y > total_height - canvas_sz.y) pan_y = total_height - canvas_sz.y;
        if (pan_y < 0.0f) pan_y = 0.0f;

        // --- Desenhar Grade de Fundo (Horizontal e Vertical) ---
        draw_list->PushClipRect(ImVec2(grid_start_x, canvas_p0.y), canvas_p1, true);
        
        // 1. Horizontal (Faixas das Teclas - Branco/Preto)
        for (int i = 0; i < num_keys; i++) {
            int pitch = start_pitch + (num_keys - 1 - i);
            float y = canvas_p0.y - pan_y + i * zoom_y;
            if (y + zoom_y < canvas_p0.y || y > canvas_p1.y) continue; // Culling
            
            int note_in_octave = pitch % 12;
            bool is_black = (note_in_octave == 1 || note_in_octave == 3 || note_in_octave == 6 || note_in_octave == 8 || note_in_octave == 10);
            
            ImU32 bg_col = is_black ? IM_COL32(22, 22, 26, 255) : IM_COL32(32, 32, 38, 255);
            draw_list->AddRectFilled(ImVec2(grid_start_x, y), ImVec2(canvas_p1.x, y + zoom_y), bg_col);
            draw_list->AddLine(ImVec2(grid_start_x, y + zoom_y), ImVec2(canvas_p1.x, y + zoom_y), IM_COL32(15, 15, 18, 255), 1.0f);
        }

        // 2. Vertical (Time Grid - Compassos e Batidas)
        float bar_duration = beat_duration * 4.0f;
        for (float t = 0.0f; (grid_start_x - pan_x) + t * zoom_x < canvas_p1.x + 1000.0f; t += snap_step) {
            float x = (grid_start_x - pan_x) + t * zoom_x;
            if (x < grid_start_x) continue;
            
            bool is_bar = std::fmod(t, bar_duration) < 0.01f;
            bool is_beat = std::fmod(t, beat_duration) < 0.01f;
            
            ImU32 col = is_bar ? IM_COL32(255,255,255,100) : (is_beat ? IM_COL32(255,255,255,50) : IM_COL32(255,255,255,15));
            float thickness = is_bar ? 2.0f : 1.0f;
            draw_list->AddLine(ImVec2(x, canvas_p0.y), ImVec2(x, canvas_p1.y), col, thickness);
        }
        draw_list->PopClipRect();

        // --- Área de Clique (Grid) ---
        ImGui::SetCursorScreenPos(ImVec2(grid_start_x, canvas_p0.y));
        ImGui::InvisibleButton("##pianoroll_grid", ImVec2(canvas_sz.x - key_width, canvas_sz.y));
        bool is_grid_hovered = ImGui::IsItemHovered();

        std::lock_guard<std::mutex> lock(timeline_mgr.timeline_mutex);
        auto& notes = clip_manager.global_patterns[clip_manager.current_pattern_idx].notes;

        // --- Interação do Mouse com Notas (Estilo FL Studio) ---
        if (is_grid_hovered) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            float time_sec = (mouse_pos.x - grid_start_x + pan_x) / zoom_x;
            float time_snapped = std::round(time_sec / snap_step) * snap_step;
            int row = (int)((mouse_pos.y - canvas_p0.y + pan_y) / zoom_y);
            int pitch = start_pitch + (num_keys - 1 - row);
            
            // Procurar se o mouse está sobre alguma nota
            int hovered_note_idx = -1;
            bool hovering_right_edge = false;
            
            for (int i = (int)notes.size() - 1; i >= 0; i--) {
                auto& n = notes[i];
                if (n.pitch == pitch && time_sec >= n.start_time && time_sec <= n.start_time + n.duration) {
                    hovered_note_idx = i;
                    // Verifica se está na borda direita (últimos 10 pixels ou 20% da nota)
                    float n_x1 = grid_start_x - pan_x + (n.start_time + n.duration) * zoom_x;
                    if (mouse_pos.x >= n_x1 - 8.0f && mouse_pos.x <= n_x1 + 4.0f) {
                        hovering_right_edge = true;
                    }
                    break;
                }
            }

            // Mouse Cursor Feedback
            if (hovering_right_edge) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

            // Left Click - Actions
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                float dy = ImGui::GetMousePos().y - canvas_p0.y;
                if (dy < 20.0f) { // Clicou na regua do tempo (topo)
                    float target_time = (mouse_pos.x - grid_start_x + pan_x) / zoom_x;
                    if (target_time < 0.0f) target_time = 0.0f;
                    *current_sample_ptr = (unsigned long long)(target_time * 44100.0f);
                    timeline_mgr.setMasterFrame(*current_sample_ptr);
                    interaction_mode = 3; // Movendo agulha
                } else {
                    if (current_tool == PianoRollTool::Draw || current_tool == PianoRollTool::Paint) {
                        if (hovered_note_idx != -1 && current_tool == PianoRollTool::Draw) {
                            interacting_note_idx = hovered_note_idx;
                            interaction_mode = hovering_right_edge ? 2 : 1; // 2=Resize, 1=Move
                            notes[hovered_note_idx].is_selected = true; // Select on click
                        } else {
                            // Adicionar nova nota (e acordes se selecionado)
                            KuroDSP::MidiNote note;
                            note.pitch = pitch;
                            note.start_time = time_snapped;
                            note.duration = snap_step;
                            note.velocity = 0.8f;
                            notes.push_back(note);
                            interacting_note_idx = (int)notes.size() - 1;
                            
                            // Adicionar notas extras para Acordes (Stamp)
                            if (current_stamp_idx == 1) { // Major (0, 4, 7)
                                notes.push_back(KuroDSP::MidiNote(pitch + 4, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                            } else if (current_stamp_idx == 2) { // Minor (0, 3, 7)
                                notes.push_back(KuroDSP::MidiNote(pitch + 3, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                            } else if (current_stamp_idx == 3) { // 7th (0, 4, 7, 10)
                                notes.push_back(KuroDSP::MidiNote(pitch + 4, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 10, time_snapped, snap_step));
                            } else if (current_stamp_idx == 4) { // Maj7 (0, 4, 7, 11)
                                notes.push_back(KuroDSP::MidiNote(pitch + 4, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 11, time_snapped, snap_step));
                            } else if (current_stamp_idx == 5) { // Min7 (0, 3, 7, 10)
                                notes.push_back(KuroDSP::MidiNote(pitch + 3, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 10, time_snapped, snap_step));
                            }
                            
                            interaction_mode = (current_tool == PianoRollTool::Paint) ? 4 : 2; 
                            last_painted_time = time_snapped;
                            g_piano_synth.triggerNote(pitch, 0.5f, 0.8f); // Preview
                        }
                    } else if (current_tool == PianoRollTool::Erase) {
                        if (hovered_note_idx != -1) {
                            notes.erase(notes.begin() + hovered_note_idx);
                        }
                        interaction_mode = 5;
                    } else if (current_tool == PianoRollTool::Mute) {
                        if (hovered_note_idx != -1) {
                            notes[hovered_note_idx].is_muted = !notes[hovered_note_idx].is_muted;
                        }
                    } else if (current_tool == PianoRollTool::Slice) {
                        interaction_mode = 6;
                        slice_start_x = mouse_pos.x;
                    } else if (current_tool == PianoRollTool::Select) {
                        interaction_mode = 7;
                        select_start_pos = mouse_pos;
                        // Deselect all se clicar fora
                        if (hovered_note_idx == -1) {
                            for (auto& n : notes) n.is_selected = false;
                        }
                    }
                }
            }
            
            // Right Click - Delete global
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                for (auto it = notes.begin(); it != notes.end(); ) {
                    if (it->pitch == pitch && time_sec >= it->start_time && time_sec <= it->start_time + it->duration) {
                        it = notes.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }

        // Dragging Logic
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            float time_sec = (mouse_pos.x - grid_start_x + pan_x) / zoom_x;
            float time_snapped = std::round(time_sec / snap_step) * snap_step;
            int row = (int)((mouse_pos.y - canvas_p0.y + pan_y) / zoom_y);
            int pitch = start_pitch + (num_keys - 1 - row);
            
            if (interaction_mode == 3) {
                float target_time = (mouse_pos.x - grid_start_x + pan_x) / zoom_x;
                if (target_time < 0.0f) target_time = 0.0f;
                *current_sample_ptr = (unsigned long long)(target_time * 44100.0f);
                timeline_mgr.setMasterFrame(*current_sample_ptr);
            } 
            else if (interaction_mode == 1 || interaction_mode == 2) {
                if (interacting_note_idx != -1 && interacting_note_idx < notes.size()) {
                    auto& n = notes[interacting_note_idx];
                    if (interaction_mode == 1) { // Move
                        n.pitch = pitch;
                        n.start_time = time_snapped;
                        if (n.start_time < 0.0f) n.start_time = 0.0f;
                    } else if (interaction_mode == 2) { // Resize
                        float new_duration = time_snapped - n.start_time;
                        if (new_duration < snap_step) new_duration = snap_step;
                        n.duration = new_duration;
                    }
                }
            } else if (interaction_mode == 4) { // Painting
                if (time_snapped != last_painted_time) {
                    KuroDSP::MidiNote note;
                    note.pitch = pitch;
                    note.start_time = time_snapped;
                    note.duration = snap_step;
                    note.velocity = 0.8f;
                    notes.push_back(note);
                    last_painted_time = time_snapped;
                    g_piano_synth.triggerNote(pitch, 0.5f, 0.8f);
                }
            } else if (interaction_mode == 5) { // Erasing
                for (auto it = notes.begin(); it != notes.end(); ) {
                    if (it->pitch == pitch && time_sec >= it->start_time && time_sec <= it->start_time + it->duration) {
                        it = notes.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
        
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (interaction_mode == 6) { // Finaliza o Slice
                float slice_time = (ImGui::GetMousePos().x - grid_start_x + pan_x) / zoom_x;
                float slice_snapped = std::round(slice_time / snap_step) * snap_step;
                
                std::vector<KuroDSP::MidiNote> new_notes;
                for (auto& n : notes) {
                    if (slice_snapped > n.start_time && slice_snapped < n.start_time + n.duration) {
                        float old_dur = n.duration;
                        n.duration = slice_snapped - n.start_time; // Corta a nota original
                        
                        KuroDSP::MidiNote split_note = n; // Cria a metade direita
                        split_note.start_time = slice_snapped;
                        split_note.duration = old_dur - n.duration;
                        new_notes.push_back(split_note);
                    }
                }
                for (const auto& nn : new_notes) notes.push_back(nn);
            } else if (interaction_mode == 7) { // Finaliza a Seleção
                ImVec2 p0 = ImVec2(std::min(select_start_pos.x, ImGui::GetMousePos().x), std::min(select_start_pos.y, ImGui::GetMousePos().y));
                ImVec2 p1 = ImVec2(std::max(select_start_pos.x, ImGui::GetMousePos().x), std::max(select_start_pos.y, ImGui::GetMousePos().y));
                
                for (auto& n : notes) {
                    float n_x = grid_start_x - pan_x + n.start_time * zoom_x;
                    float n_y = canvas_p0.y - pan_y + ((num_keys - 1) - (n.pitch - start_pitch)) * zoom_y;
                    
                    if (n_x >= p0.x && n_x <= p1.x && n_y >= p0.y && n_y <= p1.y) {
                        n.is_selected = true;
                    }
                }
            }
            interacting_note_idx = -1;
            interaction_mode = 0;
        }

        // --- Desenhar Notas Ativas ---
        draw_list->PushClipRect(ImVec2(grid_start_x, canvas_p0.y), canvas_p1, true);
        
        if (ghost_notes) {
            for (const auto& gnote : *ghost_notes) {
                int row = (num_keys - 1) - (gnote.pitch - start_pitch);
                float y0 = canvas_p0.y - pan_y + row * zoom_y;
                float x0 = grid_start_x - pan_x + gnote.start_time * zoom_x;
                float x1 = x0 + gnote.duration * zoom_x;
                
                if (y0 + zoom_y > canvas_p0.y && y0 < canvas_p1.y && x1 > grid_start_x && x0 < canvas_p1.x) {
                    draw_list->AddRectFilled(ImVec2(x0, y0 + 1), ImVec2(x1 - 1, y0 + zoom_y - 1), IM_COL32(100, 100, 100, 100), 2.0f);
                }
            }
        }

        for (auto& note : notes) {
            int row = (num_keys - 1) - (note.pitch - start_pitch);
            float y0 = canvas_p0.y - pan_y + row * zoom_y;
            float x0 = grid_start_x - pan_x + note.start_time * zoom_x;
            float x1 = x0 + note.duration * zoom_x;
            float y1 = y0 + zoom_y;
            
            if (y1 > canvas_p0.y && y0 < canvas_p1.y && x1 > grid_start_x && x0 < canvas_p1.x) {
                ImU32 color = note.is_playing ? IM_COL32(255, 255, 0, 255) : IM_COL32(0, 200, 255, 200);
                if (note.is_muted) color = IM_COL32(100, 100, 100, 150);
                ImU32 border_col = note.is_selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 100, 200, 255);
                float border_thickness = note.is_selected ? 3.0f : 2.0f;
                
                draw_list->AddRectFilled(ImVec2(x0, y0 + 1), ImVec2(x1 - 1, y1 - 1), color, 3.0f);
                draw_list->AddRect(ImVec2(x0, y0 + 1), ImVec2(x1 - 1, y1 - 1), border_col, 3.0f, 0, border_thickness);
            }
        }
        
        // Desenha ferramentas visuais ativas por cima das notas
        if (interaction_mode == 6) { // Slice Tool Line
            ImVec2 m_pos = ImGui::GetMousePos();
            draw_list->AddLine(ImVec2(m_pos.x, canvas_p0.y), ImVec2(m_pos.x, canvas_p1.y), IM_COL32(255, 50, 50, 200), 2.0f);
        } else if (interaction_mode == 7) { // Marquee Selection
            ImVec2 m_pos = ImGui::GetMousePos();
            ImVec2 p0 = ImVec2(std::min(select_start_pos.x, m_pos.x), std::min(select_start_pos.y, m_pos.y));
            ImVec2 p1 = ImVec2(std::max(select_start_pos.x, m_pos.x), std::max(select_start_pos.y, m_pos.y));
            draw_list->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, 30));
            draw_list->AddRect(p0, p1, IM_COL32(255, 255, 255, 150), 0.0f, 0, 1.0f);
        }

        // --- Régua do Tempo e Agulha (Playhead) ---
        draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p1.x, canvas_p0.y + 20.0f), IM_COL32(40, 40, 45, 255));
        draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + 20.0f), ImVec2(canvas_p1.x, canvas_p0.y + 20.0f), IM_COL32(60, 60, 70, 255), 1.0f);
        
        // Desenhar os ticks da régua do Piano Roll (Musical)
        draw_list->PushClipRect(ImVec2(grid_start_x, canvas_p0.y), ImVec2(canvas_p1.x, canvas_p0.y + 20.0f), true);
        
        float ruler_bar_duration = beat_duration * 4.0f;
        for (float t = 0.0f; (grid_start_x - pan_x) + t * zoom_x < canvas_p1.x + 1000.0f; t += beat_duration) {
            float tx = grid_start_x - pan_x + t * zoom_x;
            if (tx < grid_start_x) continue;
            
            bool is_bar = std::fmod(t, ruler_bar_duration) < 0.01f;
            if (is_bar) {
                // Major tick: Compasso (Bar)
                draw_list->AddLine(ImVec2(tx, canvas_p0.y + 4.0f), ImVec2(tx, canvas_p0.y + 20.0f), IM_COL32(200, 200, 200, 255), 1.5f);
                char label[16];
                int bar_num = (int)(std::round(t / ruler_bar_duration)) + 1;
                snprintf(label, sizeof(label), "%d", bar_num);
                draw_list->AddText(ImVec2(tx + 4, canvas_p0.y + 2), IM_COL32(200, 200, 200, 255), label);
            } else {
                // Minor tick: Batida (Beat)
                draw_list->AddLine(ImVec2(tx, canvas_p0.y + 12.0f), ImVec2(tx, canvas_p0.y + 20.0f), IM_COL32(120, 120, 120, 255), 1.0f);
            }
        }
        draw_list->PopClipRect();
        
        // Cabeçalho da Régua (canto superior esquerdo fixo acima do teclado)
        draw_list->AddRectFilled(canvas_p0, ImVec2(grid_start_x, canvas_p0.y + 20.0f), IM_COL32(25, 25, 30, 255));
        draw_list->AddLine(ImVec2(grid_start_x, canvas_p0.y), ImVec2(grid_start_x, canvas_p0.y + 20.0f), IM_COL32(60, 60, 70, 255), 2.0f);

        float playhead_time = (float)(*current_sample_ptr) / 44100.0f;
        float playhead_x = grid_start_x - pan_x + playhead_time * zoom_x;
        
        // Clip playhead inside the grid area
        draw_list->PushClipRect(ImVec2(grid_start_x, canvas_p0.y), canvas_p1, true);
        if (playhead_x >= grid_start_x && playhead_x < canvas_p1.x) {
            draw_list->AddLine(ImVec2(playhead_x, canvas_p0.y), ImVec2(playhead_x, canvas_p1.y), IM_COL32(255, 50, 50, 255), 2.0f);
            draw_list->AddTriangleFilled(ImVec2(playhead_x - 6, canvas_p0.y), ImVec2(playhead_x + 6, canvas_p0.y), ImVec2(playhead_x, canvas_p0.y + 10), IM_COL32(255, 50, 50, 255));
        }
        draw_list->PopClipRect();
        
        draw_list->PopClipRect(); // Pop of main canvas clipping

        // --- Desenhar o Teclado (Fica Fixo na Esquerda, mas scrolla Verticalmente) ---
        draw_list->PushClipRect(canvas_p0, ImVec2(canvas_p0.x + key_width, canvas_p1.y), true);
        for (int i = 0; i < num_keys; i++) {
            int pitch = start_pitch + (num_keys - 1 - i);
            float y = canvas_p0.y - pan_y + i * zoom_y;
            
            if (y + zoom_y < canvas_p0.y || y > canvas_p1.y) continue; // Culling
            
            int note_in_octave = pitch % 12;
            bool is_black = (note_in_octave == 1 || note_in_octave == 3 || note_in_octave == 6 || note_in_octave == 8 || note_in_octave == 10);
            
            // Desenho Realista de Teclado de Piano
            if (is_black) {
                // Teclas pretas são mais curtas, então desenhamos a extensão branca no fundo primeiro
                draw_list->AddRectFilled(
                    ImVec2(canvas_p0.x + key_width * 0.62f, y), 
                    ImVec2(canvas_p0.x + key_width, y + zoom_y), 
                    IM_COL32(245, 245, 240, 255)
                );
                // Linha de divisão para o fundo branco
                draw_list->AddLine(
                    ImVec2(canvas_p0.x + key_width * 0.62f, y + zoom_y), 
                    ImVec2(canvas_p0.x + key_width, y + zoom_y), 
                    IM_COL32(180, 180, 180, 255)
                );
                
                // Tecla preta em cima
                draw_list->AddRectFilled(
                    ImVec2(canvas_p0.x, y), 
                    ImVec2(canvas_p0.x + key_width * 0.62f, y + zoom_y - 1.0f), 
                    IM_COL32(25, 25, 27, 255),
                    2.0f, ImDrawFlags_RoundCornersRight
                );
                draw_list->AddRect(
                    ImVec2(canvas_p0.x, y), 
                    ImVec2(canvas_p0.x + key_width * 0.62f, y + zoom_y - 1.0f), 
                    IM_COL32(45, 45, 50, 255),
                    2.0f, ImDrawFlags_RoundCornersRight
                );
            } else {
                // Tecla branca completa (marfim)
                draw_list->AddRectFilled(
                    ImVec2(canvas_p0.x, y), 
                    ImVec2(canvas_p0.x + key_width, y + zoom_y), 
                    IM_COL32(253, 253, 250, 255)
                );
                // Sombra de relevo na tecla branca
                draw_list->AddRectFilled(
                    ImVec2(canvas_p0.x, y + zoom_y - 2.0f), 
                    ImVec2(canvas_p0.x + key_width, y + zoom_y), 
                    IM_COL32(215, 215, 210, 255)
                );
                // Linha fina de divisão cinza
                draw_list->AddLine(
                    ImVec2(canvas_p0.x, y + zoom_y), 
                    ImVec2(canvas_p0.x + key_width, y + zoom_y), 
                    IM_COL32(140, 140, 140, 255)
                );
            }
            
            // Nomes das notas e oitavas
            if (zoom_y > 12.0f) {
                const char* note_names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
                int octave = (pitch / 12) - 1;
                char buf[16];
                snprintf(buf, sizeof(buf), "%s%d", note_names[note_in_octave], octave);
                
                ImU32 text_color = is_black ? IM_COL32(180, 180, 180, 255) : IM_COL32(40, 40, 45, 255);
                
                if (is_black) {
                    draw_list->AddText(ImVec2(canvas_p0.x + 5, y + (zoom_y - 15.0f) * 0.5f), text_color, buf);
                } else {
                    if (note_in_octave == 0) {
                        draw_list->AddText(ImVec2(canvas_p0.x + key_width - 32, y + (zoom_y - 15.0f) * 0.5f), IM_COL32(230, 100, 20, 255), buf);
                    } else {
                        draw_list->AddText(ImVec2(canvas_p0.x + key_width - 25, y + (zoom_y - 15.0f) * 0.5f), text_color, buf);
                    }
                }
            }
            
            // Grade horizontal estendendo para a direita
            ImU32 line_col = is_black ? IM_COL32(255,255,255,10) : IM_COL32(255,255,255,20);
            if (note_in_octave == 0) line_col = IM_COL32(255,255,255,40); // Highlight C
            draw_list->AddLine(ImVec2(grid_start_x, y), ImVec2(canvas_p1.x, y), line_col);
            
            // Área clicável do teclado
            ImGui::SetCursorScreenPos(ImVec2(canvas_p0.x, y));
            ImGui::PushID(pitch);
            if (ImGui::InvisibleButton("##key", ImVec2(key_width, zoom_y))) {
                g_piano_synth.triggerNote(pitch, 0.5f, 0.8f);
            }
            ImGui::PopID();
        }
        draw_list->PopClipRect();

        // --- Scrollbar Horizontal ---
        ImGui::SetCursorScreenPos(ImVec2(canvas_p0.x + key_width, canvas_p1.y + 5.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        float max_pan_x = std::max(0.0f, 5000.0f - canvas_sz.x);
        ImGui::SliderFloat("##pan_x", &pan_x, 0.0f, max_pan_x, "");
        ImGui::PopStyleColor();

        // --- Scrollbar Vertical ---
        ImGui::SetCursorScreenPos(ImVec2(canvas_p1.x + 5.0f, canvas_p0.y));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        float max_pan_y = std::max(0.0f, total_height - canvas_sz.y);
        // Em ImGui::VSliderFloat, visualmente o v_min fica embaixo e v_max em cima.
        // Queremos que pan_y = 0 (topo das notas) fique no topo do slider.
        // E pan_y = max_pan_y fique embaixo.
        ImGui::VSliderFloat("##pan_y", ImVec2(15.0f, canvas_sz.y), &pan_y, max_pan_y, 0.0f, "");
        ImGui::PopStyleColor();

        ImGui::End();
    }
}
