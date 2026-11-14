#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityStereoReverbDelayPanUI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity Stereo Delay (True L/R Split Dual Delay & Inverted Echoes)
        float left_delay_time_ms = 250.0f;  // Tempo atraso L (1ms a 1000ms)
        float right_delay_time_ms = 375.0f; // Tempo atraso R (1ms a 1000ms)
        float left_feedback = 0.55f;        // Feedback L
        float right_feedback = 0.55f;       // Feedback R
        float cross_feedback_bleed = 0.25f; // Vazamento cruzado L/R
        float lowpass_damping_hz = 6500.0f; // Filtro de amortecimento de agudos
        float dry_wet_mix = 0.50f;
        bool ping_pong_cross_mode = true;

    public:
        KuroFruityStereoReverbDelayPanUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(920, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.70f, 1.00f, 0.85f)); // Azul Céu Stereo Delay

            if (ImGui::Begin("🎧 FRUITY STEREO DELAY (DUAL-CHANNEL INDEPENDENT ECHOES)###StereoDelayWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🎧 Classic 1/4 + 1/8 Dotted Psytrance Ping-Pong")) {
                            left_delay_time_ms = 250.0f; right_delay_time_ms = 375.0f; ping_pong_cross_mode = true;
                            left_feedback = 0.60f; right_feedback = 0.60f; cross_feedback_bleed = 0.40f;
                        }
                        if (ImGui::MenuItem("📻 Ambient Shimmer Space (500ms / 750ms)")) {
                            left_delay_time_ms = 500.0f; right_delay_time_ms = 750.0f; lowpass_damping_hz = 4200.0f;
                            left_feedback = 0.75f; right_feedback = 0.75f;
                        }
                        if (ImGui::MenuItem("🎸 Slapback Rockabilly Stereo Doubler (35ms / 50ms)")) {
                            left_delay_time_ms = 35.0f; right_delay_time_ms = 50.0f; left_feedback = 0.20f;
                            right_feedback = 0.20f; ping_pong_cross_mode = false;
                        }
                        if (ImGui::MenuItem("⚡ Metallic Resonant Comb Flange (12ms)")) {
                            left_delay_time_ms = 12.0f; right_delay_time_ms = 18.0f; left_feedback = 0.90f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "FRUITY STEREO DELAY: INDEPENDENT DUAL L/R TIME & CROSS-FEEDBACK ECHOES");
                ImGui::TextDisabled("Atraso estéreo independente para os canais Esquerdo e Direito com modos Ping-Pong e amortecimento analógico.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DOS ECOS DUPLOS L/R ───────────────────
                ImVec2 sd_p0 = ImGui::GetCursorScreenPos();
                ImVec2 sd_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 sd_p1 = ImVec2(sd_p0.x + sd_sz.x, sd_p0.y + sd_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(sd_p0, sd_p1, IM_COL32(10, 15, 22, 255), 4.0f);
                dl->AddRect(sd_p0, sd_p1, IM_COL32(30, 50, 70, 255), 4.0f);

                float cy_left = sd_p0.y + sd_sz.y * 0.30f;
                float cy_right = sd_p0.y + sd_sz.y * 0.70f;

                dl->AddLine(ImVec2(sd_p0.x, cy_left), ImVec2(sd_p1.x, cy_left), IM_COL32(40, 60, 80, 160), 1.0f);
                dl->AddLine(ImVec2(sd_p0.x, cy_right), ImVec2(sd_p1.x, cy_right), IM_COL32(40, 60, 80, 160), 1.0f);

                dl->AddText(ImVec2(sd_p0.x + 8.0f, cy_left - 16.0f), IM_COL32(0, 220, 255, 200), "CANAL ESQUERDO (L)");
                dl->AddText(ImVec2(sd_p0.x + 8.0f, cy_right - 16.0f), IM_COL32(255, 120, 220, 200), "CANAL DIREITO (R)");

                // Desenhar Taps de Ecos Canal L
                int taps_l = 8;
                for (int i = 0; i < taps_l; ++i) {
                    float tx = sd_p0.x + 120.0f + (i * left_delay_time_ms * 0.45f);
                    if (tx < sd_p1.x - 10.0f) {
                        float amp = std::pow(left_feedback, (float)i) * (sd_sz.y * 0.22f);
                        dl->AddLine(ImVec2(tx, cy_left), ImVec2(tx, cy_left - amp), IM_COL32(0, 240, 255, 240), 2.5f);
                        dl->AddCircleFilled(ImVec2(tx, cy_left - amp), 3.0f, IM_COL32(0, 255, 255, 255));
                    }
                }

                // Desenhar Taps de Ecos Canal R
                int taps_r = 8;
                for (int i = 0; i < taps_r; ++i) {
                    float tx = sd_p0.x + 120.0f + (i * right_delay_time_ms * 0.45f);
                    if (tx < sd_p1.x - 10.0f) {
                        float amp = std::pow(right_feedback, (float)i) * (sd_sz.y * 0.22f);
                        dl->AddLine(ImVec2(tx, cy_right), ImVec2(tx, cy_right + amp), IM_COL32(255, 80, 200, 240), 2.5f);
                        dl->AddCircleFilled(ImVec2(tx, cy_right + amp), 3.0f, IM_COL32(255, 120, 255, 255));
                    }
                }

                char sd_info[128];
                snprintf(sd_info, sizeof(sd_info), "ATRASO L: %.0f ms | ATRASO R: %.0f ms | PING-PONG: %s | AMORTECIMENTO: %.0f Hz", left_delay_time_ms, right_delay_time_ms, ping_pong_cross_mode ? "ON" : "OFF", lowpass_damping_hz);
                dl->AddText(ImVec2(sd_p0.x + 12.0f, sd_p1.y - 18.0f), IM_COL32(220, 240, 255, 240), sd_info);

                ImGui::Dummy(sd_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO STEREO DELAY ───────────────────
                ImGui::Columns(3, "StereoDelayCols", true);

                // Coluna 1: Canal Esquerdo (L)
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "🎧 CANAL ESQUERDO (L)");
                ImGui::Separator();

                ImGui::SliderFloat("Tempo Delay L", &left_delay_time_ms, 1.0f, 1000.0f, "%.0f ms");
                ImGui::SliderFloat("Feedback L", &left_feedback, 0.0f, 0.95f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: Canal Direito (R)
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.85f, 1.0f), "🎧 CANAL DIREITO (R)");
                ImGui::Separator();

                ImGui::SliderFloat("Tempo Delay R", &right_delay_time_ms, 1.0f, 1000.0f, "%.0f ms");
                ImGui::SliderFloat("Feedback R", &right_feedback, 0.0f, 0.95f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Crossfeed & Mix
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎛️ CROSSFEED & MIX");
                ImGui::Separator();

                ImGui::SliderFloat("Vazamento Cruzado", &cross_feedback_bleed, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Filtro Damping", &lowpass_damping_hz, 500.0f, 18000.0f, "%.0f Hz");
                ImGui::SliderFloat("Dry / Wet", &dry_wet_mix, 0.0f, 1.0f, "%.2f");
                ImGui::Checkbox("Modo Ping-Pong", &ping_pong_cross_mode);

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
