#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroBassDrumUI {
    private:
        bool is_open = false;

        // Parâmetros do FL Studio BassDrum (Acoustic & Electronic Kick Drum Synthesizer)
        float start_frequency_hz = 145.0f; // Frequência inicial do pitch sweep (Punch)
        float end_frequency_hz = 48.0f;   // Frequência fundamental do sub-grave (Body)
        float pitch_decay_ms = 45.0f;     // Tempo da queda rápida de afinação
        float amp_decay_ms = 320.0f;      // Tempo de decaimento do corpo do bumbo
        float drive_saturation = 0.55f;   // Saturação e distorção analógica
        float click_amount = 0.70f;       // Transiente de clique mecânico da marreta do pedal
        float sub_boost_level = 0.80f;    // Reforço de sub-graves de 30Hz a 60Hz

    public:
        KuroBassDrumUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.04f, 0.04f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.35f, 0.10f, 0.85f)); // Laranja Fogo Bumbo

            if (ImGui::Begin("🥁 FRUITY BASSDRUM (PHYSICAL & ELECTRONIC KICK SYNTH)###BassDrumWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🥊 Hardstyle / Psytrance Punchy Kick (145Hz -> 48Hz)")) {
                            start_frequency_hz = 180.0f; end_frequency_hz = 52.0f; pitch_decay_ms = 35.0f;
                            amp_decay_ms = 220.0f; drive_saturation = 0.75f; click_amount = 0.85f;
                        }
                        if (ImGui::MenuItem("💥 Deep 808 Trap Sub Kick (Long Tail)")) {
                            start_frequency_hz = 120.0f; end_frequency_hz = 38.0f; pitch_decay_ms = 60.0f;
                            amp_decay_ms = 850.0f; drive_saturation = 0.40f; click_amount = 0.30f;
                        }
                        if (ImGui::MenuItem("🥁 Real Acoustic Rock Maple Kick (Punchy Click)")) {
                            start_frequency_hz = 110.0f; end_frequency_hz = 58.0f; pitch_decay_ms = 25.0f;
                            amp_decay_ms = 180.0f; drive_saturation = 0.20f; click_amount = 0.90f;
                        }
                        if (ImGui::MenuItem("⚡ Industrial Raw Gabber Destructor")) {
                            start_frequency_hz = 240.0f; end_frequency_hz = 45.0f; drive_saturation = 1.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.15f, 1.0f), "FRUITY BASSDRUM: DUAL-LAYER PHYSICAL & ELECTRONIC KICK SYNTHESIZER");
                ImGui::TextDisabled("Sintetizador dedicado de bumbos do FL Studio com modelagem de transiente de clique e queda de afinação sub-grave.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA FORMA DE ONDA DO BUMBO SINTETIZADO ──
                ImVec2 bd_p0 = ImGui::GetCursorScreenPos();
                ImVec2 bd_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 bd_p1 = ImVec2(bd_p0.x + bd_sz.x, bd_p0.y + bd_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(bd_p0, bd_p1, IM_COL32(18, 12, 10, 255), 4.0f);
                dl->AddRect(bd_p0, bd_p1, IM_COL32(60, 35, 20, 255), 4.0f);

                // Desenhar Onda de Kick Sintetizada com Pitch Sweep e Atenuação de Ganho
                int steps = (int)bd_sz.x;
                float cy = bd_p0.y + bd_sz.y * 0.5f;

                for (int x = 0; x < steps - 1; ++x) {
                    float t = (float)x / (float)steps;
                    
                    // Decaimento Exponencial de Pitch & Amplitude
                    float current_freq = end_frequency_hz + (start_frequency_hz - end_frequency_hz) * std::exp(-t * (800.0f / pitch_decay_ms));
                    float amp = std::exp(-t * (1000.0f / amp_decay_ms));
                    
                    // Clique inicial
                    float click_layer = (t < 0.05f) ? (1.0f - t / 0.05f) * click_amount : 0.0f;
                    
                    float kick_val = std::sin(t * current_freq * 0.4f) * amp + click_layer;
                    kick_val = std::tanh(kick_val * (1.0f + drive_saturation * 1.5f));

                    float py0 = cy - kick_val * (bd_sz.y * 0.42f);
                    float py1 = cy - std::tanh((std::sin((t + 0.005f) * current_freq * 0.4f) * amp) * (1.0f + drive_saturation * 1.5f)) * (bd_sz.y * 0.42f);

                    dl->AddLine(ImVec2(bd_p0.x + x, py0), ImVec2(bd_p0.x + x + 1, py1), IM_COL32(255, 120, 30, 240), 2.0f);
                }

                char bd_info[128];
                snprintf(bd_info, sizeof(bd_info), "PUNCH: %.0f Hz -> SUB: %.0f Hz | DECAY: %.0f ms | CLIQUE: %.0f%%", start_frequency_hz, end_frequency_hz, amp_decay_ms, click_amount * 100.0f);
                dl->AddText(ImVec2(bd_p0.x + 12.0f, bd_p0.y + 6.0f), IM_COL32(255, 160, 50, 240), bd_info);

                ImGui::Dummy(bd_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO BUMBO ──────────────────────────
                ImGui::Columns(3, "BassDrumCols", true);

                // Coluna 1: Frequências & Pitch Sweep
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.1f, 1.0f), "🥊 PITCH & FREQUÊNCIAS");
                ImGui::Separator();

                ImGui::SliderFloat("Punch Inicial", &start_frequency_hz, 80.0f, 300.0f, "%.0f Hz");
                ImGui::SliderFloat("Sub Fundamental", &end_frequency_hz, 30.0f, 90.0f, "%.0f Hz");
                ImGui::SliderFloat("Tempo Pitch Sweep", &pitch_decay_ms, 5.0f, 120.0f, "%.0f ms");

                ImGui::NextColumn();

                // Coluna 2: Decaimento & Clique
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "⏱️ CORPO & TRANSIENTE");
                ImGui::Separator();

                ImGui::SliderFloat("Decaimento Corpo", &amp_decay_ms, 50.0f, 1200.0f, "%.0f ms");
                ImGui::SliderFloat("Clique de Marreta", &click_amount, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Saturação / Drive", &drive_saturation, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Disparo de Teste
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🥁 DISPARO DE TESTE");
                ImGui::Separator();

                if (ImGui::Button("💥 TOCAR BUMBO / KICK (TESTE)", ImVec2(-1, 38))) {
                    extern KuroAudio::SynthEngine g_piano_synth;
                    g_piano_synth.triggerNote(36, 1.5f, 1.0f, 0); // Dispara C1 (Kick)
                }

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
