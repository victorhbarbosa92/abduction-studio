#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct SoundfontBankItem {
        int bank_num;
        int preset_num;
        std::string name;
        std::string category;
    };

    class KuroSoundfontPlayerUI {
    private:
        bool is_open = false;
        std::string loaded_sf2_path = "C:/Audio/Soundfonts/Orchestral_Psy_Master.sf2";
        std::vector<SoundfontBankItem> preset_bank;
        int selected_preset_idx = 0;
        int active_polyphony = 32;
        float velocity_sensitivity = 0.8f;
        float sf2_reverb_send = 0.25f;
        float sf2_chorus_send = 0.15f;

    public:
        KuroSoundfontPlayerUI() {
            preset_bank = {
                { 0, 0, "Grand Piano Stereo 3D", "Teclados" },
                { 0, 19, "Church Organ Pipe", "Órgãos" },
                { 0, 24, "Nylon Acoustic Guitar", "Cordas / Violão" },
                { 0, 32, "Acoustic Contrabass Pluck", "Baixos Acústicos" },
                { 0, 40, "Violin Ensemble Section", "Orquestra" },
                { 0, 48, "Full String Orchestra", "Orquestra" },
                { 0, 56, "Brass Section & Horns", "Sopro / Metais" },
                { 0, 73, "Flute Solo Expressive", "Sopro / Madeira" },
                { 0, 88, "Psytrance Choir Pad", "Sintetizadores / Pads" }
            };
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(840, 540), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.08f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.80f, 0.00f, 0.45f));

            if (ImGui::Begin(ICON_FA_MUSIC " KURO SOUNDFONT (.SF2/.SFZ) MULTI-SAMPLER PLAYER", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // BARRA SUPERIOR DE CARREGAMENTO DE SOUNDFONT
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), ICON_FA_FOLDER_OPEN " Arquivo SoundFont Carregado:");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", loaded_sf2_path.c_str());

                ImGui::SameLine(ImGui::GetWindowWidth() - 210);
                if (ImGui::Button(ICON_FA_FOLDER_OPEN " Carregar .SF2 / .SFZ", ImVec2(190, 24))) {
                    // Carregar novo SF2
                }

                ImGui::Separator();
                ImGui::Spacing();

                // BANCO DE INSTRUMENTOS E PRESETS DO SOUNDFONT
                ImGui::BeginChild("##Sf2BankList", ImVec2(0, 240), true);
                {
                    ImGui::Columns(4, "Sf2BankTable", true);
                    ImGui::SetColumnWidth(0, 70);
                    ImGui::SetColumnWidth(1, 80);
                    ImGui::SetColumnWidth(2, 280);

                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "BANK"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "PRESET"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "NOME DO INSTRUMENTO"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "CATEGORIA"); ImGui::NextColumn();
                    ImGui::Separator();

                    for (size_t i = 0; i < preset_bank.size(); ++i) {
                        auto& item = preset_bank[i];
                        ImGui::PushID((int)i);

                        bool is_sel = (selected_preset_idx == (int)i);
                        if (ImGui::Selectable(std::to_string(item.bank_num).c_str(), is_sel, ImGuiSelectableFlags_SpanAllColumns)) {
                            selected_preset_idx = (int)i;
                        }
                        ImGui::NextColumn();

                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "#%d", item.preset_num); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "%s", item.name.c_str()); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", item.category.c_str()); ImGui::NextColumn();

                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::Spacing();

                // CONTROLES DE POLIFONIA, VELOCIDADE E EFEITOS
                ImGui::BeginChild("##Sf2EnginePanel", ImVec2(0, 150), true);
                {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_SLIDERS " PARÂMETROS DO REPRODUTOR SAMPLER:");
                    ImGui::Spacing();

                    ImGui::Columns(2, "Sf2Controls", true);

                    // Coluna 1: Polifonia & Resposta de Dinâmica
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Vozes & Resposta de Velocidade");
                    ImGui::SliderInt("Vozes de Polifonia", &active_polyphony, 8, 128);
                    ImGui::SliderFloat("Sensibilidade de Dinâmica", &velocity_sensitivity, 0.0f, 1.0f, "%.2f");
                    ImGui::NextColumn();

                    // Coluna 2: Envios Reverb/Chorus Master
                    ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "Envios de Efeitos Master");
                    ImGui::SliderFloat("Envio Reverb", &sf2_reverb_send, 0.0f, 1.0f, "%.2f");
                    ImGui::SliderFloat("Envio Chorus", &sf2_chorus_send, 0.0f, 1.0f, "%.2f");
                    ImGui::NextColumn();
                }
                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Dica: Arquivos .SF2 e .SFZ permitem carregar instrumentos orquestrais e pianos multi-amostrados de altíssima fidelidade com consumo zero de CPU.");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
