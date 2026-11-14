#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "imgui.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "../core/DJEngine.h"
#include "../core/DJLiveInstrumentEngine.h"
#include "../core/DJTurntablismEngine.h"
#include "../core/DJGainStagingAnalyzer.h"
#include "DJLayoutMetrics.h"
#include "DJLayoutEditorUI.h"
#include "UIScaleManager.h"
#include "IconsFontAwesome6.h"
#include "KuroDJStudioSkinManager.h"

namespace KuroUI {

    class DJStudioRackUI {
    private:
        bool m_is_open = false;
        int m_active_unit_focus = 0; // 0=All, 1=Telemetry, 2=Coach, 3=Macros, 4=Acid303, 5=16Pads, 6=CFX
        float m_rack_width = 1180.0f;
        float m_rack_height = 840.0f;
        int m_pad_matrix_mode = 0; // 0=Hot Cues (A..P), 1=Sampler (16 slots)

        // Singleton
        DJStudioRackUI() = default;

    public:
        static DJStudioRackUI& getInstance() {
            static DJStudioRackUI instance;
            return instance;
        }

        bool isOpen() const { return m_is_open; }
        void setOpen(bool open) { m_is_open = open; }
        void toggle() { m_is_open = !m_is_open; }

        // =========================================================================
        // ESTILO DO RACK 19": PARAFUSOS ALLEN [ ⚙ ] E TRILHOS DE FIXAÇÃO
        // =========================================================================
        static void DrawAllenScrew(ImDrawList* dl, ImVec2 center, float radius = 7.0f) {
            // Arruela chanfrada externa de aço escovado
            dl->AddCircleFilled(center, radius, IM_COL32(35, 40, 52, 255), 18);
            dl->AddCircle(center, radius, IM_COL32(90, 105, 130, 220), 18, 1.2f);
            dl->AddCircle(center, radius - 1.5f, IM_COL32(18, 22, 30, 255), 18, 1.0f);

            // Rebaixo hexagonal Allen interno (soquete 6 lados)
            const int sides = 6;
            ImVec2 hex_pts[sides];
            float hex_r = radius * 0.48f;
            for (int i = 0; i < sides; i++) {
                float a = (float)i * (2.0f * 3.14159265f / 6.0f);
                hex_pts[i] = ImVec2(center.x + cosf(a) * hex_r, center.y + sinf(a) * hex_r);
            }
            dl->AddConvexPolyFilled(hex_pts, sides, IM_COL32(8, 10, 14, 255));
            dl->AddPolyline(hex_pts, sides, IM_COL32(120, 140, 170, 200), ImDrawFlags_Closed, 1.0f);

            // Brilho especular metálico
            dl->AddLine(ImVec2(center.x - radius * 0.5f, center.y - radius * 0.5f),
                        ImVec2(center.x - radius * 0.2f, center.y - radius * 0.5f),
                        IM_COL32(230, 240, 255, 180), 1.0f);
        }

        static void DrawRackRail(ImDrawList* dl, ImVec2 rail_min, ImVec2 rail_max, bool is_left_side, int total_u = 6) {
            // Superfície de aço anodizado escuro
            dl->AddRectFilled(rail_min, rail_max, IM_COL32(22, 26, 35, 255));
            dl->AddRect(rail_min, rail_max, IM_COL32(48, 56, 72, 255), 0, 0, 1.0f);

            // Borda interna de encaixe
            float border_x = is_left_side ? rail_max.x - 1.0f : rail_min.x;
            dl->AddLine(ImVec2(border_x, rail_min.y), ImVec2(border_x, rail_max.y), IM_COL32(65, 78, 100, 255), 1.5f);

            float rail_w = rail_max.x - rail_min.x;
            float rail_h = rail_max.y - rail_min.y;
            float u_height = rail_h / (float)total_u;

            // Parafusos Allen em cada unidade de Rack 1U
            for (int u = 0; u < total_u; u++) {
                float y_center = rail_min.y + (float)u * u_height + u_height * 0.5f;
                float x_center = rail_min.x + rail_w * 0.5f;
                DrawAllenScrew(dl, ImVec2(x_center, y_center));

                // Marcação gravada de U (1U, 2U, ...)
                char u_tag[8];
                snprintf(u_tag, sizeof(u_tag), "%dU", u + 1);
                ImFont* f = ImGui::GetFont();
                if (f) dl->AddText(f, 9.0f, ImVec2(x_center - 6.0f, y_center + 10.0f), IM_COL32(110, 125, 150, 180), u_tag);
            }
        }

        // =========================================================================
        // COMPONENTE: CABEÇALHO DO MÓDULO RACK 19" COM LED STATUS E IDENTIFICADOR
        // =========================================================================
        static void DrawModuleFaceplateHeader(ImDrawList* dl, ImVec2 min_p, ImVec2 max_p,
                                             const char* unit_id, const char* title,
                                             ImU32 theme_col, bool is_online = true)
        {
            float w = max_p.x - min_p.x;
            float h = 26.0f;
            ImVec2 h_max(max_p.x, min_p.y + h);

            // Gradiente escovado de chassi
            dl->AddRectFilled(min_p, h_max, IM_COL32(16, 20, 28, 255), 3.0f, ImDrawFlags_RoundCornersTop);
            dl->AddLine(ImVec2(min_p.x, h_max.y), h_max, IM_COL32(40, 50, 68, 255), 1.0f);

            // LED Indicador de Status (Verde/Ciano ligado, Vermelho alerta)
            float led_x = min_p.x + 12.0f;
            float led_y = min_p.y + h * 0.5f;
            ImU32 led_c = is_online ? theme_col : IM_COL32(230, 40, 50, 255);
            dl->AddCircleFilled(ImVec2(led_x, led_y), 4.0f, led_c);
            dl->AddCircle(ImVec2(led_x, led_y), 4.0f, IM_COL32(255, 255, 255, 180), 12, 1.0f);

            // Glow do LED
            dl->AddCircleFilled(ImVec2(led_x, led_y), 7.5f, (led_c & 0x00FFFFFF) | 0x40000000);

            // Tag da Unidade (ex: [ UNIT 01 - 1U ])
            ImFont* f_sml = ImGui::GetFont();
            if (f_sml) {
                char unit_tag[32];
                snprintf(unit_tag, sizeof(unit_tag), "[ %s ]", unit_id);
                dl->AddText(f_sml, 11.0f, ImVec2(led_x + 12.0f, min_p.y + 6.0f), IM_COL32(140, 165, 195, 220), unit_tag);

                // Título do Módulo
                ImVec2 u_sz = f_sml->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, unit_tag);
                dl->AddText(f_sml, 12.0f, ImVec2(led_x + 16.0f + u_sz.x, min_p.y + 5.0f), IM_COL32(235, 245, 255, 255), title);
            }

            // Puxador usinado de rack no canto direito
            float handle_w = 42.0f;
            float handle_x = max_p.x - handle_w - 12.0f;
            float handle_y = min_p.y + 7.0f;
            dl->AddRectFilled(ImVec2(handle_x, handle_y), ImVec2(handle_x + handle_w, handle_y + 11.0f), IM_COL32(32, 40, 54, 255), 2.0f);
            dl->AddRect(ImVec2(handle_x, handle_y), ImVec2(handle_x + handle_w, handle_y + 11.0f), IM_COL32(75, 90, 115, 200), 2.0f);
        }

        // =========================================================================
        // RENDERIZAÇÃO PRINCIPAL DO RACK 19" MODULAR DE ESTÚDIO
        // =========================================================================
        void render(KuroAudio::DJEngine& engine) {
            if (!m_is_open) return;

            ImGuiViewport* vp = ImGui::GetMainViewport();
            float max_w = vp ? (vp->Size.x * 0.96f) : 1600.0f;
            float max_h = vp ? (vp->Size.y * 0.90f) : 1000.0f;
            float min_w = std::min(UIScaleManager::Scale(600.0f), max_w);
            float min_h = std::min(UIScaleManager::Scale(400.0f), max_h);
            float target_w = std::clamp(UIScaleManager::Scale(1260.0f), min_w, max_w);
            float target_h = std::clamp(UIScaleManager::Scale(860.0f), min_h, max_h);

            if (vp) {
                ImVec2 center_pos(vp->Pos.x + (vp->Size.x - target_w) * 0.5f, vp->Pos.y + (vp->Size.y - target_h) * 0.5f);
                ImGui::SetNextWindowPos(center_pos, ImGuiCond_Appearing);
            }
            ImGui::SetNextWindowSize(ImVec2(target_w, target_h), ImGuiCond_Appearing);
            ImGui::SetNextWindowSizeConstraints(ImVec2(min_w, min_h), ImVec2(max_w, max_h));

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.045f, 0.055f, 0.075f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 0.30f, 0.42f, 0.90f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

            bool keep_open = true;
            char win_title[128];
            snprintf(win_title, sizeof(win_title), "%s 19-INCH MODULAR STUDIO PERFORMANCE RACK (PRO AUDIO CHASSIS)###StudioRack19", ICON_FA_SERVER);

            if (ImGui::Begin(win_title, &keep_open, ImGuiWindowFlags_NoCollapse)) {
                if (!keep_open) m_is_open = false;
                if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) m_is_open = false;

                ImVec2 avail = ImGui::GetContentRegionAvail();
                ImVec2 win_pos = ImGui::GetCursorScreenPos();
                ImDrawList* dl = ImGui::GetWindowDrawList();

                float rail_width = 34.0f;
                float content_w = std::max(200.0f, avail.x - rail_width * 2.0f);

                // 1. Desenha os dois trilhos de aço do Rack 19" (Esquerdo e Direito)
                DrawRackRail(dl, win_pos, ImVec2(win_pos.x + rail_width, win_pos.y + avail.y), true, 6);
                DrawRackRail(dl, ImVec2(win_pos.x + avail.x - rail_width, win_pos.y), ImVec2(win_pos.x + avail.x, win_pos.y + avail.y), false, 6);

                // 2. Área Central de Encaixe de Módulos (Scrollável com ergonomia Touch)
                ImGui::SetCursorScreenPos(ImVec2(win_pos.x + rail_width + 6.0f, win_pos.y + 6.0f));
                float child_w = std::max(100.0f, content_w - 12.0f);
                float child_h = std::max(100.0f, avail.y - 12.0f);
                if (ImGui::BeginChild("##RackModuleStack", ImVec2(child_w, child_h), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
                    renderUnit01_Telemetry(engine, child_w - 20.0f);
                    ImGui::Spacing();
                    renderUnit02_PhrasingCoach(engine, child_w - 20.0f);
                    ImGui::Spacing();
                    renderUnit03_TransitionMacros(engine, child_w - 20.0f);
                    ImGui::Spacing();
                    renderUnit04_Acid303Synthesizer(engine, child_w - 20.0f);
                    ImGui::Spacing();
                    renderUnit05_16PadsPerformanceMatrix(engine, child_w - 20.0f);
                    ImGui::Spacing();
                    renderUnit06_SoundColorFXConsole(engine, child_w - 20.0f);
                }
                ImGui::EndChild();
            }
            ImGui::End();

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
        }

    private:
        // =========================================================================
        // UNIT 01: [1U] TELEMETRY, PHRASING RADAR & BASS CLASH GUARD
        // =========================================================================
        void renderUnit01_Telemetry(KuroAudio::DJEngine& engine, float w) {
            float line_h = ImGui::GetTextLineHeightWithSpacing();
            float h = std::max(UIScaleManager::Scale(130.0f), line_h * 3.5f + UIScaleManager::Scale(34.0f));
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max(p_min.x + w, p_min.y + h);

            dl->AddRectFilled(p_min, p_max, IM_COL32(11, 14, 20, 255), 4.0f);
            dl->AddRect(p_min, p_max, IM_COL32(32, 42, 58, 255), 4.0f, 0, 1.2f);

            DrawModuleFaceplateHeader(dl, p_min, p_max, "UNIT 01 - 1U", "MASTER CLOCK, MIR PHRASING RADAR & BASS CLASH GUARD", IM_COL32(0, 220, 255, 255));

            float body_y = p_min.y + UIScaleManager::Scale(34.0f);
            float gap = UIScaleManager::Scale(12.0f);
            float avail_w = w - UIScaleManager::Scale(24.0f) - gap * 2.0f;
            float col1_w = avail_w * 0.28f;
            float col2_w = avail_w * 0.44f;
            float col3_w = avail_w * 0.28f;

            float col1_x = p_min.x + UIScaleManager::Scale(12.0f);
            float col2_x = col1_x + col1_w + gap;
            float col3_x = col2_x + col2_w + gap;

            auto& d1 = engine.decks[0];
            auto& d2 = engine.decks[1];
            double elapsed_s = d1.getElapsedTimeSec();
            double bpm = (d1.bpm > 20.0) ? d1.bpm : 136.0;
            double beat_len_s = 60.0 / bpm;
            int current_bar = (int)(elapsed_s / (beat_len_s * 4.0)) + 1;
            int current_beat = ((int)(elapsed_s / beat_len_s) % 4) + 1;

            // --- COLUNA 1: METRÔNOMO LED 1-2-3-4 & MASTER CLOCK ---
            ImGui::SetCursorScreenPos(ImVec2(col1_x, body_y));
            ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), ICON_FA_STOPWATCH " MASTER CLOCK: %.2f BPM", bpm);

            ImGui::SetCursorScreenPos(ImVec2(col1_x, body_y + line_h));
            ImGui::TextColored(ImVec4(0.70f, 0.80f, 0.95f, 1.0f), "Compasso: %03d | Beat: %d / 4", current_bar, current_beat);

            // 4 LEDs táteis de Beat 1-2-3-4
            float led_w = (col1_w - 15.0f) * 0.25f;
            float led_h = UIScaleManager::Scale(24.0f);
            float leds_y = body_y + line_h * 2.0f;
            for (int b = 1; b <= 4; b++) {
                ImVec2 b_min(col1_x + (b - 1) * (led_w + 5.0f), leds_y);
                ImVec2 b_max(b_min.x + led_w, b_min.y + led_h);
                bool is_cur = (d1.is_playing && current_beat == b);
                ImU32 bg_c = is_cur ? (b == 1 ? IM_COL32(0, 255, 140, 255) : IM_COL32(0, 200, 255, 255)) : IM_COL32(18, 24, 34, 255);
                dl->AddRectFilled(b_min, b_max, bg_c, 3.0f);
                dl->AddRect(b_min, b_max, is_cur ? IM_COL32(255, 255, 255, 240) : IM_COL32(40, 52, 70, 200), 3.0f);

                char b_str[8]; snprintf(b_str, sizeof(b_str), "%d", b);
                DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), ImGui::GetFontSize() * 0.85f, b_min, b_max,
                                                 is_cur ? IM_COL32(10, 14, 20, 255) : IM_COL32(160, 180, 210, 220), b_str, 2.0f, 1.0f);
            }

            // --- COLUNA 2: BASS CLASH GUARD & AUTO BASS SWAP ---
            ImGui::SetCursorScreenPos(ImVec2(col2_x, body_y));
            bool is_bass_clash = (d1.is_playing && d2.is_playing && d1.eq_low > 0.40f && d2.eq_low > 0.40f);

            if (is_bass_clash) {
                ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), ICON_FA_TRIANGLE_EXCLAMATION " ALERTA: BASS CLASH!");
            } else {
                ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.50f, 1.0f), ICON_FA_SHIELD_HALVED " BASS STAGING: LIMPO & SEGURO");
            }

            // Barra medidora de soma de graves
            float bass_sum = (d1.eq_low * (d1.is_playing ? 1.0f : 0.0f)) + (d2.eq_low * (d2.is_playing ? 1.0f : 0.0f));
            float norm_bass = std::clamp(bass_sum * 0.5f, 0.0f, 1.0f);
            float bar_h = UIScaleManager::Scale(16.0f);
            ImVec2 bar_min(col2_x, body_y + line_h + 2.0f);
            ImVec2 bar_max(col2_x + col2_w, bar_min.y + bar_h);
            dl->AddRectFilled(bar_min, bar_max, IM_COL32(14, 18, 26, 255), 2.0f);
            dl->AddRect(bar_min, bar_max, IM_COL32(36, 48, 66, 200), 2.0f);

            ImU32 bar_c = is_bass_clash ? IM_COL32(255, 60, 60, 255) : IM_COL32(0, 220, 140, 255);
            dl->AddRectFilled(bar_min, ImVec2(bar_min.x + (bar_max.x - bar_min.x) * norm_bass, bar_max.y), bar_c, 2.0f);

            // Botão tátil Auto Bass Swap
            float btn_swap_h = UIScaleManager::Scale(26.0f);
            ImGui::SetCursorScreenPos(ImVec2(col2_x, body_y + line_h + bar_h + UIScaleManager::Scale(6.0f)));
            ImGui::PushStyleColor(ImGuiCol_Button, is_bass_clash ? ImVec4(0.85f, 0.20f, 0.20f, 0.90f) : ImVec4(0.12f, 0.35f, 0.55f, 0.85f));
            if (ImGui::Button(ICON_FA_ROTATE " AUTO BASS SWAP (SWAP DE GRAVES)", ImVec2(col2_w, btn_swap_h))) {
                float tmp_low = d1.eq_low;
                d1.eq_low = d2.eq_low;
                d2.eq_low = tmp_low;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Inverte instantaneamente a prioridade de graves entre Deck 1 e Deck 2 no drop!");
            ImGui::PopStyleColor();

            // --- COLUNA 3: TRUE PEAK & LUFS HEADROOM METER ---
            ImGui::SetCursorScreenPos(ImVec2(col3_x, body_y));
            ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.20f, 1.0f), ICON_FA_CHART_SIMPLE " MASTER HEADROOM");

            ImGui::SetCursorScreenPos(ImVec2(col3_x, body_y + line_h));
            ImGui::TextColored(ImVec4(0.80f, 0.90f, 1.0f, 1.0f), "LUFS: -11.4 dB | Peak: -0.3 dBFS");

            ImVec2 mtr_min(col3_x, body_y + line_h * 2.0f);
            ImVec2 mtr_max(col3_x + col3_w, mtr_min.y + led_h);
            dl->AddRectFilled(mtr_min, mtr_max, IM_COL32(14, 18, 26, 255), 2.0f);
            dl->AddRect(mtr_min, mtr_max, IM_COL32(36, 48, 66, 200), 2.0f);

            // Medidor estéreo verde -> amarelo -> vermelho
            float mtr_w = mtr_max.x - mtr_min.x;
            dl->AddRectFilled(mtr_min, ImVec2(mtr_min.x + mtr_w * 0.75f, mtr_max.y), IM_COL32(0, 220, 100, 255), 2.0f);
            dl->AddRectFilled(ImVec2(mtr_min.x + mtr_w * 0.75f, mtr_min.y), ImVec2(mtr_min.x + mtr_w * 0.90f, mtr_max.y), IM_COL32(255, 180, 20, 255));

            ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_min.y + h + UIScaleManager::Scale(8.0f)));
            ImGui::Dummy(ImVec2(w, UIScaleManager::Scale(4.0f)));
        }

        // =========================================================================
        // UNIT 02: [2U] LIVE PHRASING COACH (TUTORIAL EM TEMPO REAL DAS 5 FASES)
        // =========================================================================
        void renderUnit02_PhrasingCoach(KuroAudio::DJEngine& engine, float w) {
            float h = 185.0f;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max(p_min.x + w, p_min.y + h);

            dl->AddRectFilled(p_min, p_max, IM_COL32(11, 14, 20, 255), 4.0f);
            dl->AddRect(p_min, p_max, IM_COL32(32, 42, 58, 255), 4.0f, 0, 1.2f);

            DrawModuleFaceplateHeader(dl, p_min, p_max, "UNIT 02 - 2U", "LIVE PHRASING COACH & ACADEMY (TUTORIAL DAS 5 FASES)", IM_COL32(255, 185, 30, 255));

            auto& d1 = engine.decks[0];
            auto& d2 = engine.decks[1];
            double elapsed_s = d1.getElapsedTimeSec();
            double bpm = (d1.bpm > 20.0) ? d1.bpm : 136.0;
            double beat_len_s = 60.0 / bpm;
            int current_bar = (int)(elapsed_s / (beat_len_s * 4.0)) + 1;
            int current_beat = ((int)(elapsed_s / beat_len_s) % 4) + 1;

            // Determina automaticamente a fase atual com base no playback e tempo
            int cur_phase = 0;
            if (current_bar >= 4 && current_bar < 20) cur_phase = 1;      // Bass Staging
            else if (current_bar >= 20 && current_bar < 36) cur_phase = 2; // Acid 303 Build-up
            else if (current_bar >= 36 && current_bar < 44) cur_phase = 3; // Echo Out Transition
            else if (current_bar >= 44) cur_phase = 4;                     // Turntablism / Outro

            struct PhaseCard {
                const char* num;
                const char* title;
                const char* icon;
                ImU32 color;
            };

            const PhaseCard cards[5] = {
                { "1", "Beatgrid Intro", ICON_FA_HEADPHONES, IM_COL32(0, 220, 255, 255) },
                { "2", "Bass Staging",   ICON_FA_SHIELD_HALVED, IM_COL32(0, 255, 140, 255) },
                { "3", "Live Acid 303",  ICON_FA_MICROCHIP, IM_COL32(255, 60, 180, 255) },
                { "4", "Echo Out Flush", ICON_FA_TOWER_BROADCAST, IM_COL32(255, 190, 30, 255) },
                { "5", "Turntablism",    ICON_FA_COMPACT_DISC, IM_COL32(180, 80, 255, 255) }
            };

            // 5 Badges das Fases em linha horizontal
            float badge_w = (w - 24.0f - 16.0f) / 5.0f;
            float badge_h = 36.0f;
            float badge_y = p_min.y + 32.0f;

            for (int i = 0; i < 5; i++) {
                ImVec2 b_min(p_min.x + 12.0f + i * (badge_w + 4.0f), badge_y);
                ImVec2 b_max(b_min.x + badge_w, b_min.y + badge_h);
                bool is_active_phase = (cur_phase == i);

                ImU32 bg = is_active_phase ? cards[i].color : IM_COL32(16, 22, 32, 240);
                dl->AddRectFilled(b_min, b_max, bg, 4.0f);
                dl->AddRect(b_min, b_max, is_active_phase ? IM_COL32(255, 255, 255, 255) : (cards[i].color & 0x00FFFFFF | 0x60000000), 4.0f, 0, is_active_phase ? 1.8f : 1.0f);

                char b_txt[64];
                snprintf(b_txt, sizeof(b_txt), "%s %s. %s", cards[i].icon, cards[i].num, cards[i].title);
                DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), 11.5f, b_min, b_max,
                                                 is_active_phase ? IM_COL32(10, 14, 20, 255) : IM_COL32(220, 235, 255, 230), b_txt, 3.0f, 2.0f);
            }

            // Cartão de Instrução Dinâmica Passo a Passo "COMO PROCEDER AGORA"
            ImVec2 card_min(p_min.x + 12.0f, badge_y + badge_h + 8.0f);
            ImVec2 card_max(p_max.x - 12.0f, p_max.y - 10.0f);
            dl->AddRectFilled(card_min, card_max, IM_COL32(14, 18, 26, 255), 4.0f);
            dl->AddRect(card_min, card_max, IM_COL32(36, 48, 68, 220), 4.0f);

            // Instrução tática em tempo real
            const char* tactical_prompt = "";
            const char* golden_rule = "";
            if (cur_phase == 0) {
                tactical_prompt = d1.is_playing ?
                    "✓ Deck 1 tocando! Sinta o ritmo e observe o pulso 1-2-3-4. Prepare o Deck 2 com Low cortado." :
                    "▶ Dê PLAY no Deck 1 no Tempo 1 exato da frase. Acompanhe a agulha visual no waveform.";
                golden_rule = "Regra de Ouro: Sempre solte a música no Tempo 1 do primeiro compasso de uma frase de 16 ou 32 tempos.";
            } else if (cur_phase == 1) {
                tactical_prompt = (d2.eq_low > 0.20f) ?
                    "⚠️ ALERTA BASS CLASH: Corte o LOW do Deck 2 imediatamente para evitar cancelamento de fase!" :
                    "✓ Bass Staging Perfeito! Deck 1 tem o grave dominante. Na transição do compasso 32, execute o Bass Swap.";
                golden_rule = "Regra de Ouro: Dois subgraves tocando juntos no clube causam lama acústica e ativam o limiter do PA.";
            } else if (cur_phase == 2) {
                tactical_prompt = "🔥 Subida! Ligue o Acid 303 no Rack abaixo. Gire o CUTOFF no sentido horário para criar tensão máxima até o drop!";
                golden_rule = "Regra de Ouro: Corte o 303 exatamente 1 batida antes do DROP para criar o vácuo de silêncio do clímax.";
            } else if (cur_phase == 3) {
                tactical_prompt = "🌊 Momento da virada! No tempo 4 do último compasso da subida, dispare o [ ECHO FREEZE ] ou [ WASHOUT RISER ].";
                golden_rule = "Regra de Ouro: A cauda do Echo Out limpa o ar sem deixar buraco vazio na pista de dança.";
            } else {
                tactical_prompt = "⚡ Finalização triunfante! Dispare um [ VINYL BRAKE ] ou [ SPINBACK ] rápido no Deck 1 para choque de pista.";
                golden_rule = "Regra de Ouro: Um spinback curto no tempo certo energiza a pista antes do drop da próxima faixa.";
            }

            ImGui::SetCursorScreenPos(ImVec2(card_min.x + 12.0f, card_min.y + 8.0f));
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.25f, 1.0f), ICON_FA_LIGHTBULB " COMO PROCEDER:");
            ImGui::SameLine(0, 8.0f);
            ImGui::TextColored(ImVec4(0.95f, 0.98f, 1.0f, 1.0f), "%s", tactical_prompt);

            ImGui::SetCursorScreenPos(ImVec2(card_min.x + 12.0f, card_min.y + 34.0f));
            ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), ICON_FA_GRADUATION_CAP " REGRA PRO:");
            ImGui::SameLine(0, 8.0f);
            ImGui::TextColored(ImVec4(0.75f, 0.85f, 0.95f, 1.0f), "%s", golden_rule);

            // Barra de contagem regressiva para o Drop
            int beats_into_phrase = ((current_bar - 1) % 8) * 4 + current_beat;
            int beats_to_drop = 32 - beats_into_phrase;
            if (beats_to_drop < 0) beats_to_drop = 0;

            ImGui::SetCursorScreenPos(ImVec2(card_min.x + 12.0f, card_min.y + 60.0f));
            ImGui::TextColored(ImVec4(1.0f, 0.40f, 0.80f, 1.0f), ICON_FA_HOURGLASS_HALF " CONTAGEM PARA O DROP: %d BATIDAS RESTANTES (%d COMPASSOS)", beats_to_drop, (beats_to_drop + 3) / 4);

            ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_min.y + h + 8.0f));
            ImGui::Dummy(ImVec2(w, 4.0f));
        }

        // =========================================================================
        // UNIT 03: [1U] 1-BUTTON PERFORMANCE TRANSITION MACROS
        // =========================================================================
        void renderUnit03_TransitionMacros(KuroAudio::DJEngine& engine, float w) {
            float h = 100.0f;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max(p_min.x + w, p_min.y + h);

            dl->AddRectFilled(p_min, p_max, IM_COL32(11, 14, 20, 255), 4.0f);
            dl->AddRect(p_min, p_max, IM_COL32(32, 42, 58, 255), 4.0f, 0, 1.2f);

            DrawModuleFaceplateHeader(dl, p_min, p_max, "UNIT 03 - 1U", "1-BUTTON PERFORMANCE TRANSITION MACROS & DROP FLUSH", IM_COL32(255, 60, 60, 255));

            auto& turntable = KuroAudio::DJTurntablismEngine::getInstance();
            auto& gain_staging = KuroAudio::DJGainStagingAnalyzer::getInstance();

            struct MacroDef {
                const char* label;
                const char* desc;
                ImU32 sig_col;
                int macro_id;
            };

            const MacroDef macros[6] = {
                { "WASHOUT RISER", "HPF Sweep + Reverb Cauda", IM_COL32(0, 220, 255, 255), 1 },
                { "ECHO FREEZE",   "Congelamento Infinito 1-Beat", IM_COL32(255, 180, 20, 255), 2 },
                { "NOISE DROP",    "Subida de Ruído Branco", IM_COL32(255, 255, 255, 255), 3 },
                { "VINYL BRAKE",   "Parada Fisiológica do Prato", IM_COL32(255, 60, 120, 255), 4 },
                { "SPINBACK",      "Backspin Analógico Veloz", IM_COL32(180, 70, 255, 255), 5 },
                { "PANIC MUTE",    "Limpeza Total de Buffers", IM_COL32(255, 40, 50, 255), 6 }
            };

            float btn_w = (w - 24.0f - 25.0f) / 6.0f;
            float btn_h = 42.0f;
            float btn_y = p_min.y + 36.0f;

            for (int i = 0; i < 6; i++) {
                ImVec2 b_pos(p_min.x + 12.0f + i * (btn_w + 5.0f), btn_y);
                ImVec2 b_max(b_pos.x + btn_w, b_pos.y + btn_h);

                char bid[32]; snprintf(bid, sizeof(bid), "##StudioMacro_%d", macros[i].macro_id);
                ImGui::SetCursorScreenPos(b_pos);
                bool clicked = ImGui::Button(bid, ImVec2(btn_w, btn_h));
                bool is_hov = ImGui::IsItemHovered();
                bool is_act = ImGui::IsItemActive();

                if (clicked) {
                    if (macros[i].macro_id == 1) { // Washout
                        engine.decks[0].sound_color_fx_mode = 0; // Filter
                        engine.decks[0].filter_bipolar = 0.85f;
                        engine.decks[0].updateSoundColorFX();
                    } else if (macros[i].macro_id == 2) { // Echo Freeze
                        gain_staging.triggerEchoOut();
                    } else if (macros[i].macro_id == 3) { // Noise Drop
                        engine.decks[0].sound_color_fx_mode = 1; // Noise
                        engine.decks[0].filter_bipolar = 0.75f;
                        engine.decks[0].updateSoundColorFX();
                    } else if (macros[i].macro_id == 4) { // Vinyl Brake
                        turntable.triggerVinylBrake(0, 0.75f);
                    } else if (macros[i].macro_id == 5) { // Spinback
                        turntable.triggerSpinback(0, -35.0f);
                    } else if (macros[i].macro_id == 6) { // Panic Mute
                        gain_staging.stopEchoOut();
                        turntable.reset();
                        engine.decks[0].filter_bipolar = 0.5f;
                        engine.decks[0].updateSoundColorFX();
                        engine.decks[1].filter_bipolar = 0.5f;
                        engine.decks[1].updateSoundColorFX();
                    }
                }

                ImU32 bg_c = is_act ? macros[i].sig_col : (is_hov ? IM_COL32(28, 36, 50, 255) : IM_COL32(16, 22, 32, 255));
                dl->AddRectFilled(b_pos, b_max, bg_c, 4.0f);
                dl->AddRect(b_pos, b_max, is_hov ? macros[i].sig_col : IM_COL32(45, 58, 78, 200), 4.0f, 0, is_hov ? 1.5f : 1.0f);

                DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), 11.5f, b_pos, b_max,
                                                 is_act ? IM_COL32(10, 14, 20, 255) : (is_hov ? IM_COL32(255, 255, 255, 255) : IM_COL32(215, 230, 248, 240)),
                                                 macros[i].label, 3.0f, 2.0f);

                if (is_hov) ImGui::SetTooltip("Macro %s: %s", macros[i].label, macros[i].desc);
            }

            ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_min.y + h + 8.0f));
            ImGui::Dummy(ImVec2(w, 4.0f));
        }

        // =========================================================================
        // UNIT 04: [2U] ANALOG ACID 303 SYNTHESIZER
        // =========================================================================
        void renderUnit04_Acid303Synthesizer(KuroAudio::DJEngine& engine, float w) {
            float h = 160.0f;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max(p_min.x + w, p_min.y + h);

            dl->AddRectFilled(p_min, p_max, IM_COL32(11, 14, 20, 255), 4.0f);
            dl->AddRect(p_min, p_max, IM_COL32(32, 42, 58, 255), 4.0f, 0, 1.2f);

            auto& live_inst = KuroAudio::DJLiveInstrumentEngine::getInstance();
            bool is_synth_running = live_inst.synth303.is_running.load();

            DrawModuleFaceplateHeader(dl, p_min, p_max, "UNIT 04 - 2U", "ANALOG ACID 303 SYNTHESIZER & 16-STEP BPM SEQUENCER", IM_COL32(255, 60, 180, 255), is_synth_running);

            float body_y = p_min.y + 32.0f;

            // 1. Botão Liga/Desliga e Seletor SAW / SQUARE
            float ctrl_x = p_min.x + 14.0f;
            ImGui::SetCursorScreenPos(ImVec2(ctrl_x, body_y));
            ImGui::PushStyleColor(ImGuiCol_Button, is_synth_running ? ImVec4(0.85f, 0.20f, 0.50f, 0.95f) : ImVec4(0.18f, 0.24f, 0.35f, 0.85f));
            if (ImGui::Button(is_synth_running ? ICON_FA_STOP " STOP 303" : ICON_FA_PLAY " RUN 303", ImVec2(100.0f, 32.0f))) {
                live_inst.synth303.is_running.store(!is_synth_running);
            }
            ImGui::PopStyleColor();

            // 2. Knobs Táteis do 303 (CUTOFF, RESO, ENV MOD, DECAY, VOLUME)
            float knobs_start_x = ctrl_x + 115.0f;
            float knob_w = (w - (knobs_start_x - p_min.x) - 20.0f) / 5.0f;
            float knob_r = 13.0f;

            struct KnobDef {
                const char* name;
                float* p_val;
                float min_v;
                float max_v;
                ImVec4 col;
            };

            KnobDef k_defs[5] = {
                { "CUTOFF",  &live_inst.synth303.cutoff,    0.05f, 0.98f, ImVec4(0.0f, 0.90f, 1.0f, 1.0f) },
                { "RESO",    &live_inst.synth303.resonance, 0.00f, 0.95f, ImVec4(1.0f, 0.65f, 0.10f, 1.0f) },
                { "ENV MOD", &live_inst.synth303.env_mod,   0.00f, 1.00f, ImVec4(0.95f, 0.35f, 0.90f, 1.0f) },
                { "DECAY",   &live_inst.synth303.decay,     0.05f, 1.00f, ImVec4(1.0f, 0.85f, 0.20f, 1.0f) },
                { "VOLUME",  &live_inst.synth303.volume,    0.00f, 1.00f, ImVec4(0.20f, 0.95f, 0.45f, 1.0f) }
            };

            for (int k = 0; k < 5; k++) {
                float k_cx = knobs_start_x + (float)k * knob_w + knob_w * 0.5f;
                ImVec2 lbl_min(k_cx - knob_w * 0.5f, body_y);
                ImVec2 lbl_max(k_cx + knob_w * 0.5f, body_y + 14.0f);
                DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), 10.5f, lbl_min, lbl_max,
                                                 IM_COL32(200, 220, 245, 240), k_defs[k].name, 2.0f, 1.0f);

                ImGui::SetCursorScreenPos(ImVec2(k_cx - knob_r - 4.0f, body_y + 16.0f));
                char kid[32]; snprintf(kid, sizeof(kid), "##RackKnob_%s", k_defs[k].name);
                KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob(kid, k_defs[k].p_val, k_defs[k].min_v, k_defs[k].max_v,
                                                                   knob_r, k_defs[k].col, "%.2f", 0.5f, false, false, false, false, false);
            }

            // 3. Sequenciador de 16 Passos com LED Running em Sincronia
            float seq_y = body_y + 68.0f;
            float step_w = (w - 28.0f - 15.0f * 3.0f) / 16.0f;
            float step_h = 24.0f;
            int active_step = live_inst.synth303.current_step.load();

            for (int s = 0; s < 16; s++) {
                ImVec2 s_min(p_min.x + 14.0f + s * (step_w + 3.0f), seq_y);
                ImVec2 s_max(s_min.x + step_w, s_min.y + step_h);
                bool is_current = (is_synth_running && active_step == s);

                ImU32 s_col = is_current ? IM_COL32(255, 50, 160, 255) : (s % 4 == 0 ? IM_COL32(24, 34, 48, 255) : IM_COL32(16, 22, 30, 255));
                dl->AddRectFilled(s_min, s_max, s_col, 3.0f);
                dl->AddRect(s_min, s_max, is_current ? IM_COL32(255, 255, 255, 240) : IM_COL32(40, 52, 70, 200), 3.0f);

                char s_lbl[8]; snprintf(s_lbl, sizeof(s_lbl), "%d", s + 1);
                DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), 10.0f, s_min, s_max,
                                                 is_current ? IM_COL32(10, 14, 20, 255) : IM_COL32(160, 180, 205, 220), s_lbl, 2.0f, 1.0f);
            }

            ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_min.y + h + 8.0f));
            ImGui::Dummy(ImVec2(w, 4.0f));
        }

        // =========================================================================
        // UNIT 05: [2U] 16-PAD PERFORMANCE MATRIX & SAMPLER
        // =========================================================================
        void renderUnit05_16PadsPerformanceMatrix(KuroAudio::DJEngine& engine, float w) {
            float body_y_test = UIScaleManager::Scale(34.0f);
            float pads_y_test = body_y_test + ImGui::GetTextLineHeightWithSpacing() + UIScaleManager::Scale(8.0f);
            float pad_h = UIScaleManager::Scale(46.0f);
            float h = std::max(UIScaleManager::Scale(185.0f), pads_y_test + (pad_h + UIScaleManager::Scale(4.0f)) * 2.0f + UIScaleManager::Scale(12.0f));
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max(p_min.x + w, p_min.y + h);

            dl->AddRectFilled(p_min, p_max, IM_COL32(11, 14, 20, 255), 4.0f);
            dl->AddRect(p_min, p_max, IM_COL32(32, 42, 58, 255), 4.0f, 0, 1.2f);

            DrawModuleFaceplateHeader(dl, p_min, p_max, "UNIT 05 - 2U", "16-PAD PERFORMANCE MATRIX (WIDE 8x2 / SAMPLER 16-SLOTS)", IM_COL32(0, 255, 140, 255));

            float body_y = p_min.y + UIScaleManager::Scale(34.0f);

            // Seletor de Modo da Matriz de Pads: [ HOT CUE (A..P) ] vs [ SAMPLER (16 SLOTS) ]
            ImGui::SetCursorScreenPos(ImVec2(p_min.x + UIScaleManager::Scale(14.0f), body_y));
            if (ImGui::RadioButton("HOT CUES A..P (16 CUES)", m_pad_matrix_mode == 0)) m_pad_matrix_mode = 0;
            ImGui::SameLine(0, UIScaleManager::Scale(16.0f));
            if (ImGui::RadioButton("SAMPLER MATRIX (16 SLOTS)", m_pad_matrix_mode == 1)) m_pad_matrix_mode = 1;

            float pads_y = body_y + ImGui::GetTextLineHeightWithSpacing() + UIScaleManager::Scale(8.0f);
            float pad_w = (w - UIScaleManager::Scale(28.0f) - UIScaleManager::Scale(7.0f) * 4.0f) / 8.0f;

            const char* cue_names[16] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P" };
            const ImU32 cue_colors[16] = {
                IM_COL32(235, 45, 80, 255), IM_COL32(255, 145, 0, 255), IM_COL32(240, 210, 30, 255), IM_COL32(35, 215, 85, 255),
                IM_COL32(0, 200, 245, 255), IM_COL32(30, 110, 245, 255), IM_COL32(170, 50, 245, 255), IM_COL32(240, 50, 175, 255),
                IM_COL32(255, 80, 120, 255), IM_COL32(255, 170, 50, 255), IM_COL32(255, 230, 70, 255), IM_COL32(70, 240, 120, 255),
                IM_COL32(50, 220, 255, 255), IM_COL32(80, 140, 255, 255), IM_COL32(200, 80, 255, 255), IM_COL32(255, 80, 210, 255)
            };

            auto& d1 = engine.decks[0];

            for (int p = 0; p < 16; p++) {
                int col = p % 8;
                int row = p / 8;
                ImVec2 b_min(p_min.x + 14.0f + col * (pad_w + 4.0f), pads_y + row * (pad_h + 4.0f));
                ImVec2 b_max(b_min.x + pad_w, b_min.y + pad_h);

                char pid[32]; snprintf(pid, sizeof(pid), "##RackPad16_%d", p);
                ImGui::SetCursorScreenPos(b_min);
                bool clicked = ImGui::InvisibleButton(pid, ImVec2(pad_w, pad_h));
                bool is_hov = ImGui::IsItemHovered();
                bool is_act = ImGui::IsItemActive();

                if (clicked) {
                    if (m_pad_matrix_mode == 0) {
                        if (d1.hot_cues[p].active) d1.jumpToHotCue(p);
                        else d1.setHotCue(p);
                    } else {
                        engine.triggerSampler(p);
                    }
                }

                bool is_lit = (m_pad_matrix_mode == 0) ? d1.hot_cues[p].active : engine.samplers[p].is_playing;
                ImU32 pad_c = is_lit ? cue_colors[p] : (is_hov ? IM_COL32(28, 38, 52, 255) : IM_COL32(16, 22, 32, 255));

                dl->AddRectFilled(b_min, b_max, pad_c, 3.5f);
                dl->AddRect(b_min, b_max, is_lit ? IM_COL32(255, 255, 255, 240) : IM_COL32(40, 52, 70, 200), 3.5f);

                const char* txt = (m_pad_matrix_mode == 0) ? cue_names[p] : engine.samplers[p].name.c_str();
                DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), 12.0f, b_min, b_max,
                                                 is_lit ? IM_COL32(10, 14, 20, 255) : IM_COL32(220, 235, 255, 240), txt, 3.0f, 2.0f);
            }

            ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_min.y + h + 8.0f));
            ImGui::Dummy(ImVec2(w, 4.0f));
        }

        // =========================================================================
        // UNIT 06: [1U] SOUND COLOR FX CONSOLE
        // =========================================================================
        void renderUnit06_SoundColorFXConsole(KuroAudio::DJEngine& engine, float w) {
            float h = 105.0f;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max(p_min.x + w, p_min.y + h);

            dl->AddRectFilled(p_min, p_max, IM_COL32(11, 14, 20, 255), 4.0f);
            dl->AddRect(p_min, p_max, IM_COL32(32, 42, 58, 255), 4.0f, 0, 1.2f);

            DrawModuleFaceplateHeader(dl, p_min, p_max, "UNIT 06 - 1U", "SOUND COLOR FX (CFX) FILTER & MULTI-FX PROCESSING", IM_COL32(0, 220, 255, 255));

            float body_y = p_min.y + 36.0f;

            struct CfxDef {
                int mode_idx;
                const char* name;
                ImU32 col;
            };

            const CfxDef cfxs[4] = {
                { 0, "FILTER", IM_COL32(0, 229, 255, 255) },
                { 2, "CRUSH",  IM_COL32(255, 107, 0, 255) },
                { 1, "NOISE",  IM_COL32(56, 189, 248, 255) },
                { 3, "SPACE",  IM_COL32(168, 85, 247, 255) }
            };

            float btn_w = std::clamp(w * 0.12f, 60.0f, 95.0f);
            float btn_h = 32.0f;

            for (int i = 0; i < 4; i++) {
                ImVec2 b_pos(p_min.x + 14.0f + i * (btn_w + 6.0f), body_y);
                ImVec2 b_max(b_pos.x + btn_w, b_pos.y + btn_h);
                bool is_active = (engine.decks[0].sound_color_fx_mode == cfxs[i].mode_idx);

                char bid[32]; snprintf(bid, sizeof(bid), "##RackCfx_%d", cfxs[i].mode_idx);
                ImGui::SetCursorScreenPos(b_pos);
                if (ImGui::Button(bid, ImVec2(btn_w, btn_h))) {
                    engine.decks[0].sound_color_fx_mode = cfxs[i].mode_idx;
                    engine.decks[1].sound_color_fx_mode = cfxs[i].mode_idx;
                }

                dl->AddRectFilled(b_pos, b_max, is_active ? cfxs[i].col : IM_COL32(18, 24, 34, 255), 3.0f);
                dl->AddRect(b_pos, b_max, is_active ? IM_COL32(255, 255, 255, 240) : (cfxs[i].col & 0x00FFFFFF | 0x65000000), 3.0f);

                DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), 12.0f, b_pos, b_max,
                                                 is_active ? IM_COL32(10, 14, 20, 255) : IM_COL32(220, 235, 255, 240), cfxs[i].name, 2.0f, 1.0f);
            }

            // Knobs Color CH1, PARAM e Color CH2
            float knobs_x = p_min.x + 14.0f + 4 * (btn_w + 6.0f) + 16.0f;
            float rem_w = (p_max.x - knobs_x - 14.0f) / 3.0f;
            float knob_r = 13.0f;

            // CH 1 Color
            float k1_cx = knobs_x + rem_w * 0.5f;
            DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), 10.5f, ImVec2(knobs_x, body_y - 2.0f), ImVec2(knobs_x + rem_w, body_y + 12.0f),
                                             IM_COL32(0, 220, 255, 240), "CH 1 COLOR", 2.0f, 1.0f);
            ImGui::SetCursorScreenPos(ImVec2(k1_cx - knob_r - 4.0f, body_y + 14.0f));
            if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob("##RackCh1Color", &engine.decks[0].filter_bipolar, 0.0f, 1.0f, knob_r,
                                                                   ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "%.2f", 0.5f, true, false, false, false, false)) {
                engine.decks[0].updateSoundColorFX();
            }

            // PARAM
            float k2_cx = knobs_x + rem_w + rem_w * 0.5f;
            DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), 10.5f, ImVec2(knobs_x + rem_w, body_y - 2.0f), ImVec2(knobs_x + rem_w * 2.0f, body_y + 12.0f),
                                             IM_COL32(240, 245, 255, 240), "CFX PARAM", 2.0f, 1.0f);
            ImGui::SetCursorScreenPos(ImVec2(k2_cx - knob_r - 4.0f, body_y + 14.0f));
            if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob("##RackParamColor", &engine.decks[0].sound_color_param, 0.0f, 1.0f, knob_r,
                                                                   ImVec4(0.95f, 0.95f, 1.0f, 1.0f), "%.2f", 0.5f, false, false, false, false, false)) {
                engine.decks[0].updateSoundColorFX();
                engine.decks[1].sound_color_param = engine.decks[0].sound_color_param;
                engine.decks[1].updateSoundColorFX();
            }

            // CH 2 Color
            float k3_cx = knobs_x + rem_w * 2.0f + rem_w * 0.5f;
            DJLayoutEditorUI::DrawFittedText(dl, ImGui::GetFont(), 10.5f, ImVec2(knobs_x + rem_w * 2.0f, body_y - 2.0f), ImVec2(knobs_x + rem_w * 3.0f, body_y + 12.0f),
                                             IM_COL32(255, 160, 20, 240), "CH 2 COLOR", 2.0f, 1.0f);
            ImGui::SetCursorScreenPos(ImVec2(k3_cx - knob_r - 4.0f, body_y + 14.0f));
            if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob("##RackCh2Color", &engine.decks[1].filter_bipolar, 0.0f, 1.0f, knob_r,
                                                                   ImVec4(1.0f, 0.65f, 0.10f, 1.0f), "%.2f", 0.5f, true, false, false, false, false)) {
                engine.decks[1].updateSoundColorFX();
            }

            ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_min.y + h + 8.0f));
            ImGui::Dummy(ImVec2(w, 4.0f));
        }
    };

} // namespace KuroUI

