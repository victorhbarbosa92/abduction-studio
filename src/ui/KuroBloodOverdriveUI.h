#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroBloodOverdriveUI {
    private:
        bool is_open = false;

        // Os Parâmetros Clássicos do Fruity Blood Overdrive
        float pre_amp = 2.5f;          // Ganho de entrada e Drive (1.0x a 10.0x)
        float color_filter = 0.55f;    // Filtro tonal de cor (Low/High saturation bias)
        float post_filter = 0.70f;     // Filtro passa-baixas analógico de saída (Taming harsh highs)
        float post_gain = 0.85f;       // Volume de saída
        bool x100_multiplier = false;  // O lendário switch 'x100' para saturação extrema e destruição sonora

    public:
        KuroBloodOverdriveUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(480, 520), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.03f, 0.03f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.15f, 0.15f, 0.85f)); // Vermelho Sangue Blood Overdrive

            if (ImGui::Begin("🩸 FRUITY BLOOD OVERDRIVE (ANALOG TUBE WARMTH & X100 BOOST)###BloodOverdriveWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🔥 Warm Analog 808 Bass Saturator")) {
                            pre_amp = 3.5f; color_filter = 0.40f; post_filter = 0.65f; x100_multiplier = false;
                        }
                        if (ImGui::MenuItem("⚡ Hardstyle & Frenchcore Destructor (x100 ON)")) {
                            pre_amp = 8.0f; color_filter = 0.80f; post_filter = 0.90f; x100_multiplier = true;
                        }
                        if (ImGui::MenuItem("🎙️ Vintage Tube Mic Pre-Amp Glow")) {
                            pre_amp = 1.6f; color_filter = 0.50f; post_filter = 0.50f; x100_multiplier = false;
                        }
                        if (ImGui::MenuItem("🎸 Screaming Lead Guitar Crunch")) {
                            pre_amp = 5.2f; color_filter = 0.70f; post_filter = 0.80f; x100_multiplier = false;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "FRUITY BLOOD OVERDRIVE: ANALOG TUBE COMPRESSION");
                ImGui::TextDisabled("A clássica distorção e saturação valvulada analógica com o icônico modo de ganho extremo x100.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. GRÁFICO VISUAL DA CURVA DE SATURAÇÃO VALVULADA ────────
                ImVec2 bo_p0 = ImGui::GetCursorScreenPos();
                ImVec2 bo_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 bo_p1 = ImVec2(bo_p0.x + bo_sz.x, bo_p0.y + bo_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(bo_p0, bo_p1, IM_COL32(18, 10, 10, 255), 4.0f);
                dl->AddRect(bo_p0, bo_p1, IM_COL32(55, 25, 25, 255), 4.0f);

                // Desenhar Curva de Saturação Suave/Dura
                float effective_drive = x100_multiplier ? (pre_amp * 12.0f) : pre_amp;
                int steps = (int)bo_sz.x;
                float cy = bo_p0.y + bo_sz.y * 0.5f;

                for (int x = 0; x < steps - 1; ++x) {
                    float in_x = ((float)x / (float)steps - 0.5f) * 2.0f; // -1.0 a +1.0
                    // Equação clássica de saturação tangencial analógica
                    float out_y = std::tanh(in_x * effective_drive * 0.8f);

                    float py0 = cy - out_y * (bo_sz.y * 0.44f);
                    float py_next = cy - std::tanh((((float)(x + 1) / (float)steps - 0.5f) * 2.0f) * effective_drive * 0.8f) * (bo_sz.y * 0.44f);

                    dl->AddLine(ImVec2(bo_p0.x + x, py0), ImVec2(bo_p0.x + x + 1, py_next), x100_multiplier ? IM_COL32(255, 40, 40, 255) : IM_COL32(255, 120, 40, 255), 2.5f);
                }

                char bo_info[64];
                snprintf(bo_info, sizeof(bo_info), "DRIVE: %.1fx | MODO X100: %s", effective_drive, x100_multiplier ? "LIGADO (EXTREMO)" : "DESLIGADO");
                dl->AddText(ImVec2(bo_p0.x + 12.0f, bo_p0.y + 6.0f), IM_COL32(255, 100, 100, 240), bo_info);

                ImGui::Dummy(bo_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. CONTROLES DO BLOOD OVERDRIVE ─────────────────────────
                ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.0f, 1.0f), "🎛️ CONTROLES ANALÓGICOS DE SATURAÇÃO");
                ImGui::Separator();

                ImGui::SliderFloat("PreAmp (Drive Inicial)", &pre_amp, 0.1f, 10.0f, "%.1fx");
                ImGui::SliderFloat("Color (Tom da Distorção)", &color_filter, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Post Filter (Filtro Suave)", &post_filter, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Post Gain (Volume Out)", &post_gain, 0.0f, 2.0f, "%.2fx");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 3. O LENDÁRIO BOTÃO 'x100' ──────────────────────────────
                if (x100_multiplier) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.15f, 0.15f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.15f, 0.15f, 0.8f));
                }

                if (ImGui::Button("⚡ O MULTIPLICADOR 'x100' (SATURAÇÃO EXTREMA)", ImVec2(-1, 36))) {
                    x100_multiplier = !x100_multiplier;
                }
                ImGui::PopStyleColor();
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
