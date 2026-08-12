#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include "../plugin_manager/PluginScanner.h"
#include "../plugin_manager/ClapWrapper.h"
#include "../plugin_manager/DAG.h"
#include "FileDialog.h"

extern ::KuroDSP::AudioGraph master_graph;
extern std::string track_names[];

namespace KuroUI {
    extern bool show_monksynth_vst3;
    extern bool show_delay_lama;

    class PluginManagerUI {
    private:
        KuroDSP::PluginScannerManager scanner;
        char search_filter[128] =;
        int selected_format_filter = 0; // 0: Todos, 1: CLAP, 2: VST3, 3: DLL
        bool is_open = false;
        std::string last_loaded_status;
        float toast_timer = 0.0f;

    public:
        PluginManagerUI() = default;

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }

        KuroDSP::PluginScannerManager& getScanner() { return scanner; }

        void Render(int selected_track_idx) {
            if (!is_open) return;

            if (toast_timer > 0.0f) {
                toast_timer -= ImGui::GetIO().DeltaTime;
                if (toast_timer < 0.0f) toast_timer = 0.0f;
            }

            ImGui::SetNextWindowSize(ImVec2(850, 550), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.07f, 0.09f, 0.96f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 1.00f, 0.40f, 0.4f));

            if (ImGui::Begin(ICON_FA_PLUG " ABDUCTION STUDIO V2 - GERENCIADOR DE PLUGINS (VST3 / CLAP)", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // Header & Ação de Scan
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.60f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.85f, 0.35f, 1.0f));
                if (ImGui::Button(scanner.isScanning() ? ICON_FA_HOURGLASS_HALF " ESCANEANDO..." : " ICON_FA_MAGNIFYING_GLASS " " ESCANEAR PLUGINS AGORA", ImVec2(240, 36))) {
                    scanner.scanAsync();
                }
                ImGui::PopStyleColor(2);

                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Faixa Ativa para Carregamento: [%s]", track_names[selected_track_idx].c_str());
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Status: %s", scanner.getStatusMsg().c_str());
                ImGui::EndGroup();

                if (toast_timer > 0.0f) {
                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.35f, 0.15f, 0.8f));
                    ImGui::BeginChild("ToastBanner", ImVec2(0, 28), true);
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "%s", last_loaded_status.c_str());
                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                }

                if (scanner.isScanning()) {
                    ImGui::Spacing();
                    ImGui::ProgressBar(scanner.getProgress(), ImVec2(-1, 8));
                }

                ImGui::Separator();
                ImGui::Spacing();

                // Aba de Filtros e Busca
                ImGui::Text("Filtros:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(200);
                ImGui::InputText("##SearchFilter", search_filter, sizeof(search_filter));
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Filtrar por nome, vendor ou formato");

                ImGui::SameLine();
                ImGui::SetNextItemWidth(150);
                const char* format_options[] = { "Todos Formatos", "Somente CLAP", "Somente VST3", "Somente DLL" };
                ImGui::Combo("##FormatCombo", &selected_format_filter, format_options, IM_ARRAYSIZE(format_options));

                ImGui::Spacing();

                // Painel dobrável de Diretórios de Busca
                if (ImGui::TreeNode(ICON_FA_FOLDER_OPEN " Configurar Pastas de Busca do Windows")) {
                    auto dirs = scanner.getDirectories();
                    for (size_t i = 0; i < dirs.size(); ++i) {
                        ImGui::BulletText("%s", dirs[i].c_str());
                        ImGui::SameLine(ImGui::GetWindowWidth() - 100);
                        std::string remove_id = "Remover##" + std::to_string(i);
                        if (ImGui::Button(remove_id.c_str())) {
                            scanner.removeDirectory(i);
                            break;
                        }
                    }
                    if (ImGui::Button("+ Adicionar Pasta Customizada")) {
                        std::string new_dir = KuroUI::FileDialog::OpenFile("Diretórios\0*.*\0");
                        if (!new_dir.empty()) {
                            scanner.addDirectory(std::filesystem::path(new_dir).parent_path().string());
                        }
                    }
                    ImGui::TreePop();
                }

                ImGui::Spacing();

                // Tabela de Plugins
                auto plugins = scanner.getPlugins();
                if (plugins.empty()) {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[ Nenhum plugin escaneado ainda. Clique em 'ESCANEAR PLUGINS AGORA' acima. ]");
                } else {
                    if (ImGui::BeginTable("PluginsTable", 5, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY, ImVec2(0, 300))) {
                        
                        ImGui::TableSetupColumn("Nome do Plugin", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Formato", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                        ImGui::TableSetupColumn("Fornecedor", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                        ImGui::TableSetupColumn("Caminho", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Ação", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                        ImGui::TableHeadersRow();

                        for (const auto& plug : plugins) {
                            // Aplicar filtro de texto
                            if (strlen(search_filter) > 0) {
                                std::string search_str(search_filter);
                                for (auto& c : search_str) c = (char)tolower(c);
                                std::string plug_name = plug.name;
                                for (auto& c : plug_name) c = (char)tolower(c);

                                if (plug_name.find(search_str) == std::string::npos) continue;
                            }

                            // Aplicar filtro de formato
                            if (selected_format_filter == 1 && plug.format_type != "CLAP") continue;
                            if (selected_format_filter == 2 && plug.format_type != "VST3") continue;
                            if (selected_format_filter == 3 && plug.format_type.find("DLL") == std::string::npos) continue;

                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);
                            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", plug.name.c_str());

                            ImGui::TableSetColumnIndex(1);
                            if (plug.format_type == "CLAP") {
                                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "[ CLAP ]");
                            } else if (plug.format_type == "VST3") {
                                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "[ VST3 ]");
                            } else if (plug.format_type == "BUILT-IN") {
                                ImGui::TextColored(ImVec4(0.8f, 0.3f, 1.0f, 1.0f), "[ BUILT-IN ]");
                            } else {
                                ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "[ DLL ]");
                            }

                            ImGui::TableSetColumnIndex(2);
                            ImGui::TextDisabled("%s", plug.vendor.c_str());

                            ImGui::TableSetColumnIndex(3);
                            ImGui::TextDisabled("%s", plug.filepath.c_str());

                            ImGui::TableSetColumnIndex(4);
                            std::string load_btn_id = "+ Carregar##" + plug.name + "_" + std::to_string(selected_track_idx);
                            if (ImGui::Button(load_btn_id.c_str(), ImVec2(130, 22))) {
                                std::string node_id = "Track" + std::to_string(selected_track_idx);
                                auto rack_node = std::dynamic_pointer_cast<KuroDSP::RackNode>(master_graph.getNode(node_id));
                                
                                bool success = false;
                                if (plug.format_type == "BUILT-IN") {
                                    if (plug.name.find("Compressor") != std::string::npos) {
                                        track_pedalboards[selected_track_idx].preset_dark = true;
                                    } else if (plug.name.find("Reverb") != std::string::npos || plug.name.find("Delay") != std::string::npos) {
                                        track_pedalboards[selected_track_idx].preset_cavern = true;
                                        track_pedalboards[selected_track_idx].preset_space = true;
                                    } else if (plug.name.find("Pitch") != std::string::npos) {
                                        track_pedalboards[selected_track_idx].enable_abyss_pitch = true;
                                    }
                                    success = true;
                                } else if (rack_node) {
                                    auto wrapper = std::make_shared<KuroDSP::ClapWrapper>("vst_" + plug.name, plug.name);
                                    if (wrapper->load(plug.filepath)) {
                                        rack_node->addPlugin(wrapper);
                                        success = true;
                                    }
                                }

                                if (plug.name.find("MonkSynth") != std::string::npos) {
                                    show_monksynth_vst3 = true;
                                }
                                if (plug.name.find("Delay Lama") != std::string::npos) {
                                    show_delay_lama = true;
                                }

                                last_loaded_status = " " ICON_FA_CHECK "  " + plug.name + " carregado na Faixa [" + track_names[selected_track_idx] + "]!";
                                toast_timer = 4.0f;
                            }
                        }
                        ImGui::EndTable();
                    }
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
