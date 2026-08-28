#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroGranularDelayUI {
    private:
        bool is_open = false;

        // Parâmetros do FL Studio Granular Delay (Buffer Grain Cloud Echo)
        float grain_size_ms = 85.0f;     // Tamanho do grão (10ms a 400ms)
        float grain_pitch_shift = 12.0f; // Afinação Shimmer (+12 semitons = Oitava Shimmer Reverb)
        float delay_feedback = 0.65f;    // Realimentação
        float grain_spray_jitter = 0.45f;// Dispersão temporal estocástica dos grãos
        float stereo_pan_jitter = 0.80f; // Espalhamento estéreo dos grãos
        float filter_lowpass = 7500.0f;  // Hz
        float dry_wet_mix = 0.50f;
        bool freeze_cloud_mode = false;  // Congelamento de nuvem em loop infinito

    public:
        KuroGranularDelayUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(940, 600), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.95f, 0.85f, 0.85f)); // Verde Ciano Granular

            if (ImGui::Begin("🌌 FRUITY GRANULAR DELAY & SHIMMER CLOUD (PITCH-SHIFTED ECHO)###GranularDelayWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("✨ Celestial +12st Shimmer Echo (Brian Eno Style)")) {
                            grain_size_ms = 120.0f; grain_pitch_shift = 12.0f; delay_feedback = 0.75f;
                            grain_spray_jitter = 0.35f; stereo_pan_jitter = 0.90f; dry_wet_mix = 0.55f;
                        }
                        if (ImGui::MenuItem("🛸 Dark Alien -12st Sub-Octave Drone")) {
                            grain_size_ms = 160.0f; grain_pitch_shift = -12.0f; delay_feedback = 0.70f;
                            filter_lowpass = 3200.0f;
                        }
                        if (ImGui::MenuItem("⚡ Glitch Stutter Micro-Grains (25ms Spray)")) {
                            grain_size_ms = 25.0f; grain_pitch_shift = 0.0f; delay_feedback = 0.50f;
                            grain_spray_jitter = 0.85f;
                        }
                        if (ImGui::MenuItem("❄️ Infinite Ambient Grain Freeze (Hold)")) {
                            freeze_cloud_mode = true; grain_size_ms = 200.0f; delay_feedback = 1.0f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.85f, 1.0f), "FRUITY GRANULAR DELAY: REALTIME PITCH-SHIFTED GRAIN CLOUD & SHIMMER");
                ImGui::TextDisabled("Combina síntese granular estocástica com ecos de pitch-shift para criar caudas de Shimmer celestiais e micro-glitches.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. DISPLAY VISUAL DA NUVEM DE GRÃOS SHIMMER ─────────────
                ImVec2 gd_p0 = ImGui::GetCursorScreenPos();
                ImVec2 gd_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
                ImVec2 gd_p1 = ImVec2(gd_p0.x + gd_sz.x, gd_p0.y + gd_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(gd_p0, gd_p1, IM_COL32(10, 15, 22, 255), 4.0f);
                dl->AddRect(gd_p0, gd_p1, IM_COL32(30, 55, 65, 255), 4.0f);

                // Desenhar Grade de Pitch (+12st, 0st, -12st)
                float cy = gd_p0.y + gd_sz.y * 0.5f;
                dl->AddLine(ImVec2(gd_p0.x, cy), ImVec2(gd_p1.x, cy), IM_COL32(50, 70, 85, 160), 1.0f);
                dl->AddText(ImVec2(gd_p0.x + 8.0f, cy - 14.0f), IM_COL32(120, 160, 180, 180), "0 st (Normal)");

                float p_up = cy - (gd_sz.y * 0.35f);
                float p_dn = cy + (gd_sz.y * 0.35f);
                dl->AddLine(ImVec2(gd_p0.x, p_up), ImVec2(gd_p1.x, p_up), IM_COL32(40, 60, 75, 120), 1.0f);
                dl->AddLine(ImVec2(gd_p0.x, p_dn), ImVec2(gd_p1.x, p_dn), IM_COL32(40, 60, 75, 120), 1.0f);
                dl->AddText(ImVec2(gd_p0.x + 8.0f, p_up - 14.0f), IM_COL32(0, 240, 200, 180), "+12 st (Shimmer)");
                dl->AddText(ImVec2(gd_p0.x + 8.0f, p_dn + 2.0f), IM_COL32(200, 120, 255, 180), "-12 st (Sub Drone)");

                // Desenhar Nuvem de Grãos Deslocados no Tempo e Afinação
                int grain_count = 32;
                for (int g = 0; g < grain_count; ++g) {
                    float t_offset = ((float)g / (float)grain_count);
                    float gx = gd_p0.x + 80.0f + t_offset * (gd_sz.x - 100.0f);
                    
                    float pitch_norm = grain_pitch_shift / 24.0f; // -24 a +24 semitons
                    float gy = cy - pitch_norm * (gd_sz.y * 0.70f) + (((std::rand() % 100) / 100.0f - 0.5f) * grain_spray_jitter * 40.0f);

                    gy = std::clamp(gy, gd_p0.y + 8.0f, gd_p1.y - 8.0f);
                    float g_size = 3.0f + (grain_size_ms / 400.0f) * 5.0f;

                    // Ponto de Grão Luminoso
                    dl->AddCircleFilled(ImVec2(gx, gy), g_size, IM_COL32(0, 240, 220, 200));
                    dl->AddCircle(ImVec2(gx, gy), g_size + 2.0f, IM_COL32(0, 255, 255, 90));
                }

                char gd_info[128];
                snprintf(gd_info, sizeof(gd_info), "PITCH: %+.0f ST | GRÃO: %.0f ms | SPRAY: %.0f%% | FEEDBACK: %.0f%%", grain_pitch_shift, grain_size_ms, grain_spray_jitter * 100.0f, delay_feedback * 100.0f);
                dl->AddText(ImVec2(gd_p0.x + 12.0f, gd_p1.y - 20.0f), IM_COL32(0, 230, 255, 240), gd_info);

                ImGui::Dummy(gd_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. CONTROLES DO GRANULAR DELAY ──────────────────────────
                ImGui::Columns(3, "GranularDelayCols", true);

                // Coluna 1: Grão & Pitch
                ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.85f, 1.0f), "🔬 GRÃO & SHIMMER PITCH");
                ImGui::Separator();

                ImGui::SliderFloat("Tamanho do Grão", &grain_size_ms, 10.0f, 400.0f, "%.0f ms");
                ImGui::SliderFloat("Pitch-Shift Shimmer", &grain_pitch_shift, -24.0f, 24.0f, "%+.0f Semitons");
                ImGui::SliderFloat("Spray Jitter (Dispersão)", &grain_spray_jitter, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                // Coluna 2: Feedback & Filtro
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "🔁 FEEDBACK & AMORTECIMENTO");
                ImGui::Separator();

                ImGui::SliderFloat("Feedback Realimentação", &delay_feedback, 0.0f, 1.05f, "%.2fx");
                ImGui::SliderFloat("Filtro Passa-Baixas", &filter_lowpass, 500.0f, 18000.0f, "%.0f Hz");
                ImGui::Checkbox("❄️ Congelar Nuvem (Freeze Hold)", &freeze_cloud_mode);

                ImGui::NextColumn();

                // Coluna 3: Estéreo & Mix
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "🎧 PAN JITTER & DRY/WET");
                ImGui::Separator();

                ImGui::SliderFloat("Stereo Pan Jitter", &stereo_pan_jitter, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Dry / Wet Mix", &dry_wet_mix, 0.0f, 1.0f, "%.2f");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
