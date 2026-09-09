#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "../core/DJEngine.h"
#include "../core/DDJ200DeviceManager.h"
#include "../core/SpotifyDJService.h"
#include "../core/DJExportService.h"
#include "FileDialog.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroUI {

    class KuroDJStudioUI {
    public:
        bool is_open = false;
        bool is_fullscreen = true;
        KuroAudio::DDJ200DeviceManager ddj_hardware;
        KuroAudio::SpotifyDJService spotify_service;
        
        // Cores temáticas Neon / Obsidian Glassmorphism (4 Decks)
        ImVec4 col_deck_1 = ImVec4(0.00f, 0.96f, 0.83f, 1.0f); // Neon Cyan #00f5d4 (Deck 1)
        ImVec4 col_deck_2 = ImVec4(1.00f, 0.00f, 0.50f, 1.0f); // Neon Magenta #ff007f (Deck 2)
        ImVec4 col_deck_3 = ImVec4(1.00f, 0.65f, 0.00f, 1.0f); // Warm Amber #ffaa00 (Deck 3)
        ImVec4 col_deck_4 = ImVec4(0.00f, 1.00f, 0.40f, 1.0f); // Toxic Green #00ff66 (Deck 4)

        ImVec4 col_bg_dark = ImVec4(0.03f, 0.04f, 0.07f, 0.98f);
        ImVec4 col_card = ImVec4(0.06f, 0.08f, 0.12f, 0.95f);
        ImVec4 col_border = ImVec4(0.14f, 0.20f, 0.32f, 0.85f);

        char search_query_buf[128] = "";
        char new_folder_name_buf[64] = "";
        bool show_new_folder_dialog = false;
        int selected_track_idx = 0;
        bool show_playlist_drawer = false; // Oculto por padrão para dar foco total às waveforms
        bool show_export_modal = false;
        bool show_search_modal = false;
        bool show_exit_confirm_modal = false;
        char online_search_query_buf[128] = "";

        // Estado do Exportador Pen Drive USB
        int selected_drive_idx = 0;
        bool opt_export_rekordbox = true;
        bool opt_export_universal = true;
        std::string export_status_msg = "";
        std::vector<KuroAudio::USBDeviceDrive> detected_drives;

        KuroDJStudioUI() = default;

        void init(KuroAudio::DJEngine* engine) {
            ddj_hardware.setEngine(engine);
            ddj_hardware.autoConnect();

            // Carrega Hot Cues salvos anteriormente
            if (engine) engine->loadHotCues();
            
            // Varredura cirúrgica de todas as músicas locais baixadas para os Crates / Vertentes
            KuroAudio::DJLibraryManager::getInstance().scanLocalTracks("scratch/tracks");

            // Carrega faixas cacheadas de buscas online se existirem
            if (std::filesystem::exists("scratch/tracks/search_results.tsv")) {
                spotify_service.parseSearchResultsTsv("scratch/tracks/search_results.tsv");
            }
            
            // Navegação de playlist pelo jog com shift
            ddj_hardware.setBrowseCallback([this](int delta) {
                auto active_fid = KuroAudio::DJLibraryManager::getInstance().getActiveFolderId();
                auto tracks = KuroAudio::DJLibraryManager::getInstance().getTracksForFolder(active_fid);
                if (!tracks.empty()) {
                    if (delta > 0) {
                        selected_track_idx = (selected_track_idx + 1) % (int)tracks.size();
                    } else if (delta < 0) {
                        selected_track_idx = (selected_track_idx - 1 + (int)tracks.size()) % (int)tracks.size();
                    }
                }
            });
        }

        static bool DrawRotaryKnob(const char* label, float* p_value, float v_min, float v_max, float radius, ImVec4 theme_col, const char* unit_fmt = "%.1fx", float default_val = 1.0f) {
            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            
            float total_w = radius * 2.0f + 16.0f;
            float total_h = radius * 2.0f + 18.0f;
            
            ImGui::InvisibleButton(label, ImVec2(total_w, total_h));
            bool is_hovered = ImGui::IsItemHovered();
            bool is_active = ImGui::IsItemActive();
            
            if (is_active && io.MouseDelta.y != 0.0f) {
                float step = (v_max - v_min) * 0.006f;
                *p_value -= io.MouseDelta.y * step;
                *p_value = std::clamp(*p_value, v_min, v_max);
            }
            if (is_hovered && io.MouseWheel != 0.0f) {
                float step = (v_max - v_min) * 0.03f;
                *p_value += io.MouseWheel * step;
                *p_value = std::clamp(*p_value, v_min, v_max);
            }
            if (is_hovered && ImGui::IsMouseDoubleClicked(0)) {
                *p_value = default_val;
            }
            
            ImVec2 center = ImVec2(pos.x + total_w * 0.5f, pos.y + radius + 9.0f);
            
            std::string disp_label = label;
            size_t hash_pos = disp_label.find("##");
            if (hash_pos != std::string::npos) disp_label = disp_label.substr(0, hash_pos);
            
            ImVec2 text_sz = ImGui::CalcTextSize(disp_label.c_str());
            dl->AddText(ImVec2(center.x - text_sz.x * 0.5f, pos.y - 2.0f), is_hovered ? ImGui::GetColorU32(theme_col) : IM_COL32(185, 200, 220, 240), disp_label.c_str());
            
            float angle_min = 0.75f * (float)M_PI;
            float angle_max = 2.25f * (float)M_PI;
            float norm = (*p_value - v_min) / (v_max - v_min);
            norm = std::clamp(norm, 0.0f, 1.0f);
            float angle_val = angle_min + norm * (angle_max - angle_min);
            
            // Trilho inativo
            dl->PathArcTo(center, radius + 2.0f, angle_min, angle_max, 16);
            dl->PathStroke(IM_COL32(28, 36, 50, 255), 0, 2.0f);
            
            // Arco de valor ativo
            dl->PathArcTo(center, radius + 2.0f, angle_min, angle_val, 16);
            dl->PathStroke(ImGui::GetColorU32(theme_col), 0, 2.0f);
            
            // Corpo metálico chanfrado do knob
            dl->AddCircleFilled(center, radius, IM_COL32(18, 22, 30, 255), 20);
            dl->AddCircle(center, radius, is_hovered ? ImGui::GetColorU32(theme_col) : IM_COL32(50, 62, 85, 255), 20, 1.0f);
            dl->AddCircleFilled(center, radius * 0.85f, IM_COL32(26, 32, 44, 255), 16);
            
            // Notch indicador de rotação
            ImVec2 pointer_p = ImVec2(center.x + std::cos(angle_val) * (radius * 0.75f), center.y + std::sin(angle_val) * (radius * 0.75f));
            dl->AddLine(center, pointer_p, ImGui::GetColorU32(theme_col), 1.8f);
            dl->AddCircleFilled(pointer_p, 1.3f, IM_COL32(255, 255, 255, 255));
            
            if (is_hovered || is_active) {
                char val_buf[32];
                snprintf(val_buf, sizeof(val_buf), unit_fmt, *p_value);
                ImGui::SetTooltip("%s: %s", disp_label.c_str(), val_buf);
            }
            
            return is_active;
        }

        static void DrawSegmentedVUMeter(ImDrawList* dl, ImVec2 pos, ImVec2 size, float level) {
            int num_segs = 12;
            float gap = 1.5f;
            float seg_h = (size.y - (num_segs - 1) * gap) / num_segs;
            
            dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(10, 14, 20, 255), 2.0f);
            dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(30, 42, 60, 200), 2.0f, 0, 1.0f);
            
            int active_segs = (int)(std::clamp(level, 0.0f, 1.0f) * (float)num_segs + 0.5f);
            
            for (int i = 0; i < num_segs; i++) {
                int seg_idx = num_segs - 1 - i;
                float y_top = pos.y + seg_idx * (seg_h + gap);
                float y_bot = y_top + seg_h;
                
                bool is_lit = (i < active_segs);
                ImU32 col;
                if (i < 8) {
                    col = is_lit ? IM_COL32(0, 245, 140, 255) : IM_COL32(0, 45, 25, 150);
                } else if (i < 10) {
                    col = is_lit ? IM_COL32(255, 195, 0, 255) : IM_COL32(65, 48, 0, 150);
                } else {
                    col = is_lit ? IM_COL32(255, 45, 65, 255) : IM_COL32(65, 10, 15, 150);
                }
                dl->AddRectFilled(ImVec2(pos.x + 1.0f, y_top), ImVec2(pos.x + size.x - 1.0f, y_bot), col, 1.0f);
            }
        }

        void render(KuroAudio::DJEngine& engine, bool* p_open = nullptr) {
            if (p_open && !*p_open) return;
            is_open = true;

            // Auto-reconexão da DDJ-200
            if (!ddj_hardware.isConnected()) {
                static double last_retry = 0.0;
                double now = ImGui::GetTime();
                if (now - last_retry > 4.0) {
                    last_retry = now;
                    ddj_hardware.autoConnect(&engine);
                }
            }

            // Atualiza cargas assíncronas do Spotify para os Decks
            spotify_service.pollPendingLoads(engine);

            // LEDs do hardware DDJ-200 sincronizados com os Decks ativos
            int active_l = ddj_hardware.getActiveLeftDeck();
            int active_r = ddj_hardware.getActiveRightDeck();
            ddj_hardware.updateDeckLED(active_l, engine.decks[active_l].is_playing, engine.decks[active_l].cue_active, false);
            ddj_hardware.updateDeckLED(active_r, engine.decks[active_r].is_playing, engine.decks[active_r].cue_active, false);
            ddj_hardware.updatePadLEDs(active_l, engine.decks[active_l].pad_mode, engine.decks[active_l].hot_cues);
            ddj_hardware.updatePadLEDs(active_r, engine.decks[active_r].pad_mode, engine.decks[active_r].hot_cues);

            // Teclas de Atalho de Seleção de Decks e Tela Cheia
            if (ImGui::IsKeyPressed(ImGuiKey_1, false)) ddj_hardware.setActiveLeftDeck(0);
            if (ImGui::IsKeyPressed(ImGuiKey_3, false)) ddj_hardware.setActiveLeftDeck(2);
            if (ImGui::IsKeyPressed(ImGuiKey_2, false)) ddj_hardware.setActiveRightDeck(1);
            if (ImGui::IsKeyPressed(ImGuiKey_4, false)) ddj_hardware.setActiveRightDeck(3);
            if (ImGui::IsKeyPressed(ImGuiKey_F11, false)) is_fullscreen = !is_fullscreen;

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            if (is_fullscreen) {
                ImGui::SetNextWindowPos(viewport->Pos);
                ImGui::SetNextWindowSize(viewport->Size);
                ImGui::SetNextWindowFocus();
            } else {
                ImGui::SetNextWindowSize(ImVec2(viewport->Size.x * 0.95f, viewport->Size.y * 0.94f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x * 0.025f, viewport->Pos.y + viewport->Size.y * 0.03f), ImGuiCond_FirstUseEver);
            }

            ImGui::PushStyleColor(ImGuiCol_WindowBg, col_bg_dark);
            ImGui::PushStyleColor(ImGuiCol_Border, col_border);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, is_fullscreen ? 0.0f : 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
            if (is_fullscreen) {
                flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
            }

            if (ImGui::Begin("##AbductionDJStudioPro", p_open, flags)) {
                
                // --- 1. TOPBAR HUD ---
                renderTopHUD(engine, p_open);
                ImGui::Spacing();

                // --- 2. REKORDBOX 3-BAND DUAL STACKED WAVEFORMS (HORIZONTAL TOPO) ---
                // Waveforms permanecem coladas no topo, diretamente acima dos Decks e Mixer (Estilo Rekordbox)
                float top_wave_w = ImGui::GetContentRegionAvail().x;
                renderStackedRekordboxWaveforms(engine, top_wave_w, 88.0f);
                ImGui::Spacing();

                // --- 3. CORPO CENTRAL (DECK ESQUERDO ATIVO | MIXER DJM-V10 | DECK DIREITO ATIVO) ---
                float total_w = ImGui::GetContentRegionAvail().x;
                float total_avail_h = ImGui::GetContentRegionAvail().y;
                
                float bottom_drawer_h = 0.0f;
                if (show_playlist_drawer) bottom_drawer_h += 150.0f;
                if (show_export_modal) bottom_drawer_h += 136.0f;

                float central_h = total_avail_h - bottom_drawer_h;
                if (central_h < 350.0f && bottom_drawer_h == 0.0f) central_h = 350.0f;

                float mixer_w = 270.0f;
                float deck_w = (total_w - mixer_w - 16.0f) * 0.5f;

                ImGuiWindowFlags child_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

                ImVec4 col_left  = (active_l == 0 ? col_deck_1 : col_deck_3);
                ImVec4 col_right = (active_r == 1 ? col_deck_2 : col_deck_4);

                // DECK ESQUERDO (DECK 1 ou 3)
                ImGui::BeginChild("##DeckLeft_Child", ImVec2(deck_w, central_h), false, child_flags);
                renderDeckPanel(engine.decks[active_l], 0, col_left, engine);
                ImGui::EndChild();

                ImGui::SameLine(0, 8);

                // MIXER CENTRAL & PIONEER DJM-V10 BEAT FX / CFX
                ImGui::BeginChild("##Mixer_Child", ImVec2(mixer_w, central_h), false, child_flags);
                renderCentralMixer(engine);
                ImGui::EndChild();

                ImGui::SameLine(0, 8);

                // DECK DIREITO (DECK 2 ou 4)
                ImGui::BeginChild("##DeckRight_Child", ImVec2(deck_w, central_h), false, child_flags);
                renderDeckPanel(engine.decks[active_r], 1, col_right, engine);
                ImGui::EndChild();

                // --- 4. GAVETA SPOTIFY / CRATES NO RODAPÉ (SE ATIVA) ---
                // Agora renderizada cirurgicamente no RODAPÉ inferior da tela, nunca separando as waveforms dos decks!
                if (show_playlist_drawer) {
                    ImGui::Spacing();
                    renderPlaylistSpotifyBar(engine);
                }

                // --- 5. GAVETA PEN DRIVE USB NO RODAPÉ (SE ATIVA) ---
                if (show_export_modal) {
                    ImGui::Spacing();
                    renderUSBExportDrawer(engine);
                }

                // --- 6. MODAIS PROFISSIONAIS (BUSCA ONLINE & CONFIRMAÇÃO DE SAÍDA) ---
                renderSearchModal(engine);
                renderExitConfirmModal(engine, p_open);

            }
            ImGui::End();

            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);
        }

    private:
        void renderTopHUD(KuroAudio::DJEngine& engine, bool* p_open) {
            float avail_w = ImGui::GetContentRegionAvail().x;
            
            // Status DDJ-200 Hardware
            bool hw_ok = ddj_hardware.isConnected();
            if (hw_ok) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_PLUG_CIRCLE_CHECK " DDJ-200 (4-DECKS)");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), ICON_FA_PLUG_CIRCLE_EXCLAMATION " DDJ-200 OFF");
                ImGui::SameLine();
                if (ImGui::SmallButton("Reconectar")) {
                    ddj_hardware.autoConnect(&engine);
                }
            }

            // Seletores Rápidos de Decks Ativos no HUD
            ImGui::SameLine(0, 10);
            int left_deck = ddj_hardware.getActiveLeftDeck();
            int right_deck = ddj_hardware.getActiveRightDeck();
            
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "LEFT:");
            ImGui::SameLine();
            if (ImGui::SmallButton(left_deck == 0 ? "[*] 1" : " 1 ")) ddj_hardware.setActiveLeftDeck(0);
            ImGui::SameLine(0, 2);
            if (ImGui::SmallButton(left_deck == 2 ? "[*] 3" : " 3 ")) ddj_hardware.setActiveLeftDeck(2);

            ImGui::SameLine(0, 8);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "RIGHT:");
            ImGui::SameLine();
            if (ImGui::SmallButton(right_deck == 1 ? "[*] 2" : " 2 ")) ddj_hardware.setActiveRightDeck(1);
            ImGui::SameLine(0, 2);
            if (ImGui::SmallButton(right_deck == 3 ? "[*] 4" : " 4 ")) ddj_hardware.setActiveRightDeck(3);

            ImGui::SameLine(0, 14);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "BPM:");
            ImGui::SameLine();
            ImGui::TextColored(col_deck_1, "%.2f", engine.decks[left_deck].bpm);

            // Ações do Topo: Músicas / Busca / Pen Drive / Playlist / Limpar / Tela Cheia / Sair
            float right_actions_w = 720.0f;
            if (avail_w > 950.0f) {
                ImGui::SameLine(std::max(ImGui::GetCursorPosX() + 10.0f, avail_w - right_actions_w));
            } else {
                ImGui::SameLine(0, 8);
            }

            // Botão Músicas (Abre pasta do Explorer)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.24f, 0.38f, 0.90f));
            if (ImGui::Button(ICON_FA_FOLDER_OPEN " MÚSICAS")) {
#ifdef _WIN32
                ShellExecuteA(NULL, "open", "scratch\\tracks", NULL, NULL, SW_SHOW);
#endif
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Abrir pasta de músicas baixadas (scratch/tracks) no Windows Explorer");
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 5);

            // Botão Busca Online Modal
            ImGui::PushStyleColor(ImGuiCol_Button, show_search_modal ? ImVec4(0.00f, 0.70f, 0.85f, 1.0f) : ImVec4(0.15f, 0.35f, 0.45f, 0.90f));
            if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " BUSCA ONLINE")) {
                show_search_modal = !show_search_modal;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Abrir janela de busca online com capas, BPM e importação");
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 5);

            // Botão Pen Drive Export
            ImGui::PushStyleColor(ImGuiCol_Button, show_export_modal ? ImVec4(0.0f, 0.7f, 0.8f, 1.0f) : ImVec4(0.12f, 0.28f, 0.40f, 0.9f));
            if (ImGui::Button(ICON_FA_FLOPPY_DISK " PEN DRIVE")) {
                show_export_modal = !show_export_modal;
                if (show_export_modal) detected_drives = KuroAudio::DJExportService::scanDrives();
            }
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 5);
            if (ImGui::Button(show_playlist_drawer ? ICON_FA_LIST " Ocultar Crates" : ICON_FA_LIST " Ver Crates")) {
                show_playlist_drawer = !show_playlist_drawer;
            }

            ImGui::SameLine(0, 5);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.12f, 0.14f, 0.85f));
            if (ImGui::Button(ICON_FA_TRASH_CAN " LIMPAR")) {
                spotify_service.clearAllDownloadedFiles();
                KuroAudio::DJLibraryManager::getInstance().clearAllDownloadedTracks();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Limpar faixas baixadas da biblioteca para começar do zero");
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 5);
            if (ImGui::Button(is_fullscreen ? ICON_FA_COMPRESS " Janela" : ICON_FA_EXPAND " Tela Cheia")) {
                is_fullscreen = !is_fullscreen;
            }

            ImGui::SameLine(0, 5);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.38f, 0.08f, 0.12f, 0.9f));
            if (ImGui::Button(ICON_FA_XMARK " SAIR")) {
                show_exit_confirm_modal = true;
            }
            ImGui::PopStyleColor();
        }

        // Waveforms Duplas Horizontais Empilhadas no Topo (Estilo Rekordbox 3-Band com Beatgrid e Modulação FX)
        void renderStackedRekordboxWaveforms(KuroAudio::DJEngine& engine, float w, float h) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            
            float lane_gap = 1.0f; // Apenas 1px de fresta divisória, estilo Rekordbox
            float lane_h = (h - lane_gap) * 0.5f;
            float center_x = pos.x + w * 0.5f;

            int active_l_id = ddj_hardware.getActiveLeftDeck();
            int active_r_id = ddj_hardware.getActiveRightDeck();
            KuroAudio::DJDeck* lane_decks[2] = { &engine.decks[active_l_id], &engine.decks[active_r_id] };
            int deck_ids[2] = { active_l_id, active_r_id };
            ImVec4 lane_themes[2] = {
                (active_l_id == 0 ? col_deck_1 : col_deck_3),
                (active_r_id == 1 ? col_deck_2 : col_deck_4)
            };

            for (int lane = 0; lane < 2; lane++) {
                KuroAudio::DJDeck& deck = *lane_decks[lane];
                int d_num = deck_ids[lane] + 1;
                ImVec4 theme_col = lane_themes[lane];
                ImVec2 l_pos = ImVec2(pos.x, pos.y + lane * (lane_h + lane_gap));

                // 1. Fundo Obsidian Rekordbox
                dl->AddRectFilled(l_pos, ImVec2(l_pos.x + w, l_pos.y + lane_h), IM_COL32(4, 6, 9, 255), 2.0f);

                // 2. Linha divisória de 1px entre as duas pistas
                if (lane == 0) {
                    dl->AddLine(ImVec2(l_pos.x, l_pos.y + lane_h), ImVec2(l_pos.x + w, l_pos.y + lane_h), IM_COL32(32, 44, 62, 255), 1.0f);
                }

                // Linha de centro neutra
                float mid_y = l_pos.y + lane_h * 0.5f;
                dl->AddLine(ImVec2(l_pos.x, mid_y), ImVec2(l_pos.x + w, mid_y), IM_COL32(18, 24, 36, 140), 1.0f);

                int window_frames = (int)(deck.sample_rate * 28.0); // 28.0 segundos de janela panorâmica CDJ
                int start_f = (int)deck.current_frame - window_frames / 2;

                // 3. Renderização da Waveform 3-Band Rekordbox
                if (!deck.buffer_l.empty() && deck.sample_rate > 1000.0) {
                    size_t total_f = deck.buffer_l.size();

                    // Beat Grid Sincronizado (Linha Vermelha na cabeça de compasso 4/4 com triângulos superior/inferior)
                    if (deck.bpm > 20.0) {
                        double beat_len = (60.0 / deck.bpm) * deck.sample_rate;
                        if (beat_len > 10.0) {
                            int b_start = (int)std::floor((double)start_f / beat_len);
                            int b_end   = (int)std::ceil((double)(start_f + window_frames) / beat_len);
                            for (int b = b_start; b <= b_end; b++) {
                                double bf = (double)b * beat_len;
                                float bx = l_pos.x + (float)((bf - (double)start_f) / (double)window_frames) * w;
                                if (bx >= l_pos.x && bx <= l_pos.x + w) {
                                    bool is_bar = (b % 4 == 0);
                                    if (is_bar) {
                                        // Linha Vermelha de Compasso com ponteiros triangulares
                                        dl->AddLine(ImVec2(bx, l_pos.y), ImVec2(bx, l_pos.y + lane_h), IM_COL32(255, 42, 55, 230), 1.5f);
                                        dl->AddTriangleFilled(ImVec2(bx - 3.5f, l_pos.y), ImVec2(bx + 3.5f, l_pos.y), ImVec2(bx, l_pos.y + 4.5f), IM_COL32(255, 42, 55, 255));
                                        dl->AddTriangleFilled(ImVec2(bx - 3.5f, l_pos.y + lane_h), ImVec2(bx + 3.5f, l_pos.y + lane_h), ImVec2(bx, l_pos.y + lane_h - 4.5f), IM_COL32(255, 42, 55, 255));
                                    } else {
                                        dl->AddLine(ImVec2(bx, l_pos.y + 3.0f), ImVec2(bx, l_pos.y + lane_h - 3.0f), IM_COL32(200, 225, 255, 55), 1.0f);
                                    }
                                }
                            }
                        }
                    }

                    // Cores 3-Band Rekordbox Oficiais:
                    // Agudos = Azul Cobalto / Ciano
                    // Médios = Laranja Âmbar Vibrante
                    // Graves / Kick Punch = Branco Laser
                    ImU32 col_high = IM_COL32(0, 115, 255, 240);
                    ImU32 col_mid  = IM_COL32(255, 120, 0, 245);
                    ImU32 col_low  = IM_COL32(255, 255, 255, 255);

                    int step_px = 2;
                    int bucket_size = std::max(1, window_frames / ((int)w / step_px));

                    for (int x = 0; x < (int)w; x += step_px) {
                        int f_idx = start_f + (int)((float)x / w * (float)window_frames);
                        if (f_idx >= 0 && f_idx < (int)total_f) {
                            float peak = 0.0f;
                            float high_diff = 0.0f;
                            for (int k = 0; k < bucket_size && (f_idx + k) < (int)total_f; k += 4) {
                                float s = std::abs(deck.buffer_l[(size_t)(f_idx + k)]);
                                if (s > peak) peak = s;
                                if ((f_idx + k + 1) < (int)total_f) {
                                    high_diff += std::abs(deck.buffer_l[(size_t)(f_idx + k)] - deck.buffer_l[(size_t)(f_idx + k + 1)]);
                                }
                            }
                            peak = std::clamp(peak * 1.35f, 0.02f, 1.0f);

                            float total_amp = peak * (lane_h * 0.48f);
                            float mid_amp   = total_amp * 0.65f;
                            float low_amp   = total_amp * 0.35f;

                            float col_x = l_pos.x + x;

                            // 1. Agudos (Azul Cobalto)
                            dl->AddLine(ImVec2(col_x, mid_y - total_amp), ImVec2(col_x, mid_y + total_amp), col_high, (float)step_px);
                            // 2. Médios (Laranja Âmbar)
                            dl->AddLine(ImVec2(col_x, mid_y - mid_amp), ImVec2(col_x, mid_y + mid_amp), col_mid, (float)step_px);
                            // 3. Graves (Branco Laser)
                            dl->AddLine(ImVec2(col_x, mid_y - low_amp), ImVec2(col_x, mid_y + low_amp), col_low, (float)step_px);
                        }
                    }

                    // Hot Cues na Waveform com Bandeiras e Letras (Estilo Rekordbox)
                    const char* cue_letters[] = { "A", "B", "C", "D", "E", "F", "G", "H" };
                    for (int c = 0; c < 8; c++) {
                        if (deck.hot_cues[c].active) {
                            double cf = deck.hot_cues[c].time_sec * deck.sample_rate;
                            float cx = l_pos.x + (float)((cf - (double)start_f) / (double)window_frames) * w;
                            if (cx >= l_pos.x && cx <= l_pos.x + w) {
                                dl->AddLine(ImVec2(cx, l_pos.y), ImVec2(cx, l_pos.y + lane_h), deck.hot_cues[c].color_rgba, 1.5f);
                                dl->AddRectFilled(ImVec2(cx - 6, l_pos.y), ImVec2(cx + 6, l_pos.y + 12), deck.hot_cues[c].color_rgba, 2.0f);
                                dl->AddTriangleFilled(ImVec2(cx - 4, l_pos.y + 12), ImVec2(cx + 4, l_pos.y + 12), ImVec2(cx, l_pos.y + 15), deck.hot_cues[c].color_rgba);
                                dl->AddText(ImVec2(cx - 3, l_pos.y), IM_COL32(0, 0, 0, 255), cue_letters[c]);
                            }
                        }
                    }
                }

                // 4. Indicadores de Canto: Badge do Deck e Contador de Compassos
                char deck_badge[32];
                snprintf(deck_badge, sizeof(deck_badge), "DECK %d", d_num);
                dl->AddRectFilled(ImVec2(l_pos.x + 6, l_pos.y + 3), ImVec2(l_pos.x + 58, l_pos.y + 17), ImGui::GetColorU32(theme_col), 3.0f);
                dl->AddText(ImVec2(l_pos.x + 10, l_pos.y + 3), IM_COL32(0, 0, 0, 255), deck_badge);

                // Contador de Compassos estilo Rekordbox (ex: -1.4Bars ou 1.1Bars ao lado da agulha central)
                if (deck.bpm > 20.0 && deck.sample_rate > 1000.0) {
                    double beat_len = (60.0 / deck.bpm) * deck.sample_rate;
                    double bars = (deck.current_frame - deck.cue_frame) / (beat_len * 4.0);
                    char bars_text[32];
                    snprintf(bars_text, sizeof(bars_text), "%+.1fBars", bars);
                    dl->AddText(ImVec2(center_x - 70.0f, l_pos.y + 3.0f), IM_COL32(0, 210, 255, 230), bars_text);
                }

                // 5. Interação Completa de Mouse (Clique para Seek, Arraste para Scrubbing/Scratch, Roda para Nudge)
                char lane_btn_id[32];
                snprintf(lane_btn_id, sizeof(lane_btn_id), "##WaveLane_%d", deck.deck_id);
                ImGui::SetCursorScreenPos(l_pos);
                ImGui::InvisibleButton(lane_btn_id, ImVec2(w, lane_h));

                bool lane_active = ImGui::IsItemActive();
                bool lane_clicked = ImGui::IsItemClicked(0);
                bool lane_hovered = ImGui::IsItemHovered();

                if (lane_active) {
                    ImGuiIO& io = ImGui::GetIO();
                    deck.jog_touch = true;
                    deck.scratch_active_ticks = 6;
                    if (io.MouseDelta.x != 0.0f) {
                        double delta_frames = -((double)io.MouseDelta.x / (double)w) * (double)window_frames;
                        if (io.KeyShift) delta_frames *= 4.0; // Shift: Fast Needle Search
                        deck.current_frame += delta_frames;
                        if (deck.current_frame < 0.0) deck.current_frame = 0.0;
                        if (deck.current_frame > (double)deck.buffer_l.size()) deck.current_frame = (double)deck.buffer_l.size();
                        deck.target_scratch_velocity = (float)(-io.MouseDelta.x * 0.18f);
                        deck.jog_visual_angle -= (float)io.MouseDelta.x * 0.025f;
                    }
                } else if (lane_clicked) {
                    float click_x = ImGui::GetMousePos().x;
                    double offset_frames = ((double)(click_x - center_x) / (double)w) * (double)window_frames;
                    deck.current_frame += offset_frames;
                    if (deck.current_frame < 0.0) deck.current_frame = 0.0;
                    if (deck.current_frame > (double)deck.buffer_l.size()) deck.current_frame = (double)deck.buffer_l.size();
                } else if (lane_hovered && ImGui::IsMouseReleased(0)) {
                    deck.jog_touch = false;
                }

                if (lane_hovered && ImGui::GetIO().MouseWheel != 0.0f) {
                    float wheel = ImGui::GetIO().MouseWheel;
                    double seek_step = (double)wheel * (deck.sample_rate * (ImGui::GetIO().KeyShift ? 4.0 : 0.4));
                    deck.current_frame += seek_step;
                    if (deck.current_frame < 0.0) deck.current_frame = 0.0;
                    if (deck.current_frame > (double)deck.buffer_l.size()) deck.current_frame = (double)deck.buffer_l.size();
                    deck.jog_visual_angle += wheel * 0.08f;
                }

                if (lane_hovered) {
                    ImGui::SetTooltip("Deck %d: Arraste para navegar / scratch | Clique para ir ao ponto | Shift: Busca rápida", d_num);
                }
            }

            // 6. Agulha Playhead Central Branca Laser (Atravessando ambas as pistas)
            dl->AddLine(ImVec2(center_x, pos.y), ImVec2(center_x, pos.y + h), IM_COL32(255, 255, 255, 255), 2.0f);
            dl->AddTriangleFilled(ImVec2(center_x - 5, pos.y), ImVec2(center_x + 5, pos.y), ImVec2(center_x, pos.y + 6), IM_COL32(255, 255, 255, 255));
            dl->AddTriangleFilled(ImVec2(center_x - 5, pos.y + h), ImVec2(center_x + 5, pos.y + h), ImVec2(center_x, pos.y + h - 6), IM_COL32(255, 255, 255, 255));

            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h));
        }

        // Gaveta Modal de Exportação para Pen Drive USB (Pioneer Rekordbox XML + Formato Universal)
        void renderUSBExportDrawer(KuroAudio::DJEngine& engine) {
            float avail_w = ImGui::GetContentRegionAvail().x;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.08f, 0.14f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.75f, 0.90f, 0.9f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
            
            ImGui::BeginChild("##USBExportDrawer", ImVec2(avail_w, 130), true);
            
            ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.85f, 1.0f), ICON_FA_FLOPPY_DISK " EXPORTAÇÃO PROFISSIONAL PARA PEN DRIVE (REKORDBOX & UNIVERSAL)");
            ImGui::SameLine(avail_w - 90);
            if (ImGui::SmallButton(ICON_FA_XMARK " Fechar")) {
                show_export_modal = false;
            }
            ImGui::Separator();

            if (detected_drives.empty()) {
                detected_drives = KuroAudio::DJExportService::scanDrives();
                if (detected_drives.empty()) {
                    KuroAudio::USBDeviceDrive fallback;
                    fallback.path = "scratch/tracks/export/";
                    fallback.label = "scratch/tracks/export/ [Pasta Local de Exportação]";
                    fallback.is_removable = false;
                    detected_drives.push_back(fallback);
                }
            }

            ImGui::Columns(3, "ExportCols", false);
            ImGui::SetColumnWidth(0, avail_w * 0.35f);
            ImGui::SetColumnWidth(1, avail_w * 0.35f);
            ImGui::SetColumnWidth(2, avail_w * 0.30f);

            // Coluna 1: Dispositivo de Destino
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "1. Unidade de Destino (USB):");
            std::string cur_label = (selected_drive_idx >= 0 && selected_drive_idx < (int)detected_drives.size()) ? detected_drives[selected_drive_idx].label : "Selecione...";
            if (ImGui::BeginCombo("##TargetDrive", cur_label.c_str())) {
                for (int i = 0; i < (int)detected_drives.size(); i++) {
                    bool sel = (i == selected_drive_idx);
                    if (ImGui::Selectable(detected_drives[i].label.c_str(), sel)) {
                        selected_drive_idx = i;
                    }
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine(0, 4);
            if (ImGui::SmallButton(ICON_FA_ARROWS_ROTATE " Atualizar")) {
                detected_drives = KuroAudio::DJExportService::scanDrives();
            }

            // Coluna 2: Formatos de Exportação
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "2. Formatos Compatíveis:");
            ImGui::Checkbox("Pioneer Rekordbox XML (CDJ/XDJ)", &opt_export_rekordbox);
            ImGui::Checkbox("Universal DJ (.m3u8 + JSON Cues)", &opt_export_universal);

            // Coluna 3: Ação
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "3. Processar:");
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.45f, 0.95f));
            if (ImGui::Button(ICON_FA_DOWNLOAD " EXPORTAR AGORA", ImVec2(ImGui::GetContentRegionAvail().x - 10, 32))) {
                if (selected_drive_idx >= 0 && selected_drive_idx < (int)detected_drives.size()) {
                    std::string target = detected_drives[selected_drive_idx].path;
                    auto tracks = spotify_service.getTracks();
                    std::string status = "";
                    bool ok = true;
                    if (opt_export_rekordbox) {
                        ok &= KuroAudio::DJExportService::exportRekordboxPackage(target, tracks, engine.decks, status);
                    }
                    if (opt_export_universal) {
                        ok &= KuroAudio::DJExportService::exportUniversalPackage(target, tracks, engine.decks, status);
                    }
                    export_status_msg = ok ? ("✓ Sucesso: Exportado para " + target) : ("Erro: " + status);
                }
            }
            ImGui::PopStyleColor();

            if (!export_status_msg.empty()) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "%s", export_status_msg.c_str());
            }

            ImGui::Columns(1);
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }

        void renderPlaylistSpotifyBar(KuroAudio::DJEngine& engine) {
            float avail_w = ImGui::GetContentRegionAvail().x;
            
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.06f, 0.09f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.18f, 0.28f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
            ImGui::BeginChild("##SpotifyPlaylistDrawer", ImVec2(avail_w, 144), true);

            float left_w = 205.0f;
            float right_w = avail_w - left_w - 14.0f;

            // =========================================================================
            // PAINEL ESQUERDO: ÁRVORE DE PASTAS / CRATES POR VERTENTE
            // =========================================================================
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.03f, 0.04f, 0.07f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.10f, 0.15f, 0.24f, 0.70f));
            ImGui::BeginChild("##CratesLeftPane", ImVec2(left_w, 134), true);

            // Cabeçalho de Pastas
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_FOLDER_OPEN " PASTAS / CRATES");
            ImGui::SameLine(left_w - 45.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.35f, 0.45f, 0.85f));
            if (ImGui::SmallButton(ICON_FA_PLUS " NOVA")) {
                show_new_folder_dialog = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Criar Nova Pasta / Crate Personalizada");
            ImGui::PopStyleColor();

            ImGui::Separator();

            auto folders = KuroAudio::DJLibraryManager::getInstance().getFolders();
            std::string active_fid = KuroAudio::DJLibraryManager::getInstance().getActiveFolderId();

            for (const auto& f : folders) {
                bool is_active = (f.id == active_fid);
                const char* icon = ICON_FA_FOLDER;
                ImVec4 f_col = ImVec4(0.80f, 0.85f, 0.92f, 1.0f);

                if (f.id == "all")             { icon = ICON_FA_MUSIC;             f_col = ImVec4(0.35f, 0.90f, 1.00f, 1.0f); }
                else if (f.id == "prog_psy")   { icon = ICON_FA_FIRE;              f_col = ImVec4(0.00f, 0.96f, 0.83f, 1.0f); }
                else if (f.id == "psy_fullon") { icon = ICON_FA_BOLT;              f_col = ImVec4(0.85f, 0.40f, 1.00f, 1.0f); }
                else if (f.id == "tech_house") { icon = ICON_FA_COMPACT_DISC;     f_col = ImVec4(1.00f, 0.75f, 0.20f, 1.0f); }
                else if (f.id == "acid_techno"){ icon = ICON_FA_SLIDERS;          f_col = ImVec4(0.25f, 1.00f, 0.45f, 1.0f); }
                else if (f.id == "melodic_techno"){ icon = ICON_FA_HEADPHONES;    f_col = ImVec4(0.40f, 0.75f, 1.00f, 1.0f); }
                else if (f.id == "cloud")      { icon = ICON_FA_CLOUD_ARROW_DOWN;  f_col = ImVec4(0.95f, 0.65f, 1.00f, 1.0f); }

                char label[128];
                snprintf(label, sizeof(label), "%s %s [%d]##%s", icon, f.name.c_str(), f.track_count, f.id.c_str());

                if (is_active) {
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.00f, 0.40f, 0.45f, 0.75f));
                    ImGui::PushStyleColor(ImGuiCol_Text, f_col);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.75f, 0.82f, 1.0f));
                }

                if (ImGui::Selectable(label, is_active, ImGuiSelectableFlags_SpanAllColumns)) {
                    KuroAudio::DJLibraryManager::getInstance().setActiveFolderId(f.id);
                }

                // Menu de contexto para pastas customizadas
                if (!f.is_system && ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem(ICON_FA_TRASH_CAN " Excluir Pasta")) {
                        KuroAudio::DJLibraryManager::getInstance().deleteFolder(f.id);
                    }
                    ImGui::EndPopup();
                }

                if (is_active) {
                    ImGui::PopStyleColor(2);
                } else {
                    ImGui::PopStyleColor(1);
                }
            }

            ImGui::EndChild();
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 6.0f);

            // =========================================================================
            // PAINEL DIREITO: BARRA DE BUSCA CIRÚRGICA & TABELA DE FAIXAS FILTRADAS
            // =========================================================================
            ImGui::BeginChild("##TracksRightPane", ImVec2(right_w, 134), false);

            // Linha Superior de Busca & Filtros
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.50f, 0.75f, 1.0f, 1.0f), ICON_FA_MAGNIFYING_GLASS);
            ImGui::SameLine(0, 4);
            ImGui::PushItemWidth(170);
            if (ImGui::InputTextWithHint("##SpotifyQuery", "Música, artista ou link...", search_query_buf, sizeof(search_query_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                spotify_service.searchAsync(search_query_buf);
            }
            ImGui::PopItemWidth();

            ImGui::SameLine(0, 4);
            if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " BUSCAR")) {
                spotify_service.searchAsync(search_query_buf);
            }

            // Importar do Computador
            ImGui::SameLine(0, 5);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.35f, 0.45f, 0.85f));
            if (ImGui::Button(ICON_FA_FOLDER_PLUS " + IMPORTAR")) {
                std::string local_path = KuroUI::FileDialog::OpenFile("Audio Files (*.wav;*.mp3;*.flac;*.ogg)\0*.wav;*.mp3;*.flac;*.ogg\0All Files (*.*)\0*.*\0");
                if (!local_path.empty()) {
                    spotify_service.importLocalTrack(local_path);
                }
            }
            ImGui::PopStyleColor();

            // Chips Rápidos de Vertentes
            ImGui::SameLine(0, 6);
            auto vchip = [&](const char* label, const char* fid, ImVec4 col) {
                ImGui::PushStyleColor(ImGuiCol_Text, col);
                if (ImGui::SmallButton(label)) {
                    KuroAudio::DJLibraryManager::getInstance().setActiveFolderId(fid);
                }
                ImGui::PopStyleColor();
                ImGui::SameLine(0, 3);
            };
            vchip("Prog Psy", "prog_psy", ImVec4(0.00f, 0.96f, 0.83f, 1.0f));
            vchip("Full-On", "psy_fullon", ImVec4(0.85f, 0.40f, 1.00f, 1.0f));
            vchip("Tech House", "tech_house", ImVec4(1.00f, 0.75f, 0.20f, 1.0f));
            vchip("Acid Techno", "acid_techno", ImVec4(0.25f, 1.00f, 0.45f, 1.0f));
            vchip("Melodic", "melodic_techno", ImVec4(0.40f, 0.75f, 1.00f, 1.0f));

            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.60f, 0.85f, 1.0f, 0.9f), "%s", spotify_service.getStatus().c_str());

            ImGui::Separator();

            // Faixas da Pasta Selecionada
            auto track_list = KuroAudio::DJLibraryManager::getInstance().getTracksForFolder(active_fid);

            // Se for nuvem e estiver vazia, carrega as faixas do spotify_service
            if (track_list.empty() && active_fid == "cloud") {
                auto sp_tracks = spotify_service.getTracks();
                for (const auto& st : sp_tracks) {
                    KuroAudio::DJLibraryTrack lt;
                    lt.id = st.id;
                    lt.title = st.title;
                    lt.artist = st.artist;
                    lt.album = st.album;
                    lt.bpm = st.bpm;
                    lt.key = st.key;
                    lt.duration_sec = st.duration_sec;
                    lt.subgenre = st.subgenre;
                    lt.folder_id = "cloud";
                    lt.artwork_url = st.artwork_url;
                    lt.preview_url = st.preview_url;
                    lt.local_wav_path = st.local_wav_path;
                    lt.local_rgba_path = st.local_rgba_path;
                    lt.is_downloaded = st.is_ready;
                    lt.is_downloading = st.is_downloading;
                    track_list.push_back(lt);
                }
            }

            if (track_list.empty()) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.50f, 0.60f, 0.72f, 1.0f), 
                    ICON_FA_CIRCLE_INFO " Nenhuma música nesta pasta ainda. Busque na nuvem ou importe do computador para catalogar cirurgicamente.");
            } else {
                if (ImGui::BeginTable("##CrateTracksTable", 7, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH, ImVec2(0, 96))) {
                    ImGui::TableSetupColumn("Capa", ImGuiTableColumnFlags_WidthFixed, 32.0f);
                    ImGui::TableSetupColumn("Título e Artista", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Vertente", ImGuiTableColumnFlags_WidthFixed, 115.0f);
                    ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthFixed, 55.0f);
                    ImGui::TableSetupColumn("Tom", ImGuiTableColumnFlags_WidthFixed, 65.0f);
                    ImGui::TableSetupColumn("Duração", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                    ImGui::TableSetupColumn("Carregar nos Decks", ImGuiTableColumnFlags_WidthFixed, 195.0f);
                    ImGui::TableHeadersRow();

                    int left_id = ddj_hardware.getActiveLeftDeck();
                    int right_id = ddj_hardware.getActiveRightDeck();

                    for (int i = 0; i < (int)track_list.size(); i++) {
                        auto& t = track_list[i];
                        ImGui::TableNextRow();

                        // 1. Capa
                        ImGui::TableSetColumnIndex(0);
                        if (t.gl_tex_id != 0) {
                            ImGui::Image((ImTextureID)(intptr_t)t.gl_tex_id, ImVec2(24, 24));
                        } else {
                            ImGui::TextColored(ImVec4(0.40f, 0.55f, 0.70f, 1.0f), ICON_FA_COMPACT_DISC);
                        }

                        // 2. Título & Artista
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", t.title.c_str());
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.55f, 0.65f, 0.75f, 1.0f), "• %s", t.artist.c_str());

                        // 3. Vertente
                        ImGui::TableSetColumnIndex(2);
                        ImVec4 sg_col = ImVec4(0.80f, 0.85f, 0.90f, 1.0f);
                        if (t.subgenre.find("Progressive Psy") != std::string::npos) sg_col = ImVec4(0.00f, 0.96f, 0.83f, 1.0f);
                        else if (t.subgenre.find("Psytrance") != std::string::npos)  sg_col = ImVec4(0.85f, 0.40f, 1.00f, 1.0f);
                        else if (t.subgenre.find("Tech House") != std::string::npos) sg_col = ImVec4(1.00f, 0.75f, 0.20f, 1.0f);
                        else if (t.subgenre.find("Acid") != std::string::npos)       sg_col = ImVec4(0.25f, 1.00f, 0.45f, 1.0f);
                        else if (t.subgenre.find("Melodic") != std::string::npos)    sg_col = ImVec4(0.40f, 0.75f, 1.00f, 1.0f);

                        ImGui::TextColored(sg_col, "%s", t.subgenre.c_str());

                        // 4. BPM
                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextColored(col_deck_1, "%.1f", t.bpm);

                        // 5. Tom
                        ImGui::TableSetColumnIndex(4);
                        ImGui::TextColored(ImVec4(1.00f, 0.85f, 0.35f, 1.0f), "%s", t.key.c_str());

                        // 6. Duração
                        ImGui::TableSetColumnIndex(5);
                        int min = (int)t.duration_sec / 60;
                        int sec = (int)t.duration_sec % 60;
                        ImGui::TextColored(ImVec4(0.70f, 0.70f, 0.70f, 1.0f), "%02d:%02d", min, sec);

                        // 7. Ações Carregar
                        ImGui::TableSetColumnIndex(6);
                        ImGui::PushID(i);
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.45f, 0.55f, 0.85f));
                        char btn_l[32];
                        snprintf(btn_l, sizeof(btn_l), ICON_FA_ARROW_DOWN " DECK %d", left_id + 1);
                        if (ImGui::SmallButton(t.is_downloading ? "Baixando..." : btn_l)) {
                            if (t.is_downloaded) {
                                spotify_service.loadLibraryTrack(t, left_id, engine);
                            } else {
                                spotify_service.downloadLibraryTrackAsync(t, left_id, engine);
                            }
                        }
                        ImGui::PopStyleColor();

                        ImGui::SameLine(0, 4);

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.00f, 0.38f, 0.85f));
                        char btn_r[32];
                        snprintf(btn_r, sizeof(btn_r), ICON_FA_ARROW_DOWN " DECK %d", right_id + 1);
                        if (ImGui::SmallButton(t.is_downloading ? "Baixando..." : btn_r)) {
                            if (t.is_downloaded) {
                                spotify_service.loadLibraryTrack(t, right_id, engine);
                            } else {
                                spotify_service.downloadLibraryTrackAsync(t, right_id, engine);
                            }
                        }
                        ImGui::PopStyleColor();

                        // Menu de Contexto da Faixa (Mover / Alterar Vertente)
                        if (ImGui::BeginPopupContextItem()) {
                            ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), "%s", t.title.c_str());
                            ImGui::Separator();
                            if (ImGui::BeginMenu("Mover para Pasta")) {
                                for (const auto& fo : folders) {
                                    if (ImGui::MenuItem(fo.name.c_str())) {
                                        KuroAudio::DJLibraryManager::getInstance().moveTrackToFolder(t.id, fo.id);
                                    }
                                }
                                ImGui::EndMenu();
                            }
                            if (ImGui::BeginMenu("Alterar Vertente")) {
                                const char* genres[] = { "Progressive Psytrance", "Psytrance / Full-On", "Tech House", "Acid Techno", "Melodic Techno" };
                                for (const char* g : genres) {
                                    if (ImGui::MenuItem(g)) {
                                        KuroAudio::DJLibraryManager::getInstance().setTrackSubgenre(t.id, g);
                                    }
                                }
                                ImGui::EndMenu();
                            }
                            ImGui::EndPopup();
                        }

                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }
            }

            ImGui::EndChild();

            // Modal Criar Nova Pasta
            if (show_new_folder_dialog) {
                ImGui::OpenPopup("Nova Pasta DJ##Modal");
            }
            if (ImGui::BeginPopupModal("Nova Pasta DJ##Modal", &show_new_folder_dialog, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_FOLDER_PLUS " Criar Novo Crate / Pasta");
                ImGui::Text("Digite o nome da pasta para organizar suas faixas:");
                ImGui::SetNextItemWidth(260);
                ImGui::InputText("##NewCrateNameInput", new_folder_name_buf, sizeof(new_folder_name_buf));
                ImGui::Spacing();
                if (ImGui::Button("Criar Pasta", ImVec2(120, 0))) {
                    if (strlen(new_folder_name_buf) > 0) {
                        KuroAudio::DJLibraryManager::getInstance().createCustomFolder(new_folder_name_buf);
                        new_folder_name_buf[0] = '\0';
                        show_new_folder_dialog = false;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancelar", ImVec2(100, 0))) {
                    show_new_folder_dialog = false;
                }
                ImGui::EndPopup();
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }

        void renderDeckPanel(KuroAudio::DJDeck& deck, int deck_slot, ImVec4 theme_col, KuroAudio::DJEngine& engine) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            
            // Cartão do Deck
            dl->AddRectFilled(p_min, ImVec2(p_min.x + w, p_min.y + 28), ImGui::GetColorU32(col_card), 5.0f);
            dl->AddLine(p_min, ImVec2(p_min.x + w, p_min.y), ImGui::GetColorU32(theme_col), 2.0f);

            // Cabeçalho do Deck com Botões de Alternância 1/3 ou 2/4
            ImGui::SetCursorScreenPos(ImVec2(p_min.x + 6, p_min.y + 4));
            if (deck_slot == 0) {
                int left_idx = ddj_hardware.getActiveLeftDeck();
                if (ImGui::SmallButton(left_idx == 0 ? "[*] DECK 1" : "DECK 1")) ddj_hardware.setActiveLeftDeck(0);
                ImGui::SameLine(0, 3);
                if (ImGui::SmallButton(left_idx == 2 ? "[*] DECK 3" : "DECK 3")) ddj_hardware.setActiveLeftDeck(2);
            } else {
                int right_idx = ddj_hardware.getActiveRightDeck();
                if (ImGui::SmallButton(right_idx == 1 ? "[*] DECK 2" : "DECK 2")) ddj_hardware.setActiveRightDeck(1);
                ImGui::SameLine(0, 3);
                if (ImGui::SmallButton(right_idx == 3 ? "[*] DECK 4" : "DECK 4")) ddj_hardware.setActiveRightDeck(3);
            }

            ImGui::SameLine(0, 8);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", deck.track_title.c_str());
            ImGui::SameLine(0, 6);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "• %s", deck.track_artist.c_str());

            // Botão Local WAV
            ImGui::SetCursorScreenPos(ImVec2(p_min.x + w - 85, p_min.y + 4));
            char open_lbl[32];
            snprintf(open_lbl, sizeof(open_lbl), "Abrir WAV %d", deck.deck_id + 1);
            if (ImGui::SmallButton(open_lbl)) {
                std::string path = KuroUI::FileDialog::OpenFile("WAV Audio (*.wav)\0*.wav\0All Files (*.*)\0*.*\0");
                if (!path.empty()) {
                    deck.loadTrack(path);
                }
            }

            ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_min.y + 30));

            // --- MINI OVERVIEW WAVEFORM STRIP (NEEDLE SEARCH 3-BAND REKORDBOX) ---
            renderMiniOverviewStrip(deck, w, 26.0f, theme_col);
            ImGui::Spacing();

            // --- SEÇÃO PRINCIPAL DO PRATO ANALÓGICO COM ARTE CENTRAL ---
            ImVec2 jog_start = ImGui::GetCursorScreenPos();
            float jog_section_h = 175.0f;
            float jog_size = 165.0f;

            // Fundo da Seção do Deck
            dl->AddRectFilled(jog_start, ImVec2(jog_start.x + w, jog_start.y + jog_section_h), IM_COL32(10, 13, 18, 200), 5.0f);
            dl->AddRect(jog_start, ImVec2(jog_start.x + w, jog_start.y + jog_section_h), IM_COL32(25, 35, 50, 180), 5.0f, 0, 1.0f);

            // Vinil gigante interativo
            renderGiantVinylTurntable(deck, ImVec2(jog_start.x + 8, jog_start.y + 5), jog_size, theme_col);

            // CONTROLES DE TRANSPORTE
            float ctrl_x = jog_start.x + jog_size + 16.0f;
            ImGui::SetCursorScreenPos(ImVec2(ctrl_x, jog_start.y + 10));

            // PLAY / PAUSE
            bool playing = deck.is_playing;
            if (playing) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.85f, 0.45f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.20f, 0.28f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
            }
            if (ImGui::Button(playing ? ICON_FA_PAUSE " PAUSE" : ICON_FA_PLAY " PLAY", ImVec2(68, 28))) {
                deck.togglePlay();
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 6);

            // CUE
            bool cueing = deck.cue_active;
            if (cueing) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.65f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.16f, 0.12f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.2f, 1.0f));
            }
            if (ImGui::Button(ICON_FA_ROTATE_LEFT " CUE", ImVec2(56, 28))) {
                deck.pressCue();
            }
            if (ImGui::IsItemDeactivated() && deck.cue_active) {
                deck.releaseCue();
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 6);

            // SYNC
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.25f, 0.35f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.9f, 1.0f, 1.0f));
            if (ImGui::Button(ICON_FA_ARROWS_ROTATE " SYNC", ImVec2(56, 28))) {
                int master_deck = (deck_slot == 0) ? ddj_hardware.getActiveRightDeck() : ddj_hardware.getActiveLeftDeck();
                engine.syncBPM(master_deck, deck.deck_id);
            }
            ImGui::PopStyleColor(2);

            // BEAT LOOP
            ImGui::SetCursorScreenPos(ImVec2(ctrl_x, jog_start.y + 44));
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "BEAT LOOP:");
            ImGui::SameLine();
            auto loopBtn = [&](double beats, const char* label) {
                bool act = deck.loop_active && deck.loop_beats == beats;
                if (act) ImGui::PushStyleColor(ImGuiCol_Button, theme_col);
                if (ImGui::SmallButton(label)) deck.toggleLoop(beats);
                if (act) ImGui::PopStyleColor();
                ImGui::SameLine(0, 3);
            };
            loopBtn(1.0, " 1 "); loopBtn(2.0, " 2 "); loopBtn(4.0, " 4 "); loopBtn(8.0, " 8 ");
            ImGui::NewLine();

            // PITCH FADER 14-BIT VERTICAL
            float pitch_col_x = std::max(ctrl_x + 140.0f, jog_start.x + w - 40.0f);
            ImGui::SetCursorScreenPos(ImVec2(pitch_col_x - 10, jog_start.y + 8));
            ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "%+.1f%%", deck.pitch_percent);

            ImGui::SetCursorScreenPos(ImVec2(pitch_col_x, jog_start.y + 24));
            float pitch_norm = deck.pitch_percent / 8.0f;
            if (ImGui::VSliderFloat("##PitchSlider", ImVec2(20, 120), &pitch_norm, -1.0f, 1.0f, "")) {
                deck.pitch_percent = pitch_norm * 8.0f;
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                deck.pitch_percent = 0.0f;
            }

            ImGui::SetCursorScreenPos(ImVec2(p_min.x, jog_start.y + jog_section_h + 4));

            // --- 8 PERFORMANCE PADS MULTIFUNÇÃO ---
            renderPerformancePads(deck, w, theme_col, engine);
        }

        void renderMiniOverviewStrip(KuroAudio::DJDeck& deck, float w, float h, ImVec4 theme_col) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(6, 8, 12, 255), 3.0f);
            dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(25, 35, 50, 180), 3.0f, 0, 1.0f);

            if (!deck.overview_peaks.empty()) {
                // Cores 3-Band Rekordbox Oficiais
                ImU32 col_high = IM_COL32(0, 115, 255, 230);
                ImU32 col_mid  = IM_COL32(255, 120, 0, 240);
                ImU32 col_low  = IM_COL32(255, 255, 255, 255);
                size_t num_peaks = deck.overview_peaks.size();
                for (float x = pos.x; x < pos.x + w; x += 2.0f) {
                    int p_idx = (int)((x - pos.x) / w * (float)num_peaks);
                    if (p_idx >= 0 && p_idx < (int)num_peaks) {
                        float pk = std::clamp(deck.overview_peaks[p_idx] * 1.35f, 0.04f, 1.0f);
                        float total_val = pk * (h * 0.46f);
                        float mid_val   = total_val * 0.65f;
                        float low_val   = total_val * 0.35f;
                        float my = pos.y + h * 0.5f;

                        dl->AddLine(ImVec2(x, my - total_val), ImVec2(x, my + total_val), col_high, 1.5f);
                        dl->AddLine(ImVec2(x, my - mid_val), ImVec2(x, my + mid_val), col_mid, 1.5f);
                        dl->AddLine(ImVec2(x, my - low_val), ImVec2(x, my + low_val), col_low, 1.5f);
                    }
                }

                for (int c = 0; c < 8; c++) {
                    if (deck.hot_cues[c].active && deck.duration_sec > 0.0) {
                        float cue_x = pos.x + (float)(deck.hot_cues[c].time_sec / deck.duration_sec) * w;
                        dl->AddLine(ImVec2(cue_x, pos.y), ImVec2(cue_x, pos.y + h), deck.hot_cues[c].color_rgba, 2.0f);
                    }
                }

                if (deck.duration_sec > 0.0) {
                    float play_x = pos.x + (float)((deck.current_frame / deck.sample_rate) / deck.duration_sec) * w;
                    dl->AddLine(ImVec2(play_x, pos.y), ImVec2(play_x, pos.y + h), IM_COL32(255, 255, 255, 255), 2.5f);
                }
            }

            // Clique ou Arraste Contínuo para Needle Search (Navegação Rápida)
            char strip_btn_id[32];
            snprintf(strip_btn_id, sizeof(strip_btn_id), "##NeedleSearchStrip_%d", deck.deck_id);
            ImGui::SetCursorScreenPos(pos);
            ImGui::InvisibleButton(strip_btn_id, ImVec2(w, h));
            
            bool strip_active = ImGui::IsItemActive();
            bool strip_clicked = ImGui::IsItemClicked(0);
            bool strip_hovered = ImGui::IsItemHovered();

            if ((strip_active || strip_clicked) && deck.duration_sec > 0.0) {
                float mouse_x = ImGui::GetMousePos().x - pos.x;
                float progress = std::clamp(mouse_x / w, 0.0f, 1.0f);
                deck.current_frame = (double)progress * deck.duration_sec * deck.sample_rate;
            }

            if (strip_hovered && deck.duration_sec > 0.0) {
                float mouse_x = ImGui::GetMousePos().x - pos.x;
                float progress = std::clamp(mouse_x / w, 0.0f, 1.0f);
                double target_sec = (double)progress * deck.duration_sec;
                int t_min = (int)target_sec / 60;
                int t_sec = (int)target_sec % 60;
                int c_min = (int)(deck.current_frame / deck.sample_rate) / 60;
                int c_sec = (int)(deck.current_frame / deck.sample_rate) % 60;
                ImGui::SetTooltip("Needle Search: %02d:%02d / %02d:%02d", t_min, t_sec, (int)deck.duration_sec / 60, (int)deck.duration_sec % 60);
            }

            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h));
        }

        // Renderiza o Vinilzão Analógico Gigante com Arte Circular no Centro e Braço Fonocaptor
        void renderGiantVinylTurntable(KuroAudio::DJDeck& deck, ImVec2 top_left, float size, ImVec4 theme_col) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            float radius = size * 0.5f;
            ImVec2 center = ImVec2(top_left.x + radius, top_left.y + radius);

            // 1. Chassi circular e Anel Neon Externo (Glow)
            dl->AddCircleFilled(center, radius, IM_COL32(10, 12, 16, 255), 64);
            dl->AddCircle(center, radius, ImGui::GetColorU32(theme_col), 64, 3.5f);
            dl->AddCircle(center, radius * 0.98f, ImGui::GetColorU32(ImVec4(theme_col.x, theme_col.y, theme_col.z, 0.35f)), 64, 2.0f);

            // 2. Pontos Estroboscópicos da Borda
            int num_dots = 42;
            for (int d = 0; d < num_dots; d++) {
                float da = (float)d * (2.0f * (float)M_PI / num_dots);
                float dot_r = radius * 0.94f;
                ImVec2 dot_pos = ImVec2(center.x + std::cos(da) * dot_r, center.y + std::sin(da) * dot_r);
                dl->AddCircleFilled(dot_pos, 1.8f, IM_COL32(180, 205, 230, 160));
            }

            // 3. Ranhuras concêntricas de vinil (Micro-Grooves)
            dl->AddCircleFilled(center, radius * 0.90f, IM_COL32(12, 14, 18, 255), 64);
            for (float gr = 0.46f; gr < 0.88f; gr += 0.04f) {
                dl->AddCircle(center, radius * gr, IM_COL32(34, 38, 48, 130), 48, 1.0f);
            }

            // 4. Efeito de Reflexo Especular de Luz Radial
            float a1 = deck.jog_visual_angle;
            float a2 = a1 + (float)M_PI;
            ImVec2 s1 = ImVec2(center.x + std::cos(a1) * radius * 0.88f, center.y + std::sin(a1) * radius * 0.88f);
            ImVec2 s2 = ImVec2(center.x + std::cos(a2) * radius * 0.88f, center.y + std::sin(a2) * radius * 0.88f);
            dl->AddLine(center, s1, IM_COL32(255, 255, 255, 35), 2.5f);
            dl->AddLine(center, s2, IM_COL32(255, 255, 255, 35), 2.5f);

            // 5. RÓTULO CENTRAL REDONDO DO VINIL (FOTO/ARTE DO ÁLBUM GIRATÓRIA EM TEMPO REAL)
            float label_radius = radius * 0.42f;
            dl->AddCircleFilled(center, label_radius, IM_COL32(22, 26, 34, 255), 48);

            // Se tiver textura do Álbum/Faixa, renderiza o quad rotacionado em tempo real com o vinil
            if (deck.has_cover_texture && deck.gl_cover_texture != 0) {
                float r = label_radius;
                float angle = deck.jog_visual_angle;
                float c = std::cos(angle);
                float s = std::sin(angle);

                ImVec2 p0 = ImVec2(center.x + (-r * c - -r * s), center.y + (-r * s + -r * c));
                ImVec2 p1 = ImVec2(center.x + ( r * c - -r * s), center.y + ( r * s + -r * c));
                ImVec2 p2 = ImVec2(center.x + ( r * c -  r * s), center.y + ( r * s +  r * c));
                ImVec2 p3 = ImVec2(center.x + (-r * c -  r * s), center.y + (-r * s +  r * c));

                dl->AddImageQuad((ImTextureID)(intptr_t)deck.gl_cover_texture,
                                 p0, p1, p2, p3,
                                 ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1));

                // Máscara de contorno suave do vinil
                for (float m = label_radius + 0.5f; m <= label_radius + 16.0f; m += 1.5f) {
                    dl->AddCircle(center, m, IM_COL32(12, 14, 18, 255), 48, 2.5f);
                }
            } else {
                dl->AddCircleFilled(center, label_radius, ImGui::GetColorU32(ImVec4(theme_col.x * 0.25f, theme_col.y * 0.25f, theme_col.z * 0.25f, 1.0f)), 36);
                dl->AddCircle(center, label_radius * 0.8f, ImGui::GetColorU32(theme_col), 36, 1.5f);
                dl->AddCircle(center, label_radius * 0.6f, IM_COL32(255, 255, 255, 40), 36, 1.0f);
            }

            dl->AddCircle(center, label_radius, ImGui::GetColorU32(theme_col), 48, 2.5f);

            // Pino Central Metálico & CDJ On-Jog LCD Display (BPM, Pitch e Rotação Fiel a 33 1/3 RPM)
            float lcd_radius = radius * 0.28f;
            dl->AddCircleFilled(center, lcd_radius, IM_COL32(6, 8, 12, 250), 32);
            dl->AddCircle(center, lcd_radius, ImGui::GetColorU32(theme_col), 32, 1.5f);

            // Display Digital Central de BPM e Pitch
            if (deck.bpm > 10.0) {
                double eff_bpm = deck.bpm * (1.0 + deck.pitch_percent * 0.01);
                char bpm_str[32];
                snprintf(bpm_str, sizeof(bpm_str), "%.1f", eff_bpm);
                ImVec2 sz_bpm = ImGui::CalcTextSize(bpm_str);
                dl->AddText(ImVec2(center.x - sz_bpm.x * 0.5f, center.y - sz_bpm.y * 0.5f - 4.0f), IM_COL32(255, 255, 255, 240), bpm_str);

                char pitch_str[32];
                snprintf(pitch_str, sizeof(pitch_str), "%+.1f%%", deck.pitch_percent);
                ImVec2 sz_pitch = ImGui::CalcTextSize(pitch_str);
                dl->AddText(ImVec2(center.x - sz_pitch.x * 0.5f, center.y + 3.0f), ImGui::GetColorU32(theme_col), pitch_str);
            } else {
                ImVec2 sz_lbl = ImGui::CalcTextSize("CDJ");
                dl->AddText(ImVec2(center.x - sz_lbl.x * 0.5f, center.y - sz_lbl.y * 0.5f), ImGui::GetColorU32(theme_col), "CDJ");
            }

            // Marcador visual de rotação do prato (Calibrado exatamente a 33 1/3 RPM)
            ImVec2 mark_pos = ImVec2(center.x + std::cos(deck.jog_visual_angle) * (radius * 0.72f), center.y + std::sin(deck.jog_visual_angle) * (radius * 0.72f));
            dl->AddCircleFilled(mark_pos, 5.0f, IM_COL32(255, 255, 255, 240), 16);
            dl->AddCircle(mark_pos, 5.0f, ImGui::GetColorU32(theme_col), 16, 1.5f);

            // 6. Braço Fonocaptor (Tonearm)
            ImVec2 arm_pivot = ImVec2(top_left.x + size - 15.0f, top_left.y + 20.0f);
            dl->AddCircleFilled(arm_pivot, 10.0f, IM_COL32(75, 85, 100, 255), 24);
            dl->AddCircle(arm_pivot, 10.0f, IM_COL32(150, 165, 185, 255), 24, 1.5f);

            float song_progress = (deck.duration_sec > 0.0) ? (float)((deck.current_frame / deck.sample_rate) / deck.duration_sec) : 0.0f;
            song_progress = std::clamp(song_progress, 0.0f, 1.0f);
            float stylus_dist = radius * 0.85f - song_progress * (radius * 0.40f);
            ImVec2 stylus_pos = ImVec2(center.x + stylus_dist * 0.7f, center.y + stylus_dist * 0.7f);

            dl->AddLine(arm_pivot, stylus_pos, IM_COL32(180, 195, 210, 255), 3.0f);
            dl->AddRectFilled(ImVec2(stylus_pos.x - 4, stylus_pos.y - 6), ImVec2(stylus_pos.x + 8, stylus_pos.y + 6), IM_COL32(230, 40, 60, 255), 2.0f);
            dl->AddCircleFilled(stylus_pos, 2.0f, IM_COL32(255, 255, 255, 255));

            // 7. Interação de Mouse no Vinil (Scratch e Nudge Bidirecional Físico)
            char v_id[32];
            snprintf(v_id, sizeof(v_id), "##VinylTurntable_%d", deck.deck_id);
            ImGui::SetCursorScreenPos(top_left);
            ImGui::InvisibleButton(v_id, ImVec2(size, size));

            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();

            if (active) {
                ImVec2 mouse = ImGui::GetMousePos();
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                float dist = std::hypot(mouse.x - center.x, mouse.y - center.y);

                if (dist < radius * 0.90f) { // Platter Scratch & Seek Bidirecional
                    deck.jog_touch = true;
                    float rot_delta = (delta.x * (mouse.y - center.y) - delta.y * (mouse.x - center.x)) * 0.003f;
                    deck.target_scratch_velocity = rot_delta * 8.0f;
                    deck.scratch_active_ticks = 8;
                    deck.jog_visual_angle += rot_delta;

                    // Arrastar para frente ou para trás retrocede ou avança a música diretamente no prato
                    double frames_delta = (double)rot_delta * (deck.sample_rate * 1.2);
                    if (ImGui::GetIO().KeyShift) {
                        frames_delta *= 10.0; // SHIFT + Rotação: Fast Search super veloz estilo Rekordbox/CDJ
                    }
                    deck.current_frame += frames_delta;
                    if (deck.current_frame < 0.0) deck.current_frame = 0.0;
                    if (deck.current_frame > (double)deck.buffer_l.size()) deck.current_frame = (double)deck.buffer_l.size();
                } else if (dist < radius * 1.18f) { // Rim Pitch Bend (Nudge)
                    float rot_delta = (delta.x * (mouse.y - center.y) - delta.y * (mouse.x - center.x)) * 0.0015f;
                    deck.jog_pitch_bend += rot_delta;
                    deck.jog_visual_angle += rot_delta;
                }
            } else if (hovered && ImGui::IsMouseReleased(0)) {
                deck.jog_touch = false;
            } else if (!active && !hovered && deck.jog_touch) {
                deck.jog_touch = false;
            }

            // Scroll Wheel do mouse no vinil: Nudge ou Fast Needle Seek com Shift
            if (hovered && ImGui::GetIO().MouseWheel != 0.0f) {
                float wheel = ImGui::GetIO().MouseWheel;
                float mult = ImGui::GetIO().KeyShift ? 10.0f : 1.0f;
                deck.current_frame += (double)wheel * (deck.sample_rate * 0.35) * mult;
                if (deck.current_frame < 0.0) deck.current_frame = 0.0;
                if (deck.current_frame > (double)deck.buffer_l.size()) deck.current_frame = (double)deck.buffer_l.size();
                deck.jog_visual_angle += wheel * 0.15f;
            }
        }

        void renderPerformancePads(KuroAudio::DJDeck& deck, float w, ImVec4 theme_col, KuroAudio::DJEngine& engine) {
            const char* modes[] = { "HOT CUE", "SAMPLER", "BEAT LOOP", "PAD FX" };
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "PAD MODE:");
            ImGui::SameLine();
            for (int m = 0; m < 4; m++) {
                bool is_cur = (deck.pad_mode == m);
                if (is_cur) ImGui::PushStyleColor(ImGuiCol_Button, theme_col);
                else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.14f, 0.20f, 0.8f));

                if (ImGui::Button(modes[m], ImVec2(58, 17))) {
                    deck.pad_mode = m;
                }
                ImGui::PopStyleColor();
                ImGui::SameLine(0, 4);
            }
            ImGui::NewLine();

            float pad_w = (w - 24.0f) * 0.25f;
            float pad_h = 19.0f;

            for (int i = 0; i < 8; i++) {
                ImGui::PushID(i);
                uint32_t col = deck.hot_cues[i].color_rgba;
                bool is_active = (deck.pad_mode == 0) ? deck.hot_cues[i].active : (deck.pad_mode == 1 ? true : false);

                if (is_active) {
                    ImVec4 pad_c = ImColor(col);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(pad_c.x * 0.6f, pad_c.y * 0.6f, pad_c.z * 0.6f, 0.9f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                }

                char lbl[32];
                if (deck.pad_mode == 0) {
                    snprintf(lbl, sizeof(lbl), is_active ? "CUE %d" : "[ %d ]", i + 1);
                } else if (deck.pad_mode == 1) {
                    snprintf(lbl, sizeof(lbl), "%s", engine.samplers[i].name.c_str());
                } else if (deck.pad_mode == 2) {
                    snprintf(lbl, sizeof(lbl), "1/%d B", (int)std::pow(2, 3 - i));
                } else {
                    snprintf(lbl, sizeof(lbl), "FX %d", i + 1);
                }

                if (ImGui::Button(lbl, ImVec2(pad_w, pad_h))) {
                    if (deck.pad_mode == 0) deck.jumpToHotCue(i);
                    else if (deck.pad_mode == 1) engine.triggerSampler(i);
                    else if (deck.pad_mode == 2) deck.toggleLoop(std::pow(2.0, (double)(i - 2)));
                    else engine.beat_fx.enabled = !engine.beat_fx.enabled;
                }

                if (deck.pad_mode == 0 && ImGui::IsItemClicked(1)) {
                    deck.deleteHotCue(i);
                }

                ImGui::PopStyleColor(2);
                ImGui::PopID();

                if ((i + 1) % 4 != 0) ImGui::SameLine(0, 4);
            }
        }

        void renderCentralMixer(KuroAudio::DJEngine& engine) {
            float w = ImGui::GetContentRegionAvail().x;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 m_pos = ImGui::GetCursorScreenPos();
            float m_h = ImGui::GetContentRegionAvail().y;

            dl->AddRectFilled(m_pos, ImVec2(m_pos.x + w, m_pos.y + m_h), ImGui::GetColorU32(col_card), 6.0f);
            dl->AddRect(m_pos, ImVec2(m_pos.x + w, m_pos.y + m_h), ImGui::GetColorU32(col_border), 6.0f);

            int left_id = ddj_hardware.getActiveLeftDeck();
            int right_id = ddj_hardware.getActiveRightDeck();
            KuroAudio::DJDeck& deck_l = engine.decks[left_id];
            KuroAudio::DJDeck& deck_r = engine.decks[right_id];
            ImVec4 theme_l = (left_id == 0 ? col_deck_1 : col_deck_3);
            ImVec4 theme_r = (right_id == 1 ? col_deck_2 : col_deck_4);

            ImGui::SetCursorScreenPos(ImVec2(m_pos.x + 8, m_pos.y + 5));
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ICON_FA_SLIDERS " MIXER & DJM-V10 DSP");
            ImGui::Separator();

            // --- 1. SUÍTE DE EFEITOS PIONEER BEAT FX ---
            const char* fx_names[] = { "ECHO", "REVERB", "FLANGER", "ROLL", "SPIRAL" };
            const char* ch_assigns[] = { "LEFT DECK", "RIGHT DECK", "MASTER" };
            
            float half_combo = (w - 24.0f) * 0.5f;

            // Seletor Dropdown Tipo de Efeito (ECHO, REVERB, FLANGER, ROLL, SPIRAL)
            ImGui::SetNextItemWidth(half_combo);
            const char* cur_fx = (engine.beat_fx.type >= 0 && engine.beat_fx.type < 5) ? fx_names[engine.beat_fx.type] : fx_names[0];
            if (ImGui::BeginCombo("##FXType", cur_fx, ImGuiComboFlags_HeightSmall)) {
                for (int n = 0; n < 5; n++) {
                    bool is_selected = (engine.beat_fx.type == n);
                    if (ImGui::Selectable(fx_names[n], is_selected)) {
                        engine.beat_fx.type = n;
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine(0, 6);

            // Seletor Dropdown Roteamento de Canal (LEFT DECK, RIGHT DECK, MASTER)
            ImGui::SetNextItemWidth(half_combo);
            const char* cur_assign = (engine.beat_fx.channel_assign >= 0 && engine.beat_fx.channel_assign < 3) ? ch_assigns[engine.beat_fx.channel_assign] : ch_assigns[2];
            if (ImGui::BeginCombo("##FXAssign", cur_assign, ImGuiComboFlags_HeightSmall)) {
                for (int n = 0; n < 3; n++) {
                    bool is_selected = (engine.beat_fx.channel_assign == n);
                    if (ImGui::Selectable(ch_assigns[n], is_selected)) {
                        engine.beat_fx.channel_assign = n;
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            // Frações de batida do Beat FX
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "BEAT:");
            ImGui::SameLine();
            auto beatBtn = [&](float frac, const char* txt) {
                bool act = (engine.beat_fx.beat_fraction == frac);
                if (act) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
                if (ImGui::Button(txt, ImVec2(32, 18))) engine.beat_fx.beat_fraction = frac;
                if (act) ImGui::PopStyleColor();
                ImGui::SameLine(0, 3);
            };
            beatBtn(0.25f, "1/4"); beatBtn(0.50f, "1/2"); beatBtn(0.75f, "3/4"); beatBtn(1.00f, " 1 "); beatBtn(2.00f, " 2 ");
            ImGui::NewLine();

            // DEPTH Slider e Botão ON/OFF
            float half_btn = (w - 24.0f) * 0.5f;
            ImGui::SetNextItemWidth(half_btn);
            if (ImGui::SliderFloat("##Depth", &engine.beat_fx.dry_wet, 0.0f, 1.0f, "DEPTH %.0f%%")) {
                if (engine.beat_fx.dry_wet > 0.01f) engine.beat_fx.enabled = true;
            }
            ImGui::SameLine(0, 6);

            bool fx_on = engine.beat_fx.enabled;
            if (fx_on) {
                float pulse = (std::sin(ImGui::GetTime() * 12.0f) + 1.0f) * 0.5f;
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f + pulse * 0.4f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.18f, 0.25f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
            }
            if (ImGui::Button(fx_on ? ICON_FA_BOLT " FX ATIVO" : ICON_FA_BOLT " FX DESAT.", ImVec2(half_btn, 19))) {
                engine.beat_fx.enabled = !engine.beat_fx.enabled;
            }
            if (fx_on) {
                ImVec2 fx_btn_min = ImGui::GetItemRectMin();
                ImVec2 fx_btn_max = ImGui::GetItemRectMax();
                float pulse = (std::sin(ImGui::GetTime() * 10.0f) + 1.0f) * 0.5f;
                dl->AddRect(ImVec2(fx_btn_min.x - 2.0f, fx_btn_min.y - 2.0f), 
                            ImVec2(fx_btn_max.x + 2.0f, fx_btn_max.y + 2.0f), 
                            IM_COL32(0, 245, 255, (int)(120 + pulse * 135)), 4.0f, 0, 2.0f);
                dl->AddRect(ImVec2(fx_btn_min.x - 4.0f, fx_btn_min.y - 4.0f), 
                            ImVec2(fx_btn_max.x + 4.0f, fx_btn_max.y + 4.0f), 
                            IM_COL32(0, 200, 255, (int)(50 + pulse * 70)), 6.0f, 0, 1.5f);
                dl->AddCircleFilled(ImVec2(fx_btn_min.x + 7.0f, (fx_btn_min.y + fx_btn_max.y) * 0.5f), 3.0f, IM_COL32(0, 255, 180, 255));
            }
            ImGui::PopStyleColor(2);

            ImGui::Separator();

            // --- 2. SOUND COLOR FX (12 MODOS COMPLETOS DA DJM-V10) ---
            const char* sc_modes[12] = { 
                "FILTER", "NOISE", "CRUSH", "SPACE", 
                "DUB ECHO", "PITCH", "PING PONG", "SPIRAL", 
                "REVERB", "SHIMMER", "SLIP ROLL", "VINYL BRAKE" 
            };
            ImGui::TextColored(ImVec4(0.85f, 0.90f, 1.0f, 1.0f), ICON_FA_WAND_MAGIC_SPARKLES " COLOR FX (DJM-V10):");
            
            float cfx_btn_w = (w - 28.0f) / 4.0f;
            for (int m = 0; m < 12; m++) {
                bool act = (deck_l.sound_color_fx_mode == m);
                if (act) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.75f, 0.9f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.16f, 0.22f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 0.95f, 1.0f));
                }
                if (ImGui::Button(sc_modes[m], ImVec2(cfx_btn_w, 16))) {
                    deck_l.sound_color_fx_mode = m;
                    deck_r.sound_color_fx_mode = m;
                }
                ImGui::PopStyleColor(2);
                if ((m + 1) % 4 != 0) ImGui::SameLine(0, 3);
            }

            // --- 3. KNOBS ROTATIVOS DE EQUALIZAÇÃO E CFX ---
            ImGui::Columns(2, "MixerChans", false);
            float col_half_w = (w - 16.0f) * 0.5f;
            ImGui::SetColumnWidth(0, col_half_w);
            ImGui::SetColumnWidth(1, col_half_w);
            
            // Coluna Deck Esquerdo
            float knob_indent = std::max(0.0f, (col_half_w - 38.0f) * 0.5f);
            
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + knob_indent - 4.0f);
            char d_lbl_l[32];
            snprintf(d_lbl_l, sizeof(d_lbl_l), "DECK %d", left_id + 1);
            ImGui::TextColored(theme_l, "%s", d_lbl_l);
            
            auto draw_knob_a = [&](const char* lbl, float* val, float vmin, float vmax, ImVec4 col, const char* fmt, float def) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + knob_indent);
                DrawRotaryKnob(lbl, val, vmin, vmax, 11.0f, col, fmt, def);
            };
            draw_knob_a("HI##A", &deck_l.eq_high, 0.0f, 2.0f, theme_l, "%.1fx", 1.0f);
            draw_knob_a("MID##A", &deck_l.eq_mid, 0.0f, 2.0f, theme_l, "%.1fx", 1.0f);
            draw_knob_a("LOW##A", &deck_l.eq_low, 0.0f, 2.0f, theme_l, "%.1fx", 1.0f);
            std::string sc_lbl_a = std::string(sc_modes[std::clamp(deck_l.sound_color_fx_mode, 0, 11)]) + "##A";
            draw_knob_a(sc_lbl_a.c_str(), &deck_l.filter_bipolar, 0.0f, 1.0f, ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "%.2f", 0.5f);

            ImGui::NextColumn();

            // Coluna Deck Direito
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + knob_indent - 4.0f);
            char d_lbl_r[32];
            snprintf(d_lbl_r, sizeof(d_lbl_r), "DECK %d", right_id + 1);
            ImGui::TextColored(theme_r, "%s", d_lbl_r);
            
            auto draw_knob_b = [&](const char* lbl, float* val, float vmin, float vmax, ImVec4 col, const char* fmt, float def) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + knob_indent);
                DrawRotaryKnob(lbl, val, vmin, vmax, 11.0f, col, fmt, def);
            };
            draw_knob_b("HI##B", &deck_r.eq_high, 0.0f, 2.0f, theme_r, "%.1fx", 1.0f);
            draw_knob_b("MID##B", &deck_r.eq_mid, 0.0f, 2.0f, theme_r, "%.1fx", 1.0f);
            draw_knob_b("LOW##B", &deck_r.eq_low, 0.0f, 2.0f, theme_r, "%.1fx", 1.0f);
            std::string sc_lbl_b = std::string(sc_modes[std::clamp(deck_r.sound_color_fx_mode, 0, 11)]) + "##B";
            draw_knob_b(sc_lbl_b.c_str(), &deck_r.filter_bipolar, 0.0f, 1.0f, ImVec4(1.0f, 0.3f, 0.7f, 1.0f), "%.2f", 0.5f);

            ImGui::Columns(1);

            // --- 4. FADERS DE VOLUME E MEDIDORES VU ESTÉREO DE 12 SEGMENTOS ---
            ImGui::Columns(4, "FadersVu", false);
            float col_fader_w = (w - 20) * 0.28f;
            float col_vu_w = (w - 20) * 0.22f;
            ImGui::SetColumnWidth(0, col_fader_w);
            ImGui::SetColumnWidth(1, col_vu_w);
            ImGui::SetColumnWidth(2, col_fader_w);
            ImGui::SetColumnWidth(3, col_vu_w);

            // Fader Vol Left
            ImGui::VSliderFloat("##VolA", ImVec2(24, 52), &deck_l.volume, 0.0f, 1.0f, "");
            ImGui::NextColumn();
            
            // VU Left (12 Segmentos)
            ImVec2 vu_a_pos = ImGui::GetCursorScreenPos();
            DrawSegmentedVUMeter(dl, vu_a_pos, ImVec2(12, 52), deck_l.vu_meter);
            ImGui::NextColumn();

            // Fader Vol Right
            ImGui::VSliderFloat("##VolB", ImVec2(24, 52), &deck_r.volume, 0.0f, 1.0f, "");
            ImGui::NextColumn();

            // VU Right (12 Segmentos)
            ImVec2 vu_b_pos = ImGui::GetCursorScreenPos();
            DrawSegmentedVUMeter(dl, vu_b_pos, ImVec2(12, 52), deck_r.vu_meter);

            ImGui::Columns(1);
            ImGui::Spacing();

            // --- 5. CROSSFADER CENTRAL ---
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "CROSSFADER:");
            float cf = engine.getCrossfader();
            ImGui::PushItemWidth(w - 16);
            if (ImGui::SliderFloat("##Crossfader", &cf, 0.0f, 1.0f, "")) {
                engine.setCrossfader(cf);
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                engine.setCrossfader(0.5f);
            }
            ImGui::PopItemWidth();
        }

        // =========================================================================
        // MODAL DEDICADO DE BUSCA ONLINE DE MÚSICAS COM CAPAS E IMPORTAÇÃO DIRETA
        // =========================================================================
        void renderSearchModal(KuroAudio::DJEngine& engine) {
            if (!show_search_modal) return;

            ImGuiViewport* vp = ImGui::GetMainViewport();
            ImVec2 modal_sz = ImVec2(std::min(vp->Size.x * 0.88f, 960.0f), std::min(vp->Size.y * 0.82f, 620.0f));
            ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + (vp->Size.x - modal_sz.x) * 0.5f, vp->Pos.y + (vp->Size.y - modal_sz.y) * 0.5f), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(modal_sz, ImGuiCond_Appearing);

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.09f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.80f, 0.90f, 0.90f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);

            if (ImGui::Begin("🛸 BUSCA ONLINE DE MÚSICAS & CAPAS (SPOTIFY / CLOUD)##SearchModal", &show_search_modal, ImGuiWindowFlags_NoCollapse)) {
                float avail_w = ImGui::GetContentRegionAvail().x;

                // Barra de busca
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_MAGNIFYING_GLASS " BUSCA:");
                ImGui::SameLine();
                ImGui::PushItemWidth(avail_w - 260.0f);
                if (ImGui::InputTextWithHint("##SearchModalInput", "Digite artista, track, vertente ou link...", online_search_query_buf, sizeof(online_search_query_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    spotify_service.searchAsync(online_search_query_buf);
                }
                ImGui::PopItemWidth();
                ImGui::SameLine(0, 6);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.65f, 0.95f));
                if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " BUSCAR", ImVec2(90, 0))) {
                    spotify_service.searchAsync(online_search_query_buf);
                }
                ImGui::PopStyleColor();

                ImGui::SameLine(0, 6);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.12f, 0.16f, 0.90f));
                if (ImGui::Button(ICON_FA_XMARK " FECHAR", ImVec2(80, 0))) {
                    show_search_modal = false;
                }
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.55f, 0.75f, 0.95f, 1.0f), "%s", spotify_service.getStatus().c_str());
                ImGui::Separator();

                auto results = spotify_service.getSearchResults();
                if (results.empty()) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.50f, 0.60f, 0.75f, 1.0f),
                        ICON_FA_CIRCLE_INFO " Digite o nome de um artista (ex: 'Klipsun', 'Aura Vortex', 'Astrix') ou cole um link e clique em BUSCAR.");
                } else {
                    if (ImGui::BeginTable("##SearchResultsTable", 6, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH, ImVec2(0, ImGui::GetContentRegionAvail().y - 10.0f))) {
                        ImGui::TableSetupColumn("Capa", ImGuiTableColumnFlags_WidthFixed, 44.0f);
                        ImGui::TableSetupColumn("Título & Artista", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Vertente", ImGuiTableColumnFlags_WidthFixed, 140.0f);
                        ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                        ImGui::TableSetupColumn("Tom", ImGuiTableColumnFlags_WidthFixed, 70.0f);
                        ImGui::TableSetupColumn("Ações", ImGuiTableColumnFlags_WidthFixed, 240.0f);
                        ImGui::TableHeadersRow();

                        int left_id = ddj_hardware.getActiveLeftDeck();
                        int right_id = ddj_hardware.getActiveRightDeck();

                        for (int i = 0; i < (int)results.size(); i++) {
                            const auto& r = results[i];
                            ImGui::TableNextRow();

                            // 1. Capa
                            ImGui::TableSetColumnIndex(0);
                            if (r.gl_tex_id != 0) {
                                ImGui::Image((ImTextureID)(intptr_t)r.gl_tex_id, ImVec2(36, 36));
                            } else {
                                ImGui::TextColored(ImVec4(0.40f, 0.55f, 0.70f, 1.0f), ICON_FA_COMPACT_DISC);
                            }

                            // 2. Título & Artista
                            ImGui::TableSetColumnIndex(1);
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", r.title.c_str());
                            ImGui::TextColored(ImVec4(0.60f, 0.70f, 0.80f, 1.0f), "%s", r.artist.c_str());

                            // 3. Vertente
                            ImGui::TableSetColumnIndex(2);
                            ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), "%s", r.subgenre.c_str());

                            // 4. BPM
                            ImGui::TableSetColumnIndex(3);
                            ImGui::TextColored(col_deck_1, "%.1f", r.bpm);

                            // 5. Tom
                            ImGui::TableSetColumnIndex(4);
                            ImGui::TextColored(ImVec4(1.00f, 0.85f, 0.35f, 1.0f), "%s", r.key.c_str());

                            // 6. Ações
                            ImGui::TableSetColumnIndex(5);
                            ImGui::PushID(i + 1000);

                            // Botão Importar para Crate
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.40f, 0.35f, 0.90f));
                            if (ImGui::SmallButton(ICON_FA_PLUS " IMPORTAR")) {
                                spotify_service.importSearchResultTrack(r);
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Adicionar faixa à biblioteca / crate de sua vertente");
                            ImGui::PopStyleColor();

                            ImGui::SameLine(0, 4);

                            // Carregar direto no Deck Left
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.40f, 0.50f, 0.85f));
                            char btn_dl[32];
                            snprintf(btn_dl, sizeof(btn_dl), "DECK %d", left_id + 1);
                            if (ImGui::SmallButton(btn_dl)) {
                                spotify_service.importSearchResultTrack(r);
                                auto t_list = KuroAudio::DJLibraryManager::getInstance().getTracksForFolder("all");
                                for (auto& tr : t_list) {
                                    if (tr.id == r.id) {
                                        if (tr.is_downloaded) spotify_service.loadLibraryTrack(tr, left_id, engine);
                                        else spotify_service.downloadLibraryTrackAsync(tr, left_id, engine);
                                        break;
                                    }
                                }
                            }
                            ImGui::PopStyleColor();

                            ImGui::SameLine(0, 4);

                            // Carregar direto no Deck Right
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.50f, 0.00f, 0.35f, 0.85f));
                            char btn_dr[32];
                            snprintf(btn_dr, sizeof(btn_dr), "DECK %d", right_id + 1);
                            if (ImGui::SmallButton(btn_dr)) {
                                spotify_service.importSearchResultTrack(r);
                                auto t_list = KuroAudio::DJLibraryManager::getInstance().getTracksForFolder("all");
                                for (auto& tr : t_list) {
                                    if (tr.id == r.id) {
                                        if (tr.is_downloaded) spotify_service.loadLibraryTrack(tr, right_id, engine);
                                        else spotify_service.downloadLibraryTrackAsync(tr, right_id, engine);
                                        break;
                                    }
                                }
                            }
                            ImGui::PopStyleColor();

                            ImGui::PopID();
                        }
                        ImGui::EndTable();
                    }
                }
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
        }

        // =========================================================================
        // MODAL DE CONFIRMAÇÃO DE SAÍDA E SALVAMENTO DE HOT CUES
        // =========================================================================
        void renderExitConfirmModal(KuroAudio::DJEngine& engine, bool* p_open) {
            if (!show_exit_confirm_modal) return;

            ImGuiViewport* vp = ImGui::GetMainViewport();
            ImVec2 msz = ImVec2(460.0f, 170.0f);
            ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + (vp->Size.x - msz.x) * 0.5f, vp->Pos.y + (vp->Size.y - msz.y) * 0.5f), ImGuiCond_Always);
            ImGui::SetNextWindowSize(msz, ImGuiCond_Always);

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.12f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.30f, 0.40f, 0.90f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);

            if (ImGui::Begin("⚠️ CONFIRMAÇÃO DE SAÍDA##ExitConfirmModal", &show_exit_confirm_modal, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.4f, 1.0f), ICON_FA_TRIANGLE_EXCLAMATION " Deseja salvar as configurações e Hot Cues antes de sair?");
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "Suas marcações de Hot Cues (A-H) dos 4 decks serão salvas.");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Botão Salvar e Sair
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.55f, 0.45f, 0.95f));
                if (ImGui::Button(ICON_FA_FLOPPY_DISK " SALVAR E SAIR", ImVec2(150, 32))) {
                    engine.saveHotCues();
                    show_exit_confirm_modal = false;
                    if (p_open) *p_open = false;
                    is_open = false;
                }
                ImGui::PopStyleColor();

                ImGui::SameLine(0, 8);

                // Botão Sair Sem Salvar
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.12f, 0.15f, 0.90f));
                if (ImGui::Button(ICON_FA_XMARK " SAIR SEM SALVAR", ImVec2(140, 32))) {
                    show_exit_confirm_modal = false;
                    if (p_open) *p_open = false;
                    is_open = false;
                }
                ImGui::PopStyleColor();

                ImGui::SameLine(0, 8);

                // Botão Cancelar
                if (ImGui::Button("CANCELAR", ImVec2(100, 32))) {
                    show_exit_confirm_modal = false;
                }
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
