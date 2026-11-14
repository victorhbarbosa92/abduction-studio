#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroFruitySqueezeUI {
    private:
        bool is_open = false;

        // Parâmetros do lendário Fruity Squeeze (Bitcrusher, Sample Rate Reducer & Puncher)
        int bit_depth = 8;               // 2 a 16 bits de quantização
        float sample_rate_div = 6.0f;    // 1x (48kHz) a 64x (Redução extrema de sample rate)
        float punch_amount = 0.65f;      // Punch harmônico e impacto de transiente sujo
        float post_filter_cut = 8500.0f; // Filtro suave de altas frequências
        float dry_wet_mix = 1.0f;        // Balanço Dry / Wet

    public:
        KuroFruitySqueezeUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(880, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.20f, 0.80f, 0.85f)); // Rosa Choque / Cyberpunk Bitcrusher

            if (ImGui::Begin("🕹️ FRUITY SQUEEZE (BITCRUSHER, SAMPLE REDUCER & PUNCHER)###FruitySqueezeWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🕹️ Game Boy 4-Bit Lo-Fi Chiptune")) {
                            bit_depth = 4; sample_rate_div = 12.0f; punch_amount = 0.8f; post_filter_cut = 6000.0f;
                        }
                        if (ImGui::MenuItem("👾 Vintage E-mu SP-1200 12-Bit Hip Hop Grit")) {
                            bit_depth = 12; sample_rate_div = 2.0f; punch_amount = 0.4f; post_filter_cut = 12000.0f;
                        }
                        if (ImGui::MenuItem("⚡ Industrial Cyberpunk 2-Bit Metal Crushed")) {
                            bit_depth = 2; sample_rate_div = 24.0f; punch_amount = 1.0f;
                        }
                        if (ImGui::MenuItem("📻 AM Radio Telephone Lo-Fi (8-Bit / 8kHz)")) {
                            bit_depth = 8; sample_rate_div = 6.0f; post_filter_cut = 3500.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.85f, 1.0f), "FRUITY SQUEEZE: DIGITAL RESOLUTION REDUCTION & HARMONIC PUNCHER");
                ImGui::TextDisabled("O lendário destruidor sonoro de redução de bits e taxa de amostragem clássico do FL Studio para estética Lo-Fi e 8-bit.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA FORMA DE ONDA DEGRAU (QUANTIZED STAIRCASE)
                ImVec2 sq_p0 = ImGui::GetCursorScreenPos();
                ImVec2 sq_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 sq_p1 = ImVec2(sq_p0.x + sq_sz.x, sq_p0.y + sq_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(sq_p0, sq_p1, IM_COL32(12, 10, 20, 255), 4.0f);
                dl->AddRect(sq_p0, sq_p1, IM_COL32(50, 25, 60, 255), 4.0f);

                // Desenhar Onda Degrau Quantizada em Bits
                float levels = std::pow(2.0f, (float)bit_depth);
                int steps = (int)sq_sz.x;
                float cy = sq_p0.y + sq_sz.y * 0.5f;

                for (int x = 0; x < steps - 1; ++x) {
                    float t0 = (float)x / (float)steps;
                    float raw_sine = std::sin(t0 * 6.2831853f * 2.0f);

                    // Redução de Sample Rate (Hold)
                    int hold_steps = (int)sample_rate_div;
                    if (hold_steps < 1) hold_steps = 1;
                    int snapped_x = (x / hold_steps) * hold_steps;
                    float snapped_t = (float)snapped_x / (float)steps;
                    float held_sine = std::sin(snapped_t * 6.2831853f * 2.0f);

                    // Quantização em Degraus de Bits
                    float quantized_sine = std::round(held_sine * (levels * 0.5f)) / (levels * 0.5f);

                    float py0 = cy - quantized_sine * (sq_sz.y * 0.40f);
                    float py1 = cy - std::round(std::sin((float)((x + 1) / hold_steps * hold_steps) / (float)steps * 6.2831853f * 2.0f) * (levels * 0.5f)) / (levels * 0.5f) * (sq_sz.y * 0.40f);

                    dl->AddLine(ImVec2(sq_p0.x + x, py0), ImVec2(sq_p0.x + x + 1, py1), IM_COL32(255, 60, 200, 240), 2.0f);
                }

                char sq_info[128];
                snprintf(sq_info, sizeof(sq_info), "RESOLUÇÃO: %d BITS (%0.f NÍVEIS) | SAMPLE RATE DIVISOR: %.0fx | PUNCH: %.0f%%", bit_depth, levels, sample_rate_div, punch_amount * 100.0f);
                dl->AddText(ImVec2(sq_p0.x + 12.0f, sq_p0.y + 6.0f), IM_COL32(255, 120, 220, 240), sq_info);

                ImGui::Dummy(sq_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DO BITCRUSHER ─────────────────────
                ImGui::Columns(3, "SqueezeCols", true);

                // Coluna 1: Bits & Níveis
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.8f, 1.0f), "🕹️ PROFUNDIDADE DE BITS");
                ImGui::Separator();

                ImGui::SliderInt("Bits (Resolution)", &bit_depth, 2, 16);
                ImGui::SliderFloat("Sample Rate Divisor", &sample_rate_div, 1.0f, 64.0f, "%.0fx");

                ImGui::NextColumn();

                // Coluna 2: Punch & Filtro
                ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "🥊 PUNCH & FILTRO");
                ImGui::Separator();

                ImGui::SliderFloat("Punch Harmonic Boost", &punch_amount, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Post Filter Cutoff", &post_filter_cut, 1000.0f, 18000.0f, "%.0f Hz");

                ImGui::NextColumn();

                // Coluna 3: Nível de Mix
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎚️ MIX & BLEND");
                ImGui::Separator();

                ImGui::SliderFloat("Dry / Wet Mix", &dry_wet_mix, 0.0f, 1.0f, "%.2f");
                ImGui::ProgressBar(1.0f - (float)bit_depth / 16.0f, ImVec2(-1, 20), "Bitcrush Crush Intensity");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
