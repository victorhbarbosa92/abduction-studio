#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruityFilterUI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity Filter (Multimode State Variable Analog Filter)
        int filter_mode = 0;             // 0=Lowpass 24dB, 1=Highpass 24dB, 2=Bandpass 12dB
        float cutoff_freq_hz = 3200.0f;  // 20Hz a 20000Hz
        float resonance_q = 0.55f;       // 0% a 95%
        float lfo_depth = 0.40f;         // Profundidade da modulação LFO
        float lfo_speed_hz = 1.2f;       // Velocidade LFO (0.1Hz a 15.0Hz)
        bool lfo_tempo_sync = false;

    public:
        KuroFruityFilterUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.10f, 0.90f, 0.80f, 0.85f)); // Verde-Azul Filtro

            if (ImGui::Begin("🎛️ FRUITY FILTER (ANALOG STATE-VARIABLE MULTIMODE FILTER)###FilterWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🛸 Acid 303 Screaming Resonant Lowpass")) {
                            filter_mode = 0; cutoff_freq_hz = 2400.0f; resonance_q = 0.85f; lfo_depth = 0.60f; lfo_speed_hz = 2.0f;
                        }
                        if (ImGui::MenuItem("📻 Highpass DJ Transition Sweep (500Hz)")) {
                            filter_mode = 1; cutoff_freq_hz = 500.0f; resonance_q = 0.30f;
                        }
                        if (ImGui::MenuItem("👽 Bandpass Radio Vocal Formant")) {
                            filter_mode = 2; cutoff_freq_hz = 1500.0f; resonance_q = 0.70f;
                        }
                        if (ImGui::MenuItem("🌊 Slow Ambient Cutoff Swell (0.25 Hz)")) {
                            filter_mode = 0; cutoff_freq_hz = 1200.0f; resonance_q = 0.40f; lfo_depth = 0.80f; lfo_speed_hz = 0.25f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.1f, 0.95f, 0.85f, 1.0f), "FRUITY FILTER: 24dB STATE-VARIABLE ANALOG FILTER & INTEGRATED LFO");
                ImGui::TextDisabled("Filtro analógico de estado variável do FL Studio com curvas de Passa-Baixas, Passa-Altas e Passa-Banda.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA CURVA DE RESPOSTA EM FREQUÊNCIA ─────
                ImVec2 ff_p0 = ImGui::GetCursorScreenPos();
                ImVec2 ff_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 ff_p1 = ImVec2(ff_p0.x + ff_sz.x, ff_p0.y + ff_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(ff_p0, ff_p1, IM_COL32(10, 15, 20, 255), 4.0f);
                dl->AddRect(ff_p0, ff_p1, IM_COL32(25, 45, 55, 255), 4.0f);

                // Desenhar Curva de Filtro (LP, HP ou BP)
                float cut_norm = (std::log10(cutoff_freq_hz / 20.0f) / 3.0f); // 20Hz a 20kHz
                int steps = (int)ff_sz.x;

                for (int x = 0; x < steps - 1; x += 2) {
                    float fx = (float)x / (float)steps;
                    float resp = 0.0f;

                    if (filter_mode == 0) { // Lowpass
                        float diff = fx - cut_norm;
                        resp = (diff < 0.0f) ? 1.0f : std::exp(-diff * 12.0f);
                        if (std::abs(diff) < 0.06f) resp += resonance_q * 0.8f;
                    } else if (filter_mode == 1) { // Highpass
                        float diff = cut_norm - fx;
                        resp = (diff < 0.0f) ? 1.0f : std::exp(-diff * 12.0f);
                        if (std::abs(diff) < 0.06f) resp += resonance_q * 0.8f;
                    } else { // Bandpass
                        float diff = std::abs(fx - cut_norm);
                        resp = std::exp(-diff * 15.0f) * (1.0f + resonance_q * 0.9f);
                    }

                    float py = ff_p1.y - 12.0f - resp * (ff_sz.y * 0.45f);
                    dl->AddLine(ImVec2(ff_p0.x + x, ff_p1.y - 12.0f), ImVec2(ff_p0.x + x, py), IM_COL32(0, 230, 200, 160), 1.5f);
                }

                // Marcador do Ponto de Corte
                float marker_x = ff_p0.x + cut_norm * ff_sz.x;
                dl->AddLine(ImVec2(marker_x, ff_p0.y), ImVec2(marker_x, ff_p1.y), IM_COL32(0, 255, 230, 240), 2.0f);

                char ff_info[128];
                const char* m_names[] = { "Passa-Baixas (Lowpass 24dB)", "Passa-Altas (Highpass 24dB)", "Passa-Banda (Bandpass 12dB)" };
                snprintf(ff_info, sizeof(ff_info), "MODO: %s | CORTE: %.0f Hz | RES: %.0f%% | LFO: %.2f Hz", m_names[filter_mode], cutoff_freq_hz, resonance_q * 100.0f, lfo_speed_hz);
                dl->AddText(ImVec2(ff_p0.x + 12.0f, ff_p0.y + 6.0f), IM_COL32(0, 255, 220, 240), ff_info);

                ImGui::Dummy(ff_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO FILTRO ─────────────────────────
                ImGui::Columns(3, "FruityFilterCols", true);

                // Coluna 1: Tipo & Corte
                ImGui::TextColored(ImVec4(0.1f, 0.95f, 0.85f, 1.0f), "🎛️ MODO & CORTE");
                ImGui::Separator();

                ImGui::Combo("Tipo de Filtro", &filter_mode, m_names, IM_ARRAYSIZE(m_names));
                ImGui::SliderFloat("Frequência de Corte", &cutoff_freq_hz, 20.0f, 20000.0f, "%.0f Hz");
                ImGui::SliderFloat("Ressonância (Q)", &resonance_q, 0.0f, 0.95f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: Modulador LFO Integrado
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🌊 MODULAÇÃO LFO");
                ImGui::Separator();

                ImGui::SliderFloat("Profundidade LFO", &lfo_depth, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Velocidade LFO", &lfo_speed_hz, 0.05f, 15.0f, "%.2f Hz");
                ImGui::Checkbox("Sincronizar LFO com BPM", &lfo_tempo_sync);

                ImGui::NextColumn();

                // Coluna 3: Nível de Saída
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎚️ RESPOSTA");
                ImGui::Separator();

                ImGui::ProgressBar(cut_norm, ImVec2(-1, 20), "Cutoff Position");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
