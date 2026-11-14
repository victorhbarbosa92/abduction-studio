#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFlangusChorusUI {
    private:
        bool is_open = false;

        // Parâmetros do Fruity Flangus & Vintage Chorus (BBD Analog Bucket-Brigade Modulator)
        float mod_depth = 0.65f;         // Profundidade do LFO (0% a 100%)
        float mod_speed_hz = 0.85f;      // Velocidade do LFO (0.1Hz a 10.0Hz)
        float delay_offset_ms = 4.5f;    // Delay central (1ms = Flanger, 15ms = Chorus, 35ms = Doubler)
        float feedback_resonance = 0.45f;// Realimentação (Flanger metálico de jato de avião)
        float stereo_cross_phase = 0.75f;// Diferença de fase estéreo de 90° / 180°
        float dry_wet_mix = 0.50f;       // Balanço Dry / Wet
        int chorus_mode = 1;             // 0=Metallic Flanger, 1=Roland Juno-60 BBD Chorus, 2=Stereo Dimension Widener

    public:
        KuroFlangusChorusUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.80f, 1.00f, 0.85f)); // Azul Ciano Flangus

            if (ImGui::Begin("🌊 FRUITY FLANGUS & VINTAGE BBD CHORUS (ANALOG FLANGER & ENSEMBLE)###FlangusWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🌊 Roland Juno-60 Ensemble (Thick Synth Stereo)")) {
                            chorus_mode = 1; delay_offset_ms = 12.0f; mod_speed_hz = 0.65f;
                            mod_depth = 0.70f; feedback_resonance = 0.0f; dry_wet_mix = 0.5f;
                        }
                        if (ImGui::MenuItem("🚀 Jet Plane Resonant Flanger (Through-Zero)")) {
                            chorus_mode = 0; delay_offset_ms = 1.8f; mod_speed_hz = 0.35f;
                            mod_depth = 0.85f; feedback_resonance = 0.75f; dry_wet_mix = 0.6f;
                        }
                        if (ImGui::MenuItem("👽 Psytrance Metallic Lead Comb Filter")) {
                            chorus_mode = 0; delay_offset_ms = 3.2f; mod_speed_hz = 2.5f;
                            feedback_resonance = 0.80f;
                        }
                        if (ImGui::MenuItem("🌌 3D Dimension Space Doubler")) {
                            chorus_mode = 2; delay_offset_ms = 25.0f; stereo_cross_phase = 1.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "FRUITY FLANGUS & VINTAGE BBD CHORUS: ANALOG BUCKET-BRIGADE ENSEMBLE");
                ImGui::TextDisabled("Modulador de atraso de linha BBD analógico para efeitos de Flanger metálico, Chorus Roland Juno e Doubler.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA MODULAÇÃO DE FASE E COMB FILTER ────
                ImVec2 flg_p0 = ImGui::GetCursorScreenPos();
                ImVec2 flg_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 flg_p1 = ImVec2(flg_p0.x + flg_sz.x, flg_p0.y + flg_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(flg_p0, flg_p1, IM_COL32(10, 14, 22, 255), 4.0f);
                dl->AddRect(flg_p0, flg_p1, IM_COL32(35, 50, 70, 255), 4.0f);

                // Desenhar Curva de Comb Filter Estéreo (Dentes de Pente do Flanger)
                int steps = (int)flg_sz.x;
                float cy = flg_p0.y + flg_sz.y * 0.5f;

                for (int x = 0; x < steps - 1; x += 2) {
                    float t = (float)x / (float)steps;
                    float freq_comb = (delay_offset_ms * 2.5f) + (mod_depth * 10.0f);
                    
                    // Resposta do pente L e R com defasagem estéreo
                    float comb_l = std::cos(t * freq_comb * 6.2831853f) * (0.5f + feedback_resonance * 0.5f);
                    float comb_r = std::cos(t * freq_comb * 6.2831853f + stereo_cross_phase * 3.14159265f) * (0.5f + feedback_resonance * 0.5f);

                    float py_l = cy - comb_l * (flg_sz.y * 0.38f);
                    float py_r = cy - comb_r * (flg_sz.y * 0.38f);

                    // Canal L em Ciano e Canal R em Magenta
                    dl->AddLine(ImVec2(flg_p0.x + x, cy), ImVec2(flg_p0.x + x, py_l), IM_COL32(0, 220, 255, 160), 1.5f);
                    dl->AddLine(ImVec2(flg_p0.x + x, cy), ImVec2(flg_p0.x + x, py_r), IM_COL32(255, 60, 180, 120), 1.5f);
                }

                char flg_info[128];
                const char* m_names[] = { "Metallic Flanger", "Roland Juno-60 BBD Chorus", "Stereo Dimension Doubler" };
                snprintf(flg_info, sizeof(flg_info), "MODO: %s | DELAY: %.1f ms | VELOCIDADE: %.2f Hz | RES: %.0f%%", m_names[chorus_mode], delay_offset_ms, mod_speed_hz, feedback_resonance * 100.0f);
                dl->AddText(ImVec2(flg_p0.x + 12.0f, flg_p0.y + 6.0f), IM_COL32(0, 230, 255, 240), flg_info);

                ImGui::Dummy(flg_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO FLANGUS / CHORUS ────────────────
                ImGui::Columns(3, "FlangusCols", true);

                // Coluna 1: Modo & Delay Offset
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "🎛️ MODO & DELAY OFFSET");
                ImGui::Separator();

                ImGui::Combo("Modo de Efeito", &chorus_mode, m_names, IM_ARRAYSIZE(m_names));
                ImGui::SliderFloat("Delay Central", &delay_offset_ms, 0.5f, 40.0f, "%.1f ms");
                ImGui::SliderFloat("Realimentação (Res)", &feedback_resonance, -0.95f, 0.95f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: LFO de Modulação
                ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.2f, 1.0f), "🌊 LFO DE MODULAÇÃO");
                ImGui::Separator();

                ImGui::SliderFloat("Profundidade (Depth)", &mod_depth, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Velocidade (Speed)", &mod_speed_hz, 0.05f, 10.0f, "%.2f Hz");
                ImGui::SliderFloat("Defasagem Estéreo (Phase)", &stereo_cross_phase, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Níveis & Mix
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎚️ NÍVEIS & BALANÇO");
                ImGui::Separator();

                ImGui::SliderFloat("Dry / Wet Mix", &dry_wet_mix, 0.0f, 1.0f, "%.2f");
                ImGui::ProgressBar(dry_wet_mix, ImVec2(-1, 20), "Chorus Blend");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
