#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroUI {

    struct FmOperator {
        int id;
        std::string name;
        float ratio = 1.0f;
        float level = 0.8f;
        float feedback = 0.0f;
        float attack = 0.005f;
        float decay = 0.2f;
        float sustain = 0.5f;
        float release = 0.3f;
    };

    class KuroFmSynthUI {
    private:
        bool is_open = false;
        int selected_algorithm = 0; // 0: A->B->C->D, 1: (A+B)->C->D, 2: A->B + C->D, 3: A+B+C+D Parallel
        std::vector<FmOperator> operators;
        int active_operator = 0;

    public:
        KuroFmSynthUI() {
            operators = {
                { 1, "Operador A (Carrier 1)", 1.0f, 1.0f, 0.0f, 0.001f, 0.3f, 0.8f, 0.2f },
                { 2, "Operador B (Modulator 1)", 2.0f, 0.7f, 0.4f, 0.002f, 0.15f, 0.4f, 0.1f },
                { 3, "Operador C (Modulator 2)", 3.5f, 0.5f, 0.0f, 0.001f, 0.10f, 0.2f, 0.1f },
                { 4, "Operador D (Sub / Zap FX)", 0.5f, 0.6f, 0.2f, 0.005f, 0.4f, 0.5f, 0.3f }
            };
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(860, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.09f, 0.97f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.40f, 0.00f, 0.45f));

            if (ImGui::Begin(ICON_FA_WAVE_SQUARE " KURO ABDUCTION 4-OPERATOR FM SYNTHESIZER", &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // BARRA SUPERIOR DE ALGORITMOS FM
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), ICON_FA_SLIDERS " Algoritmo de Roteamento FM:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(280);
                const char* algo_names[] = {
                    "Algoritmo 1: Cascata Linear (A->B->C->D)",
                    "Algoritmo 2: Modulador Duplo ((A+B)->C)",
                    "Algoritmo 3: Pares Paralelos (A->B + C->D)",
                    "Algoritmo 4: Aditivo Puro (A+B+C+D)"
                };
                ImGui::Combo("##FmAlgo", &selected_algorithm, algo_names, IM_ARRAYSIZE(algo_names));

                ImGui::SameLine(ImGui::GetWindowWidth() - 200);
                if (ImGui::Button(ICON_FA_BOLT " Patch Psy Zap 16th", ImVec2(180, 24))) {
                    // Carregar preset FM Zap
                }

                ImGui::Separator();
                ImGui::Spacing();

                // VISUALIZADOR DE ROTEAMENTO FM & OSCILOSCÓPIO
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImVec2(ImGui::GetContentRegionAvail().x, 140.0f);

                draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y), IM_COL32(12, 16, 24, 255), 4.0f);
                draw_list->AddRect(canvas_p0, ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y), IM_COL32(255, 140, 0, 120), 4.0f);

                // Desenhar Blocos dos 4 Operadores
                float op_box_w = 120.0f;
                float op_box_h = 50.0f;
                float start_x = canvas_p0.x + 40.0f;
                float mid_y = canvas_p0.y + canvas_sz.y * 0.5f - 25.0f;

                for (int op = 0; op < 4; ++op) {
                    float bx = start_x + op * 180.0f;
                    ImU32 box_col = (active_operator == op) ? IM_COL32(255, 140, 0, 255) : IM_COL32(40, 50, 65, 255);
                    
                    draw_list->AddRectFilled(ImVec2(bx, mid_y), ImVec2(bx + op_box_w, mid_y + op_box_h), box_col, 4.0f);
                    draw_list->AddRect(ImVec2(bx, mid_y), ImVec2(bx + op_box_w, mid_y + op_box_h), IM_COL32(255, 255, 255, 180), 4.0f);

                    char op_title[32];
                    snprintf(op_title, sizeof(op_title), "OP %C", 'A' + op);
                    draw_list->AddText(ImVec2(bx + 15.0f, mid_y + 10.0f), IM_COL32(255, 255, 255, 255), op_title);

                    char op_sub[32];
                    snprintf(op_sub, sizeof(op_sub), "R: %.1fx", operators[op].ratio);
                    draw_list->AddText(ImVec2(bx + 15.0f, mid_y + 28.0f), IM_COL32(200, 200, 200, 220), op_sub);

                    if (op < 3) {
                        draw_list->AddLine(ImVec2(bx + op_box_w, mid_y + 25.0f), ImVec2(bx + 180.0f, mid_y + 25.0f), IM_COL32(255, 140, 0, 200), 2.0f);
                    }
                }

                ImGui::Dummy(canvas_sz);

                ImGui::Spacing();

                // CONTROLES ADSR E RATIO DO OPERADOR SELECIONADO
                ImGui::BeginChild("##OpEditorPanel", ImVec2(0, 220), true);
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), ICON_FA_GEARS " EDITAR OPERADOR SELECIONADO:");
                    ImGui::SameLine();
                    
                    for (int o = 0; o < 4; ++o) {
                        char btn_op[16];
                        snprintf(btn_op, sizeof(btn_op), "Operador %C", 'A' + o);
                        if (o > 0) ImGui::SameLine();
                        if (ImGui::RadioButton(btn_op, active_operator == o)) {
                            active_operator = o;
                        }
                    }

                    ImGui::Separator();
                    ImGui::Spacing();

                    auto& curr_op = operators[active_operator];

                    ImGui::Columns(2, "OpControls", true);

                    // Coluna 1: Ratio, Level, Feedback
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Frequência & Modulação");
                    ImGui::SliderFloat("Ratio (Multiplicador)", &curr_op.ratio, 0.25f, 16.0f, "%.2f x");
                    ImGui::SliderFloat("Nível / Nível de Modulação", &curr_op.level, 0.0f, 1.0f, "%.2f");
                    ImGui::SliderFloat("Feedback (Auto-Modulação)", &curr_op.feedback, 0.0f, 1.0f, "%.2f");
                    ImGui::NextColumn();

                    // Coluna 2: Envelope ADSR
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Envelope de Amplitude & FM (ADSR)");
                    ImGui::SliderFloat("Ataque (Attack)", &curr_op.attack, 0.001f, 1.0f, "%.3f s");
                    ImGui::SliderFloat("Decaimento (Decay)", &curr_op.decay, 0.01f, 2.0f, "%.2f s");
                    ImGui::SliderFloat("Sustentação (Sustain)", &curr_op.sustain, 0.0f, 1.0f, "%.2f");
                    ImGui::SliderFloat("Liberação (Release)", &curr_op.release, 0.01f, 3.0f, "%.2f s");
                    ImGui::NextColumn();
                }
                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Dica: Altere a razão (Ratio) do Operador B ou C em relação ao Operador A para criar tímbres metálicos, FM zaps e baixos futuristas instantâneos.");
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
