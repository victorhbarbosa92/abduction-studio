#pragma once
#include "imgui.h"
#include <string>
#include "../ai/StemSeparationEngine.h"
#include "SampleEditorUI.h"
#include <cstdlib>

extern std::string pending_moises_file;
extern int moises_stem_mode;

namespace KuroUI {

    static void RenderExtractorPage(StemSeparationEngine& engine) {
        ImGui::SetNextWindowPos(ImVec2(0, 45)); // Abaixo da barra superior
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 45));
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.02f, 0.03f, 1.0f));
        ImGui::Begin("ExtractorMode", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        
        ImVec2 window_size = ImGui::GetWindowSize();
        ImGui::SetCursorPos(ImVec2(window_size.x * 0.5f - 250, window_size.y * 0.2f));
        
        ImGui::BeginGroup();
        
        ImGui::TextColored(ImVec4(0.5f, 0.0f, 1.0f, 1.0f), "====== KURO STEM EXTRACTOR ======");
        ImGui::Spacing(); ImGui::Spacing();
        
        if (pending_moises_file.empty()) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Arraste uma musica para esta janela para comecar.");
            
            // Desenhar caixa de drop falsa
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max = ImVec2(p_min.x + 500, p_min.y + 200);
            ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, IM_COL32(20, 20, 30, 255), 10.0f);
            ImGui::GetWindowDrawList()->AddRect(p_min, p_max, IM_COL32(100, 0, 255, 255), 10.0f, 0, 2.0f);
            ImGui::GetWindowDrawList()->AddText(ImVec2(p_min.x + 180, p_min.y + 90), IM_COL32(150, 150, 150, 255), "[ DROP ZONE ]");
            ImGui::Dummy(ImVec2(500, 200));
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Arquivo Carregado:");
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
                ImGui::Text("Faixas Extraidas:");
                
                int num_tracks = engine.getTotalFrames() > 0 ? (moises_stem_mode == 8 ? 8 : 4) : 0;
                for(int i=0; i<num_tracks; i++) {
                    ImGui::BulletText("%s", track_names[i].c_str());
                }
                
                ImGui::Spacing(); ImGui::Spacing();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.3f, 1.0f));
                if (ImGui::Button("EXPORTAR PARA O PC (WAV)", ImVec2(500, 60))) {
                    // Pasta de exportação
                    std::string music_name = pending_moises_file.substr(pending_moises_file.find_last_of("/\\") + 1);
                    std::string folder_path;
                    const char* userProfile = std::getenv("USERPROFILE");
                    if (userProfile) {
                        folder_path = std::string(userProfile) + "\\Documents\\Abduction_Stems\\" + music_name + "_Stems";
                    } else {
                        folder_path = "C:\\Abduction_Stems\\" + music_name + "_Stems";
                    }
                    
                    engine.exportStems(folder_path, num_tracks);
                    
                    // Abrir pasta
                    std::string cmd = "explorer.exe \"" + folder_path + "\"";
                    system(cmd.c_str());
                    
                    pending_moises_file = ""; // Resetar
                }
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing(); ImGui::Spacing();
                
                static bool show_sample_editor = false;
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.6f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.8f, 1.0f));
                if (ImGui::Button("AUTO-SLICER: ABRIR EDITOR DE SAMPLES", ImVec2(500, 40))) {
                    std::string music_name = pending_moises_file.substr(pending_moises_file.find_last_of("/\\") + 1);
                    engine.generateOneShotsMemory(music_name, num_tracks);
                    show_sample_editor = true;
                }
                ImGui::PopStyleColor(2);
                
                if (show_sample_editor) {
                    RenderSampleEditor(&show_sample_editor, engine);
                }
                
                ImGui::Spacing();
                if (ImGui::Button("VOLTAR / EXTRAIR OUTRA MUSICA", ImVec2(500, 30))) {
                    pending_moises_file = "";
                    engine.reset();
                }
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "SELECIONE O MODO DE SEPARACAO:");
                ImGui::Spacing();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.0f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.0f, 0.8f, 1.0f));
                
                if (ImGui::Button("2 STEMS (Voz + Instrumental)", ImVec2(500, 50))) {
                    moises_stem_mode = 2; 
                    engine.startProcessing(pending_moises_file, 4);
                }
                ImGui::Spacing();
                if (ImGui::Button("4 STEMS (Voz + Baixo + Bateria + Outros)", ImVec2(500, 50))) {
                    moises_stem_mode = 4;
                    engine.startProcessing(pending_moises_file, 4);
                }
                ImGui::Spacing();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
                if (ImGui::Button("8 STEMS (Deep Psytrance Extractor)", ImVec2(500, 50))) {
                    moises_stem_mode = 8;
                    engine.startProcessing(pending_moises_file, 8);
                }
                ImGui::PopStyleColor(2);
                
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                if (ImGui::Button("CANCELAR", ImVec2(500, 30))) {
                    pending_moises_file = "";
                }
            }
        }
        
        ImGui::EndGroup();
        ImGui::End();
        ImGui::PopStyleColor();
    }
}
