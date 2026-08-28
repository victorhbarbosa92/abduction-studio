#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroDelay3UI {
    private:
        bool is_open = false;

        // Parâmetros do Fruity Delay 3 (Analog / Digital / Tape Hybrid Delay)
        float delay_time_steps = 4.0f;    // 1/4 note (4 steps), 1/8 note (2 steps), etc.
        float feedback_level = 0.55f;     // Realimentação (0% a 110% com auto-oscilação)
        float dry_level = 1.0f;
        float wet_level = 0.40f;
        
        // Seção Analógica Tape & Saturação
        float tape_wobble_flutter = 0.25f;// Modulação LFO de pitch do delay analógico
        float drive_saturation = 0.35f;   // Saturação quente de fita
        float bitcrush_reduction = 0.0f;  // Bitcrusher Lo-Fi digital (0 = 24-bit limpo, 1 = 8-bit cru)
        float sample_rate_reduction = 0.0f;

        // Filtros de Feedback (Lowpass / Highpass de Amortecimento)
        float filter_lowpass_cut = 6500.0f; // Hz
        float filter_highpass_cut = 150.0f; // Hz
        float stereo_ping_pong = 0.80f;   // Espalhamento Ping-Pong Estéreo
        bool reverse_echo_mode = false;

    public:
        KuroDelay3UI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 620), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 0.70f, 0.85f)); // Verde Esmeralda Delay 3

            if (ImGui::Begin("📼 FRUITY DELAY 3 (ANALOG TAPE, PING-PONG & LO-FI ECHO)###Delay3Window", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🛸 Psytrance Dotted 1/8 Stereo Ping-Pong")) {
                            delay_time_steps = 3.0f; stereo_ping_pong = 1.0f; feedback_level = 0.65f;
                            filter_lowpass_cut = 8000.0f; filter_highpass_cut = 250.0f;
                        }
                        if (ImGui::MenuItem("📼 Vintage Analog Tape Echo (Wobble & Saturation)")) {
                            delay_time_steps = 4.0f; tape_wobble_flutter = 0.60f; drive_saturation = 0.55f;
                            filter_lowpass_cut = 4200.0f;
                        }
                        if (ImGui::MenuItem("🕹️ Lo-Fi 8-Bit Cyberpunk Repeater")) {
                            delay_time_steps = 2.0f; bitcrush_reduction = 0.65f; sample_rate_reduction = 0.50f;
                        }
                        if (ImGui::MenuItem("🌌 Cosmic Infinite Shimmer Echo (100% Feedback)")) {
                            feedback_level = 0.95f; stereo_ping_pong = 1.0f; tape_wobble_flutter = 0.35f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.70f, 1.0f), "FRUITY DELAY 3: ANALOG TAPE WOBBLE, BIT-CRUSH & PING-PONG BPM ECHO");
                ImGui::TextDisabled("O processador de eco mais completo do FL Studio com modelagem de fita analógica, saturação e modulação.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DAS REPETIÇÕES DO DELAY EM TEMPO REAL ──
                ImVec2 dl_p0 = ImGui::GetCursorScreenPos();
                ImVec2 dl_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 dl_p1 = ImVec2(dl_p0.x + dl_sz.x, dl_p0.y + dl_sz.y);

                ImDrawList* draw = ImGui::GetWindowDrawList();
                draw->AddRectFilled(dl_p0, dl_p1, IM_COL32(10, 15, 22, 255), 4.0f);
                draw->AddRect(dl_p0, dl_p1, IM_COL32(30, 50, 65, 255), 4.0f);

                // Desenhar as Repetições de Eco no Tempo (Taps)
                int num_taps = 8;
                float base_amp = 1.0f;
                float cy = dl_p0.y + dl_sz.y * 0.5f;

                for (int t = 0; t < num_taps; ++t) {
                    float tap_x = dl_p0.x + 30.0f + (t * (dl_sz.x - 60.0f) / (float)num_taps);
                    float tap_amp = std::pow(feedback_level, (float)t);
                    if (tap_amp < 0.02f) break;

                    float tap_h = tap_amp * (dl_sz.y * 0.42f);
                    float pan_dir = ((t % 2 == 0) ? -1.0f : 1.0f) * stereo_ping_pong;

                    // Barra do Tap L/R
                    draw->AddLine(ImVec2(tap_x, cy - tap_h * (0.5f - pan_dir * 0.5f)),
                                  ImVec2(tap_x, cy + tap_h * (0.5f + pan_dir * 0.5f)),
                                  IM_COL32(0, (int)(180 + tap_amp * 75), 220, 240), 3.0f);

                    draw->AddCircleFilled(ImVec2(tap_x, cy - tap_h * (0.5f - pan_dir * 0.5f)), 4.0f, IM_COL32(0, 255, 200, 255));
                    
                    char tap_lbl[16];
                    snprintf(tap_lbl, sizeof(tap_lbl), "Tap %d", t + 1);
                    draw->AddText(ImVec2(tap_x - 12.0f, dl_p1.y - 18.0f), IM_COL32(120, 160, 180, 200), tap_lbl);
                }

                char dl_info[128];
                snprintf(dl_info, sizeof(dl_info), "TEMPO: %.1f STEPS | FEEDBACK: %.0f%% | PING-PONG: %.0f%%", delay_time_steps, feedback_level * 100.0f, stereo_ping_pong * 100.0f);
                draw->AddText(ImVec2(dl_p0.x + 12.0f, dl_p0.y + 6.0f), IM_COL32(0, 230, 200, 240), dl_info);

                ImGui::Dummy(dl_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES PRINCIPAIS DO DELAY ───────────────
                ImGui::Columns(3, "Delay3Cols", true);

                // Coluna 1: Tempo & Feedback
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.8f, 1.0f), "⏱️ TEMPO & FEEDBACK");
                ImGui::Separator();

                ImGui::SliderFloat("Tempo (Steps)", &delay_time_steps, 0.5f, 16.0f, "%.1f Steps");
                ImGui::SliderFloat("Feedback (Ecos)", &feedback_level, 0.0f, 1.05f, "%.2fx");
                ImGui::SliderFloat("Ping-Pong Estéreo", &stereo_ping_pong, 0.0f, 1.0f, "%.2f");
                ImGui::Checkbox("Modo Reverse Echo", &reverse_echo_mode);

                ImGui::NextColumn();

                // Coluna 2: Saturação Analógica & Lo-Fi
                ImGui::TextColored(ImVec4(1.0f, 0.70f, 0.2f, 1.0f), "📼 ANALOG TAPE & LO-FI");
                ImGui::Separator();

                ImGui::SliderFloat("Tape Wobble / Flutter", &tape_wobble_flutter, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Drive de Saturação", &drive_saturation, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Bitcrush (Redução de Bits)", &bitcrush_reduction, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Sample Rate Crush", &sample_rate_reduction, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 3: Filtros de Feedback & Balanço
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🌊 FILTROS & NÍVEIS");
                ImGui::Separator();

                ImGui::SliderFloat("Lowpass Cutoff", &filter_lowpass_cut, 500.0f, 18000.0f, "%.0f Hz");
                ImGui::SliderFloat("Highpass Cutoff", &filter_highpass_cut, 20.0f, 1000.0f, "%.0f Hz");
                ImGui::SliderFloat("Dry Level", &dry_level, 0.0f, 1.5f, "%.2fx");
                ImGui::SliderFloat("Wet Level", &wet_level, 0.0f, 1.5f, "%.2fx");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
