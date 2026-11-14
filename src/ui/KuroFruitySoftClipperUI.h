#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruitySoftClipperUI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity Soft Clipper (Mastering Peak Soft Saturator)
        float threshold_db = -0.5f;     // Ponto de saturação suave (-12.0dB a 0.0dB)
        float post_gain_db = 1.5f;      // Ganho de entrada/saída (-12.0dB a +12.0dB)
        float knee_softness = 0.75f;    // Arredondamento da curva de compressão suave
        bool is_oversampling_4x = true; // Anti-Aliasing 4x

    public:
        KuroFruitySoftClipperUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(840, 520), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.50f, 0.10f, 0.85f)); // Laranja / Ouro Soft Clipper

            if (ImGui::Begin("🟧 FRUITY SOFT CLIPPER (ANALOG TAPE MASTER CEILING SATURATOR)###SoftClipperWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🥊 Trap 808 Hard Puncher (Post Gain +3dB)")) {
                            threshold_db = -0.1f; post_gain_db = 3.0f; knee_softness = 0.60f;
                        }
                        if (ImGui::MenuItem("🎛️ Transparent Master Bus Ceiling (-0.3dB)")) {
                            threshold_db = -0.3f; post_gain_db = 0.5f; knee_softness = 0.90f;
                        }
                        if (ImGui::MenuItem("🥁 Drum Bus Glue & Warmth Saturation")) {
                            threshold_db = -2.0f; post_gain_db = 2.0f; knee_softness = 0.80f;
                        }
                        if (ImGui::MenuItem("⚡ Raw Aggressive Heavy Clip")) {
                            threshold_db = 0.0f; post_gain_db = 6.0f; knee_softness = 0.30f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.15f, 1.0f), "FRUITY SOFT CLIPPER: ANALOG-STYLE PEAK SOFT SATURATION & VOLUME CEILING");
                ImGui::TextDisabled("O clipper suave mais utilizado da história da música urbana para dar peso a 808s e colar a bateria sem distorção áspera.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA CURVA DE SOFT-KNEE TRANSFER ────────
                ImVec2 sc_p0 = ImGui::GetCursorScreenPos();
                ImVec2 sc_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 sc_p1 = ImVec2(sc_p0.x + sc_sz.x, sc_p0.y + sc_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(sc_p0, sc_p1, IM_COL32(14, 12, 10, 255), 4.0f);
                dl->AddRect(sc_p0, sc_p1, IM_COL32(50, 35, 20, 255), 4.0f);

                // Linha de Centro (Zero)
                float cy = sc_p0.y + sc_sz.y * 0.5f;
                dl->AddLine(ImVec2(sc_p0.x, cy), ImVec2(sc_p1.x, cy), IM_COL32(60, 45, 30, 160), 1.0f);

                // Desenhar Curva de Compressão e Saturação Suave (Tanh Soft Knee)
                int steps = (int)sc_sz.x;
                for (int x = 0; x < steps - 1; ++x) {
                    float in_val = ((float)x / (float)steps - 0.5f) * 4.0f; // -2.0 a +2.0
                    float in_val_next = (((float)(x + 1) / (float)steps) - 0.5f) * 4.0f;

                    // Função Soft Clip Analógica
                    float gain_linear = std::pow(10.0f, post_gain_db / 20.0f);
                    float out0 = std::tanh(in_val * gain_linear * (1.0f + knee_softness * 0.5f));
                    float out1 = std::tanh(in_val_next * gain_linear * (1.0f + knee_softness * 0.5f));

                    float py0 = cy - out0 * (sc_sz.y * 0.42f);
                    float py1 = cy - out1 * (sc_sz.y * 0.42f);

                    dl->AddLine(ImVec2(sc_p0.x + x, py0), ImVec2(sc_p0.x + x + 1, py1), IM_COL32(255, 140, 30, 240), 2.0f);
                }

                char sc_info[128];
                snprintf(sc_info, sizeof(sc_info), "THRESHOLD: %.1f dB | POST GAIN: %+.1f dB | KNEE: %.0f%% | OVERSAMPLING: %s", threshold_db, post_gain_db, knee_softness * 100.0f, is_oversampling_4x ? "4x HQ" : "1x");
                dl->AddText(ImVec2(sc_p0.x + 12.0f, sc_p0.y + 6.0f), IM_COL32(255, 180, 50, 240), sc_info);

                ImGui::Dummy(sc_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO SOFT CLIPPER ───────────────────
                ImGui::Columns(3, "SoftClipperCols", true);

                // Coluna 1: Threshold & Knee
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.15f, 1.0f), "🎛️ THRESHOLD & KNEE");
                ImGui::Separator();

                ImGui::SliderFloat("Threshold", &threshold_db, -12.0f, 0.0f, "%.1f dB");
                ImGui::SliderFloat("Soft Knee (Curva)", &knee_softness, 0.1f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: Post Gain
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🔊 POST GAIN DRIVE");
                ImGui::Separator();

                ImGui::SliderFloat("Post Gain", &post_gain_db, -12.0f, 12.0f, "%+.1f dB");
                ImGui::Checkbox("4x HQ Oversampling (Anti-Aliasing)", &is_oversampling_4x);

                ImGui::NextColumn();

                // Coluna 3: Nível de Saturação
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🔥 CLIP INTENSITY");
                ImGui::Separator();

                ImGui::ProgressBar(std::clamp((post_gain_db + 6.0f) / 18.0f, 0.0f, 1.0f), ImVec2(-1, 20), "Saturation Drive");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
