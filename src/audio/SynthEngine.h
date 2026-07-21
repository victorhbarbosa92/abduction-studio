#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <mutex>
#include "../core/MidiNote.h"
#include "KuroWave.h"

struct SamplerSettings {
    float pan = 0.0f;
    float vol = 0.8f;
    float pitch = 0.0f;
    int pitch_range = 2;
    int track = 1;
    
    // Time stretching
    float time_pitch = 0.0f;
    float time_mul = 1.0f;
    float time_time = 0.0f;
    int time_mode = 0; // 0=Resample
    
    // Precomputed effects
    bool remove_dc = false;
    bool normalize = false;
    bool reverse = false;
    bool rev_polarity = false;
    bool fade_stereo = false;
    bool swap_stereo = false;
    float smp_start = 0.0f;
    float length = 1.0f;
    float fade_in = 0.0f;
    float fade_out = 0.0f;
    float crossfade = 0.0f;
    float trim = 0.0f;
    
    // Envelope
    bool env_enabled = false;
    float env_delay = 0.0f;
    float env_attack = 0.01f;
    float env_hold = 0.0f;
    float env_decay = 0.5f;
    float env_sustain = 1.0f;
    float env_release = 0.2f;
    float env_att_tension = 0.0f;
    float env_dec_tension = 0.0f;
    bool env_tempo = false;
    
    // LFO
    bool lfo_enabled = false;
    int lfo_shape = 0;
    float lfo_delay = 0.0f;
    float lfo_attack = 0.0f;
    float lfo_amount = 0.0f;
    float lfo_speed = 1.0f;
    bool lfo_tempo = false;
    bool lfo_global = false;
    
    // Filter
    float filter_cutoff = 1.0f;
    float filter_res = 0.0f;
    int filter_type = 0;
    
    // Misc
    float misc_pan = 0.0f;
    float misc_vol = 1.0f;
    float misc_modx = 0.0f;
    float misc_mody = 0.0f;
    
    // Polyphony
    int poly_max = 16;
    bool poly_porta = false;
    bool poly_mono = false;
    float poly_slide = 0.0f;
    
    // Arpeggiator
    int arp_direction = 0; // 0=Off
    float arp_time = 0.25f;
    float arp_gate = 0.5f;
    int arp_range = 1;
    int arp_repeat = 0;
    bool arp_slide = false;
    int arp_chord = 0;
    
    // Echo delay
    float echo_feed = 0.0f;
    float echo_pan = 0.0f;
    float echo_modx = 0.0f;
    float echo_mody = 0.0f;
    float echo_pitch = 0.0f;
    float echo_time = 0.25f;
    int echo_count = 4;
    bool echo_pingpong = false;
    bool echo_fat = false;
};

struct FlexSettings {
    int selected_pack = 0;
    int selected_preset = 0;
    float macro_filter = 0.5f;
    float macro_vibrato = 0.0f;
    float macro_harmonic = 0.0f;
    float macro_reverb = 0.1f;
    float macro_delay = 0.1f;
    float macro_extra1 = 0.0f;
    float macro_extra2 = 0.0f;
    float macro_extra3 = 0.0f;
    float filter_cutoff = 0.8f;
    float filter_res = 0.1f;
    float filter_env_amt = 0.0f;
    float pitch = 0.0f;
    float env_filter_a = 0.0f;
    float env_filter_h = 0.0f;
    float env_filter_d = 0.5f;
    float env_filter_s = 1.0f;
    float env_filter_r = 0.2f;
    float env_vol_a = 0.01f;
    float env_vol_h = 0.0f;
    float env_vol_d = 0.5f;
    float env_vol_s = 0.8f;
    float env_vol_r = 0.2f;
    float master_filter_cutoff = 1.0f;
    float master_filter_res = 0.0f;
};

#include <vector>
#include <cmath>
#include <mutex>
#include <algorithm>
#include <string>

#include "../core/KuroConfig.h"
#include "../thirdparty/dr_wav.h"

extern float track_linear_volumes[MAX_TRACKS];
extern bool is_playing;
extern float dummy_vol[8];
extern float dummy_pan[8];
extern float track_vu_levels[8];

namespace KuroAudio {

    struct DrumSample {
        std::vector<float> sample_data;
        unsigned int channels = 0;
        unsigned int sample_rate = 44100;
        uint64_t total_frames = 0;
        bool loaded = false;

        void load(const std::string& filepath) {
            drwav wav;
            if (drwav_init_file(&wav, filepath.c_str(), nullptr)) {
                channels = wav.channels;
                sample_rate = wav.sampleRate;
                total_frames = wav.totalPCMFrameCount;
                sample_data.resize(total_frames * channels);
                drwav_uint64 read = drwav_read_pcm_frames_f32(&wav, total_frames, sample_data.data());
                drwav_uninit(&wav);
                if (read > 0) {
                    loaded = true;
                } else {
                    loaded = false;
                    sample_data.clear();
                    total_frames = 0;
                }
            }
        }
    };

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
            float filter_state = 0.0f;
            int track_idx = 5;
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
        SamplerSettings sampler_settings[8];
        FlexSettings flex_settings[8];
        bool flex_active[8] = { false };

        static float getSamplerEnvelope(const SamplerSettings& s, float t, float duration) {
            if (!s.env_enabled) return 1.0f;
            float delay = s.env_delay;
            float attack = s.env_attack;
            if (attack < 0.001f) attack = 0.001f;
            float hold = s.env_hold;
            float decay = s.env_decay;
            if (decay < 0.001f) decay = 0.001f;
            float sustain = s.env_sustain;
            float release = s.env_release;
            if (release < 0.001f) release = 0.001f;
            
            if (t < delay) return 0.0f;
            t -= delay;
            if (t < attack) return t / attack;
            t -= attack;
            if (t < hold) return 1.0f;
            t -= hold;
            if (t < decay) {
                float factor = t / decay;
                return 1.0f - (1.0f - sustain) * factor;
            }
            if (t < duration) return sustain;
            
            float rel_t = t - duration;
            if (rel_t < release) {
                return sustain * (1.0f - rel_t / release);
            }
            return 0.0f;
        }

        // ── DRUM SAMPLES: Variantes reais do FL Studio ─────────────────────
        // 3 variantes por canal de bateria: [canal][variante]
        // Canal 0=Kick, 1=Snare, 2=HiHat, 3=Clap, 4=OpenHat, 5=Tom, 6=Crash, 7=Cowbell
        static constexpr int MAX_DRUM_VARIANTS = 4;
        DrumSample drum_variants[8][MAX_DRUM_VARIANTS];  // 8 canais × 4 variantes
        int selected_variant[8] = { 0 };                 // Variante ativa por canal
        DrumSample* drum_samples = nullptr; // Aponta para drum_variants[ch][selected_variant[ch]]

        // Mantidos para compatibilidade — redirecionam para drum_variants
        DrumSample& getDrumSample(int ch) {
            return drum_variants[ch][selected_variant[ch]];
        }

        DrumSample strings_sample;
        DrumSample guitar_sample;
        DrumSample cello_sample;
        DrumSample harpsichord_sample;
        DrumSample choir_sample;

        // ── SFX SAMPLES ────────────────────────────────────────────────────
        struct SfxEntry { std::string name; DrumSample sample; };
        std::vector<SfxEntry> sfx_library;

        static constexpr const char* FL_KICKS_DIR  = "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Drums\\Kicks\\";
        static constexpr const char* FL_SNARES_DIR = "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Drums\\Snares\\";
        static constexpr const char* FL_HATS_DIR   = "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Drums\\Hats\\";
        static constexpr const char* FL_CYM_DIR    = "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Drums\\Cymbals\\";
        static constexpr const char* FL_SFX_DIR    = "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\SFX\\";

        // Nomes de variantes exibidas na UI
        const char* kick_variant_names[MAX_DRUM_VARIANTS]  = {"808 Kick", "909 Kick", "Acoustic Kick", "FPC Kick"};
        const char* snare_variant_names[MAX_DRUM_VARIANTS] = {"808 Snare", "909 Snare", "FPC Snare", "Acoustic Snare"};
        const char* hat_variant_names[MAX_DRUM_VARIANTS]   = {"808 CH", "909 CH", "Grv CH 01", "808 OH"};
        const char* crash_variant_names[MAX_DRUM_VARIANTS] = {"808 Crash", "909 Crash", "Grv Crash", "Grv Ride"};

        SynthEngine() : sample_rate(44100.0f), current_type(SynthType::PIANO) {
            // ── Kicks (canal 0) ────
            drum_variants[0][0].load(std::string(FL_KICKS_DIR) + "808 Kick.wav");
            drum_variants[0][1].load(std::string(FL_KICKS_DIR) + "909 Kick.wav");
            drum_variants[0][2].load(std::string(FL_KICKS_DIR) + "Grv Kick Acoustic 01.wav");
            drum_variants[0][3].load(std::string(FL_KICKS_DIR) + "FPC 1 Kick.wav");

            // ── Snares (canal 1) ───
            drum_variants[1][0].load(std::string(FL_SNARES_DIR) + "808 Snare.wav");
            drum_variants[1][1].load(std::string(FL_SNARES_DIR) + "909 Snare.wav");
            drum_variants[1][2].load(std::string(FL_SNARES_DIR) + "FPC Snare 1.wav");
            drum_variants[1][3].load(std::string(FL_SNARES_DIR) + "Grv Snareclap 01.wav");

            // ── Hi-Hats (canal 2) ─
            drum_variants[2][0].load(std::string(FL_HATS_DIR) + "808 CH.wav");
            drum_variants[2][1].load(std::string(FL_HATS_DIR) + "909 CH 1.wav");
            drum_variants[2][2].load(std::string(FL_HATS_DIR) + "Grv CH 01.wav");
            drum_variants[2][3].load(std::string(FL_HATS_DIR) + "808 OH.wav");

            // ── Clap (canal 3) ────
            drum_variants[3][0].load(std::string(FL_SNARES_DIR) + "707 Rim.wav");
            drum_variants[3][1].load(std::string(FL_SNARES_DIR) + "909 Rim.wav");
            drum_variants[3][2].load(std::string(FL_SNARES_DIR) + "FPC Rim.wav");
            drum_variants[3][3].load(std::string(FL_SNARES_DIR) + "Stick Rim 1.wav");

            // ── Open Hat (canal 4) 
            drum_variants[4][0].load(std::string(FL_HATS_DIR) + "808 OH.wav");
            drum_variants[4][1].load(std::string(FL_HATS_DIR) + "909 OH.wav");
            drum_variants[4][2].load(std::string(FL_HATS_DIR) + "Grv OH 01.wav");
            drum_variants[4][3].load(std::string(FL_HATS_DIR) + "AMX OH.wav");

            // ── Tom (canal 5) ─────
            drum_variants[5][0].load(std::string(FL_CYM_DIR)   + "909 Ride.wav");
            drum_variants[5][1].load(std::string(FL_CYM_DIR)   + "707 Ride.wav");
            drum_variants[5][2].load(std::string(FL_CYM_DIR)   + "Grv Ride 01.wav");
            drum_variants[5][3].load(std::string(FL_CYM_DIR)   + "Linn Ride.wav");

            // ── Crash (canal 6) ───
            drum_variants[6][0].load(std::string(FL_CYM_DIR)   + "909 Crash.wav");
            drum_variants[6][1].load(std::string(FL_CYM_DIR)   + "808 Crash.wav");
            drum_variants[6][2].load(std::string(FL_CYM_DIR)   + "Grv Crash 01.wav");
            drum_variants[6][3].load(std::string(FL_CYM_DIR)   + "Thin Crash.wav");

            // ── Cowbell/Perc (canal 7) 
            drum_variants[7][0].load(std::string(FL_HATS_DIR)  + "Clank CH 1.wav");
            drum_variants[7][1].load(std::string(FL_HATS_DIR)  + "Ice Hat 1.wav");
            drum_variants[7][2].load(std::string(FL_HATS_DIR)  + "Ice Hat 3.wav");
            drum_variants[7][3].load(std::string(FL_HATS_DIR)  + "Jung Hat 1.wav");

            // ── SFX Library ───────────────────────────────────────────────
            const std::vector<std::pair<std::string,std::string>> sfx_files = {
                {"Blackhole",      std::string(FL_SFX_DIR) + "FX Blackhole.wav"},
                {"Choir Swell",    std::string(FL_SFX_DIR) + "FX Choir Swell.wav"},
                {"Drone Strings",  std::string(FL_SFX_DIR) + "FX Drone Strings Rev.wav"},
                {"Echo FM",        std::string(FL_SFX_DIR) + "FX Echo FM.wav"},
                {"Holo Drone",     std::string(FL_SFX_DIR) + "FX Holo Drone.wav"},
                {"Long Metal",     std::string(FL_SFX_DIR) + "FX Long Metal.wav"},
                {"Mech Growl",     std::string(FL_SFX_DIR) + "FX Mech Growl.wav"},
                {"Piano Nightmare",std::string(FL_SFX_DIR) + "FX Piano Nightmare.wav"},
                {"Bass Sweep Drop",std::string(FL_SFX_DIR) + "SFX Bass Sweep Drop.wav"},
                {"Big Square Drop",std::string(FL_SFX_DIR) + "SFX Big Square Drop.wav"},
                {"Crunchy Bass",   std::string(FL_SFX_DIR) + "SFX Crunchy Bass.wav"},
                {"Electro Drop",   std::string(FL_SFX_DIR) + "SFX Electro Drop.wav"},
                {"Electro Shock",  std::string(FL_SFX_DIR) + "SFX Electro Shock.wav"},
                {"SubBass Drop",   std::string(FL_SFX_DIR) + "SFX SubBass Drop.wav"},
                {"Thunder Sheet",  std::string(FL_SFX_DIR) + "SFX Thunder Sheet.wav"},
                {"Wobble Drop",    std::string(FL_SFX_DIR) + "SFX Wobble Drop.wav"},
            };
            for (auto& [name, path] : sfx_files) {
                sfx_library.push_back({name, {}});
                sfx_library.back().sample.load(path);
            }

            // ── Instrumentos Cromáticos ────────────────────────────────────
            strings_sample.load("C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Strings\\STR_3c_Long.wav");
            guitar_sample.load("C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Guitar\\Guitar\\Acoustic Guitar 01\\ACOUSTICG 1_A3.wav");
            cello_sample.load("C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Strings\\STR_Cell_C4.wav");
            harpsichord_sample.load("C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Strings\\STR_Harpsy_C4.wav");
            choir_sample.load("C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Choirs\\CHR_Aah_A3.wav");
        }

        void setSampleRate(float sr) { sample_rate = sr; }
        void setSynthType(SynthType type) { current_type = type; }
        void setInstrument(MidiInstrument inst) { current_instrument = inst; }
        MidiInstrument getInstrument() const { return current_instrument; }

        void addNote(int pitch, float start, float duration, float velocity = 0.8f) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            notes.push_back(MidiNote(pitch, start, duration, velocity));
        }

        void triggerNote(int pitch, float duration, float velocity = 0.8f, int track_idx = 5) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            active_voices.push_back({pitch, 0.0f, 0.0f, duration, velocity, true, 0.0f, track_idx});
        }

        void triggerSfx(int index) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            if (index >= 0 && index < (int)sfx_library.size()) {
                // Use a negative pitch offset to represent SFX index in the voice pool
                active_voices.push_back({-100 - index, 0.0f, 0.0f, 6.0f, 0.8f, true});
            }
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
            
            float block_peaks[8] = { 0.0f };
            float dt = 1.0f / sample_rate;
            
            for (unsigned int i = 0; i < nFrames; i++) {
                float sample_l = 0.0f;
                float sample_r = 0.0f;
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
 
                     // Usa o instrumento MIDI selecionado ou síntese/sample de bateria se aplicável
                     float s = 0.0f;
                     int drum_idx = -1;
                     int sfx_idx = -1;
                     if (voice.pitch < -90) {
                         sfx_idx = -100 - voice.pitch;
                     } else if (voice.pitch == 36) drum_idx = 0;      // Kick
                     else if (voice.pitch == 38) drum_idx = 1; // Snare
                     else if (voice.pitch == 42) drum_idx = 2; // Hihat
                     else if (voice.pitch == 39) drum_idx = 3; // Clap
                     else if (voice.pitch == 46) drum_idx = 4; // Open Hat
                     else if (voice.pitch == 41) drum_idx = 5; // Tom
                     else if (voice.pitch == 49) drum_idx = 6; // Crash
                     else if (voice.pitch == 56) drum_idx = 7; // Cowbell
 
                     if (sfx_idx >= 0 && sfx_idx < (int)sfx_library.size() && sfx_library[sfx_idx].sample.loaded) {
                         auto& ds = sfx_library[sfx_idx].sample;
                         uint64_t frame_idx = (uint64_t)(voice.current_time * ds.sample_rate);
                         if (frame_idx < ds.total_frames) {
                             if (ds.channels == 1) {
                                 s = ds.sample_data[frame_idx];
                             } else if (ds.channels >= 2) {
                                 s = ds.sample_data[frame_idx * ds.channels];
                             }
                         }
                     } else if (drum_idx >= 0 && getDrumSample(drum_idx).loaded) {
                         auto& ds = getDrumSample(drum_idx);
                         uint64_t frame_idx = (uint64_t)(voice.current_time * ds.sample_rate);
                         if (frame_idx < ds.total_frames) {
                             if (ds.channels == 1) {
                                 s = ds.sample_data[frame_idx];
                             } else if (ds.channels >= 2) {
                                 s = ds.sample_data[frame_idx * ds.channels]; // Left/Mono channel
                             }
                         }
                     } else {
                          // Math synthesis fallbacks
                          float t = voice.current_time;
                          float p_mul = 1.0f;
                          float v_mul = 1.0f;
                          bool rev_pol = false;
                          if (drum_idx >= 0 && drum_idx < 8) {
                              auto& ss = sampler_settings[drum_idx];
                              t = ss.reverse ? std::max(0.0f, voice.duration - voice.current_time) : voice.current_time;
                              p_mul = std::pow(2.0f, ss.pitch / 12.0f);
                              v_mul = ss.vol;
                              rev_pol = ss.rev_polarity;
                          }

                          if (voice.pitch == 36) { // Kick
                              float pitch_env = std::exp(-60.0f * t);
                              float kick_freq = (50.0f + 150.0f * pitch_env) * p_mul;
                              s = std::sin(KURO_TWO_PI * kick_freq * t) * std::exp(-10.0f * t) * 1.3f;
                              s += 0.2f * std::sin(KURO_TWO_PI * 1000.0f * t) * std::exp(-120.0f * t);
                          } else if (voice.pitch == 38) { // Snare
                              static thread_local uint32_t rand_seed = 12345;
                              rand_seed = rand_seed * 196314165 + 907633385;
                              float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                              float tone = std::sin(KURO_TWO_PI * 180.0f * p_mul * t) * std::exp(-35.0f * t);
                              s = (tone * 0.35f + noise * 0.65f) * std::exp(-12.0f * t) * 1.0f;
                          } else if (voice.pitch == 42) { // Hihat
                              static thread_local uint32_t rand_seed = 54321;
                              rand_seed = rand_seed * 196314165 + 907633385;
                              float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                              s = noise * std::exp(-70.0f * t) * 0.8f;
                          } else if (voice.pitch == 39) { // Clap
                              static thread_local uint32_t rand_seed = 98765;
                              rand_seed = rand_seed * 196314165 + 907633385;
                              float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                              float env = 0.0f;
                              if (t < 0.01f) env = 1.0f;
                              else if (t < 0.02f) env = 0.8f;
                              else if (t < 0.03f) env = 0.6f;
                              else env = std::exp(-15.0f * (t - 0.03f));
                              s = noise * env * 0.7f;
                          } else if (voice.pitch == 46) { // Open Hat
                              static thread_local uint32_t rand_seed = 65432;
                              rand_seed = rand_seed * 196314165 + 907633385;
                              float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                              s = noise * std::exp(-8.0f * t) * 0.7f;
                          } else if (voice.pitch == 41) { // Low Tom
                              float pitch_env = std::exp(-30.0f * t);
                              float tom_freq = (80.0f + 70.0f * pitch_env) * p_mul;
                              s = std::sin(KURO_TWO_PI * tom_freq * t) * std::exp(-6.0f * t) * 1.1f;
                          } else if (voice.pitch == 49) { // Crash
                              static thread_local uint32_t rand_seed = 76543;
                              rand_seed = rand_seed * 196314165 + 907633385;
                              float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                              float ring = std::sin(KURO_TWO_PI * 400.0f * p_mul * t) * std::exp(-10.0f * t) +
                                           std::sin(KURO_TWO_PI * 580.0f * p_mul * t) * std::exp(-15.0f * t);
                              s = (noise * 0.7f + ring * 0.3f) * std::exp(-1.5f * t) * 0.8f;
                          } else if (voice.pitch == 56) { // Cowbell
                              float f1 = 587.0f * p_mul;
                              float f2 = 845.0f * p_mul;
                              float wave1 = (std::sin(KURO_TWO_PI * f1 * t) >= 0.0f) ? 1.0f : -1.0f;
                              float wave2 = (std::sin(KURO_TWO_PI * f2 * t) >= 0.0f) ? 1.0f : -1.0f;
                              s = (wave1 + wave2) * 0.3f * std::exp(-15.0f * t);
                          } else {
                              // Mapeia instrumentos cromáticos baseados em samples do FL Studio
                              DrumSample* ds = nullptr;
                              int base_pitch = 60;
                              bool loop = false;

                              if (current_instrument == MidiInstrument::STRING_ENSEMBLE) {
                                  ds = &strings_sample; base_pitch = 48; loop = true;
                              } else if (current_instrument == MidiInstrument::NYLON_GUITAR) {
                                  ds = &guitar_sample; base_pitch = 57; loop = false;
                              } else if (current_instrument == MidiInstrument::CELLO) {
                                  ds = &cello_sample; base_pitch = 60; loop = true;
                              } else if (current_instrument == MidiInstrument::HARPSICHORD) {
                                  ds = &harpsichord_sample; base_pitch = 60; loop = false;
                              } else if (current_instrument == MidiInstrument::MARIMBA || current_instrument == MidiInstrument::CELESTA) {
                                  ds = &choir_sample; base_pitch = 57; loop = true;
                              }

                              if (ds && ds->loaded) {
                                  float ratio = std::pow(2.0f, (voice.pitch - base_pitch) / 12.0f);
                                  uint64_t frame_idx = (uint64_t)(voice.current_time * ds->sample_rate * ratio);
                                  if (loop) {
                                      if (ds->total_frames > 0) {
                                          frame_idx = frame_idx % ds->total_frames;
                                          s = ds->sample_data[frame_idx * ds->channels];
                                      }
                                  } else {
                                      if (frame_idx < ds->total_frames) {
                                          s = ds->sample_data[frame_idx * ds->channels];
                                      }
                                  }
                                  float env = 1.0f;
                                  if (voice.current_time > voice.duration) {
                                      env = std::max(0.0f, 1.0f - (voice.current_time - voice.duration) * 4.0f);
                                  }
                                  s *= env;
                              } else {
                                  s = synthesize(current_instrument, freq, voice.current_time, voice.phase, voice.duration);
                              }
                          }
                     }
 
                     int trk = 5;
                     if (voice.pitch == 36) trk = ::channel_tracks[0];
                     else if (voice.pitch == 38) trk = ::channel_tracks[1];
                     else if (voice.pitch == 42) trk = ::channel_tracks[2];
                     else if (voice.pitch == 39) trk = ::channel_tracks[3];
                     else if (voice.pitch == 46) trk = ::channel_tracks[4];
                     else if (voice.pitch == 41) trk = ::channel_tracks[5];
                     else if (voice.pitch == 49) trk = ::channel_tracks[6];
                     else if (voice.pitch == 56) trk = ::channel_tracks[7];
                     else if (voice.pitch == 48) trk = 4;
                     else if (voice.pitch == 60) trk = 5;
                     else if (voice.pitch == 64) trk = 6;
                     else if (voice.pitch == 72) trk = 7;
 
                     float voice_gain = ::is_playing ? ::track_linear_volumes[trk] : 1.0f;
                     float channel_gain = 1.0f;
                     float pan_l = 1.0f;
                     float pan_r = 1.0f;
                     
                     if (drum_idx >= 0) {
                         channel_gain = ::dummy_vol[drum_idx];
                         float p = ::dummy_pan[drum_idx];
                         pan_l = std::min(1.0f, 1.0f - p);
                         pan_r = std::min(1.0f, 1.0f + p);
                     }
 
                     float base_sample = s * voice.velocity * 0.3f * voice_gain * channel_gain;
                     sample_l += base_sample * pan_l;
                     sample_r += base_sample * pan_r;

                     if (trk >= 0 && trk < 8) {
                         float abs_s = std::abs(base_sample);
                         if (abs_s > block_peaks[trk]) {
                             block_peaks[trk] = abs_s;
                         }
                     }
                     
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

                out_left[i] += sample_l;
                out_right[i] += sample_r;
            }

            for (int t = 0; t < 8; t++) {
                if (t != 5) {
                    ::track_vu_levels[t] = ::track_vu_levels[t] * 0.8f + block_peaks[t] * 0.2f;
                }
            }
        }

        void processMultitrack(float** track_outs_l, float** track_outs_r, unsigned int nFrames, float global_time_sec) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            
            float block_peaks[8] = { 0.0f };
            float dt = 1.0f / sample_rate;
            
            for (unsigned int i = 0; i < nFrames; i++) {
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
                    
                    // Modulação de Vibrato via Macro do FLEX
                    int ch_idx = 5;
                    if (voice.pitch == 36) ch_idx = 0;
                    else if (voice.pitch == 38) ch_idx = 1;
                    else if (voice.pitch == 42) ch_idx = 2;
                    else if (voice.pitch == 48) ch_idx = 3;
                    else if (voice.pitch == 60) ch_idx = 4;
                    else if (voice.pitch == 72) ch_idx = 5;
                    else if (voice.pitch == 39) ch_idx = 6;
                    else if (voice.pitch == 46) ch_idx = 7;
                    auto& fs = flex_settings[ch_idx];
                    
                    if (voice.pitch == 48 || voice.pitch == 60 || voice.pitch == 72) {
                        float vibrato_lfo = std::sin(KURO_TWO_PI * 6.0f * voice.current_time) * 0.03f * fs.macro_vibrato;
                        freq *= (1.0f + vibrato_lfo);
                    }
                    
                    float phase_inc = freq / sample_rate;
                    
                    if (voice.current_time >= voice.duration + 0.5f) { // +0.5s para release
                        voice.active = false;
                        continue;
                    }

                    float s = 0.0f;
                    int drum_idx = -1;
                    int sfx_idx = -1;
                    if (voice.pitch < -90) {
                        sfx_idx = -100 - voice.pitch;
                    } else if (voice.pitch == 36) drum_idx = 0;      // Kick
                    else if (voice.pitch == 38) drum_idx = 1; // Snare
                    else if (voice.pitch == 42) drum_idx = 2; // Hihat
                    else if (voice.pitch == 39) drum_idx = 3; // Clap
                    else if (voice.pitch == 46) drum_idx = 4; // Open Hat
                    else if (voice.pitch == 41) drum_idx = 5; // Tom
                    else if (voice.pitch == 49) drum_idx = 6; // Crash
                    else if (voice.pitch == 56) drum_idx = 7; // Cowbell

                    if (sfx_idx >= 0 && sfx_idx < (int)sfx_library.size() && sfx_library[sfx_idx].sample.loaded) {
                        auto& ds = sfx_library[sfx_idx].sample;
                        uint64_t frame_idx = (uint64_t)(voice.current_time * ds.sample_rate);
                        if (frame_idx < ds.total_frames) {
                            if (ds.channels == 1) {
                                s = ds.sample_data[frame_idx];
                            } else if (ds.channels >= 2) {
                                s = ds.sample_data[frame_idx * ds.channels];
                            }
                        }
                    } else if (drum_idx >= 0 && getDrumSample(drum_idx).loaded) {
                        auto& ds = getDrumSample(drum_idx);
                        uint64_t frame_idx = (uint64_t)(voice.current_time * ds.sample_rate);
                        if (frame_idx < ds.total_frames) {
                            if (ds.channels == 1) {
                                s = ds.sample_data[frame_idx];
                            } else if (ds.channels >= 2) {
                                s = ds.sample_data[frame_idx * ds.channels];
                            }
                        }
                    } else {
                        // Math synthesis fallbacks
                        float t = voice.current_time;
                        float p_mul = 1.0f;
                        float v_mul = 1.0f;
                        bool rev_pol = false;
                        if (drum_idx >= 0 && drum_idx < 8) {
                            auto& ss = sampler_settings[drum_idx];
                            t = ss.reverse ? std::max(0.0f, voice.duration - voice.current_time) : voice.current_time;
                            p_mul = std::pow(2.0f, ss.pitch / 12.0f);
                            v_mul = ss.vol;
                            rev_pol = ss.rev_polarity;
                        }

                        if (voice.pitch == 36) { // Kick
                            float pitch_env = std::exp(-60.0f * t);
                            float kick_freq = (50.0f + 150.0f * pitch_env) * p_mul;
                            s = std::sin(KURO_TWO_PI * kick_freq * t) * std::exp(-10.0f * t) * 1.3f;
                            s += 0.2f * std::sin(KURO_TWO_PI * 1000.0f * t) * std::exp(-120.0f * t);
                        } else if (voice.pitch == 38) { // Snare
                            static thread_local uint32_t rand_seed = 12345;
                            rand_seed = rand_seed * 196314165 + 907633385;
                            float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                            float tone = std::sin(KURO_TWO_PI * 180.0f * p_mul * t) * std::exp(-35.0f * t);
                            s = (tone * 0.35f + noise * 0.65f) * std::exp(-12.0f * t) * 1.0f;
                        } else if (voice.pitch == 42) { // Hihat
                            static thread_local uint32_t rand_seed = 54321;
                            rand_seed = rand_seed * 196314165 + 907633385;
                            float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                            s = noise * std::exp(-70.0f * t) * 0.8f;
                        } else if (voice.pitch == 39) { // Clap
                            static thread_local uint32_t rand_seed = 98765;
                            rand_seed = rand_seed * 196314165 + 907633385;
                            float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                            float env = 0.0f;
                            if (t < 0.01f) env = 1.0f;
                            else if (t < 0.02f) env = 0.8f;
                            else if (t < 0.03f) env = 0.6f;
                            else env = std::exp(-15.0f * (t - 0.03f));
                            s = noise * env * 0.7f;
                        } else if (voice.pitch == 46) { // Open Hat
                            static thread_local uint32_t rand_seed = 65432;
                            rand_seed = rand_seed * 196314165 + 907633385;
                            float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                            s = noise * std::exp(-8.0f * t) * 0.7f;
                        } else if (voice.pitch == 41) { // Low Tom
                            float pitch_env = std::exp(-30.0f * t);
                            float tom_freq = (80.0f + 70.0f * pitch_env) * p_mul;
                            s = std::sin(KURO_TWO_PI * tom_freq * t) * std::exp(-6.0f * t) * 1.1f;
                        } else if (voice.pitch == 49) { // Crash
                            static thread_local uint32_t rand_seed = 76543;
                            rand_seed = rand_seed * 196314165 + 907633385;
                            float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                            float ring = std::sin(KURO_TWO_PI * 400.0f * p_mul * t) * std::exp(-10.0f * t) +
                                         std::sin(KURO_TWO_PI * 580.0f * p_mul * t) * std::exp(-15.0f * t);
                            s = (noise * 0.7f + ring * 0.3f) * std::exp(-1.5f * t) * 0.8f;
                        } else if (voice.pitch == 56) { // Cowbell
                            float f1 = 587.0f * p_mul;
                            float f2 = 845.0f * p_mul;
                            float wave1 = (std::sin(KURO_TWO_PI * f1 * t) >= 0.0f) ? 1.0f : -1.0f;
                            float wave2 = (std::sin(KURO_TWO_PI * f2 * t) >= 0.0f) ? 1.0f : -1.0f;
                            s = (wave1 + wave2) * 0.3f * std::exp(-15.0f * t);
                        } else {
                            DrumSample* ds = nullptr;
                            int base_pitch = 60;
                            bool loop = false;

                            if (current_instrument == MidiInstrument::STRING_ENSEMBLE) {
                                ds = &strings_sample; base_pitch = 48; loop = true;
                            } else if (current_instrument == MidiInstrument::NYLON_GUITAR) {
                                ds = &guitar_sample; base_pitch = 57; loop = false;
                            } else if (current_instrument == MidiInstrument::CELLO) {
                                ds = &cello_sample; base_pitch = 60; loop = true;
                            } else if (current_instrument == MidiInstrument::HARPSICHORD) {
                                ds = &harpsichord_sample; base_pitch = 60; loop = false;
                            } else if (current_instrument == MidiInstrument::MARIMBA || current_instrument == MidiInstrument::CELESTA) {
                                ds = &choir_sample; base_pitch = 57; loop = true;
                            }

                            if (ds && ds->loaded) {
                                float ratio = std::pow(2.0f, (voice.pitch - base_pitch) / 12.0f);
                                uint64_t frame_idx = (uint64_t)(voice.current_time * ds->sample_rate * ratio);
                                if (loop) {
                                    if (ds->total_frames > 0) {
                                        frame_idx = frame_idx % ds->total_frames;
                                        s = ds->sample_data[frame_idx * ds->channels];
                                    }
                                } else {
                                    if (frame_idx < ds->total_frames) {
                                        s = ds->sample_data[frame_idx * ds->channels];
                                    }
                                }
                                float env = 1.0f;
                                if (voice.current_time > voice.duration) {
                                    env = std::max(0.0f, 1.0f - (voice.current_time - voice.duration) * 4.0f);
                                }
                                s *= env;
                            } else {
                                s = synthesize(current_instrument, freq, voice.current_time, voice.phase, voice.duration);
                            }
                        }
                    }

                    // Aplica Envelope ADSR dos Knobs do FLEX
                    if (voice.pitch == 48 || voice.pitch == 60 || voice.pitch == 72) {
                        float adsr = 1.0f;
                        float t_env = voice.current_time;
                        float attack = fs.env_vol_a;
                        float decay = fs.env_vol_d;
                        float sustain = fs.env_vol_s;
                        float release = fs.env_vol_r;
                        if (t_env < attack) {
                            adsr = t_env / attack;
                        } else if (t_env < attack + decay) {
                            float factor = (t_env - attack) / decay;
                            adsr = 1.0f - (1.0f - sustain) * factor;
                        } else if (t_env < voice.duration) {
                            adsr = sustain;
                        } else {
                            float rel_t = t_env - voice.duration;
                            if (rel_t < release) {
                                adsr = sustain * (1.0f - rel_t / release);
                            } else {
                                adsr = 0.0f;
                                voice.active = false;
                            }
                        }
                        s *= adsr;
                        
                        // Aplica Filtro Cutoff (Macro + Knob + Master Filter)
                        float cutoff_target = fs.filter_cutoff * 0.4f + fs.macro_filter * 0.4f + fs.master_filter_cutoff * 0.2f;
                        cutoff_target = std::max(0.01f, std::min(0.99f, cutoff_target));
                        voice.filter_state = voice.filter_state + cutoff_target * (s - voice.filter_state);
                        s = voice.filter_state;
                    }

                    int trk = ::channel_tracks[voice.track_idx];

                    float voice_gain = ::is_playing ? ::track_linear_volumes[trk] : 1.0f;
                    float channel_gain = 1.0f;
                    float pan_l = 1.0f;
                    float pan_r = 1.0f;
                    
                    if (drum_idx >= 0) {
                        channel_gain = ::dummy_vol[drum_idx];
                        float p = ::dummy_pan[drum_idx];
                        pan_l = std::min(1.0f, 1.0f - p);
                        pan_r = std::min(1.0f, 1.0f + p);
                    }

                    float base_sample = s * voice.velocity * 0.3f * voice_gain * channel_gain;
                    
                    if (trk >= 0 && trk < 8) {
                        track_outs_l[trk][i] += base_sample * pan_l;
                        track_outs_r[trk][i] += base_sample * pan_r;
                        float abs_s = std::abs(base_sample);
                        if (abs_s > block_peaks[trk]) {
                            block_peaks[trk] = abs_s;
                        }
                    }

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
            }

            for (int t = 0; t < 8; t++) {
                if (t != 5) {
                    ::track_vu_levels[t] = ::track_vu_levels[t] * 0.8f + block_peaks[t] * 0.2f;
                }
            }
        }
    };
}

