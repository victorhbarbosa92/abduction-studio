#pragma once
#include "../plugin_manager/DAG.h"
#include <string>
#include <vector>
#include <map>

namespace KuroAudio {

    class OfflineRenderer {
    public:
        // Renderiza X segundos de áudio do DAG isolando cada Track e salva em .wav
        static void RenderStems(KuroDSP::AudioGraph& graph, float duration_seconds, const std::string& output_dir) {
            unsigned int sample_rate = 44100;
            unsigned int block_size = 2048;
            unsigned int total_frames = (unsigned int)(duration_seconds * sample_rate);
            
            float temp_l[2048];
            float temp_r[2048];
            
            std::cout << "[OfflineRenderer] Iniciando Exportação Multi-Track para " << total_frames << " frames...\n";
            
            auto node_ids = graph.getNodeIds();
            std::map<std::string, std::vector<float>> stems_l;
            std::map<std::string, std::vector<float>> stems_r;
            
            for (const auto& id : node_ids) {
                stems_l[id].reserve(total_frames);
                stems_r[id].reserve(total_frames);
            }
            
            for (unsigned int processed = 0; processed < total_frames; processed += block_size) {
                unsigned int frames_to_process = std::min(block_size, total_frames - processed);
                
                for(int i=0; i<2048; i++) { temp_l[i] = 0.0f; temp_r[i] = 0.0f; }
                
                graph.process(temp_l, temp_r, frames_to_process, 140.0f);
                
                for (const auto& id : node_ids) {
                    const auto* buf_l = graph.getNodeBufferL(id);
                    const auto* buf_r = graph.getNodeBufferR(id);
                    if (buf_l && buf_r) {
                        for(unsigned int i=0; i<frames_to_process; ++i) {
                            stems_l[id].push_back((*buf_l)[i]);
                            stems_r[id].push_back((*buf_r)[i]);
                        }
                    }
                }
            }
            
            for (const auto& id : node_ids) {
                std::string filepath = output_dir + "\\stem_" + id + ".wav";
                saveBufferToWav(filepath, stems_l[id], stems_r[id], sample_rate);
                std::cout << "[OfflineRenderer] Salvo: " << filepath << "\n";
            }
            
            std::cout << "[OfflineRenderer] Concluído!\n";
        }

    private:
        static void saveBufferToWav(const std::string& filename, const std::vector<float>& buf_l, const std::vector<float>& buf_r, uint32_t sample_rate) {
            FILE* out = fopen(filename.c_str(), "wb");
            if (!out) return;
            
            uint32_t data_size = (uint32_t)(buf_l.size() * 2 * sizeof(int16_t));
            uint32_t file_size = 36 + data_size;
            
            fwrite("RIFF", 1, 4, out);
            fwrite(&file_size, 4, 1, out);
            fwrite("WAVE", 1, 4, out);
            
            fwrite("fmt ", 1, 4, out);
            uint32_t fmt_size = 16;
            fwrite(&fmt_size, 4, 1, out);
            uint16_t audio_format = 1; // PCM
            fwrite(&audio_format, 2, 1, out);
            uint16_t num_channels = 2; // Stereo
            fwrite(&num_channels, 2, 1, out);
            fwrite(&sample_rate, 4, 1, out);
            uint32_t byte_rate = sample_rate * num_channels * sizeof(int16_t);
            fwrite(&byte_rate, 4, 1, out);
            uint16_t block_align = num_channels * sizeof(int16_t);
            fwrite(&block_align, 2, 1, out);
            uint16_t bits_per_sample = 16;
            fwrite(&bits_per_sample, 2, 1, out);
            
            fwrite("data", 1, 4, out);
            fwrite(&data_size, 4, 1, out);
            
            for (size_t i = 0; i < buf_l.size(); ++i) {
                float l = buf_l[i];
                float r = buf_r[i];
                if (l < -1.0f) l = -1.0f;
                if (l > 1.0f) l = 1.0f;
                if (r < -1.0f) r = -1.0f;
                if (r > 1.0f) r = 1.0f;
                
                int16_t sample_l = (int16_t)(l * 32767.0f);
                int16_t sample_r = (int16_t)(r * 32767.0f);
                fwrite(&sample_l, 2, 1, out);
                fwrite(&sample_r, 2, 1, out);
            }
            
            fclose(out);
        }
    };

}
