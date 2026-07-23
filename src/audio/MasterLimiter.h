#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace KuroAudio {

    class MasterLimiter {
    public:
        float threshold_db = -0.1f;  // Target ceiling in dB
        float ceiling_linear = 0.988f; // ~ -0.1dB
        float release_ms = 50.0f;
        float sample_rate = 44100.0f;
        
        // Lookahead buffer (e.g. 5ms)
        static constexpr size_t LOOKAHEAD_SAMPLES = 220; 
        std::vector<float> lookahead_buffer_l;
        std::vector<float> lookahead_buffer_r;
        size_t write_pos = 0;
        
        float current_gain = 1.0f;

        MasterLimiter() {
            lookahead_buffer_l.assign(LOOKAHEAD_SAMPLES, 0.0f);
            lookahead_buffer_r.assign(LOOKAHEAD_SAMPLES, 0.0f);
            ceiling_linear = std::pow(10.0f, threshold_db / 20.0f);
        }

        void setSampleRate(float sr) {
            sample_rate = sr;
        }

        void setCeiling(float db) {
            threshold_db = db;
            ceiling_linear = std::pow(10.0f, threshold_db / 20.0f);
        }

        // Soft clipper curve (tanh-like smoothly saturated limiter)
        inline float softClip(float sample) {
            if (sample > 1.0f) return (3.0f - (2.0f - sample * 3.0f) * (2.0f - sample * 3.0f)) / 3.0f;
            if (sample < -1.0f) return -(3.0f - (2.0f + sample * 3.0f) * (2.0f + sample * 3.0f)) / 3.0f;
            return std::tanh(sample);
        }

        void processBlock(float* buffer_l, float* buffer_r, size_t num_samples) {
            float release_coeff = std::exp(-1.0f / (0.001f * release_ms * sample_rate));

            for (size_t i = 0; i < num_samples; i++) {
                // Store input into lookahead circular buffer
                lookahead_buffer_l[write_pos] = buffer_l[i];
                lookahead_buffer_r[write_pos] = buffer_r[i];

                size_t read_pos = (write_pos + 1) % LOOKAHEAD_SAMPLES;
                float in_l = lookahead_buffer_l[read_pos];
                float in_r = lookahead_buffer_r[read_pos];

                // Detect peak in current input
                float peak = std::max(std::abs(buffer_l[i]), std::abs(buffer_r[i]));
                float target_gain = 1.0f;

                if (peak > ceiling_linear) {
                    target_gain = ceiling_linear / peak;
                }

                // Attack is instant, release is smooth
                if (target_gain < current_gain) {
                    current_gain = target_gain;
                } else {
                    current_gain = release_coeff * current_gain + (1.0f - release_coeff) * target_gain;
                }

                // Apply gain to delayed signal
                float out_l = in_l * current_gain;
                float out_r = in_r * current_gain;

                // Soft clip safety net
                buffer_l[i] = softClip(out_l * 1.05f) * ceiling_linear;
                buffer_r[i] = softClip(out_r * 1.05f) * ceiling_linear;

                write_pos = (write_pos + 1) % LOOKAHEAD_SAMPLES;
            }
        }
    };
}
