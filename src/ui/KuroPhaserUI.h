#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroPhaserUI {
    private:
        bool is_open = false;

        // Parâmetros do Fruity Phaser (Multi-Stage Allpass Analog Phaser)
        int num_stages = 6;              // 2 a 24 estágios Allpass (Stages)
        float sweep_min_hz = 300.0f;     // Frequência mínima da varredura
        float sweep_max_hz = 5500.0f;    // Frequência máxima da varredura
        float lfo_rate_hz = 0.45f;       // Velocidade de oscilação do LFO
        float feedback_intensity = 0.65f;// Realimentação (0% a 95% ressonante)
        float stereo_phase_deg = 90.0f;  // Diferença de fase estéreo L/R (0° a 180°)
        float dry_wet_mix = 0.50f;
        bool invert_phase = false;

    public:
        KuroPhaserUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.80f, 0.20f, 1.00f, 0.85f)); // Roxo Cósmico Phaser

            if (ImGui::Begin("🪐 FRUITY PHASER (MULTI-STAGE ALLPASS MODULATOR)###PhaserWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🪐 Deep Space 12-Stage Analog Sweep")) {
                            num_stages = 12; lfo_rate_hz = 0.25f; feedback_intensity = 0.75f;
                            sweep_min_hz = 200.0f; sweep_max_hz = 7000.0f; stereo_phase_deg = 180.0f;
                        }
                        if (ImGui::MenuItem("🎸 Classic 4-Stage Small Stone Guitar Phaser")) {
                            num_stages = 4; lfo_rate_hz = 0.80f; feedback_intensity = 0.50f;
                            sweep_min_hz = 400.0f; sweep_max_hz = 3500.0f;
                        }
                        if (ImGui::MenuItem("⚡ Psytrance Fast Rotary 8-Stage Zap")) {
                            num_stages = 8; lfo_rate_hz = 3.5f; feedback_intensity = 0.85f;
                        }
                        if (ImGui::MenuItem("📻 Subtle Stereo Pad Widener (2-Stage)")) {
                            num_stages = 2; lfo_rate_hz = 0.15f; feedback_intensity = 0.20f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.85f, 0.35f, 1.0f, 1.0f), "FRUITY PHASER: MULTI-STAGE ALLPASS FILTER CASCADE & STEREO ROTATOR");
                ImGui::TextDisabled("Modulador de deslocamento de fase contínuo por estágios Allpass para criar varreduras psicodélicas e espaciais.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA VARREDURA DOS NOTCHES DE FASE ──────
                ImVec2 ph_p0 = ImGui::GetCursorScreenPos();
                ImVec2 ph_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 ph_p1 = ImVec2(ph_p0.x + ph_sz.x, ph_p0.y + ph_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(ph_p0, ph_p1, IM_COL32(12, 10, 22, 255), 4.0f);
                dl->AddRect(ph_p0, ph_p1, IM_COL32(45, 30, 70, 255), 4.0f);

                // Desenhar Notches de Fase (Picos e Vales de Cancelamento)
                int notches = num_stages / 2;
                if (notches < 1) notches = 1;

                int steps = (int)ph_sz.x;
                float cy = ph_p0.y + ph_sz.y * 0.5f;

                for (int x = 0; x < steps - 1; x += 2) {
                    float t = (float)x / (float)steps;
                    float notch_wave = 0.0f;

                    for (int n = 0; n < notches; ++n) {
                        float center_freq = 0.15f + (float)n * (0.8f / (float)notches);
                        float dist = std::abs(t - center_freq);
                        notch_wave += std::exp(-dist * dist * 350.0f) * (0.7f + feedback_intensity * 0.4f);
                    }

                    float py = cy + (notch_wave - 0.5f) * (ph_sz.y * 0.42f);
                    dl->AddLine(ImVec2(ph_p0.x + x, cy), ImVec2(ph_p0.x + x, py), IM_COL32(180, 50, 255, 180), 1.5f);
                }

                char ph_info[128];
                snprintf(ph_info, sizeof(ph_info), "ESTÁGIOS: %d ALLPASS (%d NOTCHES) | VARREDURA: %.0f Hz - %.0f Hz | LFO: %.2f Hz", num_stages, notches, sweep_min_hz, sweep_max_hz, lfo_rate_hz);
                dl->AddText(ImVec2(ph_p0.x + 12.0f, ph_p0.y + 6.0f), IM_COL32(220, 120, 255, 240), ph_info);

                ImGui::Dummy(ph_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. CONTROLES DE ESTÁGIOS & VARREDURA ─────────────────────
                ImGui::Columns(3, "PhaserCols", true);

                // Coluna 1: Estágios & Realimentação
                ImGui::TextColored(ImVec4(0.85f, 0.4f, 1.0f, 1.0f), "🎛️ ESTÁGIOS & RESSONÂNCIA");
                ImGui::Separator();

                ImGui::SliderInt("Estágios (Stages)", &num_stages, 2, 24);
                ImGui::SliderFloat("Realimentação (Feedback)", &feedback_intensity, 0.0f, 0.95f, "%.2f");
                ImGui::Checkbox("Inverter Fase (Negative Phase)", &invert_phase);

                ImGui::NextColumn();

                // Coluna 2: Faixa de Frequência & LFO
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🌊 FAIXA DE VARREDURA & LFO");
                ImGui::Separator();

                ImGui::SliderFloat("Freq Mínima", &sweep_min_hz, 50.0f, 1000.0f, "%.0f Hz");
                ImGui::SliderFloat("Freq Máxima", &sweep_max_hz, 1500.0f, 16000.0f, "%.0f Hz");
                ImGui::SliderFloat("Velocidade LFO", &lfo_rate_hz, 0.05f, 10.0f, "%.2f Hz");

                ImGui::NextColumn();

                // Coluna 3: Estéreo & Mix
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎧 DEFASAGEM ESTÉREO & MIX");
                ImGui::Separator();

                ImGui::SliderFloat("Fase Estéreo L/R", &stereo_phase_deg, 0.0f, 180.0f, "%.0f°");
                ImGui::SliderFloat("Dry / Wet Mix", &dry_wet_mix, 0.0f, 1.0f, "%.2f");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
