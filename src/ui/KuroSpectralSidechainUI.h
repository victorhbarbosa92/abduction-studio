#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroSpectralSidechainUI {
    private:
        bool is_open = false;
        int trigger_track_idx = 0; // 0: Kick Track (Ch 1), 1: Snare (Ch 2), 2: Vocal (Ch 5)
        int target_track_idx = 3;  // 3: Bassline (Ch 4), 4: Pad (Ch 5)
        float low_ducking_db = -12.0f;
        float mid_ducking_db = -3.0f;
        float high_ducking_db = 0.0f;
        float attack_ms = 2.0f;
        float release_ms = 45.0f;
        float crossover_low_hz = 150.0f;
        float crossover_high_hz = 2500.0f;

    public:
        KuroSpectralSidechainUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(860, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.45f));

            if (ImGui::Begin(ICON_FA_SHUFFLE " KURO DYNAMIC SPECTRAL SIDECHAIN & DUCKING ENGINE", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // BARRA SUPERIOR DE TRILHAS TRIGGER E TARGET
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_SLIDERS " Trilha Trigger (Gatilho):");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(180);
                const char* tracks[] = { "Faixa 1: Kick", "Faixa 2: Snare", "Faixa 3: Percussion", "Faixa 4: Bassline", "Faixa 5: Lead", "Faixa 6: Vocal" };
                ImGui::Combo("##TriggerTrack", &trigger_track_idx, tracks, IM_ARRAYSIZE(tracks));

                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.8f, 0.3f, 1.0f, 1.0f), "-> Trilha Alvo (Ducked):");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(180);
                ImGui::Combo("##TargetTrack", &target_track_idx, tracks, IM_ARRAYSIZE(tracks));

                ImGui::SameLine(ImGui::GetWindowWidth() - 200);
                if (ImGui::Button(ICON_FA_BOLT " Preset Kick/Bass Psy", ImVec2(180, 24))) {
                    low_ducking_db = -18.0f;
                    attack_ms = 1.0f;
                    release_ms = 35.0f;
                }

                ImGui::Separator();
                ImGui::Spacing();

                // VISUALIZADOR DE CORTE ESPECTRAL EM TEMPO REAL
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImVec2(ImGui::GetContentRegionAvail().x, 200.0f);

                // Fundo Escuro do Analisador
                draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y), IM_COL32(10, 14, 22, 255), 4.0f);
                draw_list->AddRect(canvas_p0, ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y), IM_COL32(0, 229, 255, 120), 4.0f);

                // Linha de Crossover Baixo e Alto
                float mid_y = canvas_p0.y + canvas_sz.y * 0.5f;
                float x_cross1 = canvas_p0.x + (crossover_low_hz / 10000.0f) * canvas_sz.x;
                float x_cross2 = canvas_p0.x + (crossover_high_hz / 10000.0f) * canvas_sz.x;

                draw_list->AddLine(ImVec2(x_cross1, canvas_p0.y), ImVec2(x_cross1, canvas_p0.y + canvas_sz.y), IM_COL32(255, 255, 0, 180), 1.5f);
                draw_list->AddLine(ImVec2(x_cross2, canvas_p0.y), ImVec2(x_cross2, canvas_p0.y + canvas_sz.y), IM_COL32(255, 255, 0, 180), 1.5f);

                // Curva de Atenuação Espectral em Magenta
                for (int x = 0; x < (int)canvas_sz.x; x += 3) {
                    float norm_x = (float)x / canvas_sz.x;
                    float duck_val = 0.0f;
                    
                    if (norm_x * 10000.0f < crossover_low_hz) {
                        duck_val = low_ducking_db;
                    } else if (norm_x * 10000.0f < crossover_high_hz) {
                        duck_val = mid_ducking_db;
                    } else {
                        duck_val = high_ducking_db;
                    }

                    float py = mid_y - (duck_val / 24.0f) * (canvas_sz.y * 0.4f);
                    draw_list->AddLine(ImVec2(canvas_p0.x + x, mid_y), ImVec2(canvas_p0.x + x, py), IM_COL32(255, 0, 180, 180));
                }

                draw_list->AddText(ImVec2(canvas_p0.x + 15.0f, canvas_p0.y + 15.0f), IM_COL32(0, 255, 200, 255), "SUB/LOW BAND (50Hz - 150Hz): DUCKING ATIVO");
                draw_list->AddText(ImVec2(x_cross1 + 15.0f, canvas_p0.y + 15.0f), IM_COL32(255, 255, 0, 255), "MID BAND");
                draw_list->AddText(ImVec2(x_cross2 + 15.0f, canvas_p0.y + 15.0f), IM_COL32(255, 255, 255, 255), "HIGH BAND");

                ImGui::Dummy(canvas_sz);

                ImGui::Spacing();

                // CONTROLES DE ATENUAÇÃO E ENVELOPE DE DUCKING
                ImGui::BeginChild("##DuckingControlsPanel", ImVec2(0, 180), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_GEARS " NÍVEIS DE ATENUAÇÃO ESPECTRAL E TEMPOS DE REAÇÃO:");
                    ImGui::Spacing();

                    ImGui::Columns(2, "DuckingCols", true);

                    // Coluna 1: Ganho por Banda
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Atenuação de Ganho (dB)");
                    ImGui::SliderFloat("Grave (Low Ducking)", &low_ducking_db, -24.0f, 0.0f, "%.1f dB");
                    ImGui::SliderFloat("Médio (Mid Ducking)", &mid_ducking_db, -24.0f, 0.0f, "%.1f dB");
                    ImGui::SliderFloat("Agudo (High Ducking)", &high_ducking_db, -24.0f, 0.0f, "%.1f dB");
                    ImGui::NextColumn();

                    // Coluna 2: Envelope & Crossovers
                    ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "Envelope & Crossover Freq");
                    ImGui::SliderFloat("Tempo de Ataque (Attack)", &attack_ms, 0.1f, 50.0f, "%.1f ms");
                    ImGui::SliderFloat("Tempo de Liberação (Release)", &release_ms, 5.0f, 300.0f, "%.1f ms");
                    ImGui::SliderFloat("Crossover Sub/Grave", &crossover_low_hz, 40.0f, 300.0f, "%.0f Hz");
                    ImGui::NextColumn();
                }
                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Dica: O Sidechain Espectral limpa exatamente as frequências do grave (Bassline) nos milissegundos exatos em que o Kick bate, garantindo um grave limpo e potente sem embolar.");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
