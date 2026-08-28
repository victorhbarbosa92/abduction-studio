#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroStereoShaperUI {
    private:
        bool is_open = false;

        // Matriz de Roteamento M/S Estéreo (FL Studio Stereo Shaper)
        // [Left Out]  = LL * InL + RL * InR
        // [Right Out] = LR * InL + RR * InR
        float matrix_ll = 1.0f;
        float matrix_rl = 0.0f;
        float matrix_lr = 0.0f;
        float matrix_rr = 1.0f;

        float delay_left_ms = 0.0f;       // Atraso Haas Effect no canal esquerdo
        float delay_right_ms = 12.5f;     // Atraso Haas Effect no canal direito (Spatial Stereo Widener)
        float phase_left_deg = 0.0f;
        float phase_right_deg = 0.0f;
        bool invert_phase_left = false;
        bool invert_phase_right = false;

    public:
        KuroStereoShaperUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(920, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.85f)); // Azul Ciano Estéreo

            if (ImGui::Begin("🎧 FRUITY STEREO SHAPER (MID/SIDE PROCESSOR & HAAS WIDENER)###StereoShaperWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("👽 Haas Effect Psychoacoustic Widener (12ms R)")) {
                            matrix_ll = 1.0f; matrix_rr = 1.0f; matrix_rl = 0.0f; matrix_lr = 0.0f;
                            delay_left_ms = 0.0f; delay_right_ms = 12.5f;
                        }
                        if (ImGui::MenuItem("🎙️ Mid/Side Splitter (Isolate Sides)")) {
                            matrix_ll = 0.5f; matrix_rl = -0.5f; matrix_lr = -0.5f; matrix_rr = 0.5f;
                        }
                        if (ImGui::MenuItem("📻 Mono Collapse (Isolate Mid)")) {
                            matrix_ll = 0.5f; matrix_rl = 0.5f; matrix_lr = 0.5f; matrix_rr = 0.5f;
                        }
                        if (ImGui::MenuItem("🔄 Phase Invert Right (Stereo Difference)")) {
                            invert_phase_right = true; invert_phase_left = false;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "FRUITY STEREO SHAPER (MID/SIDE MATRIX & PSYCHOACOUSTIC HAAS DELAY)");
                ImGui::TextDisabled("Controle cirúrgico da imagem estéreo, processamento Mid/Side, expansão Haas e correção de correlação de fase.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. VECTORSCOPIO / DISPLAY DE CORRELAÇÃO DE FASE ESTÉREO ─
                ImVec2 st_p0 = ImGui::GetCursorScreenPos();
                ImVec2 st_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 st_p1 = ImVec2(st_p0.x + st_sz.x, st_p0.y + st_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(st_p0, st_p1, IM_COL32(10, 15, 22, 255), 4.0f);
                dl->AddRect(st_p0, st_p1, IM_COL32(35, 50, 70, 255), 4.0f);

                // Linha de centro M/S (Lissajous 45 graus)
                float cx = st_p0.x + st_sz.x * 0.5f;
                float cy = st_p0.y + st_sz.y * 0.5f;
                dl->AddLine(ImVec2(cx, st_p0.y + 10), ImVec2(cx, st_p1.y - 10), IM_COL32(50, 65, 85, 180), 1.0f);
                dl->AddLine(ImVec2(st_p0.x + 10, cy), ImVec2(st_p1.x - 10, cy), IM_COL32(50, 65, 85, 180), 1.0f);

                // Desenhar Figura de Lissajous Estéreo
                int pts = 128;
                for (int i = 0; i < pts; ++i) {
                    float t = (float)i / (float)pts * 6.2831853f;
                    float haas_offset = (delay_right_ms - delay_left_ms) * 0.15f;
                    
                    float in_l = std::sin(t * 3.0f);
                    float in_r = std::sin(t * 3.0f + haas_offset);

                    float out_l = matrix_ll * in_l + matrix_rl * in_r;
                    float out_r = matrix_lr * in_l + matrix_rr * in_r;

                    if (invert_phase_left) out_l = -out_l;
                    if (invert_phase_right) out_r = -out_r;

                    float plot_x = cx + (out_r - out_l) * (st_sz.y * 0.35f);
                    float plot_y = cy - (out_r + out_l) * (st_sz.y * 0.35f);

                    dl->AddCircleFilled(ImVec2(plot_x, plot_y), 2.0f, IM_COL32(0, 240, 255, 200));
                }

                dl->AddText(ImVec2(st_p0.x + 12.0f, st_p0.y + 6.0f), IM_COL32(0, 230, 255, 240), "VECTORSCOPE ESTÉREO (CORRELAÇÃO M/S)");
                dl->AddText(ImVec2(st_p1.x - 140.0f, st_p0.y + 6.0f), IM_COL32(100, 255, 150, 240), "Fase: +0.94 (Mono Safe)");

                ImGui::Dummy(st_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. MATRIZ DE ROTEAMENTO M/S (4 FADERS CRUZADOS) ─────────
                ImGui::Columns(2, "StereoShaperCols", true);

                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🎛️ MATRIZ DE GANHO DE CANAL (M/S)");
                ImGui::Separator();

                ImGui::SliderFloat("Left -> Left (LL)", &matrix_ll, -1.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Right -> Left (RL)", &matrix_rl, -1.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Left -> Right (LR)", &matrix_lr, -1.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Right -> Right (RR)", &matrix_rr, -1.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // ── 3. SEÇÃO DE ATRASO PSICOACÚSTICO HAAS EFFECT ─────────────
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "⏱️ ATRASO HAAS & INVERSÃO DE FASE");
                ImGui::Separator();

                ImGui::SliderFloat("Delay Canal Esquerdo (L)", &delay_left_ms, 0.0f, 40.0f, "%.1f ms");
                ImGui::SliderFloat("Delay Canal Direito (R)", &delay_right_ms, 0.0f, 40.0f, "%.1f ms");
                
                ImGui::Spacing();
                ImGui::Checkbox("Inverter Fase (L) Ø", &invert_phase_left);
                ImGui::SameLine(180);
                ImGui::Checkbox("Inverter Fase (R) Ø", &invert_phase_right);

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
