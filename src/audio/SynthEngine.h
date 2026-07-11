#pragma once
#include <vector>
#include <cmath>
#include <mutex>
#include <algorithm>
#include <string>

namespace KuroAudio {

    struct MidiNote {
        int pitch;          // MIDI pitch (0-127)
        float start_time;   // Start time in seconds
        float duration;     // Duration in seconds
        float velocity;     // 0.0 to 1.0
        bool is_playing;
        
        MidiNote() : pitch(60), start_time(0.0f), duration(1.0f), velocity(0.8f), is_playing(false) {}
        MidiNote(int p, float st, float dur, float vel = 0.8f) 
            : pitch(p), start_time(st), duration(dur), velocity(vel), is_playing(false) {}
    };

    // Instrumentos MIDI disponíveis no Piano Roll
    enum class MidiInstrument {
        ACOUSTIC_PIANO,     // 0  - Piano Acústico (GM 1)
        ELECTRIC_PIANO,     // 1  - Piano Elétrico (GM 5 - Rhodes)
        HARPSICHORD,        // 2  - Cravo (GM 7)
        CELESTA,            // 3  - Celesta (GM 9)
        VIBRAPHONE,         // 4  - Vibrafone (GM 12)
        MARIMBA,            // 5  - Marimba (GM 13)
        CHURCH_ORGAN,       // 6  - Órgão de Igreja (GM 20)
        ACCORDION,          // 7  - Acordeão (GM 22)
        NYLON_GUITAR,       // 8  - Violão Nylon (GM 25)
        STEEL_GUITAR,       // 9  - Violão Aço (GM 26)
        ELECTRIC_BASS,      // 10 - Baixo Elétrico (GM 34)
        VIOLIN,             // 11 - Violino (GM 41)
        CELLO,              // 12 - Violoncelo (GM 43)
        STRING_ENSEMBLE,    // 13 - Ensemble de Cordas (GM 49)
        CHOIR,              // 14 - Coro (GM 53)
        FLUTE,              // 15 - Flauta (GM 74)
        COUNT
    };

    inline const char* getMidiInstrumentName(MidiInstrument inst) {
        switch (inst) {
            case MidiInstrument::ACOUSTIC_PIANO:   return "Acoustic Piano";
            case MidiInstrument::ELECTRIC_PIANO:    return "Electric Piano";
            case MidiInstrument::HARPSICHORD:       return "Harpsichord";
            case MidiInstrument::CELESTA:           return "Celesta";
            case MidiInstrument::VIBRAPHONE:        return "Vibraphone";
            case MidiInstrument::MARIMBA:           return "Marimba";
            case MidiInstrument::CHURCH_ORGAN:      return "Church Organ";
            case MidiInstrument::ACCORDION:         return "Accordion";
            case MidiInstrument::NYLON_GUITAR:      return "Nylon Guitar";
            case MidiInstrument::STEEL_GUITAR:      return "Steel Guitar";
            case MidiInstrument::ELECTRIC_BASS:     return "Electric Bass";
            case MidiInstrument::VIOLIN:            return "Violin";
            case MidiInstrument::CELLO:             return "Cello";
            case MidiInstrument::STRING_ENSEMBLE:   return "String Ensemble";
            case MidiInstrument::CHOIR:             return "Choir (Aah)";
            case MidiInstrument::FLUTE:             return "Flute";
            default: return "Unknown";
        }
    }

    // Tipos de onda básicos (mantidos para compatibilidade)
    enum class SynthType { SINE, SAWTOOTH, SQUARE, PIANO };

    class SynthEngine {
    private:
        float sample_rate;
        SynthType current_type;
        std::vector<MidiNote> notes;
        std::mutex synth_mutex;
        
        struct Voice {
            int pitch;
            float phase;
            float current_time;
            float duration;
            float velocity;
            bool active;
        };
        std::vector<Voice> active_voices;

        float getFrequency(int pitch) {
            return 440.0f * std::pow(2.0f, (pitch - 69) / 12.0f);
        }

        static constexpr float KURO_PI = 3.1415926535f;
        static constexpr float KURO_TWO_PI = 6.2831853071f;

        // ========== SÍNTESE POR INSTRUMENTO ==========
        
        // Gera uma amostra para um instrumento no tempo t, frequência freq, phase
        float synthesize(MidiInstrument inst, float freq, float t, float phase, float duration) {
            float wave = 0.0f;
            float env = 1.0f;

            switch (inst) {

            // --- ACOUSTIC PIANO: Corda percutida com harmônicos naturais e decaimento ---
            case MidiInstrument::ACOUSTIC_PIANO: {
                // Fundamental + 4 harmônicos com decaimento progressivo
                wave  = std::sin(KURO_TWO_PI * freq * t);
                wave += 0.50f * std::sin(KURO_TWO_PI * freq * 2.0f * t) * std::exp(-1.0f * t);
                wave += 0.25f * std::sin(KURO_TWO_PI * freq * 3.0f * t) * std::exp(-2.0f * t);
                wave += 0.12f * std::sin(KURO_TWO_PI * freq * 4.0f * t) * std::exp(-3.0f * t);
                wave += 0.06f * std::sin(KURO_TWO_PI * freq * 5.0f * t) * std::exp(-4.0f * t);
                
                // Ataque do martelo (click percussivo)
                wave += 0.3f * std::sin(KURO_TWO_PI * freq * 8.0f * t) * std::exp(-40.0f * t);
                
                env = std::exp(-1.5f * t);
                if (t > duration) env *= std::exp(-12.0f * (t - duration));
                break;
            }

            // --- ELECTRIC PIANO: FM Synthesis (Rhodes-style) ---
            case MidiInstrument::ELECTRIC_PIANO: {
                float mod_index = 3.0f * std::exp(-5.0f * t);
                float modulator = std::sin(KURO_TWO_PI * freq * 2.0f * t) * mod_index;
                wave = std::sin(KURO_TWO_PI * freq * t + modulator);
                
                // Tine (ataque metálico)
                wave += 0.4f * std::sin(KURO_TWO_PI * freq * 7.0f * t) * std::exp(-30.0f * t);
                
                env = std::exp(-2.5f * t);
                if (t > duration) env *= std::exp(-10.0f * (t - duration));
                break;
            }

            // --- HARPSICHORD: Ataque rápido, muitos harmônicos, sem sustain ---
            case MidiInstrument::HARPSICHORD: {
                // Onda com muitos harmônicos (quase quadrada mas assimétrica)
                wave  = std::sin(KURO_TWO_PI * freq * t);
                wave += 0.7f * std::sin(KURO_TWO_PI * freq * 2.0f * t);
                wave += 0.5f * std::sin(KURO_TWO_PI * freq * 3.0f * t);
                wave += 0.3f * std::sin(KURO_TWO_PI * freq * 4.0f * t);
                wave += 0.2f * std::sin(KURO_TWO_PI * freq * 5.0f * t);
                wave += 0.15f * std::sin(KURO_TWO_PI * freq * 6.0f * t);
                
                // Pluck rápido
                env = std::exp(-5.0f * t);
                if (t > duration) env *= std::exp(-20.0f * (t - duration));
                break;
            }

            // --- CELESTA: Tom puro brilhante com harmônicos cristalinos ---
            case MidiInstrument::CELESTA: {
                wave = std::sin(KURO_TWO_PI * freq * t);
                wave += 0.6f * std::sin(KURO_TWO_PI * freq * 3.0f * t) * std::exp(-3.0f * t);
                wave += 0.3f * std::sin(KURO_TWO_PI * freq * 5.0f * t) * std::exp(-5.0f * t);
                
                env = std::exp(-4.0f * t);
                if (t > duration) env *= std::exp(-15.0f * (t - duration));
                break;
            }

            // --- VIBRAPHONE: Senoide com tremolo (vibrato de amplitude) ---
            case MidiInstrument::VIBRAPHONE: {
                wave = std::sin(KURO_TWO_PI * freq * t);
                wave += 0.3f * std::sin(KURO_TWO_PI * freq * 4.0f * t) * std::exp(-4.0f * t);
                
                // Tremolo (motor do vibrafone girando as palhetas)
                float tremolo = 1.0f + 0.3f * std::sin(KURO_TWO_PI * 5.5f * t);
                wave *= tremolo;
                
                env = std::exp(-1.0f * t);
                if (t > duration) env *= std::exp(-8.0f * (t - duration));
                break;
            }

            // --- MARIMBA: Ataque seco percussivo, fundamental forte ---
            case MidiInstrument::MARIMBA: {
                wave = std::sin(KURO_TWO_PI * freq * t);
                wave += 0.15f * std::sin(KURO_TWO_PI * freq * 4.0f * t) * std::exp(-10.0f * t);
                
                // Click da baqueta
                wave += 0.5f * std::sin(KURO_TWO_PI * freq * 10.0f * t) * std::exp(-50.0f * t);
                
                env = std::exp(-4.5f * t);
                if (t > duration) env *= std::exp(-20.0f * (t - duration));
                break;
            }

            // --- CHURCH ORGAN: Drawbar organ (Hammond-style com harmônicos constantes) ---
            case MidiInstrument::CHURCH_ORGAN: {
                // Drawbars: 16' 8' 5 1/3' 4' 2 2/3' 2' 1 3/5' 1 1/3' 1'
                wave  = 0.8f * std::sin(KURO_TWO_PI * freq * 0.5f * t); // 16'
                wave += 1.0f * std::sin(KURO_TWO_PI * freq * t);          // 8'
                wave += 0.6f * std::sin(KURO_TWO_PI * freq * 1.5f * t);  // 5 1/3'
                wave += 0.8f * std::sin(KURO_TWO_PI * freq * 2.0f * t);  // 4'
                wave += 0.4f * std::sin(KURO_TWO_PI * freq * 3.0f * t);  // 2 2/3'
                wave += 0.5f * std::sin(KURO_TWO_PI * freq * 4.0f * t);  // 2'
                wave *= 0.25f; // Normaliza
                
                // Órgão não decai (sustain constante)
                env = 1.0f;
                float attack = 0.02f;
                if (t < attack) env = t / attack;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 15.0f);
                break;
            }

            // --- ACCORDION: Similar ao órgão mas com tremolo mais intenso ---
            case MidiInstrument::ACCORDION: {
                wave  = std::sin(KURO_TWO_PI * freq * t);
                wave += 0.6f * std::sin(KURO_TWO_PI * freq * 2.0f * t);
                wave += 0.3f * std::sin(KURO_TWO_PI * freq * 3.0f * t);
                wave += 0.15f * std::sin(KURO_TWO_PI * freq * 4.0f * t);
                
                // Tremolo da palheta vibrando
                float tremolo = 1.0f + 0.15f * std::sin(KURO_TWO_PI * 6.0f * t);
                wave *= tremolo * 0.4f;
                
                env = 1.0f;
                float attack = 0.03f;
                if (t < attack) env = t / attack;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 12.0f);
                break;
            }

            // --- NYLON GUITAR: Pluck suave, harmônicos pares fracos ---
            case MidiInstrument::NYLON_GUITAR: {
                wave  = std::sin(KURO_TWO_PI * freq * t);
                wave += 0.15f * std::sin(KURO_TWO_PI * freq * 2.0f * t); // Harmônico par fraco
                wave += 0.35f * std::sin(KURO_TWO_PI * freq * 3.0f * t);
                wave += 0.08f * std::sin(KURO_TWO_PI * freq * 4.0f * t);
                wave += 0.20f * std::sin(KURO_TWO_PI * freq * 5.0f * t) * std::exp(-3.0f * t);
                
                env = std::exp(-2.0f * t);
                if (t > duration) env *= std::exp(-10.0f * (t - duration));
                break;
            }

            // --- STEEL GUITAR: Pluck brilhante, mais harmônicos agudos ---
            case MidiInstrument::STEEL_GUITAR: {
                wave  = std::sin(KURO_TWO_PI * freq * t);
                wave += 0.45f * std::sin(KURO_TWO_PI * freq * 2.0f * t);
                wave += 0.35f * std::sin(KURO_TWO_PI * freq * 3.0f * t);
                wave += 0.25f * std::sin(KURO_TWO_PI * freq * 4.0f * t);
                wave += 0.15f * std::sin(KURO_TWO_PI * freq * 5.0f * t);
                wave += 0.10f * std::sin(KURO_TWO_PI * freq * 6.0f * t);
                
                // Pick attack
                wave += 0.3f * std::sin(KURO_TWO_PI * freq * 9.0f * t) * std::exp(-35.0f * t);
                
                env = std::exp(-2.5f * t);
                if (t > duration) env *= std::exp(-10.0f * (t - duration));
                break;
            }

            // --- ELECTRIC BASS: Fundamental potente, sub-harmônico ---
            case MidiInstrument::ELECTRIC_BASS: {
                wave  = 1.0f * std::sin(KURO_TWO_PI * freq * t);         // Fundamental forte
                wave += 0.5f * std::sin(KURO_TWO_PI * freq * 0.5f * t);  // Sub
                wave += 0.3f * std::sin(KURO_TWO_PI * freq * 2.0f * t);
                wave += 0.1f * std::sin(KURO_TWO_PI * freq * 3.0f * t);
                
                // Pick/finger attack
                wave += 0.4f * std::sin(KURO_TWO_PI * freq * 6.0f * t) * std::exp(-25.0f * t);
                wave *= 0.5f;
                
                env = std::exp(-1.5f * t);
                if (t > duration) env *= std::exp(-8.0f * (t - duration));
                break;
            }

            // --- VIOLIN: Onda rica (sawtooth filtrada) com vibrato ---
            case MidiInstrument::VIOLIN: {
                // Vibrato natural do violinista
                float vib = freq * (1.0f + 0.005f * std::sin(KURO_TWO_PI * 5.5f * t));
                
                // Sawtooth com harmônicos (corda friccionada pelo arco)
                wave  = std::sin(KURO_TWO_PI * vib * t);
                wave += 0.50f * std::sin(KURO_TWO_PI * vib * 2.0f * t);
                wave += 0.33f * std::sin(KURO_TWO_PI * vib * 3.0f * t);
                wave += 0.25f * std::sin(KURO_TWO_PI * vib * 4.0f * t);
                wave += 0.20f * std::sin(KURO_TWO_PI * vib * 5.0f * t);
                wave += 0.16f * std::sin(KURO_TWO_PI * vib * 6.0f * t);
                wave *= 0.35f;
                
                // Attack lento do arco
                float attack = 0.08f;
                env = 1.0f;
                if (t < attack) env = t / attack;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 10.0f);
                break;
            }

            // --- CELLO: Como violino mas mais grave e encorpado ---
            case MidiInstrument::CELLO: {
                float vib = freq * (1.0f + 0.004f * std::sin(KURO_TWO_PI * 5.0f * t));
                
                wave  = std::sin(KURO_TWO_PI * vib * t);
                wave += 0.60f * std::sin(KURO_TWO_PI * vib * 2.0f * t);
                wave += 0.40f * std::sin(KURO_TWO_PI * vib * 3.0f * t);
                wave += 0.30f * std::sin(KURO_TWO_PI * vib * 4.0f * t);
                wave += 0.20f * std::sin(KURO_TWO_PI * vib * 5.0f * t);
                wave *= 0.35f;
                
                float attack = 0.10f;
                env = 1.0f;
                if (t < attack) env = t / attack;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 8.0f);
                break;
            }

            // --- STRING ENSEMBLE: Múltiplas cordas com chorus e detuning ---
            case MidiInstrument::STRING_ENSEMBLE: {
                // 3 "instrumentos" levemente desafinados
                for (int s = -1; s <= 1; s++) {
                    float f = freq * (1.0f + s * 0.003f);
                    float vib = f * (1.0f + 0.003f * std::sin(KURO_TWO_PI * (4.5f + s * 0.3f) * t));
                    
                    wave += std::sin(KURO_TWO_PI * vib * t);
                    wave += 0.45f * std::sin(KURO_TWO_PI * vib * 2.0f * t);
                    wave += 0.25f * std::sin(KURO_TWO_PI * vib * 3.0f * t);
                }
                wave *= 0.15f;
                
                float attack = 0.15f;
                env = 1.0f;
                if (t < attack) env = t / attack;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 6.0f);
                break;
            }

            // --- CHOIR: Formantes vocais (Aah) ---
            case MidiInstrument::CHOIR: {
                // Simula formantes da vogal "Aah"
                // Formantes: F1~700Hz, F2~1200Hz, F3~2500Hz
                float f1 = 700.0f, f2 = 1200.0f, f3 = 2500.0f;
                float vib = freq * (1.0f + 0.004f * std::sin(KURO_TWO_PI * 5.0f * t));
                
                float base = 0.0f;
                // Gera harmônicos da frequência fundamental
                for (int h = 1; h <= 12; h++) {
                    float hf = vib * h;
                    // Formantes (ressonâncias vocais) amplificam certas faixas
                    float formant_gain = 0.2f;
                    formant_gain += 0.8f * std::exp(-std::pow((hf - f1) / 80.0f, 2.0f));
                    formant_gain += 0.5f * std::exp(-std::pow((hf - f2) / 100.0f, 2.0f));
                    formant_gain += 0.3f * std::exp(-std::pow((hf - f3) / 120.0f, 2.0f));
                    
                    base += formant_gain * std::sin(KURO_TWO_PI * hf * t) / h;
                }
                wave = base * 0.3f;
                
                float attack = 0.20f;
                env = 1.0f;
                if (t < attack) env = t / attack;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 5.0f);
                break;
            }

            // --- FLUTE: Senoide pura com sopro (ruído leve) ---
            case MidiInstrument::FLUTE: {
                float vib = freq * (1.0f + 0.003f * std::sin(KURO_TWO_PI * 5.0f * t));
                
                wave = std::sin(KURO_TWO_PI * vib * t);
                wave += 0.10f * std::sin(KURO_TWO_PI * vib * 2.0f * t); // Harmônico fraco
                wave += 0.05f * std::sin(KURO_TWO_PI * vib * 3.0f * t);
                
                // Ruído de sopro (breath noise) - usando uma pseudo-senoide alta
                float breath = 0.03f * std::sin(KURO_TWO_PI * 7919.0f * t + 3.7f * std::sin(KURO_TWO_PI * 113.0f * t));
                wave += breath;
                
                float attack = 0.05f;
                env = 1.0f;
                if (t < attack) env = t / attack;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 15.0f);
                break;
            }

            default:
                wave = std::sin(KURO_TWO_PI * freq * t);
                env = std::exp(-3.0f * t);
                break;
            }

            return wave * env;
        }

    public:
        MidiInstrument current_instrument = MidiInstrument::ACOUSTIC_PIANO;

        SynthEngine() : sample_rate(44100.0f), current_type(SynthType::PIANO) {}

        void setSampleRate(float sr) { sample_rate = sr; }
        void setSynthType(SynthType type) { current_type = type; }
        void setInstrument(MidiInstrument inst) { current_instrument = inst; }
        MidiInstrument getInstrument() const { return current_instrument; }

        void addNote(int pitch, float start, float duration, float velocity = 0.8f) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            notes.push_back(MidiNote(pitch, start, duration, velocity));
        }

        void triggerNote(int pitch, float duration, float velocity = 0.8f) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            active_voices.push_back({pitch, 0.0f, 0.0f, duration, velocity, true});
        }

        void clearNotes() {
            std::lock_guard<std::mutex> lock(synth_mutex);
            notes.clear();
            active_voices.clear();
        }
        
        std::vector<MidiNote>& getNotes() { return notes; }
        std::mutex& getMutex() { return synth_mutex; }

        // Chamado a cada frame/buffer pelo RtAudio
        void process(float* out_left, float* out_right, unsigned int nFrames, float global_time_sec) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            
            float dt = 1.0f / sample_rate;
            
            for (unsigned int i = 0; i < nFrames; i++) {
                float sample = 0.0f;
                float current_time = global_time_sec + (i * dt);

                // Checa quais notas devem começar a tocar
                for (auto& note : notes) {
                    if (!note.is_playing && current_time >= note.start_time && current_time < note.start_time + note.duration) {
                        note.is_playing = true;
                        active_voices.push_back({note.pitch, 0.0f, 0.0f, note.duration, note.velocity, true});
                    }
                }

                // Processa vozes ativas
                for (auto& voice : active_voices) {
                    if (!voice.active) continue;

                    float freq = getFrequency(voice.pitch);
                    float phase_inc = freq / sample_rate;
                    
                    if (voice.current_time >= voice.duration + 0.5f) { // +0.5s para release
                        voice.active = false;
                        continue;
                    }

                    // Usa o instrumento MIDI selecionado
                    float s = synthesize(current_instrument, freq, voice.current_time, voice.phase, voice.duration);

                    sample += s * voice.velocity * 0.3f;
                    
                    voice.phase += phase_inc;
                    if (voice.phase >= 1.0f) voice.phase -= 1.0f;
                    voice.current_time += dt;
                }

                // Limpa notas velhas
                active_voices.erase(std::remove_if(active_voices.begin(), active_voices.end(), 
                    [](const Voice& v) { return !v.active; }), active_voices.end());
                    
                // Reseta notas se voltarmos no tempo (loop/seek)
                for (auto& note : notes) {
                    if (note.is_playing && (current_time < note.start_time || current_time >= note.start_time + note.duration)) {
                        note.is_playing = false;
                    }
                }

                out_left[i] += sample;
                out_right[i] += sample;
            }
        }
    };
}
