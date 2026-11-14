#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFastLPUI {
    private:
        bool is_open = false;

        // O clássico e instantâneo Fruity Fast LP (Lowpass Filter de Automação Rápida)
        float cutoff_freq = 4500.0f;     // 20Hz a 20000Hz
        float resonance_q = 0.40f;       // 0% a 95%
        float env_mod_depth = 0.0f;      // Modulação de envelope rápida
        float drive_boost = 1.0f;        // Ganho de saturação analógica
        bool auto_sweep_lfo = false;     // LFO de varredura automática para drops

    public:
        KuroFastLPUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(480, 480), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.85f, 0.40f, 0.85f)); // Verde Fast LP

            if (ImGui::Begin("⚡ FRUITY FAST LP (INSTANT ZERO-LATENCY AUTOMATION LOWPASS)###FastLPWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🛸 EDM Drop Build-Up Sweep (300Hz -> 18kHz)")) {
                            cutoff_freq = 800.0f; resonance_q = 0.65f; auto_sweep_lfo = true;
                        }
                        if (ImGui::MenuItem("📻 Underwater / Low-End Filter (600Hz)")) {
                            cutoff_freq = 600.0f; resonance_q = 0.20f;
                        }
                        if (ImGui::MenuItem("🔥 Screaming Acid Resonance (Res 85% + Drive)")) {
                            cutoff_freq = 2800.0f; resonance_q = 0.85f; drive_boost = 2.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.2f, 0.95f, 0.45f, 1.0f), "FRUITY FAST LP: ZERO-CPU AUTOMATION FILTER");
                ImGui::TextDisabled("O filtro passa-baixas clássico do FL Studio para automações rápidas de build-up e transições.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. GRÁFICO VISUAL DA CURVA LOWPASS EM TEMPO REAL ────────
                ImVec2 fl_p0 = ImGui::GetCursorScreenPos();
                ImVec2 fl_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 fl_p1 = ImVec2(fl_p0.x + fl_sz.x, fl_p0.y + fl_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(fl_p0, fl_p1, IM_COL32(10, 15, 20, 255), 4.0f);
                dl->AddRect(fl_p0, fl_p1, IM_COL32(30, 45, 55, 255), 4.0f);

                // Desenhar Curva de Atenuação Passa-Baixas
                float cut_norm = (std::log10(cutoff_freq / 20.0f) / 3.0f); // 20Hz a 20kHz
                int steps = (int)fl_sz.x;

                for (int x = 0; x < steps - 1; x += 2) {
                    float fx_norm = (float)x / (float)steps;
                    float diff = (fx_norm - cut_norm);
                    float resp = (diff < 0.0f) ? 1.0f : std::exp(-diff * 10.0f);
                    if (std::abs(diff) < 0.06f) resp += resonance_q * 0.75f;

                    float py = fl_p1.y - 15.0f - resp * (fl_sz.y * 0.45f);

                    dl->AddLine(ImVec2(fl_p0.x + x, fl_p1.y - 15.0f), ImVec2(fl_p0.x + x, py), IM_COL32(0, 220, 80, 140), 1.5f);
                }

                // Marcador do Ponto de Corte
                float marker_x = fl_p0.x + cut_norm * fl_sz.x;
                dl->AddLine(ImVec2(marker_x, fl_p0.y), ImVec2(marker_x, fl_p1.y), IM_COL32(0, 255, 120, 240), 2.0f);
                dl->AddCircleFilled(ImVec2(marker_x, fl_p1.y - 15.0f - (1.0f + resonance_q * 0.75f) * (fl_sz.y * 0.45f)), 6.0f, IM_COL32(255, 255, 255, 255));

                char fl_info[64];
                snprintf(fl_info, sizeof(fl_info), "CUTOFF: %.0f HZ | RESSONÂNCIA: %.0f%%", cutoff_freq, resonance_q * 100.0f);
                dl->AddText(ImVec2(fl_p0.x + 12.0f, fl_p0.y + 6.0f), IM_COL32(100, 255, 150, 240), fl_info);

                ImGui::Dummy(fl_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. CONTROLES RÁPIDOS DO FAST LP ─────────────────────────
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🎛️ CONTROLE DE CORTE & RESSONÂNCIA");
                ImGui::SliderFloat("Frequência de Corte (Cutoff)", &cutoff_freq, 20.0f, 20000.0f, "%.0f Hz");
                ImGui::SliderFloat("Ressonância (Q Peak)", &resonance_q, 0.0f, 0.95f, "%.2f");
                ImGui::SliderFloat("Drive / Saturação", &drive_boost, 1.0f, 3.0f, "%.2fx");

                ImGui::Spacing();
                ImGui::Checkbox("Varredura LFO Automática (Auto-Sweep Drop)", &auto_sweep_lfo);

                ImGui::Spacing();
                ImGui::ProgressBar(cut_norm, ImVec2(-1, 20), "Cutoff Automation Position");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
