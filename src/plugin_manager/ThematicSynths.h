#pragma once
#include "DAG.h"
#include "../audio/LockFreeAudioQueue.h"
#include "imgui.h"
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>

namespace KuroDSP {

    // --- MPE and Param Events ---
    struct MpeMidiEvent {
        int note_id;
        int key;
        bool is_note_on;
        float velocity;
        float pitch_bend;   // Semitones offset
        float slide_cc74;   // 0.0 to 1.0
        float pressure;     // 0.0 to 1.0
        float duration = -1.0f;
    };

    struct ParamChangeEvent {
        int param_index;
        float target_value;
        unsigned int frames_to_lerp;
    };

    // --- Base MPE Synth Class ---
    class MpeSynthNode : public PluginNode {
    protected:
        LockFreeAudioQueue<MpeMidiEvent> midi_queue;
        LockFreeAudioQueue<ParamChangeEvent> param_queue;
        float sample_rate = 44100.0f;

        static constexpr float KURO_TWO_PI = 6.2831853071f;

        float getFrequency(int note, float pitch_bend) {
            return 440.0f * std::pow(2.0f, (note - 69.0f + pitch_bend) / 12.0f);
        }

        // LERP helpers
        float lerp(float a, float b, float t) {
            return a + t * (b - a);
        }

    public:
        MpeSynthNode(const std::string& id, const std::string& name) 
            : PluginNode(id, name), midi_queue(1024), param_queue(1024) {}

        void pushMidiEvent(const MpeMidiEvent& ev) {
            midi_queue.push(ev);
        }

        void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {
            param_queue.push({param_index, target_value, frames_to_lerp});
        }
        
        virtual void renderCustomUI() = 0;
    };


    // 1. Expressive Lead Synth (GeoShred style FM)
    class ExpressiveLeadSynth : public MpeSynthNode {
    private:
        struct Voice {
            int note_id;
            int key;
            float freq;
            float current_time;
            float phase_c; // Carrier
            float phase_m; // Modulator
            float velocity;
            float pressure;
            float slide;
            float env;
            bool active;
            bool note_on;
            float release_time;
            float duration = -1.0f;
        };
        std::vector<Voice> voices;
        
        // FM Params
        float fm_ratio = 1.0f;
        float fm_ratio_target = 1.0f;
        float modulation_index = 0.0f;
        float mod_index_target = 0.0f;
        bool ui_note_active = false;

    public:
        ExpressiveLeadSynth(const std::string& id) : MpeSynthNode(id, "Expressive Lead (FM)") {
            voices.reserve(16);
        }

        void process(float* left, float* right, unsigned int frames) override {
            if (getBypass()) return;

            MpeMidiEvent m_ev;
            while (midi_queue.pop(m_ev)) {
                if (m_ev.note_id == -999) {
                    voices.clear();
                    continue;
                }
                if (m_ev.is_note_on) {
                    bool found = false;
                    for(auto& v : voices) {
                        if(!v.active) {
                            v = {m_ev.note_id, m_ev.key, getFrequency(m_ev.key, m_ev.pitch_bend), 0.0f, 0.0f, 0.0f, m_ev.velocity, m_ev.pressure, m_ev.slide_cc74, 0.0f, true, true, 0.0f, m_ev.duration};
                            found = true; break;
                        }
                    }
                    if(!found && voices.size() < 16) {
                        voices.push_back({m_ev.note_id, m_ev.key, getFrequency(m_ev.key, m_ev.pitch_bend), 0.0f, 0.0f, 0.0f, m_ev.velocity, m_ev.pressure, m_ev.slide_cc74, 0.0f, true, true, 0.0f, m_ev.duration});
                    }
                } else {
                    for(auto& v : voices) {
                        if(v.active && v.note_id == m_ev.note_id) {
                            v.note_on = false;
                            v.release_time = 0.0f;
                        }
                    }
                }
                // Handle MPE pressure/slide updates for existing notes
                for(auto& v : voices) {
                    if (v.active && v.note_id == m_ev.note_id) {
                        v.freq = getFrequency(m_ev.key, m_ev.pitch_bend);
                        v.pressure = m_ev.pressure;
                        v.slide = m_ev.slide_cc74;
                    }
                }
            }

            ParamChangeEvent p_ev;
            while (param_queue.pop(p_ev)) {
                if (p_ev.param_index == 0) fm_ratio_target = p_ev.target_value;
                else if (p_ev.param_index == 1) mod_index_target = p_ev.target_value;
            }

            bool any_active = false;
            for (const auto& v : voices) {
                if (v.active) { any_active = true; break; }
            }
            if (!any_active) return;

            float dt = 1.0f / sample_rate;
            
            for (unsigned int i = 0; i < frames; ++i) {
                // LERP parameters
                fm_ratio = lerp(fm_ratio, fm_ratio_target, 0.01f);
                modulation_index = lerp(modulation_index, mod_index_target, 0.01f);

                float out = 0.0f;
                for (auto& v : voices) {
                    if (!v.active) continue;

                    float fm_freq = v.freq * fm_ratio;
                    
                    // Modulator
                    float mod_sig = std::sin(v.phase_m) * (modulation_index + v.slide * 5.0f);
                    
                    // Carrier
                    float carrier_sig = std::sin(v.phase_c + mod_sig);

                    // Envelope
                    if (v.duration > 0.0f && v.current_time >= v.duration) {
                        v.note_on = false;
                    }
                    if (v.note_on) {
                        v.env = std::min(1.0f, v.env + dt * 50.0f); // Fast attack
                    } else {
                        v.env *= std::exp(-5.0f * dt); // Release
                        v.release_time += dt;
                        if (v.env < 0.001f) v.active = false;
                    }

                    // Pressure affects volume slightly and brightness
                    float amp = v.velocity * (0.8f + 0.2f * v.pressure) * v.env;
                    out += carrier_sig * amp * 0.2f;

                    v.phase_c += KURO_TWO_PI * v.freq * dt;
                    if (v.phase_c > KURO_TWO_PI) v.phase_c -= KURO_TWO_PI;
                    
                    v.phase_m += KURO_TWO_PI * fm_freq * dt;
                    if (v.phase_m > KURO_TWO_PI) v.phase_m -= KURO_TWO_PI;

                    v.current_time += dt;
                }

                left[i] += out;
                right[i] += out;
            }
        }

        void renderCustomUI() override {
            ImGui::Text("Expressive Lead (FM) - XY Pad");
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size = ImVec2(200.0f, 200.0f);
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_size.x, canvas_p0.y + canvas_size.y);
            
            draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
            draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
            
            ImGui::InvisibleButton("##xy_pad", canvas_size);
            
            if (ImGui::IsItemActive()) {
                if (!ui_note_active) {
                    pushMidiEvent({60, 60, true, 0.8f, 0.0f, 0.5f, 0.5f});
                    ui_note_active = true;
                }
                
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
                    ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_p0.x, ImGui::GetIO().MousePos.y - canvas_p0.y);
                    float x_norm = std::clamp(mouse_pos_in_canvas.x / canvas_size.x, 0.0f, 1.0f);
                    float y_norm = std::clamp(mouse_pos_in_canvas.y / canvas_size.y, 0.0f, 1.0f);
                    
                    // Mapear X para FM Ratio e Y para Mod Index
                    float new_fm_ratio = 1.0f + x_norm * 4.0f;
                    float new_mod_idx = y_norm * 10.0f;
                    
                    // Push lock-free
                    setParameter(0, new_fm_ratio);
                    setParameter(1, new_mod_idx);
                    
                    // MPE parameters map to cursor movement
                    pushMidiEvent({60, 60, true, 0.8f, 0.0f, x_norm, y_norm});
                    
                    ImVec2 cursor_pos = ImVec2(canvas_p0.x + x_norm * canvas_size.x, canvas_p0.y + y_norm * canvas_size.y);
                    draw_list->AddCircleFilled(cursor_pos, 5.0f, IM_COL32(255, 100, 100, 255));
                }
            } else if (ui_note_active) {
                pushMidiEvent({60, 60, false, 0.0f, 0.0f, 0.0f, 0.0f});
                ui_note_active = false;
            }
        }
    };

    // 2. Monk Synth (Formant Filters)
    class MonkSynth : public MpeSynthNode {
    private:
        float current_freq = 440.0f;
        float target_freq = 440.0f;
        float current_phase = 0.0f;
        float current_env = 0.0f;
        bool is_note_on = false;
        int current_note_id = -1;
        float lfo_phase = 0.0f;
        float current_duration = -1.0f;
        float current_note_time = 0.0f;
        int silent_samples = 44100;

        float morph_x = 0.5f; // 0.0 (A) -> 1.0 (U)
        float morph_target = 0.5f;
        
        float throat_growl = 0.0f;
        float throat_target = 0.0f;
        bool ui_note_active = false;

        // Delay State
        std::vector<float> delay_buffer_l;
        std::vector<float> delay_buffer_r;
        int delay_write_ptr = 0;

        // Simple Biquad BP state
        struct Biquad {
            float b0=0, b1=0, b2=0, a1=0, a2=0;
            float x1=0, x2=0, y1=0, y2=0;
            void setBP(float f0, float Q, float sr) {
                float w0 = 6.2831853071f * f0 / sr;
                float alpha = std::sin(w0) / (2.0f * Q);
                float a0 = 1.0f + alpha;
                b0 = alpha / a0;
                b1 = 0.0f;
                b2 = -alpha / a0;
                a1 = -2.0f * std::cos(w0) / a0;
                a2 = (1.0f - alpha) / a0;
            }
            float processDF1(float in) {
                float out = b0*in + b1*x1 + b2*x2 - a1*y1 - a2*y2;
                x2 = x1; x1 = in;
                y2 = y1; y1 = out;
                return out;
            }
        };

        Biquad f1_filter, f2_filter, f3_filter;

    public:
        MonkSynth(const std::string& id) : MpeSynthNode(id, "Monk Synth (Formant)") {
            delay_buffer_l.resize(44100, 0.0f);
            delay_buffer_r.resize(44100, 0.0f);
        }

        void process(float* left, float* right, unsigned int frames) override {
            if (getBypass()) return;

            MpeMidiEvent m_ev;
            while (midi_queue.pop(m_ev)) {
                if (m_ev.note_id == -999) {
                    is_note_on = false;
                    current_env = 0.0f;
                    continue;
                }
                if (m_ev.is_note_on) {
                    target_freq = getFrequency(m_ev.key, m_ev.pitch_bend);
                    if (!is_note_on || current_env < 0.01f) {
                        current_freq = target_freq; // Começa instantâneo se estiver silenciado
                    }
                    is_note_on = true;
                    current_note_id = m_ev.note_id;
                    current_duration = m_ev.duration;
                    current_note_time = 0.0f;
                    silent_samples = 0;
                } else {
                    if (m_ev.note_id == current_note_id) {
                        is_note_on = false;
                    }
                }
            }

            ParamChangeEvent p_ev;
            while (param_queue.pop(p_ev)) {
                if (p_ev.param_index == 0) morph_target = p_ev.target_value;
                if (p_ev.param_index == 1) throat_target = p_ev.target_value;
            }

            if (!is_note_on && current_env < 0.0001f && silent_samples >= 44100) {
                return;
            }

            float dt = 1.0f / sample_rate;
            int delay_size = (int)(sample_rate * 0.35f); // 350ms delay
            
            for (unsigned int i = 0; i < frames; ++i) {
                morph_x = lerp(morph_x, morph_target, 0.005f);
                throat_growl = lerp(throat_growl, throat_target, 0.005f);

                // Portamento Glide
                current_freq = lerp(current_freq, target_freq, 0.005f); 
                
                // Vibrato natural (5 Hz)
                lfo_phase += 5.0f * 6.2831853071f * dt;
                if (lfo_phase > 6.2831853071f) lfo_phase -= 6.2831853071f;
                float vibrato = 1.0f + 0.012f * std::sin(lfo_phase);
                float modulated_freq = current_freq * vibrato;

                // Morph entre vogais A, E, I, O, U
                float f1_table[5] = {730.0f, 530.0f, 270.0f, 400.0f, 300.0f};
                float f2_table[5] = {1090.0f, 1840.0f, 2290.0f, 840.0f, 870.0f};
                float f3_table[5] = {2440.0f, 2480.0f, 3010.0f, 2240.0f, 2240.0f};
                
                float clamped_morph = std::max(0.0f, std::min(1.0f, morph_x));
                float idx_float = clamped_morph * 4.0f;
                int idx = (int)idx_float;
                float fract = idx_float - (float)idx;
                if (idx >= 4) { idx = 3; fract = 1.0f; }
                
                float freq_f1 = lerp(f1_table[idx], f1_table[idx+1], fract);
                float freq_f2 = lerp(f2_table[idx], f2_table[idx+1], fract);
                float freq_f3 = lerp(f3_table[idx], f3_table[idx+1], fract);

                // Update filters
                if (i % 32 == 0) {
                    f1_filter.setBP(freq_f1, 8.0f, sample_rate);
                    f2_filter.setBP(freq_f2, 5.0f, sample_rate);
                    f3_filter.setBP(freq_f3, 4.0f, sample_rate);
                }

                float raw_mix = 0.0f;
                float saw = 2.0f * (current_phase / 6.2831853071f) - 1.0f;
                float sub = std::sin(current_phase * 0.5f);
                float growl = saw * (1.0f + throat_growl * sub);

                if (current_duration > 0.0f && current_note_time >= current_duration) {
                    is_note_on = false;
                }

                if (is_note_on) {
                    current_env = std::min(1.0f, current_env + dt * 15.0f); // Fast attack
                } else {
                    current_env *= std::exp(-3.0f * dt); // Moderate release
                }

                if (current_env > 0.001f) {
                    raw_mix = growl * current_env * 0.4f;
                    current_phase += 6.2831853071f * modulated_freq * dt;
                    if (current_phase > 6.2831853071f) current_phase -= 6.2831853071f;
                }

                // Apply parallel formants
                float f_out = f1_filter.processDF1(raw_mix) + f2_filter.processDF1(raw_mix)*0.8f + f3_filter.processDF1(raw_mix)*0.5f;
                
                // Ping-Pong Delay
                float dl_read = delay_buffer_l[(delay_write_ptr - delay_size + 44100) % 44100];
                float dr_read = delay_buffer_r[(delay_write_ptr - (delay_size / 2) + 44100) % 44100]; // Stereo spread
                
                float out_l = f_out + dr_read * 0.35f;
                float out_r = f_out + dl_read * 0.35f;
                
                delay_buffer_l[delay_write_ptr] = out_l;
                delay_buffer_r[delay_write_ptr] = out_r;
                delay_write_ptr = (delay_write_ptr + 1) % 44100;
                
                left[i] += out_l;
                right[i] += out_r;
                current_note_time += dt;
                if (!is_note_on && current_env < 0.0001f) {
                    silent_samples++;
                }
            }
        }

        void renderCustomUI() override {
            ImGui::Text("Monk Synth - Throat Control");
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size = ImVec2(200.0f, 100.0f);
            
            draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p0.x + canvas_size.x, canvas_p0.y + canvas_size.y), IM_COL32(30, 20, 20, 255));
            ImGui::InvisibleButton("##throat", canvas_size);
            
            if (ImGui::IsItemActive()) {
                if (!ui_note_active) {
                    pushMidiEvent({48, 48, true, 0.8f, 0.0f, 0.5f, 0.5f});
                    ui_note_active = true;
                }
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
                    ImVec2 mouse_pos = ImVec2(ImGui::GetIO().MousePos.x - canvas_p0.x, ImGui::GetIO().MousePos.y - canvas_p0.y);
                    float x_norm = std::clamp(mouse_pos.x / canvas_size.x, 0.0f, 1.0f);
                    float y_norm = std::clamp(mouse_pos.y / canvas_size.y, 0.0f, 1.0f);
                    
                    setParameter(0, x_norm); // Morph Aah -> Ooh
                    setParameter(1, 1.0f - y_norm); // Throat growl
                    
                    pushMidiEvent({48, 48, true, 0.8f, 0.0f, x_norm, y_norm});
                    
                    ImVec2 cursor_pos = ImVec2(canvas_p0.x + x_norm * canvas_size.x, canvas_p0.y + y_norm * canvas_size.y);
                    draw_list->AddCircleFilled(cursor_pos, 8.0f, IM_COL32(200, 150, 100, 255));
                }
            } else if (ui_note_active) {
                pushMidiEvent({48, 48, false, 0.0f, 0.0f, 0.0f, 0.0f});
                ui_note_active = false;
            }
        }
    };

    // 3. Alien Voice Synth (Granular/Wavefolder chaos)
    class AlienVoiceSynth : public MpeSynthNode {
    private:
        struct Voice {
            int note_id;
            int key;
            float freq;
            float carrier_phase;
            float modulator_phase;
            float env;
            bool note_on;
            bool active;
            float duration = -1.0f;
            float current_time = 0.0f;
        };
        std::vector<Voice> voices;

        float macro_abduction = 0.5f; // Macro principal para o disco voador
        float macro_abduction_target = 0.5f;
        float global_lfo_phase = 0.0f;
        bool ui_note_active = false;

    public:
        AlienVoiceSynth(const std::string& id) : MpeSynthNode(id, "Alien Synth (FM UFO Sweep)") {
            voices.reserve(16);
        }

        void process(float* left, float* right, unsigned int frames) override {
            if (getBypass()) return;

            MpeMidiEvent m_ev;
            while (midi_queue.pop(m_ev)) {
                if (m_ev.note_id == -999) {
                    voices.clear();
                    continue;
                }
                if (m_ev.is_note_on) {
                    bool found = false;
                    for(auto& v : voices) {
                        if(!v.active) {
                            v = {m_ev.note_id, m_ev.key, getFrequency(m_ev.key, m_ev.pitch_bend), 0.0f, 0.0f, 0.0f, true, true, m_ev.duration, 0.0f};
                            found = true; break;
                        }
                    }
                    if(!found && voices.size() < 16) {
                        voices.push_back({m_ev.note_id, m_ev.key, getFrequency(m_ev.key, m_ev.pitch_bend), 0.0f, 0.0f, 0.0f, true, true, m_ev.duration, 0.0f});
                    }
                } else {
                    for(auto& v : voices) {
                        if(v.active && v.note_id == m_ev.note_id) v.note_on = false;
                    }
                }
            }

            ParamChangeEvent p_ev;
            while (param_queue.pop(p_ev)) {
                if (p_ev.param_index == 0) macro_abduction_target = p_ev.target_value;
            }

            bool any_active = false;
            for (const auto& v : voices) {
                if (v.active) { any_active = true; break; }
            }
            if (!any_active) return;

            float dt = 1.0f / sample_rate;
            
            for (unsigned int i = 0; i < frames; ++i) {
                macro_abduction = lerp(macro_abduction, macro_abduction_target, 0.005f);

                // LFO global que simula o giro do disco voador (rate dependente do macro)
                float ufo_rate = 1.0f + macro_abduction * 15.0f;
                global_lfo_phase += KURO_TWO_PI * ufo_rate * dt;
                if (global_lfo_phase > KURO_TWO_PI) global_lfo_phase -= KURO_TWO_PI;
                
                float ufo_lfo = std::sin(global_lfo_phase); // -1 to 1

                float out = 0.0f;
                for (auto& v : voices) {
                    if (!v.active) continue;

                    if (v.duration > 0.0f && v.current_time >= v.duration) {
                        v.note_on = false;
                    }

                    // Envelope de Attack lento, Release medio (Sci-Fi pad)
                    if (v.note_on) v.env = std::min(1.0f, v.env + dt * 2.0f);
                    else {
                        v.env *= std::exp(-2.0f * dt);
                        if (v.env < 0.001f) v.active = false;
                    }

                    // A magia FM:
                    // Ratio do modulador cresce agressivamente com o Macro
                    float fm_ratio = 1.0f + (macro_abduction * 8.0f) + (ufo_lfo * macro_abduction * 2.0f);
                    float mod_freq = v.freq * fm_ratio;
                    
                    // Indice de Modulação (intensidade FM)
                    float mod_index = 2.0f + macro_abduction * 10.0f;

                    // Avança a fase do modulador
                    v.modulator_phase += KURO_TWO_PI * mod_freq * dt;
                    if (v.modulator_phase > KURO_TWO_PI) v.modulator_phase -= KURO_TWO_PI;
                    float modulator_out = std::sin(v.modulator_phase) * mod_index;

                    // Frequencia da Carrier sofre Pitch Drop estilo "Abduction" baseado no macro e no LFO
                    float carrier_freq = v.freq - (macro_abduction * v.freq * 0.4f * ufo_lfo);

                    // Avança a fase da Carrier (incorporando FM do modulador)
                    v.carrier_phase += KURO_TWO_PI * carrier_freq * dt + modulator_out * dt * 1000.0f;
                    if (v.carrier_phase > KURO_TWO_PI) v.carrier_phase -= KURO_TWO_PI;
                    
                    // Som da Carrier (senoide pura modulada)
                    float carrier_out = std::sin(v.carrier_phase);
                    
                    out += carrier_out * v.env * 0.15f;
                    v.current_time += dt;
                }

                left[i] += out;
                right[i] += out;
            }
        }

        void renderCustomUI() override {
            ImGui::Text("AlienSynth - Abduction FM");
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size = ImVec2(200.0f, 200.0f);

            
            draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p0.x + canvas_size.x, canvas_p0.y + canvas_size.y), IM_COL32(10, 40, 20, 255));
            ImGui::InvisibleButton("##alien", canvas_size);
            
            if (ImGui::IsItemActive()) {
                if (!ui_note_active) {
                    pushMidiEvent({36, 36, true, 0.8f, 0.0f, 0.5f, 0.5f});
                    ui_note_active = true;
                }
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
                    ImVec2 mouse_pos = ImVec2(ImGui::GetIO().MousePos.x - canvas_p0.x, ImGui::GetIO().MousePos.y - canvas_p0.y);
                    float x_norm = std::clamp(mouse_pos.x / canvas_size.x, 0.0f, 1.0f);
                    float y_norm = std::clamp(mouse_pos.y / canvas_size.y, 0.0f, 1.0f);
                    
                    setParameter(0, x_norm * 20.0f);
                    setParameter(1, y_norm);
                    
                    pushMidiEvent({36, 36, true, 0.8f, 0.0f, x_norm, y_norm});
                    
                    ImVec2 cursor_pos = ImVec2(canvas_p0.x + x_norm * canvas_size.x, canvas_p0.y + y_norm * canvas_size.y);
                    draw_list->AddCircleFilled(cursor_pos, y_norm * 10.0f, IM_COL32(100, 255, 100, 255));
                }
            } else if (ui_note_active) {
                pushMidiEvent({36, 36, false, 0.0f, 0.0f, 0.0f, 0.0f});
                ui_note_active = false;
            }
        }
    };
    // 4. Analog Monster Synth (Inspired by FL Studio Flex "Analog Monsters")
    class AnalogMonsterSynth : public MpeSynthNode {
    private:
        struct Voice {
            int note_id;
            float freq;
            float phase1;
            float phase2;
            float velocity;
            float envelope;
            bool active;
            bool note_on;
            float filter_state_0;
            float filter_state_1;
            float duration = -1.0f;
            float current_time = 0.0f;
        };
        std::vector<Voice> voices;
        
        float cutoff = 2000.0f;
        float cutoff_target = 2000.0f;
        float drive = 1.0f;
        float drive_target = 1.0f;
        float detune = 0.05f; 
        bool ui_note_active = false;

        float processSaw(float& phase, float freq, float sr) {
            phase += freq / sr;
            if (phase >= 1.0f) phase -= 1.0f;
            return (phase * 2.0f - 1.0f);
        }

    public:
        AnalogMonsterSynth(const std::string& id) : MpeSynthNode(id, "Analog Monster") {
            voices.resize(16);
            for(auto& v : voices) v.active = false;
        }

        void process(float* left, float* right, unsigned int frames) override {
            if (getBypass()) return;

            MpeMidiEvent m_ev;
            while (midi_queue.pop(m_ev)) {
                if (m_ev.note_id == -999) {
                    for(auto& v : voices) v.active = false;
                    continue;
                }
                if (m_ev.is_note_on) {
                    bool found = false;
                    for (auto& v : voices) {
                        if (!v.active) {
                            v.note_id = m_ev.note_id;
                            v.freq = getFrequency(m_ev.key, m_ev.pitch_bend);
                            v.phase1 = 0.0f;
                            v.phase2 = 0.0f;
                            v.velocity = m_ev.velocity;
                            v.envelope = 0.0f;
                            v.active = true;
                            v.note_on = true;
                            v.filter_state_0 = 0.0f;
                            v.filter_state_1 = 0.0f;
                            v.duration = m_ev.duration;
                            v.current_time = 0.0f;
                            found = true;
                            break;
                        }
                    }
                    if(!found) {
                        voices[0].note_id = m_ev.note_id;
                        voices[0].freq = getFrequency(m_ev.key, m_ev.pitch_bend);
                        voices[0].phase1 = 0.0f;
                        voices[0].phase2 = 0.0f;
                        voices[0].velocity = m_ev.velocity;
                        voices[0].envelope = 0.0f;
                        voices[0].active = true;
                        voices[0].note_on = true;
                        voices[0].duration = m_ev.duration;
                        voices[0].current_time = 0.0f;
                    }
                } else {
                    for (auto& v : voices) {
                        if (v.active && v.note_id == m_ev.note_id) {
                            v.note_on = false;
                        }
                    }
                }
            }

            ParamChangeEvent p_ev;
            while (param_queue.pop(p_ev)) {
                if (p_ev.param_index == 0) cutoff_target = p_ev.target_value;
                if (p_ev.param_index == 1) drive_target = p_ev.target_value;
            }

            bool any_active = false;
            for (const auto& v : voices) {
                if (v.active) { any_active = true; break; }
            }
            if (!any_active) return;

            float dt = 1.0f / sample_rate;
            float safe_cutoff = std::clamp(cutoff, 20.0f, sample_rate * 0.15f);
            float fc = safe_cutoff / sample_rate;
            float q = 0.8f; 
            float f = 2.0f * std::sin(KURO_TWO_PI * fc * 0.5f);

            for (unsigned int i = 0; i < frames; i++) {
                cutoff = lerp(cutoff, cutoff_target, 0.005f);
                drive = lerp(drive, drive_target, 0.005f);
                
                safe_cutoff = std::clamp(cutoff, 20.0f, sample_rate * 0.15f);
                fc = safe_cutoff / sample_rate;
                f = 2.0f * std::sin(KURO_TWO_PI * fc * 0.5f);

                float mix = 0.0f;
                for (auto& v : voices) {
                    if (v.active) {
                        if (v.duration > 0.0f && v.current_time >= v.duration) {
                            v.note_on = false;
                        }
                        v.envelope = lerp(v.envelope, v.note_on ? 1.0f : 0.0f, v.note_on ? 0.05f : 0.005f);
                        if (!v.note_on && v.envelope < 0.001f) {
                            v.active = false;
                            continue;
                        }

                        float osc1 = processSaw(v.phase1, v.freq, sample_rate);
                        float osc2 = processSaw(v.phase2, v.freq * (1.0f + detune), sample_rate);
                        
                        float out = (osc1 + osc2) * 0.5f * v.velocity * v.envelope;
                        if (std::isnan(out) || std::isinf(out)) out = 0.0f;
                        
                        // Simple Lowpass Filter (State Variable)
                        v.filter_state_0 += f * (out - v.filter_state_0 + q * (v.filter_state_0 - v.filter_state_1));
                        v.filter_state_1 += f * (v.filter_state_0 - v.filter_state_1);
                        
                        if (std::isnan(v.filter_state_1) || std::isinf(v.filter_state_1)) {
                            v.filter_state_0 = 0.0f;
                            v.filter_state_1 = 0.0f;
                        }

                        mix += v.filter_state_1;
                        v.current_time += dt;
                    }
                }
                
                // Saturation/Drive
                mix *= drive;
                mix = std::tanh(mix);

                left[i] += mix;
                right[i] += mix;
            }
        }

        void renderCustomUI() override {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.4f, 0.0f, 1.0f)); // Laranja/Vermelho
            ImGui::BeginChild("AnalogUI", ImVec2(0, 150), true);
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), "ANALOG MONSTER SYNTH");
            
            float curr_cutoff = cutoff_target;
            if (ImGui::SliderFloat("Cutoff", &curr_cutoff, 100.0f, 10000.0f, "%.1f Hz", ImGuiSliderFlags_Logarithmic)) {
                setParameter(0, curr_cutoff);
            }
            float curr_drive = drive_target;
            if (ImGui::SliderFloat("Drive/Saturator", &curr_drive, 1.0f, 10.0f)) {
                setParameter(1, curr_drive);
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
    };

    // 5. Synthwave Synth (Inspired by FL Studio Flex "Saif Sameer Synthwave")
    class SynthwaveSynth : public MpeSynthNode {
    private:
        struct Voice {
            int note_id;
            float freq;
            float phase;
            float velocity;
            float envelope;
            bool active;
            bool note_on;
            float duration = -1.0f;
            float current_time = 0.0f;
        };
        std::vector<Voice> voices;
        
        float pwm_width = 0.5f;
        float pwm_width_target = 0.5f;
        float attack = 0.1f;
        float attack_target = 0.1f;
        float release = 0.5f;
        float release_target = 0.5f;
        
        // Chorus Delay lines
        float delay_l[44100] = {0};
        float delay_r[44100] = {0};
        int d_ptr = 0;
        float chorus_phase = 0.0f;

    public:
        SynthwaveSynth(const std::string& id) : MpeSynthNode(id, "Synthwave 80s") {
            voices.resize(16);
            for(auto& v : voices) v.active = false;
        }

        void process(float* left, float* right, unsigned int frames) override {
            if (getBypass()) return;

            MpeMidiEvent m_ev;
            while (midi_queue.pop(m_ev)) {
                if (m_ev.note_id == -999) {
                    for(auto& v : voices) v.active = false;
                    continue;
                }
                if (m_ev.is_note_on) {
                    for (auto& v : voices) {
                        if (!v.active) {
                            v.note_id = m_ev.note_id;
                            v.freq = getFrequency(m_ev.key, m_ev.pitch_bend);
                            v.phase = 0.0f;
                            v.velocity = m_ev.velocity;
                            v.envelope = 0.0f;
                            v.active = true;
                            v.note_on = true;
                            v.duration = m_ev.duration;
                            v.current_time = 0.0f;
                            break;
                        }
                    }
                } else {
                    for (auto& v : voices) {
                        if (v.active && v.note_id == m_ev.note_id) {
                            v.note_on = false;
                        }
                    }
                }
            }

            ParamChangeEvent p_ev;
            while (param_queue.pop(p_ev)) {
                if (p_ev.param_index == 0) pwm_width_target = p_ev.target_value;
                if (p_ev.param_index == 1) attack_target = p_ev.target_value;
                if (p_ev.param_index == 2) release_target = p_ev.target_value;
            }

            bool any_active = false;
            for (const auto& v : voices) {
                if (v.active) { any_active = true; break; }
            }
            if (!any_active) return;

            float dt = 1.0f / sample_rate;
            for (unsigned int i = 0; i < frames; i++) {
                pwm_width = lerp(pwm_width, pwm_width_target, 0.005f);
                attack = lerp(attack, attack_target, 0.005f);
                release = lerp(release, release_target, 0.005f);
                
                float att_coef = 1.0f / (attack * sample_rate + 1.0f);
                float rel_coef = 1.0f / (release * sample_rate + 1.0f);

                float mix = 0.0f;
                for (auto& v : voices) {
                    if (v.active) {
                        if (v.duration > 0.0f && v.current_time >= v.duration) {
                            v.note_on = false;
                        }
                        if (v.note_on) {
                            v.envelope += att_coef;
                            if (v.envelope > 1.0f) v.envelope = 1.0f;
                        } else {
                            v.envelope -= rel_coef;
                            if (v.envelope <= 0.001f) {
                                v.active = false;
                                continue;
                            }
                        }

                        v.phase += v.freq / sample_rate;
                        if (v.phase >= 1.0f) v.phase -= 1.0f;
                        
                        float osc = (v.phase < pwm_width) ? 1.0f : -1.0f; // PWM Square
                        mix += osc * v.velocity * v.envelope * 0.2f;
                        v.current_time += dt;
                    }
                }
                
                // Stereo Chorus
                chorus_phase += 1.0f / sample_rate;
                if(chorus_phase > 1.0f) chorus_phase -= 1.0f;
                
                float mod_l = std::sin(KURO_TWO_PI * chorus_phase) * 0.005f * sample_rate;
                float mod_r = std::cos(KURO_TWO_PI * chorus_phase) * 0.005f * sample_rate;
                
                int read_l = (d_ptr - (int)(0.02f * sample_rate + mod_l) + 44100) % 44100;
                int read_r = (d_ptr - (int)(0.02f * sample_rate + mod_r) + 44100) % 44100;
                
                delay_l[d_ptr] = mix;
                delay_r[d_ptr] = mix;
                d_ptr = (d_ptr + 1) % 44100;

                left[i] += mix * 0.5f + delay_l[read_l] * 0.5f;
                right[i] += mix * 0.5f + delay_r[read_r] * 0.5f;
            }
        }

        void renderCustomUI() override {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 1.0f, 1.0f)); // Neon Pink/Purple
            ImGui::BeginChild("SynthwaveUI", ImVec2(0, 150), true);
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "SYNTHWAVE 80s (PWM & CHORUS)");
            
            float curr_pwm = pwm_width_target;
            if (ImGui::SliderFloat("Pulse Width", &curr_pwm, 0.05f, 0.95f)) {
                setParameter(0, curr_pwm);
            }
            float curr_att = attack_target;
            if (ImGui::SliderFloat("Attack", &curr_att, 0.01f, 2.0f, "%.2f s")) {
                setParameter(1, curr_att);
            }
            float curr_rel = release_target;
            if (ImGui::SliderFloat("Release", &curr_rel, 0.01f, 5.0f, "%.2f s")) {
                setParameter(2, curr_rel);
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
    };

    // --- 6. Abduction FM 4-Op Synth (Alien FM Engine) ---
    class AbductionFMSynth : public MpeSynthNode {
    private:
        struct Voice {
            int note_id = 0;
            float freq = 440.0f;
            float phase_op1 = 0.0f;
            float phase_op2 = 0.0f;
            float phase_op3 = 0.0f;
            float phase_op4 = 0.0f;
            float last_op4_out = 0.0f;
            float velocity = 0.0f;
            float envelope = 0.0f;
            bool active = false;
            bool note_on = false;
            float duration = -1.0f;
            float current_time = 0.0f;
        };

        std::vector<Voice> voices;

        // Targets & smoothed params
        float ratio_op1_target = 1.0f, ratio_op1 = 1.0f;
        float ratio_op2_target = 2.0f, ratio_op2 = 2.0f;
        float ratio_op3_target = 3.5f, ratio_op3 = 3.5f;
        float ratio_op4_target = 0.5f, ratio_op4 = 0.5f;

        float mod_idx1_target = 2.0f, mod_idx1 = 2.0f; // Op2 -> Op1
        float mod_idx2_target = 1.5f, mod_idx2 = 1.5f; // Op3 -> Op2
        float mod_idx3_target = 1.0f, mod_idx3 = 1.0f; // Op4 -> Op3
        float feedback_target = 0.3f, feedback = 0.3f; // Op4 -> Op4

        float attack_target = 0.02f, attack = 0.02f;
        float release_target = 0.4f, release = 0.4f;

    public:
        AbductionFMSynth(const std::string& id) : MpeSynthNode(id, "Abduction FM 4-Op") {
            voices.resize(16);
            for (auto& v : voices) v.active = false;
        }

        void process(float* left, float* right, unsigned int frames) override {
            if (getBypass()) return;

            MpeMidiEvent m_ev;
            while (midi_queue.pop(m_ev)) {
                if (m_ev.note_id == -999) {
                    for (auto& v : voices) v.active = false;
                    continue;
                }
                if (m_ev.is_note_on) {
                    for (auto& v : voices) {
                        if (!v.active) {
                            v.note_id = m_ev.note_id;
                            v.freq = getFrequency(m_ev.key, m_ev.pitch_bend);
                            v.phase_op1 = 0.0f;
                            v.phase_op2 = 0.0f;
                            v.phase_op3 = 0.0f;
                            v.phase_op4 = 0.0f;
                            v.last_op4_out = 0.0f;
                            v.velocity = m_ev.velocity;
                            v.envelope = 0.0f;
                            v.active = true;
                            v.note_on = true;
                            v.duration = m_ev.duration;
                            v.current_time = 0.0f;
                            break;
                        }
                    }
                } else {
                    for (auto& v : voices) {
                        if (v.active && v.note_id == m_ev.note_id) {
                            v.note_on = false;
                        }
                    }
                }
            }

            ParamChangeEvent p_ev;
            while (param_queue.pop(p_ev)) {
                if (p_ev.param_index == 0) ratio_op1_target = p_ev.target_value;
                if (p_ev.param_index == 1) ratio_op2_target = p_ev.target_value;
                if (p_ev.param_index == 2) ratio_op3_target = p_ev.target_value;
                if (p_ev.param_index == 3) ratio_op4_target = p_ev.target_value;
                if (p_ev.param_index == 4) mod_idx1_target = p_ev.target_value;
                if (p_ev.param_index == 5) mod_idx2_target = p_ev.target_value;
                if (p_ev.param_index == 6) mod_idx3_target = p_ev.target_value;
                if (p_ev.param_index == 7) feedback_target = p_ev.target_value;
                if (p_ev.param_index == 8) attack_target = p_ev.target_value;
                if (p_ev.param_index == 9) release_target = p_ev.target_value;
            }

            bool any_active = false;
            for (const auto& v : voices) {
                if (v.active) { any_active = true; break; }
            }
            if (!any_active) return;

            float dt = 1.0f / sample_rate;
            for (unsigned int i = 0; i < frames; i++) {
                ratio_op1 = lerp(ratio_op1, ratio_op1_target, 0.005f);
                ratio_op2 = lerp(ratio_op2, ratio_op2_target, 0.005f);
                ratio_op3 = lerp(ratio_op3, ratio_op3_target, 0.005f);
                ratio_op4 = lerp(ratio_op4, ratio_op4_target, 0.005f);
                mod_idx1 = lerp(mod_idx1, mod_idx1_target, 0.005f);
                mod_idx2 = lerp(mod_idx2, mod_idx2_target, 0.005f);
                mod_idx3 = lerp(mod_idx3, mod_idx3_target, 0.005f);
                feedback = lerp(feedback, feedback_target, 0.005f);
                attack = lerp(attack, attack_target, 0.005f);
                release = lerp(release, release_target, 0.005f);

                float att_coef = 1.0f / (attack * sample_rate + 1.0f);
                float rel_coef = 1.0f / (release * sample_rate + 1.0f);

                float mix = 0.0f;
                for (auto& v : voices) {
                    if (v.active) {
                        if (v.duration > 0.0f && v.current_time >= v.duration) {
                            v.note_on = false;
                        }
                        if (v.note_on) {
                            v.envelope += att_coef;
                            if (v.envelope > 1.0f) v.envelope = 1.0f;
                        } else {
                            v.envelope -= rel_coef;
                            if (v.envelope <= 0.001f) {
                                v.active = false;
                                continue;
                            }
                        }

                        // Operator 4 (Feedback Operator)
                        float op4_freq = v.freq * ratio_op4;
                        v.phase_op4 += op4_freq / sample_rate;
                        if (v.phase_op4 >= 1.0f) v.phase_op4 -= 1.0f;
                        float op4_out = std::sin(KURO_TWO_PI * (v.phase_op4 + v.last_op4_out * feedback));
                        v.last_op4_out = op4_out;

                        // Operator 3 (Modulates Op2)
                        float op3_freq = v.freq * ratio_op3;
                        v.phase_op3 += op3_freq / sample_rate;
                        if (v.phase_op3 >= 1.0f) v.phase_op3 -= 1.0f;
                        float op3_out = std::sin(KURO_TWO_PI * (v.phase_op3 + op4_out * mod_idx3));

                        // Operator 2 (Modulates Op1)
                        float op2_freq = v.freq * ratio_op2;
                        v.phase_op2 += op2_freq / sample_rate;
                        if (v.phase_op2 >= 1.0f) v.phase_op2 -= 1.0f;
                        float op2_out = std::sin(KURO_TWO_PI * (v.phase_op2 + op3_out * mod_idx2));

                        // Operator 1 (Carrier)
                        float op1_freq = v.freq * ratio_op1;
                        v.phase_op1 += op1_freq / sample_rate;
                        if (v.phase_op1 >= 1.0f) v.phase_op1 -= 1.0f;
                        float carrier = std::sin(KURO_TWO_PI * (v.phase_op1 + op2_out * mod_idx1));

                        mix += carrier * v.velocity * v.envelope * 0.25f;
                        v.current_time += dt;
                    }
                }

                left[i] += mix;
                right[i] += mix;
            }
        }

        void renderCustomUI() override {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.9f, 1.0f, 1.0f)); // Alien Cyan
            ImGui::BeginChild("AbductionFMUI", ImVec2(0, 210), true);
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "ABDUCTION FM 4-OPERATOR SYNTH (ALIEN MATRIX)");

            ImGui::Columns(2, "FMCols", false);
            
            // Coluna 1: Ratios
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.6f, 1.0f), "Operator Ratios");
            float r1 = ratio_op1_target, r2 = ratio_op2_target, r3 = ratio_op3_target, r4 = ratio_op4_target;
            if (ImGui::SliderFloat("Carrier Op1 Ratio", &r1, 0.25f, 16.0f, "%.2fx")) setParameter(0, r1);
            if (ImGui::SliderFloat("Mod Op2 Ratio", &r2, 0.25f, 16.0f, "%.2fx")) setParameter(1, r2);
            if (ImGui::SliderFloat("Mod Op3 Ratio", &r3, 0.25f, 16.0f, "%.2fx")) setParameter(2, r3);
            if (ImGui::SliderFloat("Mod Op4 Ratio", &r4, 0.25f, 16.0f, "%.2fx")) setParameter(3, r4);

            ImGui::NextColumn();

            // Coluna 2: Modulation Indices & Envelope
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.6f, 1.0f), "FM Modulation Matrix");
            float m1 = mod_idx1_target, m2 = mod_idx2_target, m3 = mod_idx3_target, fb = feedback_target;
            if (ImGui::SliderFloat("Op2 -> Op1 Index", &m1, 0.0f, 10.0f)) setParameter(4, m1);
            if (ImGui::SliderFloat("Op3 -> Op2 Index", &m2, 0.0f, 10.0f)) setParameter(5, m2);
            if (ImGui::SliderFloat("Op4 -> Op3 Index", &m3, 0.0f, 10.0f)) setParameter(6, m3);
            if (ImGui::SliderFloat("Op4 Feedback", &fb, 0.0f, 1.0f)) setParameter(7, fb);

            ImGui::Columns(1);
            ImGui::Separator();

            float att = attack_target, rel = release_target;
            if (ImGui::SliderFloat("FM Attack", &att, 0.001f, 1.0f, "%.3f s")) setParameter(8, att);
            ImGui::SameLine();
            if (ImGui::SliderFloat("FM Release", &rel, 0.01f, 4.0f, "%.2f s")) setParameter(9, rel);

            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
    };

    // -------------------------------------------------------------------------
    // AcousticContrabassSynth: Sintetizador Físico de Contrabaixo Acústico Real
    // Modelo: Impulso de Pluck (Ataque de corda), Ressonância de Madeira (Dual Formant Body Cavity)
    // -------------------------------------------------------------------------
    class AcousticContrabassSynth : public MpeSynthNode {
    private:
        struct Voice {
            int note;
            float freq;
            float phase_fundamental = 0.0f;
            float phase_sub = 0.0f;
            float phase_harm3 = 0.0f;
            float envelope = 1.0f;
            float time_alive = 0.0f;
            float velocity = 0.8f;
            bool active = false;

            // Formant Body Filters (Body cavity & Wood resonance)
            float body_filter1_state1 = 0.0f, body_filter1_state2 = 0.0f;
            float body_filter2_state1 = 0.0f, body_filter2_state2 = 0.0f;
        };

        static constexpr size_t MAX_VOICES = 8;
        Voice voices[MAX_VOICES];

        float body_resonance_target = 0.85f;
        float body_resonance = 0.85f;

        float slap_amount_target = 0.40f;
        float slap_amount = 0.40f;

        float attack_target = 0.003f;
        float attack = 0.003f;

        float release_target = 0.60f;
        float release = 0.60f;

    public:
        AcousticContrabassSynth(const std::string& id) : MpeSynthNode(id, "Contrabaixo Acustico Real") {
            for (size_t i = 0; i < MAX_VOICES; i++) {
                voices[i].active = false;
            }
        }

        void process(float* left, float* right, unsigned int num_frames) override {
            MpeMidiEvent ev;
            while (midi_queue.pop(ev)) {
                if (ev.note_id == -999) {
                    for (size_t i = 0; i < MAX_VOICES; i++) {
                        voices[i].active = false;
                    }
                    continue;
                }
                if (ev.is_note_on) {
                    int v_idx = -1;
                    for (size_t i = 0; i < MAX_VOICES; i++) {
                        if (!voices[i].active) { v_idx = (int)i; break; }
                    }
                    if (v_idx == -1) v_idx = 0;

                    auto& v = voices[v_idx];
                    v.note = ev.key;
                    v.freq = getFrequency(ev.key, ev.pitch_bend);
                    v.phase_fundamental = 0.0f;
                    v.phase_sub = 0.0f;
                    v.phase_harm3 = 0.0f;
                    v.envelope = 1.0f;
                    v.time_alive = 0.0f;
                    v.velocity = ev.velocity;
                    v.active = true;
                    v.body_filter1_state1 = v.body_filter1_state2 = 0.0f;
                    v.body_filter2_state1 = v.body_filter2_state2 = 0.0f;
                } else {
                    for (size_t i = 0; i < MAX_VOICES; i++) {
                        if (voices[i].active && voices[i].note == ev.key) {
                            voices[i].envelope *= 0.5f;
                        }
                    }
                }
            }

            ParamChangeEvent pev;
            while (param_queue.pop(pev)) {
                if (pev.param_index == 0) body_resonance_target = pev.target_value;
                if (pev.param_index == 1) slap_amount_target = pev.target_value;
                if (pev.param_index == 2) attack_target = pev.target_value;
                if (pev.param_index == 3) release_target = pev.target_value;
            }

            float dt = 1.0f / sample_rate;

            for (unsigned int i = 0; i < num_frames; i++) {
                body_resonance = lerp(body_resonance, body_resonance_target, 0.001f);
                slap_amount = lerp(slap_amount, slap_amount_target, 0.001f);
                attack = lerp(attack, attack_target, 0.001f);
                release = lerp(release, release_target, 0.001f);

                float mix = 0.0f;

                for (size_t v_idx = 0; v_idx < MAX_VOICES; v_idx++) {
                    auto& v = voices[v_idx];
                    if (!v.active) continue;

                    float dec_rate = (v.time_alive < attack) ? (1.0f / attack) : (1.0f / (release + 0.1f));
                    if (v.time_alive < attack) {
                        v.envelope = std::min(1.0f, v.time_alive * dec_rate);
                    } else {
                        v.envelope *= std::exp(-dt / (release + 0.05f));
                    }

                    if (v.envelope < 0.001f) {
                        v.active = false;
                        continue;
                    }

                    float pitch_stretch = 1.0f + 0.04f * std::exp(-v.time_alive * 40.0f);
                    float cur_freq = v.freq * pitch_stretch;

                    v.phase_fundamental += cur_freq / sample_rate;
                    if (v.phase_fundamental >= 1.0f) v.phase_fundamental -= 1.0f;

                    v.phase_sub += (cur_freq * 0.5f) / sample_rate;
                    if (v.phase_sub >= 1.0f) v.phase_sub -= 1.0f;

                    v.phase_harm3 += (cur_freq * 3.0f) / sample_rate;
                    if (v.phase_harm3 >= 1.0f) v.phase_harm3 -= 1.0f;

                    float tri_fund = 2.0f * std::abs(2.0f * (v.phase_fundamental - std::floor(v.phase_fundamental + 0.5f))) - 1.0f;
                    float sine_sub = std::sin(KURO_TWO_PI * v.phase_sub);
                    float sine_harm3 = std::sin(KURO_TWO_PI * v.phase_harm3);

                    float raw_string = 0.55f * tri_fund + 0.35f * sine_sub + 0.10f * sine_harm3;

                    float noise_slap = 0.0f;
                    if (v.time_alive < 0.015f) {
                        float noise = ((rand() % 1000) / 500.0f - 1.0f);
                        noise_slap = noise * (1.0f - v.time_alive / 0.015f) * slap_amount;
                    }

                    float string_signal = raw_string + noise_slap;

                    float f1_cutoff = 110.0f / sample_rate;
                    float f1_q = 0.85f * body_resonance;
                    v.body_filter1_state1 += f1_cutoff * (string_signal - v.body_filter1_state1 + f1_q * (v.body_filter1_state1 - v.body_filter1_state2));
                    v.body_filter1_state2 += f1_cutoff * (v.body_filter1_state1 - v.body_filter2_state2);

                    float f2_cutoff = 380.0f / sample_rate;
                    v.body_filter2_state1 += f2_cutoff * (string_signal - v.body_filter2_state1 + 0.6f * (v.body_filter2_state1 - v.body_filter2_state2));
                    v.body_filter2_state2 += f2_cutoff * (v.body_filter2_state1 - v.body_filter2_state2);

                    float acoustic_body_out = v.body_filter1_state1 * 0.7f + v.body_filter2_state1 * 0.3f;

                    mix += acoustic_body_out * v.velocity * v.envelope * 0.85f;
                    v.time_alive += dt;
                }

                left[i] += mix;
                right[i] += mix;
            }
        }

        void renderCustomUI() override {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.85f, 0.55f, 0.25f, 1.0f));
            ImGui::BeginChild("ContrabassUI", ImVec2(0, 190), true);
            ImGui::TextColored(ImVec4(0.95f, 0.65f, 0.35f, 1.0f), "CONTRABAIXO ACUSTICO REAL (PHYSICAL MODELING)");

            float res = body_resonance_target, slap = slap_amount_target, att = attack_target, rel = release_target;
            if (ImGui::SliderFloat("Corpo de Madeira (Resonancia)", &res, 0.1f, 0.98f)) setParameter(0, res);
            if (ImGui::SliderFloat("Estalo do Dedo (Slap Attack)", &slap, 0.0f, 1.0f)) setParameter(1, slap);
            if (ImGui::SliderFloat("Ataque de Pluck", &att, 0.001f, 0.05f, "%.4f s")) setParameter(2, att);
            if (ImGui::SliderFloat("Ressuo de Madeira (Release)", &rel, 0.1f, 2.0f, "%.2f s")) setParameter(3, rel);

            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
    };
}

