#pragma once
#include <vector>
#include <cmath>

namespace KuroDSP {

    class TruePitchShifter {
    private:
        std::vector<float> delay_buffer_l;
        std::vector<float> delay_buffer_r;
        int write_ptr = 0;
        float phase1 = 0.0f;
        float phase2 = 0.5f;
        int window_size;
        float sample_rate;

    public:
        TruePitchShifter(float sr = 44100.0f, int window_ms = 45) {
            sample_rate = sr;
            window_size = static_cast<int>(sr * window_ms / 1000.0f);
            delay_buffer_l.resize(window_size, 0.0f);
            delay_buffer_r.resize(window_size, 0.0f);
        }

        void processSample(float* left, float* right, float pitch_ratio) {
            if (std::abs(pitch_ratio - 1.0f) < 0.01f) return;

            float phase_increment = (1.0f - pitch_ratio) / static_cast<float>(window_size);

            float in_l = *left;
            float in_r = *right;
            
            delay_buffer_l[write_ptr] = in_l;
            delay_buffer_r[write_ptr] = in_r;

            float read1 = write_ptr - (phase1 * window_size);
            if (read1 < 0) read1 += window_size;
            float read2 = write_ptr - (phase2 * window_size);
            if (read2 < 0) read2 += window_size;

            int ir1 = static_cast<int>(read1);
            int ir2 = static_cast<int>(read2);
            if (ir1 >= window_size) ir1 = window_size - 1;
            if (ir2 >= window_size) ir2 = window_size - 1;

            float out1_l = delay_buffer_l[ir1];
            float out1_r = delay_buffer_r[ir1];
            
            float out2_l = delay_buffer_l[ir2];
            float out2_r = delay_buffer_r[ir2];

            // Envelopes triângulos com equal-power crossfade aproximado
            float env1 = 1.0f - std::abs(phase1 * 2.0f - 1.0f);
            float env2 = 1.0f - std::abs(phase2 * 2.0f - 1.0f);
            
            float sum_power = std::sqrt(env1*env1 + env2*env2) + 0.001f;
            env1 /= sum_power;
            env2 /= sum_power;

            *left = (out1_l * env1) + (out2_l * env2);
            *right = (out1_r * env1) + (out2_r * env2);

            write_ptr++;
            if (write_ptr >= window_size) write_ptr = 0;

            phase1 += phase_increment;
            if (phase1 >= 1.0f) phase1 -= 1.0f;
            else if (phase1 < 0.0f) phase1 += 1.0f;

            phase2 += phase_increment;
            if (phase2 >= 1.0f) phase2 -= 1.0f;
            else if (phase2 < 0.0f) phase2 += 1.0f;
        }
    };

    class Pedalboard {
    private:
        float sample_rate = 44100.0f;
        
        // Tremolo LFO (Ritualístico)
        float lfo_phase_tremolo = 0.0f;
        
        // Ring Modulator Osc (Aliens)
        float osc_phase_ringmod = 0.0f;
        
        // Flanger Delay Line (Psicodélico)
        std::vector<float> delay_line;
        int delay_write_ptr = 0;
        float lfo_phase_flanger = 0.0f;
        
        TruePitchShifter pitch_shifter;
        
    public:
        // Parâmetros de Controle Dinâmico (Knobs/Sliders)
        float param_dark_drive = 2.5f; // 1.0 a 10.0
        float param_alien_freq = 120.0f; // 20.0 a 1000.0 Hz
        float param_ritual_rate = 1.5f; // 0.1 a 5.0 Hz
        float param_ritual_depth = 0.5f; // 0.0 a 1.0
        float param_psych_speed = 0.25f; // 0.01 a 2.0 Hz
        float param_psych_depth = 0.7f; // 0.0 a 1.0
        
        // Parâmetros Ativos de DSP (Base + Offset de LFO)
        float active_dark_drive = 2.5f;
        float active_alien_freq = 120.0f;
        float active_ritual_rate = 1.5f;
        float active_ritual_depth = 0.5f;
        
        // Parâmetros do Pitch Shifter Granular
        bool enable_abyss_pitch = false;
        float abyss_pitch_factor = 1.0f;
        
        // Presets Mágicos (Globais por Pedalboard)
        bool preset_dark = false;
        bool preset_space = false;
        bool preset_ritual = false;
        bool preset_psych = false;
        bool preset_cavern = false;
        bool preset_alien = false;
        
        Pedalboard() : delay_line(4410, 0.0f) {} // 100ms max delay para flanger
        
        void process(float* left, float* right, int samples) {
            for (int i = 0; i < samples; ++i) {
                float l = *left;
                float r = *right;
                
                // 1. GOTHIC OVERDRIVE (Efeito Dark)
                if (preset_dark) {
                    // Soft clipping hiperbólico leve + corte de agudos simulado
                    // Compensação de ganho para não estourar muito quando a sujeira sobe
                    float gain_comp = 1.2f / (1.0f + active_dark_drive * 0.2f);
                    l = std::tanh(l * active_dark_drive) * gain_comp;
                    r = std::tanh(r * active_dark_drive) * gain_comp;
                }
                
                // 2. RING MODULATOR (Falas Aliens)
                if (preset_alien) {
                    float osc = std::sin(osc_phase_ringmod * 2.0f * 3.1415926535f);
                    osc_phase_ringmod += active_alien_freq / sample_rate;
                    if (osc_phase_ringmod > 1.0f) osc_phase_ringmod -= 1.0f;
                    
                    l *= osc;
                    r *= osc;
                }
                
                // 3. TREMOLO (Ritualístico)
                if (preset_ritual) {
                    // range: (1.0 - depth) to 1.0
                    float lfo = (1.0f - active_ritual_depth) + active_ritual_depth * (0.5f + 0.5f * std::sin(lfo_phase_tremolo * 2.0f * 3.1415926535f));
                    lfo_phase_tremolo += active_ritual_rate / sample_rate;
                    if (lfo_phase_tremolo > 1.0f) lfo_phase_tremolo -= 1.0f;
                    
                    l *= lfo;
                    r *= lfo;
                }
                
                // 4. FLANGER (Psicodélico)
                if (preset_psych) {
                    float lfo = 0.5f + 0.5f * std::sin(lfo_phase_flanger * 2.0f * 3.1415926535f);
                    lfo_phase_flanger += param_psych_speed / sample_rate;
                    if (lfo_phase_flanger > 1.0f) lfo_phase_flanger -= 1.0f;
                    
                    int max_delay = 400; // ~9ms
                    float current_delay = 10.0f + (lfo * max_delay);
                    
                    int read_ptr = delay_write_ptr - static_cast<int>(current_delay);
                    if (read_ptr < 0) read_ptr += delay_line.size();
                    
                    float delayed = delay_line[read_ptr];
                    delay_line[delay_write_ptr] = (l + r) * 0.5f + (delayed * 0.7f); // Feedback
                    
                    delay_write_ptr++;
                    if (delay_write_ptr >= delay_line.size()) delay_write_ptr = 0;
                    
                    l = (l * (1.0f - param_psych_depth * 0.5f)) + (delayed * param_psych_depth);
                    r = (r * (1.0f - param_psych_depth * 0.5f)) - (delayed * param_psych_depth); // Pseudo-Stereo psicodélico
                }
                
                // (Cavern e Space usam a cadeia de Reverb do canal, então eles apenas engatilham mudanças de UI ou ganho aqui)
                if (preset_cavern) {
                    // Abafa o som direto (High-Cut fake)
                    l *= 0.4f;
                    r *= 0.4f;
                }
                
                // 5. ABYSS PITCH (Pitch Shift Granular de Domínio de Tempo)
                if (enable_abyss_pitch) {
                    pitch_shifter.processSample(&l, &r, abyss_pitch_factor);
                }
                
                *left = l;
                *right = r;
                
                left++;
                right++;
            }
        }
    };

} // namespace KuroDSP
