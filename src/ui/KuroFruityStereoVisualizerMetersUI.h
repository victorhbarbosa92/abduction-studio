#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityStereoVisualizerMetersUI {
    private:
        bool is_open = false;

        // Parâmetros do FL Studio Wave Candy (Mastering Spectrum, Oscilloscope & Vectorscope)
        int visualizer_mode = 0;         // 0=Osciloscópio 3D, 1=Spectrum RTA, 2=Vectorscópio Goniômetro, 3=Peak / RMS Meter
        float time_window_ms = 40.0f;    // Janela temporal do osciloscópio (5ms a 200ms)
        float phosphor_decay_rate = 0.82f;// Efeito fósforo analógico de persistência
        float meter_peak_l = -6.2f;      // dBFS
        float meter_peak_r = -5.8f;      // dBFS
        float meter_rms = -12.4f;        // dBFS
        bool freeze_display = false;

    public:
        KuroFruityStereoVisualizerMetersUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(920, 600), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.04f, 0.06f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.30f, 0.90f, 0.85f)); // Rosa Neon Wave Candy

            if (ImGui::Begin("🍬 WAVE CANDY (MASTERING OSCILLOSCOPE, SPECTRUM & GONIOMETER)###WaveCandyWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE MODOS VISUAIS ───────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Modos de Visualização")) {
                        if (ImGui::MenuItem("📟 Osciloscópio Analógico de Tubo (Cathode Ray)")) {
                            visualizer_mode = 0; phosphor_decay_rate = 0.85f;
                        }
                        if (ImGui::MenuItem("🌈 Espectrograma RTA de Alta Resolução")) {
                            visualizer_mode = 1;
                        }
                        if (ImGui::MenuItem("🌐 Goniômetro / Vetorscópio Lissajous")) {
                            visualizer_mode = 2;
                        }
                        if (ImGui::MenuItem("🎚️ Medidor True Peak / RMS")) {
                            visualizer_mode = 3;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.95f, 1.0f), "WAVE CANDY: REALTIME MASTERING VISUALIZER, GONIOMETER & OSCILLOSCOPE");
                ImGui::TextDisabled("A suíte definitiva de medição visual do FL Studio para análise de forma de onda, fase, espectro e correlação.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DO WAVE CANDY ─────────────────────────
                ImVec2 wc_p0 = ImGui::GetCursorScreenPos();
                ImVec2 wc_sz = ImVec2(ImGui::GetContentRegionAvail().x, 200.0f);
                ImVec2 wc_p1 = ImVec2(wc_p0.x + wc_sz.x, wc_p0.y + wc_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(wc_p0, wc_p1, IM_COL32(10, 8, 14, 255), 4.0f);
                dl->AddRect(wc_p0, wc_p1, IM_COL32(45, 20, 50, 255), 4.0f);

                ImVec2 center = ImVec2(wc_p0.x + wc_sz.x * 0.5f, wc_p0.y + wc_sz.y * 0.5f);

                if (visualizer_mode == 0) { // Osciloscópio de Tubo
                    float cy = center.y;
                    dl->AddLine(ImVec2(wc_p0.x, cy), ImVec2(wc_p1.x, cy), IM_COL32(50, 30, 60, 160), 1.0f);

                    int steps = (int)wc_sz.x;
                    for (int x = 0; x < steps - 1; ++x) {
                        float t0 = (float)x / (float)steps;
                        float t1 = (float)(x + 1) / (float)steps;

                        float wave0 = (std::sin(t0 * 6.2831853f * 4.0f) * 0.6f + std::sin(t0 * 6.2831853f * 8.0f) * 0.25f);
                        float wave1 = (std::sin(t1 * 6.2831853f * 4.0f) * 0.6f + std::sin(t1 * 6.2831853f * 8.0f) * 0.25f);

                        float py0 = cy - wave0 * (wc_sz.y * 0.40f);
                        float py1 = cy - wave1 * (wc_sz.y * 0.40f);

                        // Linha com Brilho Neon Fósforo
                        dl->AddLine(ImVec2(wc_p0.x + x, py0), ImVec2(wc_p0.x + x + 1, py1), IM_COL32(255, 60, 220, 240), 2.0f);
                        dl->AddLine(ImVec2(wc_p0.x + x, py0), ImVec2(wc_p0.x + x + 1, py1), IM_COL32(255, 180, 250, 120), 4.0f);
                    }
                } else if (visualizer_mode == 2) { // Goniômetro Lissajous
                    dl->AddLine(ImVec2(center.x, wc_p0.y + 10), ImVec2(center.x, wc_p1.y - 10), IM_COL32(60, 30, 70, 160), 1.0f);
                    dl->AddLine(ImVec2(wc_p0.x + 30, center.y), ImVec2(wc_p1.x - 30, center.y), IM_COL32(60, 30, 70, 160), 1.0f);

                    int dots = 120;
                    for (int d = 0; d < dots; ++d) {
                        float a = (float)d / (float)dots * 6.2831853f;
                        float lx = std::sin(a * 2.0f) * (wc_sz.y * 0.35f) + (((std::rand() % 100) / 100.0f - 0.5f) * 15.0f);
                        float ly = std::cos(a * 3.0f) * (wc_sz.y * 0.35f) + (((std::rand() % 100) / 100.0f - 0.5f) * 15.0f);

                        dl->AddCircleFilled(ImVec2(center.x + lx, center.y + ly), 2.0f, IM_COL32(255, 80, 220, 220));
                    }
                } else { // RTA Spectrum
                    int bands = 48;
                    float bw = (wc_sz.x - 40.0f) / (float)bands;
                    for (int b = 0; b < bands; ++b) {
                        float bh = (std::sin(b * 0.15f) * 0.5f + 0.5f) * (wc_sz.y * 0.70f);
                        float bx = wc_p0.x + 20.0f + b * bw;
                        dl->AddRectFilled(ImVec2(bx, wc_p1.y - 15.0f - bh), ImVec2(bx + bw - 2.0f, wc_p1.y - 15.0f), IM_COL32(255, (int)(40 + b * 4), 180, 220), 2.0f);
                    }
                }

                char wc_info[128];
                const char* mod_n[] = { "Osciloscópio", "Espectrograma RTA", "Goniômetro Lissajous", "Peak / RMS" };
                snprintf(wc_info, sizeof(wc_info), "MODO: %s | PEAK L: %.1f dB | PEAK R: %.1f dB | RMS: %.1f dB", mod_n[visualizer_mode], meter_peak_l, meter_peak_r, meter_rms);
                dl->AddText(ImVec2(wc_p0.x + 12.0f, wc_p0.y + 6.0f), IM_COL32(255, 160, 240, 240), wc_info);

                ImGui::Dummy(wc_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO VISUALIZADOR ───────────────────
                ImGui::Columns(3, "WaveCandyCols", true);

                // Coluna 1: Modos Visuais
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.9f, 1.0f), "📟 MODO DE ANÁLISE");
                ImGui::Separator();

                ImGui::Combo("Modo Visual", &visualizer_mode, mod_n, IM_ARRAYSIZE(mod_n));
                ImGui::Checkbox("❄️ Congelar Tela (Freeze)", &freeze_display);

                ImGui::NextColumn();

                // Coluna 2: Janela Temporal & Persistência
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "⏱️ TEMPO & FÓSFORO");
                ImGui::Separator();

                ImGui::SliderFloat("Janela Temporal", &time_window_ms, 5.0f, 200.0f, "%.0f ms");
                ImGui::SliderFloat("Persistência Fósforo", &phosphor_decay_rate, 0.1f, 0.99f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Medição Peak / RMS
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎚️ NÍVEIS MASTER");
                ImGui::Separator();

                ImGui::ProgressBar(0.78f, ImVec2(-1, 16), "L: -6.2 dBFS");
                ImGui::ProgressBar(0.81f, ImVec2(-1, 16), "R: -5.8 dBFS");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
