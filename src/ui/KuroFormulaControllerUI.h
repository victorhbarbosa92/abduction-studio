#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFormulaControllerUI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity Formula Controller (Mathematical Modulation Engine)
        float var_a = 0.50f;             // Variável A (0.0 a 1.0)
        float var_b = 0.25f;             // Variável B (0.0 a 1.0)
        float var_c = 0.75f;             // Variável C (0.0 a 1.0)
        char formula_str[256] = "Sin(SongPos*Pi*4)*a + b";
        float lfo_speed_mult = 1.0f;
        int active_preset_idx = 0;

    public:
        KuroFormulaControllerUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(920, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.85f)); // Ciano / Azul Matemático

            if (ImGui::Begin("📐 FRUITY FORMULA CONTROLLER (MATHEMATICAL AUTOMATION SYNTH)###FormulaWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets Matemáticos")) {
                        if (ImGui::MenuItem("📐 Sinusoidal LFO: Sin(SongPos*Pi*4)*a + b")) {
                            snprintf(formula_str, sizeof(formula_str), "Sin(SongPos*Pi*4)*a + b");
                            active_preset_idx = 0; var_a = 0.45f; var_b = 0.50f;
                        }
                        if (ImGui::MenuItem("⚡ Chaos Glitch Modulation: (Sin(SongPos*12)*a + Cos(SongPos*25)*b)*c")) {
                            snprintf(formula_str, sizeof(formula_str), "(Sin(SongPos*12)*a + Cos(SongPos*25)*b)*c");
                            active_preset_idx = 1; var_a = 0.6f; var_b = 0.4f; var_c = 0.8f;
                        }
                        if (ImGui::MenuItem("🌊 Exponential Sidechain Ducking: Exp(-Fract(SongPos*4)*a)*b")) {
                            snprintf(formula_str, sizeof(formula_str), "Exp(-Fract(SongPos*4)*a)*b");
                            active_preset_idx = 2; var_a = 3.5f; var_b = 0.9f;
                        }
                        if (ImGui::MenuItem("🪜 Stepped Staircase Quantizer: Round(SongPos*a)/a")) {
                            snprintf(formula_str, sizeof(formula_str), "Round(SongPos*a)/a");
                            active_preset_idx = 3; var_a = 8.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "FRUITY FORMULA CONTROLLER: MATH EXPRESSION LFO & PARAMETER GENERATOR");
                ImGui::TextDisabled("Gera curvas de automação procedural em tempo real através de fórmulas matemáticas e 3 variáveis contínuas (a, b, c).");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA CURVA MATEMÁTICA CALCULADA ─────────
                ImVec2 fm_p0 = ImGui::GetCursorScreenPos();
                ImVec2 fm_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 fm_p1 = ImVec2(fm_p0.x + fm_sz.x, fm_p0.y + fm_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(fm_p0, fm_p1, IM_COL32(10, 15, 22, 255), 4.0f);
                dl->AddRect(fm_p0, fm_p1, IM_COL32(30, 50, 70, 255), 4.0f);

                // Linha de Centro (0.5) e Limites (0.0 a 1.0)
                float cy = fm_p0.y + fm_sz.y * 0.5f;
                dl->AddLine(ImVec2(fm_p0.x, cy), ImVec2(fm_p1.x, cy), IM_COL32(40, 60, 80, 160), 1.0f);
                dl->AddLine(ImVec2(fm_p0.x, fm_p1.y - 8), ImVec2(fm_p1.x, fm_p1.y - 8), IM_COL32(30, 45, 60, 120), 1.0f);
                dl->AddLine(ImVec2(fm_p0.x, fm_p0.y + 8), ImVec2(fm_p1.x, fm_p0.y + 8), IM_COL32(30, 45, 60, 120), 1.0f);

                // Desenhar Curva Gerada pela Fórmula
                int steps = (int)fm_sz.x;
                for (int x = 0; x < steps - 1; ++x) {
                    float t0 = (float)x / (float)steps * 3.14159265f * 2.0f;
                    float t1 = (float)(x + 1) / (float)steps * 3.14159265f * 2.0f;

                    float val0 = 0.0f;
                    float val1 = 0.0f;

                    if (active_preset_idx == 0) {
                        val0 = std::sin(t0 * 2.0f) * var_a + var_b;
                        val1 = std::sin(t1 * 2.0f) * var_a + var_b;
                    } else if (active_preset_idx == 1) {
                        val0 = (std::sin(t0 * 4.0f) * var_a + std::cos(t0 * 8.0f) * var_b) * var_c + 0.5f;
                        val1 = (std::sin(t1 * 4.0f) * var_a + std::cos(t1 * 8.0f) * var_b) * var_c + 0.5f;
                    } else if (active_preset_idx == 2) {
                        float f0 = std::fmod(t0 * 0.8f, 1.0f);
                        float f1 = std::fmod(t1 * 0.8f, 1.0f);
                        val0 = std::exp(-f0 * var_a) * var_b;
                        val1 = std::exp(-f1 * var_a) * var_b;
                    } else {
                        val0 = std::round(t0 * var_a) / var_a * 0.2f;
                        val1 = std::round(t1 * var_a) / var_a * 0.2f;
                    }

                    val0 = std::clamp(val0, 0.0f, 1.0f);
                    val1 = std::clamp(val1, 0.0f, 1.0f);

                    float py0 = fm_p1.y - 10.0f - val0 * (fm_sz.y - 20.0f);
                    float py1 = fm_p1.y - 10.0f - val1 * (fm_sz.y - 20.0f);

                    dl->AddLine(ImVec2(fm_p0.x + x, py0), ImVec2(fm_p0.x + x + 1, py1), IM_COL32(0, 240, 255, 240), 2.0f);
                }

                char fm_info[128];
                snprintf(fm_info, sizeof(fm_info), "FÓRMULA: %s | A: %.2f | B: %.2f | C: %.2f", formula_str, var_a, var_b, var_c);
                dl->AddText(ImVec2(fm_p0.x + 12.0f, fm_p0.y + 6.0f), IM_COL32(0, 230, 255, 240), fm_info);

                ImGui::Dummy(fm_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. ENTRADA DA FÓRMULA & AS 3 VARIÁVEIS (A, B, C) ────────
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "📝 EXPRESSÃO MATEMÁTICA CUSTOMIZADA");
                ImGui::InputText("##FormulaInput", formula_str, sizeof(formula_str));

                ImGui::Spacing();
                ImGui::Columns(3, "FormulaVarsCols", true);

                // Variável A
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "🅰️ VARIÁVEL 'a'");
                ImGui::SliderFloat("##var_a", &var_a, 0.0f, 1.0f, "%.3f");

                ImGui::NextColumn();

                // Variável B
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🅱️ VARIÁVEL 'b'");
                ImGui::SliderFloat("##var_b", &var_b, 0.0f, 1.0f, "%.3f");

                ImGui::NextColumn();

                // Variável C
                ImGui::TextColored(ImVec4(0.9f, 0.4f, 1.0f, 1.0f), "🅲 VARIÁVEL 'c'");
                ImGui::SliderFloat("##var_c", &var_c, 0.0f, 1.0f, "%.3f");

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::ProgressBar(var_a, ImVec2(-1, 20), "Calculated Automation Output");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
