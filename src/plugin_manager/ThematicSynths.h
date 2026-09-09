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

    // 10. Psytrance Rolling Bass Synth (Kuro Rolling Bass Engine)
    class PsytranceRollingBassSynth : public MpeSynthNode {
    public:
        struct VoiceMoogState {
            float y1 = 0.0f, y2 = 0.0f, y3 = 0.0f, y4 = 0.0f;
            float oldx = 0.0f, oldy1 = 0.0f, oldy2 = 0.0f, oldy3 = 0.0f;
        };

        struct Voice {
            int note_id = 0;
            int key = 36;
            float freq = 55.0f;
            float phase = 0.0f;
            float sub_phase = 0.0f;
            float velocity = 1.0f;
            float env_filter = 1.0f;
            float env_amp = 1.0f;
            float env_pitch = 1.0f;
            float time_alive = 0.0f;
            bool active = false;
            bool note_on = false;
            float click_time = 0.0f;
            VoiceMoogState filter_state;
        };

        struct BiquadHPF {
            float x1 = 0.0f, x2 = 0.0f;
            float y1 = 0.0f, y2 = 0.0f;
            inline float process(float in, float b0, float b1, float b2, float a1, float a2) {
                float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
                x2 = x1; x1 = in;
                y2 = y1; y1 = out;
                return out;
            }
            void reset() { x1 = x2 = y1 = y2 = 0.0f; }
        };

    private:
        std::vector<Voice> voices;
        BiquadHPF hpf_stage1;
        BiquadHPF hpf_stage2;

        // Parâmetros DSP
        int osc_type = 0; // 0=Psy Saw, 1=Square, 2=Sub-Sine, 3=Morph Saw/Square
        float phase_retrigger_deg = 90.0f; // 0 a 360 graus
        float transient_click = 0.45f;
        float cutoff_hz = 450.0f;
        float resonance = 0.40f;
        float env_decay_sec = 0.085f;
        float env_amount = 0.85f;
        float drive_amt = 0.35f;
        float sub_level = 0.40f;
        float master_vol = 0.90f;

        // Laser Pitch Punch & Sub Low-Cut
        float pitch_depth_semitones = 18.0f;
        float pitch_decay_sec = 0.004f;
        float hpf_cutoff_hz = 32.0f;

        // Targets para interpolação suave
        float cutoff_target = 450.0f;
        float resonance_target = 0.40f;
        float env_decay_target = 0.085f;
        float env_amount_target = 0.85f;
        float drive_target = 0.35f;
        float sub_target = 0.40f;
        float click_target = 0.45f;
        float pitch_depth_target = 18.0f;
        float pitch_decay_target = 0.004f;
        float hpf_cutoff_target = 32.0f;

        int synth_model = 0; // 0=Kuro Moog 24dB, 1=Virus TI Hyper-Saw, 2=Nord Lead Punch, 3=FM Sub-Rolling Bass, 4=Analog Monster Sub

        // ── MOTOR DE SÍNTESE DO KICK DRUM INTEGRADO (PUNCH + SUB + PHASE ALIGN) ──
        bool kick_enabled = true;
        bool kick_active = false;
        float kick_time = 0.0f;
        float kick_phase = 0.0f;
        float kick_velocity = 1.0f;
        float kick_start_hz = 185.0f;      // Frequência de ataque do punch (120Hz a 350Hz)
        float kick_end_hz = 50.0f;         // Fundamental do sub-grave afinado (35Hz a 75Hz)
        float kick_pitch_decay_ms = 36.0f; // Queda rápida de afinação (15ms a 70ms)
        float kick_amp_decay_ms = 220.0f;  // Duração do corpo/cauda (100ms a 380ms)
        float kick_click = 0.80f;          // Transiente de marreta/clique mecânico (0 a 1)
        float kick_drive = 0.60f;          // Saturação analógica e pegada de fita (0 a 1)
        float kick_vol = 0.95f;            // Volume independente do bumbo
        float kick_phase_align = 0.0f;     // Alinhamento fino de fase (0 a 360 graus) relativo ao baixo

        // ── MOTOR DE SÍNTESE DO PSY SNARE / CLAP INTEGRADO ──
        bool snare_enabled = true;
        bool snare_active = false;
        float snare_time = 0.0f;
        float snare_phase = 0.0f;
        float snare_velocity = 1.0f;
        float snare_tone_hz = 210.0f;        // Frequência do corpo do tambor
        float snare_noise_decay_ms = 135.0f; // Cauda de ruído estalado
        float snare_snap = 0.85f;            // Estalo de transiente agudo
        float snare_vol = 0.80f;             // Volume independente da caixa
        float snare_bpf1 = 0.0f, snare_bpf2 = 0.0f;

        // Buffer do Osciloscópio Vetorial Expandido (2048 amostras para travamento perfeito de zero-crossing)
        static const int OSC_BUF_SIZE = 2048;
        float osc_buffer[OSC_BUF_SIZE] = { 0 };
        int osc_write_idx = 0;

        // PolyBLEP anti-aliasing residual
        static inline float poly_blep(float t, float dt) {
            if (t < dt) {
                t /= dt;
                return t + t - t * t - 1.0f;
            } else if (t > 1.0f - dt) {
                t = (t - 1.0f) / dt;
                return t * t + t + t + 1.0f;
            }
            return 0.0f;
        }

        // 4-Pole Moog Ladder Filter
        inline float processMoogLadder(float in, float cutoff, float res, VoiceMoogState& st) {
            float f = 2.0f * cutoff / sample_rate;
            f = std::clamp(f, 0.001f, 0.95f);
            float k = 3.6f * f - 1.6f * f * f - 1.0f;
            float p = (k + 1.0f) * 0.5f;
            float scale = std::exp((1.0f - p) * 1.386249f);
            float r = res * scale;

            float x = in - r * st.y4;
            x = std::tanh(x * 1.15f); // Saturação não-linear do circuito

            st.y1 = x * p + st.oldx * p - k * st.y1;
            st.y2 = st.y1 * p + st.oldy1 * p - k * st.y2;
            st.y3 = st.y2 * p + st.oldy2 * p - k * st.y3;
            st.y4 = st.y3 * p + st.oldy3 * p - k * st.y4;

            st.oldx = x; st.oldy1 = st.y1; st.oldy2 = st.y2; st.oldy3 = st.y3;
            return st.y4;
        }

    public:
        PsytranceRollingBassSynth(const std::string& id) 
            : MpeSynthNode(id, "Kuro Psytrance Rolling Bass") {
            voices.resize(8);
        }

        void setSynthModel(int model) { synth_model = std::clamp(model, 0, 4); }
        int getSynthModel() const { return synth_model; }

        void setOscType(int type) { osc_type = std::clamp(type, 0, 3); }
        int getOscType() const { return osc_type; }

        void setPhaseRetrigger(float deg) { phase_retrigger_deg = std::clamp(deg, 0.0f, 360.0f); }
        float getPhaseRetrigger() const { return phase_retrigger_deg; }

        void setCutoff(float hz) { cutoff_target = std::clamp(hz, 20.0f, 16000.0f); }
        float getCutoff() const { return cutoff_target; }

        void setResonance(float r) { resonance_target = std::clamp(r, 0.0f, 0.95f); }
        float getResonance() const { return resonance_target; }

        void setEnvDecay(float sec) { env_decay_target = std::clamp(sec, 0.01f, 0.5f); }
        float getEnvDecay() const { return env_decay_target; }

        void setEnvAmount(float amt) { env_amount_target = std::clamp(amt, 0.0f, 1.0f); }
        float getEnvAmount() const { return env_amount_target; }

        void setDrive(float d) { drive_target = std::clamp(d, 0.0f, 1.0f); }
        float getDrive() const { return drive_target; }

        void setSubLevel(float s) { sub_target = std::clamp(s, 0.0f, 1.0f); }
        float getSubLevel() const { return sub_target; }

        void setTransientClick(float c) { click_target = std::clamp(c, 0.0f, 1.0f); }
        float getTransientClick() const { return click_target; }

        void setPitchDepth(float st) { pitch_depth_target = std::clamp(st, 0.0f, 36.0f); }
        float getPitchDepth() const { return pitch_depth_target; }

        void setPitchDecay(float sec) { pitch_decay_target = std::clamp(sec, 0.001f, 0.025f); }
        float getPitchDecay() const { return pitch_decay_target; }

        void setHpfCutoff(float hz) { hpf_cutoff_target = std::clamp(hz, 15.0f, 80.0f); }
        float getHpfCutoff() const { return hpf_cutoff_target; }

        // ── CONTROLES DO KICK DRUM INTEGRADO ──
        void setKickStartFreq(float f) { kick_start_hz = std::clamp(f, 100.0f, 400.0f); }
        float getKickStartFreq() const { return kick_start_hz; }

        void setKickEndFreq(float f) { kick_end_hz = std::clamp(f, 30.0f, 95.0f); }
        float getKickEndFreq() const { return kick_end_hz; }

        void setKickPitchDecay(float ms) { kick_pitch_decay_ms = std::clamp(ms, 15.0f, 100.0f); }
        float getKickPitchDecay() const { return kick_pitch_decay_ms; }

        void setKickAmpDecay(float ms) { kick_amp_decay_ms = std::clamp(ms, 80.0f, 450.0f); }
        float getKickAmpDecay() const { return kick_amp_decay_ms; }

        void setKickClick(float c) { kick_click = std::clamp(c, 0.0f, 1.0f); }
        float getKickClick() const { return kick_click; }

        void setKickDrive(float d) { kick_drive = std::clamp(d, 0.0f, 1.0f); }
        float getKickDrive() const { return kick_drive; }

        void setKickVolume(float v) { kick_vol = std::clamp(v, 0.0f, 1.5f); }
        float getKickVolume() const { return kick_vol; }

        void setKickPhaseAlign(float deg) { kick_phase_align = std::clamp(deg, 0.0f, 360.0f); }
        float getKickPhaseAlign() const { return kick_phase_align; }

        void setKickEnabled(bool b) { kick_enabled = b; }
        bool isKickEnabled() const { return kick_enabled; }

        void triggerKick(float velocity = 1.0f) {
            kick_active = true;
            kick_time = 0.0f;
            kick_phase = kick_phase_align / 360.0f;
            kick_velocity = std::clamp(velocity, 0.1f, 1.0f);
        }

        // Afinação inteligente do bumbo para a tônica da música
        void tuneKickToRootNote(int root_idx) {
            static const float root_sub_freqs[12] = {
                32.7f, 34.6f, 36.7f, 38.9f, 41.2f, 43.6f,
                46.2f, 49.0f, 51.9f, 55.0f, 58.3f, 61.7f
            };
            if (root_idx >= 0 && root_idx < 12) {
                kick_end_hz = root_sub_freqs[root_idx];
            }
        }

        // ── CONTROLES DO PSY SNARE / CLAP INTEGRADO ──
        void setSnareTone(float f) { snare_tone_hz = std::clamp(f, 120.0f, 350.0f); }
        float getSnareTone() const { return snare_tone_hz; }

        void setSnareNoiseDecay(float ms) { snare_noise_decay_ms = std::clamp(ms, 50.0f, 350.0f); }
        float getSnareNoiseDecay() const { return snare_noise_decay_ms; }

        void setSnareSnap(float s) { snare_snap = std::clamp(s, 0.0f, 1.0f); }
        float getSnareSnap() const { return snare_snap; }

        void setSnareVolume(float v) { snare_vol = std::clamp(v, 0.0f, 1.5f); }
        float getSnareVolume() const { return snare_vol; }

        void setSnareEnabled(bool b) { snare_enabled = b; }
        bool isSnareEnabled() const { return snare_enabled; }

        void triggerSnare(float velocity = 1.0f) {
            snare_active = true;
            snare_time = 0.0f;
            snare_phase = 0.0f;
            snare_velocity = std::clamp(velocity, 0.1f, 1.0f);
            snare_bpf1 = 0.0f;
            snare_bpf2 = 0.0f;
        }

        void stopAllDrums() {
            kick_active = false;
            snare_active = false;
        }

        // Síntese imediata da forma de onda quando em repouso (reflete 100% dos botões e knobs em tempo real)
        void getWaveformPreview(float* out_data, int size, int preview_mode = 0) {
            if (preview_mode == 1) {
                // ── PREVIEW DO KICK DRUM ──
                for (int i = 0; i < size; ++i) {
                    float t = (float)i / (float)size;
                    float cur_f = kick_end_hz + (kick_start_hz - kick_end_hz) * std::exp(-t * (800.0f / kick_pitch_decay_ms));
                    float amp = std::exp(-t * (1000.0f / kick_amp_decay_ms));
                    float click_layer = (t < 0.04f) ? (1.0f - t / 0.04f) * kick_click : 0.0f;
                    float kick_val = std::sin(t * cur_f * 0.38f + kick_phase_align * 0.01745f) * amp + click_layer;
                    kick_val = std::tanh(kick_val * (1.0f + kick_drive * 1.6f));
                    out_data[i] = std::clamp(kick_val * 0.90f, -0.98f, 0.98f);
                }
                return;
            } else if (preview_mode == 2) {
                // ── PREVIEW DO PSY SNARE / CLAP ──
                for (int i = 0; i < size; ++i) {
                    float t = (float)i / (float)size;
                    float tone = std::sin(t * snare_tone_hz * 0.35f) * std::exp(-t * 22.0f) * 0.5f;
                    float noise = std::sin(t * 3200.0f) * std::cos(t * 1800.0f) * std::exp(-t * (1000.0f / snare_noise_decay_ms));
                    float snap = (t < 0.04f) ? (1.0f - t / 0.04f) * snare_snap : 0.0f;
                    float snare_val = std::tanh((tone + noise * 0.8f + snap * 0.7f) * 1.2f);
                    out_data[i] = std::clamp(snare_val * 0.88f, -0.98f, 0.98f);
                }
                return;
            } else if (preview_mode == 3) {
                // ── PREVIEW DO FULL K&B (KICK + ROLLING BASS JUNTOS) ──
                int half = size / 2;
                for (int i = 0; i < size; ++i) {
                    if (i < half) {
                        float t = (float)i / (float)half;
                        float cur_f = kick_end_hz + (kick_start_hz - kick_end_hz) * std::exp(-t * (800.0f / kick_pitch_decay_ms));
                        float amp = std::exp(-t * (1000.0f / kick_amp_decay_ms));
                        float click_layer = (t < 0.05f) ? (1.0f - t / 0.05f) * kick_click : 0.0f;
                        float k = std::tanh((std::sin(t * cur_f * 0.35f) * amp + click_layer) * (1.0f + kick_drive * 1.5f));
                        out_data[i] = std::clamp(k * 0.90f, -0.98f, 0.98f);
                    } else {
                        float t = (float)(i - half) / (float)half;
                        float ph = std::fmod(t * 2.0f, 1.0f);
                        float b = (1.0f - ph * 2.0f) * 0.7f + std::sin(ph * 6.283185f * 0.5f) * 0.3f;
                        out_data[i] = std::clamp(b * 0.85f, -0.98f, 0.98f);
                    }
                }
                return;
            }

            // ── PREVIEW DO ROLLING BASS (PADRÃO) ──
            float phase_start = phase_retrigger_deg / 360.0f;
            float filter_damp = std::clamp(cutoff_target / 3200.0f, 0.12f, 1.0f);
            float reso_bump = resonance_target * 0.42f;
            float sub_amt = sub_target * 0.45f;
            float click_amt = click_target * 0.35f;

            for (int i = 0; i < size; ++i) {
                float t = (float)i / (float)size; // 2 ciclos para visualização clara
                float ph = std::fmod(phase_start + t * 2.0f, 1.0f);
                float ph_sub = std::fmod((phase_start * 0.5f) + t * 1.0f, 1.0f);

                float raw = 0.0f;
                if (osc_type == 0) { // Saw
                    raw = (1.0f - ph * 2.0f);
                    raw = raw * filter_damp + std::sin(ph * 6.283185f) * (1.0f - filter_damp);
                } else if (osc_type == 1) { // Square
                    raw = (ph < 0.5f ? 0.85f : -0.85f);
                    raw = raw * filter_damp + std::sin(ph * 6.283185f) * (1.0f - filter_damp);
                } else if (osc_type == 2) { // Sub-sine
                    raw = std::sin(ph * 6.283185f);
                } else { // Morph
                    float saw_part = (1.0f - ph * 2.0f);
                    float sq_part = (ph < 0.5f ? 0.85f : -0.85f);
                    raw = 0.6f * saw_part + 0.4f * sq_part;
                }

                float sub_w = std::sin(ph_sub * 6.283185f) * sub_amt;
                float ring = std::sin(ph * 6.283185f * (3.0f + resonance_target * 4.5f)) * reso_bump * std::exp(-ph * 3.5f);
                float click_w = 0.0f;
                if (t < 0.08f) {
                    click_w = std::sin(t * 150.0f) * (1.0f - t / 0.08f) * click_amt;
                }

                float combined = (raw * 0.72f + sub_w + ring + click_w);
                if (synth_model == 1) { // Virus TI Hyper-Saw
                    combined += 0.22f * (1.0f - std::fmod(ph + 0.05f, 1.0f) * 2.0f);
                } else if (synth_model == 2) { // Nord Lead Punch
                    combined = std::tanh(combined * 1.5f);
                } else if (synth_model == 3) { // FM Sub-Rolling
                    combined = std::sin(ph * 6.283185f + 1.1f * std::sin(ph * 12.56637f));
                } else if (synth_model == 4) { // Analog Monster
                    combined = 0.5f * combined + 0.5f * std::sin(ph * 6.283185f);
                }

                out_data[i] = std::clamp(combined * 0.82f, -0.98f, 0.98f);
            }
        }

        void getOscilloscopeBuffer(float* out_data, int size, int preview_mode = 0) {
            // Analisa pico de energia no buffer recente
            float peak_lvl = 0.0f;
            int scan_len = std::min(size * 4, OSC_BUF_SIZE);
            int start_scan = (osc_write_idx - scan_len + OSC_BUF_SIZE) % OSC_BUF_SIZE;
            for (int i = 0; i < scan_len; ++i) {
                float v = std::abs(osc_buffer[(start_scan + i) % OSC_BUF_SIZE]);
                if (v > peak_lvl) peak_lvl = v;
            }

            if (peak_lvl > 0.015f) {
                // Áudio ativo: Busca por zero-crossing ascendente para travar a fase (Trigger Sync)
                int trigger_idx = (osc_write_idx - size * 2 + OSC_BUF_SIZE) % OSC_BUF_SIZE;
                for (int i = 0; i < size; ++i) {
                    int i0 = (trigger_idx + i) % OSC_BUF_SIZE;
                    int i1 = (i0 + 1) % OSC_BUF_SIZE;
                    if (osc_buffer[i0] <= 0.0f && osc_buffer[i1] > 0.0f) {
                        trigger_idx = i1;
                        break;
                    }
                }
                for (int i = 0; i < size; ++i) {
                    out_data[i] = osc_buffer[(trigger_idx + i) % OSC_BUF_SIZE];
                }
            } else {
                // Em repouso: forma de onda viva em tempo real dos parâmetros atuais
                getWaveformPreview(out_data, size, preview_mode);
            }
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
                    // Alocar voz (monofônico prioritário para bassline com retrigger)
                    Voice* target_voice = nullptr;
                    for (auto& v : voices) {
                        if (!v.active) { target_voice = &v; break; }
                    }
                    if (!target_voice) target_voice = &voices[0];

                    target_voice->note_id = m_ev.note_id;
                    target_voice->key = m_ev.key;
                    target_voice->freq = getFrequency(m_ev.key, m_ev.pitch_bend);
                    // Phase lock retrigger exato
                    target_voice->phase = phase_retrigger_deg / 360.0f;
                    target_voice->sub_phase = target_voice->phase * 0.5f;
                    target_voice->velocity = m_ev.velocity;
                    target_voice->env_filter = 1.0f;
                    target_voice->env_amp = 1.0f;
                    target_voice->env_pitch = 1.0f;
                    target_voice->time_alive = 0.0f;
                    target_voice->click_time = 0.003f; // 3ms click de ataque
                    target_voice->active = true;
                    target_voice->note_on = true;
                    target_voice->filter_state = VoiceMoogState(); // Reset de estados
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
                switch (p_ev.param_index) {
                    case 0: osc_type = (int)p_ev.target_value; break;
                    case 1: phase_retrigger_deg = p_ev.target_value; break;
                    case 2: click_target = p_ev.target_value; break;
                    case 3: cutoff_target = p_ev.target_value; break;
                    case 4: resonance_target = p_ev.target_value; break;
                    case 5: env_decay_target = p_ev.target_value; break;
                    case 6: env_amount_target = p_ev.target_value; break;
                    case 7: drive_target = p_ev.target_value; break;
                    case 8: sub_target = p_ev.target_value; break;
                    case 9: pitch_depth_target = p_ev.target_value; break;
                    case 10: pitch_decay_target = p_ev.target_value; break;
                    case 11: hpf_cutoff_target = p_ev.target_value; break;
                }
            }

            bool any_active = false;
            for (const auto& v : voices) {
                if (v.active) { any_active = true; break; }
            }
            if (!any_active) return;

            float dt = 1.0f / sample_rate;

            // Coeficientes do High-Pass Filter Butterworth 24dB (2 estágios)
            float w0 = 2.0f * 3.14159265f * hpf_cutoff_hz / sample_rate;
            w0 = std::clamp(w0, 0.0005f, 0.35f);
            float cos_w = std::cos(w0);
            float sin_w = std::sin(w0);
            float alpha = sin_w / (2.0f * 0.70710678f);
            float a0 = 1.0f + alpha;
            float b0 = ((1.0f + cos_w) * 0.5f) / a0;
            float b1 = (-(1.0f + cos_w)) / a0;
            float b2 = ((1.0f + cos_w) * 0.5f) / a0;
            float a1 = (-2.0f * cos_w) / a0;
            float a2 = (1.0f - alpha) / a0;

            for (unsigned int i = 0; i < frames; ++i) {
                // Interpolação suave de parâmetros
                cutoff_hz = lerp(cutoff_hz, cutoff_target, 0.02f);
                resonance = lerp(resonance, resonance_target, 0.02f);
                env_decay_sec = lerp(env_decay_sec, env_decay_target, 0.02f);
                env_amount = lerp(env_amount, env_amount_target, 0.02f);
                drive_amt = lerp(drive_amt, drive_target, 0.02f);
                sub_level = lerp(sub_level, sub_target, 0.02f);
                transient_click = lerp(transient_click, click_target, 0.02f);
                pitch_depth_semitones = lerp(pitch_depth_semitones, pitch_depth_target, 0.02f);
                pitch_decay_sec = lerp(pitch_decay_sec, pitch_decay_target, 0.02f);
                hpf_cutoff_hz = lerp(hpf_cutoff_hz, hpf_cutoff_target, 0.02f);

                float mix = 0.0f;

                for (auto& v : voices) {
                    if (!v.active) continue;

                    // Envelope de decaimento do filtro ultra-rápido logarítmico (Psytrance Pluck)
                    float decay_coeff = std::exp(-dt / std::max(0.005f, env_decay_sec));
                    v.env_filter *= decay_coeff;

                    // Envelope de amplitude
                    if (!v.note_on) {
                        v.env_amp *= std::exp(-dt / 0.015f); // Release seco de 15ms
                        if (v.env_amp < 0.001f) {
                            v.active = false;
                            continue;
                        }
                    }

                    // Envelope de afinação exponencial ultrarrápido (Laser Pitch Punch)
                    float pitch_decay_coeff = std::exp(-dt / std::max(0.0008f, pitch_decay_sec));
                    v.env_pitch *= pitch_decay_coeff;
                    float pitch_mult = std::pow(2.0f, (v.env_pitch * pitch_depth_semitones) / 12.0f);
                    float current_freq = v.freq * pitch_mult;

                    // Frequência modulada por pitch punch
                    float dt_phase = current_freq / sample_rate;
                    v.phase += dt_phase;
                    if (v.phase >= 1.0f) v.phase -= 1.0f;

                    v.sub_phase += (current_freq * 0.5f) / sample_rate;
                    if (v.sub_phase >= 1.0f) v.sub_phase -= 1.0f;

                    // Geração de onda anti-aliased (PolyBLEP)
                    float raw_saw = 2.0f * v.phase - 1.0f;
                    float blep_saw = raw_saw - poly_blep(v.phase, dt_phase);

                    float raw_sq = (v.phase < 0.5f) ? 1.0f : -1.0f;
                    float blep_sq = raw_sq + poly_blep(v.phase, dt_phase) - poly_blep(std::fmod(v.phase + 0.5f, 1.0f), dt_phase);

                    float osc_out = 0.0f;
                    if (osc_type == 0) osc_out = blep_saw;
                    else if (osc_type == 1) osc_out = blep_sq;
                    else if (osc_type == 2) osc_out = std::sin(KURO_TWO_PI * v.phase);
                    else osc_out = 0.7f * blep_saw + 0.3f * blep_sq; // Morph Saw/Square

                    // Timbre específico do modelo de sintetizador selecionado
                    if (synth_model == 1) { // Virus TI Hyper-Saw (Detuned dual saw)
                        float ph2 = std::fmod(v.phase + 0.04f, 1.0f);
                        float blep_saw2 = (2.0f * ph2 - 1.0f) - poly_blep(ph2, dt_phase);
                        osc_out = 0.65f * osc_out + 0.35f * blep_saw2;
                    } else if (synth_model == 2) { // Nord Lead Punch 303 (Saturação cortante)
                        osc_out = std::tanh(osc_out * 1.5f);
                    } else if (synth_model == 3) { // FM Sub-Rolling Bass (Modulação de fase FM)
                        float mod = std::sin(KURO_TWO_PI * v.phase * 2.0f) * 0.8f;
                        osc_out = std::sin(KURO_TWO_PI * v.phase + mod);
                    } else if (synth_model == 4) { // Analog Monster Sub (Mistura densa de pulso e sub)
                        osc_out = 0.55f * blep_saw + 0.45f * blep_sq;
                    }

                    // Sub-oscilador (Sub-grave puro em oitava abaixo)
                    float sub_sig = std::sin(KURO_TWO_PI * v.sub_phase) * sub_level;

                    // Transient Click (Estalo cirúrgico de transiente no ataque)
                    float click_sig = 0.0f;
                    if (v.click_time > 0.0f) {
                        float click_phase = (0.003f - v.click_time) / 0.003f;
                        click_sig = std::sin(KURO_TWO_PI * 3500.0f * v.time_alive) * (1.0f - click_phase) * transient_click;
                        v.click_time -= dt;
                    }

                    float raw_synth = (osc_out * 0.8f + sub_sig * 0.6f + click_sig * 0.5f);

                    // Cutoff modulado pelo envelope do filtro
                    float dynamic_cutoff = cutoff_hz + env_amount * 4500.0f * (v.env_filter * v.env_filter);
                    dynamic_cutoff = std::clamp(dynamic_cutoff, 20.0f, 18000.0f);

                    // Passar pelo Moog Ladder 24dB
                    float filtered = processMoogLadder(raw_synth, dynamic_cutoff, resonance, v.filter_state);

                    // Saturação de fita / Drive de graves
                    if (drive_amt > 0.01f) {
                        float drive_gain = 1.0f + drive_amt * 4.0f;
                        filtered = std::tanh(filtered * drive_gain) / std::sqrt(drive_gain);
                    }

                    mix += filtered * v.velocity * v.env_amp * master_vol;
                    v.time_alive += dt;
                }

                // Passar pelo High-Pass Filter 24dB (Limpeza cirúrgica de Sub-Low Cut)
                float hpf_out = hpf_stage1.process(mix, b0, b1, b2, a1, a2);
                hpf_out = hpf_stage2.process(hpf_out, b0, b1, b2, a1, a2);

                // ── MOTOR DE SÍNTESE DO KICK DRUM INTEGRADO ──
                float kick_out = 0.0f;
                if (kick_active && kick_enabled) {
                    float cur_k_freq = kick_end_hz + (kick_start_hz - kick_end_hz) * std::exp(-kick_time * (800.0f / (kick_pitch_decay_ms * 0.001f * sample_rate)));
                    kick_phase += cur_k_freq / sample_rate;
                    if (kick_phase >= 1.0f) kick_phase -= 1.0f;

                    float k_amp = std::exp(-kick_time / (kick_amp_decay_ms * 0.001f * sample_rate));
                    float k_click_amp = (kick_time < 0.004f * sample_rate) ? (1.0f - kick_time / (0.004f * sample_rate)) * kick_click : 0.0f;
                    float k_click_val = std::sin(KURO_TWO_PI * 4200.0f * (kick_time / sample_rate)) * k_click_amp;

                    float raw_kick = std::sin(KURO_TWO_PI * kick_phase) * k_amp + k_click_val;
                    kick_out = std::tanh(raw_kick * (1.0f + kick_drive * 1.8f)) * kick_velocity * kick_vol;

                    kick_time += 1.0f;
                    if (k_amp < 0.001f && kick_time > 0.08f * sample_rate) {
                        kick_active = false;
                    }
                }

                // ── MOTOR DE SÍNTESE DO PSY SNARE / CLAP INTEGRADO ──
                float snare_out = 0.0f;
                if (snare_active && snare_enabled) {
                    float cur_snare_freq = snare_tone_hz * std::exp(-snare_time / (0.025f * sample_rate));
                    snare_phase += cur_snare_freq / sample_rate;
                    if (snare_phase >= 1.0f) snare_phase -= 1.0f;
                    float tone_body = std::sin(KURO_TWO_PI * snare_phase) * std::exp(-snare_time / (0.060f * sample_rate)) * 0.5f;

                    float noise_env = std::exp(-snare_time / (snare_noise_decay_ms * 0.001f * sample_rate));
                    float white_noise = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * noise_env;
                    snare_bpf1 += 0.28f * (white_noise - snare_bpf1);
                    snare_bpf2 += 0.28f * (snare_bpf1 - snare_bpf2);
                    float filtered_noise = (snare_bpf1 - snare_bpf2) * 2.2f;

                    float snap_layer = (snare_time < 0.003f * sample_rate) ? (1.0f - snare_time / (0.003f * sample_rate)) * snare_snap : 0.0f;
                    snare_out = std::tanh((tone_body + filtered_noise * 0.85f + snap_layer * 0.6f) * 1.2f) * snare_velocity * snare_vol;

                    snare_time += 1.0f;
                    if (noise_env < 0.001f && snare_time > 0.08f * sample_rate) {
                        snare_active = false;
                    }
                }

                float final_mix = hpf_out + kick_out + snare_out;

                left[i] += final_mix;
                right[i] += final_mix;

                // Alimentar buffer do osciloscópio com sinal final
                osc_buffer[osc_write_idx] = final_mix;
                osc_write_idx = (osc_write_idx + 1) % OSC_BUF_SIZE;
            }
        }

        void renderCustomUI() override {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "KURO PSYTRANCE ROLLING BASS ENGINE");
            ImGui::Separator();
            
            const char* osc_names[] = { "Psy Sawtooth (Nord)", "Square / Pulse", "Sub Sine Pure", "Morph Saw-Square" };
            int cur_osc = osc_type;
            if (ImGui::Combo("Forma de Onda", &cur_osc, osc_names, IM_ARRAYSIZE(osc_names))) {
                setOscType(cur_osc);
            }

            float p_deg = phase_retrigger_deg;
            if (ImGui::SliderFloat("Phase Retrigger", &p_deg, 0.0f, 360.0f, "%.0f deg")) {
                setPhaseRetrigger(p_deg);
            }

            float c_hz = cutoff_target, r_val = resonance_target, d_sec = env_decay_target, amt_val = env_amount_target;
            if (ImGui::SliderFloat("Filtro Cutoff", &c_hz, 30.0f, 6000.0f, "%.0f Hz")) setCutoff(c_hz);
            if (ImGui::SliderFloat("Ressonancia Moog", &r_val, 0.0f, 0.95f)) setResonance(r_val);
            if (ImGui::SliderFloat("Decay do Pluck", &d_sec, 0.02f, 0.35f, "%.3f s")) setEnvDecay(d_sec);
            if (ImGui::SliderFloat("Envelope Depth", &amt_val, 0.0f, 1.0f)) setEnvAmount(amt_val);

            float drv = drive_target, sub = sub_target, clk = click_target;
            if (ImGui::SliderFloat("Drive / Saturação", &drv, 0.0f, 1.0f)) setDrive(drv);
            if (ImGui::SliderFloat("Sub Harmonic Level", &sub, 0.0f, 1.0f)) setSubLevel(sub);
            if (ImGui::SliderFloat("Transient Click", &clk, 0.0f, 1.0f)) setTransientClick(clk);

            float p_depth = pitch_depth_target, p_decay = pitch_decay_target, hpf_cut = hpf_cutoff_target;
            if (ImGui::SliderFloat("Laser Pitch Depth", &p_depth, 0.0f, 36.0f, "%.1f st")) setPitchDepth(p_depth);
            if (ImGui::SliderFloat("Pitch Decay", &p_decay, 0.001f, 0.025f, "%.4f s")) setPitchDecay(p_decay);
            if (ImGui::SliderFloat("30Hz Sub Low-Cut", &hpf_cut, 15.0f, 80.0f, "%.0f Hz")) setHpfCutoff(hpf_cut);
        }
    };
}


