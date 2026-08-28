#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityStereoToolUI {
    private:
        bool is_open = false;

        // Parâmetros do FL Studio Fruity Stereo Shaper Matrix Processor
        float left_to_left = 1.0f;       // LL Ganho
        float left_to_right = 0.0f;      // LR Crossfeed
        float right_to_left = 0.0f;      // RL Crossfeed
        float right_to_right = 1.0f;     // RR Ganho
        float mid_gain = 1.0f;           // Canal Central (Mid)
        float side_gain = 1.0f;          // Canal Lateral (Side)
        bool invert_left_phase = false;
        bool invert_right_phase = false;
        float delay_offset_samples = 0.0f; // Atraso fino em samples

    public:
        KuroFruityStereoToolUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.80f, 1.00f, 0.85f)); // Azul Ciano Estéreo Shaper

            if (ImGui::Begin("🎧 FRUITY STEREO SHAPER (MID/SIDE MATRIX & CROSSFEED PROCESSOR)###StereoShaperWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets Matrix")) {
                        if (ImGui::MenuItem("🎧 Mid/Side Encoder (Isolate Center & Sides)")) {
                            left_to_left = 0.5f; left_to_right = 0.5f; right_to_left = 0.5f; right_to_right = -0.5f;
                            mid_gain = 1.0f; side_gain = 1.2f;
                        }
                        if (ImGui::MenuItem("📻 Pure Mid Solo (Center Vocals Only)")) {
                            mid_gain = 1.5f; side_gain = 0.0f;
                        }
                        if (ImGui::MenuItem("🌌 Pure Side Solo (Ambient Reverb & Width Only)")) {
                            mid_gain = 0.0f; side_gain = 1.5f;
                        }
                        if (ImGui::MenuItem("🔄 Binaural 3D Headphone Crossfeed (Subtle Bleed)")) {
                            left_to_right = 0.25f; right_to_left = 0.25f; delay_offset_samples = 12.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "FRUITY STEREO SHAPER: MULTI-CHANNEL STEREO MATRIX & MID/SIDE PROCESSOR");
                ImGui::TextDisabled("Matriz de roteamento M/S do FL Studio para isolar vocais no centro, expandir ambiências laterais e crossfeed binaural.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA MATRIZ MID / SIDE EM TEMPO REAL ────
                ImVec2 ss_p0 = ImGui::GetCursorScreenPos();
                ImVec2 ss_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 ss_p1 = ImVec2(ss_p0.x + ss_sz.x, ss_p0.y + ss_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(ss_p0, ss_p1, IM_COL32(10, 14, 22, 255), 4.0f);
                dl->AddRect(ss_p0, ss_p1, IM_COL32(30, 50, 70, 255), 4.0f);

                // Desenhar Barras de Nível Mid (Centro) e Side (Laterais)
                float bar_w = (ss_sz.x - 60.0f) * 0.5f;
                float cy = ss_p1.y - 20.0f;

                // Barra Mid
                float h_mid = mid_gain * 0.5f * (ss_sz.y - 40.0f);
                dl->AddRectFilled(ImVec2(ss_p0.x + 20.0f, cy - h_mid), ImVec2(ss_p0.x + 20.0f + bar_w, cy), IM_COL32(0, 200, 255, 200), 3.0f);
                dl->AddText(ImVec2(ss_p0.x + 24.0f, cy - h_mid - 18.0f), IM_COL32(0, 240, 255, 240), "CANAL MID (CENTRO / VOCAIS / BUMBO)");

                // Barra Side
                float h_side = side_gain * 0.5f * (ss_sz.y - 40.0f);
                dl->AddRectFilled(ImVec2(ss_p0.x + 40.0f + bar_w, cy - h_side), ImVec2(ss_p1.x - 20.0f, cy), IM_COL32(255, 60, 180, 200), 3.0f);
                dl->AddText(ImVec2(ss_p0.x + 44.0f + bar_w, cy - h_side - 18.0f), IM_COL32(255, 120, 220, 240), "CANAL SIDE (LATERAIS / REVERB / STEREO)");

                char ss_info[128];
                snprintf(ss_info, sizeof(ss_info), "MID GAIN: %.2fx | SIDE GAIN: %.2fx | INVERSÃO FASE: L:%s R:%s", mid_gain, side_gain, invert_left_phase ? "INV" : "NORM", invert_right_phase ? "INV" : "NORM");
                dl->AddText(ImVec2(ss_p0.x + 12.0f, ss_p0.y + 6.0f), IM_COL32(220, 240, 255, 240), ss_info);

                ImGui::Dummy(ss_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. MATRIZ DE CROSSFEED & NÍVEIS M/S ──────────────────────
                ImGui::Columns(3, "StereoMatrixCols", true);

                // Coluna 1: Ganho Mid / Side
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "🌐 NÍVEIS MID / SIDE");
                ImGui::Separator();

                ImGui::SliderFloat("Ganho Mid (Centro)", &mid_gain, 0.0f, 2.0f, "%.2fx");
                ImGui::SliderFloat("Ganho Side (Laterais)", &side_gain, 0.0f, 2.0f, "%.2fx");

                ImGui::NextColumn();

                // Coluna 2: Matriz de Roteamento Estéreo
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🎛️ CROSSFEED MATRIX");
                ImGui::Separator();

                ImGui::SliderFloat("Left -> Left (LL)", &left_to_left, -1.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Left -> Right (LR)", &left_to_right, -1.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Right -> Left (RL)", &right_to_left, -1.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Right -> Right (RR)", &right_to_right, -1.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Fase & Atraso de Amostras
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "⏱️ FASE & DELAY FINO");
                ImGui::Separator();

                ImGui::Checkbox("Inverter Fase Canal L", &invert_left_phase);
                ImGui::Checkbox("Inverter Fase Canal R", &invert_right_phase);
                ImGui::SliderFloat("Delay Offset", &delay_offset_samples, 0.0f, 64.0f, "%.0f Samples");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
