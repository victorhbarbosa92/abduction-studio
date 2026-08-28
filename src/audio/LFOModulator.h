#pragma once
#include "LFOEngine.h"
#include <string>
#include <algorithm>
#include <cmath>

namespace KuroDSP {

    struct LFOModulator {
        LFOEngine engine;
        bool active = false;
        int shape = 0; // 0=Sine, 1=Triangle, 2=Sawtooth, 3=Square, 4=Perlin/Random, 5=Peak Controller (Envelope Follower)
        float rate = 1.0f; // Hz
        float depth = 0.5f; // 0.0 to 1.0
        bool sync = false;
        int sync_rate_idx = 2; // default 1/4 note
        
        int target_track = -1; // -1 = None, 0-7 = Track
        int target_param = 0;  // 0 = Dark Drive, 1 = Alien Freq, 2 = Ritual Depth, 3 = Ritual Rate
        
        // Peak Controller & Envelope Follower
        float envelope_peak = 0.0f;
        float attack_ms = 10.0f;
        float release_ms = 100.0f;
        float formula_multiplier = 1.0f;
        float formula_offset = 0.0f;

        // Base/default values of target parameter
        float base_value = 2.5f;

        // Processa seguidor de envelope de áudio em tempo real
        void feedAudioSample(float sample, float sample_rate = 44100.0f) {
            float abs_s = std::abs(sample);
            float att_coeff = std::exp(-1.0f / (0.001f * attack_ms * sample_rate));
            float rel_coeff = std::exp(-1.0f / (0.001f * release_ms * sample_rate));
            
            if (abs_s > envelope_peak) {
                envelope_peak = att_coeff * envelope_peak + (1.0f - att_coeff) * abs_s;
            } else {
                envelope_peak = rel_coeff * envelope_peak + (1.0f - rel_coeff) * abs_s;
            }
        }

        // Calcula o valor final com fórmula: (Input * Multiplier + Offset)
        float getModulatedValue(float raw_input) const {
            return std::clamp(raw_input * formula_multiplier + formula_offset, 0.0f, 1.0f);
        }
    };
}

extern KuroDSP::LFOModulator g_lfo_modulators[4];

