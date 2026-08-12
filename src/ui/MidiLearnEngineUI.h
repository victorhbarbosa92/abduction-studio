#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <map>

namespace KuroUI {

    struct MidiMappingRule {
        int cc_number;
        int channel;
        std::string target_param_name;
        int track_target;
        float min_val;
        float max_val;
    };

    class MidiLearnEngineUI {
    private:
        bool is_open = false;
        bool is_learning = false;
        int active_learning_cc = -1;
        std::vector<MidiMappingRule> mappings;

    public:
        MidiLearnEngineUI() {
            // Mapeamentos padrão de fábrica para controladores populares (Novation Launchkey, Akai MPK, Arturia KeyLab)
            mappings = {
                { 1, 1, "Cutoff Filtro Lead", 5, 20.0f, 20000.0f },
                { 7, 1, "Volume Faixa 1 (Kick)", 0, -60.0f, 6.0f },
                { 10, 1, "Panorâmico Faixa 1", 0, -1.0f, 1.0f },
                { 74, 1, "Ressonância Synth", 5, 0.0f, 1.0f },
                { 91, 1, "Reverb Send A", 0, 0.0f, 1.0f }
            };
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(740, 480), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.08f, 0.96f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.60f, 0.00f, 0.4f));

            if (ImGui::Begin(ICON_FA_PLUG " MAPEAMENTO MIDI HARDWARE & MIDI LEARN", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // BARRA SUPERIOR: STATUS DO APRENDIZADO
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), ICON_FA_SLIDERS " Mapeador de Controladores Físicos MIDI:");
                ImGui::SameLine(ImGui::GetWindowWidth() - 220);
                
                if (is_learning) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));
                    if (ImGui::Button(ICON_FA_CIRCLE " Aguardando Knob/Fader...", ImVec2(200, 26))) {
                        is_learning = false;
                    }
                    ImGui::PopStyleColor();
                } else {
                    if (ImGui::Button(ICON_FA_GRADUATION_CAP " Ativar MIDI Learn", ImVec2(200, 26))) {
                        is_learning = true;
                    }
                }

                ImGui::Separator();
                ImGui::Spacing();

                // TABELA DE MAPEAMENTOS ATIVOS
                ImGui::BeginChild("##MidiMapTable", ImVec2(0, 320), true);
                {
                    ImGui::Columns(5, "MidiTable", true);
                    ImGui::SetColumnWidth(0, 90);
                    ImGui::SetColumnWidth(1, 90);
                    ImGui::SetColumnWidth(2, 220);
                    ImGui::SetColumnWidth(3, 140);

                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "MIDI CC"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "CANAL"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "PARÂMETRO ALVO"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "FAIXA ALVO"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "AÇÃO"); ImGui::NextColumn();
                    ImGui::Separator();

                    for (size_t i = 0; i < mappings.size(); ++i) {
                        const auto& map_item = mappings[i];
                        ImGui::PushID((int)i);

                        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.0f, 1.0f), "CC #%d", map_item.cc_number); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Ch %d", map_item.channel); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "%s", map_item.target_param_name.c_str()); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Faixa %d", map_item.track_target + 1); ImGui::NextColumn();

                        if (ImGui::Button(ICON_FA_TRASH " Remover", ImVec2(85, 20))) {
                            mappings.erase(mappings.begin() + i);
                            ImGui::PopID();
                            break;
                        }
                        ImGui::NextColumn();

                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Dica: Clique em 'Ativar MIDI Learn' e gire qualquer botão ou mova qualquer fader do seu controlador físico USB/MIDI para vinculá-lo ao parâmetro selecionado.");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
