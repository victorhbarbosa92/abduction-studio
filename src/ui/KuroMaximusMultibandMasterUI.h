#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroMaximusMultibandMasterUI {
    private:
        bool is_open = false;

        // Band Frequencies (Low/Mid split, Mid/High split)
        float low_mid_split = 200.0f;   // Hz (Low/Mid)
        float mid_high_split = 4000.0f; // Hz (Mid/High)

        // Low Band (Bass / Sub)
        float low_pre_gain = 1.0f;
        float low_post_gain = 1.0f;
        float low_threshold = -12.0f;
        float low_ratio = 3.5f;
        float low_attack = 10.0f; // ms
        float low_release = 150.0f; // ms
        bool low_solo = false;
        bool low_mute = false;

        // Mid Band (Vocals / Leads / Snare Body)
        float mid_pre_gain = 1.0f;
        float mid_post_gain = 1.0f;
        float mid_threshold = -14.0f;
        float mid_ratio = 2.5f;
        float mid_attack = 15.0f;
        float mid_release = 180.0f;
        bool mid_solo = false;
        bool mid_mute = false;

        // High Band (Air / Cymbals / Hi-Hats)
        float high_pre_gain = 1.0f;
        float high_post_gain = 1.0f;
        float high_threshold = -16.0f;
        float high_ratio = 2.0f;
        float high_attack = 5.0f;
        float high_release = 120.0f;
        bool high_solo = false;
        bool high_mute = false;

        // Master Limiter & Saturation (Warmth)
        float master_ceiling = -0.1f; // dBFS True Peak
        float master_saturation = 0.35f;
        float stereo_enhancer_width = 1.25f; // 125% Stereo Spread

    public:
        KuroMaximusMultibandMasterUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(960, 640), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.40f, 0.00f, 0.85f)); // Laranja / Ouro Maximus

            if (ImGui::Begin("🎚️ MAXIMUS MULTIBAND MAXIMIZER & MASTERING LIMITER (FL STUDIO STYLE)###MaximusWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU DE PRESETS ─────────────────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Presets")) {
                        if (ImGui::MenuItem("🔥 Transparent Master Peak Limiter (-9 LUFS)")) {
                            master_saturation = 0.20f; stereo_enhancer_width = 1.10f;
                        }
                        if (ImGui::MenuItem("👽 Psytrance Punch & Tight Sub Bass")) {
                            low_threshold = -15.0f; low_ratio = 4.0f; stereo_enhancer_width = 1.30f;
                        }
                        if (ImGui::MenuItem("🚀 Clean Broadcast Loudness (-14 LUFS)")) {
                            master_saturation = 0.10f; stereo_enhancer_width = 1.05f;
                        }
                        if (ImGui::MenuItem("🌌 Wide & Warm Tube Tape Saturation")) {
                            master_saturation = 0.65f; stereo_enhancer_width = 1.45f;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Cabeçalho
                ImGui::TextColored(ImVec4(1.0f, 0.50f, 0.0f, 1.0f), "MAXIMUS 3-BAND DYNAMICS COMPRESSOR, STEREO EXPANDER & CEILING LIMITER");
                ImGui::TextDisabled("Compressor multibanda e limitador de pico máximo de referência da indústria musical para finalização sonora.");
                ImGui::Separator();
                ImGui::Spacing();

                // ── 1. GRÁFICO ESPECTRAL DE 3 BANDAS (LOW | MID | HIGH) ─────
                ImVec2 band_p0 = ImGui::GetCursorScreenPos();
                ImVec2 band_sz = ImVec2(ImGui::GetContentRegionAvail().x, 130.0f);
                ImVec2 band_p1 = ImVec2(band_p0.x + band_sz.x, band_p0.y + band_sz.y);

                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(band_p0, band_p1, IM_COL32(12, 16, 22, 255), 4.0f);
                dl->AddRect(band_p0, band_p1, IM_COL32(40, 50, 65, 255), 4.0f);

                // Divisores de frequência (Low/Mid e Mid/High)
                float split1_x = band_p0.x + (std::log10(low_mid_split / 20.0f) / 3.0f) * band_sz.x;
                float split2_x = band_p0.x + (std::log10(mid_high_split / 20.0f) / 3.0f) * band_sz.x;

                // Fundo das Bandas Coloridas (Vermelho Sub / Verde Médios / Azul Agudos)
                dl->AddRectFilled(band_p0, ImVec2(split1_x, band_p1.y), IM_COL32(230, 40, 40, 40));
                dl->AddRectFilled(ImVec2(split1_x, band_p0.y), ImVec2(split2_x, band_p1.y), IM_COL32(40, 200, 80, 40));
                dl->AddRectFilled(ImVec2(split2_x, band_p0.y), band_p1, IM_COL32(0, 150, 255, 40));

                // Linhas Divisórias Verticais
                dl->AddLine(ImVec2(split1_x, band_p0.y), ImVec2(split1_x, band_p1.y), IM_COL32(255, 120, 0, 220), 2.0f);
                dl->AddLine(ImVec2(split2_x, band_p0.y), ImVec2(split2_x, band_p1.y), IM_COL32(255, 120, 0, 220), 2.0f);

                // Labels de cada banda
                char lbl_low[32], lbl_mid[32], lbl_high[32];
                snprintf(lbl_low, sizeof(lbl_low), "LOW (Sub: 20-%.0fHz)", low_mid_split);
                snprintf(lbl_mid, sizeof(lbl_mid), "MID (Body: %.0f-%.0fkHz)", low_mid_split, mid_high_split / 1000.0f);
                snprintf(lbl_high, sizeof(lbl_high), "HIGH (Air: %.0fkHz-20kHz)", mid_high_split / 1000.0f);

                dl->AddText(ImVec2(band_p0.x + 8.0f, band_p0.y + 6.0f), IM_COL32(255, 100, 100, 240), lbl_low);
                dl->AddText(ImVec2(split1_x + 8.0f, band_p0.y + 6.0f), IM_COL32(100, 255, 120, 240), lbl_mid);
                dl->AddText(ImVec2(split2_x + 8.0f, band_p0.y + 6.0f), IM_COL32(80, 200, 255, 240), lbl_high);

                ImGui::Dummy(band_sz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 2. SEÇÃO DE CONTROLES DAS 3 BANDAS INDIVIDUAIS ───────────
                ImGui::Columns(3, "MaximusBands", true);

                // --- BANDA LOW ---
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "🔴 LOW BAND (SUB BASS)");
                ImGui::Separator();
                ImGui::SliderFloat("Low Threshold", &low_threshold, -30.0f, 0.0f, "%.1f dB");
                ImGui::SliderFloat("Low Ratio", &low_ratio, 1.0f, 10.0f, "%.1f:1");
                ImGui::SliderFloat("Low Attack", &low_attack, 0.1f, 50.0f, "%.1f ms");
                ImGui::SliderFloat("Low Release", &low_release, 10.0f, 500.0f, "%.0f ms");
                ImGui::SliderFloat("Split Low/Mid", &low_mid_split, 60.0f, 500.0f, "%.0f Hz");

                ImGui::NextColumn();

                // --- BANDA MID ---
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.5f, 1.0f), "🟢 MID BAND (PRESENCE)");
                ImGui::Separator();
                ImGui::SliderFloat("Mid Threshold", &mid_threshold, -30.0f, 0.0f, "%.1f dB");
                ImGui::SliderFloat("Mid Ratio", &mid_ratio, 1.0f, 10.0f, "%.1f:1");
                ImGui::SliderFloat("Mid Attack", &mid_attack, 0.1f, 50.0f, "%.1f ms");
                ImGui::SliderFloat("Mid Release", &mid_release, 10.0f, 500.0f, "%.0f ms");
                ImGui::SliderFloat("Split Mid/High", &mid_high_split, 1000.0f, 10000.0f, "%.0f Hz");

                ImGui::NextColumn();

                // --- BANDA HIGH ---
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "🔵 HIGH BAND (AIR / SPARKLE)");
                ImGui::Separator();
                ImGui::SliderFloat("High Threshold", &high_threshold, -30.0f, 0.0f, "%.1f dB");
                ImGui::SliderFloat("High Ratio", &high_ratio, 1.0f, 10.0f, "%.1f:1");
                ImGui::SliderFloat("High Attack", &high_attack, 0.1f, 50.0f, "%.1f ms");
                ImGui::SliderFloat("High Release", &high_release, 10.0f, 500.0f, "%.0f ms");

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── 3. MASTER LIMITER, SATURATION & STEREO ENHANCER ─────────
                ImGui::Columns(2, "MasterLimiterCols", false);

                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.0f, 1.0f), "🛡️ MASTER CEILING LIMITER & SATURATION");
                ImGui::SliderFloat("True Peak Ceiling", &master_ceiling, -2.0f, 0.0f, "%.1f dBFS");
                ImGui::SliderFloat("Tape / Tube Saturation", &master_saturation, 0.0f, 1.0f, "%.2f");

                ImGui::NextColumn();

                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "🎧 STEREO ENHANCER & AIR WIDENER");
                ImGui::SliderFloat("Stereo Width Spread", &stereo_enhancer_width, 0.5f, 2.0f, "%.2fx");
                ImGui::ProgressBar(0.72f, ImVec2(-1, 20), "Master Output: -9.2 LUFS (Ready for Spotify / Club)");

                ImGui::Columns(1);
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
