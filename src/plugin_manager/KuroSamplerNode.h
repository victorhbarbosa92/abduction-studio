#pragma once
#include "DAG.h"
#include "../thirdparty/dr_wav.h"
#include <vector>
#include <string>
#include <iostream>

namespace KuroDSP {

    class KuroSamplerNode : public PluginNode {
    private:
        std::vector<float> sample_data;
        unsigned int channels = 0;
        unsigned int sample_rate = 0;
        uint64_t total_frames = 0;
        
        // Estado de reprodução
        uint64_t current_frame = 0;
        bool is_playing = false;
        unsigned int pending_offset = 0;

    public:
        KuroSamplerNode(const std::string& id, const std::string& name) 
            : PluginNode(id, name) {}

        ~KuroSamplerNode() {
            sample_data.clear();
        }

        bool loadSample(const std::string& file_path) {
            drwav wav;
            if (!drwav_init_file(&wav, file_path.c_str(), nullptr)) {
                std::cerr << "Falha ao abrir WAV: " << file_path << std::endl;
                return false;
            }

            channels = wav.channels;
            sample_rate = wav.sampleRate;
            total_frames = wav.totalPCMFrameCount;
            
            sample_data.resize(total_frames * channels);
            drwav_read_pcm_frames_f32(&wav, total_frames, sample_data.data());
            drwav_uninit(&wav);

            std::cout << "Sample carregado: " << file_path 
                      << " (" << total_frames << " frames, " << channels << " ch)" << std::endl;

            // Reset loop pra tocar imediatamente (somente pra carregar e ter a certeza que leu)
            // Em fase 6, o sampler fica em silêncio até o noteOn.
            is_playing = false;
            return true;
        }

        void loadFromMemory(const std::vector<float>& data, unsigned int num_channels, unsigned int s_rate) {
            sample_data = data;
            channels = num_channels;
            sample_rate = s_rate;
            total_frames = data.size() / channels;
            is_playing = false;
            std::cout << "[Sampler] Memory loaded: " << total_frames << " frames" << std::endl;
        }

        void noteOn(unsigned int offset_frames = 0) {
            current_frame = 0;
            pending_offset = offset_frames;
            is_playing = true;
        }

        const std::vector<float>& getSampleData() const { return sample_data; }
        uint64_t getTotalFrames() const { return total_frames; }
        uint64_t getCurrentFrame() const { return current_frame; }
        unsigned int getChannels() const { return channels; }
        bool isPlaying() const { return is_playing; }

        void process(float* in_out_l, float* in_out_r, unsigned int frames) override {
            if (is_bypassed || !is_playing || sample_data.empty()) return;

            for (unsigned int i = 0; i < frames; ++i) {
                // Aguarda o offset (Sample-Accurate delay)
                if (pending_offset > 0) {
                    pending_offset--;
                    continue; // Silence until the exact offset frame is reached
                }

                if (current_frame >= total_frames) {
                    // Tocou até o final (One-shot, sem loop)
                    is_playing = false;
                    break;
                }

                if (channels == 1) {
                    float s = sample_data[current_frame];
                    in_out_l[i] += s;
                    in_out_r[i] += s;
                } else if (channels >= 2) {
                    in_out_l[i] += sample_data[current_frame * channels + 0];
                    in_out_r[i] += sample_data[current_frame * channels + 1];
                }

                current_frame++;
            }
        }

        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {
            // Em fases futuras, implementaremos Envelope, Pitch, etc.
        }
    };

}
