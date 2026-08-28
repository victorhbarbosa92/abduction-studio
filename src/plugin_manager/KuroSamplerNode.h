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
        unsigned int sample_rate = 44100;
        uint64_t total_frames = 0;
        
        // Estado de reprodução
        double current_frame_pos = 0.0;
        bool is_main_stream_playing = false;
        unsigned int pending_offset = 0;

        struct SamplerVoice {
            bool active = false;
            double current_pos = 0.0;
            uint64_t start_frame = 0;
            uint64_t end_frame = 0;
            double step = 1.0;
            float gain = 1.0f;
            float pan = 0.0f;
            int loop_mode = 0; // 0: One-shot, 1: Loop
            uint64_t loop_start = 0;
            uint64_t loop_end = 0;
        };

        static constexpr int MAX_VOICES = 32;
        SamplerVoice voices[MAX_VOICES];

    public:
        KuroSamplerNode(const std::string& id, const std::string& name) 
            : PluginNode(id, name) {
            for (int v = 0; v < MAX_VOICES; ++v) voices[v].active = false;
        }

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
                      << " (" << total_frames << " frames, " << channels << " ch, " << sample_rate << " Hz)" << std::endl;

            is_main_stream_playing = false;
            for (int v = 0; v < MAX_VOICES; ++v) voices[v].active = false;
            return true;
        }

        void loadFromMemory(const std::vector<float>& data, unsigned int num_channels, unsigned int s_rate) {
            sample_data = data;
            channels = num_channels;
            sample_rate = s_rate;
            total_frames = data.size() / channels;
            is_main_stream_playing = false;
            for (int v = 0; v < MAX_VOICES; ++v) voices[v].active = false;
            std::cout << "[Sampler] Memory loaded: " << total_frames << " frames @ " << sample_rate << " Hz" << std::endl;
        }

        void stopAllVoices() {
            for (int v = 0; v < MAX_VOICES; ++v) voices[v].active = false;
        }

        void playSlice(uint64_t start_f, uint64_t end_f, double pitch_mult = 1.0, float g = 1.0f, float p = 0.0f, int loop_m = 0) {
            if (sample_data.empty() || total_frames == 0) return;
            if (end_f == 0 || end_f > total_frames) end_f = total_frames;
            if (start_f >= end_f) start_f = 0;

            int voice_idx = -1;
            for (int v = 0; v < MAX_VOICES; ++v) {
                if (!voices[v].active) {
                    voice_idx = v;
                    break;
                }
            }
            if (voice_idx < 0) voice_idx = 0; // Se todas ocupadas, recicla a primeira

            double base_step = (sample_rate > 0) ? ((double)sample_rate / 44100.0) : 1.0;
            voices[voice_idx].active = true;
            voices[voice_idx].current_pos = (double)start_f;
            voices[voice_idx].start_frame = start_f;
            voices[voice_idx].end_frame = end_f;
            voices[voice_idx].step = base_step * pitch_mult;
            voices[voice_idx].gain = g;
            voices[voice_idx].pan = p;
            voices[voice_idx].loop_mode = loop_m;
            voices[voice_idx].loop_start = start_f;
            voices[voice_idx].loop_end = end_f;
        }

        void noteOn(unsigned int offset_frames = 0) {
            current_frame_pos = 0.0;
            pending_offset = offset_frames;
            is_main_stream_playing = true;
        }

        void play(uint64_t start_frame = 0) {
            double rate_ratio = (sample_rate > 0) ? ((double)sample_rate / 44100.0) : 1.0;
            current_frame_pos = (double)start_frame * rate_ratio;
            pending_offset = 0;
            is_main_stream_playing = true;
        }

        void pause() {
            is_main_stream_playing = false;
            for (int v = 0; v < MAX_VOICES; ++v) voices[v].active = false;
        }

        void stop() {
            current_frame_pos = 0.0;
            pending_offset = 0;
            is_main_stream_playing = false;
            for (int v = 0; v < MAX_VOICES; ++v) voices[v].active = false;
        }

        void seek(uint64_t frame) {
            double rate_ratio = (sample_rate > 0) ? ((double)sample_rate / 44100.0) : 1.0;
            current_frame_pos = (double)frame * rate_ratio;
        }

        const std::vector<float>& getSampleData() const { return sample_data; }
        uint64_t getTotalFrames() const { return total_frames; }
        uint64_t getCurrentFrame() const { return (uint64_t)current_frame_pos; }
        unsigned int getChannels() const { return channels; }
        unsigned int getSampleRate() const { return sample_rate; }
        bool isPlaying() const { return is_main_stream_playing; }

        void process(float* in_out_l, float* in_out_r, unsigned int frames) override {
            if (is_bypassed || sample_data.empty() || total_frames == 0) return;

            // 1. Processa todas as vozes ativas de Slices
            for (int v = 0; v < MAX_VOICES; ++v) {
                if (!voices[v].active) continue;

                auto& vox = voices[v];
                float pan_l = (vox.pan <= 0.0f) ? 1.0f : (1.0f - vox.pan);
                float pan_r = (vox.pan >= 0.0f) ? 1.0f : (1.0f + vox.pan);
                float vol_l = vox.gain * pan_l;
                float vol_r = vox.gain * pan_r;

                for (unsigned int i = 0; i < frames; ++i) {
                    uint64_t f0 = (uint64_t)vox.current_pos;
                    if (f0 >= vox.end_frame) {
                        if (vox.loop_mode == 1 && vox.loop_end > vox.loop_start) {
                            vox.current_pos = (double)vox.loop_start;
                            f0 = vox.loop_start;
                        } else {
                            vox.active = false;
                            break;
                        }
                    }

                    uint64_t f1 = (f0 + 1 < total_frames) ? (f0 + 1) : f0;
                    float frac = (float)(vox.current_pos - (double)f0);

                    if (channels == 1) {
                        float s0 = sample_data[f0];
                        float s1 = sample_data[f1];
                        float s = (s0 + frac * (s1 - s0));
                        in_out_l[i] += s * vol_l;
                        in_out_r[i] += s * vol_r;
                    } else if (channels >= 2) {
                        float l0 = sample_data[f0 * channels + 0];
                        float l1 = sample_data[f1 * channels + 0];
                        float r0 = sample_data[f0 * channels + 1];
                        float r1 = sample_data[f1 * channels + 1];
                        in_out_l[i] += (l0 + frac * (l1 - l0)) * vol_l;
                        in_out_r[i] += (r0 + frac * (r1 - r0)) * vol_r;
                    }

                    vox.current_pos += vox.step;
                }
            }

            // 2. Processa a reprodução principal contínua (se houver)
            if (is_main_stream_playing && pending_offset == 0 && current_frame_pos < (double)total_frames) {
                double step = (sample_rate > 0) ? ((double)sample_rate / 44100.0) : 1.0;
                for (unsigned int i = 0; i < frames; ++i) {
                    uint64_t f0 = (uint64_t)current_frame_pos;
                    if (f0 >= total_frames) {
                        is_main_stream_playing = false;
                        break;
                    }
                    uint64_t f1 = (f0 + 1 < total_frames) ? (f0 + 1) : (total_frames - 1);
                    float frac = (float)(current_frame_pos - (double)f0);

                    if (channels == 1) {
                        float s0 = sample_data[f0];
                        float s1 = sample_data[f1];
                        float s = s0 + frac * (s1 - s0);
                        in_out_l[i] += s;
                        in_out_r[i] += s;
                    } else if (channels >= 2) {
                        float l0 = sample_data[f0 * channels + 0];
                        float l1 = sample_data[f1 * channels + 0];
                        float r0 = sample_data[f0 * channels + 1];
                        float r1 = sample_data[f1 * channels + 1];
                        in_out_l[i] += l0 + frac * (l1 - l0);
                        in_out_r[i] += r0 + frac * (r1 - r0);
                    }
                    current_frame_pos += step;
                }
            }
        }

        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {
        }
    };

}
