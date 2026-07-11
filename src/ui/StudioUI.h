#pragma once
#include "imgui.h"
#include <string>
#include "../ai/StemSeparationEngine.h"
#include "../core/ClipManager.h"
#include "../audio/SynthEngine.h"
#include "../audio/RecordManager.h"
#include "../audio/OfflineRenderer.h"
#include "PianoRollUI.h"
#include "StepSequencerUI.h"
#include "ModulationPanelUI.h"
#include "KuroDSPUI.h"
#include "KuroWaveUI.h"
#include "KuroSamplerUI.h"

extern void setupAbductionTheme(); // FASE 23

extern bool is_playing;
extern unsigned long long global_sample_count;
extern ClipManager g_clip_manager;
extern KuroAudio::KuroWave g_kurowave;
extern KuroAudio::RecordManager g_record_manager;
extern KuroDSP::AudioGraph master_graph;
extern std::string track_names[MAX_TRACKS];
extern KuroDSP::TimelineManager timeline;
extern std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;

#include "../core/DJEngine.h"
#include "../core/StemExtractorEngine.h"
extern std::unique_ptr<KuroAudio::DJEngine> g_dj_engine;
extern std::unique_ptr<KuroAudio::StemExtractorEngine> g_stem_engine;

namespace ProjectManagerBridge {
    void Save(const std::string& path);
    void Load(const std::string& path);
}

namespace KuroUI {
    extern CommandManager g_command_manager;

    enum class AppMode {
        STUDIO_MODE,
        DJ_MODE,
        STEM_MODE
    };
    static AppMode current_app_mode = AppMode::STUDIO_MODE;

    static int selected_track_idx = 0;
    static bool set_mixer_focus = false;
    static bool show_piano_roll = false;
    static bool show_step_sequencer = false;
    static bool show_modulation_panel = false;

    // FASE 28: Estado Global de Efeitos Flutuantes e Cadeias por Faixa
    struct FloatingPluginWindow {
        int track_idx;
        int plugin_id;
        std::string name;
        bool is_open = true;
        bool is_bypassed = false;
        ImVec2 window_pos = ImVec2(0,0);
    };
    static std::vector<FloatingPluginWindow> active_plugin_windows;
    
    // Cada faixa tem uma lista de plugins (IDs: 1=Dark Drive, 2=RingMod, 3=Tremolo, 4=Flanger, 5=AbyssPitch)
    static std::vector<int> track_fx_chain[MAX_TRACKS]; 
    static int dragging_fx_idx = -1;
    static int dragging_fx_source_track = -1;

    static void RenderStudioMode(StemSeparationEngine& ai_engine) {
        ImVec2 window_size = ImGui::GetContentRegionAvail();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // ==========================================
        // 1. TOP BAR (TRANSPORTE) - Fixo no topo
        // ==========================================
        ImGui::BeginChild("TransportBar", ImVec2(0, 50), true);
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.08f, 0.10f, 1.0f));
        
        // Controles de Playback
        if (is_playing) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.22f, 1.0f, 0.08f, 1.0f)); // Neon Green
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        }
        if (ImGui::Button(is_playing ? "|| PAUSE" : "> PLAY", ImVec2(80, 30))) {
            is_playing = !is_playing;
            timeline.setPlaying(is_playing);
        }
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        if (ImGui::Button("[] STOP", ImVec2(80, 30))) {
            is_playing = false;
            global_sample_count = 0;
            timeline.setPlaying(false);
        }
        
        ImGui::SameLine();
        
        // Record Controls
        if (g_record_manager.isRecording()) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            if (ImGui::Button("STOP GRAVAR", ImVec2(100, 30))) {
                g_record_manager.setRecording(false);
            }
            ImGui::PopStyleColor();
        } else {
            if (ImGui::Button("O GRAVAR MIC", ImVec2(100, 30))) {
                g_record_manager.setRecording(true, false);
            }
            ImGui::SameLine();
            if (ImGui::Button("O BOUNCE", ImVec2(80, 30))) {
                g_record_manager.setRecording(true, true);
            }
        }
        
        if (!g_record_manager.isRecording() && g_record_manager.getRecordedFrames() > 0) {
            ImGui::SameLine();
            if (ImGui::Button("💾 SALVAR .WAV", ImVec2(100, 30))) {
                g_record_manager.saveToWav("bounce.wav");
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("🚀 EXPORTAR", ImVec2(100, 30))) {
            KuroAudio::OfflineRenderer::RenderStems(master_graph, 10.0f, ".");
        }
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
        if (ImGui::Button("🎹 PIANO ROLL", ImVec2(120, 30))) {
            show_piano_roll = true;
        }
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.6f, 0.3f, 1.0f));
        if (ImGui::Button("🥁 STEP SEQ", ImVec2(100, 30))) {
            show_step_sequencer = true;
        }
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.7f, 1.0f));
        if (ImGui::Button("🌀 LFO MATRIX", ImVec2(120, 30))) {
            show_modulation_panel = true;
        }
        ImGui::PopStyleColor();
        
        ImGui::PopStyleColor();

        // Display de Tempo (Alien HUD Style)
        size_t sample_rate = ai_engine.getSampleRate();
        if (sample_rate == 0) sample_rate = 44100;
        size_t total_sec = ai_engine.getTotalFrames() > 0 ? (ai_engine.getTotalFrames() / sample_rate) : 0;
        size_t curr_sec = global_sample_count / sample_rate;
        size_t ms = ((global_sample_count % sample_rate) * 10 / sample_rate); 
        
        char time_str[64];
        snprintf(time_str, sizeof(time_str), "%02d:%02d.%1d / %02d:%02d", 
                 (int)curr_sec / 60, (int)curr_sec % 60, (int)ms, 
                 (int)total_sec / 60, (int)total_sec % 60);

        ImGui::SameLine(0.0f, 40.0f);
        
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.02f, 0.02f, 0.03f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 1.0f, 0.08f, 0.5f));
        ImGui::BeginChild("TimeDisplay", ImVec2(300, 30), true);
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), " TIME: %s ", time_str);
        ImGui::SameLine(180);
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "BPM: %.1f", ai_engine.getBPM() > 0 ? ai_engine.getBPM() : 140.0f);
        ImGui::EndChild();
        ImGui::PopStyleColor(2);

        // Botão Limpar
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        if (ImGui::Button("Limpar Tudo", ImVec2(100, 30))) {
            ai_engine.reset();
            g_clip_manager.reset();
            global_sample_count = 0;
            is_playing = false;
            timeline.setPlaying(false);
        }
        
        ImGui::EndChild();

        // ==========================================
        // 2. PAINEL CENTRAL (BROWSER + TIMELINE)
        // ==========================================
        float middle_height = window_size.y - 50 - 280; // Deixa 280px para o Rack inferior
        if (middle_height < 100) middle_height = 100;

        ImGui::BeginChild("CentralAreaGroup", ImVec2(0, middle_height), false);
        
        // --- BROWSER PANEL (Left) ---
        ImGui::BeginChild("BrowserPanel", ImVec2(220, middle_height), true);
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "🛸 BROWSER");
        ImGui::Separator();
        if (ImGui::TreeNodeEx("Samples & Loops", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BulletText("Kick_Alien_01.wav");
            ImGui::BulletText("Bass_FM_C.wav");
            ImGui::BulletText("Vocal_Abduction.wav");
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Plugins (VST3/CLAP)", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BulletText("Serum (VST3)");
            ImGui::BulletText("Vital (CLAP)");
            ImGui::BulletText("Kuro Synth (Native)");
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Projetos")) {
            ImGui::BulletText("Area51_Jam.abduct");
            ImGui::TreePop();
        }
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // --- TIMELINE PANEL (Right) ---
        ImGui::BeginChild("CentralArea", ImVec2(0, middle_height), true);

        // Cores das faixas (Tons cibernéticos e alienígenas)
        ImVec4 track_colors[MAX_TRACKS] = {
            ImVec4(0.8f, 0.2f, 0.2f, 1.0f), // KICK/BASS
            ImVec4(0.2f, 0.8f, 0.2f, 1.0f), // LEADS
            ImVec4(0.2f, 0.2f, 0.8f, 1.0f), // VOX
            ImVec4(0.8f, 0.8f, 0.2f, 1.0f), // FX
            ImVec4(0.2f, 0.8f, 0.8f, 1.0f), // DRUMS
            ImVec4(0.8f, 0.2f, 0.8f, 1.0f), // SYNTH
            ImVec4(0.5f, 0.5f, 0.5f, 1.0f), // PADS
            ImVec4(0.3f, 0.7f, 0.4f, 1.0f),  // EXTRA
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f)
        };

        float header_width = 180.0f;
        ImGui::BeginChild("Timeline", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        draw_list = ImGui::GetWindowDrawList(); // Pegar o draw_list AQUI garante o clipping do ChildWindow!
        
        static float pixels_per_second = 15.0f;
        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsWindowHovered() && io.KeyCtrl && io.MouseWheel != 0.0f) {
            pixels_per_second += io.MouseWheel * 1.5f;
            if (pixels_per_second < 4.0f) pixels_per_second = 4.0f;
            if (pixels_per_second > 200.0f) pixels_per_second = 200.0f;
        }
        int num_tracks = 8;
        float track_height = 55.0f; // Um pouco mais alto para ficar espaçoso
        
        float playhead_x_offset = ((float)global_sample_count / 44100.0f) * pixels_per_second;

        // Calcular a largura total da linha do tempo com base nos clipes e na agulha
        float total_timeline_width = ImGui::GetWindowWidth();
        if (playhead_x_offset + header_width + 500.0f > total_timeline_width) {
            total_timeline_width = playhead_x_offset + header_width + 500.0f;
        }
        for (int i = 0; i < MAX_TRACKS; i++) {
            for (const auto& clip : g_clip_manager.getClips(i)) {
                float clip_end = (clip.start_time_sec + clip.length_sec) * pixels_per_second;
                if (clip_end + header_width + 500.0f > total_timeline_width) {
                    total_timeline_width = clip_end + header_width + 500.0f;
                }
            }
        }

        // --- Régua da Timeline ---
        ImVec2 ruler_pos = ImGui::GetCursorScreenPos();
        float ruler_height = 20.0f;
        
        // Desenha o fundo da régua (cinza escuro)
        draw_list->AddRectFilled(ruler_pos, ImVec2(ruler_pos.x + total_timeline_width, ruler_pos.y + ruler_height), IM_COL32(40, 40, 45, 255));
        draw_list->AddLine(ImVec2(ruler_pos.x, ruler_pos.y + ruler_height), ImVec2(ruler_pos.x + total_timeline_width, ruler_pos.y + ruler_height), IM_COL32(60, 60, 70, 255), 1.0f);
        
        // Desenhar os ticks da régua
        // Cada tick representa 6 segundos (0, 1, 2, 3, 4, 5... na régua correspondem a 0s, 6s, 12s, 18s, 24s, 30s...)
        // Lembrete: 6 segundos = 6 * pixels_per_second = 300 pixels.
        float scroll_x = ImGui::GetScrollX();
        float window_width = ImGui::GetWindowWidth();
        float tick_spacing = 6.0f * pixels_per_second; // 300.0f
        
        // Find visible ticks range
        int start_tick = (int)(scroll_x / tick_spacing);
        int end_tick = (int)((scroll_x + window_width) / tick_spacing) + 2;
        
        float visible_min_x = ImGui::GetWindowPos().x + header_width + 10;
        float visible_max_x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
        
        // Clip ticks to visible grid area
        draw_list->PushClipRect(ImVec2(visible_min_x, ruler_pos.y), ImVec2(visible_max_x, ruler_pos.y + ruler_height), true);
        
        for (int t = start_tick; t <= end_tick; t++) {
            float tx = ruler_pos.x + header_width + 10 + t * tick_spacing;
            
            if (t % 5 == 0) {
                // Major tick: maior, com número
                draw_list->AddLine(ImVec2(tx, ruler_pos.y + 4.0f), ImVec2(tx, ruler_pos.y + ruler_height), IM_COL32(200, 200, 200, 255), 1.5f);
                
                char label[16];
                snprintf(label, sizeof(label), "%d", t); // 0, 5, 10, 15...
                draw_list->AddText(ImVec2(tx + 4, ruler_pos.y + 2), IM_COL32(200, 200, 200, 255), label);
            } else {
                // Minor tick
                draw_list->AddLine(ImVec2(tx, ruler_pos.y + 12.0f), ImVec2(tx, ruler_pos.y + ruler_height), IM_COL32(120, 120, 120, 255), 1.0f);
            }
        }
        draw_list->PopClipRect();
        
        // Cabeçalho da Régua (Fixo na esquerda)
        scroll_x = ImGui::GetScrollX();
        ImVec2 header_ruler_min = ImVec2(ruler_pos.x + scroll_x, ruler_pos.y);
        ImVec2 header_ruler_max = ImVec2(header_ruler_min.x + header_width, ruler_pos.y + ruler_height);
        draw_list->AddRectFilled(header_ruler_min, header_ruler_max, IM_COL32(25, 25, 30, 255));
        draw_list->AddLine(ImVec2(header_ruler_max.x, header_ruler_min.y), ImVec2(header_ruler_max.x, header_ruler_max.y), IM_COL32(60, 60, 70, 255), 2.0f);
        
        // Pula o cursor da UI para baixo da régua
        ImGui::Dummy(ImVec2(total_timeline_width, ruler_height));

        for (int i = 0; i < num_tracks; i++) {
            ImGui::PushID(i);
            
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            // p_min.x já considera o scroll negativo da janela do ImGui!
            // Então, a largura visual da track precisa ser total_timeline_width real.
            ImVec2 p_max_track = ImVec2(p_min.x + total_timeline_width, p_min.y + track_height);
            
            // Fundo da Faixa
            draw_list->AddRectFilled(p_min, p_max_track, selected_track_idx == i ? IM_COL32(30, 40, 30, 255) : IM_COL32(15, 15, 18, 255));
            draw_list->AddRect(p_min, p_max_track, IM_COL32(40, 40, 50, 255));
            
            // Cabeçalho da Faixa (Acompanha o scroll na visualização, mas vamos prender ele na esquerda da tela subtraindo o scrollX)
            float scroll_x = ImGui::GetScrollX();
            ImVec2 header_p_min = ImVec2(p_min.x + scroll_x, p_min.y);
            ImVec2 header_p_max = ImVec2(header_p_min.x + header_width, p_min.y + track_height);
            
            draw_list->AddRectFilled(header_p_min, header_p_max, IM_COL32(25, 25, 30, 255));
            draw_list->AddLine(ImVec2(header_p_max.x, header_p_min.y), ImVec2(header_p_max.x, header_p_max.y), IM_COL32(60, 60, 70, 255), 2.0f);
            
            // Linha colorida de destaque no cabeçalho
            draw_list->AddRectFilled(header_p_min, ImVec2(header_p_min.x + 6, header_p_max.y), ImColor(track_colors[i]));
            
            ImGui::SetCursorScreenPos(ImVec2(header_p_min.x + 12, p_min.y + 5));
            
            // Seleção de Faixa
            if (ImGui::Selectable((track_names[i] + "##sel" + std::to_string(i)).c_str(), selected_track_idx == i, 0, ImVec2(100, 15))) {
                selected_track_idx = i;
            }
            
            ImGui::SetCursorScreenPos(ImVec2(header_p_min.x + 12, p_min.y + 28));
            
            if (::track_mutes[i]) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            }
            if (ImGui::Button((std::string("M##m") + std::to_string(i)).c_str(), ImVec2(24, 20))) { ::track_mutes[i] = !::track_mutes[i]; }
            ImGui::PopStyleColor();
            
            ImGui::SameLine();
            if (::track_solos[i]) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.2f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            }
            if (ImGui::Button((std::string("S##s") + std::to_string(i)).c_str(), ImVec2(24, 20))) { ::track_solos[i] = !::track_solos[i]; }
            ImGui::PopStyleColor();

            // FASE 28: FX RACK MODULAR
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.2f, 0.1f, 1.0f));
            if (ImGui::Button((std::string("+ FX##fx") + std::to_string(i)).c_str(), ImVec2(35, 20))) {
                ImGui::OpenPopup((std::string("fx_menu_") + std::to_string(i)).c_str());
            }
            ImGui::PopStyleColor();

            if (ImGui::BeginPopup((std::string("fx_menu_") + std::to_string(i)).c_str())) {
                ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "ADICIONAR EFEITO");
                ImGui::Separator();
                if (ImGui::MenuItem("Gothic Overdrive")) { track_fx_chain[i].push_back(1); }
                if (ImGui::MenuItem("Ring Modulator")) { track_fx_chain[i].push_back(2); }
                if (ImGui::MenuItem("Tremolo Ritual")) { track_fx_chain[i].push_back(3); }
                if (ImGui::MenuItem("Psych Flanger")) { track_fx_chain[i].push_back(4); }
                if (ImGui::MenuItem("Abyss Pitch")) { track_fx_chain[i].push_back(5); }
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            for (size_t fx_idx = 0; fx_idx < track_fx_chain[i].size(); fx_idx++) {
                int fx_id = track_fx_chain[i][fx_idx];
                ImVec4 color;
                std::string label;
                if (fx_id == 1) { color = ImVec4(0.8f, 0.2f, 0.2f, 1.0f); label = "DRV"; }
                else if (fx_id == 2) { color = ImVec4(0.2f, 0.8f, 0.2f, 1.0f); label = "RNG"; }
                else if (fx_id == 3) { color = ImVec4(0.8f, 0.2f, 0.8f, 1.0f); label = "TRM"; }
                else if (fx_id == 4) { color = ImVec4(0.2f, 0.8f, 0.8f, 1.0f); label = "FLG"; }
                else if (fx_id == 5) { color = ImVec4(0.8f, 0.8f, 0.2f, 1.0f); label = "PTC"; }

                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0,0,0,1));
                std::string btn_id = label + "##fxblock" + std::to_string(i) + "_" + std::to_string(fx_idx);
                if (ImGui::Button(btn_id.c_str(), ImVec2(24, 20))) {
                    bool found = false;
                    for (auto& win : active_plugin_windows) {
                        if (win.track_idx == i && win.plugin_id == fx_id) {
                            win.is_open = true;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        FloatingPluginWindow new_win;
                        new_win.track_idx = i;
                        new_win.plugin_id = fx_id;
                        new_win.name = label + " Track " + std::to_string(i+1);
                        active_plugin_windows.push_back(new_win);
                    }
                }
                ImGui::PopStyleColor(2);

                // DRAG & DROP
                if (ImGui::BeginDragDropSource()) {
                    int payload_data[2] = { i, (int)fx_idx };
                    ImGui::SetDragDropPayload("FX_BLOCK", &payload_data, sizeof(payload_data));
                    ImGui::Text("Movendo %s", label.c_str());
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FX_BLOCK")) {
                        int* data = (int*)payload->Data;
                        int src_track = data[0];
                        int src_fx_idx = data[1];
                        
                        if (src_track == i && src_fx_idx != fx_idx) {
                            int moving_fx = track_fx_chain[i][src_fx_idx];
                            track_fx_chain[i].erase(track_fx_chain[i].begin() + src_fx_idx);
                            track_fx_chain[i].insert(track_fx_chain[i].begin() + fx_idx, moving_fx);
                        } else if (src_track != i) {
                            int moving_fx = track_fx_chain[src_track][src_fx_idx];
                            track_fx_chain[src_track].erase(track_fx_chain[src_track].begin() + src_fx_idx);
                            track_fx_chain[i].insert(track_fx_chain[i].begin() + fx_idx, moving_fx);
                            for (auto& win : active_plugin_windows) {
                                if (win.track_idx == src_track && win.plugin_id == moving_fx) win.track_idx = i;
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                
                // Tooltip para Excluir
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Clique para Abrir. Botão Direito para Excluir.");
                    if (ImGui::IsMouseClicked(1)) { // Botão direito
                        track_fx_chain[i].erase(track_fx_chain[i].begin() + fx_idx);
                        // Opcionalmente, pode fechar a janela também, mas conforme a regra, apenas esconde/exclui o processamento
                        for (auto it = active_plugin_windows.begin(); it != active_plugin_windows.end(); ) {
                            if (it->track_idx == i && it->plugin_id == fx_id) {
                                it = active_plugin_windows.erase(it);
                            } else {
                                ++it;
                            }
                        }
                    }
                }

                if (fx_idx < track_fx_chain[i].size() - 1) ImGui::SameLine();
            }

            // Timeline Waveforms
            ImU32 col = ImColor(track_colors[i]);
            float mid_y = p_min.y + (track_height / 2);
            float start_x = p_min.x + header_width + 10;
            
            const auto& wave_data = ai_engine.getWaveformOverview(i);
            auto clips = g_clip_manager.getClips(i);
            
            // Visibilidade real da tela
            float screen_view_min_x = p_min.x + scroll_x + header_width;
            float screen_view_max_x = p_min.x + scroll_x + ImGui::GetWindowWidth();
            
            for (size_t c_idx = 0; c_idx < clips.size(); c_idx++) {
                const auto& clip = clips[c_idx];
                float clip_start_x = start_x + (clip.start_time_sec * pixels_per_second);
                float clip_width = clip.length_sec * pixels_per_second;
                float clip_end_x = clip_start_x + clip_width;
                
                if (clip_end_x > screen_view_min_x && clip_start_x < screen_view_max_x) {
                    float draw_start = std::max(clip_start_x, screen_view_min_x);
                    float draw_end = std::min(clip_end_x, screen_view_max_x);
                    
                    ImU32 fill_col = ImColor(track_colors[i].x, track_colors[i].y, track_colors[i].z, clip.is_selected ? 0.4f : 0.15f);
                    ImU32 outline_col = ImColor(track_colors[i].x, track_colors[i].y, track_colors[i].z, 0.8f);
                    
                    draw_list->AddRectFilled(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), fill_col, 4.0f);
                    draw_list->AddRect(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), outline_col, 4.0f);
                    
                    if (wave_data.size() > 0) {
                        float source_px_start = clip.source_offset_sec * pixels_per_second;
                        for (float px = 0; px < clip_width; px++) {
                            float point_x = clip_start_x + px;
                            if (point_x >= draw_start && point_x <= draw_end) {
                                size_t wave_idx = (size_t)(source_px_start + px);
                                if (wave_idx < wave_data.size()) {
                                    float amp = wave_data[wave_idx] * ((track_height / 2) - 4); 
                                    draw_list->AddLine(ImVec2(point_x, mid_y - amp), ImVec2(point_x, mid_y + amp), col, 1.0f);
                                }
                            }
                        }
                    }
                    
                    ImGui::SetCursorScreenPos(ImVec2(draw_start, p_min.y));
                    ImGui::InvisibleButton("##clip_btn", ImVec2(draw_end - draw_start, track_height));
                }
            }

            ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_min.y)); 
            ImGui::Dummy(ImVec2(total_timeline_width, track_height));
            ImGui::PopID();
            ImGui::Spacing();
        }

        // Agulha (Playhead) Verde Neon e Interação
        if (is_playing) {
            float target_scroll = playhead_x_offset - (ImGui::GetWindowWidth() * 0.5f);
            if (target_scroll < 0) target_scroll = 0;
            ImGui::SetScrollX(target_scroll);
        }
        
        // Interação de Clique/Arraste na Timeline
        ImVec2 timeline_min = ImGui::GetWindowPos();
        ImVec2 timeline_max = ImVec2(timeline_min.x + ImGui::GetWindowSize().x, timeline_min.y + ImGui::GetWindowSize().y);
        bool is_timeline_hovered = ImGui::IsMouseHoveringRect(timeline_min, timeline_max, true);

        if (is_timeline_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            if (mouse_pos.x > ImGui::GetWindowPos().x + header_width) { // Clicou depois do cabeçalho
                float click_x_in_timeline = mouse_pos.x - (ImGui::GetWindowPos().x + header_width + 10 - ImGui::GetScrollX());
                if (click_x_in_timeline < 0) click_x_in_timeline = 0;
                float clicked_time_sec = click_x_in_timeline / pixels_per_second;
                global_sample_count = (unsigned long long)(clicked_time_sec * 44100.0f);
                timeline.setMasterFrame(global_sample_count);
                playhead_x_offset = click_x_in_timeline; // Update instantâneo visual
            }
        }
        
        float playhead_x = ImGui::GetWindowPos().x + header_width + 10 - ImGui::GetScrollX() + playhead_x_offset; 
        float min_playhead_x = ImGui::GetWindowPos().x + header_width + 10;
        
        // Clip playhead line and triangle so it doesn't leak into the track headers
        float playhead_y_min = ImGui::GetWindowPos().y; 
        float playhead_y_max = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
        
        draw_list->PushClipRect(ImVec2(min_playhead_x, playhead_y_min), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, playhead_y_max), true);
        if (playhead_x >= min_playhead_x) {
            draw_list->AddLine(ImVec2(playhead_x, playhead_y_min), ImVec2(playhead_x, playhead_y_max), IM_COL32(57, 255, 20, 200), 2.0f);
            draw_list->AddTriangleFilled(ImVec2(playhead_x - 6, playhead_y_min), ImVec2(playhead_x + 6, playhead_y_min), ImVec2(playhead_x, playhead_y_min + 8), IM_COL32(57, 255, 20, 255));
        }
        draw_list->PopClipRect();

        ImGui::EndChild(); // Timeline
        ImGui::EndChild(); // CentralArea
        ImGui::EndChild(); // CentralAreaGroup

        // ==========================================
        // 3. BOTTOM RACK (DEVICE VIEW)
        // ==========================================
        ImGui::BeginChild("BottomPanel", ImVec2(0, 0), true);
        
        // Header do Device View
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "DEVICE RACK: %s", track_names[selected_track_idx].c_str());
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("DeviceTabs")) {
            // O FX RACK foi removido daqui e transferido para o Track Header e Viewports (Fase 28)
            if (ImGui::BeginTabItem("SYNTH (KuroWave)")) {
                ImGui::Spacing();
                KuroUI::RenderKuroWaveSynth(g_kurowave);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("SAMPLER")) {
                ImGui::Spacing();
                KuroUI::RenderKuroSampler(g_global_sampler);
                ImGui::EndTabItem();
            }
            // Aba PIANO ROLL removida a pedido do usuario (virou botao no topo e janela flutuante)
            
            ImGuiTabItemFlags mixer_flags = set_mixer_focus ? ImGuiTabItemFlags_SetSelected : 0;
            if (ImGui::BeginTabItem("GLOBAL MIXER", nullptr, mixer_flags)) {
                ImGui::Spacing();
                // Renderizador de mixer global simples
                ImGui::Columns(9, "MixerCols", true);
                for (int i = 0; i < 9; i++) {
                    if (i < MAX_TRACKS) {
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", track_names[i].c_str());
                        ImGui::Spacing();
                        float slider_h = 180.0f;
                        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.22f, 1.0f, 0.08f, 1.0f));
                        if (ImGui::VSliderFloat((std::string("##v") + std::to_string(i)).c_str(), ImVec2(40, slider_h), &::track_volumes[i], -60.0f, 6.0f, "")) {
                            // Envia evento de volume futuramente
                        }
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "MASTER");
                    }
                    ImGui::NextColumn();
                }
                ImGui::Columns(1);
                ImGui::EndTabItem();
            }
            if (set_mixer_focus) set_mixer_focus = false;
            
            ImGui::EndTabBar();
        }

        ImGui::EndChild(); // BottomPanel

        // ==========================================
        // 4. ATALHOS GLOBAIS (FL STUDIO STYLE)
        // ==========================================
        if (!io.WantCaptureKeyboard) {
            if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
                is_playing = !is_playing;
                timeline.setPlaying(is_playing);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_S) && io.KeyCtrl) {
                g_record_manager.saveToWav("Kuro_Export_Master.wav");
            }
            if (ImGui::IsKeyPressed(ImGuiKey_M)) {
                ::track_mutes[selected_track_idx] = !::track_mutes[selected_track_idx];
            }
            if (ImGui::IsKeyPressed(ImGuiKey_S) && !io.KeyCtrl) {
                ::track_solos[selected_track_idx] = !::track_solos[selected_track_idx];
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
                set_mixer_focus = true;
            }
        }
    }

    // FASE 28: Renderiza Janelas Flutuantes (Fora da Janela Principal)
    inline void RenderFloatingWindows() {
        if (show_piano_roll) {
            KuroUI::RenderPianoRoll(&show_piano_roll, ::timeline, selected_track_idx, timeline.getBPM(), &::global_sample_count, ::is_playing, nullptr);
        }
        if (show_step_sequencer) {
            KuroUI::RenderStepSequencer(&show_step_sequencer, ::timeline, timeline.getBPM());
        }
        if (show_modulation_panel) {
            KuroUI::RenderModulationPanel(&show_modulation_panel);
        }
    }

    // Renderiza Modos Mockados para DJ e Stems (Fase 24)
    static void RenderDJMode() {
        ImGui::BeginChild("DJMode", ImVec2(0, 0), true);
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "🎛️ MODO DJ: EM CONSTRUÇÃO");
        ImGui::Separator();
        
        if (g_dj_engine) {
            ImGui::Spacing();
            ImGui::Columns(2, "Decks", true);
            // Deck A
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "DECK A");
            ImGui::TextWrapped("Faixa: %s", g_dj_engine->getDeckATrack().c_str());
            if (ImGui::Button("Carregar A", ImVec2(100, 30))) {
                std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                if (!path.empty()) g_dj_engine->loadDeckA(path);
            }
            ImGui::SameLine();
            if (ImGui::Button(g_dj_engine->isDeckAPlaying() ? "Pause A" : "Play A", ImVec2(100, 30))) {
                if (g_dj_engine->isDeckAPlaying()) g_dj_engine->pauseDeckA();
                else g_dj_engine->playDeckA();
            }
            ImGui::NextColumn();
            
            // Deck B
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "DECK B");
            ImGui::TextWrapped("Faixa: %s", g_dj_engine->getDeckBTrack().c_str());
            if (ImGui::Button("Carregar B", ImVec2(100, 30))) {
                std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                if (!path.empty()) g_dj_engine->loadDeckB(path);
            }
            ImGui::SameLine();
            if (ImGui::Button(g_dj_engine->isDeckBPlaying() ? "Pause B" : "Play B", ImVec2(100, 30))) {
                if (g_dj_engine->isDeckBPlaying()) g_dj_engine->pauseDeckB();
                else g_dj_engine->playDeckB();
            }
            ImGui::Columns(1);
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Mixer Central Placeholder
            ImGui::Text("CROSSFADER");
            float cf = g_dj_engine->getCrossfader();
            if (ImGui::SliderFloat("##crossfader", &cf, 0.0f, 1.0f, "%.2f")) {
                g_dj_engine->setCrossfader(cf);
            }
            if (ImGui::Button("SYNC BPM", ImVec2(200, 40))) {
                g_dj_engine->syncBPM();
            }
        }
        ImGui::EndChild();
    }

    static void RenderStemMode() {
        ImGui::BeginChild("StemMode", ImVec2(0, 0), true);
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "🧬 MODO EXTRAÇÃO DE STEMS: EM CONSTRUÇÃO");
        ImGui::Separator();
        
        if (g_stem_engine) {
            ImGui::TextWrapped("Arquivo Isolado Atual: %s", g_stem_engine->getCurrentFile().c_str());
            if (ImGui::Button("Carregar Faixa para Isolar", ImVec2(300, 40))) {
                std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                if (!path.empty()) g_stem_engine->dropFileToIsolate(path);
            }
            
            ImGui::Spacing();
            if (g_stem_engine->isProcessing()) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Status: %s", g_stem_engine->getStatus().c_str());
                ImGui::ProgressBar(g_stem_engine->getProgress(), ImVec2(-1, 0));
            } else if (g_stem_engine->isFinished()) {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Pronto para exportar!");
                ImGui::Spacing();
                if (ImGui::Button("Baixar Kick/Bass", ImVec2(200, 30))) g_stem_engine->exportSingleStem(0);
                if (ImGui::Button("Baixar Leads/Drums", ImVec2(200, 30))) g_stem_engine->exportSingleStem(1);
                if (ImGui::Button("Baixar Vocals", ImVec2(200, 30))) g_stem_engine->exportSingleStem(2);
                if (ImGui::Button("Baixar FX", ImVec2(200, 30))) g_stem_engine->exportSingleStem(3);
            }
        }
        ImGui::EndChild();
    }

#include <GLFW/glfw3.h> // Para controle nativo da janela (Min/Max/Close)

    static void RenderMainMenu(StemSeparationEngine& ai_engine) {
        if (ImGui::BeginMenuBar()) {
            
            // Drag logic for borderless window
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                GLFWwindow* win = glfwGetCurrentContext();
                if (win) {
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    int wx, wy;
                    glfwGetWindowPos(win, &wx, &wy);
                    glfwSetWindowPos(win, wx + (int)delta.x, wy + (int)delta.y);
                }
            }

            if (ImGui::BeginMenu("Arquivo")) {
                if (ImGui::MenuItem("Novo Projeto")) {
                    ai_engine.reset();
                    g_clip_manager.reset();
                    global_sample_count = 0;
                    timeline.setPlaying(false);
                }
                if (ImGui::MenuItem("Abrir Projeto...")) {
                    std::string path = KuroUI::FileDialog::OpenFile("Abduction Project (*.kuro)\0*.kuro\0");
                    if (!path.empty()) {
                        ProjectManagerBridge::Load(path);
                    }
                }
                if (ImGui::MenuItem("Salvar Projeto...")) {
                    std::string path = KuroUI::FileDialog::SaveFile("Abduction Project (*.kuro)\0*.kuro\0");
                    if (!path.empty()) {
                        if (path.find(".kuro") == std::string::npos) path += ".kuro";
                        ProjectManagerBridge::Save(path);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Importar Áudio...")) {
                    std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                    if (!path.empty()) {
                        if (g_global_sampler && (path.find(".wav") != std::string::npos || path.find(".WAV") != std::string::npos)) {
                            g_global_sampler->loadSample(path);
                        }
                        if (!ai_engine.isRunning()) {
                            ai_engine.startProcessing(path);
                        }
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exportar Stems...")) {
                    KuroAudio::OfflineRenderer::RenderStems(master_graph, 10.0f, ".");
                }
                if (ImGui::MenuItem("Exportar Master (.wav)...")) {
                    g_record_manager.saveToWav("Kuro_Export_Master.wav");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Configurações da DAW")) {}
                if (ImGui::MenuItem("Sair")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Editar")) {
                if (ImGui::MenuItem("Desfazer", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Refazer", "Ctrl+Y")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Cortar", "Ctrl+X")) {}
                if (ImGui::MenuItem("Copiar", "Ctrl+C")) {}
                if (ImGui::MenuItem("Colar", "Ctrl+V")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Exibir")) {
                if (ImGui::MenuItem("Mostrar Browser", "", true)) {}
                if (ImGui::MenuItem("Mostrar Mixer", "", true)) {}
                ImGui::Separator();
                if (ImGui::BeginMenu("Temas")) {
                    if (ImGui::MenuItem("Abduction (Nave)", "", true)) {}
                    if (ImGui::MenuItem("Gótico Kuro")) {}
                    if (ImGui::MenuItem("Ableton Clássico")) {}
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Modos")) {
                if (ImGui::MenuItem("Modo Studio", "", current_app_mode == AppMode::STUDIO_MODE)) {
                    current_app_mode = AppMode::STUDIO_MODE;
                }
                if (ImGui::MenuItem("Modo DJ", "", current_app_mode == AppMode::DJ_MODE)) {
                    current_app_mode = AppMode::DJ_MODE;
                }
                if (ImGui::MenuItem("Modo Extração (Stems)", "", current_app_mode == AppMode::STEM_MODE)) {
                    current_app_mode = AppMode::STEM_MODE;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Ajuda")) {
                if (ImGui::MenuItem("Documentação")) {}
                if (ImGui::MenuItem("Atalhos do Teclado")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Sobre o Abduction Studio")) {}
                ImGui::EndMenu();
            }
            
            // Window Controls (Min, Max, Close) na direita
            float menu_width = ImGui::GetWindowWidth();
            ImGui::SameLine(menu_width - 120);
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
            if (ImGui::Button("_", ImVec2(30, 20))) {
                GLFWwindow* win = glfwGetCurrentContext();
                if (win) glfwIconifyWindow(win);
            }
            ImGui::SameLine();
            if (ImGui::Button("[ ]", ImVec2(30, 20))) {
                GLFWwindow* win = glfwGetCurrentContext();
                if (win) {
                    if (glfwGetWindowAttrib(win, GLFW_MAXIMIZED)) glfwRestoreWindow(win);
                    else glfwMaximizeWindow(win);
                }
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("X", ImVec2(30, 20))) {
                GLFWwindow* win = glfwGetCurrentContext();
                if (win) glfwSetWindowShouldClose(win, GLFW_TRUE);
            }
            ImGui::PopStyleColor(); // text
            ImGui::PopStyleColor(); // bg

            ImGui::EndMenuBar();
        }
    }

    // FASE 28: Renderizar Janelas Flutuantes e Sincronizar DSP
    static void RenderFloatingPluginWindows() {
        // Usar a declaração global de KuroDSPUI.h
        
        // Sincronizar UI com o C++ DSP (Pedalboard)
        for (int i = 0; i < MAX_TRACKS; i++) {
            auto& board = ::track_pedalboards[i];
            board.preset_dark = false;
            board.preset_alien = false;
            board.preset_ritual = false;
            board.preset_psych = false;
            board.enable_abyss_pitch = false;
            
            for (int fx_id : track_fx_chain[i]) {
                bool bypassed = false;
                for (auto& win : active_plugin_windows) {
                    if (win.track_idx == i && win.plugin_id == fx_id) { bypassed = win.is_bypassed; break; }
                }
                if (!bypassed) {
                    if (fx_id == 1) board.preset_dark = true;
                    if (fx_id == 2) board.preset_alien = true;
                    if (fx_id == 3) board.preset_ritual = true;
                    if (fx_id == 4) board.preset_psych = true;
                    if (fx_id == 5) board.enable_abyss_pitch = true;
                }
            }
        }

        // Renderizar Janelas
        for (auto& win : active_plugin_windows) {
            if (!win.is_open) continue;
            
            ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
            std::string window_id = win.name + "##win" + std::to_string(win.track_idx) + "_" + std::to_string(win.plugin_id);
            
            if (ImGui::Begin(window_id.c_str(), &win.is_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing)) {
                auto& board = ::track_pedalboards[win.track_idx];

                // Header customizado na janela flutuante
                if (win.is_bypassed) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 1.0f, 0.08f, 1.0f));
                }
                if (ImGui::Button("BYPASS", ImVec2(80, 25))) { win.is_bypassed = !win.is_bypassed; }
                ImGui::PopStyleColor();
                
                ImGui::SameLine(ImGui::GetWindowWidth() - 90);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("EXCLUIR", ImVec2(80, 25))) {
                    auto& chain = track_fx_chain[win.track_idx];
                    for (auto it = chain.begin(); it != chain.end(); ++it) {
                        if (*it == win.plugin_id) { chain.erase(it); break; }
                    }
                    win.is_open = false; 
                }
                ImGui::PopStyleColor();
                
                ImGui::Separator();
                ImGui::Spacing();
                
                // Controles de Efeito (DSP)
                if (win.plugin_id == 1) {
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "GOTHIC OVERDRIVE");
                    ImGui::SliderFloat("Drive", &board.param_dark_drive, 1.0f, 10.0f);
                } else if (win.plugin_id == 2) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "RING MODULATOR");
                    ImGui::SliderFloat("Frequency", &board.param_alien_freq, 20.0f, 1000.0f);
                } else if (win.plugin_id == 3) {
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.8f, 1.0f), "TREMOLO RITUAL");
                    ImGui::SliderFloat("Rate", &board.param_ritual_rate, 0.1f, 5.0f);
                    ImGui::SliderFloat("Depth", &board.param_ritual_depth, 0.0f, 1.0f);
                } else if (win.plugin_id == 4) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.8f, 1.0f), "PSYCH FLANGER");
                    ImGui::SliderFloat("Speed", &board.param_psych_speed, 0.01f, 2.0f);
                    ImGui::SliderFloat("Depth", &board.param_psych_depth, 0.0f, 1.0f);
                } else if (win.plugin_id == 5) {
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "ABYSS PITCH");
                    ImGui::SliderFloat("Pitch Ratio", &board.abyss_pitch_factor, 0.5f, 2.0f);
                }
            }
            ImGui::End();
        }
    }

    // Função Raiz que gerencia a janela principal (Fase 24)
    static void RenderMainApp(StemSeparationEngine& ai_engine) {
        RenderMainMenu(ai_engine);

        switch (current_app_mode) {
            case AppMode::STUDIO_MODE:
                RenderStudioMode(ai_engine);
                RenderFloatingPluginWindows(); // Renderizar Janelas de Plugins (Viewports)
                break;
            case AppMode::DJ_MODE:
                RenderDJMode();
                break;
            case AppMode::STEM_MODE:
                RenderStemMode();
                break;
        }

        // ==========================================
        // 5. HUD MODAL DE CARREGAMENTO (FASE 26)
        // ==========================================
        static bool was_running = false;
        if (ai_engine.isRunning() && !was_running) {
            ImGui::OpenPopup("LoadingModal");
        }
        was_running = ai_engine.isRunning();

        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 180));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.05f, 0.15f, 0.05f, 0.95f)); // Transparente esverdeado
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 1.0f, 0.08f, 1.0f));
        if (ImGui::BeginPopupModal("LoadingModal", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "🛸 PROCESSANDO ÁUDIO...");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextWrapped("%s", ai_engine.getStatus().c_str());
            ImGui::Spacing();
            ImGui::ProgressBar(ai_engine.getProgress(), ImVec2(-1, 0));
            if (!ai_engine.isRunning()) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
    }
}
