#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <mutex>
#include <filesystem>
#include "../core/MidiNote.h"
#include "KuroWave.h"

extern int channel_tracks[MAX_TRACKS];

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
    
    // Root Note & Cut Groups
    int root_note = 60; // C5
    bool enable_main_pitch = true;
    bool add_to_key = false;
    float fine_tune = 0.0f; // -100 to +100 cents
    int cut_group = 0;
    int cut_by = 0;
    bool cut_self = true;
    
    // UI Navigation State
    int active_tab = 0; // 0=Waveform, 1=Envelope & Filter, 2=Miscellaneous
    int active_env_subtab = 1; // 0=Panning, 1=Volume, 2=Mod X, 3=Mod Y, 4=Pitch
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
extern float dummy_vol[MAX_TRACKS];
extern float dummy_pan[MAX_TRACKS];
extern float track_vu_levels[MAX_TRACKS];

inline float g_delay_lama_vowel_x = 0.50f; // 0.0=OOH, 0.25=OW, 0.50=AH, 0.75=AYH, 1.0=EEH
inline float g_delay_lama_pitch_y = 0.0f;  // -1.0 to +1.0 semitones
inline float g_delay_lama_delay_time = 0.35f;
inline float g_delay_lama_delay_feedback = 0.45f;
inline float g_delay_lama_delay_mix = 0.40f;

namespace KuroAudio {

    struct DrumSample {
        std::vector<float> sample_data;
        unsigned int channels = 0;
        unsigned int sample_rate = 44100;
        uint64_t total_frames = 0;
        bool loaded = false;
        std::string filepath;

        void load(const std::string& path) {
            filepath = path;
            drwav wav;
            if (drwav_init_file(&wav, path.c_str(), nullptr)) {
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
        HONKY_TONK,         // 2  - Piano Detunado (GM 4)
        HARPSICHORD,        // 3  - Cravo (GM 7)
        CLAVINET,           // 4  - Clavinet (GM 8)
        CELESTA,            // 5  - Celesta (GM 9)
        VIBRAPHONE,         // 6  - Vibrafone (GM 12)
        MARIMBA,            // 7  - Marimba (GM 13)
        ORGAN,              // 8  - Órgão (GM 17)
        CHURCH_ORGAN,       // 9  - Órgão de Igreja (GM 20)
        ACCORDION,          // 10 - Acordeão (GM 22)
        NYLON_GUITAR,       // 11 - Violão Nylon (GM 25)
        STEEL_GUITAR,       // 12 - Violão Aço (GM 26)
        ELECTRIC_GUITAR,    // 13 - Guitarra Elétrica (GM 28)
        ELECTRIC_BASS,      // 14 - Baixo Elétrico (GM 34)
        VIOLIN,             // 15 - Violino (GM 41)
        CELLO,              // 16 - Violoncelo (GM 43)
        STRING_ENSEMBLE,    // 17 - Ensemble de Cordas (GM 49)
        CHOIR,              // 18 - Coro (GM 53)
        BRASS_SYNTH,        // 19 - Synth Brass (GM 63)
        FLUTE,              // 20 - Flauta (GM 74)
        SQUARE_LEAD,        // 21 - Square Chiptune Lead
        SYNTH_SAW,          // 22 - 303 Acid Saw
        LEAD_SYNTH,         // 23 - Goa Lead
        FM_SYNTH,           // 24 - Alien FM Zap
        PAD_SYNTH,          // 25 - Dark Psy Drone Pad
        TRANCE_LEAD,        // 26 - Gated Trance Lead
        SUB_BASS,           // 27 - FM Sub Boom
        SWEEP_PAD,          // 28 - Psy Noise Sweep
        FULLON_LEAD,        // 29 - Full-On Scream Lead
        DELAY_LAMA,         // 30 - Delay Lama Tibetan Monk Vocal Synth
        ALIEN_LED_SYNTH,    // 31 - Alien LED Cyber Synth (Som Autoral Sci-Fi)
        COUNT
    };

    inline const char* getMidiInstrumentName(MidiInstrument inst) {
        switch (inst) {
            case MidiInstrument::ACOUSTIC_PIANO:   return "Acoustic Piano";
            case MidiInstrument::ELECTRIC_PIANO:   return "Electric Piano";
            case MidiInstrument::HONKY_TONK:       return "Honky-tonk Piano";
            case MidiInstrument::HARPSICHORD:      return "Harpsichord";
            case MidiInstrument::CLAVINET:         return "Clavinet";
            case MidiInstrument::CELESTA:          return "Celesta";
            case MidiInstrument::VIBRAPHONE:       return "Vibraphone";
            case MidiInstrument::MARIMBA:          return "Marimba";
            case MidiInstrument::ORGAN:            return "Rock Organ";
            case MidiInstrument::CHURCH_ORGAN:     return "Church Organ";
            case MidiInstrument::ACCORDION:        return "Accordion";
            case MidiInstrument::NYLON_GUITAR:     return "Nylon Guitar";
            case MidiInstrument::STEEL_GUITAR:     return "Steel Guitar";
            case MidiInstrument::ELECTRIC_GUITAR:  return "Electric Guitar";
            case MidiInstrument::ELECTRIC_BASS:    return "Electric Bass";
            case MidiInstrument::VIOLIN:           return "Violin";
            case MidiInstrument::CELLO:            return "Cello";
            case MidiInstrument::STRING_ENSEMBLE:  return "String Ensemble";
            case MidiInstrument::CHOIR:            return "Choir (Aah)";
            case MidiInstrument::BRASS_SYNTH:      return "Synth Brass";
            case MidiInstrument::FLUTE:            return "Flute";
            case MidiInstrument::SQUARE_LEAD:      return "Square Chiptune Lead";
            case MidiInstrument::SYNTH_SAW:        return "303 Acid Saw";
            case MidiInstrument::LEAD_SYNTH:       return "Goa Saw Lead";
            case MidiInstrument::FM_SYNTH:         return "Alien FM Zap";
            case MidiInstrument::PAD_SYNTH:        return "Dark Psy Drone Pad";
            case MidiInstrument::TRANCE_LEAD:      return "Gated Trance Lead";
            case MidiInstrument::SUB_BASS:         return "FM Sub Boom";
            case MidiInstrument::SWEEP_PAD:        return "Psy Noise Sweep";
            case MidiInstrument::FULLON_LEAD:      return "Full-On Scream Lead";
            case MidiInstrument::DELAY_LAMA:       return "Delay Lama (Monge Tibetano Real)";
            case MidiInstrument::ALIEN_LED_SYNTH:  return "Alien LED Synth (Chiptune Sci-Fi)";
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
            float phase = 0.0f;
            float current_time = 0.0f;
            float duration = 1.0f;
            float velocity = 0.8f;
            bool active = true;
            float filter_state = 0.0f;
            float filter_low = 0.0f;
            float filter_band = 0.0f;
            int track_idx = 5;

            Voice(int p = 60, float ph = 0.0f, float ct = 0.0f, float dur = 1.0f, float vel = 0.8f, bool act = true, int trk = 5)
                : pitch(p), phase(ph), current_time(ct), duration(dur), velocity(vel), active(act), filter_state(0.0f), filter_low(0.0f), filter_band(0.0f), track_idx(trk) {}
            Voice(int p, float ph, float ct, float dur, float vel, bool act, float fs, int trk)
                : pitch(p), phase(ph), current_time(ct), duration(dur), velocity(vel), active(act), filter_state(fs), filter_low(0.0f), filter_band(0.0f), track_idx(trk) {}
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

            // --- ACOUSTIC PIANO: Grand Piano Steinway (Uníssono Triplo de Cordas & Tábua Harmônica) ---
            case MidiInstrument::ACOUSTIC_PIANO: {
                // Uníssono triplo de cordas com micro-desafinação natural de piano acústico
                float w1 = std::sin(KURO_TWO_PI * freq * t);
                float w2 = std::sin(KURO_TWO_PI * freq * 1.0008f * t);
                float w3 = std::sin(KURO_TWO_PI * freq * 0.9992f * t);
                wave = (w1 + w2 + w3) * 0.33f;
                
                // Harmônicos naturais da corda com decaimento físico progressivo
                wave += 0.45f * std::sin(KURO_TWO_PI * freq * 2.0f * t) * std::exp(-0.8f * t);
                wave += 0.28f * std::sin(KURO_TWO_PI * freq * 3.0f * t) * std::exp(-1.4f * t);
                wave += 0.15f * std::sin(KURO_TWO_PI * freq * 4.0f * t) * std::exp(-2.2f * t);
                wave += 0.08f * std::sin(KURO_TWO_PI * freq * 5.0f * t) * std::exp(-3.2f * t);
                wave += 0.04f * std::sin(KURO_TWO_PI * freq * 6.0f * t) * std::exp(-4.5f * t);
                
                // Ataque de martelo feltro/madeira
                wave += 0.25f * std::sin(KURO_TWO_PI * (freq * 7.5f + 120.0f) * t) * std::exp(-45.0f * t);
                
                // Ressonância da tábua harmônica de madeira (Soundboard resonance)
                float soundboard = std::sin(KURO_TWO_PI * 140.0f * t) * 0.12f * std::exp(-10.0f * t);
                wave += soundboard;
                
                env = std::exp(-0.75f * t); // Sustain longo e cantado
                if (t > duration) env *= std::exp(-10.0f * (t - duration));
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

            // --- NYLON GUITAR: Violão Clássico de Nylon (Pluck Quente & Tampo Harmônico) ---
            case MidiInstrument::NYLON_GUITAR: {
                float fundamental = std::sin(KURO_TWO_PI * freq * t);
                float h2 = 0.20f * std::sin(KURO_TWO_PI * freq * 2.0f * t);
                float h3 = 0.40f * std::sin(KURO_TWO_PI * freq * 3.0f * t);
                float h4 = 0.10f * std::sin(KURO_TWO_PI * freq * 4.0f * t);
                float h5 = 0.25f * std::sin(KURO_TWO_PI * freq * 5.0f * t) * std::exp(-2.0f * t);
                float h7 = 0.12f * std::sin(KURO_TWO_PI * freq * 7.0f * t) * std::exp(-4.0f * t);
                wave = fundamental + h2 + h3 + h4 + h5 + h7;
                
                // Ataque de unha/polegar com ressonância de madeira do violão
                wave += 0.35f * std::sin(KURO_TWO_PI * (freq * 8.0f + 300.0f) * t) * std::exp(-40.0f * t);
                wave += 0.15f * std::sin(KURO_TWO_PI * 190.0f * t) * std::exp(-15.0f * t);
                
                wave *= 0.65f;
                env = std::exp(-1.4f * t);
                if (t > duration) env *= std::exp(-12.0f * (t - duration));
                break;
            }

            // --- STEEL GUITAR: Violão Folk de Aço (Brilho e Ataque de Palheta) ---
            case MidiInstrument::STEEL_GUITAR: {
                wave  = std::sin(KURO_TWO_PI * freq * t);
                wave += 0.45f * std::sin(KURO_TWO_PI * freq * 2.0f * t);
                wave += 0.35f * std::sin(KURO_TWO_PI * freq * 3.0f * t);
                wave += 0.25f * std::sin(KURO_TWO_PI * freq * 4.0f * t);
                wave += 0.15f * std::sin(KURO_TWO_PI * freq * 5.0f * t);
                wave += 0.10f * std::sin(KURO_TWO_PI * freq * 6.0f * t);
                
                wave += 0.35f * std::sin(KURO_TWO_PI * (freq * 9.0f + 500.0f) * t) * std::exp(-35.0f * t);
                wave *= 0.60f;
                
                env = std::exp(-1.8f * t);
                if (t > duration) env *= std::exp(-10.0f * (t - duration));
                break;
            }

            // --- ELECTRIC BASS: Baixo Elétrico & Acústico Encorpado (Peso e Pegada) ---
            case MidiInstrument::ELECTRIC_BASS: {
                float fundamental = std::sin(KURO_TWO_PI * freq * t);
                float sub = 0.60f * std::sin(KURO_TWO_PI * (freq * 0.5f) * t);
                float h2 = 0.40f * std::sin(KURO_TWO_PI * freq * 2.0f * t);
                float h3 = 0.20f * std::sin(KURO_TWO_PI * freq * 3.0f * t);
                float h4 = 0.10f * std::sin(KURO_TWO_PI * freq * 4.0f * t);
                wave = fundamental + sub + h2 + h3 + h4;
                
                // Ataque de dedo/slap
                wave += 0.45f * std::sin(KURO_TWO_PI * (freq * 6.0f + 80.0f) * t) * std::exp(-30.0f * t);
                wave *= 0.55f;
                
                env = std::exp(-1.1f * t);
                if (t > duration) env *= std::exp(-8.0f * (t - duration));
                break;
            }

            // --- VIOLIN: Violino Solo Expressivo (Atrito de Arco & Vibrato Natural) ---
            case MidiInstrument::VIOLIN: {
                float vib_amount = std::min(1.0f, t * 2.5f) * 0.006f;
                float vib = freq * (1.0f + vib_amount * std::sin(KURO_TWO_PI * 5.6f * t));
                
                wave  = std::sin(KURO_TWO_PI * vib * t);
                wave += 0.55f * std::sin(KURO_TWO_PI * vib * 2.0f * t);
                wave += 0.38f * std::sin(KURO_TWO_PI * vib * 3.0f * t);
                wave += 0.28f * std::sin(KURO_TWO_PI * vib * 4.0f * t);
                wave += 0.20f * std::sin(KURO_TWO_PI * vib * 5.0f * t);
                wave += 0.15f * std::sin(KURO_TWO_PI * vib * 6.0f * t);
                
                float bow_noise = std::sin(KURO_TWO_PI * 3450.0f * t + 2.0f * std::sin(KURO_TWO_PI * 87.0f * t)) * 0.04f;
                wave = (wave * 0.38f) + bow_noise;
                
                float attack = 0.06f;
                env = 1.0f;
                if (t < attack) env = t / attack;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 8.0f);
                break;
            }

            // --- CELLO: Violoncelo Orquestral Grave ---
            case MidiInstrument::CELLO: {
                float vib = freq * (1.0f + 0.004f * std::sin(KURO_TWO_PI * 5.0f * t));
                
                wave  = std::sin(KURO_TWO_PI * vib * t);
                wave += 0.60f * std::sin(KURO_TWO_PI * vib * 2.0f * t);
                wave += 0.40f * std::sin(KURO_TWO_PI * vib * 3.0f * t);
                wave += 0.30f * std::sin(KURO_TWO_PI * vib * 4.0f * t);
                wave += 0.20f * std::sin(KURO_TWO_PI * vib * 5.0f * t);
                wave *= 0.38f;
                
                float attack = 0.08f;
                env = 1.0f;
                if (t < attack) env = t / attack;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 8.0f);
                break;
            }

            // --- STRING ENSEMBLE: Orquestra Sinfônica de Cordas (5-Voice Unison & Detuning) ---
            case MidiInstrument::STRING_ENSEMBLE: {
                float s_sum = 0.0f;
                float detunes[5] = {-0.004f, -0.002f, 0.0f, 0.002f, 0.004f};
                float vib_rates[5] = {4.8f, 5.2f, 5.0f, 5.5f, 4.5f};
                for (int i = 0; i < 5; ++i) {
                    float f = freq * (1.0f + detunes[i]);
                    float vib = f * (1.0f + 0.0035f * std::sin(KURO_TWO_PI * vib_rates[i] * t));
                    float w = std::sin(KURO_TWO_PI * vib * t);
                    w += 0.50f * std::sin(KURO_TWO_PI * vib * 2.0f * t);
                    w += 0.30f * std::sin(KURO_TWO_PI * vib * 3.0f * t);
                    w += 0.18f * std::sin(KURO_TWO_PI * vib * 4.0f * t);
                    s_sum += w;
                }
                wave = s_sum * 0.15f;
                
                float attack = 0.10f;
                env = 1.0f;
                if (t < attack) env = t / attack;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 4.0f);
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

            // --- 303 ACID SAW (Psytrance Acid Squelch) ---
            case MidiInstrument::SYNTH_SAW: {
                float p = std::fmod(freq * t, 1.0f);
                float saw = 2.0f * p - 1.0f;
                float cutoff_env = std::exp(-8.0f * t);
                wave = saw * (1.0f + 3.0f * cutoff_env);
                env = std::exp(-4.0f * t);
                break;
            }

            // --- GOA SAW LEAD (Detuned Unison Saw Lead) ---
            case MidiInstrument::LEAD_SYNTH: {
                float s1 = 2.0f * std::fmod(freq * t, 1.0f) - 1.0f;
                float s2 = 2.0f * std::fmod(freq * 1.006f * t, 1.0f) - 1.0f;
                float s3 = 2.0f * std::fmod(freq * 0.994f * t, 1.0f) - 1.0f;
                wave = (s1 + s2 + s3) * 0.35f;
                env = std::exp(-1.5f * t);
                break;
            }

            // --- ALIEN FM ZAP (Fast FM Pitch Sweep) ---
            case MidiInstrument::FM_SYNTH: {
                float mod_freq = freq * 3.5f;
                float mod = std::sin(KURO_TWO_PI * mod_freq * t) * 4.0f * std::exp(-20.0f * t);
                wave = std::sin(KURO_TWO_PI * (freq + mod * 100.0f) * t);
                env = std::exp(-12.0f * t);
                break;
            }

            // --- DARK PSY DRONE PAD ---
            case MidiInstrument::PAD_SYNTH: {
                float lfo = std::sin(KURO_TWO_PI * 0.3f * t) * 0.005f;
                wave = std::sin(KURO_TWO_PI * freq * (1.0f + lfo) * t) * 0.6f + std::sin(KURO_TWO_PI * freq * 0.5f * t) * 0.4f;
                float att = 0.8f;
                env = (t < att) ? (t / att) : 1.0f;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 0.8f);
                break;
            }

            // --- GATED TRANCE LEAD ---
            case MidiInstrument::TRANCE_LEAD: {
                float pulse = (std::fmod(t * 16.0f, 1.0f) < 0.5f) ? 1.0f : 0.05f; // 1/16th Gate
                float s1 = 2.0f * std::fmod(freq * t, 1.0f) - 1.0f;
                wave = s1 * pulse;
                env = std::exp(-2.0f * t);
                break;
            }

            // --- FM SUB BOOM ---
            case MidiInstrument::SUB_BASS: {
                float sub_env = std::exp(-6.0f * t);
                wave = std::sin(KURO_TWO_PI * (freq * 0.5f) * t + sub_env * 1.5f);
                env = std::exp(-3.5f * t);
                break;
            }

            // --- PSY NOISE SWEEP ---
            case MidiInstrument::SWEEP_PAD: {
                float sweep_freq = freq * (1.0f + 5.0f * std::min(1.0f, t * 0.8f));
                wave = std::sin(KURO_TWO_PI * sweep_freq * t);
                env = std::min(1.0f, t * 1.5f);
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 2.0f);
                break;
            }

            // --- FULL-ON SCREAM LEAD ---
            case MidiInstrument::FULLON_LEAD: {
                float pitch_bend = std::exp(-15.0f * t) * 12.0f; // 1 Octave pitch drop at attack
                float cur_freq = freq * std::pow(2.0f, pitch_bend / 12.0f);
                float sq = (std::fmod(cur_freq * t, 1.0f) < 0.4f) ? 1.0f : -1.0f;
                wave = sq * 0.8f;
                env = std::exp(-1.8f * t);
                break;
            }

            // --- HONKY-TONK PIANO: Piano desafinado percussivo ---
            case MidiInstrument::HONKY_TONK: {
                wave  = std::sin(KURO_TWO_PI * freq * 0.996f * t);
                wave += std::sin(KURO_TWO_PI * freq * 1.004f * t);
                wave += 0.40f * std::sin(KURO_TWO_PI * freq * 2.0f * t);
                wave *= 0.40f;
                env = std::exp(-2.2f * t);
                break;
            }

            // --- CLAVINET: Onda dente de serra filtrada com ataque rápido ---
            case MidiInstrument::CLAVINET: {
                float p = std::fmod(freq * t, 1.0f);
                wave = (p < 0.25f) ? 1.0f : -0.33f;
                env = std::exp(-5.0f * t);
                break;
            }

            // --- ROCK ORGAN: Órgão B3 Hammond com 4 tonewheels ---
            case MidiInstrument::ORGAN: {
                wave  = std::sin(KURO_TWO_PI * freq * t);
                wave += 0.80f * std::sin(KURO_TWO_PI * freq * 2.0f * t);
                wave += 0.60f * std::sin(KURO_TWO_PI * freq * 3.0f * t);
                wave += 0.40f * std::sin(KURO_TWO_PI * freq * 4.0f * t);
                wave *= 0.30f;
                env = 1.0f;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 20.0f);
                break;
            }

            // --- ELECTRIC GUITAR: Guitarra saturada com distorção harmônica ---
            case MidiInstrument::ELECTRIC_GUITAR: {
                float raw = std::sin(KURO_TWO_PI * freq * t) + 0.5f * std::sin(KURO_TWO_PI * freq * 2.0f * t);
                wave = std::tanh(raw * 2.5f); // Overdrive / Saturação
                env = std::exp(-3.0f * t);
                break;
            }

            // --- SQUARE CHIPTUNE LEAD: Onda quadrada pura 50% duty ---
            case MidiInstrument::SQUARE_LEAD: {
                wave = (std::fmod(freq * t, 1.0f) < 0.5f) ? 0.7f : -0.7f;
                env = std::exp(-4.0f * t);
                break;
            }

            // --- BRASS SYNTH: Metais analógicos encorpados ---
            case MidiInstrument::BRASS_SYNTH: {
                float s1 = 2.0f * std::fmod(freq * t, 1.0f) - 1.0f;
                float s2 = 2.0f * std::fmod(freq * 1.002f * t, 1.0f) - 1.0f;
                wave = (s1 + s2) * 0.40f;
                float att = 0.08f;
                env = (t < att) ? (t / att) : 1.0f;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 5.0f);
                break;
            }

            // --- DELAY LAMA: SÍNTESE VOCAL POR FILTROS DE FORMANTES BANDPASS (2002 1:1) ---
            case MidiInstrument::DELAY_LAMA: {
                float vx = std::clamp(g_delay_lama_vowel_x, 0.0f, 1.0f);
                
                // Transposição para a Oitava de Monge (C3-C5, 130Hz - 520Hz)
                float vocal_pitch_freq = freq;
                while (vocal_pitch_freq < 130.0f) vocal_pitch_freq *= 2.0f;
                while (vocal_pitch_freq > 520.0f) vocal_pitch_freq *= 0.5f;

                // Pitch Bend pelo XY Pad (-12 a +12 semitones)
                float eff_freq = vocal_pitch_freq * std::pow(2.0f, (g_delay_lama_pitch_y * 12.0f) / 12.0f);

                // 1. Vibrato Místico Vocal Meditativo (4.5 Hz)
                float vib = 1.0f + 0.018f * std::sin(KURO_TWO_PI * 4.5f * t);
                eff_freq *= vib;

                // 2. Excitação Dente de Serra Glótica Nítida
                float p = std::fmod(eff_freq * t, 1.0f);
                float saw = 2.0f * p - 1.0f;

                // 3. Sub-oscilador de Garganta Gutural Kargyraa (Sub-oitava f0/2)
                float sub_throat = std::sin(KURO_TWO_PI * (eff_freq * 0.5f) * t) * 0.40f;

                float exciter = saw * 0.65f + sub_throat * 0.35f;

                // 4. Mapeamento Preciso de Formantes de Vogais Vocais (OOH -> OW -> AH -> AYH -> EEH)
                float f1, f2;
                if (vx < 0.25f) {
                    float norm = vx / 0.25f;
                    f1 = 350.0f + norm * 150.0f; // 350Hz -> 500Hz (OOH -> OW)
                    f2 = 750.0f + norm * 250.0f; // 750Hz -> 1000Hz
                } else if (vx < 0.50f) {
                    float norm = (vx - 0.25f) / 0.25f;
                    f1 = 500.0f + norm * 320.0f; // 500Hz -> 820Hz (OW -> AH)
                    f2 = 1000.0f + norm * 300.0f; // 1000Hz -> 1300Hz
                } else if (vx < 0.75f) {
                    float norm = (vx - 0.50f) / 0.25f;
                    f1 = 820.0f - norm * 220.0f; // 820Hz -> 600Hz (AH -> AYH)
                    f2 = 1300.0f + norm * 600.0f; // 1300Hz -> 1900Hz
                } else {
                    float norm = (vx - 0.75f) / 0.25f;
                    f1 = 600.0f - norm * 250.0f; // 600Hz -> 350Hz (AYH -> EEH)
                    f2 = 1900.0f + norm * 500.0f; // 1900Hz -> 2400Hz
                }

                // 5. Filtro de Formantes Ressonante Bandpass (SVF 2ª Ordem - Sem retificação motorizada!)
                static float svf1_b = 0.0f, svf1_l = 0.0f;
                static float svf2_b = 0.0f, svf2_l = 0.0f;
                float q1 = 5.5f, q2 = 7.0f;

                float omega1 = std::clamp(2.0f * std::sin(KURO_PI * f1 / sample_rate), 0.01f, 0.99f);
                float omega2 = std::clamp(2.0f * std::sin(KURO_PI * f2 / sample_rate), 0.01f, 0.99f);

                // Processamento F1
                svf1_l += omega1 * svf1_b;
                float high1 = exciter - svf1_l - (1.0f / q1) * svf1_b;
                svf1_b += omega1 * high1;

                // Processamento F2
                svf2_l += omega2 * svf2_b;
                float high2 = exciter - svf2_l - (1.0f / q2) * svf2_b;
                svf2_b += omega2 * high2;

                // Sinal Vocal Nítido Pronunciando Vogais "OOOH - AAAH - YEAOUWW"
                wave = std::clamp((svf1_b * 0.65f + svf2_b * 0.35f) * 2.2f, -1.0f, 1.0f);

                // 6. Envelope Vocal Suave
                env = (t < 0.02f) ? (t / 0.02f) : 1.0f;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 4.0f);
                break;
            }

            // --- ALIEN LED SYNTH: SINTETIZADOR AUTORAL SCI-FI (O som que o usuário gostou!) ---
            case MidiInstrument::ALIEN_LED_SYNTH: {
                float eff_freq = freq;
                float p = std::fmod(eff_freq * t, 1.0f);
                float pulse = (p < 0.35f) ? 0.75f : -0.75f;
                float ring = std::sin(KURO_TWO_PI * (eff_freq * 1.5f) * t) * 0.35f;
                float mod = std::sin(KURO_TWO_PI * 4.0f * t) * 0.05f;
                wave = (pulse + ring) * (1.0f + mod);
                env = (t < 0.01f) ? (t / 0.01f) : 1.0f;
                if (t > duration) env = std::max(0.0f, 1.0f - (t - duration) * 4.0f);
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
        MidiInstrument flex_channel_instrument[MAX_TRACKS] = {
            MidiInstrument::ACOUSTIC_PIANO,   // Ch 0: Kick
            MidiInstrument::SUB_BASS,         // Ch 1: Sub Bass
            MidiInstrument::SYNTH_SAW,        // Ch 2: Mid Bass
            MidiInstrument::ACOUSTIC_PIANO,   // Ch 3: Snare/Clap
            MidiInstrument::ACOUSTIC_PIANO,   // Ch 4: Open Hat
            MidiInstrument::ACOUSTIC_PIANO,   // Ch 5: Closed Hat
            MidiInstrument::MARIMBA,          // Ch 6: Tribal Perc
            MidiInstrument::FM_SYNTH,         // Ch 7: FM Squelch
            MidiInstrument::SYNTH_SAW,        // Ch 8: Acid Lead
            MidiInstrument::TRANCE_LEAD,      // Ch 9: Counter-Arp Pluck
            MidiInstrument::FULLON_LEAD,      // Ch 10: SuperSaw Lead
            MidiInstrument::PAD_SYNTH,        // Ch 11: Dark Sub Drone
            MidiInstrument::STRING_ENSEMBLE,  // Ch 12: Symphonic Strings
            MidiInstrument::ACOUSTIC_PIANO,   // Ch 13: Steinway Piano
            MidiInstrument::CHOIR,            // Ch 14: Vocal Mantra Chants
            MidiInstrument::SWEEP_PAD         // Ch 15: Snare Roll & FX
        };
        SamplerSettings sampler_settings[MAX_TRACKS];
        FlexSettings flex_settings[MAX_TRACKS];
        bool flex_active[MAX_TRACKS] = { false, true, true, false, false, false, true, true, true, true, true, true, true, true, true, true };

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
        DrumSample drum_variants[MAX_TRACKS][MAX_DRUM_VARIANTS];  // 20 canais × 4 variantes
        int selected_variant[MAX_TRACKS] = { 0 };                 // Variante ativa por canal
        DrumSample* drum_samples = nullptr; // Aponta para drum_variants[ch][selected_variant[ch]]

        // Mantidos para compatibilidade — redirecionam para drum_variants de forma 100% segura
        DrumSample& getDrumSample(int ch) {
            static DrumSample s_empty_drum;
            if (ch < 0 || ch >= MAX_TRACKS) return s_empty_drum;
            int v = std::clamp(selected_variant[ch], 0, MAX_DRUM_VARIANTS - 1);
            return drum_variants[ch][v];
        }

        bool loadTrackSample(int track_idx, const std::string& filepath) {
            if (track_idx < 0 || track_idx >= MAX_TRACKS) return false;
            int v = std::clamp(selected_variant[track_idx], 0, MAX_DRUM_VARIANTS - 1);
            drum_variants[track_idx][v].load(filepath);
            bool ok = drum_variants[track_idx][v].loaded;
            if (ok) {
                flex_active[track_idx] = false;
                std::string fname = std::filesystem::path(filepath).filename().string();
                if (fname.find("_A1") != std::string::npos || fname.find(" A1") != std::string::npos) sampler_settings[track_idx].root_note = 33;
                else if (fname.find("_A2") != std::string::npos || fname.find(" A2") != std::string::npos) sampler_settings[track_idx].root_note = 45;
                else if (fname.find("_A3") != std::string::npos || fname.find(" A3") != std::string::npos) sampler_settings[track_idx].root_note = 57;
                else if (fname.find("_A4") != std::string::npos || fname.find(" A4") != std::string::npos) sampler_settings[track_idx].root_note = 69;
                else if (fname.find("_A") != std::string::npos) sampler_settings[track_idx].root_note = 57;
                else if (fname.find("_C2") != std::string::npos || fname.find(" C2") != std::string::npos) sampler_settings[track_idx].root_note = 36;
                else if (fname.find("_C3") != std::string::npos || fname.find(" C3") != std::string::npos) sampler_settings[track_idx].root_note = 48;
                else if (fname.find("_C4") != std::string::npos || fname.find(" C4") != std::string::npos) sampler_settings[track_idx].root_note = 60;
                else if (fname.find("_C5") != std::string::npos || fname.find(" C5") != std::string::npos) sampler_settings[track_idx].root_note = 72;
                else if (fname.find("_F#1") != std::string::npos || fname.find(" F#1") != std::string::npos) sampler_settings[track_idx].root_note = 30;
                else if (fname.find("_F#2") != std::string::npos || fname.find(" F#2") != std::string::npos) sampler_settings[track_idx].root_note = 42;
                else if (fname.find("_G1") != std::string::npos || fname.find(" G1") != std::string::npos) sampler_settings[track_idx].root_note = 31;
                else if (fname.find("_G2") != std::string::npos || fname.find(" G2") != std::string::npos) sampler_settings[track_idx].root_note = 43;
            }
            return ok;
        }

        DrumSample strings_sample;
        DrumSample guitar_sample;
        DrumSample cello_sample;
        DrumSample harpsichord_sample;
        DrumSample choir_sample;
        DrumSample piano_sample;
        DrumSample bass_sample;
        DrumSample pad_sample;

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
            auto load_sample_file = [](DrumSample& ds, const std::string& local_path, const std::string& fallback_fl_path) {
                ds.load(local_path);
                if (!ds.loaded && !fallback_fl_path.empty()) {
                    ds.load(fallback_fl_path);
                }
            };

            // ── Kicks (canal 0) ────
            load_sample_file(drum_variants[0][0], "assets/samples/adhana_signature/Adhana_Astrix_Kick_Punch_01.wav", "assets/samples/Psytrance_Kick_140BPM.wav");
            load_sample_file(drum_variants[0][1], "assets/samples/Psytrance_Kick_140BPM.wav", "assets/samples/Real_Drums/Punch_909_Kick.wav");
            load_sample_file(drum_variants[0][2], "assets/samples/Real_Drums/Punch_909_Kick.wav", std::string(FL_KICKS_DIR) + "Monster Kick 002.wav");
            load_sample_file(drum_variants[0][3], "assets/samples/Slap_Punch_Kick.wav", std::string(FL_KICKS_DIR) + "909 Kick.wav");

            // ── Snares (canal 1) ───
            load_sample_file(drum_variants[1][0], "assets/samples/Real_Drums/Real_Acoustic_Snare.wav", std::string(FL_SNARES_DIR) + "Grv Snareclap 01.wav");
            load_sample_file(drum_variants[1][1], "assets/samples/Real_Drums/Real_808_Snare.wav", std::string(FL_SNARES_DIR) + "808 Snare.wav");
            load_sample_file(drum_variants[1][2], "assets/samples/Real_Drums/Studio_Snare_Warm.wav", std::string(FL_SNARES_DIR) + "FPC Snare 1.wav");
            load_sample_file(drum_variants[1][3], "assets/samples/808_Crisp_Clap.wav", std::string(FL_SNARES_DIR) + "909 Snare.wav");

            // ── Hi-Hats (canal 2) ─
            load_sample_file(drum_variants[2][0], "assets/samples/Real_Drums/Real_Acoustic_HiHat_Closed.wav", std::string(FL_HATS_DIR) + "Grv CH 01.wav");
            load_sample_file(drum_variants[2][1], "assets/samples/Real_Drums/Real_Acoustic_HiHat_Open.wav", std::string(FL_HATS_DIR) + "808 OH.wav");
            load_sample_file(drum_variants[2][2], "assets/samples/Closed_Metal_Hat.wav", std::string(FL_HATS_DIR) + "808 CH.wav");
            load_sample_file(drum_variants[2][3], "assets/samples/Open_Psy_Hat.wav", std::string(FL_HATS_DIR) + "909 CH 1.wav");

            // ── Clap / Rim (canal 3) ────
            load_sample_file(drum_variants[3][0], "assets/samples/Real_Drums/Real_Acoustic_Snare.wav", std::string(FL_SNARES_DIR) + "707 Rim.wav");
            load_sample_file(drum_variants[3][1], "assets/samples/808_Crisp_Clap.wav", std::string(FL_SNARES_DIR) + "909 Rim.wav");
            load_sample_file(drum_variants[3][2], "assets/samples/Ghost_Snare_Click.wav", std::string(FL_SNARES_DIR) + "FPC Rim.wav");
            load_sample_file(drum_variants[3][3], "assets/samples/EDM_Smash_Clap.wav", std::string(FL_SNARES_DIR) + "Stick Rim 1.wav");

            // ── Open Hat / Cymbals (canal 4) 
            load_sample_file(drum_variants[4][0], "assets/samples/Real_Drums/Real_Acoustic_HiHat_Open.wav", std::string(FL_HATS_DIR) + "808 OH.wav");
            load_sample_file(drum_variants[4][1], "assets/samples/Open_Psy_Hat.wav", std::string(FL_HATS_DIR) + "909 OH.wav");
            load_sample_file(drum_variants[4][2], "assets/samples/Real_Drums/Real_Acoustic_HiHat_Closed.wav", std::string(FL_HATS_DIR) + "Grv OH 01.wav");
            load_sample_file(drum_variants[4][3], "assets/samples/Shaker_Groove_High.wav", std::string(FL_HATS_DIR) + "AMX OH.wav");

            // ── Ride (canal 5) ─────
            load_sample_file(drum_variants[5][0], "assets/samples/Real_Drums/Real_Acoustic_Ride.wav", std::string(FL_CYM_DIR) + "Grv Ride 01.wav");
            load_sample_file(drum_variants[5][1], "assets/samples/Real_Drums/Real_Acoustic_Crash.wav", std::string(FL_CYM_DIR) + "909 Ride.wav");
            load_sample_file(drum_variants[5][2], "assets/samples/Real_Drums/Real_Acoustic_HiHat_Open.wav", std::string(FL_CYM_DIR) + "707 Ride.wav");
            load_sample_file(drum_variants[5][3], "assets/samples/Closed_Metal_Hat.wav", std::string(FL_CYM_DIR) + "Linn Ride.wav");

            // ── Crash (canal 6) ───
            load_sample_file(drum_variants[6][0], "assets/samples/Real_Drums/Real_Acoustic_Crash.wav", std::string(FL_CYM_DIR) + "Grv Crash 01.wav");
            load_sample_file(drum_variants[6][1], "assets/samples/Real_Drums/Real_Acoustic_Ride.wav", std::string(FL_CYM_DIR) + "909 Crash.wav");
            load_sample_file(drum_variants[6][2], "assets/samples/Cyber_Sub_Impact.wav", std::string(FL_CYM_DIR) + "808 Crash.wav");
            load_sample_file(drum_variants[6][3], "assets/samples/White_Noise_Sweep_Up.wav", std::string(FL_CYM_DIR) + "Thin Crash.wav");

            // ── Perc / SFX (canal 7) 
            load_sample_file(drum_variants[7][0], "assets/samples/Real_SFX/FX_Choir_Swell.wav", std::string(FL_HATS_DIR) + "Clank CH 1.wav");
            load_sample_file(drum_variants[7][1], "assets/samples/Real_SFX/SFX_Bass_Sweep.wav", std::string(FL_HATS_DIR) + "Ice Hat 1.wav");
            load_sample_file(drum_variants[7][2], "assets/samples/Psy_Click_Perc.wav", std::string(FL_HATS_DIR) + "Ice Hat 3.wav");
            load_sample_file(drum_variants[7][3], "assets/samples/Psy_Zap_Laser.wav", std::string(FL_HATS_DIR) + "Jung Hat 1.wav");

            // ── SFX Library ───────────────────────────────────────────────
            const std::vector<std::pair<std::string,std::string>> sfx_files = {
                {"Choir Swell",    "assets/samples/Real_SFX/FX_Choir_Swell.wav"},
                {"Bass Sweep Drop","assets/samples/Real_SFX/SFX_Bass_Sweep.wav"},
                {"Drone Strings",  "assets/samples/Real_SFX/FX_Drone_Strings_Rev.wav"},
                {"Cyber Impact",   "assets/samples/Cyber_Sub_Impact.wav"},
                {"Noise Sweep",    "assets/samples/White_Noise_Sweep_Up.wav"},
                {"Downlifter",     "assets/samples/Alien_Downlifter_Sweep.wav"},
                {"303 Saw Hit",    "assets/samples/Acid_303_Saw_Hit.wav"},
                {"Blackhole",      std::string(FL_SFX_DIR) + "FX Blackhole.wav"},
                {"Mech Growl",     std::string(FL_SFX_DIR) + "FX Mech Growl.wav"},
                {"SubBass Drop",   std::string(FL_SFX_DIR) + "SFX SubBass Drop.wav"},
            };
            for (auto& [name, path] : sfx_files) {
                sfx_library.push_back({name, {}});
                sfx_library.back().sample.load(path);
            }

            // ── Instrumentos Cromáticos Reais (Sample Bank Enriquecido) ───────────
            load_sample_file(strings_sample, "assets/samples/Real_Strings/Symphonic_Strings_Section.wav", "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Strings\\STR_3c_Long.wav");
            load_sample_file(guitar_sample, "assets/samples/Real_Guitar/ACOUSTICG 1_A3.wav", "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Guitar\\Guitar\\Acoustic Guitar 01\\ACOUSTICG 1_A3.wav");
            load_sample_file(cello_sample, "assets/samples/Real_Strings/Real_Cello_Solo.wav", "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Strings\\STR_Cell_C4.wav");
            load_sample_file(harpsichord_sample, "assets/samples/Real_Strings/Real_Harpsichord.wav", "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Strings\\STR_Harpsy_C4.wav");
            load_sample_file(choir_sample, "assets/samples/Real_Choir/Real_Choir_Aah_A3.wav", "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Choirs\\CHR_Aah_A3.wav");
            load_sample_file(piano_sample, "assets/samples/Real_Piano/Gz_C4ogg.wav", "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Piano\\Piano 2\\Gz_C4ogg.wav");
            load_sample_file(bass_sample, "assets/samples/Real_Bass/Real_Bass_Deep_C2.wav", "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Bass\\BASS_EfEm_C2.wav");
            load_sample_file(pad_sample, "assets/samples/Real_Strings/Orchestral_Pad_Strings.wav", "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\Legacy\\Instruments\\Strings\\DNC_OrionString.wav");

            // Configurar Notas Raiz padrão para baterias e instrumentos cromáticos
            for (int i = 0; i < MAX_TRACKS; i++) {
                sampler_settings[i].track = i + 1;
                sampler_settings[i].root_note = 60; // Padrão C5
            }
            sampler_settings[0].root_note = 36; // C2 (Kick)
            sampler_settings[1].root_note = 38; // D2 (Snare)
            sampler_settings[2].root_note = 42; // F#2 (HiHat)
            sampler_settings[3].root_note = 39; // D#2 (Clap)
            sampler_settings[4].root_note = 46; // A#2 (OpenHat)
            sampler_settings[6].root_note = 49; // C#3 (Crash)
            sampler_settings[7].root_note = 51; // D#3 (Ride)
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
            if (active_voices.size() >= 64) {
                active_voices.erase(active_voices.begin());
            }
            if (track_idx >= 0 && track_idx < 8) {
                if (sampler_settings[track_idx].cut_self) {
                    for (auto& v : active_voices) {
                        if (v.track_idx == track_idx) {
                            v.active = false;
                        }
                    }
                }
            }
            active_voices.push_back({pitch, 0.0f, 0.0f, duration, velocity, true, 0.0f, track_idx});
        }

        void releaseNote(int pitch) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            for (auto& voice : active_voices) {
                if (voice.pitch == pitch && voice.current_time < voice.duration) {
                    voice.duration = voice.current_time;
                }
            }
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

                 // Processa vozes ativas
                 for (auto& voice : active_voices) {
                     if (!voice.active) continue;
 
                     float freq = getFrequency(voice.pitch);
                     float phase_inc = freq / sample_rate;
                     
                     if (voice.current_time >= voice.duration + 0.5f) { // +0.5s para release
                         voice.active = false;
                         continue;
                     }
 
                     // Use track_idx to determine drum sample vs synth channel
                     float s = 0.0f;
                     int drum_idx = -1;
                     int sfx_idx = -1;
                     int ch_idx = std::clamp(voice.track_idx, 0, 7);
                     
                     if (voice.pitch < -90) {
                         sfx_idx = -100 - voice.pitch;
                     } else if (ch_idx == 0 || ch_idx == 1 || ch_idx == 2 || ch_idx == 6 || ch_idx == 7) {
                         drum_idx = ch_idx; // Drum sampler channels: Kick (0), Snare (1), HiHat (2), Clap (6), OpenHat (7)
                     } else if (getDrumSample(ch_idx).loaded && !flex_active[ch_idx]) {
                         drum_idx = ch_idx;
                     }

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
                     } else if (drum_idx >= 0 && drum_idx < 8) {
                          auto& ss = sampler_settings[drum_idx];
                          
                          // Pitch calculation
                          float pitch_st = ss.pitch + (voice.pitch - ss.root_note) + ss.time_pitch + (ss.fine_tune / 100.0f);
                          float p_ratio = std::pow(2.0f, pitch_st / 12.0f) * ss.time_mul;
                          if (p_ratio < 0.05f) p_ratio = 0.05f;
                          
                          if (getDrumSample(drum_idx).loaded) {
                              auto& ds = getDrumSample(drum_idx);
                              uint64_t start_frame = (uint64_t)(ss.smp_start * ds.total_frames);
                              uint64_t avail_frames = (ds.total_frames > start_frame) ? (ds.total_frames - start_frame) : 0;
                              if (ss.length < 0.999f) {
                                  avail_frames = (uint64_t)(ss.length * avail_frames);
                              }
                              
                              double exact_play_frame = (double)voice.current_time * (double)ds.sample_rate * (double)p_ratio;
                              uint64_t play_frame = (uint64_t)exact_play_frame;
                              if (ss.reverse && avail_frames > 0) {
                                  play_frame = (avail_frames > play_frame) ? (avail_frames - play_frame) : 0;
                              }
                              
                              uint64_t target_frame = start_frame + play_frame;
                              if (target_frame < ds.total_frames && avail_frames > 0) {
                                  double frac = exact_play_frame - (double)((uint64_t)exact_play_frame);
                                  uint64_t next_frame = (target_frame + 1 < ds.total_frames) ? target_frame + 1 : target_frame;
                                  if (ds.channels == 1) {
                                      float s0 = ds.sample_data[target_frame];
                                      float s1 = ds.sample_data[next_frame];
                                      s = s0 + (float)frac * (s1 - s0);
                                  } else if (ds.channels >= 2) {
                                      float s0 = ds.sample_data[target_frame * ds.channels];
                                      float s1 = ds.sample_data[next_frame * ds.channels];
                                      s = s0 + (float)frac * (s1 - s0);
                                  }
                              } else {
                                  s = 0.0f;
                              }
                              
                              // Fade In / Fade Out
                              if (ss.fade_in > 0.001f) {
                                  float fade_in_sec = ss.fade_in * 0.5f;
                                  if (voice.current_time < fade_in_sec) s *= (voice.current_time / fade_in_sec);
                              }
                              if (ss.fade_out > 0.001f && avail_frames > 0) {
                                  float total_dur = (float)avail_frames / (ds.sample_rate * p_ratio);
                                  float rem = total_dur - voice.current_time;
                                  if (rem < ss.fade_out) s *= std::max(0.0f, rem / ss.fade_out);
                              }
                          } else {
                               // Math synthesis fallbacks with pitch & reverse
                               float t = voice.current_time * p_ratio;
                               if (ss.reverse) t = std::max(0.0f, voice.duration - t);

                               if (drum_idx == 0) { // Kick
                                   float pitch_env = std::exp(-60.0f * t);
                                   float kick_freq = (50.0f + 150.0f * pitch_env);
                                   s = std::sin(KURO_TWO_PI * kick_freq * t) * std::exp(-10.0f * t) * 1.3f;
                                   s += 0.2f * std::sin(KURO_TWO_PI * 1000.0f * t) * std::exp(-120.0f * t);
                               } else if (drum_idx == 1) { // Snare
                                   static thread_local uint32_t rand_seed = 12345;
                                   rand_seed = rand_seed * 196314165 + 907633385;
                                   float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                                   float tone = std::sin(KURO_TWO_PI * 180.0f * t) * std::exp(-35.0f * t);
                                   s = (tone * 0.35f + noise * 0.65f) * std::exp(-12.0f * t) * 1.0f;
                               } else if (drum_idx == 2) { // Hihat
                                   static thread_local uint32_t rand_seed = 54321;
                                   rand_seed = rand_seed * 196314165 + 907633385;
                                   float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                                   s = noise * std::exp(-70.0f * t) * 0.8f;
                               } else if (drum_idx == 6) { // Clap
                                   static thread_local uint32_t rand_seed = 98765;
                                   rand_seed = rand_seed * 196314165 + 907633385;
                                   float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                                   float env = (t < 0.01f) ? 1.0f : ((t < 0.02f) ? 0.8f : ((t < 0.03f) ? 0.6f : std::exp(-15.0f * (t - 0.03f))));
                                   s = noise * env * 0.7f;
                               } else if (drum_idx == 7) { // Open Hat
                                   static thread_local uint32_t rand_seed = 65432;
                                   rand_seed = rand_seed * 196314165 + 907633385;
                                   float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                                   s = noise * std::exp(-8.0f * t) * 0.7f;
                               } else {
                                   float pitch_env = std::exp(-30.0f * t);
                                   float tom_freq = (80.0f + 70.0f * pitch_env);
                                   s = std::sin(KURO_TWO_PI * tom_freq * t) * std::exp(-6.0f * t) * 1.1f;
                               }
                          }
                          
                          // Reverse Polarity
                          if (ss.rev_polarity) s = -s;
                          
                          // ADSR Envelope
                          if (ss.env_enabled) {
                              s *= getSamplerEnvelope(ss, voice.current_time, voice.duration);
                          }
                          
                          // Filter Cutoff & Resonance
                          if (ss.filter_cutoff < 0.99f || ss.filter_res > 0.01f) {
                              float fc = ss.filter_cutoff * 16000.0f + 40.0f;
                              float res_q = 1.0f + ss.filter_res * 6.0f;
                              float w0 = KURO_TWO_PI * fc / 44100.0f;
                              float alpha = std::sin(w0) / (2.0f * res_q);
                              float b0 = (1.0f - std::cos(w0)) * 0.5f;
                              float a0 = 1.0f + alpha;
                              
                              voice.filter_low = voice.filter_low + alpha * voice.filter_band;
                              float high = (s * b0 / a0) - voice.filter_low - (alpha * voice.filter_band);
                              voice.filter_band = voice.filter_band + alpha * high;
                              s = voice.filter_low;
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
                              MidiInstrument target_inst = (voice.track_idx >= 0 && voice.track_idx < MAX_TRACKS) ? flex_channel_instrument[voice.track_idx] : current_instrument;

                              DrumSample* ds = nullptr;
                              int base_pitch = 60;
                              bool loop = false;

                              if (target_inst == MidiInstrument::ACOUSTIC_PIANO || target_inst == MidiInstrument::HONKY_TONK) {
                                  ds = &piano_sample; base_pitch = 60; loop = false;
                              } else if (target_inst == MidiInstrument::ELECTRIC_BASS || target_inst == MidiInstrument::SUB_BASS) {
                                  ds = &bass_sample; base_pitch = 36; loop = false;
                              } else if (target_inst == MidiInstrument::PAD_SYNTH || target_inst == MidiInstrument::SWEEP_PAD) {
                                  ds = &pad_sample; base_pitch = 48; loop = true;
                              } else if (target_inst == MidiInstrument::STRING_ENSEMBLE || target_inst == MidiInstrument::VIOLIN) {
                                  ds = &strings_sample; base_pitch = 48; loop = true;
                              } else if (target_inst == MidiInstrument::NYLON_GUITAR || target_inst == MidiInstrument::STEEL_GUITAR) {
                                  ds = &guitar_sample; base_pitch = 57; loop = false;
                              } else if (target_inst == MidiInstrument::CELLO) {
                                  ds = &cello_sample; base_pitch = 60; loop = true;
                              } else if (target_inst == MidiInstrument::HARPSICHORD || target_inst == MidiInstrument::CLAVINET) {
                                  ds = &harpsichord_sample; base_pitch = 60; loop = false;
                              } else if (target_inst == MidiInstrument::MARIMBA || target_inst == MidiInstrument::CELESTA || target_inst == MidiInstrument::CHOIR) {
                                  ds = &choir_sample; base_pitch = 57; loop = true;
                              }

                              if (ds && ds->loaded && !ds->sample_data.empty()) {
                                  float ratio = std::pow(2.0f, (voice.pitch - base_pitch) / 12.0f);
                                  uint64_t frame_idx = (uint64_t)(voice.current_time * ds->sample_rate * ratio);
                                  if (loop) {
                                      if (ds->total_frames > 0) {
                                          frame_idx = frame_idx % ds->total_frames;
                                          size_t sample_idx = frame_idx * ds->channels;
                                          if (sample_idx < ds->sample_data.size()) {
                                              s = ds->sample_data[sample_idx];
                                          }
                                      }
                                  } else {
                                      if (frame_idx < ds->total_frames) {
                                          size_t sample_idx = frame_idx * ds->channels;
                                          if (sample_idx < ds->sample_data.size()) {
                                              s = ds->sample_data[sample_idx];
                                          }
                                      }
                                  }
                                  float env = 1.0f;
                                  if (voice.current_time > voice.duration) {
                                      env = std::max(0.0f, 1.0f - (voice.current_time - voice.duration) * 4.0f);
                                  }
                                  s *= env;
                              } else {
                                  MidiInstrument inst = (voice.track_idx >= 0 && voice.track_idx < MAX_TRACKS && flex_active[voice.track_idx]) 
                                                        ? flex_channel_instrument[voice.track_idx] 
                                                        : current_instrument;
                                   s = synthesize(inst, freq, voice.current_time, voice.phase, voice.duration);

                                   // Aplicar Envelope ADSR e Filtro de Equalização Suave do FLEX se ativo
                                   int target_ch = (voice.track_idx >= 0 && voice.track_idx < MAX_TRACKS) ? voice.track_idx : 0;
                                   if (flex_active[target_ch]) {
                                       auto& fs = flex_settings[target_ch];
                                       
                                       // Envelope ADSR
                                       float env = 1.0f;
                                       float t_sec = voice.current_time;
                                       float att = std::max(0.001f, fs.env_vol_a);
                                       float dec = std::max(0.01f, fs.env_vol_d);
                                       float sus = std::clamp(fs.env_vol_s, 0.0f, 1.0f);
                                       float rel = std::max(0.01f, fs.env_vol_r);
                                       
                                       if (t_sec < att) {
                                           env = t_sec / att;
                                       } else if (t_sec < att + dec) {
                                           float dt_vol = (t_sec - att) / dec;
                                           env = 1.0f - dt_vol * (1.0f - sus);
                                       } else {
                                           env = sus;
                                       }
                                       if (t_sec > voice.duration) {
                                           float rt = t_sec - voice.duration;
                                           env = sus * std::max(0.0f, 1.0f - rt / rel);
                                       }
                                       s *= env;

                                       // Filtro IIR Suave para eliminar agudos ásperos / estourados
                                       float fc = std::clamp(fs.filter_cutoff, 0.05f, 1.0f) * 12500.0f + 50.0f;
                                       float res_q = 0.707f + fs.filter_res * 3.5f;
                                       float w0 = KURO_TWO_PI * fc / 44100.0f;
                                       float alpha = std::sin(w0) / (2.0f * res_q);
                                       float b0 = (1.0f - std::cos(w0)) * 0.5f;
                                       float a0 = 1.0f + alpha;

                                       voice.filter_low = voice.filter_low + alpha * voice.filter_band;
                                       float high = (s * b0 / a0) - voice.filter_low - (alpha * voice.filter_band);
                                       voice.filter_band = voice.filter_band + alpha * high;
                                       s = voice.filter_low;
                                   }
                              }
                          }
                     }
 
                     int trk = (voice.track_idx >= 0 && voice.track_idx < 8) ? voice.track_idx : 5;
                     if (drum_idx >= 0 && drum_idx < 8) {
                         auto& ss = sampler_settings[drum_idx];
                         if (ss.track >= 1 && ss.track <= 20) {
                             trk = ss.track - 1;
                         } else {
                             trk = channel_tracks[drum_idx];
                         }
                     }
 
                     float voice_gain = ::is_playing ? ::track_linear_volumes[trk] : 1.0f;
                     float channel_gain = 1.0f;
                     float pan_l = 1.0f;
                     float pan_r = 1.0f;
                     
                     if (drum_idx >= 0 && drum_idx < 8) {
                         auto& ss = sampler_settings[drum_idx];
                         channel_gain = ss.vol * ss.misc_vol * ::dummy_vol[drum_idx];
                         float p = std::clamp(ss.pan + ss.misc_pan + ::dummy_pan[drum_idx], -1.0f, 1.0f);
                         pan_l = std::min(1.0f, 1.0f - p);
                         pan_r = std::min(1.0f, 1.0f + p);

                         // De-clicking micro-fade (2.0ms attack & 2.5ms release)
                         constexpr float declick_att_sec = 0.0020f;
                         if (voice.current_time < declick_att_sec) {
                             s *= (voice.current_time / declick_att_sec);
                         }
                         float rem_dur = voice.duration - voice.current_time;
                         constexpr float declick_rel_sec = 0.0025f;
                         if (rem_dur < declick_rel_sec && rem_dur >= 0.0f) {
                             s *= std::max(0.0f, rem_dur / declick_rel_sec);
                         }

                     } else if (drum_idx >= 0) {
                         channel_gain = ::dummy_vol[drum_idx];
                         float p = ::dummy_pan[drum_idx];
                         pan_l = std::min(1.0f, 1.0f - p);
                         pan_r = std::min(1.0f, 1.0f + p);
                     }
 
                     float base_sample = s * voice.velocity * 0.25f * voice_gain * channel_gain;
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

                out_left[i] += sample_l;
                out_right[i] += sample_r;
            }

            // Limpa vozes inativas uma única vez por bloco de áudio (NÃO por amostra)
            active_voices.erase(std::remove_if(active_voices.begin(), active_voices.end(), 
                [](const Voice& v) { return !v.active; }), active_voices.end());

            for (int t = 0; t < 8; t++) {
                if (t != 5) {
                    ::track_vu_levels[t] = ::track_vu_levels[t] * 0.8f + block_peaks[t] * 0.2f;
                }
            }
        }

        void processMultitrack(float** track_outs_l, float** track_outs_r, unsigned int nFrames, float global_time_sec) {
            std::lock_guard<std::mutex> lock(synth_mutex);
            
            float block_peaks[MAX_TRACKS] = { 0.0f };
            float dt = 1.0f / sample_rate;
            
            for (unsigned int i = 0; i < nFrames; i++) {
                float current_time = global_time_sec + (i * dt);

                // Processa vozes ativas
                for (auto& voice : active_voices) {
                    if (!voice.active) continue;

                    float freq = getFrequency(voice.pitch);
                    
                    // Modulação de Vibrato via Macro do FLEX
                    int ch_idx = std::clamp(voice.track_idx, 0, 19);
                    auto& fs = flex_settings[ch_idx];
                    
                    if (fs.macro_vibrato > 0.01f) {
                        float vibrato_lfo = std::sin(KURO_TWO_PI * 6.0f * voice.current_time) * 0.03f * fs.macro_vibrato;
                        freq *= (1.0f + vibrato_lfo);
                    }
                    
                    float phase_inc = freq / sample_rate;
                    
                    int ch_idx_mt = std::clamp(voice.track_idx, 0, 19);
                    int drum_idx = -1;
                    int sfx_idx = -1;
                    
                    if (voice.pitch < -90) {
                        sfx_idx = -100 - voice.pitch;
                    } else if (getDrumSample(ch_idx_mt).loaded || !flex_active[ch_idx_mt]) {
                        drum_idx = ch_idx_mt; // Se o canal tem sample carregado, ou não tem flex, toca no sampler cromático!
                    }

                    // Extensão inteligente do tempo de voz para samples one-shot
                    float max_voice_dur = voice.duration + 0.5f;
                    if (drum_idx >= 0 && getDrumSample(drum_idx).loaded && !sampler_settings[drum_idx].env_enabled) {
                        auto& ds_dur = getDrumSample(drum_idx);
                        float pitch_st_dur = sampler_settings[drum_idx].pitch + (voice.pitch - sampler_settings[drum_idx].root_note);
                        float p_ratio_dur = std::pow(2.0f, pitch_st_dur / 12.0f);
                        if (p_ratio_dur < 0.05f) p_ratio_dur = 0.05f;
                        float sample_len_sec = (float)ds_dur.total_frames / (ds_dur.sample_rate * p_ratio_dur);
                        if (sample_len_sec + 0.15f > max_voice_dur) max_voice_dur = sample_len_sec + 0.15f;
                    }
                    
                    if (voice.current_time >= max_voice_dur) { // Respeita cauda completa do sample
                        voice.active = false;
                        continue;
                    }

                    float s_l = 0.0f;
                    float s_r = 0.0f;

                    if (sfx_idx >= 0 && sfx_idx < (int)sfx_library.size() && sfx_library[sfx_idx].sample.loaded) {
                        auto& ds = sfx_library[sfx_idx].sample;
                        uint64_t frame_idx = (uint64_t)(voice.current_time * ds.sample_rate);
                        if (frame_idx < ds.total_frames) {
                            if (ds.channels == 1) {
                                s_l = s_r = ds.sample_data[frame_idx];
                            } else if (ds.channels >= 2) {
                                s_l = ds.sample_data[frame_idx * ds.channels];
                                s_r = ds.sample_data[frame_idx * ds.channels + 1];
                            }
                        }
                    } else if (drum_idx >= 0 && getDrumSample(drum_idx).loaded) {
                        auto& ds = getDrumSample(drum_idx);
                        auto& ss = sampler_settings[drum_idx];

                        // Cálculo Cromático de Pitch Completo (FL Studio Style)
                        float pitch_st = ss.pitch + (voice.pitch - ss.root_note) + ss.time_pitch + (ss.fine_tune / 100.0f);
                        float p_ratio = std::pow(2.0f, pitch_st / 12.0f) * ss.time_mul;
                        if (p_ratio < 0.02f) p_ratio = 0.02f;

                        uint64_t start_frame = (uint64_t)(ss.smp_start * ds.total_frames);
                        uint64_t avail_frames = (ds.total_frames > start_frame) ? (ds.total_frames - start_frame) : 0;
                        if (ss.length < 0.999f) {
                            avail_frames = (uint64_t)(ss.length * avail_frames);
                        }

                        double exact_play_frame = (double)voice.current_time * (double)ds.sample_rate * (double)p_ratio;
                        uint64_t play_frame = (uint64_t)exact_play_frame;
                        if (ss.reverse && avail_frames > 0) {
                            play_frame = (avail_frames > play_frame) ? (avail_frames - play_frame) : 0;
                        }

                        uint64_t target_frame = start_frame + play_frame;
                        if (target_frame < ds.total_frames && avail_frames > 0) {
                            // Interpolação Linear de Alta Fidelidade True Stereo
                            double frac = exact_play_frame - (double)((uint64_t)exact_play_frame);
                            uint64_t next_frame = (target_frame + 1 < ds.total_frames) ? target_frame + 1 : target_frame;

                            if (ds.channels == 1) {
                                float s0 = ds.sample_data[target_frame];
                                float s1 = ds.sample_data[next_frame];
                                s_l = s_r = s0 + (float)frac * (s1 - s0);
                            } else if (ds.channels >= 2) {
                                float s0_l = ds.sample_data[target_frame * ds.channels];
                                float s1_l = ds.sample_data[next_frame * ds.channels];
                                s_l = s0_l + (float)frac * (s1_l - s0_l);

                                float s0_r = ds.sample_data[target_frame * ds.channels + 1];
                                float s1_r = ds.sample_data[next_frame * ds.channels + 1];
                                s_r = s0_r + (float)frac * (s1_r - s0_r);
                            }
                        } else {
                            s_l = s_r = 0.0f;
                        }

                        // Fade In / Fade Out do Sampler
                        if (ss.fade_in > 0.001f) {
                            float fade_in_sec = ss.fade_in * 0.5f;
                            if (voice.current_time < fade_in_sec) {
                                float f_in = (voice.current_time / fade_in_sec);
                                s_l *= f_in;
                                s_r *= f_in;
                            }
                        }
                        if (ss.fade_out > 0.001f && avail_frames > 0) {
                            float total_dur = (float)avail_frames / (ds.sample_rate * p_ratio);
                            float rem = total_dur - voice.current_time;
                            if (rem < ss.fade_out) {
                                float f_out = std::max(0.0f, rem / ss.fade_out);
                                s_l *= f_out;
                                s_r *= f_out;
                            }
                        }

                        // Ganho de volume e polaridade
                        s_l *= ss.vol;
                        s_r *= ss.vol;
                        if (ss.rev_polarity) {
                            s_l = -s_l;
                            s_r = -s_r;
                        }

                        // Envelope ADSR
                        if (ss.env_enabled) {
                            float adsr_smp = getSamplerEnvelope(ss, voice.current_time, voice.duration);
                            s_l *= adsr_smp;
                            s_r *= adsr_smp;
                        }
                    } else {
                        // Math synthesis fallbacks
                        float s = 0.0f;
                        float t = voice.current_time;
                        float p_mul = 1.0f;
                        float v_mul = 1.0f;
                        bool rev_pol = false;
                        if (drum_idx >= 0 && drum_idx < 8) {
                            auto& ss = sampler_settings[drum_idx];
                            t = ss.reverse ? std::max(0.0f, voice.duration - voice.current_time) : voice.current_time;
                            float pitch_st = ss.pitch + (voice.pitch - ss.root_note) + ss.time_pitch + (ss.fine_tune / 100.0f);
                            p_mul = std::pow(2.0f, pitch_st / 12.0f);
                            v_mul = ss.vol;
                            rev_pol = ss.rev_polarity;
                        }

                        if (drum_idx == 0) { // Psytrance Punch & Click Transient Kick ("Estalo")
                            // 1. Extreme Transient Click Snap (3.6 kHz -> 800 Hz sweep)
                            float click_env = std::exp(-400.0f * t);
                            float click_osc = std::sin(KURO_TWO_PI * (3500.0f - 2200.0f * (t * 250.0f)) * t) * click_env;
                            
                            // 2. High-Passed Noise Impulse Click
                            static thread_local uint32_t click_seed = 777;
                            click_seed = click_seed * 196314165 + 907633385;
                            float click_noise = (((float)click_seed / 4294967296.0f) - 0.5f) * std::exp(-750.0f * t);
                            
                            // 3. Mid Punch (150Hz - 80Hz sweep)
                            float punch_env = std::exp(-75.0f * t);
                            
                            // 4. Sub Body (52Hz fundamental sine with punchy decay)
                            float body_env = std::exp(-11.0f * t);
                            float kick_freq = (52.0f + 300.0f * punch_env) * p_mul;
                            float sub_body = std::sin(KURO_TWO_PI * kick_freq * t) * body_env;
                            
                            // 5. Saturated Analog Punch
                            float raw_kick = (sub_body * 1.35f) + (click_osc * 0.85f) + (click_noise * 0.45f);
                            s = std::tanh(raw_kick * 1.6f) * 1.3f;
                        } else if (drum_idx == 1 || drum_idx == 3) { // Snare & Smash Clap
                            static thread_local uint32_t rand_seed = 12345;
                            rand_seed = rand_seed * 196314165 + 907633385;
                            float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                            float tone = std::sin(KURO_TWO_PI * 180.0f * p_mul * t) * std::exp(-35.0f * t);
                            s = (tone * 0.35f + noise * 0.65f) * std::exp(-12.0f * t) * 1.0f;
                        } else if (drum_idx == 2 || drum_idx == 5) { // Hihat & Closed Hats / Shakers
                            static thread_local uint32_t rand_seed = 54321;
                            rand_seed = rand_seed * 196314165 + 907633385;
                            float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                            s = noise * std::exp(-70.0f * t) * 0.8f;
                        } else if (drum_idx == 4 || drum_idx == 7) { // Open Hat
                            static thread_local uint32_t rand_seed = 65432;
                            rand_seed = rand_seed * 196314165 + 907633385;
                            float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                            s = noise * std::exp(-8.0f * t) * 0.7f;
                        } else if (drum_idx == 6) { // Clap / Percussion
                            static thread_local uint32_t rand_seed = 98765;
                            rand_seed = rand_seed * 196314165 + 907633385;
                            float noise = ((float)rand_seed / 4294967296.0f) - 0.5f;
                            float env = 0.0f;
                            if (t < 0.01f) env = 1.0f;
                            else if (t < 0.02f) env = 0.8f;
                            else if (t < 0.03f) env = 0.6f;
                            else env = std::exp(-15.0f * (t - 0.03f));
                            s = noise * env * 0.7f;
                        } else if (drum_idx >= 0) { // Other drum fallback
                            float pitch_env = std::exp(-30.0f * t);
                            float tom_freq = (80.0f + 70.0f * pitch_env) * p_mul;
                            s = std::sin(KURO_TWO_PI * tom_freq * t) * std::exp(-6.0f * t) * 1.1f;
                        } else {
                            DrumSample* ds = nullptr;
                            int base_pitch = 60;
                            bool loop = false;

                            MidiInstrument target_inst = (ch_idx_mt >= 0 && ch_idx_mt < MAX_TRACKS && flex_active[ch_idx_mt]) 
                                                         ? flex_channel_instrument[ch_idx_mt] 
                                                         : current_instrument;

                            if (target_inst == MidiInstrument::ACOUSTIC_PIANO || target_inst == MidiInstrument::HONKY_TONK) {
                                ds = &piano_sample; base_pitch = 60; loop = false;
                            } else if (target_inst == MidiInstrument::ELECTRIC_BASS || target_inst == MidiInstrument::SUB_BASS) {
                                ds = &bass_sample; base_pitch = 36; loop = false;
                            } else if (target_inst == MidiInstrument::PAD_SYNTH || target_inst == MidiInstrument::SWEEP_PAD) {
                                ds = &pad_sample; base_pitch = 48; loop = true;
                            } else if (target_inst == MidiInstrument::STRING_ENSEMBLE || target_inst == MidiInstrument::VIOLIN) {
                                ds = &strings_sample; base_pitch = 48; loop = true;
                            } else if (target_inst == MidiInstrument::NYLON_GUITAR || target_inst == MidiInstrument::STEEL_GUITAR) {
                                ds = &guitar_sample; base_pitch = 57; loop = false;
                            } else if (target_inst == MidiInstrument::CELLO) {
                                ds = &cello_sample; base_pitch = 60; loop = true;
                            } else if (target_inst == MidiInstrument::HARPSICHORD || target_inst == MidiInstrument::CLAVINET) {
                                ds = &harpsichord_sample; base_pitch = 60; loop = false;
                            } else if (target_inst == MidiInstrument::MARIMBA || target_inst == MidiInstrument::CELESTA || target_inst == MidiInstrument::CHOIR) {
                                ds = &choir_sample; base_pitch = 57; loop = true;
                            }

                            if (ds && ds->loaded && !ds->sample_data.empty()) {
                                float ratio = std::pow(2.0f, (voice.pitch - base_pitch) / 12.0f);
                                uint64_t frame_idx = (uint64_t)(voice.current_time * ds->sample_rate * ratio);
                                if (loop) {
                                    if (ds->total_frames > 0) {
                                        frame_idx = frame_idx % ds->total_frames;
                                        size_t sample_idx = frame_idx * ds->channels;
                                        if (sample_idx < ds->sample_data.size()) {
                                            s = ds->sample_data[sample_idx];
                                        }
                                    }
                                } else {
                                    if (frame_idx < ds->total_frames) {
                                        size_t sample_idx = frame_idx * ds->channels;
                                        if (sample_idx < ds->sample_data.size()) {
                                            s = ds->sample_data[sample_idx];
                                        }
                                    }
                                }
                                float env = 1.0f;
                                if (voice.current_time > voice.duration) {
                                    env = std::max(0.0f, 1.0f - (voice.current_time - voice.duration) * 4.0f);
                                }
                                s *= env;
                            } else {
                                MidiInstrument inst_to_play = flex_active[ch_idx_mt] ? flex_channel_instrument[ch_idx_mt] : current_instrument;
                                s = synthesize(inst_to_play, freq, voice.current_time, voice.phase, voice.duration);
                            }
                        }
                        s_l = s;
                        s_r = s;
                    }

                    // Aplica Envelope ADSR dos Knobs do FLEX para todos os canais flex-active
                    if (flex_active[ch_idx_mt]) {
                        float adsr = 1.0f;
                        float t_env = voice.current_time;
                        float attack = std::max(0.001f, fs.env_vol_a);
                        float decay = std::max(0.001f, fs.env_vol_d);
                        float sustain = std::clamp(fs.env_vol_s, 0.0f, 1.0f);
                        float release = std::max(0.001f, fs.env_vol_r);
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
                        s_l *= adsr;
                        s_r *= adsr;
                        
                        // Aplica Filtro Cutoff Ressonante (Macro + Knob + Master Filter + Filter Env)
                        float flt_env_decay = std::exp(-5.0f * voice.current_time);
                        float cutoff_target = fs.filter_cutoff * 0.45f + fs.macro_filter * 0.40f + fs.master_filter_cutoff * 0.15f + flt_env_decay * fs.filter_env_amt * 0.4f;
                        cutoff_target = std::max(0.005f, std::min(0.99f, cutoff_target));
                        
                        // Filtro IIR com ressonância dinâmica
                        voice.filter_state = voice.filter_state + cutoff_target * (s_l - voice.filter_state);
                        s_l = voice.filter_state * (1.0f + fs.filter_res * 0.9f);
                        s_r = s_l;
                    }

                    // ── DE-CLICKING / MICRO-FADE DE TRANSIÇÃO (Anti-Pop) ──
                    // 2.0ms na subida e 2.5ms no corte/término da voz
                    constexpr float declick_att_sec = 0.0020f;
                    if (voice.current_time < declick_att_sec) {
                        float declick = voice.current_time / declick_att_sec;
                        s_l *= declick;
                        s_r *= declick;
                    }
                    float rem_voice_t = max_voice_dur - voice.current_time;
                    constexpr float declick_rel_sec = 0.0025f;
                    if (rem_voice_t < declick_rel_sec && rem_voice_t >= 0.0f) {
                        float declick = std::max(0.0f, rem_voice_t / declick_rel_sec);
                        s_l *= declick;
                        s_r *= declick;
                    }

                    int trk = (voice.track_idx >= 0 && voice.track_idx < MAX_TRACKS) ? voice.track_idx : 0;

                    float voice_gain = ::is_playing ? ::track_linear_volumes[trk] : 1.0f;
                    float channel_gain = 1.0f;
                    float pan_l = 1.0f;
                    float pan_r = 1.0f;
                    
                    if (drum_idx >= 0 && drum_idx < 8) {
                        channel_gain = ::dummy_vol[drum_idx];
                        float p = ::dummy_pan[drum_idx];
                        pan_l = std::min(1.0f, 1.0f - p);
                        pan_r = std::min(1.0f, 1.0f + p);
                    }

                    float base_l = s_l * voice.velocity * 0.3f * voice_gain * channel_gain;
                    float base_r = s_r * voice.velocity * 0.3f * voice_gain * channel_gain;
                    
                    if (trk >= 0 && trk < MAX_TRACKS) {
                        track_outs_l[trk][i] += base_l * pan_l;
                        track_outs_r[trk][i] += base_r * pan_r;
                        float peak_val = std::max(std::abs(base_l), std::abs(base_r));
                        if (peak_val > block_peaks[trk]) {
                            block_peaks[trk] = peak_val;
                        }
                    }

                    voice.phase += phase_inc;
                    if (voice.phase >= 1.0f) voice.phase -= 1.0f;
                    voice.current_time += dt;
                }
            }

            // Limpa vozes inativas uma única vez por bloco de áudio (NÃO por amostra)
            active_voices.erase(std::remove_if(active_voices.begin(), active_voices.end(), 
                [](const Voice& v) { return !v.active; }), active_voices.end());

            for (int t = 0; t < MAX_TRACKS; t++) {
                ::track_vu_levels[t] = ::track_vu_levels[t] * 0.8f + block_peaks[t] * 0.2f;
            }
        }
    };
}

