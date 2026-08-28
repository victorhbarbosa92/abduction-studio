#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroSoundgoodizerUI {
    private:
        bool is_open = false;

        // O Lendário Knob Único do Soundgoodizer (FL Studio Classic Maximus Preset Host)
        float goodizing_amount = 0.55f; // 0% a 100%
        int active_mode = 0;           // 0=Preset A (Warm & Punchy), 1=Preset B (Deep Bass), 2=Preset C (Bright & Sparkly), 3=Preset D (Loud & Aggressive)

    public:
        KuroSoundgoodizerUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(440, 520), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.80f, 0.00f, 0.90f)); // Ouro / Laranja Soundgoodizer

            if (ImGui::Begin("✨ SOUNDGOODIZER (INSTANT STEREO POLISH & MASTERING GLOW)###SoundgoodizerWindow", &is_open)) {
                
                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.1f, 1.0f), "SOUNDGOODIZER: INSTANT MULTIBAND POLISH");
                ImGui::TextDisabled("O icônico processador de botão único baseado nas 4 curvas analógicas clássicas do Maximus.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. OS 4 MODOS CLÁSSICOS (A, B, C, D) ─────────────────────
                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.00f, 1.0f), "SELECIONE O PRESET DE MATRIZ:");
                ImGui::Spacing();

                const char* mode_letters[] = { "A", "B", "C", "D" };
                const char* mode_desc[] = { "A (Warm & Punchy)", "B (Deep Bass & Sub)", "C (High Sparkle Air)", "D (In-Your-Face Aggressive)" };

                for (int m = 0; m < 4; ++m) {
                    bool is_active = (active_mode == m);
                    if (is_active) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.70f, 0.0f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.18f, 0.22f, 0.8f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.85f, 0.9f, 1.0f));
                    }

                    if (ImGui::Button(mode_letters[m], ImVec2(90, 32))) {
                        active_mode = m;
                    }
                    ImGui::PopStyleColor(2);
                    if (m < 3) ImGui::SameLine();
                }

                ImGui::Spacing();
                ImGui::TextDisabled("Modo Selecionado: %s", mode_desc[active_mode]);
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. O GRANDE KNOB CENTRAL BRILHANTE (THE BIG GOODIZING KNOB)
                ImVec2 knob_center = ImGui::GetCursorScreenPos();
                float knob_radius = 85.0f;
                knob_center.x += ImGui::GetContentRegionAvail().x * 0.5f;
                knob_center.y += knob_radius + 15.0f;

                ImDrawList* dl = ImGui::GetWindowDrawList();

                // Fundo do Knob
                dl->AddCircleFilled(knob_center, knob_radius + 10.0f, IM_COL32(18, 22, 30, 255));
                dl->AddCircle(knob_center, knob_radius + 10.0f, IM_COL32(45, 55, 75, 255), 2.0f);

                // Arco de Brilho Dinâmico (0% a 100%)
                float angle_start = -2.356194f; // -135 deg
                float angle_end = 2.356194f;    // +135 deg
                float current_angle = angle_start + goodizing_amount * (angle_end - angle_start);

                int arc_segments = 36;
                for (int s = 0; s < arc_segments; ++s) {
                    float t = (float)s / (float)arc_segments;
                    float seg_angle = angle_start + t * (angle_end - angle_start);
                    if (seg_angle > current_angle) break;

                    float ax = knob_center.x + std::sin(seg_angle) * (knob_radius + 5.0f);
                    float ay = knob_center.y - std::cos(seg_angle) * (knob_radius + 5.0f);
                    dl->AddCircleFilled(ImVec2(ax, ay), 3.5f, IM_COL32(255, (int)(160 + t * 95), 0, 255));
                }

                // Corpo Interno do Knob
                dl->AddCircleFilled(knob_center, knob_radius, IM_COL32(28, 35, 48, 255));
                dl->AddCircle(knob_center, knob_radius, IM_COL32(255, 180, 0, (int)(100 + goodizing_amount * 155)), 2.5f);

                // Ponteiro Indicador do Knob
                float ptr_x = knob_center.x + std::sin(current_angle) * (knob_radius * 0.75f);
                float ptr_y = knob_center.y - std::cos(current_angle) * (knob_radius * 0.75f);
                dl->AddLine(knob_center, ImVec2(ptr_x, ptr_y), IM_COL32(255, 240, 100, 255), 4.0f);
                dl->AddCircleFilled(knob_center, 8.0f, IM_COL32(255, 180, 0, 255));

                // Texto Central de Porcentagem
                char pct_txt[16];
                snprintf(pct_txt, sizeof(pct_txt), "%.0f%%", goodizing_amount * 100.0f);
                ImVec2 txt_sz = ImGui::CalcTextSize(pct_txt);
                dl->AddText(ImVec2(knob_center.x - txt_sz.x * 0.5f, knob_center.y + 35.0f), IM_COL32(255, 220, 120, 240), pct_txt);

                ImGui::Dummy(ImVec2(0, knob_radius * 2.0f + 30.0f));

                // ── 3. CONTROLE DESLIZANTE DO SOUNDGOODIZER ──────────────────
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "INTENSIDADE DO SOUNDGOODIZER:");
                ImGui::SliderFloat("##good_slider", &goodizing_amount, 0.0f, 1.0f, "%.2f");

                ImGui::Spacing();
                ImGui::ProgressBar(goodizing_amount, ImVec2(-1, 18), "Harmonic Saturation & Maximizer Active");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
