#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityStereoReverbDelayPanChorusUI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity DX10 (FM Electric Piano & Percussive FM Synth)
        float operator_ratio_coarse = 2.0f; // Multiplicador de harmônico do modulador
        float operator_ratio_fine = 0.0f;   // Ajuste fino
        float fm_mod_index = 0.65f;         // Intensidade de modulação FM (Brilho metálico / Bell tone)
        float decay_time_ms = 450.0f;       // Decaimento percussivo
        float release_time_ms = 180.0f;     // Release da cauda
        float stereo_chorus_depth = 0.40f;  // Chorus estéreo de piano elétrico vintage
        float wave_hardness = 0.50f;        // Dureza do timbre (Soft Sine a Hard FM Pulse)

    public:
        KuroFruityStereoReverbDelayPanChorusUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.80f, 1.00f, 0.85f)); // Azul Yamaha FM DX10

            if (ImGui::Begin("🎹 FRUITY DX10 (FM ELECTRIC PIANO & BELL SYNTH)###DX10Window", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets DX FM")) {
                        if (ImGui::MenuItem("🎹 Classic 80s Yamaha DX7 Electric Piano (Rhodes)")) {
                            operator_ratio_coarse = 2.0f; fm_mod_index = 0.60f; decay_time_ms = 600.0f;
                            stereo_chorus_depth = 0.75f; wave_hardness = 0.45f;
                        }
                        if (ImGui::MenuItem("🔔 Crystal Clear Tubular Bell (Tubular Chimes)")) {
                            operator_ratio_coarse = 3.5f; fm_mod_index = 0.85f; decay_time_ms = 1200.0f;
                            stereo_chorus_depth = 0.30f; wave_hardness = 0.80f;
                        }
                        if (ImGui::MenuItem("🎸 Metallic FM Slap Bass (Seinfeld / 90s Style)")) {
                            operator_ratio_coarse = 1.0f; fm_mod_index = 0.75f; decay_time_ms = 180.0f;
                            stereo_chorus_depth = 0.15f; wave_hardness = 0.90f;
                        }
                        if (ImGui::MenuItem("👽 Sci-Fi Alien Cyber Glass Pluck")) {
                            operator_ratio_coarse = 5.0f; fm_mod_index = 0.95f; decay_time_ms = 350.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "FRUITY DX10: 2-OPERATOR FM SYNTHESIZER & ELECTRIC PIANO ENGINE");
                ImGui::TextDisabled("O clássico sintetizador FM de pianos elétricos dos anos 80, sinos cristalinos e baixos metálicos do FL Studio.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA FORMA DE ONDA FM CALCULADA ─────────
                ImVec2 dx_p0 = ImGui::GetCursorScreenPos();
                ImVec2 dx_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 dx_p1 = ImVec2(dx_p0.x + dx_sz.x, dx_p0.y + dx_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(dx_p0, dx_p1, IM_COL32(10, 15, 24, 255), 4.0f);
                dl->AddRect(dx_p0, dx_p1, IM_COL32(30, 50, 75, 255), 4.0f);

                float cy = dx_p0.y + dx_sz.y * 0.5f;
                dl->AddLine(ImVec2(dx_p0.x, cy), ImVec2(dx_p1.x, cy), IM_COL32(40, 65, 90, 160), 1.0f);

                int steps = (int)dx_sz.x;
                for (int x = 0; x < steps - 1; ++x) {
                    float t0 = (float)x / (float)steps * 6.2831853f * 2.0f;
                    float t1 = (float)(x + 1) / (float)steps * 6.2831853f * 2.0f;

                    // Síntese FM: Modulador modula a fase da Portadora
                    float mod0 = std::sin(t0 * (operator_ratio_coarse + operator_ratio_fine)) * (fm_mod_index * 3.0f);
                    float mod1 = std::sin(t1 * (operator_ratio_coarse + operator_ratio_fine)) * (fm_mod_index * 3.0f);

                    float car0 = std::sin(t0 + mod0);
                    float car1 = std::sin(t1 + mod1);

                    float py0 = cy - car0 * (dx_sz.y * 0.40f);
                    float py1 = cy - car1 * (dx_sz.y * 0.40f);

                    dl->AddLine(ImVec2(dx_p0.x + x, py0), ImVec2(dx_p0.x + x + 1, py1), IM_COL32(0, 220, 255, 240), 2.0f);
                }

                char dx_info[128];
                snprintf(dx_info, sizeof(dx_info), "RATIO MOD: %.1fx | FM INDEX: %.0f%% | DECAY: %.0f ms | CHORUS: %.0f%%", operator_ratio_coarse + operator_ratio_fine, fm_mod_index * 100.0f, decay_time_ms, stereo_chorus_depth * 100.0f);
                dl->AddText(ImVec2(dx_p0.x + 12.0f, dx_p0.y + 6.0f), IM_COL32(0, 240, 255, 240), dx_info);

                ImGui::Dummy(dx_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO DX10 ───────────────────────────
                ImGui::Columns(3, "DX10Cols", true);

                // Coluna 1: Operador Modulador FM
                ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "⚡ OPERADOR MODULADOR");
                ImGui::Separator();

                ImGui::SliderFloat("Ratio Harmônico", &operator_ratio_coarse, 0.5f, 12.0f, "%.1fx");
                ImGui::SliderFloat("FM Mod Index", &fm_mod_index, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Dureza Timbre", &wave_hardness, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: Envelope & Decaimento
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "⏱️ ENVELOPE DECAY");
                ImGui::Separator();

                ImGui::SliderFloat("Decay Time", &decay_time_ms, 20.0f, 2000.0f, "%.0f ms");
                ImGui::SliderFloat("Release Time", &release_time_ms, 10.0f, 800.0f, "%.0f ms");
                ImGui::SliderFloat("Stereo Chorus", &stereo_chorus_depth, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Disparo de Teste
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎹 DISPARO DE TESTE");
                ImGui::Separator();

                if (ImGui::Button("🎹 TOCAR RHODES FM (TESTE)", ImVec2(-1, 38))) {
                    extern KuroAudio::SynthEngine g_piano_synth;
                    g_piano_synth.triggerNote(60, 2.0f, 0.90f, 0); // Dispara C4 Rhodes
                }

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
