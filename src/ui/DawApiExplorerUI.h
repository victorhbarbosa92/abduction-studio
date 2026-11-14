#pragma once
#include "imgui.h"
#include "../core/DawApiData.h"
#include "../core/DawApiBridge.h"
#include <vector>
#include <string>
#include <algorithm>
#include <set>

namespace KuroUI {

    class DawApiExplorerUI {
    public:
        static void Render(bool& open) {
            if (!open) return;

            ImGui::SetNextWindowSize(ImVec2(850, 520), ImGuiCond_FirstUseEver);
            
            // Estética sleek/glassmorphism com cores customizadas (Abduction Style)
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.12f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.18f, 0.12f, 0.28f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.24f, 0.16f, 0.38f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.30f, 0.20f, 0.48f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.12f, 0.28f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.16f, 0.40f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.20f, 0.55f, 1.00f));

            if (ImGui::Begin("DAW API Explorer & Integrator", &open, ImGuiWindowFlags_NoCollapse)) {
                
                // Variáveis estáticas de controle de estado e cache de filtragem
                static int selected_daw_idx = 0;
                static int selected_container_idx = 0;
                static int selected_func_idx = -1;
                
                static char search_query[128] = "";
                static std::string last_search = "";
                
                // Obter os dados brutos estáticos
                const auto& all_data = KuroAI::GetDawApiData();
                
                // Extrair DAWs únicas
                static std::vector<std::string> daws;
                if (daws.empty()) {
                    std::set<std::string> unique_daws;
                    for (const auto& item : all_data) {
                        unique_daws.insert(item.daw);
                    }
                    daws.assign(unique_daws.begin(), unique_daws.end());
                }
                
                if (daws.empty()) {
                    ImGui::Text("Nenhum dado de API disponível.");
                    ImGui::End();
                    ImGui::PopStyleColor(7);
                    return;
                }
                
                std::string current_daw = daws[selected_daw_idx];
                
                // Extrair módulos/classes únicos para a DAW selecionada
                std::vector<std::pair<std::string, std::string>> containers; // {name, type}
                {
                    std::set<std::string> unique_containers;
                    for (const auto& item : all_data) {
                        if (item.daw == current_daw) {
                            if (unique_containers.find(item.container) == unique_containers.end()) {
                                unique_containers.insert(item.container);
                                containers.push_back({item.container, item.container_type});
                            }
                        }
                    }
                }
                
                if (selected_container_idx >= (int)containers.size()) {
                    selected_container_idx = 0;
                    selected_func_idx = -1;
                }
                
                std::string current_container = !containers.empty() ? containers[selected_container_idx].first : "";
                std::string current_container_type = !containers.empty() ? containers[selected_container_idx].second : "";
                
                // Filtrar funções/propriedades pertencentes ao container selecionado e que batam com a busca
                std::vector<KuroAI::ApiItem> filtered_functions;
                std::string sq(search_query);
                std::transform(sq.begin(), sq.end(), sq.begin(), ::tolower);
                
                for (const auto& item : all_data) {
                    if (item.daw == current_daw && item.container == current_container) {
                        if (sq.empty()) {
                            filtered_functions.push_back(item);
                        } else {
                            std::string name_lower = item.name;
                            std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
                            std::string desc_lower = item.description;
                            std::transform(desc_lower.begin(), desc_lower.end(), desc_lower.begin(), ::tolower);
                            
                            if (name_lower.find(sq) != std::string::npos || desc_lower.find(sq) != std::string::npos) {
                                filtered_functions.push_back(item);
                            }
                        }
                    }
                }
                
                // ── Coluna Esquerda: Filtros e Seletores ──────────────────────────────
                ImGui::BeginChild("Sidebar", ImVec2(240, 0), true);
                
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "1. Escolha a DAW");
                ImGui::SetNextItemWidth(-1);
                
                std::vector<const char*> daw_names;
                for (const auto& d : daws) daw_names.push_back(d.c_str());
                
                int curr_daw = selected_daw_idx;
                if (ImGui::Combo("##DawCombo", &curr_daw, daw_names.data(), (int)daw_names.size())) {
                    if (curr_daw != selected_daw_idx) {
                        selected_daw_idx = curr_daw;
                        selected_container_idx = 0;
                        selected_func_idx = -1;
                    }
                }
                
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "2. Módulos / Classes");
                ImGui::SetNextItemWidth(-1);
                
                std::vector<const char*> container_names;
                for (const auto& c : containers) container_names.push_back(c.first.c_str());
                
                int curr_container = selected_container_idx;
                if (ImGui::Combo("##ContainerCombo", &curr_container, container_names.data(), (int)container_names.size())) {
                    if (curr_container != selected_container_idx) {
                        selected_container_idx = curr_container;
                        selected_func_idx = -1;
                    }
                }
                
                if (!containers.empty() && selected_container_idx < (int)containers.size()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                    ImGui::TextWrapped("Tipo: %s", current_container_type.c_str());
                    ImGui::TextWrapped("Exposição de API da DAW para integração remota no Abduction.");
                    ImGui::PopStyleColor();
                }
                
                ImGui::EndChild();
                
                ImGui::SameLine();
                
                // ── Coluna Direita: Lista de Funções e Detalhes ───────────────────────
                ImGui::BeginGroup();
                
                // Busca em tempo real
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Buscar Parâmetros / Funções");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##SearchAPI", search_query, sizeof(search_query));
                
                ImGui::Separator();
                
                // Layout dividido para Lista e Detalhes
                float split_height = ImGui::GetContentRegionAvail().y * 0.50f;
                
                // Painel da Lista de Funções
                ImGui::BeginChild("FunctionsList", ImVec2(0, split_height), true);
                if (ImGui::BeginTable("FuncTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY)) {
                    ImGui::TableSetupColumn("Tipo", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("Nome", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Retorno", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableSetupColumn("Categoria", ImGuiTableColumnFlags_WidthFixed, 140.0f);
                    ImGui::TableHeadersRow();
                    
                    for (int i = 0; i < (int)filtered_functions.size(); i++) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        
                        ImVec4 type_col = ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
                        if (filtered_functions[i].item_type == "Property") type_col = ImVec4(0.8f, 0.6f, 1.0f, 1.0f);
                        else if (filtered_functions[i].item_type == "Method") type_col = ImVec4(0.4f, 1.0f, 0.6f, 1.0f);
                        
                        ImGui::TextColored(type_col, "%s", filtered_functions[i].item_type.c_str());
                        
                        ImGui::TableSetColumnIndex(1);
                        bool is_selected = (selected_func_idx == i);
                        if (ImGui::Selectable(filtered_functions[i].name.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                            selected_func_idx = i;
                        }
                        
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%s", filtered_functions[i].return_type.c_str());
                        
                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("%s", filtered_functions[i].category.c_str());
                    }
                    ImGui::EndTable();
                }
                ImGui::EndChild();
                
                // Painel de Detalhes da Função Selecionada
                ImGui::BeginChild("FunctionDetails", ImVec2(0, 0), true);
                if (selected_func_idx >= 0 && selected_func_idx < (int)filtered_functions.size()) {
                    const auto& fn = filtered_functions[selected_func_idx];
                    
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Assinatura e Associações");
                    
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.5f, 1.0f), "%s", fn.signature.empty() ? fn.name.c_str() : fn.signature.c_str());
                    
                    ImGui::Text("Retorno: "); ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "%s", fn.return_type.c_str());
                    
                    ImGui::Text("Categoria: %s", fn.category.c_str());
                    ImGui::Separator();
                    
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Descrição Técnica");
                    ImGui::TextWrapped("%s", fn.description.c_str());
                    
                    // Geração do snippet
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Template de Código Autogerado (Python)");
                    
                    std::string snippet = "";
                    if (current_daw == "Ableton Live") {
                        if (fn.item_type == "Property") {
                            snippet = "# Ler propriedade no Live Set\nvalue = self.song()." + fn.name + "\n";
                            snippet += "# Escrever propriedade no Live Set\nself.song()." + fn.name + " = value";
                        } else {
                            snippet = "# Executar método no Live Set\nself.song()." + fn.name + "(args)";
                        }
                    } else if (current_daw == "FL Studio") {
                        snippet = "import " + current_container + "\n";
                        snippet += "# Executar chamada de integração no FL Studio\n" + fn.signature;
                    }
                    
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.04f, 0.05f, 0.07f, 1.0f));
                    ImGui::InputTextMultiline("##SnippetText", const_cast<char*>(snippet.c_str()), snippet.size(), ImVec2(0, 70), ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopStyleColor();
                    
                    static float copy_timer = 0.0f;
                    if (ImGui::Button("Copiar Código para Área de Transferência")) {
                        ImGui::SetClipboardText(snippet.c_str());
                        copy_timer = 2.0f; // Exibe feedback por 2s
                    }
                    
                    if (copy_timer > 0.0f) {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Código copiado!");
                        copy_timer -= ImGui::GetIO().DeltaTime;
                    }

                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Integração Remota (UDP Bridge - Porta 9000)");

                    static char udp_command[256] = "";
                    static std::string last_fn_name = "";
                    if (last_fn_name != fn.name) {
                        last_fn_name = fn.name;
                        std::string default_cmd = "";
                        if (current_daw == "Ableton Live") {
                            if (fn.name == "tempo") default_cmd = "Song.tempo 128.0";
                            else if (fn.name == "start_playing") default_cmd = "Song.start_playing";
                            else if (fn.name == "stop_playing") default_cmd = "Song.stop_playing";
                            else if (fn.name == "mute") default_cmd = "Track.setMute 1 1";
                            else if (fn.name == "solo") default_cmd = "Track.setSolo 1 1";
                        } else if (current_daw == "FL Studio") {
                            if (current_container == "mixer") {
                                if (fn.name == "setTrackVolume") default_cmd = "mixer.setTrackVolume 1 0.8";
                                else if (fn.name == "setTrackPan") default_cmd = "mixer.setTrackPan 1 0.0";
                                else if (fn.name == "muteTrack") default_cmd = "mixer.muteTrack 1 1";
                                else if (fn.name == "soloTrack") default_cmd = "mixer.soloTrack 1 1";
                            } else if (current_container == "transport") {
                                if (fn.name == "start") default_cmd = "transport.start";
                                else if (fn.name == "stop") default_cmd = "transport.stop";
                            }
                        }
                        snprintf(udp_command, sizeof(udp_command), "%s", default_cmd.c_str());
                    }

                    ImGui::InputText("Comando UDP", udp_command, sizeof(udp_command));

                    static std::string udp_status = "";
                    static float udp_status_timer = 0.0f;
                    static ImVec4 udp_status_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

                    if (ImGui::Button("Testar Comando (Enviar UDP)")) {
                        std::string cmd_str(udp_command);
                        if (!cmd_str.empty()) {
                            DawApiBridge::sendLocalUdpCommand(cmd_str);
                            udp_status = "Comando '" + cmd_str + "' enviado via UDP!";
                            udp_status_color = ImVec4(0.22f, 1.0f, 0.08f, 1.0f); // Abduction green
                        } else {
                            udp_status = "Comando vazio.";
                            udp_status_color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                        }
                        udp_status_timer = 3.0f;
                    }

                    if (udp_status_timer > 0.0f) {
                        ImGui::SameLine();
                        ImGui::TextColored(udp_status_color, "%s", udp_status.c_str());
                        udp_status_timer -= ImGui::GetIO().DeltaTime;
                    }
                } else {
                    ImGui::Text("Selecione um método ou propriedade acima para ver a documentação técnica e templates de código.");
                }
                ImGui::EndChild();
                
                ImGui::EndGroup();
            }
            ImGui::End();
            
            ImGui::PopStyleColor(7);
        }
    };
}
