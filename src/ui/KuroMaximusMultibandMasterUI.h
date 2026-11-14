#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

#ifndef ICON_FA_DNA
#define ICON_FA_DNA "\xef\x81\xb1"
#endif

namespace KuroUI {

    class KuroMaximusMultibandMasterUI {
    private:
        bool is_open = false;

        // Band Frequencies (Low/Mid split, Mid/High split)
        float low_mid_split = 200.0f;   // Hz (Low/Mid Crossover)
        float mid_high_split = 4000.0f; // Hz (Mid/High Crossover)

        // Low Band (Bass / Sub 20 - 200 Hz)
        float low_pre_gain = 1.5f;       // dB
        float low_post_gain = 0.5f;      // dB
        float low_threshold = -14.0f;    // dB
        float low_ratio = 3.5f;          // :1
        float low_attack = 12.0f;        // ms
        float low_release = 140.0f;      // ms
        bool low_solo = false;
        bool low_mute = false;

        // Mid Band (Presence / Vocals / Instruments 200 - 4000 Hz)
        float mid_pre_gain = 0.5f;       // dB
        float mid_post_gain = 0.0f;      // dB
        float mid_threshold = -16.0f;    // dB
        float mid_ratio = 2.5f;          // :1
        float mid_attack = 18.0f;        // ms
        float mid_release = 160.0f;      // ms
        bool mid_solo = false;
        bool mid_mute = false;

        // High Band (Air / Cymbals / Sparkle 4000 - 20000 Hz)
        float high_pre_gain = 2.0f;      // dB
        float high_post_gain = 1.0f;     // dB
        float high_threshold = -18.0f;   // dB
        float high_ratio = 2.0f;         // :1
        float high_attack = 6.0f;        // ms
        float high_release = 110.0f;     // ms
        bool high_solo = false;
        bool high_mute = false;

        // Master Limiter, Saturation & Stereo Expander
        float master_ceiling = -0.1f;    // dBFS True Peak
        float master_saturation = 0.35f; // Tape / Tube Warmth
        float stereo_enhancer_width = 1.25f; // 125% Stereo Width Spread
        bool auto_gain_compensation = true;
        bool linear_phase_oversampling = true;

        // Metering state & history buffers
        float integrated_lufs = -9.0f;
        float short_term_lufs = -9.2f;
        float momentary_lufs = -8.4f;
        float loudness_range_lra = 5.2f;
        float true_peak_l = -0.1f;
        float true_peak_r = -0.1f;
        bool true_peak_clip = false;

        // Radar circular buffer (64 polar samples)
        std::vector<float> radar_history;
        float radar_sweep_angle = 0.0f;

        // 3D Goniometer rotation angles
        float gonio_rot_x = 0.45f;
        float gonio_rot_y = 0.75f;
        bool gonio_auto_rotate = true;

    public:
        KuroMaximusMultibandMasterUI() {
            radar_history.resize(64, 0.45f);
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

    private:
        // Helper: 3D point projection to 2D screen coordinates
        ImVec2 Project3D(float x, float y, float z, ImVec2 center, float scale) {
            // Rotate around Y axis
            float cos_y = std::cos(gonio_rot_y), sin_y = std::sin(gonio_rot_y);
            float x1 = x * cos_y + z * sin_y;
            float y1 = y;
            float z1 = -x * sin_y + z * cos_y;

            // Rotate around X axis
            float cos_x = std::cos(gonio_rot_x), sin_x = std::sin(gonio_rot_x);
            float x2 = x1;
            float y2 = y1 * cos_x - z1 * sin_x;
            float z2 = y1 * sin_x + z1 * cos_x;

            // Isometric projection with subtle depth perspective
            float depth = 1.0f / (1.0f + z2 * 0.003f);
            return ImVec2(center.x + x2 * scale * depth, center.y - y2 * scale * depth);
        }

        // Helper: Custom Futuristic Circular Knob with glowing arc
        void DrawMaximusKnob(const char* id, const char* label, float* val, float min_v, float max_v, const char* fmt, ImU32 accent_col, float radius = 18.0f) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImVec2 center = ImVec2(p.x + radius + 14.0f, p.y + radius + 2.0f);

            ImGui::InvisibleButton(id, ImVec2((radius + 14.0f) * 2.0f, radius * 2.0f + 24.0f));
            bool is_active = ImGui::IsItemActive();
            bool is_hovered = ImGui::IsItemHovered();

            if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                float delta = -ImGui::GetIO().MouseDelta.y * 0.006f;
                *val = std::clamp(*val + delta * (max_v - min_v), min_v, max_v);
            }

            float norm = std::clamp((*val - min_v) / (max_v - min_v), 0.0f, 1.0f);
            float angle_min = 2.4f; // ~137 deg
            float angle_max = 7.0f; // ~400 deg
            float current_angle = angle_min + norm * (angle_max - angle_min);

            // Outer dark track ring
            dl->AddCircle(center, radius, IM_COL32(20, 26, 36, 255), 32, 3.5f);

            // Active glowing arc
            int num_segments = 24;
            for (int i = 0; i < num_segments; ++i) {
                float a0 = angle_min + (float)i / (float)num_segments * (current_angle - angle_min);
                float a1 = angle_min + (float)(i + 1) / (float)num_segments * (current_angle - angle_min);
                if (a0 >= current_angle) break;
                a1 = std::min(a1, current_angle);
                dl->AddLine(
                    ImVec2(center.x + std::cos(a0) * radius, center.y + std::sin(a0) * radius),
                    ImVec2(center.x + std::cos(a1) * radius, center.y + std::sin(a1) * radius),
                    accent_col, 3.5f
                );
            }

            // Knob Center Body
            dl->AddCircleFilled(center, radius - 4.0f, is_hovered ? IM_COL32(28, 38, 52, 255) : IM_COL32(16, 22, 32, 255), 24);
            dl->AddCircle(center, radius - 4.0f, is_active ? accent_col : IM_COL32(45, 60, 80, 255), 24, 1.5f);

            // Indicator Needle
            ImVec2 ind_p(center.x + std::cos(current_angle) * (radius - 5.0f), center.y + std::sin(current_angle) * (radius - 5.0f));
            dl->AddLine(center, ind_p, IM_COL32(255, 255, 255, 240), 2.0f);

            // Value text below
            char val_str[32];
            snprintf(val_str, sizeof(val_str), fmt, *val);
            ImVec2 val_sz = ImGui::CalcTextSize(val_str);
            dl->AddText(ImVec2(center.x - val_sz.x * 0.5f, center.y + radius + 4.0f), IM_COL32(210, 225, 240, 230), val_str);

            // Label text below value
            ImVec2 lbl_sz = ImGui::CalcTextSize(label);
            dl->AddText(ImVec2(center.x - lbl_sz.x * 0.5f, center.y + radius + 17.0f), IM_COL32(130, 150, 175, 200), label);
        }

    public:
        void Render() {
            if (!is_open) return;

            float cur_time = (float)ImGui::GetTime();

            // Dynamic meter animation simulation
            integrated_lufs = -9.0f + std::sin(cur_time * 0.5f) * 0.2f;
            momentary_lufs = -8.5f + std::sin(cur_time * 2.8f) * 1.8f;
            short_term_lufs = -9.1f + std::sin(cur_time * 1.1f) * 0.6f;
            true_peak_l = -0.1f + std::sin(cur_time * 3.4f) * 0.05f;
            true_peak_r = -0.1f + std::cos(cur_time * 3.1f) * 0.05f;

            // Update radar sweep angle
            radar_sweep_angle += ImGui::GetIO().DeltaTime * 0.9f;
            if (radar_sweep_angle > 6.28318f) radar_sweep_angle -= 6.28318f;

            // Update radar polar history
            int sweep_idx = (int)((radar_sweep_angle / 6.28318f) * (float)radar_history.size()) % radar_history.size();
            float sim_lufs_norm = std::clamp((momentary_lufs + 32.0f) / 32.0f, 0.15f, 0.95f);
            radar_history[sweep_idx] = sim_lufs_norm;

            // Auto-rotate 3D goniometer
            if (gonio_auto_rotate) {
                gonio_rot_y += ImGui::GetIO().DeltaTime * 0.45f;
            }

            ImGui::SetNextWindowPos(ImVec2(10, 68), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(625, 625), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.035f, 0.045f, 0.065f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.70f)); // Neon Cyan
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

            if (ImGui::Begin("🎚️ MASTER BUS MULTIBAND MASTERING RACK (MAXIMUS DYNAMICS)###MaximusWindow", &is_open, ImGuiWindowFlags_MenuBar)) {

                // ── MENU BAR DE PRESETS E FERRAMENTAS ────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu(ICON_FA_SLIDERS " Presets de Masterização")) {
                        if (ImGui::MenuItem("🔥 Transparent Master Peak Limiter (-9 LUFS - Club Ready)")) {
                            master_saturation = 0.20f; stereo_enhancer_width = 1.10f;
                            low_threshold = -14.0f; low_ratio = 3.5f;
                            mid_threshold = -16.0f; mid_ratio = 2.5f;
                            high_threshold = -18.0f; high_ratio = 2.0f;
                        }
                        if (ImGui::MenuItem("👽 Psytrance Punch & Maximum Sub Pressure")) {
                            master_saturation = 0.45f; stereo_enhancer_width = 1.35f;
                            low_threshold = -16.0f; low_ratio = 4.2f; low_attack = 8.0f;
                            mid_threshold = -15.0f; mid_ratio = 3.0f;
                            high_threshold = -17.0f; high_ratio = 2.4f;
                        }
                        if (ImGui::MenuItem("🌐 Clean Broadcast Loudness (-14 LUFS - Spotify / Apple Music)")) {
                            master_saturation = 0.10f; stereo_enhancer_width = 1.05f;
                            low_threshold = -18.0f; low_ratio = 2.5f;
                            mid_threshold = -19.0f; mid_ratio = 2.0f;
                            high_threshold = -20.0f; high_ratio = 1.8f;
                        }
                        if (ImGui::MenuItem("🌌 Wide & Warm Tube Tape Saturation (Analog Warmth)")) {
                            master_saturation = 0.70f; stereo_enhancer_width = 1.45f;
                            low_threshold = -13.0f; low_ratio = 3.0f;
                            mid_threshold = -14.0f; mid_ratio = 2.2f;
                            high_threshold = -16.0f; high_ratio = 2.0f;
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu(ICON_FA_GEARS " Oversampling")) {
                        if (ImGui::MenuItem("1x (Direct Realtime)", "", !linear_phase_oversampling)) linear_phase_oversampling = false;
                        if (ImGui::MenuItem("4x Linear-Phase True Peak Oversampling", "", linear_phase_oversampling)) linear_phase_oversampling = true;
                        ImGui::EndMenu();
                    }

                    float menu_w = ImGui::GetWindowWidth();
                    ImGui::SameLine(menu_w - 180.0f);
                    ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "⚡ LATENCY: 0.8ms (4x FIR)");
                    ImGui::EndMenuBar();
                }

                // ── CABEÇALHO FUTURISTA CYBERPUNK ────────────────────────────
                ImVec2 avail = ImGui::GetContentRegionAvail();
                float col_w = (avail.x - 12.0f) * 0.5f;
                float top_row_h = 240.0f;
                float bot_row_h = avail.y - top_row_h - 14.0f;

                // =============================================================
                // ── SEÇÃO SUPERIOR: ESQUERDA (CROSSOVER) | DIREITA (LUFS RADAR)
                // =============================================================
                ImGui::BeginChild("MaximusTopLeftChild", ImVec2(col_w, top_row_h), true);
                {
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImGui::TextColored(ImVec4(0.85f, 0.90f, 0.95f, 1.0f), "3-band Maximus compressor crossover display");
                    ImGui::SameLine(col_w - 75.0f);
                    ImGui::TextDisabled("Maximus");

                    ImVec2 g_pos = ImGui::GetCursorScreenPos();
                    float g_w = col_w - 24.0f;
                    float g_h = top_row_h - 48.0f;
                    ImVec2 g_p1 = ImVec2(g_pos.x + g_w, g_pos.y + g_h);

                    // Fundo da grade espectral
                    dl->AddRectFilled(g_pos, g_p1, IM_COL32(10, 14, 20, 255), 4.0f);
                    dl->AddRect(g_pos, g_p1, IM_COL32(30, 42, 58, 255), 4.0f);

                    // Posições X dos divisores de frequência logarítmicos
                    float x_low_mid = g_pos.x + (std::log10(low_mid_split / 20.0f) / 3.0f) * (g_w - 26.0f);
                    float x_mid_high = g_pos.x + (std::log10(mid_high_split / 20.0f) / 3.0f) * (g_w - 26.0f);
                    x_low_mid = std::clamp(x_low_mid, g_pos.x + 40.0f, g_p1.x - 60.0f);
                    x_mid_high = std::clamp(x_mid_high, x_low_mid + 30.0f, g_p1.x - 30.0f);

                    // Gradientes das 3 Bandas Coloridas:
                    // Low Band (Vermelho/Coral #E53935)
                    dl->AddRectFilledMultiColor(g_pos, ImVec2(x_low_mid, g_p1.y),
                        IM_COL32(220, 50, 60, 45), IM_COL32(180, 40, 50, 20),
                        IM_COL32(140, 30, 40, 60), IM_COL32(230, 45, 55, 80));

                    // Mid Band (Verde Neon / Esmeralda #00E676)
                    dl->AddRectFilledMultiColor(ImVec2(x_low_mid, g_pos.y), ImVec2(x_mid_high, g_p1.y),
                        IM_COL32(0, 230, 118, 45), IM_COL32(0, 200, 100, 20),
                        IM_COL32(0, 160, 80, 60), IM_COL32(0, 240, 130, 75));

                    // High Band (Azul Ciano / Air #00E5FF)
                    dl->AddRectFilledMultiColor(ImVec2(x_mid_high, g_pos.y), ImVec2(g_p1.x - 26.0f, g_p1.y),
                        IM_COL32(0, 229, 255, 45), IM_COL32(0, 180, 220, 20),
                        IM_COL32(0, 150, 190, 60), IM_COL32(0, 240, 255, 80));

                    // Linhas de Separação Vertical (Frequências de Crossover)
                    dl->AddLine(ImVec2(x_low_mid, g_pos.y), ImVec2(x_low_mid, g_p1.y), IM_COL32(255, 120, 20, 220), 2.0f);
                    dl->AddLine(ImVec2(x_mid_high, g_pos.y), ImVec2(x_mid_high, g_p1.y), IM_COL32(0, 229, 255, 220), 2.0f);

                    // Linhas de Grade dB Horizontais (0, -10, -20, -30, -40 dB)
                    const char* db_labels[] = { "0", "-10", "-20", "-30", "-40" };
                    for (int i = 0; i < 5; ++i) {
                        float gy = g_pos.y + 10.0f + ((float)i / 4.0f) * (g_h - 30.0f);
                        dl->AddLine(ImVec2(g_pos.x, gy), ImVec2(g_p1.x - 26.0f, gy), IM_COL32(35, 48, 64, 180), 1.0f);
                        dl->AddText(ImVec2(g_pos.x + 4.0f, gy - 8.0f), IM_COL32(110, 130, 155, 200), db_labels[i]);
                    }

                    // Títulos das Bandas no Topo do Gráfico
                    dl->AddText(ImVec2(g_pos.x + (x_low_mid - g_pos.x) * 0.45f - 10.0f, g_pos.y + 6.0f), IM_COL32(255, 80, 90, 255), "Low");
                    dl->AddText(ImVec2(x_low_mid + (x_mid_high - x_low_mid) * 0.45f - 10.0f, g_pos.y + 6.0f), IM_COL32(0, 240, 120, 255), "Mid");
                    dl->AddText(ImVec2(x_mid_high + (g_p1.x - 26.0f - x_mid_high) * 0.45f - 12.0f, g_pos.y + 6.0f), IM_COL32(0, 229, 255, 255), "High");

                    // Curva de Transferência Dinâmica (Maximus Crossover Compression Knee Curve)
                    float knee_y0 = g_p1.y - 18.0f;
                    float knee_x_low = g_pos.x + 18.0f;
                    ImVec2 p_prev = ImVec2(knee_x_low, knee_y0);
                    for (float px = knee_x_low; px <= g_p1.x - 28.0f; px += 3.0f) {
                        float prog = (px - knee_x_low) / (g_p1.x - 28.0f - knee_x_low);
                        float curve_val;
                        if (prog < 0.25f) {
                            // Sub bass linear rise
                            curve_val = prog * 2.8f;
                        } else if (prog < 0.70f) {
                            // Mid plateau / compression slope
                            curve_val = 0.70f + (prog - 0.25f) * 0.45f;
                        } else {
                            // High ceiling limiter flat top
                            curve_val = 0.90f + std::sin((px + cur_time * 20.0f) * 0.08f) * 0.015f;
                        }
                        float py = knee_y0 - curve_val * (g_h - 36.0f);
                        ImVec2 p_curr(px, py);
                        ImU32 c_col = (px < x_low_mid) ? IM_COL32(255, 70, 80, 255) : ((px < x_mid_high) ? IM_COL32(0, 240, 120, 255) : IM_COL32(0, 229, 255, 255));
                        dl->AddLine(p_prev, p_curr, c_col, 2.5f);
                        p_prev = p_curr;
                    }

                    // Rótulos de Frequência no rodapé: -20, -50, 100, 200, 1k, 5k, 16k
                    const char* freq_labels[] = { "-20", "-50", "100", "200", "1k", "5k", "16k" };
                    float freq_x_steps[] = { 0.04f, 0.16f, 0.32f, 0.46f, 0.62f, 0.78f, 0.94f };
                    for (int f = 0; f < 7; ++f) {
                        float fx = g_pos.x + freq_x_steps[f] * (g_w - 30.0f);
                        dl->AddText(ImVec2(fx, g_p1.y - 16.0f), IM_COL32(110, 130, 155, 220), freq_labels[f]);
                    }

                    // Dual Output Peak Meters na lateral direita do display
                    float meter_x = g_p1.x - 20.0f;
                    float m_h = g_h - 10.0f;
                    dl->AddRectFilled(ImVec2(meter_x, g_pos.y + 5.0f), ImVec2(meter_x + 6.0f, g_pos.y + 5.0f + m_h), IM_COL32(15, 20, 28, 255));
                    dl->AddRectFilled(ImVec2(meter_x + 9.0f, g_pos.y + 5.0f), ImVec2(meter_x + 15.0f, g_pos.y + 5.0f + m_h), IM_COL32(15, 20, 28, 255));

                    float fill_l = std::clamp(0.82f + std::sin(cur_time * 3.0f) * 0.12f, 0.05f, 0.98f);
                    float fill_r = std::clamp(0.85f + std::cos(cur_time * 2.7f) * 0.10f, 0.05f, 0.98f);
                    dl->AddRectFilled(ImVec2(meter_x, g_pos.y + 5.0f + m_h * (1.0f - fill_l)), ImVec2(meter_x + 6.0f, g_pos.y + 5.0f + m_h), IM_COL32(0, 229, 255, 240), 1.0f);
                    dl->AddRectFilled(ImVec2(meter_x + 9.0f, g_pos.y + 5.0f + m_h * (1.0f - fill_r)), ImVec2(meter_x + 15.0f, g_pos.y + 5.0f + m_h), IM_COL32(0, 229, 255, 240), 1.0f);

                    ImGui::Dummy(ImVec2(g_w, g_h));
                }
                ImGui::EndChild();

                ImGui::SameLine(0, 12.0f);

                // ── SEÇÃO SUPERIOR: DIREITA (BROADCAST LUFS LOUDNESS RADAR & TRUE PEAK) ──
                ImGui::BeginChild("MaximusTopRightChild", ImVec2(col_w, top_row_h), true);
                {
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImGui::TextColored(ImVec4(0.85f, 0.90f, 0.95f, 1.0f), "Broadcast LUFS loudness radar meter");

                    ImVec2 r_pos = ImGui::GetCursorScreenPos();
                    float r_w = col_w - 24.0f;
                    float r_h = top_row_h - 48.0f;

                    // Radar Center e Raio
                    float radar_r = std::clamp(r_h * 0.38f, 40.0f, 62.0f);
                    ImVec2 radar_c(r_pos.x + radar_r + 20.0f, r_pos.y + radar_r + 14.0f);

                    // Círculos concêntricos do Radar LUFS (-66, -32, -18, -14, -9, 0 LUFS)
                    float ring_r_factors[] = { 0.25f, 0.45f, 0.65f, 0.80f, 1.0f };
                    const char* ring_labels[] = { "-66", "-32", "-18", "-14", "0" };
                    for (int k = 0; k < 5; ++k) {
                        float ring_rad = radar_r * ring_r_factors[k];
                        dl->AddCircle(radar_c, ring_rad, IM_COL32(0, 180, 220, 50), 36, 1.0f);
                    }

                    // Linhas radiais dos quadrantes
                    for (int q = 0; q < 4; ++q) {
                        float ang = q * (3.14159f * 0.5f);
                        ImVec2 p_end(radar_c.x + std::cos(ang) * radar_r, radar_c.y + std::sin(ang) * radar_r);
                        dl->AddLine(radar_c, p_end, IM_COL32(0, 180, 220, 60), 1.0f);
                    }

                    // Rótulos de escala ao redor do radar: 0 (topo), -6 (direita), -10 (esquerda)
                    dl->AddText(ImVec2(radar_c.x - 4.0f, radar_c.y - radar_r - 14.0f), IM_COL32(160, 180, 200, 220), "0");
                    dl->AddText(ImVec2(radar_c.x + radar_r + 6.0f, radar_c.y - 6.0f), IM_COL32(160, 180, 200, 220), "-6");
                    dl->AddText(ImVec2(radar_c.x - radar_r - 24.0f, radar_c.y - 6.0f), IM_COL32(160, 180, 200, 220), "-10");

                    // Histórico do Contorno de Sonoridade Circular (Curva em Ciano Neon)
                    int h_count = (int)radar_history.size();
                    for (int i = 0; i < h_count; ++i) {
                        int next_i = (i + 1) % h_count;
                        float a0 = (float)i / (float)h_count * 6.28318f;
                        float a1 = (float)next_i / (float)h_count * 6.28318f;

                        float radius_0 = radar_r * (0.35f + radar_history[i] * 0.60f);
                        float radius_1 = radar_r * (0.35f + radar_history[next_i] * 0.60f);

                        ImVec2 pt0(radar_c.x + std::cos(a0) * radius_0, radar_c.y + std::sin(a0) * radius_0);
                        ImVec2 pt1(radar_c.x + std::cos(a1) * radius_1, radar_c.y + std::sin(a1) * radius_1);
                        dl->AddLine(pt0, pt1, IM_COL32(0, 240, 255, 230), 1.8f);
                    }

                    // Ponteiro / Feixe de Varredura Radar em Âmbar / Ouro com rastro
                    ImVec2 sweep_tip(radar_c.x + std::cos(radar_sweep_angle) * radar_r, radar_c.y + std::sin(radar_sweep_angle) * radar_r);
                    dl->AddLine(radar_c, sweep_tip, IM_COL32(255, 215, 64, 255), 2.2f);
                    
                    // Rastro suave
                    float trail_ang = radar_sweep_angle - 0.25f;
                    ImVec2 trail_tip(radar_c.x + std::cos(trail_ang) * (radar_r * 0.9f), radar_c.y + std::sin(trail_ang) * (radar_r * 0.9f));
                    dl->AddTriangleFilled(radar_c, sweep_tip, trail_tip, IM_COL32(255, 215, 64, 50));

                    // ── COLUNA DIREITA DO RADAR: TRUE PEAK METERS & READOUTS ──
                    float tp_x = r_pos.x + r_w - 92.0f;
                    float tp_y0 = r_pos.y + 4.0f;
                    float tp_h = 135.0f;

                    // Valor True Peak em Destaque no Topo
                    dl->AddText(ImVec2(tp_x + 6.0f, tp_y0 - 2.0f), IM_COL32(255, 215, 64, 255), "-0.1 dBFS");

                    // Barras verticais do medidor True Peak (L & R)
                    float b_w = 9.0f;
                    float b_l_x = tp_x + 18.0f;
                    float b_r_x = b_l_x + 14.0f;

                    dl->AddRectFilled(ImVec2(b_l_x, tp_y0 + 20.0f), ImVec2(b_l_x + b_w, tp_y0 + 20.0f + tp_h), IM_COL32(14, 18, 26, 255));
                    dl->AddRectFilled(ImVec2(b_r_x, tp_y0 + 20.0f), ImVec2(b_r_x + b_w, tp_y0 + 20.0f + tp_h), IM_COL32(14, 18, 26, 255));

                    // Preenchimento gradiente ciano para True Peak
                    float tp_norm = 0.985f; // Quase no teto (-0.1 dBFS)
                    dl->AddRectFilled(ImVec2(b_l_x, tp_y0 + 20.0f + tp_h * (1.0f - tp_norm)), ImVec2(b_l_x + b_w, tp_y0 + 20.0f + tp_h), IM_COL32(0, 229, 255, 240), 1.0f);
                    dl->AddRectFilled(ImVec2(b_r_x, tp_y0 + 20.0f + tp_h * (1.0f - (tp_norm - 0.01f))), ImVec2(b_r_x + b_w, tp_y0 + 20.0f + tp_h), IM_COL32(0, 229, 255, 240), 1.0f);

                    // Indicador de Peak Hold no Teto (-0.1 dBFS)
                    dl->AddLine(ImVec2(b_l_x - 1.0f, tp_y0 + 22.0f), ImVec2(b_l_x + b_w + 1.0f, tp_y0 + 22.0f), IM_COL32(255, 215, 64, 255), 2.0f);
                    dl->AddLine(ImVec2(b_r_x - 1.0f, tp_y0 + 22.0f), ImVec2(b_r_x + b_w + 1.0f, tp_y0 + 22.0f), IM_COL32(255, 215, 64, 255), 2.0f);

                    // Escala numérica dBFS no lado esquerdo das barras: 0, -3, -6, -9, -16, -28, -32
                    const char* tp_labels[] = { "0", "-3", "-6", "-9", "-16", "-28", "-32" };
                    float tp_label_y[] = { 0.0f, 0.12f, 0.24f, 0.38f, 0.60f, 0.82f, 0.98f };
                    for (int s = 0; s < 7; ++s) {
                        float sy = tp_y0 + 20.0f + tp_label_y[s] * tp_h;
                        dl->AddText(ImVec2(b_l_x - 24.0f, sy - 6.0f), IM_COL32(130, 150, 175, 200), tp_labels[s]);
                    }

                    // Marcador '-0.1' apontando para o teto à direita
                    dl->AddText(ImVec2(b_r_x + 14.0f, tp_y0 + 20.0f), IM_COL32(255, 215, 64, 255), "-0.1");

                    // Rótulo dBFS no pé das barras
                    dl->AddText(ImVec2(b_l_x + 2.0f, tp_y0 + tp_h + 24.0f), IM_COL32(120, 140, 165, 220), "dBFS");

                    // ── GRANDES LEITURAS DIGITAIS NO RODAPÉ DO RADAR ──────────
                    float read_y = r_pos.y + r_h - 22.0f;
                    
                    // -9 LUFS Integrated (Dourado / Âmbar Grande)
                    char lufs_str[48];
                    snprintf(lufs_str, sizeof(lufs_str), "%.0f LUFS Int", integrated_lufs);
                    dl->AddText(ImVec2(r_pos.x + 14.0f, read_y - 10.0f), IM_COL32(255, 215, 64, 255), lufs_str);

                    // -0.1 dBFS True Peak
                    dl->AddText(ImVec2(r_pos.x + r_w - 145.0f, read_y - 10.0f), IM_COL32(255, 215, 64, 255), "-0.1 dBFS Peak");

                    ImGui::Dummy(ImVec2(r_w, r_h));
                }
                ImGui::EndChild();

                ImGui::Spacing();

                // =============================================================
                // ── SEÇÃO INFERIOR: ESQUERDA (3 BANDAS CONTROLS) | DIREITA (3D GONIÔMETRO)
                // =============================================================
                ImGui::BeginChild("MaximusBotLeftChild", ImVec2(col_w, bot_row_h), true);
                {
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    float band_col_w = (col_w - 24.0f) / 3.0f;

                    // 1. COLUNA LOW BAND (VERMELHO / CORAL)
                    ImGui::BeginChild("BandLowCol", ImVec2(band_col_w, bot_row_h - 16.0f), false);
                    {
                        ImVec2 p0 = ImGui::GetCursorScreenPos();
                        ImGui::TextColored(ImVec4(1.0f, 0.40f, 0.40f, 1.0f), "  Low");

                        // Nível Vertical / Gain Reduction Meter
                        ImVec2 m_pos(p0.x + band_col_w * 0.46f - 4.0f, p0.y + 24.0f);
                        float m_h = 70.0f;
                        dl->AddRectFilled(m_pos, ImVec2(m_pos.x + 8.0f, m_pos.y + m_h), IM_COL32(18, 22, 30, 255), 2.0f);
                        float l_fill = 0.72f + std::sin(cur_time * 4.0f) * 0.15f;
                        dl->AddRectFilled(ImVec2(m_pos.x, m_pos.y + m_h * (1.0f - l_fill)), ImVec2(m_pos.x + 8.0f, m_pos.y + m_h), IM_COL32(230, 50, 60, 240), 2.0f);

                        ImGui::Dummy(ImVec2(band_col_w, 100.0f));

                        // 3 Knobs Circulares Tecnológicos
                        DrawMaximusKnob("##LowThresh", "Threshold", &low_threshold, -30.0f, 0.0f, "%.1fdB", IM_COL32(230, 50, 60, 255));
                        ImGui::Spacing();
                        DrawMaximusKnob("##LowRatio", "Ratio", &low_ratio, 1.0f, 10.0f, "%.1f:1", IM_COL32(230, 50, 60, 255));
                        ImGui::Spacing();
                        DrawMaximusKnob("##LowAttack", "Attack", &low_attack, 0.5f, 50.0f, "%.0fms", IM_COL32(230, 50, 60, 255));
                    }
                    ImGui::EndChild();

                    ImGui::SameLine(0, 4.0f);

                    // 2. COLUNA MID BAND (VERDE NEON / ESMERALDA)
                    ImGui::BeginChild("BandMidCol", ImVec2(band_col_w, bot_row_h - 16.0f), false);
                    {
                        ImVec2 p0 = ImGui::GetCursorScreenPos();
                        ImGui::TextColored(ImVec4(0.0f, 0.90f, 0.45f, 1.0f), "  Mid");

                        // Nível Vertical / Gain Reduction Meter
                        ImVec2 m_pos(p0.x + band_col_w * 0.46f - 4.0f, p0.y + 24.0f);
                        float m_h = 70.0f;
                        dl->AddRectFilled(m_pos, ImVec2(m_pos.x + 8.0f, m_pos.y + m_h), IM_COL32(18, 22, 30, 255), 2.0f);
                        float m_fill = 0.65f + std::sin(cur_time * 3.2f) * 0.12f;
                        dl->AddRectFilled(ImVec2(m_pos.x, m_pos.y + m_h * (1.0f - m_fill)), ImVec2(m_pos.x + 8.0f, m_pos.y + m_h), IM_COL32(0, 230, 118, 240), 2.0f);

                        ImGui::Dummy(ImVec2(band_col_w, 100.0f));

                        // 3 Knobs Circulares Tecnológicos
                        DrawMaximusKnob("##MidThresh", "Threshold", &mid_threshold, -30.0f, 0.0f, "%.1fdB", IM_COL32(0, 230, 118, 255));
                        ImGui::Spacing();
                        DrawMaximusKnob("##MidRatio", "Ratio", &mid_ratio, 1.0f, 10.0f, "%.1f:1", IM_COL32(0, 230, 118, 255));
                        ImGui::Spacing();
                        DrawMaximusKnob("##MidAttack", "Attack", &mid_attack, 0.5f, 50.0f, "%.0fms", IM_COL32(0, 230, 118, 255));
                    }
                    ImGui::EndChild();

                    ImGui::SameLine(0, 4.0f);

                    // 3. COLUNA HIGH BAND (AZUL CIANO / AIR)
                    ImGui::BeginChild("BandHighCol", ImVec2(band_col_w, bot_row_h - 16.0f), false);
                    {
                        ImVec2 p0 = ImGui::GetCursorScreenPos();
                        ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "  High");

                        // Nível Vertical com Escala dB
                        ImVec2 m_pos(p0.x + band_col_w * 0.40f - 4.0f, p0.y + 24.0f);
                        float m_h = 70.0f;
                        dl->AddRectFilled(m_pos, ImVec2(m_pos.x + 8.0f, m_pos.y + m_h), IM_COL32(18, 22, 30, 255), 2.0f);
                        float h_fill = 0.80f + std::cos(cur_time * 4.2f) * 0.10f;
                        dl->AddRectFilled(ImVec2(m_pos.x, m_pos.y + m_h * (1.0f - h_fill)), ImVec2(m_pos.x + 8.0f, m_pos.y + m_h), IM_COL32(0, 229, 255, 240), 2.0f);

                        // Escala dB ao lado do meter High (0, -4, -8, -13, -20, -25)
                        const char* h_scale[] = { "0", "-4", "-8", "-13", "-20", "-25" };
                        for (int k = 0; k < 6; ++k) {
                            float ky = m_pos.y + ((float)k / 5.0f) * (m_h - 10.0f);
                            dl->AddText(ImVec2(m_pos.x + 14.0f, ky - 6.0f), IM_COL32(120, 140, 165, 200), h_scale[k]);
                        }

                        ImGui::Dummy(ImVec2(band_col_w, 100.0f));

                        // 3 Knobs Circulares Tecnológicos
                        DrawMaximusKnob("##HighThresh", "Threshold", &high_threshold, -30.0f, 0.0f, "%.1fdB", IM_COL32(0, 229, 255, 255));
                        ImGui::Spacing();
                        DrawMaximusKnob("##HighRatio", "Ratio", &high_ratio, 1.0f, 10.0f, "%.1f:1", IM_COL32(0, 229, 255, 255));
                        ImGui::Spacing();
                        DrawMaximusKnob("##HighAttack", "Attack", &high_attack, 0.5f, 50.0f, "%.0fms", IM_COL32(0, 229, 255, 255));
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();

                ImGui::SameLine(0, 12.0f);

                // ── SEÇÃO INFERIOR: DIREITA (3D GLOWING NEON LISSAJOUS VECTOR SCOPE GONIOMETER) ──
                ImGui::BeginChild("MaximusBotRightChild", ImVec2(col_w, bot_row_h), true);
                {
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImGui::TextColored(ImVec4(0.85f, 0.90f, 0.95f, 1.0f), "3D glowing neon Lissajous vector scope stereo goniometer");

                    ImVec2 g_pos = ImGui::GetCursorScreenPos();
                    float g_w = col_w - 24.0f;
                    float g_h = bot_row_h - 48.0f;
                    ImVec2 center = ImVec2(g_pos.x + g_w * 0.50f, g_pos.y + g_h * 0.52f);

                    // Fundo escuro do visualizador 3D
                    dl->AddRectFilled(g_pos, ImVec2(g_pos.x + g_w, g_pos.y + g_h), IM_COL32(8, 11, 16, 255), 4.0f);
                    dl->AddRect(g_pos, ImVec2(g_pos.x + g_w, g_pos.y + g_h), IM_COL32(24, 36, 50, 255), 4.0f);

                    // Rótulos de eixos 3D estilizados
                    dl->AddText(ImVec2(g_pos.x + 12.0f, g_pos.y + 10.0f), IM_COL32(120, 140, 165, 220), "3D");
                    dl->AddText(ImVec2(g_pos.x + 12.0f, center.y), IM_COL32(100, 120, 145, 180), "0");
                    dl->AddText(ImVec2(g_pos.x + 12.0f, g_pos.y + g_h - 24.0f), IM_COL32(100, 120, 145, 180), "-30");
                    dl->AddText(ImVec2(g_pos.x + 24.0f, g_pos.y + g_h - 14.0f), IM_COL32(100, 120, 145, 180), "20");
                    dl->AddText(ImVec2(center.x - 10.0f, g_pos.y + g_h - 24.0f), IM_COL32(100, 120, 145, 180), "0");
                    dl->AddText(ImVec2(center.x + 40.0f, g_pos.y + g_h - 14.0f), IM_COL32(100, 120, 145, 180), "-20");
                    dl->AddText(ImVec2(center.x + 75.0f, g_pos.y + g_h - 14.0f), IM_COL32(100, 120, 145, 180), "10");
                    dl->AddText(ImVec2(g_pos.x + g_w - 32.0f, center.y + 20.0f), IM_COL32(100, 120, 145, 180), "+20");

                    // 1. Gaiola / Cubo Aramado 3D de Delimitação (Bounding Box)
                    float box_s = 65.0f;
                    ImVec2 v[8];
                    v[0] = Project3D(-box_s, -box_s, -box_s, center, 1.0f);
                    v[1] = Project3D( box_s, -box_s, -box_s, center, 1.0f);
                    v[2] = Project3D( box_s,  box_s, -box_s, center, 1.0f);
                    v[3] = Project3D(-box_s,  box_s, -box_s, center, 1.0f);
                    v[4] = Project3D(-box_s, -box_s,  box_s, center, 1.0f);
                    v[5] = Project3D( box_s, -box_s,  box_s, center, 1.0f);
                    v[6] = Project3D( box_s,  box_s,  box_s, center, 1.0f);
                    v[7] = Project3D(-box_s,  box_s,  box_s, center, 1.0f);

                    ImU32 box_col = IM_COL32(40, 60, 85, 160);
                    // Arestas da base inferior e superior
                    for (int i = 0; i < 4; ++i) {
                        dl->AddLine(v[i], v[(i + 1) % 4], box_col, 1.0f);
                        dl->AddLine(v[i + 4], v[((i + 1) % 4) + 4], box_col, 1.0f);
                        dl->AddLine(v[i], v[i + 4], box_col, 1.0f);
                    }

                    // 2. Superfície Holográfica Lissajous 3D Neon Brilhante (Stereo Phase Vector Scope)
                    int rings = 14;
                    int pts_per_ring = 28;
                    float width_mult = stereo_enhancer_width;

                    for (int r = 0; r < rings; ++r) {
                        float v_norm = ((float)r / (float)(rings - 1)) * 2.0f - 1.0f; // -1 to +1
                        float y_val = v_norm * box_s * 0.88f;

                        // Raio no plano X-Z dependendo da amplitude e fase estéreo
                        float r_xz = box_s * 0.75f * std::cos(v_norm * 1.57f * 0.45f);

                        ImVec2 prev_p;
                        bool has_prev = false;

                        for (int p = 0; p <= pts_per_ring; ++p) {
                            float u_ang = (float)p / (float)pts_per_ring * 6.28318f;

                            // Curva paramétrica Lissajous estéreo
                            float phase_shift = cur_time * 1.2f;
                            float x_raw = std::sin(u_ang * 2.0f + phase_shift) * r_xz * width_mult;
                            float z_raw = std::cos(u_ang * 3.0f + phase_shift * 0.7f) * r_xz;

                            // Deformação harmônica de áudio simulada
                            float audio_bump = 1.0f + std::sin(u_ang * 5.0f + cur_time * 3.5f) * 0.12f;
                            x_raw *= audio_bump;
                            z_raw *= audio_bump;

                            ImVec2 proj = Project3D(x_raw, y_val, z_raw, center, 1.0f);

                            if (has_prev) {
                                // Brilho neon ciano intenso estilo FL Studio Maximus
                                dl->AddLine(prev_p, proj, IM_COL32(0, 229, 255, 230), 1.6f);
                            }
                            prev_p = proj;
                            has_prev = true;
                        }
                    }

                    // Linhas verticais cruzadas para formar a malha hiper-cúbica 3D
                    for (int p = 0; p < pts_per_ring; p += 3) {
                        float u_ang = (float)p / (float)pts_per_ring * 6.28318f;
                        ImVec2 prev_p;
                        bool has_prev = false;

                        for (int r = 0; r < rings; ++r) {
                            float v_norm = ((float)r / (float)(rings - 1)) * 2.0f - 1.0f;
                            float y_val = v_norm * box_s * 0.88f;
                            float r_xz = box_s * 0.75f * std::cos(v_norm * 1.57f * 0.45f);

                            float phase_shift = cur_time * 1.2f;
                            float x_raw = std::sin(u_ang * 2.0f + phase_shift) * r_xz * width_mult;
                            float z_raw = std::cos(u_ang * 3.0f + phase_shift * 0.7f) * r_xz;

                            ImVec2 proj = Project3D(x_raw, y_val, z_raw, center, 1.0f);
                            if (has_prev) {
                                dl->AddLine(prev_p, proj, IM_COL32(0, 240, 255, 120), 1.0f);
                            }
                            prev_p = proj;
                            has_prev = true;
                        }
                    }

                    // Controles no rodapé do goniômetro: Width e Auto-rotate
                    dl->AddRectFilled(ImVec2(g_pos.x + 6.0f, g_pos.y + g_h - 30.0f), ImVec2(g_pos.x + 225.0f, g_pos.y + g_h - 4.0f), IM_COL32(10, 14, 20, 220), 4.0f);
                    dl->AddRect(ImVec2(g_pos.x + 6.0f, g_pos.y + g_h - 30.0f), ImVec2(g_pos.x + 225.0f, g_pos.y + g_h - 4.0f), IM_COL32(40, 55, 75, 200), 4.0f);

                    ImGui::SetCursorScreenPos(ImVec2(g_pos.x + 10.0f, g_pos.y + g_h - 27.0f));
                    ImGui::SetNextItemWidth(115.0f);
                    ImGui::SliderFloat("##WidthSpread", &stereo_enhancer_width, 0.5f, 2.0f, "Width %.2fx");
                    ImGui::SameLine();
                    ImGui::Checkbox("3D Orbit", &gonio_auto_rotate);

                    ImGui::SetCursorScreenPos(g_pos);
                    ImGui::Dummy(ImVec2(g_w, g_h));
                }
                ImGui::EndChild();
            }
            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
