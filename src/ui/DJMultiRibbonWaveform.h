#pragma once
#include "imgui.h"
#include "DJLayoutMetrics.h"
#include "DJBeatPulseEngine.h"
#include "../core/DJMIRPhrasingEngine.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

extern ImFont* g_font_rekordbox_bold;
extern ImFont* g_font_rekordbox_regular;
extern ImFont* g_font_rekordbox_small;
extern ImFont* g_font_control_small;
extern ImFont* g_font_micro_tag;

namespace KuroUI {

    struct RibbonPoint {
        float x = 0.0f;
        float high_amp = 0.0f;   // Altura em pixels calculada com curva Pioneer dB (h_high)
        float mid_amp  = 0.0f;   // Altura em pixels calculada com curva Pioneer dB (h_mid)
        float low_amp  = 0.0f;   // Altura em pixels calculada com curva Pioneer dB (h_low)
        float total_amp = 0.0f;  // Altura total em pixels (h_total)
        float low = 0.0f;        // Componente normalizado de graves [0.0..1.0]
        float mid = 0.0f;        // Componente normalizado de médios [0.0..1.0]
        float high = 0.0f;       // Componente normalizado de agudos [0.0..1.0]
        float cycle_max = 1.0f;  // Excursão instantânea positiva do áudio [0.35..1.25]
        float cycle_min = -1.0f; // Excursão instantânea negativa do áudio [-1.25..-0.35]
        bool  is_tail = false;   // Silêncio / cauda de osciloscópio
        bool  active = true;
    };

    class DJMultiRibbonWaveform {
    public:
        // Renderizador comum de Playhead central, Metrônomo Óptico e Badges Informativas
        void renderPlayheadAndBadges(
            ImDrawList* dl,
            ImVec2 pos,
            float width,
            float height,
            float center_x,
            double cur_b_phase,
            int cur_b_in_bar,
            bool is_playing,
            KuroAudio::TrackPhraseType cur_ph
        ) {
            auto& metrics = DJLayoutMetrics::getInstance();

            // Metrônomo Óptico no Playhead Central estilo Tessaiga (Inuyasha)
            // Barra respirando organicamente com pulso em todos os beats e clarão no Beat 1
            DJBeatPulseEngine::getInstance().renderPlayheadMetronome(
                dl, ImVec2(center_x, pos.y), height, cur_b_in_bar, cur_b_phase, is_playing
            );

            // Badges superiores: Anti-colisão dinâmica sequencial para Beat e Frase MIR
            ImFont* f_badge = g_font_control_small ? g_font_control_small : (g_font_rekordbox_small ? g_font_rekordbox_small : (g_font_rekordbox_bold ? g_font_rekordbox_bold : ImGui::GetFont()));
            float f_sz = 11.0f * metrics.ui_font_scale;

            // 1. Badge BEAT X/4
            char beat_badge[32];
            snprintf(beat_badge, sizeof(beat_badge), "BEAT %d/4", cur_b_in_bar);
            ImU32 beat_col = DJBeatPulseEngine::getBeatColor(cur_b_in_bar, 1.0f);

            ImVec2 beat_txt_sz = f_badge->CalcTextSizeA(f_sz, FLT_MAX, 0.0f, beat_badge);
            float beat_w = beat_txt_sz.x + 12.0f;
            float badge_h = std::max(15.0f, beat_txt_sz.y + 4.0f);

            ImVec2 beat_p0(pos.x + 8.0f, pos.y + 3.0f);
            ImVec2 beat_p1(beat_p0.x + beat_w, beat_p0.y + badge_h);

            dl->AddRectFilled(beat_p0, beat_p1, IM_COL32(10, 14, 22, 230), 3.0f);
            dl->AddRect(beat_p0, beat_p1, IM_COL32(50, 68, 95, 200), 3.0f, 0, 1.0f);
            dl->AddText(f_badge, f_sz, ImVec2(beat_p0.x + 6.0f, beat_p0.y + (badge_h - beat_txt_sz.y) * 0.5f - 0.5f), beat_col, beat_badge);

            // 2. Badge FRASE MIR (Colocado estritamente após o BEAT com margem garantida de 6px)
            const char* ph_name = "VERSE";
            switch (cur_ph) {
                case KuroAudio::TrackPhraseType::INTRO: ph_name = "INTRO"; break;
                case KuroAudio::TrackPhraseType::VERSE_1: ph_name = "VERSE 1"; break;
                case KuroAudio::TrackPhraseType::BUILDUP: ph_name = "BUILD-UP"; break;
                case KuroAudio::TrackPhraseType::DROP: ph_name = "DROP [HOT]"; break;
                case KuroAudio::TrackPhraseType::BREAK_RESPRO: ph_name = "BREAK / RESPIRO"; break;
                case KuroAudio::TrackPhraseType::VERSE_2: ph_name = "VERSE 2"; break;
                case KuroAudio::TrackPhraseType::OUTRO: ph_name = "OUTRO"; break;
                default: ph_name = "PHRASE"; break;
            }
            ImU32 ph_col = KuroAudio::DJMIRPhrasingEngine::getPhraseColor(cur_ph, false, 1.0f);
            ImVec2 ph_txt_sz = f_badge->CalcTextSizeA(f_sz, FLT_MAX, 0.0f, ph_name);
            float ph_w = ph_txt_sz.x + 14.0f;

            float ph_x0 = beat_p1.x + 6.0f; // Margem anti-colisão estrita
            ImVec2 ph_p0(ph_x0, pos.y + 3.0f);
            ImVec2 ph_p1(ph_x0 + ph_w, ph_p0.y + badge_h);

            ImU32 ph_border = (ph_col & 0x00FFFFFF) | 0x88000000;
            dl->AddRectFilled(ph_p0, ph_p1, IM_COL32(10, 14, 22, 230), 3.0f);
            dl->AddRect(ph_p0, ph_p1, ph_border, 3.0f, 0, 1.0f);
            dl->AddText(f_badge, f_sz, ImVec2(ph_p0.x + 7.0f, ph_p0.y + (badge_h - ph_txt_sz.y) * 0.5f - 0.5f), ph_col, ph_name);
        }

    private:
        DJMultiRibbonWaveform() = default;

    public:
        static DJMultiRibbonWaveform& getInstance() {
            static DJMultiRibbonWaveform instance;
            return instance;
        }

        // =========================================================================
        // MODO 1: REKORDBOX 3-BAND MULTI-LANE (AMOSTRAGEM PADRÃO OFICIAL EM 3 BAIAS)
        // 3 Baias independentes (High, Mid, Low) com a autêntica amostragem contínua
        // Rekordbox por colunas de 1.0px, curva dB Pioneer e miolo incandescente (50%).
        // =========================================================================
        void renderRekordbox3BandMultiLane(
            ImDrawList* dl,
            const std::vector<RibbonPoint>& rib_pts,
            ImVec2 pos,
            float width,
            float height,
            float center_x,
            double cur_b_phase,
            int cur_b_in_bar,
            bool is_playing,
            KuroAudio::TrackPhraseType cur_ph,
            int color_mode = -1
        ) {
            if (!dl || rib_pts.size() < 2 || width <= 10.0f || height <= 10.0f) return;

            auto& metrics = DJLayoutMetrics::getInstance();
            int eff_color_mode = (color_mode >= 0) ? color_mode : metrics.waveform_color_mode;

            // 1. Fundo da baia de waveforms com gradiente sutil
            ImU32 bg_color_top = IM_COL32(10, 13, 19, 245);
            ImU32 bg_color_bot = IM_COL32(15, 18, 26, 250);
            dl->AddRectFilledMultiColor(pos, ImVec2(pos.x + width, pos.y + height),
                                        bg_color_top, bg_color_top, bg_color_bot, bg_color_bot);

            // Tonalidade sutil da frase musical detectada (MIR)
            ImU32 phrase_tint = KuroAudio::DJMIRPhrasingEngine::getPhraseColor(cur_ph, false, 0.07f);
            dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), phrase_tint);

            // 2. Altura de cada baia
            float high_ratio = metrics.ribbon_lane_high_ratio;
            float mid_ratio  = metrics.ribbon_lane_mid_ratio;
            float low_ratio  = metrics.ribbon_lane_low_ratio;
            float total_r = high_ratio + mid_ratio + low_ratio;
            if (total_r <= 0.01f) { high_ratio = 0.28f; mid_ratio = 0.36f; low_ratio = 0.36f; total_r = 1.0f; }

            float high_h = height * (high_ratio / total_r);
            float mid_h  = height * (mid_ratio  / total_r);
            float low_h  = height * (low_ratio  / total_r);

            float high_base_y = pos.y + high_h * 0.5f;
            float mid_base_y  = pos.y + high_h + mid_h * 0.5f;
            float low_base_y  = pos.y + high_h + mid_h + low_h * 0.5f;

            // Linhas divisórias sutis de lane
            ImU32 div_col = IM_COL32(255, 255, 255, 22);
            dl->AddLine(ImVec2(pos.x, pos.y + high_h), ImVec2(pos.x + width, pos.y + high_h), div_col, 1.0f);
            dl->AddLine(ImVec2(pos.x, pos.y + high_h + mid_h), ImVec2(pos.x + width, pos.y + high_h + mid_h), div_col, 1.0f);

            // Paletas por Color Mode
            ImU32 high_outer, high_core, mid_outer, mid_core, low_outer, low_core;
            if (eff_color_mode == 0) {
                // RGB MODE: Ciano Elétrico nos Agudos, Âmbar Solar nos Médios, Magenta Shock nos Graves
                high_outer = IM_COL32(0, 229, 255, 245);
                high_core  = IM_COL32(180, 255, 255, 255);
                mid_outer  = IM_COL32(255, 160, 20, 245);
                mid_core   = IM_COL32(255, 215, 60, 255);
                low_outer  = IM_COL32(255, 0, 110, 245);
                low_core   = IM_COL32(255, 120, 185, 255);
            } else if (eff_color_mode == 1) {
                // 3-BAND CDJ-3000: Branco Diamante Agudos, Âmbar Médios, Azul Royal Graves
                high_outer = IM_COL32(180, 240, 255, 245);
                high_core  = IM_COL32(255, 255, 255, 255);
                mid_outer  = IM_COL32(225, 130, 15, 245);
                mid_core   = IM_COL32(255, 195, 45, 255);
                low_outer  = IM_COL32(0, 85, 245, 245);
                low_core   = IM_COL32(45, 175, 255, 255);
            } else if (eff_color_mode == 2) {
                // BLUE CDJ-2000
                high_outer = IM_COL32(200, 235, 255, 245);
                high_core  = IM_COL32(255, 255, 255, 255);
                mid_outer  = IM_COL32(20, 150, 255, 240);
                mid_core   = IM_COL32(110, 205, 255, 255);
                low_outer  = IM_COL32(0, 60, 200, 245);
                low_core   = IM_COL32(40, 140, 255, 255);
            } else if (eff_color_mode == 3) {
                // NEON CYBERPUNK
                high_outer = IM_COL32(0, 245, 255, 245);
                high_core  = IM_COL32(200, 255, 255, 255);
                mid_outer  = IM_COL32(0, 255, 120, 245);
                mid_core   = IM_COL32(140, 255, 180, 255);
                low_outer  = IM_COL32(220, 0, 255, 245);
                low_core   = IM_COL32(255, 130, 255, 255);
            } else if (eff_color_mode == 4) {
                // SUNSET SOLAR
                high_outer = IM_COL32(255, 235, 60, 245);
                high_core  = IM_COL32(255, 255, 200, 255);
                mid_outer  = IM_COL32(255, 135, 10, 245);
                mid_core   = IM_COL32(255, 195, 50, 255);
                low_outer  = IM_COL32(235, 25, 40, 245);
                low_core   = IM_COL32(255, 110, 110, 255);
            } else if (eff_color_mode == 5) {
                // EMERALD MATRIX
                high_outer = IM_COL32(140, 255, 200, 245);
                high_core  = IM_COL32(220, 255, 235, 255);
                mid_outer  = IM_COL32(40, 235, 70, 245);
                mid_core   = IM_COL32(110, 255, 140, 255);
                low_outer  = IM_COL32(0, 115, 40, 245);
                low_core   = IM_COL32(35, 205, 85, 255);
            } else if (eff_color_mode == 6) {
                // VAPORWAVE
                high_outer = IM_COL32(50, 240, 255, 245);
                high_core  = IM_COL32(200, 255, 255, 255);
                mid_outer  = IM_COL32(255, 25, 180, 245);
                mid_core   = IM_COL32(255, 120, 220, 255);
                low_outer  = IM_COL32(120, 12, 195, 245);
                low_core   = IM_COL32(195, 65, 255, 255);
            } else if (eff_color_mode == 7) {
                // INFERNO MAGMA
                high_outer = IM_COL32(255, 230, 70, 245);
                high_core  = IM_COL32(255, 255, 200, 255);
                mid_outer  = IM_COL32(255, 100, 0, 245);
                mid_core   = IM_COL32(255, 165, 50, 255);
                low_outer  = IM_COL32(220, 10, 10, 245);
                low_core   = IM_COL32(255, 65, 45, 255);
            } else if (eff_color_mode == 8) {
                // GLACIER FROST
                high_outer = IM_COL32(215, 250, 255, 245);
                high_core  = IM_COL32(255, 255, 255, 255);
                mid_outer  = IM_COL32(0, 185, 235, 245);
                mid_core   = IM_COL32(80, 225, 255, 255);
                low_outer  = IM_COL32(10, 50, 160, 245);
                low_core   = IM_COL32(45, 140, 255, 255);
            } else {
                // MONOCHROME PLATINUM
                high_outer = IM_COL32(245, 250, 255, 245);
                high_core  = IM_COL32(255, 255, 255, 255);
                mid_outer  = IM_COL32(155, 165, 180, 245);
                mid_core   = IM_COL32(215, 225, 240, 255);
                low_outer  = IM_COL32(55, 60, 70, 245);
                low_core   = IM_COL32(125, 135, 150, 255);
            }

            if (cur_ph == KuroAudio::TrackPhraseType::DROP && eff_color_mode == 1) {
                low_outer = IM_COL32(255, 25, 70, 245);
                low_core  = IM_COL32(255, 130, 185, 255);
            }

            // Sub-rótulos laterais de frequência Rekordbox
            dl->AddText(ImVec2(pos.x + 8.0f, high_base_y - 6.0f), high_outer, "HI");
            dl->AddText(ImVec2(pos.x + 8.0f, mid_base_y - 6.0f),  mid_outer,  "MID");
            dl->AddText(ImVec2(pos.x + 8.0f, low_base_y - 6.0f),  low_outer,  "LOW");

            float max_high_sub_h = (high_h * 0.5f) * 0.88f;
            float max_mid_sub_h  = (mid_h  * 0.5f) * 0.88f;
            float max_low_sub_h  = (low_h  * 0.5f) * 0.88f;

            // Ganho calibrado para manter proporção compacta e elegante (não alonga para cima)
            float gain_elongate = 1.25f;

            // 3. Renderização com suporte a Linhas Rekordbox 7, Barras ou Malha
            int line_style = metrics.waveform_line_style;
            float line_w = std::max(0.8f, metrics.waveform_line_width);
            float line_gap = metrics.waveform_line_gap;
            float core_bright = metrics.waveform_core_brightness;
            const size_t pt_count = rib_pts.size();
            for (size_t i = 0; i < pt_count; i++) {
                const auto& p = rib_pts[i];
                if (p.x < pos.x || p.x > pos.x + width) continue;

                float cur_x = p.x;
                float x1 = (line_style == 1) ? (cur_x - std::max(0.8f, line_w - line_gap * 0.5f) * 0.5f) : cur_x;
                float x2 = (line_style == 1) ? (cur_x + std::max(0.8f, line_w - line_gap * 0.5f) * 0.5f) : (cur_x + 1.0f);

                if (p.is_tail || !p.active) {
                    // Lead-in elegante de silêncio pré-track Rekordbox/CDJ-3000
                    // Evita o vazio preto que parecia áudio "bugado" antes de 00:00
                    ImU32 lead_col = IM_COL32(35, 48, 68, 140);
                    dl->AddLine(ImVec2(x1, high_base_y), ImVec2(x2, high_base_y), lead_col, 1.0f);
                    dl->AddLine(ImVec2(x1, mid_base_y),  ImVec2(x2, mid_base_y),  lead_col, 1.0f);
                    dl->AddLine(ImVec2(x1, low_base_y),  ImVec2(x2, low_base_y),  lead_col, 1.0f);
                    if (((int)cur_x) % 32 < 2) {
                        dl->AddLine(ImVec2(cur_x, high_base_y - 2.5f), ImVec2(cur_x, high_base_y + 2.5f), IM_COL32(65, 88, 120, 160), 1.0f);
                        dl->AddLine(ImVec2(cur_x, mid_base_y - 2.5f),  ImVec2(cur_x, mid_base_y + 2.5f),  IM_COL32(65, 88, 120, 160), 1.0f);
                        dl->AddLine(ImVec2(cur_x, low_base_y - 2.5f),  ImVec2(cur_x, low_base_y + 2.5f),  IM_COL32(65, 88, 120, 160), 1.0f);
                    }
                    continue;
                }

                // --- HIGH SUB-LANE ---
                {
                    float amp_h = std::clamp(p.high_amp * gain_elongate, 2.0f, max_high_sub_h);
                    float y_top = high_base_y - amp_h * std::clamp(p.cycle_max, 0.65f, 1.20f);
                    float y_bot = high_base_y + amp_h * std::clamp(-p.cycle_min, 0.65f, 1.20f);
                    if (y_bot - y_top < 2.0f) { y_top = high_base_y - 1.0f; y_bot = high_base_y + 1.0f; }

                    // Pass 1: Envelope externo contíguo Rekordbox
                    dl->AddRectFilledMultiColor(ImVec2(x1, y_top), ImVec2(x2, high_base_y),
                                                high_outer, high_outer, high_core, high_core);
                    dl->AddRectFilledMultiColor(ImVec2(x1, high_base_y), ImVec2(x2, y_bot),
                                                high_core, high_core, high_outer, high_outer);

                    // Pass 2: Miolo Incandescente denso a 50% de amplitude
                    float core_top = high_base_y - (high_base_y - y_top) * 0.5f;
                    float core_bot = high_base_y + (y_bot - high_base_y) * 0.5f;
                    dl->AddRectFilled(ImVec2(x1, core_top), ImVec2(x2, core_bot), high_core);
                }

                // --- MID SUB-LANE ---
                {
                    float amp_m = std::clamp(p.mid_amp * gain_elongate, 2.0f, max_mid_sub_h);
                    float y_top = mid_base_y - amp_m * std::clamp(p.cycle_max, 0.65f, 1.20f);
                    float y_bot = mid_base_y + amp_m * std::clamp(-p.cycle_min, 0.65f, 1.20f);
                    if (y_bot - y_top < 2.0f) { y_top = mid_base_y - 1.0f; y_bot = mid_base_y + 1.0f; }

                    dl->AddRectFilledMultiColor(ImVec2(x1, y_top), ImVec2(x2, mid_base_y),
                                                mid_outer, mid_outer, mid_core, mid_core);
                    dl->AddRectFilledMultiColor(ImVec2(x1, mid_base_y), ImVec2(x2, y_bot),
                                                mid_core, mid_core, mid_outer, mid_outer);

                    float core_top = mid_base_y - (mid_base_y - y_top) * 0.5f;
                    float core_bot = mid_base_y + (y_bot - mid_base_y) * 0.5f;
                    dl->AddRectFilled(ImVec2(x1, core_top), ImVec2(x2, core_bot), mid_core);
                }

                // --- LOW SUB-LANE ---
                {
                    float amp_l = std::clamp(p.low_amp * (gain_elongate * 0.95f), 2.0f, max_low_sub_h);
                    float y_top = low_base_y - amp_l * std::clamp(p.cycle_max, 0.65f, 1.20f);
                    float y_bot = low_base_y + amp_l * std::clamp(-p.cycle_min, 0.65f, 1.20f);
                    if (y_bot - y_top < 2.0f) { y_top = low_base_y - 1.0f; y_bot = low_base_y + 1.0f; }

                    dl->AddRectFilledMultiColor(ImVec2(x1, y_top), ImVec2(x2, low_base_y),
                                                low_outer, low_outer, low_core, low_core);
                    dl->AddRectFilledMultiColor(ImVec2(x1, low_base_y), ImVec2(x2, y_bot),
                                                low_core, low_core, low_outer, low_outer);

                    float core_top = low_base_y - (low_base_y - y_top) * 0.5f;
                    float core_bot = low_base_y + (y_bot - low_base_y) * 0.5f;
                    dl->AddRectFilled(ImVec2(x1, core_top), ImVec2(x2, core_bot), low_core);
                }
            }

            // 4. Linhas neutras de zero-crossing (centro de cada sub-baia)
            ImU32 zero_cross_col = IM_COL32(255, 255, 255, 45);
            dl->AddLine(ImVec2(pos.x, high_base_y), ImVec2(pos.x + width, high_base_y), zero_cross_col, 1.0f);
            dl->AddLine(ImVec2(pos.x, mid_base_y),  ImVec2(pos.x + width, mid_base_y),  zero_cross_col, 1.0f);
            dl->AddLine(ImVec2(pos.x, low_base_y),  ImVec2(pos.x + width, low_base_y),  zero_cross_col, 1.0f);

            // 5. Playhead, Metrônomo e Badges
            renderPlayheadAndBadges(dl, pos, width, height, center_x, cur_b_phase, cur_b_in_bar, is_playing, cur_ph);
        }

        // =========================================================================
        // MODO 2: REKORDBOX 3-BAND FUSED (FUSÃO DAS WAVEFORMS PADRÃO CDJ-3000)
        // Fusão em eixo único compartilhado com suporte total à Paleta RGB (Rekordbox/Traktor)
        // e 3-Band CDJ-3000 Oficial (Graves Azul/Magenta, Médios Âmbar, Agudos Ciano/Branco).
        // =========================================================================
        void renderRekordbox3BandFusedLane(
            ImDrawList* dl,
            const std::vector<RibbonPoint>& rib_pts,
            ImVec2 pos,
            float width,
            float height,
            float center_x,
            double cur_b_phase,
            int cur_b_in_bar,
            bool is_playing,
            KuroAudio::TrackPhraseType cur_ph,
            int color_mode = -1
        ) {
            if (!dl || rib_pts.size() < 2 || width <= 10.0f || height <= 10.0f) return;

            auto& metrics = DJLayoutMetrics::getInstance();
            int eff_color_mode = (color_mode >= 0) ? color_mode : metrics.waveform_color_mode;

            // Fundo da pista
            ImU32 bg_color_top = IM_COL32(10, 13, 19, 250);
            ImU32 bg_color_bot = IM_COL32(14, 18, 25, 255);
            dl->AddRectFilledMultiColor(pos, ImVec2(pos.x + width, pos.y + height),
                                        bg_color_top, bg_color_top, bg_color_bot, bg_color_bot);

            ImU32 phrase_tint = KuroAudio::DJMIRPhrasingEngine::getPhraseColor(cur_ph, false, 0.06f);
            dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), phrase_tint);

            float mid_y = pos.y + height * 0.5f;
            float max_half_h = (height * 0.5f) * 0.82f;
            float gain_fused = 1.25f;

            int line_style = metrics.waveform_line_style;
            float line_w = std::max(0.8f, metrics.waveform_line_width);
            float line_gap = metrics.waveform_line_gap;
            float core_bright = metrics.waveform_core_brightness;
            float punch = std::clamp(metrics.waveform_transient_punch, 0.50f, 3.00f);

            // Respiro dinâmico entre linhas (aplica o slider Separação / Gap entre Linhas)
            float eff_line_w = std::max(0.6f, line_w - line_gap * 0.5f);

            // Paletas padrão quando em 3-Band, Blue, Neon ou Sunset
            ImU32 low_outer, low_core, mid_outer, mid_core, high_outer, high_core;
            if (eff_color_mode == 1) {
                // 3-BAND CDJ-3000 OFICIAL
                low_outer  = (cur_ph == KuroAudio::TrackPhraseType::DROP) ? IM_COL32(255, 25, 70, 245) : IM_COL32(0, 85, 245, 245);
                low_core   = (cur_ph == KuroAudio::TrackPhraseType::DROP) ? IM_COL32(255, 130, 185, 255) : IM_COL32(45, 175, 255, 255);
                mid_outer  = IM_COL32(235, 135, 15, 245);
                mid_core   = IM_COL32(255, 195, 45, 255);
                high_outer = IM_COL32(180, 240, 255, 245);
                high_core  = IM_COL32(255, 255, 255, 255);
            } else if (eff_color_mode == 2) {
                // BLUE CDJ-2000
                low_outer  = IM_COL32(0, 60, 200, 245);
                low_core   = IM_COL32(40, 140, 255, 255);
                mid_outer  = IM_COL32(20, 150, 255, 240);
                mid_core   = IM_COL32(110, 205, 255, 255);
                high_outer = IM_COL32(200, 235, 255, 245);
                high_core  = IM_COL32(255, 255, 255, 255);
            } else if (eff_color_mode == 3) {
                // NEON CYBERPUNK
                low_outer  = IM_COL32(220, 0, 255, 245);
                low_core   = IM_COL32(255, 130, 255, 255);
                mid_outer  = IM_COL32(0, 255, 120, 245);
                mid_core   = IM_COL32(140, 255, 180, 255);
                high_outer = IM_COL32(0, 245, 255, 245);
                high_core  = IM_COL32(200, 255, 255, 255);
            } else if (eff_color_mode == 4) {
                // SUNSET SOLAR
                low_outer  = IM_COL32(235, 25, 40, 245);
                low_core   = IM_COL32(255, 110, 110, 255);
                mid_outer  = IM_COL32(255, 135, 10, 245);
                mid_core   = IM_COL32(255, 195, 50, 255);
                high_outer = IM_COL32(255, 235, 60, 245);
                high_core  = IM_COL32(255, 255, 200, 255);
            } else if (eff_color_mode == 5) {
                // EMERALD MATRIX
                low_outer  = (cur_ph == KuroAudio::TrackPhraseType::DROP) ? IM_COL32(255, 25, 70, 245) : IM_COL32(0, 115, 40, 245);
                low_core   = (cur_ph == KuroAudio::TrackPhraseType::DROP) ? IM_COL32(255, 130, 185, 255) : IM_COL32(35, 205, 85, 255);
                mid_outer  = IM_COL32(40, 235, 70, 245);
                mid_core   = IM_COL32(110, 255, 140, 255);
                high_outer = IM_COL32(140, 255, 200, 245);
                high_core  = IM_COL32(220, 255, 235, 255);
            } else if (eff_color_mode == 6) {
                // VAPORWAVE
                low_outer  = (cur_ph == KuroAudio::TrackPhraseType::DROP) ? IM_COL32(255, 25, 70, 245) : IM_COL32(120, 12, 195, 245);
                low_core   = (cur_ph == KuroAudio::TrackPhraseType::DROP) ? IM_COL32(255, 130, 185, 255) : IM_COL32(195, 65, 255, 255);
                mid_outer  = IM_COL32(255, 25, 180, 245);
                mid_core   = IM_COL32(255, 120, 220, 255);
                high_outer = IM_COL32(50, 240, 255, 245);
                high_core  = IM_COL32(200, 255, 255, 255);
            } else if (eff_color_mode == 7) {
                // INFERNO MAGMA
                low_outer  = IM_COL32(220, 10, 10, 245);
                low_core   = IM_COL32(255, 65, 45, 255);
                mid_outer  = IM_COL32(255, 100, 0, 245);
                mid_core   = IM_COL32(255, 165, 50, 255);
                high_outer = IM_COL32(255, 230, 70, 245);
                high_core  = IM_COL32(255, 255, 200, 255);
            } else if (eff_color_mode == 8) {
                // GLACIER FROST
                low_outer  = (cur_ph == KuroAudio::TrackPhraseType::DROP) ? IM_COL32(255, 25, 70, 245) : IM_COL32(10, 50, 160, 245);
                low_core   = (cur_ph == KuroAudio::TrackPhraseType::DROP) ? IM_COL32(255, 130, 185, 255) : IM_COL32(45, 140, 255, 255);
                mid_outer  = IM_COL32(0, 185, 235, 245);
                mid_core   = IM_COL32(80, 225, 255, 255);
                high_outer = IM_COL32(215, 250, 255, 245);
                high_core  = IM_COL32(255, 255, 255, 255);
            } else if (eff_color_mode == 9) {
                // MONOCHROME PLATINUM
                low_outer  = IM_COL32(55, 60, 70, 245);
                low_core   = IM_COL32(125, 135, 150, 255);
                mid_outer  = IM_COL32(155, 165, 180, 245);
                mid_core   = IM_COL32(215, 225, 240, 255);
                high_outer = IM_COL32(245, 250, 255, 245);
                high_core  = IM_COL32(255, 255, 255, 255);
            } else {
                // Default placeholders para compilação antes do cálculo por ponto (RGB)
                low_outer  = IM_COL32(255, 0, 110, 245);
                low_core   = IM_COL32(255, 120, 185, 255);
                mid_outer  = IM_COL32(255, 160, 20, 245);
                mid_core   = IM_COL32(255, 215, 60, 255);
                high_outer = IM_COL32(0, 229, 255, 245);
                high_core  = IM_COL32(180, 255, 255, 255);
            }

            const size_t pt_count = rib_pts.size();
            for (size_t i = 0; i < pt_count; i++) {
                const auto& p = rib_pts[i];
                if (p.x < pos.x || p.x > pos.x + width) continue;

                float cur_x = std::round(p.x);
                if (p.is_tail || !p.active) {
                    ImU32 lead_col = IM_COL32(35, 48, 68, 140);
                    dl->AddLine(ImVec2(cur_x, mid_y), ImVec2(cur_x + 1.0f, mid_y), lead_col, 1.0f);
                    if (((int)cur_x) % 32 < 2) {
                        dl->AddLine(ImVec2(cur_x, mid_y - 3.5f), ImVec2(cur_x, mid_y + 3.5f), IM_COL32(65, 88, 120, 160), 1.0f);
                    }
                    continue;
                }

                // Alturas de cada banda calibradas com o punch de transientes (Envelope suave sem jitter)
                float amp_l = std::clamp(p.low_amp * (gain_fused * punch * 0.95f), 1.5f, max_half_h);
                float y_l_top = mid_y - amp_l;
                float y_l_bot = mid_y + amp_l;

                float amp_m = std::clamp(p.mid_amp * (gain_fused * punch), 1.2f, max_half_h * 0.88f);
                float y_m_top = mid_y - amp_m;
                float y_m_bot = mid_y + amp_m;

                float amp_h = std::clamp(p.high_amp * (gain_fused * punch * 1.15f), 1.0f, max_half_h * 0.78f);
                float y_h_top = mid_y - amp_h;
                float y_h_bot = mid_y + amp_h;

                // Colorimetria dinâmica no Modo RGB Espectral (elimina o visual cinza/apagado)
                ImU32 col_outer_cur = low_outer;
                ImU32 col_core_cur  = low_core;

                if (eff_color_mode == 0) {
                    float l_v = (p.low > 0.001f) ? p.low : std::clamp(amp_l / (max_half_h * 0.70f), 0.0f, 1.0f);
                    float m_v = (p.mid > 0.001f) ? p.mid : std::clamp(amp_m / (max_half_h * 0.70f), 0.0f, 1.0f);
                    float h_v = (p.high > 0.001f) ? p.high : std::clamp(amp_h / (max_half_h * 0.70f), 0.0f, 1.0f);

                    // Mapeamento RGB Espectral Rekordbox / Traktor Oficial:
                    // Graves = Magenta/Rosa Shock (#FF007F / #E6007A)
                    // Médios = Âmbar/Ouro Solar (#FF9500 / #FFA000)
                    // Agudos = Ciano Elétrico/Branco (#00E5FF / #00FFEE)
                    float r = l_v * 255.0f + m_v * 235.0f + h_v * 30.0f;
                    float g = m_v * 160.0f + h_v * 225.0f;
                    float b = l_v * 140.0f + h_v * 255.0f;

                    if (l_v > 0.25f && m_v < 0.35f) g *= 0.65f;
                    if (h_v > 0.15f && l_v > 0.18f) { r = std::max(r, 240.0f); b = std::max(b, 210.0f); }

                    float max_c = std::max({r, g, b, 1.0f});
                    float boost = 255.0f / max_c;
                    r = std::clamp(r * boost, 0.0f, 255.0f);
                    g = std::clamp(g * boost, 0.0f, 255.0f);
                    b = std::clamp(b * boost, 0.0f, 255.0f);

                    col_outer_cur = IM_COL32((int)(r * 0.75f), (int)(g * 0.70f), (int)(b * 0.75f), 245);
                    col_core_cur  = IM_COL32((int)r, (int)g, (int)b, 255);
                }

                if (line_style == 0) {
                    // MODO 0: LINHAS VERTICAIS NÍTIDAS REKORDBOX 7 (ESTILO LINHAS COM RESPIRO)
                    if (eff_color_mode == 0) {
                        // RGB Espectral: Linha vertical unificada com a cor espectral vibrante do ponto
                        float y_top = std::min({y_l_top, y_m_top, y_h_top});
                        float y_bot = std::max({y_l_bot, y_m_bot, y_h_bot});
                        dl->AddLine(ImVec2(cur_x, y_top), ImVec2(cur_x, y_bot), col_outer_cur, eff_line_w);

                        if (core_bright > 0.05f) {
                            float core_w = std::max(0.6f, eff_line_w * 0.70f);
                            float c_top = mid_y - (mid_y - y_top) * 0.52f;
                            float c_bot = mid_y + (y_bot - mid_y) * 0.52f;
                            dl->AddLine(ImVec2(cur_x, c_top), ImVec2(cur_x, c_bot), col_core_cur, core_w);
                        }

                        // Agulhas de transientes de agudos nas pontas (Needle Highlights)
                        if (p.high > 0.18f || amp_h > 3.0f) {
                            float tip_len = std::min(4.0f, (mid_y - y_top) * 0.22f);
                            dl->AddLine(ImVec2(cur_x, y_top), ImVec2(cur_x, y_top + tip_len), IM_COL32(0, 240, 255, 245), eff_line_w);
                            dl->AddLine(ImVec2(cur_x, y_bot - tip_len), ImVec2(cur_x, y_bot), IM_COL32(0, 240, 255, 245), eff_line_w);
                        }
                    } else {
                        // 3-Band / Blue / Neon / Sunset: Camadas sobrepostas com espessuras proporcionais
                        // Linha dos graves (mais ampla)
                        dl->AddLine(ImVec2(cur_x, y_l_top), ImVec2(cur_x, y_l_bot), low_outer, eff_line_w);
                        // Médios (corpo)
                        if (amp_m > 2.0f) {
                            dl->AddLine(ImVec2(cur_x, y_m_top), ImVec2(cur_x, y_m_bot), mid_outer, std::max(0.6f, eff_line_w * 0.85f));
                        }
                        // Agudos (agulha fina, não oculta médios/graves)
                        if (amp_h > 1.8f) {
                            dl->AddLine(ImVec2(cur_x, y_h_top), ImVec2(cur_x, y_h_bot), high_outer, std::max(0.5f, eff_line_w * 0.45f));
                        }
                        // Miolo luminoso
                        if (core_bright > 0.05f) {
                            float core_w = std::max(0.5f, eff_line_w * 0.35f);
                            if (amp_l > 3.5f) dl->AddLine(ImVec2(cur_x, mid_y - (mid_y - y_l_top)*0.5f), ImVec2(cur_x, mid_y + (y_l_bot - mid_y)*0.5f), low_core, core_w);
                            if (amp_m > 3.5f) dl->AddLine(ImVec2(cur_x, mid_y - (mid_y - y_m_top)*0.5f), ImVec2(cur_x, mid_y + (y_m_bot - mid_y)*0.5f), mid_core, core_w);
                            if (amp_h > 2.8f) dl->AddLine(ImVec2(cur_x, mid_y - (mid_y - y_h_top)*0.5f), ImVec2(cur_x, mid_y + (y_h_bot - mid_y)*0.5f), high_core, core_w);
                        }
                    }
                } else if (line_style == 1) {
                    // MODO 1: BARRAS COM RESPIRO (DISCRETE STEMS COM INTERVALO)
                    float eff_w = std::max(1.0f, line_w - line_gap * 0.5f);
                    float x1 = std::floor(cur_x - eff_w * 0.5f);
                    float x2 = x1 + eff_w;

                    if (eff_color_mode == 0) {
                        float y_top = std::min({y_l_top, y_m_top, y_h_top});
                        float y_bot = std::max({y_l_bot, y_m_bot, y_h_bot});
                        dl->AddRectFilled(ImVec2(x1, y_top), ImVec2(x2, y_bot), col_outer_cur);
                        if (core_bright > 0.1f) {
                            dl->AddRectFilled(ImVec2(x1, mid_y - (mid_y - y_top)*0.5f), ImVec2(x2, mid_y + (y_bot - mid_y)*0.5f), col_core_cur);
                        }
                    } else {
                        dl->AddRectFilled(ImVec2(x1, y_l_top), ImVec2(x2, y_l_bot), low_outer);
                        if (amp_m > 2.0f) dl->AddRectFilled(ImVec2(x1, y_m_top), ImVec2(x2, y_m_bot), mid_outer);
                        if (amp_h > 1.8f) dl->AddRectFilled(ImVec2(x1, y_h_top), ImVec2(x2, y_h_bot), high_outer);
                        if (core_bright > 0.1f) {
                            dl->AddRectFilled(ImVec2(x1, mid_y - (mid_y - y_l_top)*0.5f), ImVec2(x2, mid_y + (y_l_bot - mid_y)*0.5f), low_core);
                            dl->AddRectFilled(ImVec2(x1, mid_y - (mid_y - y_m_top)*0.5f), ImVec2(x2, mid_y + (y_m_bot - mid_y)*0.5f), mid_core);
                        }
                    }
                } else {
                    // MODO 2: MALHA SÓLIDA CONTÍNUA (LEGACY MESH)
                    float x1 = std::floor(cur_x);
                    float x2 = x1 + 1.0f;
                    if (eff_color_mode == 0) {
                        float y_top = std::min({y_l_top, y_m_top, y_h_top});
                        float y_bot = std::max({y_l_bot, y_m_bot, y_h_bot});
                        dl->AddRectFilledMultiColor(ImVec2(x1, y_top), ImVec2(x2, mid_y), col_outer_cur, col_outer_cur, col_core_cur, col_core_cur);
                        dl->AddRectFilledMultiColor(ImVec2(x1, mid_y), ImVec2(x2, y_bot), col_core_cur, col_core_cur, col_outer_cur, col_outer_cur);
                    } else {
                        dl->AddRectFilledMultiColor(ImVec2(x1, y_l_top), ImVec2(x2, mid_y), low_outer, low_outer, low_core, low_core);
                        dl->AddRectFilledMultiColor(ImVec2(x1, mid_y), ImVec2(x2, y_l_bot), low_core, low_core, low_outer, low_outer);
                        dl->AddRectFilledMultiColor(ImVec2(x1, y_m_top), ImVec2(x2, mid_y), mid_outer, mid_outer, mid_core, mid_core);
                        dl->AddRectFilledMultiColor(ImVec2(x1, mid_y), ImVec2(x2, y_m_bot), mid_core, mid_core, mid_outer, mid_outer);
                        dl->AddRectFilledMultiColor(ImVec2(x1, y_h_top), ImVec2(x2, mid_y), high_outer, high_outer, high_core, high_core);
                        dl->AddRectFilledMultiColor(ImVec2(x1, mid_y), ImVec2(x2, y_h_bot), high_core, high_core, high_outer, high_outer);
                    }
                }
            }

            // Linha central neutra 1px
            dl->AddLine(ImVec2(pos.x, mid_y), ImVec2(pos.x + width, mid_y), IM_COL32(255, 255, 255, 160), 1.0f);

            // Playhead, Metrônomo e Badges
            renderPlayheadAndBadges(dl, pos, width, height, center_x, cur_b_phase, cur_b_in_bar, is_playing, cur_ph);
        }

        // =========================================================================
        // MODO 3: FUSÃO HÍBRIDA (REKORDBOX CONTINUOUS MESH + FITAS NEON RIBBON)
        // Combina o miolo denso de estriação contínua Rekordbox com o contorno luminoso
        // e glow de fitas desacopladas estilo Guitar Hero.
        // =========================================================================
        void renderHybridFusionDeckLane(
            ImDrawList* dl,
            const std::vector<RibbonPoint>& rib_pts,
            ImVec2 pos,
            float width,
            float height,
            float center_x,
            double cur_b_phase,
            int cur_b_in_bar,
            bool is_playing,
            KuroAudio::TrackPhraseType cur_ph,
            int color_mode = -1
        ) {
            // Renderiza a base contínua de mesh 3-Band Rekordbox
            renderRekordbox3BandMultiLane(dl, rib_pts, pos, width, height, center_x,
                                          cur_b_phase, cur_b_in_bar, is_playing, cur_ph, color_mode);

            if (rib_pts.size() < 2) return;

            auto& metrics = DJLayoutMetrics::getInstance();
            int eff_color_mode = (color_mode >= 0) ? color_mode : metrics.waveform_color_mode;
            float high_ratio = metrics.ribbon_lane_high_ratio;
            float mid_ratio  = metrics.ribbon_lane_mid_ratio;
            float low_ratio  = metrics.ribbon_lane_low_ratio;
            float total_r = high_ratio + mid_ratio + low_ratio;
            if (total_r <= 0.01f) { high_ratio = 0.28f; mid_ratio = 0.36f; low_ratio = 0.36f; total_r = 1.0f; }

            float high_h = height * (high_ratio / total_r);
            float mid_h  = height * (mid_ratio  / total_r);
            float low_h  = height * (low_ratio  / total_r);

            float high_base_y = pos.y + high_h * 0.5f;
            float mid_base_y  = pos.y + high_h + mid_h * 0.5f;
            float low_base_y  = pos.y + high_h + mid_h + low_h * 0.5f;

            float max_high_sub_h = (high_h * 0.5f) * 0.88f;
            float max_mid_sub_h  = (mid_h  * 0.5f) * 0.88f;
            float max_low_sub_h  = (low_h  * 0.5f) * 0.88f;
            float gain_elongate = 1.25f;

            ImU32 high_neon = (eff_color_mode == 0) ? IM_COL32(0, 240, 255, 240) : IM_COL32(0, 240, 255, 230);
            ImU32 mid_neon  = (eff_color_mode == 0) ? IM_COL32(255, 175, 25, 240) : IM_COL32(255, 185, 35, 230);
            ImU32 low_neon  = (eff_color_mode == 0) ? IM_COL32(255, 0, 110, 250) : ((cur_ph == KuroAudio::TrackPhraseType::DROP) ? IM_COL32(255, 25, 70, 255) : IM_COL32(0, 160, 255, 230));

            // Traço vetorial de contorno neon de precisão sobre as estrias
            const size_t pt_count = rib_pts.size();
            for (size_t i = 0; i + 1 < pt_count; i++) {
                const auto& p1 = rib_pts[i];
                const auto& p2 = rib_pts[i + 1];
                if (p2.x < pos.x || p1.x > pos.x + width) continue;

                // High contour
                float h1 = std::clamp(p1.high_amp * gain_elongate, 2.0f, max_high_sub_h);
                float h2 = std::clamp(p2.high_amp * gain_elongate, 2.0f, max_high_sub_h);
                dl->AddLine(ImVec2(p1.x, high_base_y - h1), ImVec2(p2.x, high_base_y - h2), high_neon, 1.5f);
                dl->AddLine(ImVec2(p1.x, high_base_y + h1), ImVec2(p2.x, high_base_y + h2), high_neon, 1.5f);

                // Mid contour
                float m1 = std::clamp(p1.mid_amp * gain_elongate, 2.0f, max_mid_sub_h);
                float m2 = std::clamp(p2.mid_amp * gain_elongate, 2.0f, max_mid_sub_h);
                dl->AddLine(ImVec2(p1.x, mid_base_y - m1), ImVec2(p2.x, mid_base_y - m2), mid_neon, 1.5f);
                dl->AddLine(ImVec2(p1.x, mid_base_y + m1), ImVec2(p2.x, mid_base_y + m2), mid_neon, 1.5f);

                // Low contour
                float l1 = std::clamp(p1.low_amp * (gain_elongate * 0.95f), 2.0f, max_low_sub_h);
                float l2 = std::clamp(p2.low_amp * (gain_elongate * 0.95f), 2.0f, max_low_sub_h);
                dl->AddLine(ImVec2(p1.x, low_base_y - l1), ImVec2(p2.x, low_base_y - l2), low_neon, 1.7f);
                dl->AddLine(ImVec2(p1.x, low_base_y + l1), ImVec2(p2.x, low_base_y + l2), low_neon, 1.7f);
            }
        }

        // =========================================================================
        // MODO 4: MULTI-RIBBON DESACOPLADO (ESTILO VOCAL GUITAR HERO ORIGINAL)
        // 3 Fitas independentes empilhadas com preenchimento vetorial e contornos neon
        // =========================================================================
        void renderMultiRibbonDeckLane(
            ImDrawList* dl,
            const std::vector<RibbonPoint>& rib_pts,
            ImVec2 pos,
            float width,
            float height,
            float center_x,
            double cur_b_phase,
            int cur_b_in_bar,
            bool is_playing,
            KuroAudio::TrackPhraseType cur_ph,
            int color_mode = -1
        ) {
            if (!dl || rib_pts.size() < 2 || width <= 10.0f || height <= 10.0f) return;

            auto& metrics = DJLayoutMetrics::getInstance();
            int eff_color_mode = (color_mode >= 0) ? color_mode : metrics.waveform_color_mode;

            // 1. Fundo da baia de waveforms com gradiente sutil
            ImU32 bg_color_top = IM_COL32(11, 14, 20, 240);
            ImU32 bg_color_bot = IM_COL32(16, 19, 28, 245);
            dl->AddRectFilledMultiColor(pos, ImVec2(pos.x + width, pos.y + height),
                                        bg_color_top, bg_color_top, bg_color_bot, bg_color_bot);

            ImU32 phrase_tint = KuroAudio::DJMIRPhrasingEngine::getPhraseColor(cur_ph, false, 0.08f);
            dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), phrase_tint);

            // 2. Cálculo das alturas das 3 baias empilhadas
            float high_ratio = metrics.ribbon_lane_high_ratio;
            float mid_ratio  = metrics.ribbon_lane_mid_ratio;
            float low_ratio  = metrics.ribbon_lane_low_ratio;
            float total_r = high_ratio + mid_ratio + low_ratio;
            if (total_r <= 0.01f) { high_ratio = 0.28f; mid_ratio = 0.36f; low_ratio = 0.36f; total_r = 1.0f; }

            float high_h = height * (high_ratio / total_r);
            float mid_h  = height * (mid_ratio  / total_r);
            float low_h  = height * (low_ratio  / total_r);

            float high_base_y = pos.y + high_h * 0.5f;
            float mid_base_y  = pos.y + high_h + mid_h * 0.5f;
            float low_base_y  = pos.y + high_h + mid_h + low_h * 0.5f;

            // Linhas divisórias sutis de lane
            ImU32 div_col = IM_COL32(255, 255, 255, 20);
            dl->AddLine(ImVec2(pos.x, pos.y + high_h), ImVec2(pos.x + width, pos.y + high_h), div_col, 1.0f);
            dl->AddLine(ImVec2(pos.x, pos.y + high_h + mid_h), ImVec2(pos.x + width, pos.y + high_h + mid_h), div_col, 1.0f);

            // Cores base das 3 fitas (Guitar Hero Multi-Ribbon adaptadas à paleta de cor)
            ImU32 high_edge_col = (eff_color_mode == 0) ? IM_COL32(0, 240, 255, 245) : IM_COL32(0, 240, 255, 240);
            ImU32 high_fill_col = (eff_color_mode == 0) ? IM_COL32(0, 200, 255, 90)  : IM_COL32(0, 180, 230, 80);

            ImU32 mid_edge_col  = (eff_color_mode == 0) ? IM_COL32(255, 175, 25, 245) : IM_COL32(255, 185, 35, 240);
            ImU32 mid_fill_col  = (eff_color_mode == 0) ? IM_COL32(245, 150, 15, 95)  : IM_COL32(230, 140, 20, 85);

            ImU32 low_edge_col  = (eff_color_mode == 0) ? IM_COL32(255, 0, 110, 250) : IM_COL32(255, 45, 160, 245);
            ImU32 low_fill_col  = (eff_color_mode == 0) ? IM_COL32(220, 0, 95, 105)  : IM_COL32(200, 20, 120, 95);

            if (cur_ph == KuroAudio::TrackPhraseType::DROP && eff_color_mode != 0) {
                low_edge_col = IM_COL32(255, 25, 60, 255);
                low_fill_col = IM_COL32(255, 40, 40, 120);
            }

            float max_high_sub_h = (high_h * 0.5f) * 0.88f;
            float max_mid_sub_h  = (mid_h  * 0.5f) * 0.88f;
            float max_low_sub_h  = (low_h  * 0.5f) * 0.88f;
            float gain_elongate = 1.25f;

            // 3. Renderização contínua das 3 fitas empilhadas
            const size_t pt_count = rib_pts.size();
            for (size_t i = 0; i + 1 < pt_count; i++) {
                const auto& p1 = rib_pts[i];
                const auto& p2 = rib_pts[i + 1];

                if (p2.x < pos.x || p1.x > pos.x + width) continue;

                // --- HIGH RIBBON ---
                float h1 = std::clamp(p1.high_amp * gain_elongate, 1.2f, max_high_sub_h);
                float h2 = std::clamp(p2.high_amp * gain_elongate, 1.2f, max_high_sub_h);
                ImVec2 h_top1(p1.x, high_base_y - h1);
                ImVec2 h_bot1(p1.x, high_base_y + h1);
                ImVec2 h_top2(p2.x, high_base_y - h2);
                ImVec2 h_bot2(p2.x, high_base_y + h2);
                dl->AddQuadFilled(h_top1, h_top2, h_bot2, h_bot1, high_fill_col);
                dl->AddLine(h_top1, h_top2, high_edge_col, 1.8f);
                dl->AddLine(h_bot1, h_bot2, high_edge_col, 1.8f);

                // --- MID RIBBON ---
                float m1 = std::clamp(p1.mid_amp * gain_elongate, 1.2f, max_mid_sub_h);
                float m2 = std::clamp(p2.mid_amp * gain_elongate, 1.2f, max_mid_sub_h);
                ImVec2 m_top1(p1.x, mid_base_y - m1);
                ImVec2 m_bot1(p1.x, mid_base_y + m1);
                ImVec2 m_top2(p2.x, mid_base_y - m2);
                ImVec2 m_bot2(p2.x, mid_base_y + m2);
                dl->AddQuadFilled(m_top1, m_top2, m_bot2, m_bot1, mid_fill_col);
                dl->AddLine(m_top1, m_top2, mid_edge_col, 1.8f);
                dl->AddLine(m_bot1, m_bot2, mid_edge_col, 1.8f);

                // --- LOW RIBBON ---
                float l1 = std::clamp(p1.low_amp * (gain_elongate * 0.95f), 1.2f, max_low_sub_h);
                float l2 = std::clamp(p2.low_amp * (gain_elongate * 0.95f), 1.2f, max_low_sub_h);
                ImVec2 l_top1(p1.x, low_base_y - l1);
                ImVec2 l_bot1(p1.x, low_base_y + l1);
                ImVec2 l_top2(p2.x, low_base_y - l2);
                ImVec2 l_bot2(p2.x, low_base_y + l2);
                dl->AddQuadFilled(l_top1, l_top2, l_bot2, l_bot1, low_fill_col);
                dl->AddLine(l_top1, l_top2, low_edge_col, 2.0f);
                dl->AddLine(l_bot1, l_bot2, low_edge_col, 2.0f);
            }

            // 4. Linhas neutras de zero-crossing (centro de cada fita)
            ImU32 zero_cross_col = IM_COL32(255, 255, 255, 45);
            dl->AddLine(ImVec2(pos.x, high_base_y), ImVec2(pos.x + width, high_base_y), zero_cross_col, 1.0f);
            dl->AddLine(ImVec2(pos.x, mid_base_y),  ImVec2(pos.x + width, mid_base_y),  zero_cross_col, 1.0f);
            dl->AddLine(ImVec2(pos.x, low_base_y),  ImVec2(pos.x + width, low_base_y),  zero_cross_col, 1.0f);

            // 5. Playhead, Metrônomo e Badges
            renderPlayheadAndBadges(dl, pos, width, height, center_x, cur_b_phase, cur_b_in_bar, is_playing, cur_ph);
        }
    };

} // namespace KuroUI
