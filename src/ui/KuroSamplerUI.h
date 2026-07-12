#pragma once
#include "imgui.h"
#include "../plugin_manager/KuroSamplerNode.h"
#include <memory>
#include <vector>

namespace KuroUI {

    inline void RenderKuroSampler(std::shared_ptr<KuroDSP::KuroSamplerNode> sampler) {
        if (!sampler) {
            ImGui::TextDisabled("Global Sampler Instance Not Found.");
            return;
        }

        ImGui::Text("Kuro Sampler Engine - V4.0");
        ImGui::Separator();

        const auto& sample_data = sampler->getSampleData();
        uint64_t total_frames = sampler->getTotalFrames();
        unsigned int channels = sampler->getChannels();

        if (sample_data.empty() || total_frames == 0) {
            ImGui::TextDisabled("Nenhum arquivo carregado no Sampler. (Arraste e solte um arquivo .wav aqui)");
            return;
        }

        ImGui::Text("Frames: %llu | Canais: %u | Status: %s", 
                    total_frames, channels, 
                    sampler->isPlaying() ? "Tocando" : "Parado");

        // Desenhar a forma de onda
        ImVec2 p_min = ImGui::GetCursorScreenPos();
        float width = ImGui::GetContentRegionAvail().x;
        float height = 150.0f; // Altura fixa para o gráfico da onda
        ImVec2 p_max = ImVec2(p_min.x + width, p_min.y + height);
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        
        // Fundo do gráfico
        draw_list->AddRectFilled(p_min, p_max, IM_COL32(20, 20, 25, 255), 5.0f);
        draw_list->AddRect(p_min, p_max, IM_COL32(80, 80, 90, 255), 5.0f);
        
        // Linha do zero (centro)
        float mid_y = p_min.y + (height * 0.5f);
        draw_list->AddLine(ImVec2(p_min.x, mid_y), ImVec2(p_max.x, mid_y), IM_COL32(50, 50, 60, 255));

        // Desenhar os picos pulando amostras (Otimização para arquivos grandes)
        if (width > 0) {
            float samples_per_pixel = (float)total_frames / width;
            if (samples_per_pixel < 1.0f) samples_per_pixel = 1.0f;
            
            unsigned int step = (unsigned int)samples_per_pixel;
            if (step == 0) step = 1;

            ImVec2 last_pos(p_min.x, mid_y);
            
            for (float x = 0; x < width; x++) {
                uint64_t idx = (uint64_t)(x * samples_per_pixel) * channels;
                if (idx >= sample_data.size()) break;

                // Para uma onda bonita, podemos procurar o pico no intervalo do pixel atual
                float peak = 0.0f;
                uint64_t max_idx = std::min((uint64_t)sample_data.size(), (uint64_t)((x + 1) * samples_per_pixel * channels));
                for(uint64_t i = idx; i < max_idx; i += channels) {
                    float val = std::abs(sample_data[i]);
                    if (val > peak) peak = val;
                }

                float sample_val = peak;
                
                // Desenhar barra ou linha do envelope
                float y_top = mid_y - (sample_val * (height * 0.45f));
                float y_bottom = mid_y + (sample_val * (height * 0.45f));
                
                draw_list->AddLine(ImVec2(p_min.x + x, y_top), ImVec2(p_min.x + x, y_bottom), IM_COL32(57, 255, 20, 200));
            }
        }
        
        // Indicador de Reprodução (Playhead interno do Sampler)
        if (sampler->isPlaying()) {
            uint64_t current = sampler->getCurrentFrame();
            float playhead_x = p_min.x + ((float)current / total_frames) * width;
            draw_list->AddLine(ImVec2(playhead_x, p_min.y), ImVec2(playhead_x, p_max.y), IM_COL32(255, 50, 50, 255), 2.0f);
        }

        // Espaçador para o próximo componente de UI
        ImGui::Dummy(ImVec2(width, height + 10.0f));
        
        if (ImGui::Button("Preview Play", ImVec2(120, 30))) {
            sampler->noteOn(0);
        }
    }

}
