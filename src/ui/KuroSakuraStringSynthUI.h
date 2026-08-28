#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroSakuraStringSynthUI {
    private:
        bool is_open = false;

        // Modelagem Física de Cordas Sakura (FL Studio Physical Modeling)
        float string_damping = 0.35f;     // Amortecimento da corda (Madeira / Metal / Nylon)
        float string_tension = 0.80f;     // Tensão e afinação fina
        float body_resonance = 0.75f;     // Cavidade Acústica / Tampo de Ressonância (Body Cavity)
        float pluck_hardness = 0.60f;     // Dureza da palheta / Dedo (Impulso de Ataque)
        int string_material = 0;          // 0=Nylon Guitar, 1=Steel Acoustic, 2=Koto/Sitar, 3=Bowed Cello
        float dual_pickup_pos = 0.45f;    // Posição dos 2 captadores virtuais na corda
        float acoustic_air_reverb = 0.40f;
        float warmth_drive = 0.25f;

    public:
        KuroSakuraStringSynthUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 600), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.06f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.55f, 0.75f, 0.85f)); // Rosa Sakura

            if (ImGui::Begin("🌸 SAKURA PHYSICAL MODELING STRING INSTRUMENT (FL STUDIO STYLE)###SakuraWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS DO SAKURA ───────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🌸 Japanese Koto & Shamisen Pluck")) {
                            string_material = 2; pluck_hardness = 0.85f; string_damping = 0.5f;
                        }
                        if (ImGui::MenuItem("🎸 Nylon Concert Acoustic Guitar")) {
                            string_material = 0; pluck_hardness = 0.45f; body_resonance = 0.85f;
                        }
                        if (ImGui::MenuItem("🎻 Expressive Bowed Cello / Erhu")) {
                            string_material = 3; pluck_hardness = 0.20f; string_damping = 0.15f;
                        }
                        if (ImGui::MenuItem("✨ Metallic Sitar Drone & Resonant sympathetic strings")) {
                            string_material = 2; body_resonance = 0.95f; warmth_drive = 0.5f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.60f, 0.80f, 1.0f), "SAKURA PHYSICAL MODELING ACOUSTIC STRING & BODY CAVITY SYNTHESIZER");
                ImGui::TextDisabled("Sintetizador de modelagem física de cordas que simula vibração mecânica de nylon, aço, palhetas e ressonância de madeira real.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA VIBRAÇÃO DA CORDA EM TEMPO REAL ─────
                ImVec2 str_p0 = ImGui::GetCursorScreenPos();
                ImVec2 str_sz = ImVec2(ImGui::GetContentRegionAvail().x, 140.0f);
                ImVec2 str_p1 = ImVec2(str_p0.x + str_sz.x, str_p0.y + str_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(str_p0, str_p1, IM_COL32(14, 16, 22, 255), 4.0f);
                dl->AddRect(str_p0, str_p1, IM_COL32(50, 40, 55, 255), 4.0f);

                // Desenhar 6 Cordas Vibratórias Estilizadas com Curva Física (Standing Waves)
                float cy = str_p0.y + str_sz.y * 0.5f;
                int points = (int)str_sz.x;

                for (int corda = 0; corda < 6; ++corda) {
                    float corda_y = str_p0.y + 20.0f + corda * 20.0f;
                    float harm_mult = (float)(corda + 1);

                    for (int x = 0; x < points; x += 3) {
                        float t = (float)x / (float)points;
                        float wave = std::sin(t * 3.14159265f * harm_mult) * std::sin(t * 25.0f + corda) * (1.0f - string_damping * 0.6f);
                        float py = corda_y - wave * 12.0f;

                        dl->AddLine(ImVec2(str_p0.x + x, corda_y), ImVec2(str_p0.x + x, py), IM_COL32(255, 120 + corda * 20, 180, 190), 1.2f);
                    }

                    // Ponto de palheta / ataque
                    float pluck_x = str_p0.x + dual_pickup_pos * str_sz.x;
                    dl->AddCircleFilled(ImVec2(pluck_x, corda_y), 3.0f, IM_COL32(255, 255, 255, 240));
                }

                char str_info[64];
                const char* mat_names[] = { "Nylon Concert", "Steel Acoustic", "Koto / Sitar", "Bowed Cello / Erhu" };
                snprintf(str_info, sizeof(str_info), "CORDA: %s | RESSONÂNCIA DE MADEIRA: %.0f%%", mat_names[string_material], body_resonance * 100.0f);
                dl->AddText(ImVec2(str_p0.x + 12.0f, str_p0.y + 6.0f), IM_COL32(255, 180, 220, 240), str_info);

                ImGui::Dummy(str_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. CONTROLES DE MODELAGEM FÍSICA ────────────────────────
                ImGui::Columns(3, "SakuraCols", true);

                // Coluna 1: Material da Corda
                ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.85f, 1.0f), "🎻 MATERIAL DA CORDA");
                ImGui::Separator();

                ImGui::Combo("Material##sm", &string_material, mat_names, IM_ARRAYSIZE(mat_names));
                ImGui::SliderFloat("Amortecimento (Damping)", &string_damping, 0.01f, 1.0f, "%.2f");
                ImGui::SliderFloat("Tensão da Corda", &string_tension, 0.1f, 1.5f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: Ataque / Pluck & Captadores
                ImGui::TextColored(ImVec4(0.3f, 0.9f, 1.0f, 1.0f), "🖐️ IMPULSO DE ATAQUE");
                ImGui::Separator();

                ImGui::SliderFloat("Dureza da Palheta (Pluck)", &pluck_hardness, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Posição do Captador", &dual_pickup_pos, 0.1f, 0.9f, "%.2f");
                ImGui::SliderFloat("Saturação Quente (Warmth)", &warmth_drive, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Corpo Acústico de Madeira
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "🪵 CORPO & RESSONÂNCIA");
                ImGui::Separator();

                ImGui::SliderFloat("Cavidade de Madeira", &body_resonance, 0.1f, 1.0f, "%.2f");
                ImGui::SliderFloat("Reverb Espacial de Ar", &acoustic_air_reverb, 0.0f, 1.0f, "%.2f");

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Botão de Disparo / Teste
                if (ImGui::Button("🌸 TOCAR SAKURA (C4 - CORDA ACÚSTICA DEDILHADA)", ImVec2(-1, 38))) {
                    extern KuroAudio::SynthEngine g_piano_synth;
                    g_piano_synth.triggerNote(60, 2.0f, 0.9f, 3); // Dispara contrabaixo/corda física
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
