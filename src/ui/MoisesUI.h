#pragma once
#include "imgui.h"
#include <string>
#include "../ai/StemSeparationEngine.h"

extern bool show_moises_modal;
extern std::string pending_moises_file;
extern int moises_stem_mode;

namespace KuroUI {

    static void RenderMoisesModal(StemSeparationEngine& engine) {
        if (!show_moises_modal) return;
        
        ImGui::OpenPopup("MOISES.AI - ISOLADOR DE STEMS");
        
        ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.02f, 0.02f, 0.03f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.0f, 1.0f, 1.0f));
        
        if (ImGui::BeginPopupModal("MOISES.AI - ISOLADOR DE STEMS", NULL, flags)) {
            
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Arquivo Carregado:");
            ImGui::TextWrapped("%s", pending_moises_file.c_str());
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            
            if (engine.isRunning()) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "PROCESSANDO... POR FAVOR AGUARDE");
                ImGui::Spacing();
                ImGui::ProgressBar(engine.getProgress(), ImVec2(500, 25));
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", engine.getStatus().c_str());
            } 
            else if (engine.hasFinished()) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "SEPARACAO CONCLUIDA COM SUCESSO!");
                ImGui::Spacing();
                if (ImGui::Button("ABRIR NO ABDUCTION STUDIO", ImVec2(500, 50))) {
                    show_moises_modal = false;
                    ImGui::CloseCurrentPopup();
                }
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "SELECIONE O MODO DE SEPARACAO:");
                ImGui::Spacing();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.0f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.0f, 0.8f, 1.0f));
                
                if (ImGui::Button("2 STEMS (Voz + Instrumental)", ImVec2(500, 60))) {
                    moises_stem_mode = 2; // Passaremos isso pro engine futuramente se necessario, por enquanto isola 4 e a gente so usa 2 na UI
                    engine.startProcessing(pending_moises_file, 4);
                }
                
                ImGui::Spacing();
                
                if (ImGui::Button("4 STEMS (Voz + Baixo + Bateria + Outros)", ImVec2(500, 60))) {
                    moises_stem_mode = 4;
                    engine.startProcessing(pending_moises_file, 4);
                }
                
                ImGui::Spacing();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.0f, 1.0f, 1.0f));
                if (ImGui::Button("8 STEMS (Psytrance Deep Extract - FX/Zaps)", ImVec2(500, 60))) {
                    moises_stem_mode = 8;
                    engine.startProcessing(pending_moises_file, 8);
                }
                
                ImGui::Spacing();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.5f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.7f, 1.0f));
                if (ImGui::Button("20 STEMS (Psytrance Total Deconstruction)", ImVec2(500, 60))) {
                    moises_stem_mode = 20;
                    engine.startProcessing(pending_moises_file, 20);
                }
                ImGui::PopStyleColor(2);
                
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                
                if (ImGui::Button("CANCELAR IMPORTACAO", ImVec2(500, 30))) {
                    show_moises_modal = false;
                    ImGui::CloseCurrentPopup();
                }
            }
            
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
    }

}
