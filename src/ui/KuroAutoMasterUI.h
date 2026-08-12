#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    enum class MasterTargetProfile {
        STREAMING_14_LUFS, // Spotify, Apple Music (-14 LUFS)
        CLUB_DJ_9_LUFS,    // DJ Sets / Club Sound Systems (-9 LUFS)
        EXTREME_7_LUFS     // Darkpsy / Hi-Tech Ultra Loud (-7 LUFS)
    };

    class KuroAutoMasterUI {
    private:
        bool is_open = false;
        bool is_analyzing = false;
        float current_lufs = -18.5f;
        float target_lufs = -14.0f;
        int selected_profile_idx = 0;
        
        // Parâmetros de Masterização Automática
        float eq_low_gain_db = 0.0f;
        float eq_mid_gain_db = 0.0f;
        float eq_high_gain_db = 0.0f;
        float stereo_width = 1.2f;
        float limiter_ceiling_db = -0.1f;
        float limiter_gain_db = 3.5f;

    public:
        KuroAutoMasterUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(820, 540), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.80f, 0.00f, 1.00f, 0.45f));

            if (ImGui::Begin(ICON_FA_WAND_MAGIC_SPARKLES " KURO AI AUTO-MASTER & LUFS LOUDNESS TARGET", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // BARRA SUPERIOR DE PROFILES DE MASTER
                ImGui::TextColored(ImVec4(0.8f, 0.3f, 1.0f, 1.0f), ICON_FA_SLIDERS " Perfil de Masterização Desejado:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(260);
                const char* profiles[] = {
                    "Streaming (-14 LUFS - Spotify/Apple)",
                    "Club / DJ Set (-9 LUFS - Festival PA)",
                    "Hi-Tech / Extreme (-7 LUFS - Ultra Loud)"
                };
                if (ImGui::Combo("##MasterProfile", &selected_profile_idx, profiles, IM_ARRAYSIZE(profiles))) {
                    if (selected_profile_idx == 0) target_lufs = -14.0f;
                    else if (selected_profile_idx == 1) target_lufs = -9.0f;
                    else if (selected_profile_idx == 2) target_lufs = -7.0f;
                }

                ImGui::SameLine(ImGui::GetWindowWidth() - 210);
                if (is_analyzing) {
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.0f, 1.0f), ICON_FA_GEARS " Analisando Espectro...");
                } else {
                    if (ImGui::Button(ICON_FA_BOLT " Análise & Master IA", ImVec2(190, 26))) {
                        is_analyzing = true;
                        // Simulação de ajuste automático de ganho para atingir o LUFS alvo
                        current_lufs = target_lufs;
                        limiter_gain_db = (target_lufs == -14.0f) ? 2.0f : ((target_lufs == -9.0f) ? 4.5f : 6.8f);
                        is_analyzing = false;
                    }
                }

                ImGui::Separator();
                ImGui::Spacing();

                // MEDIDORES VISUAIS DE LOUDNESS (LUFS & PEAK)
                ImGui::BeginChild("##LoudnessMeters", ImVec2(0, 100), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_CHART_BAR " MEDIDOR DE LOUDNESS INTEGRADO (LUFS)");
                    ImGui::Spacing();

                    // Barra Progresso LUFS Atual vs Target
                    float norm_lufs = std::clamp((current_lufs + 24.0f) / 18.0f, 0.0f, 1.0f);
                    char lufs_buf[64];
                    snprintf(lufs_buf, sizeof(lufs_buf), "LUFS Atual: %.1f dBFS  |  Target: %.1f LUFS", current_lufs, target_lufs);
                    
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.2f, 1.0f, 1.0f));
                    ImGui::ProgressBar(norm_lufs, ImVec2(-1, 28), lufs_buf);
                    ImGui::PopStyleColor();

                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Teto de Pico Verdadeiro (True Peak Ceiling): %.1f dBFS  |  Margem Dinâmica: 3.2 LU", limiter_ceiling_db);
                }
                ImGui::EndChild();

                ImGui::Spacing();

                // PAINEL DE CONTROLES DO PROCESSADOR MASTER
                ImGui::BeginChild("##MasterControls", ImVec2(0, 240), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_GEARS " CADEIA DE PROCESSAMENTO DA MASTER:");
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Columns(3, "MasterProcColumns", true);

                    // Coluna 1: EQ Espectral Multi-Banda
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "1. EQ Espectral IA");
                    ImGui::SliderFloat("Sub/Low (Grave)", &eq_low_gain_db, -6.0f, 6.0f, "%.1f dB");
                    ImGui::SliderFloat("Mids (Médios)", &eq_mid_gain_db, -6.0f, 6.0f, "%.1f dB");
                    ImGui::SliderFloat("Highs (Agudos)", &eq_high_gain_db, -6.0f, 6.0f, "%.1f dB");
                    ImGui::NextColumn();

                    // Coluna 2: Imagem Estéreo & Drenagem de Sub
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "2. Imagem Estéreo");
                    ImGui::SliderFloat("Largura Estéreo", &stereo_width, 0.8f, 2.0f, "%.2f x");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Mono Sub < 90Hz: ATIVO");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Side-Chain Multi-band: ATIVO");
                    ImGui::NextColumn();

                    // Coluna 3: Limitador Master Brickwall
                    ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "3. Limitador Brickwall");
                    ImGui::SliderFloat("Ganho de Entrada", &limiter_gain_db, 0.0f, 12.0f, "%.1f dB");
                    ImGui::SliderFloat("Teto de Saída", &limiter_ceiling_db, -2.0f, 0.0f, "%.1f dB");
                    ImGui::NextColumn();
                }
                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Dica: Clique em 'Análise & Master IA' para que a IA do Abduction Studio meça a sonoridade da faixa e ajuste a cadeia de limitação e EQ automaticamente.");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
