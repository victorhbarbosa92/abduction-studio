#pragma once
#include <vector>
#include <cmath>
#include <mutex>
#include <algorithm>
#include "../core/MidiNote.h" // For MidiNote and basic types
#include "LFOEngine.h"

namespace KuroAudio {

    // Simples State Variable Filter (SVF) para dar o peso do Psytrance
    class SVFilter {
    private:
        float f, q;
        float hp, bp, lp;
    public:
        SVFilter() : f(0.0f), q(1.0f), hp(0.0f), bp(0.0f), lp(0.0f) {}

        void setParameters(float cutoff, float resonance, float sample_rate) {
            float safe_cutoff = std::clamp(cutoff, 20.0f, sample_rate * 0.15f);
            float safe_res = std::clamp(resonance, 0.5f, 10.0f);
            f = 2.0f * std::sin(3.1415926535f * safe_cutoff / sample_rate);
            q = 1.0f / safe_res;
        }

        float processLowpass(float input) {
            if (std::isnan(input) || std::isinf(input)) input = 0.0f;
            hp = input - lp - q * bp;
            bp += f * hp;
            lp += f * bp;
            if (std::isnan(lp) || std::isinf(lp)) {
                hp = bp = lp = 0.0f;
            }
            return lp;
        }
    };

    // Oscilador Wavetable com Unison (estilo Serum/Vital)
    class UnisonOscillator {
    private:
        int num_voices;
        float detune_spread; // em centésimos de semitom
        float stereo_spread; // 0.0 a 1.0

        struct UnisonVoice {
            float phase = 0.0f;
            float detune_ratio = 1.0f;
            float pan_l = 0.5f;
            float pan_r = 0.5f;
        };
        std::vector<UnisonVoice> voices;

    public:
        UnisonOscillator(int voices_count = 5, float detune = 0.15f, float spread = 0.8f) {
            setUnison(voices_count, detune, spread);
        }

        void setUnison(int count, float detune, float spread) {
            num_voices = std::clamp(count, 1, 16);
            detune_spread = detune;
            stereo_spread = spread;
            voices.resize(num_voices);

            if (num_voices == 1) {
                voices[0].detune_ratio = 1.0f;
                voices[0].pan_l = 0.5f;
                voices[0].pan_r = 0.5f;
                voices[0].phase = 0.0f;
                return;
            }

            for (int i = 0; i < num_voices; i++) {
                float norm = (float)i / (num_voices - 1); // 0.0 to 1.0
                float bipolar = norm * 2.0f - 1.0f; // -1.0 to 1.0
                
                // Detune: converte centavos em ratio de pitch
                float cents = bipolar * detune_spread * 100.0f;
                voices[i].detune_ratio = std::pow(2.0f, cents / 1200.0f);
                
                // Panning
                float pan = bipolar * stereo_spread; // -spread to +spread
                voices[i].pan_l = 0.5f - (pan * 0.5f);
                voices[i].pan_r = 0.5f + (pan * 0.5f);
                
                // Random phase
                voices[i].phase = ((float)rand() / RAND_MAX);
            }
        }

        float getWavetableSample(float phase, float wt_pos) {
            // wt_pos = 0.0 to 3.0
            float sine = std::sin(phase * 2.0f * 3.1415926535f);
            float tri = 2.0f * std::abs(2.0f * (phase - std::floor(phase + 0.5f))) - 1.0f;
            float saw = 2.0f * (phase - std::floor(phase + 0.5f));
            float sqr = (phase - std::floor(phase) < 0.5f) ? 1.0f : -1.0f;
            
            if (wt_pos < 1.0f) {
                return sine * (1.0f - wt_pos) + tri * wt_pos;
            } else if (wt_pos < 2.0f) {
                float mix = wt_pos - 1.0f;
                return tri * (1.0f - mix) + saw * mix;
            } else {
                float mix = wt_pos - 2.0f;
                return saw * (1.0f - mix) + sqr * mix;
            }
        }

        void process(float base_freq, float sample_rate, float wt_pos, float& out_l, float& out_r) {
            out_l = 0.0f;
            out_r = 0.0f;
            
            float mix_comp = 1.0f / std::sqrt((float)num_voices);

            for (auto& v : voices) {
                float freq = base_freq * v.detune_ratio;
                float phase_inc = freq / sample_rate;
                
                float wave = getWavetableSample(v.phase, wt_pos);
                
                out_l += wave * v.pan_l * mix_comp;
                out_r += wave * v.pan_r * mix_comp;
                
                v.phase += phase_inc;
                if (v.phase >= 1.0f) v.phase -= 1.0f;
            }
        }
    };

    class KuroWave {
    private:
        float sample_rate;
        std::vector<KuroDSP::MidiNote> notes;
        std::mutex synth_mutex;
        
        struct SynthVoice {
            int pitch;
            float current_time;
            float duration;
            float velocity;
            bool active;
            
            UnisonOscillator osc1;
            SVFilter filter_l;
            SVFilter filter_r;
            
            SynthVoice(int p, float dur, float vel) 
                : pitch(p), current_time(0.0f), duration(dur), velocity(vel), active(true) {
                osc1.setUnison(7, 0.15f, 0.8f); // Psytrance supersaw default
            }
        };
        std::vector<SynthVoice> active_voices;

        float getFrequency(int pitch) {
            return 440.0f * std::pow(2.0f, (pitch - 69) / 12.0f);
        }

    public:
        bool enabled = true;
        // Parâmetros Globais
        float param_cutoff = 2000.0f;
        float param_resonance = 1.5f;
        int param_unison_voices = 7;
        float param_unison_detune = 0.15f;
        float param_wt_position = 2.0f; // Padrão: Sawtooth
        
        // LFO
        KuroDSP::LFOEngine lfo_wt;
        float param_lfo_wt_mod = 0.5f; // Quanta modulação aplicar à wt_pos

        KuroWave() : sample_rate(44100.0f) {
            lfo_wt.setFrequency(0.5f); // 0.5 Hz varredura lenta
            lfo_wt.setShape(KuroDSP::LFOShape::SINE);
        }

        void setSampleRate(float sr) { 
            sample_rate = sr; 
            lfo_wt.setSampleRate(sr);
        }

        void noteOn(int pitch, float start, float duration, float velocity = 0.8f) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            notes.push_back(KuroDSP::MidiNote(pitch, start, duration, velocity));
        }

        void triggerNote(int pitch, float duration, float velocity = 0.8f) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            SynthVoice new_voice(pitch, duration, velocity);
            new_voice.osc1.setUnison(param_unison_voices, param_unison_detune, 0.8f);
            active_voices.push_back(new_voice);
        }

        void releaseNote(int pitch) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            for (auto& voice : active_voices) {
                if (voice.pitch == pitch && voice.current_time < voice.duration) {
                    voice.duration = voice.current_time;
                }
            }
        }

        void clearNotes() {
            std::lock_guard<std::mutex> lock(synth_mutex);
            notes.clear();
            active_voices.clear();
        }

        void process(float* out_left, float* out_right, unsigned int nFrames, float global_time_sec) {
            if (!enabled) return;
            std::lock_guard<std::mutex> lock(synth_mutex);
            float dt = 1.0f / sample_rate;
            
            for (unsigned int i = 0; i < nFrames; i++) {
                float sample_l = 0.0f;
                float sample_r = 0.0f;
                float current_time = global_time_sec + (i * dt);
                
                // Processa o LFO global de Wavetable (Gera valor de -1 a +1)
                float lfo_val = lfo_wt.process(140.0f); // Usando 140 BPM falso, mas o lfo ta em Hz free-run
                float current_wt_pos = param_wt_position + (lfo_val * param_lfo_wt_mod * 3.0f);
                current_wt_pos = std::clamp(current_wt_pos, 0.0f, 3.0f);

                for (auto& voice : active_voices) {
                    if (!voice.active) continue;

                    float freq = getFrequency(voice.pitch);
                    
                    // Envelopes (Pluck pra psytrance)
                    float env = 1.0f;
                    float attack = 0.005f;
                    float decay = 0.3f;
                    float sustain = 0.1f;
                    
                    if (voice.current_time < attack) {
                        env = voice.current_time / attack;
                    } else if (voice.current_time < attack + decay) {
                        env = 1.0f - (1.0f - sustain) * ((voice.current_time - attack) / decay);
                    } else {
                        env = sustain;
                    }

                    if (voice.current_time >= voice.duration) {
                        // Release
                        float release = 0.1f;
                        float rel_time = voice.current_time - voice.duration;
                        if (rel_time > release) voice.active = false;
                        else env = sustain * (1.0f - (rel_time / release));
                    }

                    // Modulação do Filtro pelo Envelope (Filter Env)
                    float current_cutoff = param_cutoff + (env * 5000.0f);
                    current_cutoff = std::clamp(current_cutoff, 20.0f, 20000.0f);
                    
                    voice.filter_l.setParameters(current_cutoff, param_resonance, sample_rate);
                    voice.filter_r.setParameters(current_cutoff, param_resonance, sample_rate);

                    float osc_l, osc_r;
                    voice.osc1.process(freq, sample_rate, current_wt_pos, osc_l, osc_r);

                    sample_l += voice.filter_l.processLowpass(osc_l) * env * voice.velocity * 0.5f;
                    sample_r += voice.filter_r.processLowpass(osc_r) * env * voice.velocity * 0.5f;
                    
                    voice.current_time += dt;
                }

                active_voices.erase(std::remove_if(active_voices.begin(), active_voices.end(), 
                    [](const SynthVoice& v) { return !v.active; }), active_voices.end());
                    
                out_left[i] += sample_l;
                out_right[i] += sample_r;
            }
        }
    };
}
