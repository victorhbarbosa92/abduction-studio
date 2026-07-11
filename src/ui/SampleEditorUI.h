#pragma once
#include "imgui.h"
#include "../ai/StemSeparationEngine.h"
#include "../plugin_manager/KuroSamplerNode.h"
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include "dr_wav.h"

namespace KuroUI {

    inline void RenderSampleEditor(bool* open, StemSeparationEngine& engine) {
        if (!*open) return;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.15f, 1.0f));
        if (ImGui::BeginChild("Editor de Samples (Curadoria)", ImVec2(0, 600), true)) {
            
            if (ImGui::Button("FECHAR EDITOR", ImVec2(150, 30))) {
                *open = false;
            }
            ImGui::Separator();
            
            if (engine.pending_slices.empty()) {
                ImGui::Text("Nenhum recorte na memoria. Use o Auto-Slicer primeiro.");
                ImGui::End();
                return;
            }

            ImGui::Text("Recortes Identificados: %d", (int)engine.pending_slices.size());
            ImGui::Separator();
            
            ImGui::BeginChild("SlicesList", ImVec2(0, -60), true);
            
            for (size_t i = 0; i < engine.pending_slices.size(); i++) {
                auto& slice = engine.pending_slices[i];
                
                ImGui::PushID((int)i);
                
                // Checkbox
                ImGui::Checkbox("##sel", &slice.selected);
                ImGui::SameLine();
                
                // Play Button
                if (ImGui::Button("PLAY", ImVec2(50, 30))) {
                    extern std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;
                    if (g_global_sampler) {
                        g_global_sampler->loadFromMemory(slice.data, 2, engine.getSampleRate());
                        g_global_sampler->noteOn(0);
                    }
                }
                ImGui::SameLine();
                
                // Nome
                ImGui::SetNextItemWidth(200);
                char name_buf[128];
                strncpy(name_buf, slice.suggested_name.c_str(), sizeof(name_buf));
                if (ImGui::InputText("##name", name_buf, sizeof(name_buf))) {
                    slice.suggested_name = name_buf;
                }
                ImGui::SameLine();
                
                // Waveform preview
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImVec2 p1 = ImVec2(p0.x + 400, p0.y + 30);
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_list->AddRectFilled(p0, p1, IM_COL32(20, 20, 20, 255));
                
                float w = 400.0f;
                float h = 15.0f; // Half height (center line)
                for (size_t w_idx = 0; w_idx < slice.waveform_preview.size() - 1; w_idx++) {
                    float v0 = slice.waveform_preview[w_idx] * h;
                    float v1 = slice.waveform_preview[w_idx+1] * h;
                    if (v0 > h) v0 = h;
                    if (v1 > h) v1 = h;
                    
                    float x0 = p0.x + (w_idx / 100.0f) * w;
                    float x1 = p0.x + ((w_idx+1) / 100.0f) * w;
                    
                    draw_list->AddLine(ImVec2(x0, p0.y + h - v0), ImVec2(x1, p0.y + h - v1), IM_COL32(100, 255, 100, 255), 1.5f);
                    draw_list->AddLine(ImVec2(x0, p0.y + h + v0), ImVec2(x1, p0.y + h + v1), IM_COL32(100, 255, 100, 255), 1.5f);
                }
                
                ImGui::SetCursorScreenPos(ImVec2(p0.x, p0.y + 35));
                
                ImGui::PopID();
                ImGui::Separator();
            }
            
            ImGui::EndChild();
            
            // Botão Salvar
            if (ImGui::Button("SALVAR SELECIONADOS NAS PASTAS DO STUDIO", ImVec2(-1, 40))) {
                std::string base_path;
                const char* userProfile = std::getenv("USERPROFILE");
                if (userProfile) {
                    base_path = std::string(userProfile) + "\\Documents\\Abduction_Studio_Samples";
                } else {
                    base_path = "C:\\Abduction_Studio_Samples";
                }
                std::filesystem::create_directories(base_path);
                
                drwav_data_format format;
                format.container = drwav_container_riff;
                format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
                format.channels = 2;
                format.sampleRate = engine.getSampleRate();
                format.bitsPerSample = 32;
                
                int salvos = 0;
                for (auto& slice : engine.pending_slices) {
                    if (slice.selected) {
                        std::string track_folder = base_path + "/" + track_names[slice.source_track];
                        std::filesystem::create_directories(track_folder);
                        
                        std::string filepath = track_folder + "/" + slice.suggested_name + ".wav";
                        drwav wav;
                        if (drwav_init_file_write(&wav, filepath.c_str(), &format, NULL)) {
                            drwav_write_pcm_frames(&wav, slice.data.size() / 2, slice.data.data());
                            drwav_uninit(&wav);
                            salvos++;
                        }
                    }
                }
                engine.pending_slices.clear();
                *open = false; // Fecha a janela após salvar
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
}
