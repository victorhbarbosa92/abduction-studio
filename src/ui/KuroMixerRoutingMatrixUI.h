#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    class KuroMixerRoutingMatrixUI {
    private:
        bool is_open = false;
        // Matriz de Roteamento 9x9 (0 = Master, 1..8 = Tracks 1 a 8)
        // routing_matrix[origem][destino] = ganho de envio (0.0f a 1.0f)
        float send_matrix[9][9] = { 0 };
        bool send_enable[9][9] = { false };
        bool is_sidechain_only[9][9] = { false };

    public:
        KuroMixerRoutingMatrixUI() {
            // Inicializar roteamento padrão: Todas as faixas 1..8 enviam para a Master (0) com ganho 1.0f
            for (int t = 1; t <= 8; ++t) {
                send_enable[t][0] = true;
                send_matrix[t][0] = 1.0f;
            }
            // Roteamento padrão de Sidechain (Kick faixa 1 -> Bass faixa 4)
            send_enable[1][4] = true;
            send_matrix[1][4] = 0.85f;
            is_sidechain_only[1][4] = true;

            // Roteamento de Send FX (Lead faixa 6 -> Reverb/FX faixa 7)
            send_enable[6][7] = true;
            send_matrix[6][7] = 0.35f;
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(860, 580), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.11f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 0.50f, 0.85f));

            if (ImGui::Begin("🎛️ FL STUDIO ADVANCED MIXER ROUTING & SIDECHAIN MATRIX###MixerRoutingMatrixWindow", &is_open)) {
                
                // Cabeçalho da Matriz
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "MATRIX DE ROTEAMENTO DE ÁUDIO, BUSES & SIDECHAIN MULTICANAL");
                ImGui::TextDisabled("Controle avançado de envio de áudio entre todas as faixas do mixer no estilo FL Studio / Ableton Live.");
                ImGui::Separator();
                ImGui::Spacing();

                const char* ch_names[] = { "Master", "1: Kick", "2: Snare", "3: HiHat", "4: Bass", "5: Chords", "6: Lead", "7: FX Bus", "8: Extra" };
                
                // Tabela da Matriz de Envio
                if (ImGui::BeginTable("RoutingGrid", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX)) {
                    
                    // Cabeçalho das Colunas (Destino)
                    ImGui::TableSetupColumn("De \\ Para", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    for (int dest = 0; dest < 9; ++dest) {
                        ImGui::TableSetupColumn(ch_names[dest], ImGuiTableColumnFlags_WidthFixed, 75.0f);
                    }
                    ImGui::TableHeadersRow();

                    // Linhas (Origem de Envio)
                    for (int src = 0; src < 9; ++src) {
                        ImGui::TableNextRow();
                        
                        // Nome da Faixa de Origem
                        ImGui::TableSetColumnIndex(0);
                        if (src == 0) ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", ch_names[src]);
                        else ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "%s", ch_names[src]);

                        // Células de Cruzamento
                        for (int dest = 0; dest < 9; ++dest) {
                            ImGui::TableSetColumnIndex(dest + 1);
                            
                            if (src == dest) {
                                // Auto-envio desativado (Feedback loop prevention)
                                ImGui::TextDisabled(" -- ");
                            } else if (src == 0 && dest > 0) {
                                // Master para pistas desativado
                                ImGui::TextDisabled(" -- ");
                            } else {
                                ImGui::PushID(src * 100 + dest);
                                
                                bool enabled = send_enable[src][dest];
                                bool is_sc = is_sidechain_only[src][dest];

                                if (enabled) {
                                    if (is_sc) {
                                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.0f, 0.85f));
                                    } else {
                                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.4f, 0.85f));
                                    }
                                } else {
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.18f, 0.22f, 0.6f));
                                }

                                char btn_label[32];
                                if (enabled) {
                                    snprintf(btn_label, sizeof(btn_label), "%s %.0f%%", is_sc ? "SC" : "ON", send_matrix[src][dest] * 100.0f);
                                } else {
                                    snprintf(btn_label, sizeof(btn_label), "OFF");
                                }

                                if (ImGui::Button(btn_label, ImVec2(68, 22))) {
                                    send_enable[src][dest] = !send_enable[src][dest];
                                    if (send_enable[src][dest] && send_matrix[src][dest] == 0.0f) {
                                        send_matrix[src][dest] = 1.0f;
                                    }
                                }

                                // Menu de contexto de clique direito para alterar Sidechain / Nível
                                if (ImGui::BeginPopupContextItem()) {
                                    ImGui::Text("Roteamento: %s -> %s", ch_names[src], ch_names[dest]);
                                    ImGui::Separator();
                                    ImGui::Checkbox("Apenas Sidechain (Sem envio de Áudio Direto)", &is_sidechain_only[src][dest]);
                                    ImGui::SliderFloat("Ganho de Envio (Send Gain)", &send_matrix[src][dest], 0.0f, 1.0f, "%.2f");
                                    ImGui::EndPopup();
                                }

                                ImGui::PopStyleColor();
                                ImGui::PopID();
                            }
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Botões de Presets de Roteamento Rápido
                ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "⚡ PRESETS DE ROTEAMENTO RÁPIDO DO MIXER:");
                
                if (ImGui::Button("Resetar Roteamento Padrão (Todas as faixas -> Master)", ImVec2(380, 26))) {
                    for (int s = 0; s < 9; ++s) {
                        for (int d = 0; d < 9; ++d) {
                            send_enable[s][d] = (s > 0 && d == 0);
                            send_matrix[s][d] = (s > 0 && d == 0) ? 1.0f : 0.0f;
                            is_sidechain_only[s][d] = false;
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Ativar Grupo de Bateria (Drums Bus na Faixa 7)", ImVec2(360, 26))) {
                    // Kick, Snare, HiHat enviam para Faixa 7 (FX Bus)
                    send_enable[1][7] = true; send_matrix[1][7] = 1.0f;
                    send_enable[2][7] = true; send_matrix[2][7] = 1.0f;
                    send_enable[3][7] = true; send_matrix[3][7] = 1.0f;
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
