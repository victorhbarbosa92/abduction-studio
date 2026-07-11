#pragma once
#include "imgui.h"
#include "../core/TimelineManager.h"
#include "Commands.h"
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>

namespace KuroUI {
    extern CommandManager g_command_manager;

    inline void RenderPianoRoll(bool* open, KuroDSP::TimelineManager& timeline_mgr, int track_idx, float bpm, unsigned long long current_sample, bool is_playing, std::vector<KuroDSP::MidiNote>* ghost_notes = nullptr) {
        if (!*open) return;

        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Piano Roll", open, ImGuiWindowFlags_MenuBar)) {
            ImGui::End();
            return;
        }

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Ações")) {
                if (ImGui::MenuItem("Limpar Todas as Notas")) timeline_mgr.clearNotes(track_idx);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // --- Estado do Piano Roll ---
        static float pan_x = 0.0f;
        static float pan_y = 0.0f;
        static float zoom_x = 100.0f; // pixels por segundo
        static float zoom_y = 20.0f;  // pixels por tecla
        
        static int interacting_note_idx = -1; // Índice da nota sendo modificada
        static int interaction_mode = 0;      // 0=nenhum, 1=movendo, 2=redimensionando

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
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

        float key_width = 60.0f;
        int num_keys = 120; // Expanded to 10 octaves
        int start_pitch = 0;
        
        float grid_start_x = canvas_p0.x + key_width;
        float beat_duration = 60.0f / bpm;
        float snap_step = beat_duration / 4.0f; // 1/16 note snap

        // Prevenir pan_y além do número de chaves
        float total_height = num_keys * zoom_y;
        if (pan_y > total_height - canvas_sz.y) pan_y = total_height - canvas_sz.y;
        if (pan_y < 0.0f) pan_y = 0.0f;

        // --- Desenhar Grade Vertical (Tempo) ---
        draw_list->PushClipRect(ImVec2(grid_start_x, canvas_p0.y), canvas_p1, true);
        for (float t = 0.0f; (grid_start_x - pan_x) + t * zoom_x < canvas_p1.x + 1000.0f; t += snap_step) {
            float x = (grid_start_x - pan_x) + t * zoom_x;
            if (x < grid_start_x) continue;
            
            // Desenha linha mais forte no beat inteiro
            bool is_beat = std::fmod(t, beat_duration) < 0.01f;
            ImU32 col = is_beat ? IM_COL32(255,255,255,60) : IM_COL32(255,255,255,20);
            draw_list->AddLine(ImVec2(x, canvas_p0.y), ImVec2(x, canvas_p1.y), col);
        }
        draw_list->PopClipRect();

        // --- Área de Clique (Grid) ---
        ImGui::SetCursorScreenPos(ImVec2(grid_start_x, canvas_p0.y));
        ImGui::InvisibleButton("##pianoroll_grid", ImVec2(canvas_sz.x - key_width, canvas_sz.y));
        bool is_grid_hovered = ImGui::IsItemHovered();

        std::lock_guard<std::mutex> lock(timeline_mgr.timeline_mutex);
        auto& notes = timeline_mgr.track_notes[track_idx];

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
                if (hovered_note_idx != -1) {
                    interacting_note_idx = hovered_note_idx;
                    interaction_mode = hovering_right_edge ? 2 : 1; // 2=Resize, 1=Move
                } else {
                    // Adicionar nova nota
                    KuroDSP::MidiNote note;
                    note.pitch = pitch;
                    note.start_time = time_snapped;
                    note.duration = snap_step * 2.0f; // Default duration
                    note.velocity = 0.8f;
                    notes.push_back(note);
                    
                    interacting_note_idx = (int)notes.size() - 1;
                    interaction_mode = 2; // Ao criar, entra em modo resize automático se arrastar
                }
            }
            
            // Right Click - Delete (Brush)
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
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && interacting_note_idx != -1 && interacting_note_idx < notes.size()) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            float time_sec = (mouse_pos.x - grid_start_x + pan_x) / zoom_x;
            float time_snapped = std::round(time_sec / snap_step) * snap_step;
            int row = (int)((mouse_pos.y - canvas_p0.y + pan_y) / zoom_y);
            int pitch = start_pitch + (num_keys - 1 - row);
            
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
        
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
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
                draw_list->AddRectFilled(ImVec2(x0, y0 + 1), ImVec2(x1 - 1, y1 - 1), color, 3.0f);
                draw_list->AddRect(ImVec2(x0, y0 + 1), ImVec2(x1 - 1, y1 - 1), IM_COL32(0, 100, 200, 255), 3.0f);
            }
        }

        // --- Agulha do tempo (Playhead) ---
        float playhead_time = (float)current_sample / 44100.0f;
        float playhead_x = grid_start_x - pan_x + playhead_time * zoom_x;
        if (playhead_x > grid_start_x && playhead_x < canvas_p1.x) {
            draw_list->AddLine(ImVec2(playhead_x, canvas_p0.y), ImVec2(playhead_x, canvas_p1.y), IM_COL32(255, 50, 50, 255), 2.0f);
            // Draw small triangle on top
            draw_list->AddTriangleFilled(ImVec2(playhead_x - 5, canvas_p0.y), ImVec2(playhead_x + 5, canvas_p0.y), ImVec2(playhead_x, canvas_p0.y + 8), IM_COL32(255, 50, 50, 255));
        }

        draw_list->PopClipRect();

        // --- Desenhar o Teclado (Fica Fixo na Esquerda, mas scrolla Verticalmente) ---
        draw_list->PushClipRect(canvas_p0, ImVec2(canvas_p0.x + key_width, canvas_p1.y), true);
        for (int i = 0; i < num_keys; i++) {
            int pitch = start_pitch + (num_keys - 1 - i);
            float y = canvas_p0.y - pan_y + i * zoom_y;
            
            if (y + zoom_y < canvas_p0.y || y > canvas_p1.y) continue; // Culling
            
            int note_in_octave = pitch % 12;
            bool is_black = (note_in_octave == 1 || note_in_octave == 3 || note_in_octave == 6 || note_in_octave == 8 || note_in_octave == 10);
            
            ImU32 key_color = is_black ? IM_COL32(20, 20, 20, 255) : IM_COL32(200, 200, 200, 255);
            ImU32 text_color = is_black ? IM_COL32(200, 200, 200, 255) : IM_COL32(20, 20, 20, 255);
            
            draw_list->AddRectFilled(ImVec2(canvas_p0.x, y), ImVec2(canvas_p0.x + key_width, y + zoom_y), key_color);
            draw_list->AddRect(ImVec2(canvas_p0.x, y), ImVec2(canvas_p0.x + key_width, y + zoom_y), IM_COL32(50, 50, 50, 255));
            
            if (note_in_octave == 0 && zoom_y > 10.0f) { // Desenha o "C" da oitava
                int octave = (pitch / 12) - 1;
                char buf[16];
                snprintf(buf, sizeof(buf), "C%d", octave);
                draw_list->AddText(ImVec2(canvas_p0.x + 5, y + 2), text_color, buf);
            }
            
            // Desenha a linha da grade horizontal estendendo para a direita
            ImU32 line_col = is_black ? IM_COL32(255,255,255,10) : IM_COL32(255,255,255,20);
            if (note_in_octave == 0) line_col = IM_COL32(255,255,255,40); // Highlight C
            draw_list->AddLine(ImVec2(grid_start_x, y), ImVec2(canvas_p1.x, y), line_col);
        }
        draw_list->PopClipRect();

        ImGui::End();
    }
}
