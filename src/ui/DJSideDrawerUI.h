#pragma once
#include <imgui.h>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include "IconsFontAwesome6.h"
#include "../core/DJHarmonicEngine.h"
#include "../core/DJLibraryManager.h"
#include "../core/DJSetlistManager.h"
#include "../core/DJSurgicalAudioEngine.h"
#include "../core/SpotifyDJService.h"
#include "../core/DJEngine.h"
#include "UIScaleManager.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroUI {

    class DJSideDrawerUI {
    public:
        // Configurações de layout da gaveta
        bool is_open = false;               // Fechada por padrão para dar foco total à Biblioteca Inferior Rekordbox
        bool dock_left = false;             // false = Direita (padrão), true = Esquerda
        float drawer_width = 410.0f;        // Largura ajustável
        const float min_width = 320.0f;
        const float max_width = 680.0f;
        const float splitter_w = 6.0f;

        // Modos de Organização / Ordenação da Biblioteca
        enum LibrarySortMode {
            SORT_SUGGESTION = 0, // 🎯 Sugestão Harmônica Inteligente (Combina Tom Camelot + proximidade de BPM com o Deck no ar)
            SORT_BPM,           // ⚡ BPM (Crescente / Decrescente)
            SORT_KEY,           // 🎹 Tom (Círculo Camelot 1A..12B)
            SORT_TITLE          // 🔤 Título (A-Z)
        };
        LibrarySortMode sort_mode = SORT_SUGGESTION;
        bool sort_bpm_ascending = true;
        bool sort_key_ascending = true;

        // Estado do Radar, Lookahead & Hover Preditivo Instantâneo
        bool show_camelot_radar = true; // Permite colapsar para liberar espaço máximo para as músicas
        std::string hovered_track_id = "";
        std::string hovered_track_key = "";
        double hovered_track_bpm = 138.0;
        std::string hovered_track_title = "";
        std::string hovered_track_artist = "";
        float hover_anim_timer = 0.0f;
        float radar_pulse_phase = 0.0f;

        // Detectores temporários de hover por frame
        std::string detected_hover_id = "";
        std::string detected_hover_key = "";
        double detected_hover_bpm = 138.0;
        std::string detected_hover_title = "";
        std::string detected_hover_artist = "";

        bool external_hover_active = false;

        void setHoveredTrack(const std::string& id, const std::string& key, double bpm, const std::string& title = "", const std::string& artist = "") {
            detected_hover_id = id;
            detected_hover_key = key;
            detected_hover_bpm = bpm;
            detected_hover_title = title;
            detected_hover_artist = artist;
            hovered_track_id = id;
            hovered_track_key = key;
            hovered_track_bpm = bpm;
            hovered_track_title = title;
            hovered_track_artist = artist;
            hover_anim_timer = 1.0f;
            external_hover_active = true;
        }

        // Filtros rápidos
        char search_buf[128] = "";
        int active_genre_filter = 0; // 0=Todos, 1=Prog Psy, 2=Full-On, 3=Tech House, 4=Acid, 5=Melodic

        // Diálogos secundários
        bool show_edit_metadata_modal = false;
        KuroAudio::DJLibraryTrack edit_track_copy;
        bool show_confirm_delete_modal = false;
        std::string delete_track_id = "";
        bool delete_disk_file_flag = false;
        bool show_surgical_results_dialog = false;
        KuroAudio::SurgicalAnalysisResult surgical_result;

        // Mensagem de feedback
        std::string toast_message = "";
        float toast_timer = 0.0f;

        // Comunicação externa com Kuro Cloud / Fontes de Download
        bool request_open_cloud_hub = false;
        bool request_open_sources_track = false;
        std::string request_track_id = "";
        std::string request_track_title = "";
        std::string request_track_artist = "";
        KuroAudio::SpotifyDJService* active_spotify_service = nullptr;

        // Renderização principal da Gaveta Lateral
        void render(KuroAudio::DJEngine& engine, KuroAudio::SpotifyDJService& spotify_service, float avail_h) {
            if (!is_open) return;
            active_spotify_service = &spotify_service;

            auto& lib_mgr = KuroAudio::DJLibraryManager::getInstance();
            auto& setlist_mgr = KuroAudio::DJSetlistManager::getInstance();

            // Atualiza timers de animação
            float dt = ImGui::GetIO().DeltaTime;
            if (dt <= 0.0f || dt > 0.1f) dt = 0.016f;
            radar_pulse_phase += dt * 3.5f;
            if (radar_pulse_phase > 6.28318f) radar_pulse_phase -= 6.28318f;

            if (!hovered_track_id.empty()) {
                hover_anim_timer = std::min(1.0f, hover_anim_timer + dt * 6.0f);
            } else {
                hover_anim_timer = std::max(0.0f, hover_anim_timer - dt * 4.0f);
            }

            // Identifica Deck Master Ativo
            int master_deck_idx = 0;
            if (engine.decks[1].is_playing.load() && !engine.decks[0].is_playing.load()) {
                master_deck_idx = 1;
            } else if (engine.decks[0].is_playing.load()) {
                master_deck_idx = 0;
            }
            auto& master_deck = engine.decks[master_deck_idx];
            std::string master_key_str = master_deck.getEffectiveKey();
            if (master_key_str.empty()) master_key_str = "9B / G";
            double master_bpm = master_deck.bpm > 40.0 ? master_deck.bpm : 138.0;

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.018f, 0.024f, 0.038f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.20f, 0.32f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);

            ImGui::BeginChild("##DJSideDrawerMain", ImVec2(drawer_width, avail_h), true, ImGuiWindowFlags_NoScrollbar);

            // 1. CABEÇALHO DA GAVETA COM CONTROLES DE POSIÇÃO
            renderHeader(spotify_service);

            // 2. RADAR CAMELOT PERMANENTE & MOTOR LOOKAHEAD DE FRASES
            if (show_camelot_radar) {
                renderCamelotRadarWidget(engine, master_deck_idx, master_key_str, master_bpm);
            } else {
                renderCompactRadarStrip(engine, master_deck_idx, master_key_str, master_bpm);
            }

            // 3. BARRA DE BUSCA E PÍLULAS DE GÊNEROS (EM LINHA ÚNICA)
            renderSearchAndFilterBar(spotify_service);

            // 4. FEED ROLÁVEL DE CARDS DE MÚSICA
            renderTrackCardsList(engine, spotify_service, master_deck_idx, master_key_str, master_bpm);

            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);

            // Diálogos modais auxiliares
            renderAuxiliaryDialogs(spotify_service);

            // Toast de Notificação
            if (toast_timer > 0.0f) {
                toast_timer -= dt;
                ImDrawList* dl = ImGui::GetForegroundDrawList();
                ImVec2 vp_size = ImGui::GetMainViewport()->Size;
                ImVec2 t_pos(vp_size.x * 0.5f - 180.0f, vp_size.y - 70.0f);
                dl->AddRectFilled(t_pos, ImVec2(t_pos.x + 360.0f, t_pos.y + 36.0f), IM_COL32(10, 24, 38, 235), 8.0f);
                dl->AddRect(t_pos, ImVec2(t_pos.x + 360.0f, t_pos.y + 36.0f), IM_COL32(0, 225, 255, 255), 8.0f, 0, 1.5f);
                dl->AddText(ImVec2(t_pos.x + 18.0f, t_pos.y + 9.0f), IM_COL32(255, 255, 255, 255), toast_message.c_str());
            }
        }

        // Divisor Arrastável (Splitter)
        void renderSplitter(float avail_h) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.06f, 0.09f, 0.70f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.85f, 0.95f, 0.50f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.95f, 1.00f, 0.90f));

            ImGui::SetNextItemAllowOverlap();
            ImGui::InvisibleButton("##SideDrawerSplitter", ImVec2(splitter_w, avail_h));
            
            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();
            if (hovered || active) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }

            if (active) {
                float mouse_delta_x = ImGui::GetIO().MouseDelta.x;
                if (dock_left) {
                    drawer_width += mouse_delta_x;
                } else {
                    drawer_width -= mouse_delta_x;
                }
                drawer_width = std::clamp(drawer_width, min_width, max_width);
            }

            // Desenha linha divisória iluminada
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p0 = ImGui::GetItemRectMin();
            ImVec2 p1 = ImGui::GetItemRectMax();
            ImU32 col = active ? IM_COL32(0, 240, 255, 255) : (hovered ? IM_COL32(0, 200, 230, 180) : IM_COL32(30, 42, 60, 200));
            dl->AddRectFilled(p0, p1, col);

            // Alça central (grip dots)
            float cy = (p0.y + p1.y) * 0.5f;
            float cx = (p0.x + p1.x) * 0.5f;
            dl->AddCircleFilled(ImVec2(cx, cy - 8.0f), 1.5f, IM_COL32(255, 255, 255, 220));
            dl->AddCircleFilled(ImVec2(cx, cy),        1.5f, IM_COL32(255, 255, 255, 220));
            dl->AddCircleFilled(ImVec2(cx, cy + 8.0f), 1.5f, IM_COL32(255, 255, 255, 220));

            ImGui::PopStyleColor(3);
        }

        // Botão de Recolhimento quando a gaveta está fechada
        void renderCollapsedStrip(float avail_h) {
            float strip_w = 26.0f;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.02f, 0.03f, 0.05f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.18f, 0.28f, 0.85f));
            ImGui::BeginChild("##SideDrawerCollapsedStrip", ImVec2(strip_w, avail_h), true, ImGuiWindowFlags_NoScrollbar);
            
            const char* icon = dock_left ? ICON_FA_CHEVRON_RIGHT : ICON_FA_CHEVRON_LEFT;
            if (ImGui::Button(icon, ImVec2(strip_w - 4.0f, 40.0f))) {
                is_open = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Expandir Gaveta Lateral de Biblioteca & Radar");

            // Texto vertical "BIBLIOTECA"
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 sp = ImGui::GetCursorScreenPos();
            dl->AddText(ImVec2(sp.x + 6.0f, sp.y + 20.0f), IM_COL32(0, 220, 240, 220), "B\nI\nB\nL\nI\nO\nT\nE\nC\nA");

            ImGui::EndChild();
            ImGui::PopStyleColor(2);
        }

    private:
        // 1. Cabeçalho da Gaveta
        void renderHeader(KuroAudio::SpotifyDJService& spotify_service) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.00f, 0.96f, 0.83f, 1.0f), ICON_FA_COMPASS " RADAR & BIBLIOTECA");
            
            float avail_w = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x * 2.0f;
            
            // Botão Fontes / Kuro Cloud Hub (Acesso Direto ao Buscador e Streaming)
            int dl_pct_drawer = active_spotify_service ? std::clamp((int)active_spotify_service->getDownloadProgressPct(), 0, 100) : 0;
            std::string btn_drawer = (active_spotify_service && active_spotify_service->isDownloading()) 
                ? (dl_pct_drawer > 0 ? (std::string(ICON_FA_BOLT) + " " + std::to_string(dl_pct_drawer) + "%") : (std::string(ICON_FA_BOLT) + " ..."))
                : (std::string(ICON_FA_SHUTTLE_SPACE) + " ABDUCTION HUB");
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.40f, 0.50f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.70f, 0.85f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.95f, 1.00f, 1.0f));
            if (ImGui::Button(btn_drawer.c_str(), ImVec2(124.0f, 22.0f))) {
                request_open_cloud_hub = true;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("ABDUCTION STUDIO // MASTER SOURCE MATRIX:\nBuscador de fontes de áudio e abdução de faixas de estúdio");
            }
            ImGui::PopStyleColor(3);

            // Botão 0: Colapsar / Expandir Radar Camelot (libera 168px verticais para ver mais músicas)
            ImGui::SameLine(avail_w - 176.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, show_camelot_radar ? ImVec4(0.00f, 0.45f, 0.55f, 0.85f) : ImVec4(0.08f, 0.14f, 0.22f, 0.85f));
            if (ImGui::Button(show_camelot_radar ? ICON_FA_COMPASS " Radar ▲" : ICON_FA_COMPASS " Radar ▼", ImVec2(74.0f, 22.0f))) {
                show_camelot_radar = !show_camelot_radar;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip(show_camelot_radar ? "Recolher Radar Camelot para ver muito mais músicas no feed" : "Expandir Radar Camelot & Lookahead");
            }
            ImGui::PopStyleColor();

            // Botão 1: Alternar Posição (Esquerda / Direita) com ícone claro e feedback visual
            ImGui::SameLine(avail_w - 96.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.15f, 0.25f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.80f, 0.95f, 0.50f));
            std::string dock_label = std::string(ICON_FA_ARROW_RIGHT_ARROW_LEFT) + (dock_left ? " ESQ" : " DIR");
            if (ImGui::Button(dock_label.c_str(), ImVec2(58.0f, 22.0f))) {
                dock_left = !dock_left;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Mudar Posição: Alternar Ancoragem da Gaveta (Atualmente na %s)", dock_left ? "Esquerda" : "Direita");
            }
            ImGui::PopStyleColor(2);

            // Botão 2: Recolher Gaveta
            ImGui::SameLine(0, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.16f, 0.22f, 0.90f));
            if (ImGui::Button(dock_left ? ICON_FA_CHEVRON_LEFT : ICON_FA_CHEVRON_RIGHT, ImVec2(28.0f, 22.0f))) {
                is_open = false;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Recolher Gaveta Lateral");
            ImGui::PopStyleColor();

            ImGui::Separator();
        }

        // 2b. Radar Camelot em Modo Compacto (libera 140px no feed de músicas)
        void renderCompactRadarStrip(KuroAudio::DJEngine& engine, int master_deck_idx, const std::string& master_key_str, double master_bpm) {
            float strip_w = ImGui::GetContentRegionAvail().x;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.022f, 0.032f, 0.052f, 0.96f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.10f, 0.18f, 0.30f, 0.75f));
            ImGui::BeginChild("##RadarCamelotCompactHUD", ImVec2(strip_w, 28.0f), true, ImGuiWindowFlags_NoScrollbar);
            
            KuroAudio::CamelotKey mk = KuroAudio::DJHarmonicEngine::parseKey(master_key_str);
            ImU32 mk_col = mk.valid ? KuroAudio::DJHarmonicEngine::getCamelotWheelColor(mk.number) : IM_COL32(0, 220, 255, 255);
            ImVec4 key_vec = ImGui::ColorConvertU32ToFloat4(mk_col);
            
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.00f, 0.85f, 0.95f, 1.0f), ICON_FA_COMPASS " RADAR:");
            ImGui::SameLine(0, 6.0f);
            ImGui::TextColored(key_vec, "Tom %s", master_key_str.c_str());
            ImGui::SameLine(0, 8.0f);
            ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.10f, 1.0f), "%.1f BPM", master_bpm);
            
            if (!hovered_track_id.empty()) {
                ImGui::SameLine(0, 10.0f);
                std::string comp_lbl = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeText(master_key_str, hovered_track_key);
                ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.70f, 1.0f), "| Hover: %s (%s)", hovered_track_key.c_str(), comp_lbl.c_str());
            }

            ImGui::SameLine(strip_w - 76.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.14f, 0.22f, 0.85f));
            if (ImGui::SmallButton(ICON_FA_CHEVRON_DOWN " Abrir")) {
                show_camelot_radar = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Expandir Círculo Camelot Completo");
            ImGui::PopStyleColor();

            ImGui::EndChild();
            ImGui::PopStyleColor(2);
        }

        // 2. Radar Camelot Permanente & Motor Lookahead com Suporte a Dual Deck (D1 + D2) e Hover Preditivo
        void renderCamelotRadarWidget(KuroAudio::DJEngine& engine, int master_deck_idx, const std::string& master_key_str, double master_bpm) {
            float widget_w = ImGui::GetContentRegionAvail().x;
            float widget_h = KuroUI::UIScaleManager::Scale(205.0f);

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.022f, 0.032f, 0.052f, 0.96f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.10f, 0.18f, 0.30f, 0.75f));
            ImGui::BeginChild("##RadarCamelotLookaheadHUD", ImVec2(widget_w, widget_h), true, ImGuiWindowFlags_NoScrollbar);

            // Chaves e status dos Decks
            std::string d1_key_str = engine.decks[0].getEffectiveKey();
            std::string d1_title = engine.decks[0].track_title;
            double d1_bpm = engine.decks[0].bpm > 30.0 ? engine.decks[0].bpm : 138.0;
            bool d1_loaded = !d1_title.empty() && d1_title != "No Track" && d1_title != "Empty";
            KuroAudio::CamelotKey d1_ck = KuroAudio::DJHarmonicEngine::parseKey(d1_key_str);

            std::string d2_key_str = engine.decks[1].getEffectiveKey();
            std::string d2_title = engine.decks[1].track_title;
            double d2_bpm = engine.decks[1].bpm > 30.0 ? engine.decks[1].bpm : 138.0;
            bool d2_loaded = !d2_title.empty() && d2_title != "No Track" && d2_title != "Empty";
            KuroAudio::CamelotKey d2_ck = KuroAudio::DJHarmonicEngine::parseKey(d2_key_str);

            KuroAudio::CamelotKey master_ck = KuroAudio::DJHarmonicEngine::parseKey(master_key_str);
            KuroAudio::CamelotKey hovered_ck = KuroAudio::DJHarmonicEngine::parseKey(hovered_track_key);
            bool has_hover = (hover_anim_timer > 0.01f && hovered_ck.valid && !hovered_track_id.empty());

            // Cálculos da Beatgrid e Fraseamento Preditivo (Lookahead)
            auto& m_deck = engine.decks[master_deck_idx];
            double elapsed = m_deck.getElapsedTimeSec();
            double beat_duration = 60.0 / std::max(master_bpm, 60.0);
            int current_beat = (int)(elapsed / beat_duration);
            int current_bar = current_beat / 4;

            int bars_in_phrase = 32;
            int bar_in_block = current_bar % bars_in_phrase;
            int bars_to_next_phase = bars_in_phrase - bar_in_block;
            if (bars_to_next_phase <= 0) bars_to_next_phase = 32;
            double sec_to_next_phase = bars_to_next_phase * 4.0 * beat_duration;

            const char* current_phase_str = "2. Construção & Tensão";
            const char* next_phase_str = "Breakdown Melódico";
            int target_rec_num = master_ck.valid ? master_ck.number : 9;
            char target_rec_letter = master_ck.valid ? master_ck.letter : 'B';

            if (bar_in_block < 8) {
                current_phase_str = "1. Intro & Hook";
                next_phase_str = "Build-Up (+1 Energia)";
                target_rec_num = (target_rec_num % 12) + 1; // +1 Camelot
            } else if (bar_in_block < 24) {
                current_phase_str = "2. Construção & Drop";
                next_phase_str = "Breakdown & Melodia";
                target_rec_letter = (target_rec_letter == 'A') ? 'B' : 'A'; // Relativo
            } else {
                current_phase_str = "3. Peak Time Clímax";
                next_phase_str = "Transição / Saída do Drop";
            }

            // Desenho do Disco Camelot (Esquerda)
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 screen_pos = ImGui::GetCursorScreenPos();
            float radar_radius = KuroUI::UIScaleManager::Scale(82.0f);
            ImVec2 radar_center(screen_pos.x + radar_radius + KuroUI::UIScaleManager::Scale(12.0f), screen_pos.y + widget_h * 0.5f);

            // Fundo escuro do radar com anel protetor
            dl->AddCircleFilled(radar_center, radar_radius + KuroUI::UIScaleManager::Scale(5.0f), IM_COL32(8, 12, 20, 245));
            dl->AddCircle(radar_center, radar_radius + KuroUI::UIScaleManager::Scale(5.0f), IM_COL32(35, 52, 75, 180), 36, KuroUI::UIScaleManager::Scale(1.0f));

            // Raios dos anéis
            float r_outer = radar_radius;
            float r_mid = radar_radius * 0.68f;
            float r_inner = radar_radius * 0.36f;

            // 12 Setores Camelot (Major B externo, Minor A interno)
            for (int i = 1; i <= 12; i++) {
                float angle_mid = (float)(-M_PI * 0.5 + (i % 12) * (2.0 * M_PI / 12.0));
                float angle_start = angle_mid - (float)(M_PI / 12.0);
                float angle_end = angle_mid + (float)(M_PI / 12.0);

                bool d1_is_b = (d1_loaded && d1_ck.valid && d1_ck.number == i && d1_ck.letter == 'B');
                bool d1_is_a = (d1_loaded && d1_ck.valid && d1_ck.number == i && d1_ck.letter == 'A');
                bool d2_is_b = (d2_loaded && d2_ck.valid && d2_ck.number == i && d2_ck.letter == 'B');
                bool d2_is_a = (d2_loaded && d2_ck.valid && d2_ck.number == i && d2_ck.letter == 'A');
                bool hov_is_b = (has_hover && hovered_ck.valid && hovered_ck.number == i && hovered_ck.letter == 'B');
                bool hov_is_a = (has_hover && hovered_ck.valid && hovered_ck.number == i && hovered_ck.letter == 'A');

                ImU32 base_col = KuroAudio::DJHarmonicEngine::getCamelotWheelColor(i);

                // Pulso animado para o setor em hover
                float hov_glow = has_hover ? (0.6f + 0.4f * std::sin(radar_pulse_phase * 2.2f)) : 1.0f;

                // --- SETOR EXTERNO (MAJOR B) ---
                ImU32 col_b = (base_col & 0x00FFFFFF) | 0x30000000;
                if (d1_is_b && d2_is_b) {
                    col_b = IM_COL32(0, 235, 255, 230); // Ambos Decks no mesmo tom!
                } else if (d1_is_b) {
                    col_b = IM_COL32(0, 220, 255, 230); // Deck 1 (Ciano)
                } else if (d2_is_b) {
                    col_b = IM_COL32(255, 140, 25, 230); // Deck 2 (Laranja)
                }
                if (hov_is_b) {
                    // Glow pulsante amarelo neon + layer de brilho extra
                    int hov_alpha = (int)(200.0f + 55.0f * hov_glow);
                    col_b = IM_COL32(255, 240, 50, hov_alpha);
                    // Halo externo de energia
                    dl->PathArcTo(radar_center, r_outer + KuroUI::UIScaleManager::Scale(3.0f), angle_start - 0.02f, angle_end + 0.02f, 8);
                    dl->PathStroke(IM_COL32(255, 220, 30, (int)(80.0f * hov_glow)), 0, KuroUI::UIScaleManager::Scale(6.0f));
                }

                dl->PathArcTo(radar_center, (r_outer + r_mid) * 0.5f, angle_start + 0.04f, angle_end - 0.04f, 6);
                dl->PathStroke(col_b, 0, (r_outer - r_mid));

                // --- SETOR INTERNO (MINOR A) ---
                ImU32 col_a = (base_col & 0x00FFFFFF) | 0x20000000;
                if (d1_is_a && d2_is_a) {
                    col_a = IM_COL32(0, 235, 255, 230);
                } else if (d1_is_a) {
                    col_a = IM_COL32(0, 220, 255, 230); // Deck 1 (Ciano)
                } else if (d2_is_a) {
                    col_a = IM_COL32(255, 140, 25, 230); // Deck 2 (Laranja)
                }
                if (hov_is_a) {
                    int hov_alpha = (int)(200.0f + 55.0f * hov_glow);
                    col_a = IM_COL32(255, 240, 50, hov_alpha);
                    // Halo interno de energia
                    dl->PathArcTo(radar_center, r_mid - KuroUI::UIScaleManager::Scale(2.0f), angle_start - 0.02f, angle_end + 0.02f, 8);
                    dl->PathStroke(IM_COL32(255, 220, 30, (int)(60.0f * hov_glow)), 0, KuroUI::UIScaleManager::Scale(5.0f));
                }

                dl->PathArcTo(radar_center, (r_mid + r_inner) * 0.5f, angle_start + 0.04f, angle_end - 0.04f, 6);
                dl->PathStroke(col_a, 0, (r_mid - r_inner));

                // Pins Indicadores de Decks Carregados nos Setores
                float pin_r = KuroUI::UIScaleManager::Scale(7.5f);
                if (d1_is_b || d1_is_a) {
                    float r_p1 = d1_is_b ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_inner) * 0.5f);
                    ImVec2 p_pin1(radar_center.x + std::cos(angle_mid) * r_p1, radar_center.y + std::sin(angle_mid) * r_p1);
                    dl->AddCircleFilled(p_pin1, pin_r, IM_COL32(0, 20, 32, 230));
                    dl->AddCircle(p_pin1, pin_r, IM_COL32(0, 235, 255, 255), 12, KuroUI::UIScaleManager::Scale(1.5f));
                    ImVec2 p1_sz = ImGui::CalcTextSize("1");
                    dl->AddText(ImVec2(p_pin1.x - p1_sz.x * 0.5f, p_pin1.y - p1_sz.y * 0.5f), IM_COL32(0, 255, 255, 255), "1");
                }
                if (d2_is_b || d2_is_a) {
                    float r_p2 = d2_is_b ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_inner) * 0.5f);
                    ImVec2 p_pin2(radar_center.x + std::cos(angle_mid) * r_p2, radar_center.y + std::sin(angle_mid) * r_p2);
                    if ((d1_is_b && d2_is_b) || (d1_is_a && d2_is_a)) {
                        p_pin2.x += KuroUI::UIScaleManager::Scale(4.0f);
                    }
                    dl->AddCircleFilled(p_pin2, pin_r, IM_COL32(32, 16, 0, 230));
                    dl->AddCircle(p_pin2, pin_r, IM_COL32(255, 140, 20, 255), 12, KuroUI::UIScaleManager::Scale(1.5f));
                    ImVec2 p2_sz = ImGui::CalcTextSize("2");
                    dl->AddText(ImVec2(p_pin2.x - p2_sz.x * 0.5f, p_pin2.y - p2_sz.y * 0.5f), IM_COL32(255, 170, 40, 255), "2");
                }

                // Rótulo numérico no anel externo
                float lx = radar_center.x + std::cos(angle_mid) * (r_outer - KuroUI::UIScaleManager::Scale(9.0f));
                float ly = radar_center.y + std::sin(angle_mid) * (r_outer - KuroUI::UIScaleManager::Scale(9.0f));
                char num_txt[8]; snprintf(num_txt, sizeof(num_txt), "%d", i);
                bool is_lit = (d1_is_b || d2_is_b || hov_is_b);
                ImU32 num_col = is_lit ? IM_COL32(255, 255, 255, 255) : IM_COL32(160, 180, 200, 170);
                if (hov_is_b || hov_is_a) num_col = IM_COL32(255, 245, 80, 255); // Amarelo neon no hover
                ImVec2 n_sz = ImGui::CalcTextSize(num_txt);
                dl->AddText(ImVec2(lx - n_sz.x * 0.5f, ly - n_sz.y * 0.5f), num_col, num_txt);
            }

            // PONTE HARMÔNICA ENTRE DECK 1 E DECK 2 NO DISCO
            if (d1_loaded && d2_loaded && d1_ck.valid && d2_ck.valid) {
                float a_d1 = (float)(-M_PI * 0.5 + (d1_ck.number % 12) * (2.0 * M_PI / 12.0));
                float r_d1 = (d1_ck.letter == 'B') ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_inner) * 0.5f);
                ImVec2 pt_d1(radar_center.x + std::cos(a_d1) * r_d1, radar_center.y + std::sin(a_d1) * r_d1);

                float a_d2 = (float)(-M_PI * 0.5 + (d2_ck.number % 12) * (2.0 * M_PI / 12.0));
                float r_d2 = (d2_ck.letter == 'B') ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_inner) * 0.5f);
                ImVec2 pt_d2(radar_center.x + std::cos(a_d2) * r_d2, radar_center.y + std::sin(a_d2) * r_d2);

                if (d1_ck.number != d2_ck.number || d1_ck.letter != d2_ck.letter) {
                    dl->AddLine(pt_d1, pt_d2, IM_COL32(0, 220, 255, 140), KuroUI::UIScaleManager::Scale(2.5f));
                    dl->AddLine(pt_d1, pt_d2, IM_COL32(255, 140, 30, 100), KuroUI::UIScaleManager::Scale(1.2f));
                }
            }

            // CENTRO DO RADAR (HUB TONAL DO DECK MASTER)
            dl->AddCircleFilled(radar_center, r_inner, IM_COL32(10, 16, 26, 255));
            dl->AddCircle(radar_center, r_inner, (master_deck_idx == 0) ? IM_COL32(0, 220, 255, 220) : IM_COL32(255, 140, 30, 220), 24, KuroUI::UIScaleManager::Scale(1.5f));
            std::string code = master_ck.valid ? master_ck.getCamelotCode() : "9B";
            ImVec2 c_sz = ImGui::CalcTextSize(code.c_str());
            dl->AddText(ImVec2(radar_center.x - c_sz.x * 0.5f, radar_center.y - c_sz.y * 0.5f), (master_deck_idx == 0) ? IM_COL32(0, 240, 255, 255) : IM_COL32(255, 160, 40, 255), code.c_str());

            // FEIXE HARMÔNICO DINÂMICO (LASER BEAM) AO PASSAR O MOUSE
            if (has_hover && master_ck.valid) {
                float a_src = (float)(-M_PI * 0.5 + (master_ck.number % 12) * (2.0 * M_PI / 12.0));
                float r_src = (master_ck.letter == 'B') ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_inner) * 0.5f);
                ImVec2 p_src(radar_center.x + std::cos(a_src) * r_src, radar_center.y + std::sin(a_src) * r_src);

                float a_tgt = (float)(-M_PI * 0.5 + (hovered_ck.number % 12) * (2.0 * M_PI / 12.0));
                float r_tgt = (hovered_ck.letter == 'B') ? ((r_outer + r_mid) * 0.5f) : ((r_mid + r_inner) * 0.5f);
                ImVec2 p_tgt(radar_center.x + std::cos(a_tgt) * r_tgt, radar_center.y + std::sin(a_tgt) * r_tgt);

                KuroAudio::HarmonicCompatibility comp = KuroAudio::DJHarmonicEngine::compareKeys(master_ck, hovered_ck);
                ImU32 beam_col = IM_COL32(0, 255, 128, 255); // Verde 100%
                if (comp == KuroAudio::HarmonicCompatibility::HARMONIC_RELATIVE) beam_col = IM_COL32(0, 225, 255, 255); // Ciano
                else if (comp == KuroAudio::HarmonicCompatibility::ENERGY_PLUS_1)  beam_col = IM_COL32(255, 205, 0, 255);  // Dourado
                else if (comp == KuroAudio::HarmonicCompatibility::ENERGY_MINUS_1) beam_col = IM_COL32(0, 180, 255, 255);  // Azul
                else if (comp == KuroAudio::HarmonicCompatibility::ENERGY_BOOST_2) beam_col = IM_COL32(255, 45, 140, 255); // Magenta
                else if (comp == KuroAudio::HarmonicCompatibility::CLASH)          beam_col = IM_COL32(255, 60, 60, 255);   // Vermelho

                float alpha = hover_anim_timer * (0.80f + 0.20f * std::sin(radar_pulse_phase));
                ImU32 pulse_col = (beam_col & 0x00FFFFFF) | ((int)(alpha * 255.0f) << 24);
                ImU32 glow_col  = (beam_col & 0x00FFFFFF) | ((int)(alpha * 90.0f) << 24);
                ImU32 core_col  = (beam_col & 0x00FFFFFF) | ((int)(alpha * 255.0f) << 24);

                // Camada de glow difuso (mais larga e semitransparente)
                dl->AddLine(p_src, p_tgt, glow_col, KuroUI::UIScaleManager::Scale(7.0f));
                // Feixe principal nítido
                dl->AddLine(p_src, p_tgt, pulse_col, KuroUI::UIScaleManager::Scale(2.5f));
                // Linha central branca brilhante (core do laser)
                dl->AddLine(p_src, p_tgt, IM_COL32(255, 255, 255, (int)(alpha * 120.0f)), KuroUI::UIScaleManager::Scale(1.0f));
                // Marcador de destino com anel pulsante duplo
                dl->AddCircleFilled(p_tgt, KuroUI::UIScaleManager::Scale(5.5f), core_col);
                dl->AddCircleFilled(p_tgt, KuroUI::UIScaleManager::Scale(2.5f), IM_COL32(255, 255, 255, (int)(alpha * 200.0f)));
                dl->AddCircle(p_tgt, KuroUI::UIScaleManager::Scale(8.0f) + KuroUI::UIScaleManager::Scale(3.5f) * std::sin(radar_pulse_phase), pulse_col, 20, KuroUI::UIScaleManager::Scale(1.5f));
                dl->AddCircle(p_tgt, KuroUI::UIScaleManager::Scale(12.0f) + KuroUI::UIScaleManager::Scale(2.0f) * std::sin(radar_pulse_phase + 1.0f), glow_col, 16, KuroUI::UIScaleManager::Scale(1.0f));
            }

            // PAINEL TEXTUAL DE DUAL DECK & PREVISÃO DE ENCAIXE (DIREITA)
            float text_x = screen_pos.x + radar_radius * 2.0f + KuroUI::UIScaleManager::Scale(26.0f);
            float text_y = screen_pos.y + KuroUI::UIScaleManager::Scale(7.0f);

            dl->AddText(ImVec2(text_x, text_y), IM_COL32(0, 220, 255, 255), ICON_FA_COMPASS " RADAR PREDITIVO · DUAL DECK & LOOKAHEAD");

            // Status Compacto dos Dois Decks Carregados
            std::string d1_desc = "[D1] " + (d1_loaded ? (d1_title.substr(0, 15) + " (" + (d1_ck.valid ? d1_ck.getCamelotCode() : d1_key_str) + ")") : "Vazio");
            dl->AddText(ImVec2(text_x, text_y + KuroUI::UIScaleManager::Scale(17.0f)), (master_deck_idx == 0) ? IM_COL32(0, 240, 255, 255) : IM_COL32(140, 190, 210, 200), d1_desc.c_str());

            float d1_w = ImGui::CalcTextSize(d1_desc.c_str()).x;
            float d2_x = text_x + std::max(d1_w + KuroUI::UIScaleManager::Scale(12.0f), KuroUI::UIScaleManager::Scale(140.0f));
            std::string d2_desc = "[D2] " + (d2_loaded ? (d2_title.substr(0, 15) + " (" + (d2_ck.valid ? d2_ck.getCamelotCode() : d2_key_str) + ")") : "Vazio");
            dl->AddText(ImVec2(d2_x, text_y + KuroUI::UIScaleManager::Scale(17.0f)), (master_deck_idx == 1) ? IM_COL32(255, 160, 40, 255) : IM_COL32(210, 150, 90, 200), d2_desc.c_str());

            // Fase e Fraseamento do Master
            dl->AddText(ImVec2(text_x, text_y + KuroUI::UIScaleManager::Scale(35.0f)), IM_COL32(255, 185, 30, 255), "Fase Atual:");
            dl->AddText(ImVec2(text_x + KuroUI::UIScaleManager::Scale(68.0f), text_y + KuroUI::UIScaleManager::Scale(35.0f)), IM_COL32(240, 245, 255, 255), current_phase_str);

            char next_str[96];
            snprintf(next_str, sizeof(next_str), "Próx: %s em %d bars (%02d:%02d)", next_phase_str, bars_to_next_phase, (int)sec_to_next_phase / 60, (int)sec_to_next_phase % 60);
            dl->AddText(ImVec2(text_x, text_y + KuroUI::UIScaleManager::Scale(51.0f)), IM_COL32(0, 255, 140, 255), next_str);

            // ÁREA DE PREVISÃO EM TEMPO REAL AO PASSAR O MOUSE
            if (has_hover && hovered_ck.valid) {
                std::string match_badge = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeText(master_key_str, hovered_track_key);
                ImU32 badge_c = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeColor(master_key_str, hovered_track_key);

                // Box translúcida de destaque para a previsão
                ImVec2 box_p0(text_x - KuroUI::UIScaleManager::Scale(4.0f), text_y + KuroUI::UIScaleManager::Scale(68.0f));
                ImVec2 box_p1(screen_pos.x + widget_w - KuroUI::UIScaleManager::Scale(6.0f), text_y + KuroUI::UIScaleManager::Scale(185.0f));
                dl->AddRectFilled(box_p0, box_p1, IM_COL32(14, 22, 34, 225), KuroUI::UIScaleManager::Scale(4.0f));
                dl->AddRect(box_p0, box_p1, (badge_c & 0x00FFFFFF) | 0x90000000, KuroUI::UIScaleManager::Scale(4.0f), 0, KuroUI::UIScaleManager::Scale(1.0f));

                // Faixa candidata sob o mouse
                std::string cand_title = hovered_track_title.empty() ? "Faixa sob o mouse" : hovered_track_title.substr(0, 20);
                char cand_str[96];
                snprintf(cand_str, sizeof(cand_str), "🎯 %s · %s", cand_title.c_str(), hovered_ck.getCamelotCode().c_str());
                dl->AddText(ImVec2(text_x, text_y + KuroUI::UIScaleManager::Scale(72.0f)), IM_COL32(255, 235, 60, 255), cand_str);

                char comp_str[96];
                snprintf(comp_str, sizeof(comp_str), "Harmonia: %s", match_badge.c_str());
                dl->AddText(ImVec2(text_x, text_y + KuroUI::UIScaleManager::Scale(90.0f)), badge_c, comp_str);

                double bpm_diff = hovered_track_bpm - master_bpm;
                double pitch_pct = (master_bpm > 0.0) ? ((bpm_diff / master_bpm) * 100.0) : 0.0;
                const char* diag = (std::abs(bpm_diff) <= 2.5) ? "Encaixe Imediato" : ((std::abs(bpm_diff) <= 6.0) ? "Pitch Fluido" : "Sync de BPM");
                char bpm_sim[96];
                snprintf(bpm_sim, sizeof(bpm_sim), "BPM: %.1f (Δ %+.1f | %+.1f%%) · %s", hovered_track_bpm, bpm_diff, pitch_pct, diag);
                dl->AddText(ImVec2(text_x, text_y + KuroUI::UIScaleManager::Scale(108.0f)), IM_COL32(0, 220, 255, 230), bpm_sim);

                dl->AddText(ImVec2(text_x, text_y + KuroUI::UIScaleManager::Scale(126.0f)), IM_COL32(170, 195, 220, 200), "Dica: Mova o cursor por outras faixas para ver outros tons!");
            } else {
                char target_rec[64];
                snprintf(target_rec, sizeof(target_rec), "Alvo Sugerido: %d%c / %d%c", target_rec_num, target_rec_letter, master_ck.number, (master_ck.letter == 'A' ? 'B' : 'A'));
                dl->AddText(ImVec2(text_x, text_y + KuroUI::UIScaleManager::Scale(74.0f)), IM_COL32(255, 205, 40, 255), target_rec);
                dl->AddText(ImVec2(text_x, text_y + KuroUI::UIScaleManager::Scale(94.0f)), IM_COL32(0, 230, 255, 220), "💡 Passe o mouse nas faixas abaixo");
                dl->AddText(ImVec2(text_x, text_y + KuroUI::UIScaleManager::Scale(112.0f)), IM_COL32(180, 200, 220, 200), "para prever o encaixe tonal & BPM na hora!");
            }

            ImGui::EndChild();
            ImGui::PopStyleColor(2);
        }

        // 3. Barra de Pastas / Crates, Vertentes e Busca
        void renderSearchAndFilterBar(KuroAudio::SpotifyDJService& spotify_service) {
            auto& lib_mgr = KuroAudio::DJLibraryManager::getInstance();
            std::string cur_folder = lib_mgr.getActiveFolderId();
            auto folders = lib_mgr.getFolders();

            ImGui::Spacing();

            // Linha Organizada de Pastas & Crates com Auto-Wrap (Visibilidade Total sem Corte)
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));

            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.00f, 0.88f, 0.96f, 1.0f), ICON_FA_FOLDER_OPEN " CRATES:");

            float max_line_w = drawer_width - ImGui::GetStyle().WindowPadding.x * 2.0f - 14.0f;
            float crates_tag_w = ImGui::CalcTextSize(ICON_FA_FOLDER_OPEN " CRATES:").x + 8.0f;
            float cur_line_x = crates_tag_w;

            for (size_t i = 0; i < folders.size(); i++) {
                auto& f = folders[i];
                bool is_active = (cur_folder == f.id);

                char btn_lbl[128];
                snprintf(btn_lbl, sizeof(btn_lbl), "%s (%d)", f.name.c_str(), f.track_count);

                float btn_w = ImGui::CalcTextSize(btn_lbl).x + ImGui::GetStyle().FramePadding.x * 2.0f + 6.0f;
                if (i == 0) {
                    ImGui::SameLine(0, 6.0f);
                    cur_line_x += btn_w + 6.0f;
                } else {
                    if (cur_line_x + btn_w > max_line_w) {
                        // Quebra de linha limpa sem overflow
                        cur_line_x = btn_w + 4.0f;
                    } else {
                        ImGui::SameLine(0, 4.0f);
                        cur_line_x += btn_w + 4.0f;
                    }
                }

                if (is_active) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.70f, 0.85f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.02f, 0.04f, 0.06f, 1.0f));
                } else {
                    if (f.id == "first_kuru") {
                        // Destaque em ciano para a pasta importada real do usuário
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.20f, 0.32f, 0.95f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.30f, 0.92f, 1.00f, 1.0f));
                    } else if (f.id == "psy_fullon") {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.09f, 0.26f, 0.90f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.65f, 1.00f, 1.0f));
                    } else if (f.id == "forest_dark") {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.20f, 0.14f, 0.90f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.95f, 0.70f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.08f, 0.12f, 0.85f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.80f, 0.90f, 0.90f));
                    }
                }

                if (ImGui::SmallButton(btn_lbl)) {
                    lib_mgr.setActiveFolderId(f.id);
                }
                ImGui::PopStyleColor(2);
            }
            ImGui::PopStyleVar(2);

            ImGui::Spacing();

            // LINHA DE ORDENAÇÃO: SUGESTÃO, BPM, TOM CAMELOT E TÍTULO
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.00f, 0.88f, 0.96f, 1.0f), ICON_FA_ARROW_DOWN_WIDE_SHORT " ORDENAR:");
            ImGui::SameLine();

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));

            // Botão 1: Sugestão Harmônica (Tom + BPM combinados)
            bool is_sug = (sort_mode == SORT_SUGGESTION);
            if (is_sug) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.58f, 0.68f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.11f, 0.17f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.78f, 0.90f, 0.90f));
            }
            if (ImGui::SmallButton(ICON_FA_BULLSEYE " Sugestão (Tom+BPM)")) {
                sort_mode = SORT_SUGGESTION;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("🎯 Sugestão Harmônica: Agrupa e prioriza músicas com tom compatível (mesmo tom, +1/-1 energia, relativo) e BPM próximo ao deck no ar!");
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 4.0f);

            // Botão 2: BPM
            bool is_bpm = (sort_mode == SORT_BPM);
            if (is_bpm) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.45f, 0.05f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.11f, 0.17f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.78f, 0.90f, 0.90f));
            }
            char bpm_lbl[32];
            snprintf(bpm_lbl, sizeof(bpm_lbl), "%s BPM %s", ICON_FA_BOLT, is_bpm ? (sort_bpm_ascending ? "▲" : "▼") : "");
            if (ImGui::SmallButton(bpm_lbl)) {
                if (sort_mode == SORT_BPM) {
                    sort_bpm_ascending = !sort_bpm_ascending;
                } else {
                    sort_mode = SORT_BPM;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("⚡ Organizar por BPM (%s). Clique novamente para inverter crescente / decrescente.", sort_bpm_ascending ? "Crescente" : "Decrescente");
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 4.0f);

            // Botão 3: Tom (Camelot Wheel)
            bool is_key = (sort_mode == SORT_KEY);
            if (is_key) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.20f, 0.65f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.11f, 0.17f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.78f, 0.90f, 0.90f));
            }
            char key_lbl[32];
            snprintf(key_lbl, sizeof(key_lbl), "Tom %s", is_key ? (sort_key_ascending ? "▲" : "▼") : "");
            if (ImGui::SmallButton(key_lbl)) {
                if (sort_mode == SORT_KEY) {
                    sort_key_ascending = !sort_key_ascending;
                } else {
                    sort_mode = SORT_KEY;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("🎹 Organizar pelo Círculo Camelot (1A, 1B, 2A, 2B... 12B). Clique para alternar.");
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 4.0f);

            // Botão 4: Título
            bool is_tit = (sort_mode == SORT_TITLE);
            if (is_tit) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.38f, 0.58f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.11f, 0.17f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.78f, 0.90f, 0.90f));
            }
            if (ImGui::SmallButton("Nome")) {
                sort_mode = SORT_TITLE;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("🔤 Organizar em ordem alfabética por Título.");
            ImGui::PopStyleColor(2);

            ImGui::PopStyleVar(2);

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.04f, 0.06f, 0.09f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.22f, 0.35f, 0.80f));

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 88.0f);
            ImGui::InputTextWithHint("##SideDrawerSearch", ICON_FA_MAGNIFYING_GLASS " Filtrar nesta pasta (título, artista, BPM, tom)...", search_buf, sizeof(search_buf));
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.40f, 0.50f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.70f, 0.85f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.95f, 1.00f, 1.0f));
            int dl_pct_filter = active_spotify_service ? std::clamp((int)active_spotify_service->getDownloadProgressPct(), 0, 100) : 0;
            std::string btn_filter_abduzir = (active_spotify_service && active_spotify_service->isDownloading()) 
                ? (dl_pct_filter > 0 ? (std::string(ICON_FA_BOLT) + " " + std::to_string(dl_pct_filter) + "%") : (std::string(ICON_FA_BOLT) + " ..."))
                : (std::string(ICON_FA_SHUTTLE_SPACE) + " ABDUCTION HUB");
            if (ImGui::Button(btn_filter_abduzir.c_str(), ImVec2(128.0f, 0.0f))) {
                request_open_cloud_hub = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("ABDUCTION STUDIO // MASTER SOURCE MATRIX:\nBuscador de fontes e abdução de novas faixas para Crates");
            ImGui::PopStyleColor(3);
            ImGui::Spacing();

            // Pílulas de Vertentes Musicais (Subgêneros rápidos)
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.85f, 0.45f, 0.95f, 1.0f), ICON_FA_TAGS " VERTENTE:");
            ImGui::SameLine(0, 4.0f);

            auto vert_pill = [&](const char* label, int filter_idx, ImVec4 col) {
                bool is_active = (active_genre_filter == filter_idx);
                if (is_active) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.x * 0.40f, col.y * 0.40f, col.z * 0.40f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Text, col);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.09f, 0.13f, 0.85f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.72f, 0.80f, 0.85f));
                }
                if (ImGui::SmallButton(label)) {
                    active_genre_filter = is_active ? 0 : filter_idx;
                }
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 3.0f);
            };

            vert_pill("Todas", 0, ImVec4(0.35f, 0.90f, 1.00f, 1.0f));
            vert_pill("Prog", 1, ImVec4(1.00f, 0.58f, 0.16f, 1.0f));
            vert_pill("Full-On", 2, ImVec4(0.85f, 0.40f, 1.00f, 1.0f));
            vert_pill("Forest", 3, ImVec4(0.20f, 0.95f, 0.55f, 1.0f));
            vert_pill("Tech", 4, ImVec4(0.00f, 0.85f, 0.95f, 1.0f));
            ImGui::Spacing();
        }

        // 4. Feed Rolável de Cards de Música
        void renderTrackCardsList(KuroAudio::DJEngine& engine, KuroAudio::SpotifyDJService& spotify_service, int master_deck_idx, const std::string& master_key_str, double master_bpm) {
            auto& lib_mgr = KuroAudio::DJLibraryManager::getInstance();
            std::string active_folder = lib_mgr.getActiveFolderId();
            auto tracks = lib_mgr.getTracksForFolder(active_folder);

            // Filtro por texto de busca
            std::string q = search_buf;
            std::transform(q.begin(), q.end(), q.begin(), ::tolower);

            std::vector<KuroAudio::DJLibraryTrack> filtered;
            for (const auto& t : tracks) {
                // Filtro estrito: expurgar qualquer dummy de acervo que tenha sobrado
                std::string a_low = t.artist;
                std::transform(a_low.begin(), a_low.end(), a_low.begin(), ::tolower);
                std::string tit_low = t.title;
                std::transform(tit_low.begin(), tit_low.end(), tit_low.begin(), ::tolower);
                if (a_low.find("acervo") != std::string::npos || tit_low.find("acervo") != std::string::npos || a_low.find("coleção first kuru") != std::string::npos) continue;

                // Filtro por vertente / subgênero
                if (active_genre_filter == 1 && t.subgenre.find("Progressive") == std::string::npos) continue;
                if (active_genre_filter == 2 && t.subgenre.find("Full-On") == std::string::npos) continue;
                if (active_genre_filter == 3 && t.subgenre.find("Forest") == std::string::npos && t.subgenre.find("Twilight") == std::string::npos) continue;
                if (active_genre_filter == 4 && t.subgenre.find("Tech") == std::string::npos) continue;

                if (!q.empty()) {
                    std::string t_all = t.title + " " + t.artist + " " + t.key + " " + t.subgenre + " " + std::to_string((int)t.bpm);
                    std::transform(t_all.begin(), t_all.end(), t_all.begin(), ::tolower);
                    if (t_all.find(q) == std::string::npos) continue;
                }

                filtered.push_back(t);
            }

            // APLICAÇÃO DO MODO DE ORDENAÇÃO SELECIONADO
            if (sort_mode == SORT_SUGGESTION) {
                // 🎯 SUGESTÃO HARMÔNICA (REÚNE TOM CAMELOT + BPM CONFORME A LEITURA DA MÚSICA NO AR)
                auto master_ck = KuroAudio::DJHarmonicEngine::parseKey(master_key_str);
                std::sort(filtered.begin(), filtered.end(), [&](const KuroAudio::DJLibraryTrack& a, const KuroAudio::DJLibraryTrack& b) {
                    auto get_suggestion_score = [&](const KuroAudio::DJLibraryTrack& t) -> double {
                        double harm_penalty = 50.0;
                        if (master_ck.valid) {
                            auto comp = KuroAudio::DJHarmonicEngine::compareKeys(master_key_str, t.key);
                            switch (comp) {
                                case KuroAudio::HarmonicCompatibility::PERFECT:           harm_penalty = 0.0; break;
                                case KuroAudio::HarmonicCompatibility::HARMONIC_RELATIVE:  harm_penalty = 4.0; break;
                                case KuroAudio::HarmonicCompatibility::ENERGY_PLUS_1:     harm_penalty = 6.0; break;
                                case KuroAudio::HarmonicCompatibility::ENERGY_MINUS_1:    harm_penalty = 8.0; break;
                                case KuroAudio::HarmonicCompatibility::ENERGY_BOOST_2:    harm_penalty = 15.0; break;
                                case KuroAudio::HarmonicCompatibility::DIAGONAL_PLUS_1:   harm_penalty = 18.0; break;
                                case KuroAudio::HarmonicCompatibility::DIAGONAL_MINUS_1:  harm_penalty = 20.0; break;
                                default:                                                  harm_penalty = 45.0; break;
                            }
                        }
                        double bpm_diff = std::abs(t.bpm - master_bpm);
                        return (harm_penalty * 2.5) + bpm_diff;
                    };
                    return get_suggestion_score(a) < get_suggestion_score(b);
                });
            } else if (sort_mode == SORT_BPM) {
                // ⚡ ORDENAÇÃO POR BPM
                std::sort(filtered.begin(), filtered.end(), [&](const KuroAudio::DJLibraryTrack& a, const KuroAudio::DJLibraryTrack& b) {
                    return sort_bpm_ascending ? (a.bpm < b.bpm) : (a.bpm > b.bpm);
                });
            } else if (sort_mode == SORT_KEY) {
                // 🎹 ORDENAÇÃO POR TOM (CÍRCULO CAMELOT 1A..12B)
                std::sort(filtered.begin(), filtered.end(), [&](const KuroAudio::DJLibraryTrack& a, const KuroAudio::DJLibraryTrack& b) {
                    auto cka = KuroAudio::DJHarmonicEngine::parseKey(a.key);
                    auto ckb = KuroAudio::DJHarmonicEngine::parseKey(b.key);
                    int val_a = cka.valid ? (cka.number * 2 + (cka.letter == 'B' ? 1 : 0)) : 999;
                    int val_b = ckb.valid ? (ckb.number * 2 + (ckb.letter == 'B' ? 1 : 0)) : 999;
                    return sort_key_ascending ? (val_a < val_b) : (val_a > val_b);
                });
            } else if (sort_mode == SORT_TITLE) {
                // 🔤 ORDENAÇÃO POR TÍTULO
                std::sort(filtered.begin(), filtered.end(), [&](const KuroAudio::DJLibraryTrack& a, const KuroAudio::DJLibraryTrack& b) {
                    return a.title < b.title;
                });
            }

            float avail_h = ImGui::GetContentRegionAvail().y;
            ImGui::BeginChild("##TrackCardsScrollArea", ImVec2(0, avail_h), false);

            // Reseta detecção temporária deste frame somente se não veio de hover externo
            if (!external_hover_active) {
                detected_hover_id = "";
                detected_hover_key = "";
                detected_hover_bpm = 138.0;
                detected_hover_title = "";
                detected_hover_artist = "";
            }
            external_hover_active = false;

            for (size_t i = 0; i < filtered.size(); i++) {
                auto& trk = filtered[i];
                ImGui::PushID((int)(i + 1000));
                renderSingleTrackCard(trk, engine, spotify_service, master_key_str, master_bpm);
                ImGui::PopID();
                ImGui::Spacing();
            }

            if (filtered.empty()) {
                ImGui::TextDisabled("Nenhuma música encontrada com os filtros selecionados.");
            }

            // ATUALIZAÇÃO INSTANTÂNEA E FLUIDA DO HOVER
            float dt = ImGui::GetIO().DeltaTime;
            if (dt <= 0.0f || dt > 0.1f) dt = 0.016f;

            if (!detected_hover_id.empty()) {
                hovered_track_id = detected_hover_id;
                hovered_track_key = detected_hover_key;
                hovered_track_bpm = detected_hover_bpm;
                hovered_track_title = detected_hover_title;
                hovered_track_artist = detected_hover_artist;
                hover_anim_timer = 1.0f; // Imediato sem delay
            } else {
                hover_anim_timer = std::max(0.0f, hover_anim_timer - dt * 4.0f);
                if (hover_anim_timer <= 0.01f) {
                    hovered_track_id = "";
                }
            }

            ImGui::EndChild();
        }

        // Card Individual de Cada Música
        void renderSingleTrackCard(KuroAudio::DJLibraryTrack& trk, KuroAudio::DJEngine& engine, KuroAudio::SpotifyDJService& spotify_service, const std::string& master_key_str, double master_bpm) {
            float card_w = ImGui::GetContentRegionAvail().x;
            float card_h = 104.0f;

            KuroAudio::CamelotKey trk_ck = KuroAudio::DJHarmonicEngine::parseKey(trk.key);
            ImU32 key_color = trk_ck.valid ? KuroAudio::DJHarmonicEngine::getCamelotWheelColor(trk_ck.number) : IM_COL32(0, 220, 255, 255);

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.035f, 0.045f, 0.07f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.18f, 0.28f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);

            std::string child_id = "##Card_" + trk.id;
            ImGui::BeginChild(child_id.c_str(), ImVec2(card_w, card_h), true, ImGuiWindowFlags_NoScrollbar);

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 c_p0 = ImGui::GetWindowPos();
            ImVec2 c_p1 = ImVec2(c_p0.x + card_w, c_p0.y + card_h);
            ImVec2 m_pos = ImGui::GetIO().MousePos;

            // DETECÇÃO GEOMÉTRICA INSTANTÂNEA AO PASSAR O MOUSE
            bool is_card_hovered = (m_pos.x >= c_p0.x && m_pos.x <= c_p1.x &&
                                    m_pos.y >= c_p0.y && m_pos.y <= c_p1.y);

            if (is_card_hovered) {
                detected_hover_id = trk.id;
                detected_hover_key = trk.key;
                detected_hover_bpm = trk.bpm;
                detected_hover_title = trk.title;
                detected_hover_artist = trk.artist;
            }

            // Borda de Hover Neon com glow animado
            if (is_card_hovered || hovered_track_id == trk.id) {
                float glow_a = 0.55f + 0.45f * std::sin(radar_pulse_phase * 2.0f);
                // Halo difuso externo
                dl->AddRect(ImVec2(c_p0.x - 2.0f, c_p0.y - 2.0f), ImVec2(c_p1.x + 2.0f, c_p1.y + 2.0f),
                    IM_COL32(0, 200, 255, (int)(60.0f * glow_a)), 8.0f, 0, 4.0f);
                // Borda neon principal
                dl->AddRect(c_p0, c_p1, IM_COL32(0, 240, 255, (int)(200.0f + 55.0f * glow_a)), 6.0f, 0, 1.8f);
            }

            // 1. CAPA / THUMBNAIL (60x60)
            float thumb_size = 58.0f;
            ImVec2 thumb_p0(c_p0.x + 8.0f, c_p0.y + 8.0f);
            ImVec2 thumb_p1(thumb_p0.x + thumb_size, thumb_p0.y + thumb_size);

            GLuint tex_id = (GLuint)trk.gl_tex_id;
            if (tex_id == 0) {
                std::string rgba_p = trk.local_rgba_path;
                if (rgba_p.empty()) {
                    auto candidate = KuroAudio::getTracksDirectory() / ("track_" + trk.id + ".rgba");
                    std::error_code ec;
                    if (std::filesystem::exists(candidate, ec)) {
                        rgba_p = candidate.string();
                    }
                }
                if (!rgba_p.empty()) {
                    tex_id = KuroAudio::SpotifyDJService::UploadRGBATexture(rgba_p);
                    if (tex_id != 0) {
                        trk.gl_tex_id = (unsigned int)tex_id;
                        KuroAudio::DJLibraryManager::getInstance().setTrackTexture(trk.id, (unsigned int)tex_id);
                    } else {
                        trk.gl_tex_id = 0xFFFFFFFF;
                    }
                } else {
                    trk.gl_tex_id = 0xFFFFFFFF;
                }
            }

            if (tex_id != 0 && tex_id != 0xFFFFFFFF) {
                dl->AddImageRounded((ImTextureID)(intptr_t)tex_id, thumb_p0, thumb_p1, ImVec2(0,0), ImVec2(1,1), IM_COL32(255,255,255,255), 4.0f);
            } else {
                dl->AddRectFilled(thumb_p0, thumb_p1, IM_COL32(18, 25, 38, 255), 4.0f);
                dl->AddText(ImVec2(thumb_p0.x + 18.0f, thumb_p0.y + 20.0f), IM_COL32(0, 220, 255, 200), ICON_FA_MUSIC);
            }
            // Moldura da capa com a cor Camelot
            dl->AddRect(thumb_p0, thumb_p1, key_color, 4.0f, 0, 1.5f);

            // 2. TEXTOS (TÍTULO, ARTISTA, VERTENTE)
            float text_x = thumb_p1.x + 10.0f;
            float text_y = c_p0.y + 8.0f;

            // Título
            std::string disp_title = trk.title.substr(0, 26);
            dl->AddText(ImVec2(text_x, text_y), IM_COL32(255, 255, 255, 255), disp_title.c_str());

            // Artista
            std::string disp_art = trk.artist.substr(0, 24);
            dl->AddText(ImVec2(text_x, text_y + 16.0f), IM_COL32(170, 190, 210, 255), disp_art.c_str());

            // Badge de Vertente com coloração cirúrgica
            ImU32 vert_color = IM_COL32(190, 90, 255, 255); // Full-On (Roxo)
            if (trk.subgenre.find("Forest") != std::string::npos || trk.subgenre.find("Twilight") != std::string::npos) {
                vert_color = IM_COL32(40, 225, 130, 255); // Emerald Green
            } else if (trk.subgenre.find("Progressive") != std::string::npos) {
                vert_color = IM_COL32(255, 145, 40, 255); // Orange
            } else if (trk.subgenre.find("Tech") != std::string::npos) {
                vert_color = IM_COL32(0, 210, 255, 255); // Cyan
            }

            float art_w = ImGui::CalcTextSize(disp_art.c_str()).x;
            if (text_x + art_w + 12.0f < card_w - 70.0f) {
                std::string short_sub = trk.subgenre;
                if (short_sub.length() > 18) short_sub = short_sub.substr(0, 18);
                dl->AddText(ImVec2(text_x + art_w + 10.0f, text_y + 16.0f), vert_color, ("· " + short_sub).c_str());
            }

            // Badges Linha 3: BPM + Tom Camelot + Match Score + Tag de Origem
            char bpm_str[32]; snprintf(bpm_str, sizeof(bpm_str), "%.1f BPM", trk.bpm);
            dl->AddText(ImVec2(text_x, text_y + 34.0f), IM_COL32(255, 195, 30, 255), bpm_str);

            std::string camelot_badge = trk_ck.valid ? trk_ck.getCamelotCode() : trk.key;
            dl->AddText(ImVec2(text_x + 68.0f, text_y + 34.0f), key_color, camelot_badge.c_str());

            std::string match_badge = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeText(master_key_str, trk.key);
            ImU32 match_c = KuroAudio::DJHarmonicEngine::getCompatibilityBadgeColor(master_key_str, trk.key);
            dl->AddText(ImVec2(text_x + 104.0f, text_y + 34.0f), match_c, ("★ " + match_badge).c_str());

            if (trk.folder_id == "first_kuru" || trk.local_wav_path.find("FIRST KURU") != std::string::npos) {
                dl->AddText(ImVec2(text_x + 180.0f, text_y + 34.0f), IM_COL32(0, 240, 255, 230), "[📁 KURU]");
            }

            // PILL DE TOM NEON — revelada instantaneamente ao hover (sem clicar)
            // Flutua sobre a thumbnail mostrando o tom Camelot + nome musical real
            if (is_card_hovered && trk_ck.valid) {
                float pill_glow = 0.75f + 0.25f * std::sin(radar_pulse_phase * 2.5f);
                // Pill fundo + borda colorida na cor do setor Camelot
                ImVec2 pill_p0(c_p0.x + 4.0f, c_p0.y + card_h - 26.0f);
                // Tamanho dinâmico baseado no conteúdo: código + nome musical + BPM
                std::string key_name_str = camelot_badge;
                if (!trk.key.empty()) {
                    // Extrai nome musical (ex: "9B / G" → mostra "G")
                    size_t sl = trk.key.find('/');
                    if (sl != std::string::npos && sl + 2 < trk.key.size()) {
                        std::string music_name = trk.key.substr(sl + 2);
                        // Remove espaços extras
                        while (!music_name.empty() && music_name[0] == ' ') music_name.erase(0, 1);
                        if (!music_name.empty()) key_name_str = camelot_badge + " · " + music_name;
                    }
                }
                char pill_full[64];
                snprintf(pill_full, sizeof(pill_full), "♪ %s", key_name_str.c_str());
                ImVec2 pill_sz = ImGui::CalcTextSize(pill_full);
                ImVec2 pill_p1(pill_p0.x + pill_sz.x + 14.0f, pill_p0.y + 18.0f);

                // Halo difuso (glow)
                dl->AddRectFilled(ImVec2(pill_p0.x - 2.0f, pill_p0.y - 2.0f), ImVec2(pill_p1.x + 2.0f, pill_p1.y + 2.0f),
                    (key_color & 0x00FFFFFF) | ((int)(50.0f * pill_glow) << 24), 10.0f);
                // Fundo sólido escuro
                dl->AddRectFilled(pill_p0, pill_p1, IM_COL32(8, 12, 20, 240), 8.0f);
                // Borda com cor Camelot pulsante
                dl->AddRect(pill_p0, pill_p1,
                    (key_color & 0x00FFFFFF) | ((int)(180.0f + 75.0f * pill_glow) << 24), 8.0f, 0, 1.5f);
                // Texto da pill
                dl->AddText(ImVec2(pill_p0.x + 7.0f, pill_p0.y + 3.0f),
                    (key_color & 0x00FFFFFF) | IM_COL32(0, 0, 0, (int)(220.0f + 35.0f * pill_glow)), pill_full);

                // Badge de compatibilidade harmônica ao lado (pill secundária)
                ImVec2 match_pill_p0(pill_p1.x + 5.0f, pill_p0.y);
                ImVec2 match_sz = ImGui::CalcTextSize(match_badge.c_str());
                ImVec2 match_pill_p1(match_pill_p0.x + match_sz.x + 12.0f, pill_p1.y);
                if (match_pill_p1.x < c_p1.x - 4.0f) {
                    dl->AddRectFilled(match_pill_p0, match_pill_p1, IM_COL32(8, 12, 20, 220), 8.0f);
                    dl->AddRect(match_pill_p0, match_pill_p1, (match_c & 0x00FFFFFF) | 0xD0000000, 8.0f, 0, 1.5f);
                    dl->AddText(ImVec2(match_pill_p0.x + 6.0f, match_pill_p0.y + 3.0f), match_c, match_badge.c_str());
                }
            }

            // 3. MICRO-WAVEFORM DINÂMICA COM AGULHA
            float wave_w = card_w - (text_x - c_p0.x) - 10.0f;
            float wave_h = 10.0f;
            ImVec2 w_p0(text_x, text_y + 50.0f);
            ImVec2 w_p1(w_p0.x + wave_w, w_p0.y + wave_h);
            dl->AddRectFilled(w_p0, w_p1, IM_COL32(14, 20, 32, 240), 2.0f);
            
            // Simulação de transientes da forma de onda
            int segments = (int)(wave_w / 3.0f);
            for (int s = 0; s < segments; s++) {
                float sx = w_p0.x + s * 3.0f;
                float norm_s = (float)s / (float)segments;
                float h_factor = 0.3f + 0.7f * std::sin(norm_s * 3.1415f * 2.0f + 1.2f);
                if (norm_s > 0.45f && norm_s < 0.75f) h_factor = 0.95f; // Drop
                float sh = wave_h * h_factor * 0.5f;
                dl->AddLine(ImVec2(sx, w_p0.y + wave_h*0.5f - sh), ImVec2(sx, w_p0.y + wave_h*0.5f + sh), key_color, 1.5f);
            }

            // Agulha de tempo em hover
            if (is_card_hovered) {
                float mouse_x = ImGui::GetIO().MousePos.x;
                if (mouse_x >= w_p0.x && mouse_x <= w_p1.x) {
                    dl->AddLine(ImVec2(mouse_x, w_p0.y - 2.0f), ImVec2(mouse_x, w_p1.y + 2.0f), IM_COL32(255, 255, 255, 255), 2.0f);
                }
            }

            // 4. BOTÕES GIGANTES DE CARGA: [ LOAD D1 ] e [ LOAD D2 ]
            ImGui::SetCursorPos(ImVec2(8.0f, 72.0f));
            float btn_w = (card_w - 24.0f) * 0.5f;
            float btn_h = 24.0f;

            // Botão LOAD D1 (Ciano)
            bool d1_playing = engine.decks[0].is_playing.load();
            if (d1_playing) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.35f, 0.45f, 0.85f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.65f, 0.75f, 0.95f));
            }
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.85f, 0.95f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            char b1_label[32];
            snprintf(b1_label, sizeof(b1_label), d1_playing ? "LOAD D1 [NO AR]" : ICON_FA_PLAY " LOAD D1");
            if (ImGui::Button(b1_label, ImVec2(btn_w, btn_h))) {
                KuroAudio::DJLibraryTrack copy_trk = trk;
                spotify_service.loadLibraryTrack(copy_trk, 0, engine);
                toast_message = "Carregado no Deck 1: " + trk.title;
                toast_timer = 2.5f;
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine(0, 8.0f);

            // Botão LOAD D2 (Âmbar / Laranja)
            bool d2_playing = engine.decks[1].is_playing.load();
            if (d2_playing) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.25f, 0.05f, 0.85f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.50f, 0.05f, 0.95f));
            }
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.00f, 0.65f, 0.10f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            char b2_label[32];
            snprintf(b2_label, sizeof(b2_label), d2_playing ? "LOAD D2 [NO AR]" : ICON_FA_PLAY " LOAD D2");
            if (ImGui::Button(b2_label, ImVec2(btn_w, btn_h))) {
                KuroAudio::DJLibraryTrack copy_trk = trk;
                spotify_service.loadLibraryTrack(copy_trk, 1, engine);
                toast_message = "Carregado no Deck 2: " + trk.title;
                toast_timer = 2.5f;
            }
            ImGui::PopStyleColor(3);

            // Menu de Contexto com Botão Direito no Card
            if (ImGui::BeginPopupContextWindow("TrackCardCtxMenu", ImGuiPopupFlags_MouseButtonRight)) {
                auto& lib_mgr = KuroAudio::DJLibraryManager::getInstance();
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.85f, 1.0f), "%s", trk.title.c_str());
                ImGui::Separator();

                if (ImGui::BeginMenu(ICON_FA_FOLDER " 📁 Mover para Pasta / Crate")) {
                    for (auto& f : lib_mgr.getFolders()) {
                        if (f.id == "all") continue;
                        if (ImGui::MenuItem(f.name.c_str(), nullptr, (trk.folder_id == f.id))) {
                            lib_mgr.moveTrackToFolder(trk.id, f.id);
                            toast_message = "Movida para: " + f.name;
                            toast_timer = 2.5f;
                        }
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(ICON_FA_TAGS " 🏷 Definir Vertente Musical")) {
                    const char* subgenres[] = { "Progressive Psytrance", "Psytrance / Full-On", "Forest & Twilight", "Tech House & Minimal" };
                    for (const char* sg : subgenres) {
                        if (ImGui::MenuItem(sg, nullptr, (trk.subgenre == sg))) {
                            lib_mgr.setTrackSubgenre(trk.id, sg);
                            toast_message = "Vertente alterada para: " + std::string(sg);
                            toast_timer = 2.5f;
                        }
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem(ICON_FA_MICROCHIP " 🔬 Reanálise Cirúrgica Rekordbox")) {
                    surgical_result = KuroAudio::DJSurgicalAudioEngine::reanalyzeTrack(trk.id);
                    if (surgical_result.exact_bpm > 20.0) {
                        trk.bpm = surgical_result.exact_bpm;
                        trk.key = surgical_result.key;
                        lib_mgr.updateTrackMetadata(trk.id, trk.title, trk.artist, trk.bpm, trk.key, trk.subgenre);
                    }
                    show_surgical_results_dialog = true;
                }

                if (ImGui::MenuItem(ICON_FA_SATELLITE_DISH " 📡 Buscar Fontes / Masters Alternativos")) {
                    request_open_sources_track = true;
                    request_track_id = trk.id;
                    request_track_title = trk.title;
                    request_track_artist = trk.artist;
                }

                if (ImGui::MenuItem(ICON_FA_PLUS " Encaixar no Setlist")) {
                    KuroAudio::DJSetlistManager::getInstance().addTrackToActiveSetlist(trk, 1, "Encaixe harmônico via gaveta");
                    toast_message = "Faixa adicionada ao setlist!";
                    toast_timer = 2.5f;
                }

                if (ImGui::MenuItem(ICON_FA_PEN " ✏ Editar Metadados")) {
                    edit_track_copy = trk;
                    show_edit_metadata_modal = true;
                }

                ImGui::Separator();
                if (ImGui::MenuItem(ICON_FA_TRASH " 🗑 Excluir da Biblioteca")) {
                    delete_track_id = trk.id;
                    show_confirm_delete_modal = true;
                }

                ImGui::EndPopup();
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }

        // Diálogos Modais Auxiliares (Edição, Exclusão, Análise Cirúrgica)
        void renderAuxiliaryDialogs(KuroAudio::SpotifyDJService& spotify_service) {
            auto& lib_mgr = KuroAudio::DJLibraryManager::getInstance();

            // 1. Modal de Edição de Metadados
            if (show_edit_metadata_modal) {
                ImGui::OpenPopup("Editar Metadados da Música##SideDrawerEdit");
                if (ImGui::BeginPopupModal("Editar Metadados da Música##SideDrawerEdit", &show_edit_metadata_modal, ImGuiWindowFlags_AlwaysAutoResize)) {
                    static char e_title[128];
                    static char e_artist[128];
                    static char e_genre[64];
                    static double e_bpm = 138.0;
                    static char e_key[32];

                    static std::string last_id = "";
                    if (last_id != edit_track_copy.id) {
                        last_id = edit_track_copy.id;
                        strncpy(e_title, edit_track_copy.title.c_str(), sizeof(e_title));
                        strncpy(e_artist, edit_track_copy.artist.c_str(), sizeof(e_artist));
                        strncpy(e_genre, edit_track_copy.subgenre.c_str(), sizeof(e_genre));
                        e_bpm = edit_track_copy.bpm;
                        strncpy(e_key, edit_track_copy.key.c_str(), sizeof(e_key));
                    }

                    ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.85f, 1.0f), "Edição Cirúrgica de Metadados:");
                    ImGui::SameLine(ImGui::GetWindowWidth() - 115.0f);
                    std::string m_dock_btn1 = std::string(ICON_FA_ARROW_RIGHT_ARROW_LEFT) + (dock_left ? " ESQ" : " DIR");
                    if (ImGui::SmallButton(m_dock_btn1.c_str())) {
                        dock_left = !dock_left;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Alternar ancoragem da Gaveta (Esquerda ⇄ Direita)");
                    ImGui::Separator();

                    ImGui::InputText("Título", e_title, sizeof(e_title));
                    ImGui::InputText("Artista", e_artist, sizeof(e_artist));
                    ImGui::InputText("Vertente / Gênero", e_genre, sizeof(e_genre));
                    ImGui::InputDouble("BPM Exato", &e_bpm, 0.1, 1.0, "%.2f");
                    ImGui::InputText("Tom / Camelot", e_key, sizeof(e_key));

                    ImGui::Separator();
                    if (ImGui::Button("SALVAR ALTERAÇÕES", ImVec2(180, 30))) {
                        lib_mgr.updateTrackMetadata(edit_track_copy.id, e_title, e_artist, e_bpm, e_key, e_genre);
                        toast_message = "Metadados atualizados com sucesso!";
                        toast_timer = 2.5f;
                        show_edit_metadata_modal = false;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("CANCELAR", ImVec2(100, 30))) {
                        show_edit_metadata_modal = false;
                    }
                    ImGui::EndPopup();
                }
            }

            // 2. Modal de Confirmação de Exclusão
            if (show_confirm_delete_modal) {
                ImGui::OpenPopup("Excluir Música da Biblioteca##SideDrawerDel");
                if (ImGui::BeginPopupModal("Excluir Música da Biblioteca##SideDrawerDel", &show_confirm_delete_modal, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Tem certeza que deseja remover esta música?");
                    ImGui::Checkbox("Excluir também o arquivo de áudio físico do disco rígido", &delete_disk_file_flag);

                    ImGui::Separator();
                    if (ImGui::Button("SIM, EXCLUIR", ImVec2(140, 28))) {
                        lib_mgr.deleteTrack(delete_track_id, delete_disk_file_flag);
                        toast_message = "Música removida da biblioteca.";
                        toast_timer = 2.5f;
                        show_confirm_delete_modal = false;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("CANCELAR", ImVec2(100, 28))) {
                        show_confirm_delete_modal = false;
                    }
                    ImGui::EndPopup();
                }
            }

            // 3. Modal de Resultado da Análise Cirúrgica
            if (show_surgical_results_dialog) {
                ImGui::OpenPopup("Resultado da Reanálise Cirúrgica Rekordbox##SideDrawerSurg");
                if (ImGui::BeginPopupModal("Resultado da Reanálise Cirúrgica Rekordbox##SideDrawerSurg", &show_surgical_results_dialog, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_CHECK " Reanálise Concluída com Sucesso!");
                    ImGui::SameLine(ImGui::GetWindowWidth() - 115.0f);
                    std::string m_dock_btn2 = std::string(ICON_FA_ARROW_RIGHT_ARROW_LEFT) + (dock_left ? " ESQ" : " DIR");
                    if (ImGui::SmallButton(m_dock_btn2.c_str())) {
                        dock_left = !dock_left;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Alternar ancoragem da Gaveta (Esquerda ⇄ Direita)");
                    ImGui::Separator();
                    ImGui::Text("Faixa: %s - %s", surgical_result.title.c_str(), surgical_result.artist.c_str());
                    ImGui::Text("BPM Cirúrgico: %.2f", surgical_result.exact_bpm);
                    ImGui::Text("Tom Detectado: %s", surgical_result.key.c_str());
                    ImGui::Text("Marco 1.1 da Beatgrid: Frame %.0f (Downbeat sincronizado)", surgical_result.downbeat_frame);
                    ImGui::Text("Nível de Energia: %.1f / 10", surgical_result.dynamic_energy_rating);

                    ImGui::Separator();
                    if (ImGui::Button("FECHAR", ImVec2(120, 26))) {
                        show_surgical_results_dialog = false;
                    }
                    ImGui::EndPopup();
                }
            }
        }
    };

} // namespace KuroUI
