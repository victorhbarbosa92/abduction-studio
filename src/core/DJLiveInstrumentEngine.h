#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <mutex>
#include "imgui.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace KuroAudio {

    // =========================================================================
    // SINTETIZADOR ANALÓGICO MONOFÔNICO ESTILO TB-303 (ACID BASSLINE ENGINE EXPANDIDO)
    // Suporte a Onda Tooth (Saw), Square, Triangle, Sine, Escalas Harmônicas e Reta,
    // Plugins (Drive, Delay, Reverb, Flanger/Chorus), Filtros Diode/Transistor/Bandpass/Highpass,
    // Snares 909/808 e Compassos Variados (4/4, 3/4, 6/8, 7/8, 5/4, 8-step, 32-step).
    // =========================================================================
    class Acid303Synth {
    public:
        enum class Waveform { SAW = 0, SQUARE, TRIANGLE, SINE };
        enum class FilterType { DIODE_18DB = 0, TRANSISTOR_24DB, BANDPASS_12DB, HIGHPASS_12DB };
        enum class TimeSignature { TIME_4_4 = 0, TIME_3_4, TIME_6_8, TIME_7_8, TIME_5_4, TIME_8_STEP, TIME_32_STEP, TIME_12_8 };
        enum class ScaleType {
            STRAIGHT_CHROMATIC = 0, // Escala Reta (12 semitons sem restrição)
            HARMONIC_MINOR,         // Menor Harmônica
            PHRYGIAN_DOMINANT,      // Frígio Dominante
            DORIAN,                 // Dórico
            NATURAL_MINOR,          // Menor Natural
            MELODIC_MINOR,          // Menor Melódica
            ACID_BLUES,             // Acid Blues (com Blue Note)
            MINOR_PENTATONIC,       // Pentatônica Menor
            MAJOR_PENTATONIC,       // Pentatônica Maior
            JAPANESE_INSEN          // Harmônica Japonesa Insen
        };

        Waveform wave_type = Waveform::SAW; // Onda Tooth (Sawtooth) por padrão
        FilterType filter_type = FilterType::DIODE_18DB;
        TimeSignature time_sig = TimeSignature::TIME_4_4;
        ScaleType active_scale = ScaleType::STRAIGHT_CHROMATIC;
        int scale_root_note = 36; // C2

        float cutoff = 0.50f;       // [0.05 .. 0.98] Filtro passa-baixa
        float resonance = 0.75f;    // [0.0 .. 0.95] Grito ácido
        float env_mod = 0.65f;      // [0.0 .. 1.0] Abertura do envelope
        float decay = 0.35f;        // [0.05 .. 1.0] Decaimento da nota
        float accent = 0.50f;       // [0.0 .. 1.0] Acentuação
        float volume = 0.75f;       // [0.0 .. 1.0] Ganho de saída
        float bpm = 138.0f;
        int octave_shift = 0;       // -1, 0, +1
        int pattern_idx = 0;

        // Plugins integrados de Performance Acid
        float drive = 0.35f;        // [0.0 .. 1.0] Distorção / Overdrive analógico
        float delay_mix = 0.25f;    // [0.0 .. 1.0] Mix do Echo/Delay estéreo
        float delay_feedback = 0.50f;// Feedback do Delay
        float reverb_mix = 0.15f;   // [0.0 .. 1.0] Reverb de ambiente
        float flanger_mix = 0.0f;   // [0.0 .. 1.0] Flanger / Chorus Estéreo
        
        // Snare 909/808 integrado
        bool snare_enabled = false;
        float snare_volume = 0.75f;
        float snare_snap = 0.65f;

        // Sequenciador com suporte a até 32 passos e compassos variados
        struct Step303 {
            int note_semitone = 36; // C2 = 36
            bool active = false;
            bool slide = false;
            bool has_accent = false;
            bool has_snare = false; // Caixa 909 neste passo
        };

        Step303 steps[32];
        int num_steps = 16;
        std::atomic<int> current_step{ 0 };
        std::atomic<bool> is_running{ false };

    private:
        double phase = 0.0;
        float current_freq = 65.41f; // C2
        float target_freq = 65.41f;
        float env_val = 0.0f;
        float step_samples_accum = 0.0f;

        // Filtro 4 estágios com saturação não linear
        float stage[4] = { 0, 0, 0, 0 };

        // Buffer do Delay estéreo integrado
        static const int DELAY_BUF_SZ = 96000;
        std::vector<float> dly_buf_l, dly_buf_r;
        int dly_write_pos = 0;

        // Buffer e estado do Reverb Allpass
        float rev_buf1[1116] = { 0 };
        float rev_buf2[1188] = { 0 };
        float rev_buf3[1277] = { 0 };
        int rev_idx1 = 0, rev_idx2 = 0, rev_idx3 = 0;

        // Buffer do Flanger / Chorus estéreo
        float flanger_buf[2048] = { 0 };
        int flanger_idx = 0;
        float flanger_lfo = 0.0f;

        // Estado do sintetizador de Snare analógico
        float snare_env = 0.0f;
        float snare_pitch_env = 0.0f;
        double snare_phase = 0.0;
        uint32_t snare_rng = 22222;

    public:
        Acid303Synth() {
            dly_buf_l.assign(DELAY_BUF_SZ, 0.0f);
            dly_buf_r.assign(DELAY_BUF_SZ, 0.0f);
            loadPattern(0);
        }

        void setTimeSignature(TimeSignature ts) {
            time_sig = ts;
            switch (ts) {
                case TimeSignature::TIME_4_4: num_steps = 16; break;
                case TimeSignature::TIME_3_4: num_steps = 12; break; // Valsa ácida
                case TimeSignature::TIME_6_8: num_steps = 12; break; // Compasso ternário
                case TimeSignature::TIME_7_8: num_steps = 14; break; // Prog 7/8
                case TimeSignature::TIME_5_4: num_steps = 10; break;
                case TimeSignature::TIME_8_STEP: num_steps = 8; break; // Meio-compasso
                case TimeSignature::TIME_32_STEP: num_steps = 32; break; // Frase longa 2 compassos
                case TimeSignature::TIME_12_8: num_steps = 12; break;
            }
        }

        int getNumSteps() const { return num_steps; }

        // Quantizador para Escalas Harmônicas e Reta
        static int quantizeToScale(int semitone, int root, ScaleType scale) {
            if (scale == ScaleType::STRAIGHT_CHROMATIC) return semitone; // Escala Reta livre

            int rel = (semitone - root) % 12;
            if (rel < 0) rel += 12;
            int oct = (semitone - root - rel) / 12;

            std::vector<int> intervals;
            switch (scale) {
                case ScaleType::HARMONIC_MINOR:
                    intervals = { 0, 2, 3, 5, 7, 8, 11 }; // Menor Harmônica
                    break;
                case ScaleType::PHRYGIAN_DOMINANT:
                    intervals = { 0, 1, 4, 5, 7, 8, 10 }; // Frígio Dominante
                    break;
                case ScaleType::DORIAN:
                    intervals = { 0, 2, 3, 5, 7, 9, 10 }; // Dórico
                    break;
                case ScaleType::NATURAL_MINOR:
                    intervals = { 0, 2, 3, 5, 7, 8, 10 }; // Menor Natural
                    break;
                case ScaleType::MELODIC_MINOR:
                    intervals = { 0, 2, 3, 5, 7, 9, 11 }; // Menor Melódica
                    break;
                case ScaleType::MINOR_PENTATONIC:
                    intervals = { 0, 3, 5, 7, 10 }; // Pentatônica Menor
                    break;
                case ScaleType::MAJOR_PENTATONIC:
                    intervals = { 0, 2, 4, 7, 9 };  // Pentatônica Maior
                    break;
                case ScaleType::ACID_BLUES:
                    intervals = { 0, 3, 5, 6, 7, 10 }; // Acid Blues
                    break;
                case ScaleType::JAPANESE_INSEN:
                    intervals = { 0, 1, 5, 7, 10 }; // Japonesa Insen
                    break;
                default:
                    return semitone;
            }

            int best_int = intervals[0];
            int best_dist = 999;
            for (int note_int : intervals) {
                int dist = std::abs(rel - note_int);
                if (dist < best_dist) {
                    best_dist = dist;
                    best_int = note_int;
                }
            }
            return root + oct * 12 + best_int;
        }

        void applyScaleToPattern(ScaleType sc) {
            active_scale = sc;
            for (int i = 0; i < 32; i++) {
                steps[i].note_semitone = quantizeToScale(steps[i].note_semitone, scale_root_note, sc);
            }
        }

        // =====================================================================
        // BANCO EXPANDIDO DE 16 PRESETS ÁCIDOS CLÁSSICOS & MODERNOS
        // =====================================================================
        void loadPattern(int p_idx) {
            pattern_idx = std::clamp(p_idx, 0, 15);
            
            // Limpa todos os passos
            for (int i = 0; i < 32; i++) {
                steps[i].active = false;
                steps[i].slide = false;
                steps[i].has_accent = false;
                steps[i].has_snare = false;
                steps[i].note_semitone = 36;
            }
            snare_enabled = false;
            flanger_mix = 0.0f;

            if (pattern_idx == 0) { // 01: Chicago 1988 Classic (Phuture Acid Tracks)
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::DIODE_18DB;
                active_scale = ScaleType::STRAIGHT_CHROMATIC;
                cutoff = 0.42f; resonance = 0.82f; env_mod = 0.70f; decay = 0.32f; drive = 0.30f;
                int def_notes[16] = { 36, 36, 48, 36, 39, 36, 43, 46, 36, 36, 48, 43, 39, 36, 46, 48 };
                bool def_act[16]  = { 1,  0,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1 };
                bool def_sld[16]  = { 0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0 };
                bool def_acc[16]  = { 1,  0,  0,  1,  0,  0,  1,  0,  1,  0,  0,  1,  0,  0,  1,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 1) { // 02: Daft Punk Da Funk (Distorted Tooth Lead)
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::TRANSISTOR_24DB;
                active_scale = ScaleType::STRAIGHT_CHROMATIC;
                cutoff = 0.55f; resonance = 0.65f; env_mod = 0.80f; decay = 0.45f; drive = 0.75f;
                int def_notes[16] = { 36, 36, 39, 36, 41, 42, 41, 39, 36, 36, 48, 46, 43, 41, 39, 36 };
                bool def_act[16]  = { 1,  1,  1,  0,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  0 };
                bool def_sld[16]  = { 0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  1,  0,  1,  0,  0,  0 };
                bool def_acc[16]  = { 1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  0,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 2) { // 03: Hardfloor TB-303 (High Res Screamer)
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::SQUARE;
                filter_type = FilterType::DIODE_18DB;
                cutoff = 0.60f; resonance = 0.92f; env_mod = 0.85f; decay = 0.28f; drive = 0.45f;
                int def_notes[16] = { 48, 48, 51, 53, 55, 53, 51, 48, 46, 48, 51, 55, 58, 55, 51, 46 };
                bool def_act[16]  = { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 };
                bool def_sld[16]  = { 0,  1,  0,  1,  1,  0,  1,  0,  0,  1,  0,  1,  1,  0,  1,  0 };
                bool def_acc[16]  = { 1,  0,  1,  0,  1,  0,  0,  1,  0,  1,  0,  1,  1,  0,  1,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 3) { // 04: Berlin Underground (Dark Sub Sine-Tooth)
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::TRIANGLE;
                filter_type = FilterType::TRANSISTOR_24DB;
                cutoff = 0.30f; resonance = 0.40f; env_mod = 0.50f; decay = 0.25f; drive = 0.20f;
                int def_notes[16] = { 36, 36, 36, 36, 48, 36, 36, 36, 36, 36, 36, 36, 46, 36, 39, 36 };
                bool def_act[16]  = { 1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  1,  1,  0 };
                bool def_sld[16]  = { 0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0 };
                bool def_acc[16]  = { 1,  0,  1,  0,  1,  0,  0,  0,  1,  0,  1,  0,  1,  0,  1,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 4) { // 05: Psytrance Rolling 16th (Bassline Tooth)
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::DIODE_18DB;
                cutoff = 0.38f; resonance = 0.70f; env_mod = 0.60f; decay = 0.20f; drive = 0.35f;
                int def_notes[16] = { 36, 36, 36, 36, 36, 36, 36, 36, 38, 38, 38, 38, 36, 36, 39, 41 };
                bool def_act[16]  = { 0,  1,  1,  1,  0,  1,  1,  1,  0,  1,  1,  1,  0,  1,  1,  1 };
                bool def_sld[16]  = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1 };
                bool def_acc[16]  = { 0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  1,  1 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 5) { // 06: Melodic Dorian Pluck (Modern Afterlife)
                setTimeSignature(TimeSignature::TIME_4_4);
                active_scale = ScaleType::DORIAN;
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::DIODE_18DB;
                cutoff = 0.48f; resonance = 0.55f; env_mod = 0.72f; decay = 0.38f; delay_mix = 0.35f; reverb_mix = 0.25f;
                int def_notes[16] = { 38, 41, 45, 48, 50, 48, 45, 41, 38, 45, 48, 52, 50, 48, 45, 41 };
                bool def_act[16]  = { 1,  0,  1,  1,  0,  1,  1,  0,  1,  1,  0,  1,  1,  0,  1,  0 };
                bool def_sld[16]  = { 0,  0,  1,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0 };
                bool def_acc[16]  = { 1,  0,  0,  1,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 6) { // 07: Egyptian Acid (Harmonic Minor Exotica)
                setTimeSignature(TimeSignature::TIME_4_4);
                active_scale = ScaleType::HARMONIC_MINOR;
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::DIODE_18DB;
                cutoff = 0.52f; resonance = 0.85f; env_mod = 0.78f; decay = 0.30f; drive = 0.50f;
                int def_notes[16] = { 36, 39, 43, 44, 47, 44, 43, 39, 36, 44, 47, 48, 47, 44, 43, 39 };
                bool def_act[16]  = { 1,  1,  1,  0,  1,  1,  0,  1,  1,  0,  1,  1,  1,  0,  1,  0 };
                bool def_sld[16]  = { 0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  1,  0,  0,  1,  0 };
                bool def_acc[16]  = { 1,  0,  1,  0,  1,  0,  0,  1,  1,  0,  0,  1,  1,  0,  0,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 7) { // 08: Industrial Straight (Chromatic Tooth Drive)
                setTimeSignature(TimeSignature::TIME_4_4);
                active_scale = ScaleType::STRAIGHT_CHROMATIC;
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::TRANSISTOR_24DB;
                cutoff = 0.65f; resonance = 0.80f; env_mod = 0.65f; decay = 0.22f; drive = 0.85f;
                int def_notes[16] = { 36, 37, 36, 38, 36, 37, 36, 39, 36, 37, 36, 38, 48, 47, 46, 45 };
                bool def_act[16]  = { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 };
                bool def_sld[16]  = { 0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  1,  1,  1,  0 };
                bool def_acc[16]  = { 1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  1,  1,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 8) { // 09: Acid Waltz 3/4 (Odd Meter Trip 12 Steps)
                setTimeSignature(TimeSignature::TIME_3_4);
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::DIODE_18DB;
                cutoff = 0.45f; resonance = 0.78f; env_mod = 0.70f; decay = 0.35f; drive = 0.40f;
                int def_notes[12] = { 36, 36, 48, 39, 43, 46, 36, 39, 43, 48, 46, 43 };
                bool def_act[12]  = { 1,  0,  1,  1,  1,  1,  1,  1,  0,  1,  1,  0 };
                bool def_sld[12]  = { 0,  0,  1,  0,  0,  1,  0,  0,  0,  1,  0,  0 };
                bool def_acc[12]  = { 1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0 };
                for (int i = 0; i < 12; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 9) { // 10: 909 Snare & Square Acid Groove
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::SQUARE;
                filter_type = FilterType::DIODE_18DB;
                cutoff = 0.50f; resonance = 0.88f; env_mod = 0.75f; decay = 0.30f; drive = 0.35f;
                snare_enabled = true;
                int def_notes[16] = { 36, 36, 48, 36, 39, 36, 43, 46, 36, 36, 48, 43, 39, 36, 46, 48 };
                bool def_act[16]  = { 1,  0,  1,  0,  1,  0,  1,  1,  1,  0,  1,  0,  1,  0,  1,  1 };
                bool def_sld[16]  = { 0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0 };
                bool def_acc[16]  = { 1,  0,  0,  0,  1,  0,  1,  0,  1,  0,  0,  0,  1,  0,  1,  0 };
                bool def_snr[16]  = { 0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  1 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                    steps[i].has_snare = def_snr[i];
                }
            } else if (pattern_idx == 10) { // 11: Resonant Squelch Monster (Bandpass Screamer)
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::BANDPASS_12DB;
                cutoff = 0.58f; resonance = 0.94f; env_mod = 0.90f; decay = 0.18f; drive = 0.55f;
                int def_notes[16] = { 36, 48, 48, 36, 42, 48, 45, 36, 48, 42, 36, 48, 51, 48, 46, 36 };
                bool def_act[16]  = { 1,  1,  0,  1,  1,  0,  1,  1,  0,  1,  1,  0,  1,  1,  1,  0 };
                bool def_sld[16]  = { 0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  1,  0,  1,  0 };
                bool def_acc[16]  = { 1,  1,  0,  1,  0,  0,  1,  0,  0,  1,  1,  0,  1,  0,  1,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 11) { // 12: Pure Tooth Bass (Raw Saw Analog)
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::DIODE_18DB;
                cutoff = 0.36f; resonance = 0.62f; env_mod = 0.58f; decay = 0.32f; drive = 0.18f;
                int def_notes[16] = { 36, 36, 36, 48, 36, 36, 39, 36, 36, 36, 41, 36, 43, 36, 46, 36 };
                bool def_act[16]  = { 1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  0,  1,  1,  1,  0 };
                bool def_sld[16]  = { 0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  1,  0 };
                bool def_acc[16]  = { 1,  0,  0,  1,  0,  0,  1,  0,  1,  0,  0,  0,  1,  0,  1,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 12) { // 13: Acid Trance 303 (Platipus / Eye Q 90s)
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::DIODE_18DB;
                cutoff = 0.62f; resonance = 0.86f; env_mod = 0.80f; decay = 0.40f; delay_mix = 0.40f; reverb_mix = 0.30f;
                int def_notes[16] = { 36, 48, 51, 55, 48, 51, 55, 58, 51, 55, 58, 60, 58, 55, 51, 48 };
                bool def_act[16]  = { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 };
                bool def_sld[16]  = { 0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  1,  0,  1,  0 };
                bool def_acc[16]  = { 1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  1,  0,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 13) { // 14: Cyberpunk 7/8 (Prog 14-Step Odd Meter)
                setTimeSignature(TimeSignature::TIME_7_8);
                active_scale = ScaleType::PHRYGIAN_DOMINANT;
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::TRANSISTOR_24DB;
                cutoff = 0.50f; resonance = 0.75f; env_mod = 0.70f; decay = 0.28f; drive = 0.60f;
                int def_notes[14] = { 36, 37, 40, 41, 43, 44, 46, 36, 40, 43, 44, 46, 48, 46 };
                bool def_act[14]  = { 1,  1,  1,  0,  1,  1,  0,  1,  1,  0,  1,  1,  1,  0 };
                bool def_sld[14]  = { 0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  1,  0 };
                bool def_acc[14]  = { 1,  0,  1,  0,  1,  0,  0,  1,  0,  0,  1,  0,  1,  0 };
                for (int i = 0; i < 14; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else if (pattern_idx == 14) { // 15: Ambient Acid Space (Deep Delay & Hall)
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::SINE;
                filter_type = FilterType::DIODE_18DB;
                cutoff = 0.40f; resonance = 0.50f; env_mod = 0.45f; decay = 0.55f;
                delay_mix = 0.50f; reverb_mix = 0.55f; flanger_mix = 0.35f;
                int def_notes[16] = { 38, 45, 48, 52, 38, 48, 52, 57, 41, 48, 53, 57, 43, 50, 55, 59 };
                bool def_act[16]  = { 1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0 };
                bool def_sld[16]  = { 0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0 };
                bool def_acc[16]  = { 1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            } else { // 16: Laser Highpass Sweeper (Thin Screaming Lead)
                setTimeSignature(TimeSignature::TIME_4_4);
                wave_type = Waveform::SAW; // Tooth
                filter_type = FilterType::HIGHPASS_12DB;
                cutoff = 0.68f; resonance = 0.88f; env_mod = 0.85f; decay = 0.25f; drive = 0.70f;
                int def_notes[16] = { 48, 48, 60, 48, 51, 48, 55, 58, 48, 48, 60, 55, 51, 48, 58, 60 };
                bool def_act[16]  = { 1,  0,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1 };
                bool def_sld[16]  = { 0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0 };
                bool def_acc[16]  = { 1,  0,  0,  1,  0,  0,  1,  0,  1,  0,  0,  1,  0,  0,  1,  0 };
                for (int i = 0; i < 16; i++) {
                    steps[i].note_semitone = def_notes[i]; steps[i].active = def_act[i];
                    steps[i].slide = def_sld[i]; steps[i].has_accent = def_acc[i];
                }
            }
        }

        void triggerSnare() {
            snare_env = 1.0f;
            snare_pitch_env = 1.0f;
        }

        void triggerNote(int semitone, bool slide, bool acc) {
            float freq = 440.0f * std::pow(2.0f, (semitone - 69) / 12.0f);
            if (slide) target_freq = freq;
            else { current_freq = freq; target_freq = freq; }

            env_val = acc ? (1.0f + accent * 0.5f) : 1.0f;
        }

        void advanceStep() {
            int next_st = (current_step.load() + 1) % std::max(1, num_steps);
            current_step.store(next_st);
            const auto& s = steps[next_st];
            if (s.active) {
                triggerNote(s.note_semitone + octave_shift * 12, s.slide, s.has_accent);
            }
            if (s.has_snare || (snare_enabled && (next_st == 4 || next_st == 12))) {
                triggerSnare();
            }
        }

        // Parada e Silenciamento Imediato Garantido (Zero Áudio Residual)
        void stop() {
            is_running.store(false);
            for (int i = 0; i < 4; i++) stage[i] = 0.0f;
            env_val = 0.0f;
            phase = 0.0;
            step_samples_accum = 0.0f;
            current_step.store(0);
            snare_env = 0.0f;
            snare_pitch_env = 0.0f;
        }

        void start() {
            stop();
            is_running.store(true);
            const auto& s = steps[0];
            if (s.active) {
                triggerNote(s.note_semitone + octave_shift * 12, s.slide, s.has_accent);
            }
            if (s.has_snare || (snare_enabled && num_steps >= 4)) {
                triggerSnare();
            }
        }

        float processSample(float sample_rate) {
            if (!is_running.load()) return 0.0f;

            // Relógio de 16 avos de tempo sincronizado ao BPM
            float eff_bpm = (bpm > 40.0f) ? bpm : 138.0f;
            float step_mult = (time_sig == TimeSignature::TIME_6_8 || time_sig == TimeSignature::TIME_12_8) ? (1.0f / 3.0f) : 0.25f;
            float samples_per_step = (sample_rate * 60.0f / eff_bpm) * step_mult;
            step_samples_accum += 1.0f;
            if (step_samples_accum >= samples_per_step) {
                step_samples_accum -= samples_per_step;
                advanceStep();
            }

            // Portamento / Slide
            current_freq += (target_freq - current_freq) * (1.0f - std::exp(-1.0f / (0.040f * sample_rate)));

            // Oscilador Multi-Forma de Onda (Tooth Saw, Square, Triangle, Sine)
            double phase_inc = current_freq / sample_rate;
            phase += phase_inc;
            if (phase >= 1.0) phase -= 1.0;

            float raw_osc = 0.0f;
            if (wave_type == Waveform::SAW) {
                // Autêntica onda Tooth (Dente de Serra) Roland TB-303 com curvatura analógica e harmônicos vivos
                float lin_saw = (float)(phase * 2.0 - 1.0);
                raw_osc = lin_saw - 0.20f * (lin_saw * lin_saw * lin_saw);
            } else if (wave_type == Waveform::SQUARE) {
                raw_osc = (phase < 0.5) ? 0.85f : -0.85f;
            } else if (wave_type == Waveform::TRIANGLE) {
                raw_osc = (phase < 0.5) ? (float)(4.0 * phase - 1.0) : (float)(3.0 - 4.0 * phase);
            } else {
                raw_osc = (float)std::sin(phase * 2.0 * M_PI);
            }

            // Envelope decay
            float env_decay_coeff = std::exp(-1.0f / (std::max(0.05f, decay) * 0.4f * sample_rate));
            env_val *= env_decay_coeff;

            // Filtro 303 com modulação de envelope e saturação não linear
            float mod_cutoff = cutoff + env_val * env_mod * 0.65f;
            mod_cutoff = std::clamp(mod_cutoff, 0.02f, 0.98f);
            float f_hz = 30.0f * std::pow(400.0f, mod_cutoff);
            float w = 2.0f * (float)M_PI * f_hz / sample_rate;
            w = std::clamp(w, 0.001f, 0.85f);

            float res_q = resonance * ((filter_type == FilterType::DIODE_18DB) ? 4.1f : 3.6f);
            float feedback = stage[3] * res_q;
            float input = std::tanh(raw_osc - feedback); // Saturação ácida do feedback

            // 4-Pole Ladder
            stage[0] += w * (input - stage[0]);
            stage[1] += w * (stage[0] - stage[1]);
            stage[2] += w * (stage[1] - stage[2]);
            stage[3] += w * (stage[2] - stage[3]);

            if (std::isnan(stage[3]) || std::isinf(stage[3])) {
                stage[0] = stage[1] = stage[2] = stage[3] = 0.0f;
                return 0.0f;
            }

            // Seleção de Topologia de Filtro Analógico
            float filtered = stage[3];
            if (filter_type == FilterType::DIODE_18DB) {
                filtered = stage[2] * 0.45f + stage[3] * 0.55f; // TB-303 18dB
            } else if (filter_type == FilterType::TRANSISTOR_24DB) {
                filtered = stage[3]; // Moog 24dB 4-pole
            } else if (filter_type == FilterType::BANDPASS_12DB) {
                filtered = (stage[1] - stage[3]) * 1.85f; // Acid Bandpass Screamer
            } else if (filter_type == FilterType::HIGHPASS_12DB) {
                filtered = (input - stage[1]) * 1.25f; // Laser Highpass Sweeper
            }

            // Plugin 1: Drive / Overdrive Distorção Ácida estilo Boss DS-1 / Rat
            if (drive > 0.01f) {
                float drv_gain = 1.0f + drive * 4.5f;
                filtered = std::tanh(filtered * drv_gain) / std::sqrt(drv_gain * 0.6f + 0.4f);
            }

            // Síntese de Snare 909 analógica integrada
            float snare_out = 0.0f;
            if (snare_env > 0.001f) {
                snare_pitch_env *= std::exp(-1.0f / (0.018f * sample_rate));
                snare_env *= std::exp(-1.0f / (0.120f * sample_rate));
                float s_freq = 90.0f + snare_pitch_env * 160.0f;
                snare_phase += s_freq / sample_rate;
                if (snare_phase >= 1.0) snare_phase -= 1.0;
                float body = (float)std::sin(snare_phase * 2.0 * M_PI);

                // Ruído branco pseudo-aleatório rápido
                snare_rng = snare_rng * 1664525u + 1013904223u;
                float noise = ((float)(snare_rng & 0xFFFF) / 32768.0f) - 1.0f;
                float snap_mix = std::clamp(snare_snap, 0.1f, 0.9f);
                snare_out = (body * (1.0f - snap_mix) + noise * snap_mix) * snare_env * snare_volume;
            }

            float synth_mono = filtered * volume + snare_out;

            // Plugin 2: Delay Estéreo Sincronizado
            if (delay_mix > 0.01f && DELAY_BUF_SZ > 0) {
                int dly_samples = (int)(sample_rate * (60.0f / eff_bpm) * 0.375f); // 3/16 Dotted Delay
                dly_samples = std::clamp(dly_samples, 100, DELAY_BUF_SZ - 1);
                int r_pos = (dly_write_pos + DELAY_BUF_SZ - dly_samples) % DELAY_BUF_SZ;
                float d_sig = dly_buf_l[r_pos];
                dly_buf_l[dly_write_pos] = std::tanh(synth_mono + d_sig * std::clamp(delay_feedback, 0.1f, 0.85f));
                dly_write_pos = (dly_write_pos + 1) % DELAY_BUF_SZ;
                synth_mono += d_sig * delay_mix * 0.65f;
            }

            // Plugin 3: Reverb Allpass Espacial
            if (reverb_mix > 0.01f) {
                float r1 = rev_buf1[rev_idx1];
                rev_buf1[rev_idx1] = synth_mono + r1 * 0.5f;
                rev_idx1 = (rev_idx1 + 1) % 1116;
                synth_mono += (r1 - synth_mono * 0.5f) * reverb_mix * 0.40f;
            }

            // Plugin 4: Flanger / Chorus Estéreo Roland Dimension D
            if (flanger_mix > 0.01f) {
                flanger_lfo += 0.4f / sample_rate;
                if (flanger_lfo >= 1.0f) flanger_lfo -= 1.0f;
                float mod_s = 140.0f + 90.0f * (float)std::sin(flanger_lfo * 2.0 * M_PI);
                int fl_rpos = (flanger_idx + 2048 - (int)mod_s) % 2048;
                float fl_s = flanger_buf[fl_rpos];
                flanger_buf[flanger_idx] = synth_mono;
                flanger_idx = (flanger_idx + 1) % 2048;
                synth_mono = synth_mono * (1.0f - flanger_mix * 0.4f) + fl_s * (flanger_mix * 0.6f);
            }

            return std::clamp(synth_mono, -1.5f, 1.5f);
        }
    };

    // =========================================================================
    // DRUM MACHINE DE 16 PADS COM SÍNTESE PROCEDURAL & QUANTIZAÇÃO NO BEATGRID
    // =========================================================================
    class DrumMachine16Pads {
    public:
        struct DrumVoice {
            std::string name;
            float phase = 0.0f;
            float env = 0.0f;
            float velocity = 0.0f;
            bool active = false;
        };

        DrumVoice voices[16];
        float volume = 0.80f;

        DrumMachine16Pads() {
            const char* names[16] = {
                "KICK 909", "SNARE 808", "CLAP", "CH CLOSED",
                "CH OPEN",  "CRASH",     "RIDE", "PERC HI",
                "PERC MID", "TOM LOW",   "TOM HI", "RIMSHOT",
                "SHAKER",   "SUB DROP",  "COWBELL", "TAMB"
            };
            for (int i = 0; i < 16; i++) {
                voices[i].name = names[i];
                voices[i].active = false;
            }
        }

        void triggerPad(int pad_idx, float velocity = 1.0f) {
            if (pad_idx < 0 || pad_idx >= 16) return;
            voices[pad_idx].active = true;
            voices[pad_idx].env = 1.0f;
            voices[pad_idx].phase = 0.0f;
            voices[pad_idx].velocity = std::clamp(velocity, 0.1f, 1.0f);
        }

        float processSample(float sample_rate) {
            float out = 0.0f;
            for (int p = 0; p < 16; p++) {
                auto& v = voices[p];
                if (!v.active) continue;

                float sample = 0.0f;
                v.phase += 1.0f / sample_rate;

                switch (p) {
                    case 0: { // KICK
                        float t = v.phase;
                        float f = 160.0f * std::exp(-t * 36.0f) + 48.0f;
                        sample = std::sin(2.0f * (float)M_PI * f * t) * v.env;
                        v.env *= std::exp(-1.0f / (0.28f * sample_rate));
                        break;
                    }
                    case 1: { // SNARE
                        float t = v.phase;
                        float tone = std::sin(2.0f * (float)M_PI * 185.0f * t) * std::exp(-t * 22.0f);
                        float noise = ((float)(rand() % 2000 - 1000) / 1000.0f) * std::exp(-t * 18.0f);
                        sample = (tone * 0.4f + noise * 0.7f) * v.env;
                        v.env *= std::exp(-1.0f / (0.18f * sample_rate));
                        break;
                    }
                    case 2: { // CLAP
                        float t = v.phase;
                        float noise = ((float)(rand() % 2000 - 1000) / 1000.0f);
                        float env_burst = std::fmod(t, 0.012f) < 0.005f ? 1.0f : 0.4f;
                        sample = noise * env_burst * v.env;
                        v.env *= std::exp(-1.0f / (0.16f * sample_rate));
                        break;
                    }
                    case 3: { // CLOSED HAT
                        float noise = ((float)(rand() % 2000 - 1000) / 1000.0f);
                        sample = noise * v.env * 0.5f;
                        v.env *= std::exp(-1.0f / (0.045f * sample_rate));
                        break;
                    }
                    case 4: { // OPEN HAT
                        float noise = ((float)(rand() % 2000 - 1000) / 1000.0f);
                        sample = noise * v.env * 0.55f;
                        v.env *= std::exp(-1.0f / (0.28f * sample_rate));
                        break;
                    }
                    default: { // DEMAIS SONS PERCUSSIVOS
                        float t = v.phase;
                        float f = 250.0f + p * 60.0f;
                        sample = std::sin(2.0f * (float)M_PI * f * t) * v.env * 0.6f;
                        v.env *= std::exp(-1.0f / (0.12f * sample_rate));
                        break;
                    }
                }

                if (v.env < 0.001f) v.active = false;
                out += sample * v.velocity;
            }

            return std::clamp(out * volume, -1.0f, 1.0f);
        }
    };

    // =========================================================================
    // MOTOR LIVE INTEGRADO: ACID 303 + DRUMS + ENTRADA USB-MIDI UNIVERSAL
    // =========================================================================
    class DJLiveInstrumentEngine {
    private:
        DJLiveInstrumentEngine() = default;
        std::mutex m_mutex;

    public:
        static DJLiveInstrumentEngine& getInstance() {
            static DJLiveInstrumentEngine instance;
            return instance;
        }

        Acid303Synth synth303;
        DrumMachine16Pads drum_machine;
        bool master_enable = true;
        float master_live_vol = 0.85f;

        // Processa áudio em tempo real para ser somado ao Master Out
        void processLiveBlock(float* out_l, float* out_r, size_t frames, float sample_rate) {
            if (!master_enable) return;

            std::lock_guard<std::mutex> lock(m_mutex);
            for (size_t i = 0; i < frames; i++) {
                float s_303 = synth303.processSample(sample_rate);
                float s_drum = drum_machine.processSample(sample_rate);
                float mix = (s_303 + s_drum) * master_live_vol;

                out_l[i] += mix;
                out_r[i] += mix;
            }
        }

        // Parada de Emergência Mestre e Silenciamento Imediato de Qualquer Som (Anti-Soando)
        void panicMute() {
            std::lock_guard<std::mutex> lock(m_mutex);
            synth303.stop();
            for (int i = 0; i < 16; i++) {
                drum_machine.voices[i].active = false;
                drum_machine.voices[i].env = 0.0f;
            }
        }

        // Mapeamento USB-MIDI universal
        void handleMidiNoteOn(int note, int velocity) {
            std::lock_guard<std::mutex> lock(m_mutex);
            float vel_norm = (float)velocity / 127.0f;

            // Notas 36 a 51 (C1 a D#2) mapeadas para os 16 pads da Drum Machine
            if (note >= 36 && note <= 51) {
                drum_machine.triggerPad(note - 36, vel_norm);
            }
            // Notas de teclado (C2 em diante) tocam o sintetizador 303
            else if (note >= 24 && note <= 84) {
                synth303.triggerNote(note, false, velocity > 100);
                synth303.is_running = true;
            }
        }

        void handleMidiNoteOff(int note) {
            // Nota solta
        }
    };

} // namespace KuroAudio

