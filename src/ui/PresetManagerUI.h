#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace KuroUI {

    struct SynthPreset {
        std::string name;
        std::string category; // Lead, Bass, Pad, FX, Pluck
        std::string target_synth; // KuroWave, MonkSynth, AlienVoice, AnalogMonster, AbductionFM
        float cutoff = 0.5f;
        float resonance = 0.3f;
        float attack = 0.01f;
        float release = 0.3f;
        float drive = 0.0f;
    };

    class PresetManagerUI {
    private:
        bool is_open = false;
        std::vector<SynthPreset> preset_library;
        int selected_preset_idx = 0;
        int selected_category_idx = 0;
        char search_filter[128] = "";

    public:
        PresetManagerUI() {
            // Presets padrão da biblioteca Kuro
            preset_library = {
                { "Psytrance Aggressive Saw", "Lead", "ExpressiveLead", 0.85f, 0.45f, 0.005f, 0.25f, 0.6f },
                { "Deep Sub KBB Bass", "Bass", "AnalogMonster", 0.35f, 0.20f, 0.001f, 0.15f, 0.4f },
                { "Alien Chanted Choir", "Pad", "MonkSynth", 0.60f, 0.50f, 0.100f, 1.20f, 0.2f },
                { "FM Cyber Zap 16th", "FX", "AbductionFM", 0.90f, 0.70f, 0.002f, 0.10f, 0.8f },
                { "Wavetable Hypnotic Pluck", "Pluck", "KuroWave", 0.75f, 0.30f, 0.001f, 0.40f, 0.3f },
                { "Acoustic Contrabass Warm", "Bass", "AcousticContrabass", 0.40f, 0.15f, 0.020f, 0.50f, 0.1f }
            };
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(780, 520), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.07f, 0.09f, 0.96f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.90f, 1.00f, 0.4f));

            if (ImGui::Begin(ICON_FA_SLIDERS " GERENCIADOR DE PRESETS & PATCHES NATIVOS", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // BARRA SUPERIOR DE BUSCA E FILTROS
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_MAGNIFYING_GLASS " Filtrar Presets:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(220);
                ImGui::InputText("##PresetSearch", search_filter, sizeof(search_filter));

                ImGui::SameLine();
                ImGui::SetNextItemWidth(140);
                const char* categories[] = { "Todas Categorias", "Lead", "Bass", "Pad", "FX", "Pluck" };
                ImGui::Combo("##PresetCat", &selected_category_idx, categories, IM_ARRAYSIZE(categories));

                ImGui::SameLine(ImGui::GetWindowWidth() - 210);
                if (ImGui::Button(ICON_FA_FLOPPY_DISK " Salvar Patch Atual", ImVec2(190, 24))) {
                    // Salvar Preset
                }

                ImGui::Separator();
                ImGui::Spacing();

                // LISTA DE PRESETS (TABELA MODERNA)
                ImGui::BeginChild("##PresetListGrid", ImVec2(0, 360), true);
                {
                    ImGui::Columns(4, "PresetTable", true);
                    ImGui::SetColumnWidth(0, 240);
                    ImGui::SetColumnWidth(1, 110);
                    ImGui::SetColumnWidth(2, 160);

                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "NOME DO PATCH"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "CATEGORIA"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "SINTETIZADOR"); ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "AÇÃO"); ImGui::NextColumn();
                    ImGui::Separator();

                    for (size_t i = 0; i < preset_library.size(); ++i) {
                        const auto& patch = preset_library[i];

                        // Filtro de Busca
                        if (strlen(search_filter) > 0 && patch.name.find(search_filter) == std::string::npos) {
                            continue;
                        }
                        if (selected_category_idx > 0 && patch.category != categories[selected_category_idx]) {
                            continue;
                        }

                        ImGui::PushID((int)i);
                        
                        bool is_selected = (selected_preset_idx == (int)i);
                        if (ImGui::Selectable(patch.name.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                            selected_preset_idx = (int)i;
                        }
                        ImGui::NextColumn();

                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", patch.category.c_str()); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "%s", patch.target_synth.c_str()); ImGui::NextColumn();

                        if (ImGui::Button(ICON_FA_BOLT " Carregar Patch", ImVec2(120, 20))) {
                            selected_preset_idx = (int)i;
                        }
                        ImGui::NextColumn();

                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Selecione qualquer patch para aplicar instantaneamente os parâmetros de Cutoff, Resonance, Envelope ADSR e Drive no sintetizador nativo ativo.");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
