#pragma once
#include <vector>
#include <atomic>
#include <mutex>

namespace KuroAudio {

    class RecordManager {
    private:
        std::vector<float> recorded_buffer_l;
        std::vector<float> recorded_buffer_r;
        std::atomic<bool> is_recording{false};
        std::atomic<bool> record_master{false}; // Se true grava a saída, se false grava o mic
        std::mutex record_mutex;
        
    public:
        RecordManager() {}
        
        void setRecording(bool state, bool master = false) {
            if (state && !is_recording.load()) {
                std::lock_guard<std::mutex> lock(record_mutex);
                recorded_buffer_l.clear();
                recorded_buffer_r.clear();
                record_master.store(master);
            }
            is_recording.store(state);
        }
        
        bool isRecording() const { return is_recording.load(); }
        bool isMasterBouncing() const { return record_master.load(); }
        
        // Grava o sinal de Input (Microfone)
        void processInput(const float* in_interleaved, unsigned int frames, unsigned int channels) {
            if (!is_recording.load() || record_master.load()) return;
            if (!in_interleaved) return;
            
            std::lock_guard<std::mutex> lock(record_mutex);
            // Assumindo input interleaved
            for(unsigned int i = 0; i < frames; i++) {
                if (channels >= 2) {
                    recorded_buffer_l.push_back(in_interleaved[i*channels]);
                    recorded_buffer_r.push_back(in_interleaved[i*channels + 1]);
                } else if (channels == 1) {
                    recorded_buffer_l.push_back(in_interleaved[i]);
                    recorded_buffer_r.push_back(in_interleaved[i]);
                }
            }
        }
        
        // Grava o sinal de Output (Bouncing)
        void processOutput(const float* out_l, const float* out_r, unsigned int frames) {
            if (!is_recording.load() || !record_master.load()) return;
            
            std::lock_guard<std::mutex> lock(record_mutex);
            for(unsigned int i = 0; i < frames; i++) {
                recorded_buffer_l.push_back(out_l[i]);
                recorded_buffer_r.push_back(out_r[i]);
            }
        }
        
        size_t getRecordedFrames() {
            std::lock_guard<std::mutex> lock(record_mutex);
            return recorded_buffer_l.size();
        }

        // FASE 13: Salvar em Disco
        bool saveToWav(const std::string& filename, uint32_t sample_rate = 44100) {
            std::lock_guard<std::mutex> lock(record_mutex);
            if (recorded_buffer_l.empty()) return false;
            
            FILE* out = fopen(filename.c_str(), "wb");
            if (!out) return false;
            
            uint32_t data_size = (uint32_t)(recorded_buffer_l.size() * 2 * sizeof(int16_t));
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
            
            for (size_t i = 0; i < recorded_buffer_l.size(); ++i) {
                float l = recorded_buffer_l[i];
                float r = recorded_buffer_r[i];
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
            return true;
        }
    };

}
