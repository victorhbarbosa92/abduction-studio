#pragma once
#include <cmath>
#include <vector>

namespace KuroDSP {

    enum class LFOShape { SINE, TRIANGLE, SAWTOOTH, SQUARE, PERLIN_RANDOM };

    class LFOEngine {
    private:
        float sample_rate = 44100.0f;
        float frequency = 1.0f; // Hz
        float phase = 0.0f;
        LFOShape shape = LFOShape::SINE;
        
        bool is_sync = false;
        float sync_rate_beat = 1.0f / 4.0f; // Ex: 1/4 (seminima)

        // Random Noise Smoother State
        float current_rand_val = 0.0f;
        float next_rand_val = 0.0f;

    public:
        LFOEngine() {
            current_rand_val = ((float)std::rand() / RAND_MAX) * 2.0f - 1.0f;
            next_rand_val = ((float)std::rand() / RAND_MAX) * 2.0f - 1.0f;
        }

        void setSampleRate(float sr) { sample_rate = sr; }
        void setFrequency(float freq) { frequency = freq; }
        void setShape(LFOShape s) { shape = s; }
        
        void setSync(bool sync, float beat_fraction = 0.25f) {
            is_sync = sync;
            sync_rate_beat = beat_fraction;
        }

        // Chamado a cada frame de áudio para gerar valor (-1 a +1)
        float process(float current_bpm = 140.0f) {
            float freq_to_use = frequency;
            
            if (is_sync && current_bpm > 0) {
                freq_to_use = (current_bpm / 60.0f) / (sync_rate_beat * 4.0f); 
            }

            float phase_inc = freq_to_use / sample_rate;
            phase += phase_inc;
            if (phase >= 1.0f) {
                phase -= 1.0f;
                current_rand_val = next_rand_val;
                next_rand_val = ((float)std::rand() / RAND_MAX) * 2.0f - 1.0f;
            }

            float out = 0.0f;
            switch (shape) {
                case LFOShape::SINE:
                    out = std::sin(2.0f * 3.1415926535f * phase);
                    break;
                case LFOShape::TRIANGLE:
                    out = 2.0f * std::abs(2.0f * phase - 1.0f) - 1.0f;
                    break;
                case LFOShape::SAWTOOTH:
                    out = 2.0f * phase - 1.0f;
                    break;
                case LFOShape::SQUARE:
                    out = phase < 0.5f ? 1.0f : -1.0f;
                    break;
                case LFOShape::PERLIN_RANDOM:
                    // Interpolação Hermite / Cosseno suave entre amostras aleatórias
                    float ft = phase * 3.1415926535f;
                    float f = (1.0f - std::cos(ft)) * 0.5f;
                    out = current_rand_val * (1.0f - f) + next_rand_val * f;
                    break;
            }
            return out;
        }
    };
}
