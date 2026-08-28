#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroTransientProcessorUI {
    private:
        bool is_open = false;

        // Parâmetros do FL Studio Transient Processor
        float attack_boost_db = 4.5f;     // -24dB a +24dB (Punch & Snap de transiente inicial)
        float release_sustain_db = -2.0f; // -24dB a +24dB (Corpo & Cauda do som)
        float split_frequency_hz = 1200.0f;// Frequência de crossover para focar o punch
        float output_gain_db = 0.0f;
        int transient_mode = 0;           // 0=Wideband (Geral), 1=Punch Focus (Bateria), 2=Tail Cleaner (Remover reverberação)

    public:
        KuroTransientProcessorUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.40f, 0.00f, 0.85f)); // Laranja / Ouro Transient Processor

            if (ImGui::Begin("💥 TRANSIENT PROCESSOR (DRUM PUNCH & SUSTAIN SHAPER)###TransientWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🥊 Snappy Kick & Snare Punch (+6dB Attack)")) {
                            attack_boost_db = 6.0f; release_sustain_db = -1.5f; transient_mode = 1;
                        }
                        if (ImGui::MenuItem("🧹 Reverb & Room Tail Killer (-12dB Release)")) {
                            attack_boost_db = 1.0f; release_sustain_db = -12.0f; transient_mode = 2;
                        }
                        if (ImGui::MenuItem("🎸 Acoustic Guitar Strum Enhancer (+4dB)")) {
                            attack_boost_db = 4.0f; release_sustain_db = 3.0f; transient_mode = 0;
                        }
                        if (ImGui::MenuItem("⚡ Extreme Aggressive Smack (+12dB Attack)")) {
                            attack_boost_db = 12.0f; release_sustain_db = -4.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.50f, 0.1f, 1.0f), "TRANSIENT PROCESSOR: INDEPENDENT ATTACK & SUSTAIN DYNAMICS SHAPER");
                ImGui::TextDisabled("Controle independente do impacto inicial (Attack) e da cauda/reverberação (Sustain) sem depender de Threshold fixo.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DO ENVELOPE DE TRANSIENTE EM TEMPO REAL ─
                ImVec2 tp_p0 = ImGui::GetCursorScreenPos();
                ImVec2 tp_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 tp_p1 = ImVec2(tp_p0.x + tp_sz.x, tp_p0.y + tp_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(tp_p0, tp_p1, IM_COL32(12, 14, 20, 255), 4.0f);
                dl->AddRect(tp_p0, tp_p1, IM_COL32(45, 40, 60, 255), 4.0f);

                // Linha de centro 0 dB
                float cy = tp_p0.y + tp_sz.y * 0.5f;
                dl->AddLine(ImVec2(tp_p0.x, cy), ImVec2(tp_p1.x, cy), IM_COL32(50, 60, 75, 160), 1.0f);

                // Desenhar Onda com Pico de Ataque Modificado e Cauda de Sustain
                int steps = (int)tp_sz.x;
                for (int x = 0; x < steps - 1; x += 2) {
                    float t = (float)x / (float)steps;
                    float cycle = std::fmod(t * 3.0f, 1.0f); // 3 Batidas de Bateria

                    // Onda Original vs Processada
                    float raw_attack = (cycle < 0.15f) ? (1.0f - cycle / 0.15f) : 0.0f;
                    float raw_sustain = (cycle >= 0.15f) ? std::exp(-(cycle - 0.15f) * 4.0f) * 0.5f : 0.0f;
                    
                    float shaped_sig = (raw_attack * (1.0f + attack_boost_db * 0.15f) + raw_sustain * (1.0f + release_sustain_db * 0.15f)) * std::sin(cycle * 60.0f);

                    float py = cy - shaped_sig * (tp_sz.y * 0.40f);

                    // Transiente em Laranja Fogo e Sustain em Azul
                    ImU32 col = (cycle < 0.15f) ? IM_COL32(255, 120, 0, 240) : IM_COL32(0, 200, 255, 180);
                    dl->AddLine(ImVec2(tp_p0.x + x, cy), ImVec2(tp_p0.x + x, py), col, 2.0f);
                }

                char tp_info[128];
                const char* m_names[] = { "Wideband (Geral)", "Punch Focus (Bateria)", "Tail Cleaner (Anti-Room)" };
                snprintf(tp_info, sizeof(tp_info), "ATAQUE: %+.1f dB | SUSTAIN: %+.1f dB | MODO: %s", attack_boost_db, release_sustain_db, m_names[transient_mode]);
                dl->AddText(ImVec2(tp_p0.x + 12.0f, tp_p0.y + 6.0f), IM_COL32(255, 180, 50, 240), tp_info);

                ImGui::Dummy(tp_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES PRINCIPAIS DO TRANSIENTE ──────────
                ImGui::Columns(3, "TransientCols", true);

                // Coluna 1: Attack & Punch
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "🥊 ATAQUE (PUNCH / IMPACTO)");
                ImGui::Separator();

                ImGui::SliderFloat("Attack Boost", &attack_boost_db, -24.0f, 24.0f, "%+.1f dB");
                ImGui::Combo("Modo de Transiente", &transient_mode, m_names, IM_ARRAYSIZE(m_names));

                ImGui::NextColumn();

                // Coluna 2: Sustain & Cauda
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "🌊 SUSTAIN (CORPO / CAUDA)");
                ImGui::Separator();

                ImGui::SliderFloat("Release / Sustain", &release_sustain_db, -24.0f, 24.0f, "%+.1f dB");
                ImGui::SliderFloat("Crossover Split", &split_frequency_hz, 200.0f, 5000.0f, "%.0f Hz");

                ImGui::NextColumn();

                // Coluna 3: Nível de Saída & Ganho
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎚️ GANHO DE SAÍDA");
                ImGui::Separator();

                ImGui::SliderFloat("Output Trim", &output_gain_db, -12.0f, 12.0f, "%+.1f dB");
                ImGui::ProgressBar(0.75f, ImVec2(-1, 20), "Punch Intensity: Active");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
