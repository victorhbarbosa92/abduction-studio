#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

// Implementações puras e limpas de DSP
namespace KuroDSP {

    // 1. CompGótica: Compressor dinâmico rápido
    class CompGotica {
    private:
        float threshold_db = -12.0f;
        float ratio = 4.0f;
        float attack_ms = 2.0f;
        float release_ms = 40.0f;
        float sample_rate = 44100.0f;
        float env = 0.0f;

    public:
        bool enabled = true;
        
        void setThreshold(float t) { threshold_db = t; }
        float getThreshold() const { return threshold_db; }
        
        void process(float* buffer, int samples) {
            float attack_coef = std::exp(-1.0f / (attack_ms * 0.001f * sample_rate));
            float release_coef = std::exp(-1.0f / (release_ms * 0.001f * sample_rate));

            for (int i = 0; i < samples; ++i) {
                float abs_in = std::abs(buffer[i]);
                if (abs_in > env) env = attack_coef * (env - abs_in) + abs_in;
                else env = release_coef * (env - abs_in) + abs_in;

                float env_db = 20.0f * std::log10(env + 1e-6f);
                float gain_db = 0.0f;
                if (env_db > threshold_db) {
                    gain_db = -(env_db - threshold_db) * (1.0f - 1.0f / ratio);
                }
                
                float gain_linear = std::pow(10.0f, gain_db / 20.0f);
                buffer[i] *= gain_linear;
            }
        }
    };

    // 2. GothicEQ: Paramétrico de baixo custo (MOCK Simples Biquad)
    class GothicEQ {
    public:
        bool enabled = true;
        void process(float* buffer, int samples) {
            // Aplicação de filtro simples FIR ou IIR com intrinsecs AVX2 (Simulado aqui com loop standard)
            // Simples ganho atenuado para representar processamento
            for (int i = 0; i < samples; ++i) {
                buffer[i] *= 0.95f; 
            }
        }
    };

    // 3. AtrasoEspacial: Delay Estéreo psicodélico
    class AtrasoEspacial {
    private:
        std::vector<float> delay_buffer;
        int write_ptr = 0;
        float feedback = 0.5f;

    public:
        bool enabled = true;
        
        void setFeedback(float f) { feedback = f; }
        float getFeedback() const { return feedback; }
        
        AtrasoEspacial(int max_delay_samples = 44100) : delay_buffer(max_delay_samples, 0.0f) {}

        void process(float* buffer_left, float* buffer_right, int samples) {
            int delay_len = static_cast<int>(delay_buffer.size());
            for (int i = 0; i < samples; ++i) {
                float delayed = delay_buffer[write_ptr];
                
                // Simples echo mix
                float in_mix = (buffer_left[i] + buffer_right[i]) * 0.5f;
                delay_buffer[write_ptr] = in_mix + delayed * feedback;
                
                buffer_left[i] += delayed * 0.4f;
                buffer_right[i] += delayed * 0.4f;

                write_ptr = (write_ptr + 1) % delay_len;
            }
        }
    };

    // 4. Atrasoleria: Reverb Minimalista CPU-Free (MOCK)
    class Atrasoleria {
    public:
        bool enabled = true;
        void process(float* buffer, int samples) {
            // Em implementação real: Redes Allpass e Comb filters.
            for (int i = 0; i < samples; ++i) {
                buffer[i] *= 0.98f; // Representação pass-through
            }
        }
    };

} // namespace KuroDSP
