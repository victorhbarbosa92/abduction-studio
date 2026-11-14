#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroWaveShaperUI {
    private:
        bool is_open = false;

        // Curva de Transferência Não-Linear de Distorção (FL Studio WaveShaper)
        // 5 Pontos de controle da curva de transferência de amplitude (In -> Out)
        float curve_point_y[5] = { 0.0f, 0.35f, 0.70f, 0.90f, 1.0f }; // Pontos X = 0.0, 0.25, 0.50, 0.75, 1.0
        float pre_amp_gain = 1.25f;       // Ganho de entrada (Drive)
        float post_amp_gain = 1.0f;       // Ganho de saída
        float mix_dry_wet = 1.0f;
        int oversampling_rate = 2;        // 0=Off, 1=2x, 2=4x (Zero Aliasing HQ)
        bool unipolar_bipolar_mode = false;// false=Bipolar (Simétrico), true=Unipolar (Assimétrico / Tubo)
        bool soft_clip_ceiling = true;

    public:
        KuroWaveShaperUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.30f, 0.00f, 0.85f)); // Laranja Fogo WaveShaper

            if (ImGui::Begin("🔥 FRUITY WAVESHAPER (NON-LINEAR DISTORTION & SATURATION)###WaveShaperWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🔥 Warm Analog Tape / Tube Warmth")) {
                            curve_point_y[0] = 0.0f; curve_point_y[1] = 0.40f; curve_point_y[2] = 0.72f;
                            curve_point_y[3] = 0.92f; curve_point_y[4] = 1.0f; pre_amp_gain = 1.5f;
                        }
                        if (ImGui::MenuItem("⚡ Hard Clipping Distortion (Trap 808 / Hardstyle)")) {
                            curve_point_y[0] = 0.0f; curve_point_y[1] = 0.80f; curve_point_y[2] = 1.0f;
                            curve_point_y[3] = 1.0f; curve_point_y[4] = 1.0f; pre_amp_gain = 2.2f;
                        }
                        if (ImGui::MenuItem("👽 S-Curve Soft Saturation (Moog Drive)")) {
                            curve_point_y[0] = 0.0f; curve_point_y[1] = 0.20f; curve_point_y[2] = 0.65f;
                            curve_point_y[3] = 0.95f; curve_point_y[4] = 1.0f;
                        }
                        if (ImGui::MenuItem("📻 Asymmetric Tube Bias (Odd Harmonics)")) {
                            unipolar_bipolar_mode = true; pre_amp_gain = 1.8f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.40f, 0.0f, 1.0f), "FRUITY WAVESHAPER NON-LINEAR TRANSFER GRAPH & MULTI-SAMPLE OVERSAMPLING");
                ImGui::TextDisabled("Modelador de curvas não-lineares para criação de harmônicos ímpares/pares, saturação de fita e distorção extrema.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. GRÁFICO INTERATIVO DA CURVA DE TRANSFERÊNCIA DE ONDA ─
                ImVec2 gr_p0 = ImGui::GetCursorScreenPos();
                ImVec2 gr_sz = ImVec2(ImGui::GetContentRegionAvail().x, 200.0f);
                ImVec2 gr_p1 = ImVec2(gr_p0.x + gr_sz.x, gr_p0.y + gr_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(gr_p0, gr_p1, IM_COL32(12, 16, 22, 255), 4.0f);
                dl->AddRect(gr_p0, gr_p1, IM_COL32(45, 55, 70, 255), 4.0f);

                // Linha de Referência Linear (Sem distorção: Y = X)
                dl->AddLine(ImVec2(gr_p0.x, gr_p1.y), ImVec2(gr_p1.x, gr_p0.y), IM_COL32(60, 70, 85, 160), 1.0f);

                // Desenhar a Curva de Transferência Não-Linear Contínua
                int steps = (int)gr_sz.x;
                for (int x = 0; x < steps - 1; ++x) {
                    float t0 = (float)x / (float)steps;
                    float t1 = (float)(x + 1) / (float)steps;

                    // Interpolação cúbica através dos 5 pontos
                    float idx0 = t0 * 4.0f;
                    int i0 = (int)idx0;
                    float f0 = idx0 - i0;
                    float y0_val = (i0 < 4) ? (curve_point_y[i0] * (1.0f - f0) + curve_point_y[i0 + 1] * f0) : 1.0f;

                    float idx1 = t1 * 4.0f;
                    int i1 = (int)idx1;
                    float f1 = idx1 - i1;
                    float y1_val = (i1 < 4) ? (curve_point_y[i1] * (1.0f - f1) + curve_point_y[i1 + 1] * f1) : 1.0f;

                    float px0 = gr_p0.x + t0 * gr_sz.x;
                    float py0 = gr_p1.y - y0_val * gr_sz.y;
                    float px1 = gr_p0.x + t1 * gr_sz.x;
                    float py1 = gr_p1.y - y1_val * gr_sz.y;

                    dl->AddLine(ImVec2(px0, py0), ImVec2(px1, py1), IM_COL32(255, 100, 0, 255), 2.5f);
                }

                // Desenhar os 5 Pontos de Controle Interativos
                for (int p = 0; p < 5; ++p) {
                    float px = gr_p0.x + (p / 4.0f) * gr_sz.x;
                    float py = gr_p1.y - curve_point_y[p] * gr_sz.y;

                    dl->AddCircleFilled(ImVec2(px, py), 6.0f, IM_COL32(255, 200, 0, 255));
                    dl->AddCircle(ImVec2(px, py), 8.0f, IM_COL32(255, 255, 255, 200));
                }

                dl->AddText(ImVec2(gr_p0.x + 12.0f, gr_p0.y + 8.0f), IM_COL32(255, 140, 0, 240), "CURVA DE TRANSFERÊNCIA DE RESPOSTA (IN -> OUT)");
                ImGui::Dummy(gr_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE AJUSTE DOS PONTOS DE CONTROLE & GANHOS ──────
                ImGui::Columns(2, "WaveShaperCols", true);

                ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.0f, 1.0f), "🎛️ PONTOS DE TRANSFERÊNCIA DA CURVA");
                ImGui::Separator();

                for (int p = 1; p < 4; ++p) {
                    char sl_id[32];
                    snprintf(sl_id, sizeof(sl_id), "Ponto %d (Nível In %.0f%%)", p, p * 25.0f);
                    ImGui::SliderFloat(sl_id, &curve_point_y[p], 0.0f, 1.0f, "%.2f");
                }

                ImGui::Checkbox("Modo Assimétrico (Unipolar / Tube Bias)", &unipolar_bipolar_mode);

                ImGui::NextColumn();

                // Coluna 2: Drive & Oversampling HQ
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "⚡ DRIVE & OVERSAMPLING HQ");
                ImGui::Separator();

                ImGui::SliderFloat("Pre-Amp (Drive In)", &pre_amp_gain, 0.5f, 5.0f, "%.2fx");
                ImGui::SliderFloat("Post-Amp (Out Trim)", &post_amp_gain, 0.1f, 2.0f, "%.2fx");
                ImGui::SliderFloat("Dry / Wet Mix", &mix_dry_wet, 0.0f, 1.0f, "%.2f");

                const char* os_modes[] = { "Desativado (1x)", "2x Oversampling", "4x High-Quality (Zero Aliasing)" };
                ImGui::Combo("Oversampling", &oversampling_rate, os_modes, IM_ARRAYSIZE(os_modes));

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
