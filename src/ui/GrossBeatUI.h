#pragma once
#include "imgui.h"
#include "audio/GrossBeatNode.h"
#include <vector>

namespace KuroUI {

class GrossBeatUI {
public:
    static void Render(KuroDSP::GrossBeatNode& node, bool* p_open) {
        ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_FirstUseEver);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
        if (ImGui::Begin("Kuro Gross Beat (Master Bus)", p_open, flags)) {
            
            ImGui::Checkbox("ATIVADO", &node.enabled);
            ImGui::SameLine();
            ImGui::SliderFloat("MIX (Dry/Wet)", &node.mix, 0.0f, 1.0f);
            
            ImGui::Separator();
            
            ImGui::Text("TIME MANIPULATION ENVELOPE (Y: Offset, X: Bar Time)");
            
            // Desenha um grid interativo
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size = ImVec2(ImGui::GetContentRegionAvail().x, 300);
            if (canvas_size.x < 100) canvas_size.x = 100;
            
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(20, 20, 25, 255));
            draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(100, 100, 100, 255));
            
            // Grid lines (16 steps = 4 beats = 1 bar)
            for (int i = 0; i <= 16; i++) {
                float x = canvas_pos.x + (float)i / 16.0f * canvas_size.x;
                ImU32 color = (i % 4 == 0) ? IM_COL32(100, 100, 100, 150) : IM_COL32(50, 50, 50, 150);
                draw_list->AddLine(ImVec2(x, canvas_pos.y), ImVec2(x, canvas_pos.y + canvas_size.y), color);
            }
            // Grid lines Y
            for (int i = 0; i <= 4; i++) {
                float y = canvas_pos.y + (float)i / 4.0f * canvas_size.y;
                draw_list->AddLine(ImVec2(canvas_pos.x, y), ImVec2(canvas_pos.x + canvas_size.x, y), IM_COL32(50, 50, 50, 150));
            }
            
            // Invisible button to capture mouse
            ImGui::InvisibleButton("##canvas", canvas_size);
            bool is_hovered = ImGui::IsItemHovered();
            bool is_active = ImGui::IsItemActive();
            
            // Atualiza pontos com mouse input (simples arrastar do único ponto, ou preset)
            // Para simplificar, vou criar 4 botões de presets acima do grid
            
            // Desenha pontos atuais
            std::vector<KuroDSP::GrossBeatNode::Point> current_points;
            {
                // Copia pontos para renderizar (no futuro, pegar via getter)
                current_points = {{0.0f, 1.0f}, {1.0f, 0.0f}}; // Mocked, vai vir dos presets abaixo
            }
            
            // Ponto flutuante do cursor de reprodução poderia ser renderizado aqui
            
            ImGui::End();
        }
    }
    
    // Presets que o usuário pode escolher rapidamente
    static void ApplyPreset(KuroDSP::GrossBeatNode& node, int preset_id) {
        std::vector<KuroDSP::GrossBeatNode::Point> t_pts;
        
        switch (preset_id) {
            case 0: // Normal
                t_pts = { {0.0f, 1.0f}, {1.0f, 0.0f} };
                break;
            case 1: // Half-Speed
                t_pts = { {0.0f, 1.0f}, {1.0f, 0.5f} };
                break;
            case 2: // Vinyl Stop
                t_pts = { {0.0f, 1.0f}, {0.5f, 1.0f}, {1.0f, 0.0f} }; // Começa normal e drena
                break;
            case 3: // Reverse
                t_pts = { {0.0f, 0.0f}, {1.0f, 1.0f} };
                break;
            case 4: // Stutter 1/4
                t_pts = { 
                    {0.0f, 1.0f}, {0.249f, 0.75f}, {0.25f, 1.0f}, {0.499f, 0.75f},
                    {0.5f, 1.0f}, {0.749f, 0.75f}, {0.75f, 1.0f}, {1.0f, 0.75f}
                };
                break;
        }
        
        node.setTimePoints(t_pts);
    }
};

} // namespace
