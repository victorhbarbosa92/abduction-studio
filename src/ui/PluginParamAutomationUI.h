#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct PluginParamInfo {
        int id;
        std::string name;
        float current_val;
        float min_val;
        float max_val;
        std::string unit;
        bool is_automated;
        int bound_automation_lane_id;
    };

    class PluginParamAutomationUI {
    private:
        bool is_open = false;
        std::string active_plugin_name = "Serum VST3 / Delay Lama / MonkSynth";
        std::vector<PluginParamInfo> parameters;
        int selected_param_idx = 0;
        float lfo_speed_hz = 2.0f;
        int selected_lfo_shape = 0; // 0: Senoidal, 1: Triangular, 2: Quadrada, 3: Random S&H

    public:
        PluginParamAutomationUI() {
            // Parâmetros demonstrativos descobertos do plugin ativo
            parameters = {
                { 0, "Cutoff Frequency", 0.65f, 20.0f, 20000.0f, "Hz", true, 1 },
                { 1, "Resonance (Q)", 0.40f, 0.0f, 1.0f, "%", false, -1 },
                { 2, "Drive / Saturation", 0.25f, 0.0f, 24.0f, "dB", false, -1 },
                { 3, "Reverb Mix", 0.30f, 0.0f, 100.0f, "%", true, 2 },
                { 4, "Delay Time L/R", 0.50f, 0.0f, 1000.0f, "ms", false, -1 },
                { 5, "Pitch Shift Detune", 0.00f, -12.0f, 12.0f, "semitones", false, -1 }
            };
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(840, 520), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 0.50f, 0.45f));

            if (ImGui::Begin(ICON_FA_GEARS " PONTE DE AUTOMAÇÃO DE PARÂMETROS VST3 / CLAP", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // BARRA SUPERIOR DE PLUGIN ATIVO
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_PLUG " Plugin Selecionado:");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", active_plugin_name.c_str());

                ImGui::SameLine(ImGui::GetWindowWidth() - 230);
                if (ImGui::Button(ICON_FA_ROTATE " Escanear Parâmetros", ImVec2(210, 24))) {
                    // Escanear parâmetros
                }

                ImGui::Separator();
                ImGui::Spacing();

                // TABELA DE PARÂMETROS DESCOBERTOS
                ImGui::BeginChild("##ParamAutomationGrid", ImVec2(0, 280), true);
                {
                    ImGui::Columns(5, "ParamTable", true);
                    ImGui::SetColumnWidth(0, 60);
                    ImGui::SetColumnWidth(1, 220);
                    ImGui::SetColumnWidth(2, 180);
                    ImGui::SetColumnWidth(3, 160);

                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "ID"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "NOME DO PARÂMETRO"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "VALOR ATUAL"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "STATUS AUTOMAÇÃO"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "AÇÕES"); ImGui::NextColumn();
                    ImGui::Separator();

                    for (size_t i = 0; i < parameters.size(); ++i) {
                        auto& param = parameters[i];
                        ImGui::PushID((int)i);

                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "#%d", param.id); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", param.name.c_str()); ImGui::NextColumn();

                        ImGui::SetNextItemWidth(120);
                        ImGui::SliderFloat("##Val", &param.current_val, 0.0f, 1.0f, "%.2f"); ImGui::NextColumn();

                        if (param.is_automated) {
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_CHART_LINE " Pista #%d Vinculada", param.bound_automation_lane_id);
                        } else {
                            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Não automatizado");
                        }
                        ImGui::NextColumn();

                        if (param.is_automated) {
                            if (ImGui::Button(ICON_FA_XMARK " Desvincular", ImVec2(100, 20))) {
                                param.is_automated = false;
                            }
                        } else {
                            if (ImGui::Button(ICON_FA_PLUS " Criar Pista", ImVec2(100, 20))) {
                                param.is_automated = true;
                                param.bound_automation_lane_id = (int)i + 1;
                            }
                        }
                        ImGui::NextColumn();

                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::Spacing();

                // DOCK DE MODULAÇÃO LFO INTERNA PARA O PARÂMETRO SELECIONADO
                ImGui::BeginChild("##LfoBridgePanel", ImVec2(0, 100), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_WAVE_SQUARE " GERADOR DE LFO INTERNO PARA PARÂMETROS:");
                    ImGui::Spacing();

                    ImGui::SetNextItemWidth(150);
                    const char* lfo_shapes[] = { "Senoidal (Sine)", "Triangular (Triangle)", "Quadrada (Square)", "Random (S&H)" };
                    ImGui::Combo("##LfoShape", &selected_lfo_shape, lfo_shapes, IM_ARRAYSIZE(lfo_shapes));

                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(160);
                    ImGui::SliderFloat("Velocidade (Hz)", &lfo_speed_hz, 0.1f, 20.0f, "%.1f Hz");

                    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
                    if (ImGui::Button(ICON_FA_BOLT " Linkar LFO ao Cutoff", ImVec2(180, 24))) {
                        // Vínculo LFO
                    }
                }
                ImGui::EndChild();
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
