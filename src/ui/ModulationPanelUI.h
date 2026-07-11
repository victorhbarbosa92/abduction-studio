#pragma once
#include "imgui.h"
#include "../audio/LFOModulator.h"
#include <string>

namespace KuroUI {

    inline void RenderModulationPanel(bool* open) {
        if (!*open) return;

        ImGui::SetNextWindowSize(ImVec2(800, 280), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Ableton LFO Modulators Matrix", open)) {
            ImGui::End();
            return;
        }

        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Matrix LFO Modulation Routing");
        ImGui::Separator();

        if (ImGui::BeginTabBar("LFOTabs")) {
            for (int l = 0; l < 4; l++) {
                char tab_name[32];
                snprintf(tab_name, sizeof(tab_name), "LFO %d", l + 1);
                
                if (ImGui::BeginTabItem(tab_name)) {
                    auto& mod = g_lfo_modulators[l];
                    
                    ImGui::Columns(2, nullptr, false);
                    ImGui::SetColumnWidth(0, 350);

                    // Left Column: LFO Controls
                    ImGui::Checkbox("Ativar LFO", &mod.active);
                    
                    const char* shapes[] = { "Senoide (Sine)", "Triângulo (Triangle)", "Dente de Serra (Sawtooth)", "Quadrada (Square)" };
                    ImGui::Combo("Forma de Onda", &mod.shape, shapes, IM_ARRAYSIZE(shapes));
                    
                    ImGui::Checkbox("Sincronizar com BPM (Sync)", &mod.sync);
                    
                    if (mod.sync) {
                        const char* sync_rates[] = { "1/1 Beat", "1/2 Beat", "1/4 Beat (Default)", "1/8 Beat" };
                        ImGui::Combo("Taxa de Sync", &mod.sync_rate_idx, sync_rates, IM_ARRAYSIZE(sync_rates));
                    } else {
                        ImGui::SliderFloat("Frequência (Hz)", &mod.rate, 0.1f, 20.0f, "%.2f Hz");
                    }
                    
                    ImGui::SliderFloat("Profundidade (Depth)", &mod.depth, 0.0f, 1.0f, "%.2f");

                    ImGui::NextColumn();

                    // Right Column: Target Routing
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Destino da Modulação (Routing)");
                    ImGui::Separator();

                    const char* tracks[] = { "Nenhum", "Track 0 (KICK/BASS)", "Track 1 (LEADS)", "Track 2 (VOX)", "Track 3 (FX)", "Track 4 (DRUMS)", "Track 5 (SYNTH)", "Track 6 (PADS)", "Track 7 (EXTRA)" };
                    int current_tr = mod.target_track + 1; // map -1..7 to 0..8
                    if (ImGui::Combo("Canal de Efeitos", &current_tr, tracks, IM_ARRAYSIZE(tracks))) {
                        mod.target_track = current_tr - 1;
                    }

                    if (mod.target_track >= 0) {
                        const char* params[] = { "Gothic Overdrive Drive", "RingMod Alien Freq", "Tremolo depth", "Tremolo rate" };
                        ImGui::Combo("Parâmetro Alvo", &mod.target_param, params, IM_ARRAYSIZE(params));
                        
                        // Set base value range depending on parameter
                        if (mod.target_param == 0) {
                            ImGui::SliderFloat("Valor Base (Drive)", &mod.base_value, 1.0f, 10.0f, "%.1f");
                        } else if (mod.target_param == 1) {
                            ImGui::SliderFloat("Valor Base (Freq)", &mod.base_value, 20.0f, 1000.0f, "%.1f Hz");
                        } else if (mod.target_param == 2) {
                            ImGui::SliderFloat("Valor Base (Depth)", &mod.base_value, 0.0f, 1.0f, "%.2f");
                        } else if (mod.target_param == 3) {
                            ImGui::SliderFloat("Valor Base (Rate)", &mod.base_value, 0.1f, 5.0f, "%.2f Hz");
                        }
                    }

                    ImGui::Columns(1);
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}
