#pragma once
#include "imgui.h"
#include "../plugin_manager/KuroSamplerNode.h"
#include <memory>
#include <vector>
#include <algorithm>

namespace KuroUI {

    static void RenderKuroSampler(std::shared_ptr<KuroDSP::KuroSamplerNode> sampler) {
        if (!sampler) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Sampler pointer is null!");
            return;
        }

        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "🎛️ KURO SAMPLER (Drag & Drop WAV here)");
        ImGui::Separator();

        const auto& data = sampler->getSampleData();
        uint64_t total_frames = sampler->getTotalFrames();
        unsigned int channels = sampler->getChannels();

        if (data.empty() || total_frames == 0) {
            ImGui::Dummy(ImVec2(0, 50));
            ImGui::TextDisabled("Nenhum sample carregado. Arraste um arquivo .wav para a janela.");
            ImGui::Dummy(ImVec2(0, 50));
            return;
        }

        ImGui::Text("Sample Status: %llu frames | %d channels", total_frames, channels);
        
        // Controles simples de teste
        if (ImGui::Button("PLAY / RETRIGGER", ImVec2(150, 30))) {
            sampler->noteOn(0);
        }

        ImGui::Spacing();

        // Waveform Viewer
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p_min = ImGui::GetCursorScreenPos();
        ImVec2 p_max = ImVec2(p_min.x + ImGui::GetContentRegionAvail().x, p_min.y + 150);
        
        draw_list->AddRectFilled(p_min, p_max, IM_COL32(20, 20, 25, 255));
        draw_list->AddRect(p_min, p_max, IM_COL32(100, 100, 100, 255));

        float width = p_max.x - p_min.x;
        float mid_y = p_min.y + 75.0f;
        
        // Decimate data for drawing
        size_t samples_per_pixel = std::max<size_t>(1, total_frames / (size_t)width);
        
        for (float px = 0; px < width; ++px) {
            size_t start_idx = (size_t)(px * samples_per_pixel);
            if (start_idx >= total_frames) break;
            
            float min_val = 0.0f;
            float max_val = 0.0f;
            
            size_t end_idx = std::min<size_t>(total_frames, start_idx + samples_per_pixel);
            for (size_t i = start_idx; i < end_idx; ++i) {
                float v = data[i * channels]; // Usa o canal esquerdo pra desenhar
                if (v < min_val) min_val = v;
                if (v > max_val) max_val = v;
            }
            
            float amp_min = min_val * 70.0f;
            float amp_max = max_val * 70.0f;
            
            draw_list->AddLine(ImVec2(p_min.x + px, mid_y - amp_max), 
                               ImVec2(p_min.x + px, mid_y - amp_min), 
                               IM_COL32(0, 200, 255, 255));
        }

        // Desenhar agulha de playback se estiver tocando
        if (sampler->isPlaying()) {
            float playhead_x = p_min.x + ((float)sampler->getCurrentFrame() / (float)total_frames) * width;
            draw_list->AddLine(ImVec2(playhead_x, p_min.y), ImVec2(playhead_x, p_max.y), IM_COL32(255, 255, 255, 200), 2.0f);
        }

        ImGui::Dummy(ImVec2(0, 160));
    }
}
