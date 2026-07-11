#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroDSP {

    // 1. KuroLimiter: Brickwall Limiter com lookahead zero e curva soft-knee
    class KuroLimiter {
    private:
        float threshold_db = -0.5f;
        float release_ms = 50.0f;
        float sample_rate = 44100.0f;
        float env = 0.0f;

    public:
        bool enabled = false;
        
        void setThreshold(float t) { threshold_db = t; }
        float getThreshold() const { return threshold_db; }
        void setRelease(float r) { release_ms = r; }
        
        void process(float* buffer_left, float* buffer_right, int samples) {
            if (!enabled) return;
            float release_coef = std::exp(-1.0f / (release_ms * 0.001f * sample_rate));
            float threshold_lin = std::pow(10.0f, threshold_db / 20.0f);

            for (int i = 0; i < samples; ++i) {
                float abs_l = std::abs(buffer_left[i]);
                float abs_r = std::abs(buffer_right[i]);
                float peak = std::max(abs_l, abs_r);
                
                if (peak > env) env = peak; // Instant attack
                else env = release_coef * (env - peak) + peak;
                
                if (env > threshold_lin) {
                    float gain = threshold_lin / env;
                    buffer_left[i] *= gain;
                    buffer_right[i] *= gain;
                }
            }
        }
    };

    // 2. KuroDelay: Ping-pong Delay Estereo
    class KuroDelay {
    private:
        std::vector<float> delay_buffer_l;
        std::vector<float> delay_buffer_r;
        int write_ptr = 0;
        float feedback = 0.4f;
        float mix = 0.3f;
        int delay_samples = 22050; // 500ms at 44100

    public:
        bool enabled = false;
        
        KuroDelay(int max_delay_samples = 88200) {
            delay_buffer_l.resize(max_delay_samples, 0.0f);
            delay_buffer_r.resize(max_delay_samples, 0.0f);
        }

        void setFeedback(float f) { feedback = std::clamp(f, 0.0f, 0.95f); }
        void setMix(float m) { mix = std::clamp(m, 0.0f, 1.0f); }
        void setTimeMs(float ms, float sr = 44100.0f) { delay_samples = std::clamp((int)(ms * 0.001f * sr), 1, (int)delay_buffer_l.size()-1); }

        void process(float* buffer_left, float* buffer_right, int samples) {
            if (!enabled) return;
            int len = static_cast<int>(delay_buffer_l.size());
            int read_ptr = (write_ptr - delay_samples + len) % len;
            
            for (int i = 0; i < samples; ++i) {
                float in_l = buffer_left[i];
                float in_r = buffer_right[i];
                
                float out_l = delay_buffer_l[read_ptr];
                float out_r = delay_buffer_r[read_ptr];
                
                // Ping-pong: feed L to R, and R to L
                delay_buffer_l[write_ptr] = in_l + out_r * feedback;
                delay_buffer_r[write_ptr] = in_r + out_l * feedback;
                
                buffer_left[i] = in_l * (1.0f - mix) + out_l * mix;
                buffer_right[i] = in_r * (1.0f - mix) + out_r * mix;

                write_ptr = (write_ptr + 1) % len;
                read_ptr = (read_ptr + 1) % len;
            }
        }
    };

    // 3. KuroChorus: Modulacao pitch basica
    class KuroChorus {
    private:
        std::vector<float> buffer_l;
        std::vector<float> buffer_r;
        int write_ptr = 0;
        float phase = 0.0f;
        float rate = 1.5f; // Hz
        float depth = 3.0f; // ms
        float mix = 0.5f;
        float sr = 44100.0f;

    public:
        bool enabled = false;
        
        KuroChorus(int max_samples = 4410) {
            buffer_l.resize(max_samples, 0.0f);
            buffer_r.resize(max_samples, 0.0f);
        }
        
        void setRate(float r) { rate = r; }
        void setDepth(float d) { depth = d; }
        void setMix(float m) { mix = m; }

        void process(float* out_l, float* out_r, int samples) {
            if (!enabled) return;
            int len = static_cast<int>(buffer_l.size());
            float phase_inc = 2.0f * 3.14159265f * rate / sr;
            
            for (int i = 0; i < samples; ++i) {
                buffer_l[write_ptr] = out_l[i];
                buffer_r[write_ptr] = out_r[i];
                
                float lfo_val = std::sin(phase);
                float lfo_val_r = std::cos(phase); // Quadrature para estéreo largo
                
                float delay_ms_l = 10.0f + lfo_val * depth;
                float delay_ms_r = 10.0f + lfo_val_r * depth;
                
                float d_samp_l = delay_ms_l * 0.001f * sr;
                float d_samp_r = delay_ms_r * 0.001f * sr;
                
                int read_ptr_l = (write_ptr - (int)d_samp_l + len) % len;
                int read_ptr_r = (write_ptr - (int)d_samp_r + len) % len;
                
                out_l[i] = out_l[i] * (1.0f - mix) + buffer_l[read_ptr_l] * mix;
                out_r[i] = out_r[i] * (1.0f - mix) + buffer_r[read_ptr_r] * mix;
                
                write_ptr = (write_ptr + 1) % len;
                phase += phase_inc;
                if (phase > 2.0f * 3.14159265f) phase -= 2.0f * 3.14159265f;
            }
        }
    };

    // 4. KuroReverb: FDN Simples
    class KuroReverb {
    private:
        float mix = 0.3f;
        float decay = 0.85f;
        
        // Comb filters simplificados (Schroeder style)
        std::vector<float> comb1, comb2, comb3, comb4;
        int p1 = 0, p2 = 0, p3 = 0, p4 = 0;
        
    public:
        bool enabled = false;
        
        KuroReverb() {
            comb1.resize(1557, 0.0f);
            comb2.resize(1617, 0.0f);
            comb3.resize(1491, 0.0f);
            comb4.resize(1422, 0.0f);
        }
        
        void setMix(float m) { mix = std::clamp(m, 0.0f, 1.0f); }
        void setDecay(float d) { decay = std::clamp(d, 0.0f, 0.99f); }
        
        void process(float* out_l, float* out_r, int samples) {
            if (!enabled) return;
            for (int i = 0; i < samples; ++i) {
                float in_mono = (out_l[i] + out_r[i]) * 0.5f;
                
                float out1 = comb1[p1];
                comb1[p1] = in_mono + out1 * decay;
                p1 = (p1 + 1) % comb1.size();
                
                float out2 = comb2[p2];
                comb2[p2] = in_mono + out2 * decay;
                p2 = (p2 + 1) % comb2.size();
                
                float out3 = comb3[p3];
                comb3[p3] = in_mono + out3 * decay;
                p3 = (p3 + 1) % comb3.size();
                
                float out4 = comb4[p4];
                comb4[p4] = in_mono + out4 * decay;
                p4 = (p4 + 1) % comb4.size();
                
                float rev_out = (out1 + out2 + out3 + out4) * 0.25f;
                
                // Pseudo-stereo spread
                out_l[i] = out_l[i] * (1.0f - mix) + rev_out * mix;
                out_r[i] = out_r[i] * (1.0f - mix) + (-rev_out) * mix; // Inversao de fase para pseudo-stereo
            }
        }
    };

    // Mantendo os mocks antigos para nao quebrar compatibilidade do main
    class CompGotica { public: bool enabled=true; void process(float* buf, int s) {} };
    class GothicEQ { public: bool enabled=true; void process(float* buf, int s) {} };
    class AtrasoEspacial { public: bool enabled=true; void process(float* bl, float* br, int s) {} };
    class Atrasoleria { public: bool enabled=true; void process(float* buf, int s) {} };

} // namespace KuroDSP
