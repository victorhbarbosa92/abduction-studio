#pragma once
#include <cmath>
#include <vector>

namespace KuroDSP {

    enum class LFOShape { SINE, TRIANGLE, SAWTOOTH, SQUARE };

    class LFOEngine {
    private:
        float sample_rate = 44100.0f;
        float frequency = 1.0f; // Hz
        float phase = 0.0f;
        LFOShape shape = LFOShape::SINE;
        
        bool is_sync = false;
        float sync_rate_beat = 1.0f / 4.0f; // Ex: 1/4 (seminima)

    public:
        LFOEngine() {}

        void setSampleRate(float sr) { sample_rate = sr; }
        void setFrequency(float freq) { frequency = freq; }
        void setShape(LFOShape s) { shape = s; }
        
        void setSync(bool sync, float beat_fraction = 0.25f) {
            is_sync = sync;
            sync_rate_beat = beat_fraction;
        }

        // Deve ser chamado a cada frame de audio para gerar o valor (-1 a +1)
        float process(float current_bpm = 140.0f) {
            float freq_to_use = frequency;
            
            if (is_sync && current_bpm > 0) {
                // Se 120 BPM -> 2 beats por segundo
                // Se 1/4 note -> (bpm / 60) * (1.0 / sync_rate_beat) ?
                // frequency = (BPM / 60) * (1.0 / (sync_rate_beat * 4)) -- depende de como contamos.
                // Ex: 1 batida (seminima = 1/4). 120 BPM = 2 batidas por seg = 2Hz pra sync 1/4.
                freq_to_use = (current_bpm / 60.0f) / (sync_rate_beat * 4.0f); 
            }

            float phase_inc = freq_to_use / sample_rate;
            phase += phase_inc;
            if (phase >= 1.0f) phase -= 1.0f;

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
            }
            return out;
        }
    };
}
