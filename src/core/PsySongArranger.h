#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include "ClipManager.h"
#include "TimelineManager.h"
#include "../audio/SynthEngine.h"

extern KuroAudio::SynthEngine g_piano_synth;
extern std::string track_names[MAX_TRACKS];

namespace KuroArranger {

    struct PsyArrangerConfig {
        int subgenre = 1;      // 0 = Progressive Psy (138 BPM), 1 = Full-On Psy (142 BPM), 2 = Dark/Twilight Psy (148 BPM)
        int root_note = 30;    // F#1 (30), G1 (31), A1 (33), D1 (26), E1 (28)
        float bpm = 142.0f;
        
        // Seções e Durações (em compassos / bars)
        bool has_intro = true;
        int intro_bars = 16;
        
        bool has_buildup1 = true;
        int buildup1_bars = 8;
        
        bool has_drop1 = true;
        int drop1_bars = 32;
        
        bool has_break = true;
        int break_bars = 16;
        
        bool has_buildup2 = true;
        int buildup2_bars = 8;
        
        bool has_drop2 = true;
        int drop2_bars = 32;
        
        bool has_outro = true;
        int outro_bars = 16;
        
        // Automações
        bool enable_filter_sweep = true;
        bool enable_reverb_washout = true;
        bool enable_pitch_riser = true;
        
        // Amostras de alta fidelidade
        bool use_pryzma_samples = true;
    };

    class PsySongArranger {
    public:
        static void GenerateArrangement(const PsyArrangerConfig& cfg, ClipManager& cm, KuroDSP::TimelineManager& tl) {
            tl.setBPM(cfg.bpm);
            tl.setSwing(0.12f); // Balanço rítmico orgânico suave de Psytrance
            tl.is_pattern_mode = false; // Comuta automaticamente para SONG MODE para tocar o arranjo da Playlist!

            cm.pushUndo();
            cm.reset();
            tl.clearSectionMarkers();

            float beat = 60.0f / cfg.bpm;
            float step = beat * 0.25f; // Semicolcheia (16th note)
            float bar = beat * 4.0f;   // 1 compasso de 4 tempos

            // 1. Configuração dos Instrumentos dos Canais
            // Track 0: Kick
            // Track 1: Sub Bass
            // Track 2: Mid Saw Bass
            // Track 3: Snare / Clap
            // Track 4: Open Hat
            // Track 5: Closed Hat
            // Track 6: Tribal Percussion
            // Track 7: FM Squelch
            // Track 8: Acid Lead Hook
            // Track 9: Counter-Arp Pluck
            // Track 10: SuperSaw Poly Lead
            // Track 11: Dark Sub Drone
            // Track 12: Mystic Strings
            // Track 13: Steinway Piano
            // Track 14: Vocal Mantra
            // Track 15: Snare Roll & FX Risers
            // Track 16: [AUTO] Filter Cutoff
            // Track 17: [AUTO] Reverb Washout
            // Track 18: [AUTO] Pitch Riser

            track_names[0]  = "01. Psy Kick";
            track_names[1]  = "02. Sub Bass (KB-B-B)";
            track_names[2]  = "03. Mid Saw Bass";
            track_names[3]  = "04. Snare & Smash Clap";
            track_names[4]  = "05. Offbeat Open Hat";
            track_names[5]  = "06. Closed Hats / Shaker";
            track_names[6]  = "07. Tribal Percussion";
            track_names[7]  = "08. FM Squelch & Zaps";
            track_names[8]  = "09. Main Acid Lead (303)";
            track_names[9]  = "10. Counter-Arp Pluck";
            track_names[10] = "11. SuperSaw Poly Lead";
            track_names[11] = "12. Dark Sub Drone";
            track_names[12] = "13. Mystic Strings & Pads";
            track_names[13] = "14. Steinway Piano";
            track_names[14] = "15. Vocal Mantra Chants";
            track_names[15] = "16. Snare Roll & FX Risers";
            track_names[16] = "17. [AUTO] Filter Cutoff";
            track_names[17] = "18. [AUTO] Reverb Washout";
            track_names[18] = "19. [AUTO] Pitch Riser";

            g_piano_synth.flex_active[0]  = false; // Drum Sampler Kick
            g_piano_synth.flex_active[1]  = true;  g_piano_synth.flex_channel_instrument[1]  = KuroAudio::MidiInstrument::SUB_BASS;
            g_piano_synth.flex_active[2]  = true;  g_piano_synth.flex_channel_instrument[2]  = KuroAudio::MidiInstrument::SYNTH_SAW;
            g_piano_synth.flex_active[3]  = false; // Drum Sampler Snare
            g_piano_synth.flex_active[4]  = false; // Open Hat
            g_piano_synth.flex_active[5]  = false; // Closed Hat
            g_piano_synth.flex_active[6]  = true;  g_piano_synth.flex_channel_instrument[6]  = KuroAudio::MidiInstrument::MARIMBA;
            g_piano_synth.flex_active[7]  = true;  g_piano_synth.flex_channel_instrument[7]  = KuroAudio::MidiInstrument::FM_SYNTH;
            g_piano_synth.flex_active[8]  = true;  g_piano_synth.flex_channel_instrument[8]  = KuroAudio::MidiInstrument::SYNTH_SAW;
            g_piano_synth.flex_active[9]  = true;  g_piano_synth.flex_channel_instrument[9]  = KuroAudio::MidiInstrument::TRANCE_LEAD;
            g_piano_synth.flex_active[10] = true;  g_piano_synth.flex_channel_instrument[10] = KuroAudio::MidiInstrument::FULLON_LEAD;
            g_piano_synth.flex_active[11] = true;  g_piano_synth.flex_channel_instrument[11] = KuroAudio::MidiInstrument::PAD_SYNTH;
            g_piano_synth.flex_active[12] = true;  g_piano_synth.flex_channel_instrument[12] = KuroAudio::MidiInstrument::STRING_ENSEMBLE;
            g_piano_synth.flex_active[13] = true;  g_piano_synth.flex_channel_instrument[13] = KuroAudio::MidiInstrument::ACOUSTIC_PIANO;
            g_piano_synth.flex_active[14] = true;  g_piano_synth.flex_channel_instrument[14] = KuroAudio::MidiInstrument::CHOIR;
            g_piano_synth.flex_active[15] = true;  g_piano_synth.flex_channel_instrument[15] = KuroAudio::MidiInstrument::SWEEP_PAD;

            // Carregamento de Samples do Sonicspore PRYZMA se solicitado
            if (cfg.use_pryzma_samples) {
                std::string pryzma_kick = "assets/samples/Sonicspore_PRYZMA/01_Kick/Sonicspore _ FPS _ Kick 01 - 144 BPM.wav";
                if (std::filesystem::exists(pryzma_kick)) {
                    g_piano_synth.getDrumSample(0).load(pryzma_kick);
                    g_piano_synth.sampler_settings[0].root_note = 36;
                }
                std::string pryzma_snare = "assets/samples/Sonicspore_PRYZMA/03_Snare/Sonicspore FPS _ Snare 01 .wav";
                if (std::filesystem::exists(pryzma_snare)) {
                    g_piano_synth.getDrumSample(3).load(pryzma_snare);
                    g_piano_synth.sampler_settings[3].root_note = 38;
                }
            }

            // 2. Criação dos Patterns Musicais
            cm.global_patterns.clear();

            // ── Pattern 1: PSY KICK (4-on-the-floor) ──
            Pattern p_kick;
            p_kick.id = 1;
            p_kick.name = "01. Psy Kick (Punch)";
            p_kick.color = 0xFF00E5FF;
            for (int b = 0; b < 8; ++b) {
                for (int beat_i = 0; beat_i < 4; ++beat_i) {
                    p_kick.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, b * bar + beat_i * beat, 0.18f, 1.0f, 1.0f, 0));
                }
            }
            cm.global_patterns.push_back(p_kick);

            // ── Pattern 2: ROLLING BASS (Sub & Mid Saw KB-B-B) ──
            Pattern p_bass;
            p_bass.id = 2;
            p_bass.name = "02. Rolling Bass (KB-B-B)";
            p_bass.color = 0xFFFF0080;
            int r_note = cfg.root_note; // ex: 30 = F#1
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                int bar_root = r_note;
                if (b >= 4 && b < 6) bar_root = r_note + 3; // Minor 3rd
                else if (b == 6)     bar_root = r_note - 2; // Flat 7th
                else if (b == 7)     bar_root = r_note - 4; // Flat 6th

                for (int beat_i = 0; beat_i < 4; ++beat_i) {
                    float beat_offset = bar_offset + beat_i * beat;
                    // KB-B-B: Sub/Mid notas nos steps 1, 2, 3 de cada tempo
                    for (int s = 1; s <= 3; ++s) {
                        float note_t = beat_offset + s * step;
                        float dur = step * 0.88f;
                        float vel = (s == 1) ? 0.94f : 0.86f;
                        // Sub Bass (Ch 1)
                        p_bass.getChannelNotes(1).push_back(KuroDSP::MidiNote(bar_root, note_t, dur, vel, 1.0f, 1));
                        // Mid Saw Bass (Ch 2 - oitava acima)
                        p_bass.getChannelNotes(2).push_back(KuroDSP::MidiNote(bar_root + 12, note_t, dur * 0.90f, vel * 0.90f, 1.0f, 2));
                    }
                }
            }
            cm.global_patterns.push_back(p_bass);

            // ── Pattern 3: OFFBEAT OPEN HI-HAT ──
            Pattern p_open_hat;
            p_open_hat.id = 3;
            p_open_hat.name = "03. Offbeat Open Hat";
            p_open_hat.color = 0xFFFFDD00;
            for (int b = 0; b < 8; ++b) {
                for (int beat_i = 0; beat_i < 4; ++beat_i) {
                    float hat_t = b * bar + beat_i * beat + step * 2.0f; // No "&" de cada tempo
                    p_open_hat.getChannelNotes(4).push_back(KuroDSP::MidiNote(46, hat_t, step * 1.5f, 0.92f, 1.0f, 4));
                }
            }
            cm.global_patterns.push_back(p_open_hat);

            // ── Pattern 4: SNARE & SMASH CLAP ──
            Pattern p_snare;
            p_snare.id = 4;
            p_snare.name = "04. Snare & Smash Clap";
            p_snare.color = 0xFFFF8000;
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                // Batida nos tempos 2 e 4
                p_snare.getChannelNotes(3).push_back(KuroDSP::MidiNote(38, bar_offset + beat * 1.0f, 0.25f, 0.96f, 1.0f, 3));
                p_snare.getChannelNotes(3).push_back(KuroDSP::MidiNote(38, bar_offset + beat * 3.0f, 0.25f, 0.96f, 1.0f, 3));
                // Fill no 4º compasso
                if (b % 4 == 3) {
                    p_snare.getChannelNotes(3).push_back(KuroDSP::MidiNote(38, bar_offset + beat * 3.5f, 0.12f, 0.85f, 1.0f, 3));
                    p_snare.getChannelNotes(3).push_back(KuroDSP::MidiNote(38, bar_offset + beat * 3.75f, 0.10f, 0.90f, 1.0f, 3));
                }
            }
            cm.global_patterns.push_back(p_snare);

            // ── Pattern 5: CLOSED HATS & SHAKERS ──
            Pattern p_shakers;
            p_shakers.id = 5;
            p_shakers.name = "05. Closed Hats & Shakers";
            p_shakers.color = 0xFF80FF00;
            for (int b = 0; b < 8; ++b) {
                for (int s = 0; s < 16; ++s) {
                    if (s % 4 == 2) continue; // Pula offbeat ocupado pelo open hat
                    float vel = (s % 2 == 1) ? 0.70f : 0.45f;
                    p_shakers.getChannelNotes(5).push_back(KuroDSP::MidiNote(42, b * bar + s * step, step * 0.8f, vel, 1.0f, 5));
                }
            }
            cm.global_patterns.push_back(p_shakers);

            // ── Pattern 6: BRAZILIAN TRIBAL PERCUSSION ──
            Pattern p_tribal;
            p_tribal.id = 6;
            p_tribal.name = "06. Tribal Percussion";
            p_tribal.color = 0xFF00FF80;
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                p_tribal.getChannelNotes(6).push_back(KuroDSP::MidiNote(65, bar_offset + step * 3.0f, step * 1.2f, 0.85f, 1.0f, 6));
                p_tribal.getChannelNotes(6).push_back(KuroDSP::MidiNote(62, bar_offset + step * 6.0f, step * 1.2f, 0.88f, 1.0f, 6));
                p_tribal.getChannelNotes(6).push_back(KuroDSP::MidiNote(67, bar_offset + step * 10.0f, step * 1.2f, 0.92f, 1.0f, 6));
                p_tribal.getChannelNotes(6).push_back(KuroDSP::MidiNote(69, bar_offset + step * 14.0f, step * 1.2f, 0.90f, 1.0f, 6));
            }
            cm.global_patterns.push_back(p_tribal);

            // ── Pattern 7: FM SQUELCH & MODULAR ZAPS ──
            Pattern p_squelch;
            p_squelch.id = 7;
            p_squelch.name = "07. FM Squelch & Modular Zaps";
            p_squelch.color = 0xFF00FFC8;
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                if (b % 2 == 1) {
                    p_squelch.getChannelNotes(7).push_back(KuroDSP::MidiNote(78, bar_offset + step * 12.0f, step * 0.8f, 0.95f, 1.0f, 7));
                    p_squelch.getChannelNotes(7).push_back(KuroDSP::MidiNote(81, bar_offset + step * 13.5f, step * 0.8f, 0.98f, 1.0f, 7));
                    p_squelch.getChannelNotes(7).push_back(KuroDSP::MidiNote(73, bar_offset + step * 15.0f, step * 0.8f, 0.92f, 1.0f, 7));
                }
            }
            cm.global_patterns.push_back(p_squelch);

            // ── Pattern 8: MAIN ACID LEAD (303 Screamer) ──
            Pattern p_acid;
            p_acid.id = 8;
            p_acid.name = "08. Main Acid Lead Hook (303)";
            p_acid.color = 0xFF00B4FF;
            int acid_root = r_note + 36; // 3 oitavas acima
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                int cur_root = acid_root;
                if (b >= 4 && b < 6) cur_root = acid_root + 3;
                else if (b == 6)     cur_root = acid_root - 2;
                else if (b == 7)     cur_root = acid_root - 4;

                for (int s = 0; s < 16; ++s) {
                    int note = cur_root;
                    if (s == 3 || s == 7) note += 3;
                    else if (s == 8 || s == 12) note += 7;
                    else if (s == 11 || s == 15) note += 12;
                    p_acid.getChannelNotes(8).push_back(KuroDSP::MidiNote(note, bar_offset + s * step, step * 0.85f, 0.94f, 1.0f, 8));
                }
            }
            cm.global_patterns.push_back(p_acid);

            // ── Pattern 9: COUNTER-ARP PLUCK MATRIX ──
            Pattern p_counter_arp;
            p_counter_arp.id = 9;
            p_counter_arp.name = "09. Counter-Arp Pluck Matrix";
            p_counter_arp.color = 0xFF5078FF;
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                int arp_seq[8] = { r_note + 48, r_note + 46, r_note + 43, r_note + 39, r_note + 41, r_note + 43, r_note + 46, r_note + 51 };
                for (int i = 0; i < 8; ++i) {
                    p_counter_arp.getChannelNotes(9).push_back(KuroDSP::MidiNote(arp_seq[i], bar_offset + i * (step * 2.0f), step * 1.1f, 0.88f, 1.0f, 9));
                }
            }
            cm.global_patterns.push_back(p_counter_arp);

            // ── Pattern 10: SUPERSAW POLYPHONIC LEAD (Peak Climax) ──
            Pattern p_supersaw;
            p_supersaw.id = 10;
            p_supersaw.name = "10. SuperSaw Poly Lead Hook";
            p_supersaw.color = 0xFFA050FF;
            for (int b = 0; b < 8; b += 2) {
                float chord_t = b * bar;
                int c_root = r_note + 24;
                if (b == 2) c_root += 3;
                else if (b == 4) c_root -= 2;
                else if (b == 6) c_root -= 4;

                p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(c_root, chord_t, bar * 2.0f, 0.95f, 1.0f, 10));
                p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(c_root + 7, chord_t, bar * 2.0f, 0.95f, 1.0f, 10));
                p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(c_root + 12, chord_t, bar * 2.0f, 0.95f, 1.0f, 10));
                p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(c_root + 15, chord_t, bar * 2.0f, 0.95f, 1.0f, 10));
            }
            cm.global_patterns.push_back(p_supersaw);

            // ── Pattern 11: DARK SUB DRONE ──
            Pattern p_drone;
            p_drone.id = 11;
            p_drone.name = "11. Dark Sub Drone";
            p_drone.color = 0xFF7828D2;
            p_drone.getChannelNotes(11).push_back(KuroDSP::MidiNote(r_note - 12, 0.0f, bar * 8.0f, 0.95f, 1.0f, 11));
            cm.global_patterns.push_back(p_drone);

            // ── Pattern 12: MYSTIC STRINGS & PADS ──
            Pattern p_strings;
            p_strings.id = 12;
            p_strings.name = "12. Mystic Strings & Pads";
            p_strings.color = 0xFF00D2E6;
            for (int b = 0; b < 8; b += 2) {
                float chord_t = b * bar;
                int s_root = r_note + 24;
                if (b == 2) s_root += 3;
                else if (b == 4) s_root -= 2;
                else if (b == 6) s_root -= 4;
                p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(s_root, chord_t, bar * 2.0f, 0.88f, 1.0f, 12));
                p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(s_root + 7, chord_t, bar * 2.0f, 0.88f, 1.0f, 12));
                p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(s_root + 15, chord_t, bar * 2.0f, 0.90f, 1.0f, 12));
            }
            cm.global_patterns.push_back(p_strings);

            // ── Pattern 13: STEINWAY PIANO STABS ──
            Pattern p_piano;
            p_piano.id = 13;
            p_piano.name = "13. Steinway Piano Stabs";
            p_piano.color = 0xFFFFD700;
            for (int b = 0; b < 8; b += 2) {
                float stab_t = b * bar;
                int p_root = r_note + 36;
                p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(p_root, stab_t, bar * 1.5f, 0.92f, 1.0f, 13));
                p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(p_root + 3, stab_t, bar * 1.5f, 0.92f, 1.0f, 13));
                p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(p_root + 7, stab_t, bar * 1.5f, 0.92f, 1.0f, 13));
            }
            cm.global_patterns.push_back(p_piano);

            // ── Pattern 14: VOCAL MANTRA CHANTS ──
            Pattern p_vocal;
            p_vocal.id = 14;
            p_vocal.name = "14. Vocal Mantra Chants";
            p_vocal.color = 0xFFFF5078;
            for (int b = 0; b < 8; b += 2) {
                float v_t = b * bar;
                p_vocal.getChannelNotes(14).push_back(KuroDSP::MidiNote(r_note + 24, v_t, bar * 1.8f, 0.90f, 1.0f, 14));
            }
            cm.global_patterns.push_back(p_vocal);

            // ── Pattern 15: SNARE ROLL CRESCENDO & FX RISERS ──
            Pattern p_riser;
            p_riser.id = 15;
            p_riser.name = "15. Snare Roll & FX Risers";
            p_riser.color = 0xFFFF3232;
            // Snare roll acelerando nos últimos 4 compassos
            for (int s = 0; s < 32; ++s) {
                float progress = (float)s / 32.0f;
                float vel = 0.40f + 0.60f * progress;
                float note_t = bar * 4.0f + s * (bar * 4.0f / 32.0f);
                p_riser.getChannelNotes(15).push_back(KuroDSP::MidiNote(38, note_t, 0.10f, vel, 1.0f, 15));
            }
            cm.global_patterns.push_back(p_riser);

            // 3. Montagem dos Blocos na Playlist por Seção
            auto addClip = [&](int track, int pat_id, float start_sec, float len_sec) {
                MidiClip mc;
                mc.id = cm.next_id++;
                mc.start_time_sec = start_sec;
                mc.length_sec = len_sec;
                mc.pattern_id = pat_id;
                mc.is_selected = false;
                mc.is_muted = false;
                for (const auto& pat : cm.global_patterns) {
                    if (pat.id == pat_id) { mc.name = pat.name; break; }
                }
                if (mc.name.empty()) mc.name = "Pattern " + std::to_string(pat_id);
                cm.track_midi_clips[track].push_back(mc);
            };

            auto addAudioClip = [&](int track, const std::string& name, float start_sec, float len_sec, unsigned int col) {
                AudioClip ac;
                ac.id = cm.next_id++;
                ac.name = name;
                ac.start_time_sec = start_sec;
                ac.length_sec = len_sec;
                ac.source_offset_sec = 0.0f;
                // ac.color = col;
                ac.is_selected = false;
                ac.is_muted = false;
                cm.track_clips[track].push_back(ac);
            };

            float cur_t = 0.0f;

            // ── 1. INTRO (Full Psytrance Intro with Stereo Stems & Neon MIDI Patterns) ──
            if (cfg.has_intro && cfg.intro_bars > 0) {
                float sec_len = cfg.intro_bars * bar;
                tl.addSectionMarker(cur_t, "INTRO", 0xFFFFD700);

                int blocks = cfg.intro_bars / 8;
                if (blocks < 1) blocks = 1;
                float block_dur = 8.0f * bar;

                // Audio Stem Kick Stereo contínuo por toda a Intro (Compassos 1 a 16)
                addAudioClip(0, "PRYZMA_PsyKick_Stereo_Stem.wav", cur_t, sec_len, 0);

                // Compasso 1 a 8: Atmosfera, Arpeggios e Percussão Tribal
                addClip(6, 6, cur_t, block_dur);   // 06. Tribal Percussion
                addClip(7, 7, cur_t, block_dur);   // 07. FM Squelch & Modular Zaps
                addClip(9, 9, cur_t, block_dur);   // 09. Counter-Arp Pluck Matrix
                addClip(11, 11, cur_t, sec_len);   // 11. Dark Sub Drone
                addClip(12, 12, cur_t, sec_len);   // 12. Mystic Strings & Pads
                addAudioClip(14, "PRYZMA_Vocal_Chant_142.wav", cur_t, sec_len, 0xFFFF5078);
                addAudioClip(15, "PRYZMA_FX_Atmo_Sweep.wav", cur_t, sec_len, 0xFFFF3232);

                // Compasso 9 a 16 (Intro Parte 2): O Rolling Bassline entra com força total!
                float t_p2 = cur_t + block_dur;
                addClip(1, 2, t_p2, block_dur);    // 02. Sub Bass (KB-B-B)
                addClip(2, 2, t_p2, block_dur);    // 03. Mid Saw Bass (KB-B-B)
                addClip(3, 4, t_p2, block_dur);    // 04. Snare & Smash Clap
                addAudioClip(4, "PRYZMA_Percussion_Stereo_Loop.wav", t_p2, block_dur, 0);
                addClip(5, 5, t_p2, block_dur);    // 05. Closed Hats & Shakers
                addClip(6, 6, t_p2, block_dur);    // 06. Tribal Percussion
                addClip(7, 7, t_p2, block_dur);    // 07. FM Squelch
                addClip(8, 8, t_p2, block_dur);    // 08. Main Acid Lead (303 Hook)
                addClip(9, 9, t_p2, block_dur);    // 09. Counter-Arp Pluck Matrix

                cur_t += sec_len;
            }

            // ── 2. BUILD-UP 1 ──
            if (cfg.has_buildup1 && cfg.buildup1_bars > 0) {
                float sec_len = cfg.buildup1_bars * bar;
                tl.addSectionMarker(cur_t, "BUILD-UP 1", 0xFFFF007F);

                addClip(12, 12, cur_t, sec_len); // Mystic Strings swell
                addClip(9, 9, cur_t, sec_len);   // Counter-Arp teaser
                addClip(11, 11, cur_t, sec_len); // Dark Sub Drone
                addClip(15, 15, cur_t, sec_len); // Snare Roll Crescendo

                cur_t += sec_len;
            }

            // ── 3. DROP 1 (FULL-ON DRIVE) ──
            if (cfg.has_drop1 && cfg.drop1_bars > 0) {
                float sec_len = cfg.drop1_bars * bar;
                tl.addSectionMarker(cur_t, "DROP 1 (FULL-ON)", 0xFF00E5FF);

                int blocks = cfg.drop1_bars / 8;
                if (blocks < 1) blocks = 1;
                float block_dur = 8.0f * bar;

                for (int b = 0; b < blocks; ++b) {
                    float bt = cur_t + b * block_dur;
                    addClip(0, 1, bt, block_dur); // Psy Kick
                    addAudioClip(0, "PRYZMA_Kick01_144BPM.wav", bt, block_dur, 0xFF00E5FF);
                    addClip(1, 2, bt, block_dur); // Sub Bass KB-B-B
                    addClip(2, 2, bt, block_dur); // Mid Saw Bass
                    addClip(4, 3, bt, block_dur); // Offbeat Open Hat
                    addClip(3, 4, bt, block_dur); // Snare & Smash Clap
                    addClip(7, 7, bt, block_dur); // FM Squelches & Zaps
                    addClip(8, 8, bt, block_dur); // Main Acid Lead Hook
                    addClip(6, 6, bt, block_dur); // Tribal Percussion
                }
                cur_t += sec_len;
            }

            // ── 4. BREAKDOWN / CHILL ──
            if (cfg.has_break && cfg.break_bars > 0) {
                float sec_len = cfg.break_bars * bar;
                tl.addSectionMarker(cur_t, "BREAKDOWN", 0xFF00FF7F);

                int blocks = cfg.break_bars / 8;
                if (blocks < 1) blocks = 1;
                float block_dur = 8.0f * bar;

                for (int b = 0; b < blocks; ++b) {
                    float bt = cur_t + b * block_dur;
                    addClip(12, 12, bt, block_dur); // Mystic Strings
                    addClip(13, 13, bt, block_dur); // Steinway Piano
                    addClip(14, 14, bt, block_dur); // Vocal Mantra
                    addClip(9, 9, bt, block_dur);   // Counter-Arp
                    addClip(11, 11, bt, block_dur); // Dark Drone
                }
                cur_t += sec_len;
            }

            // ── 5. BUILD-UP 2 (CLIMAX RUSH) ──
            if (cfg.has_buildup2 && cfg.buildup2_bars > 0) {
                float sec_len = cfg.buildup2_bars * bar;
                tl.addSectionMarker(cur_t, "BUILD-UP 2 (CLIMAX)", 0xFFFF00FF);

                addClip(15, 15, cur_t, sec_len); // Snare Roll
                addClip(10, 10, cur_t, sec_len); // SuperSaw Lead teaser
                addClip(8, 8, cur_t, sec_len);   // Acid Lead stutter
                addClip(11, 11, cur_t, sec_len); // Drone

                cur_t += sec_len;
            }

            // ── 6. DROP 2 (PEAK ENERGY / SUPERSAW ANTHEM) ──
            if (cfg.has_drop2 && cfg.drop2_bars > 0) {
                float sec_len = cfg.drop2_bars * bar;
                tl.addSectionMarker(cur_t, "DROP 2 (PEAK ENERGY)", 0xFF00B4FF);

                int blocks = cfg.drop2_bars / 8;
                if (blocks < 1) blocks = 1;
                float block_dur = 8.0f * bar;

                for (int b = 0; b < blocks; ++b) {
                    float bt = cur_t + b * block_dur;
                    addClip(0, 1, bt, block_dur);   // Kick
                    addClip(1, 2, bt, block_dur);   // Sub Bass
                    addClip(2, 2, bt, block_dur);   // Mid Bass
                    addClip(4, 3, bt, block_dur);   // Open Hat
                    addClip(3, 4, bt, block_dur);   // Snare
                    addClip(10, 10, bt, block_dur); // SuperSaw Poly Lead Hook
                    addClip(8, 8, bt, block_dur);   // Acid Lead
                    addClip(7, 7, bt, block_dur);   // FM Squelch
                    addClip(6, 6, bt, block_dur);   // Tribal Perc
                }
                cur_t += sec_len;
            }

            // ── 7. OUTRO ──
            if (cfg.has_outro && cfg.outro_bars > 0) {
                float sec_len = cfg.outro_bars * bar;
                tl.addSectionMarker(cur_t, "OUTRO", 0xFFFF9900);

                int blocks = cfg.outro_bars / 8;
                if (blocks < 1) blocks = 1;
                float block_dur = 8.0f * bar;

                for (int b = 0; b < blocks; ++b) {
                    float bt = cur_t + b * block_dur;
                    addClip(0, 1, bt, block_dur); // Kick
                    addClip(1, 2, bt, block_dur); // Sub Bass
                    addClip(4, 3, bt, block_dur); // Open Hat
                    addClip(6, 6, bt, block_dur); // Tribal Perc
                }
                cur_t += sec_len;
            }

            // 4. Automações Dinâmicas (Filter Sweep, Reverb Washout, Pitch Riser)
            float total_song_sec = cur_t;

            // ── Automação de Filter Cutoff Sweep (Track 16) ──
            if (cfg.enable_filter_sweep && total_song_sec > 10.0f) {
                AutomationClip ac_filter;
                ac_filter.id = cm.next_id++;
                ac_filter.name = "Filter Cutoff Sweep (Master)";
                ac_filter.color = 0xFF00FF7F; // Neon Green
                ac_filter.target_node_id = "MasterFilter";
                ac_filter.param_index = 0;
                ac_filter.start_time_sec = 0.0f;
                ac_filter.length_sec = total_song_sec;
                ac_filter.points.clear();

                // Curva do Filtro por Seção
                float t_ptr = 0.0f;
                ac_filter.points.push_back({0.0f, 0.20f, 0.3f}); // Começa filtrado

                if (cfg.has_intro) {
                    t_ptr += cfg.intro_bars * bar;
                    ac_filter.points.push_back({t_ptr, 0.60f, 0.4f}); // Abre até 60%
                }
                if (cfg.has_buildup1) {
                    t_ptr += cfg.buildup1_bars * bar;
                    ac_filter.points.push_back({t_ptr - bar, 0.95f, 0.6f}); // Rampa de subida
                    ac_filter.points.push_back({t_ptr, 1.00f, 0.0f});       // Drop 100% aberto
                }
                if (cfg.has_drop1) {
                    t_ptr += cfg.drop1_bars * bar;
                    ac_filter.points.push_back({t_ptr, 1.00f, -0.4f});      // Mantém aberto
                }
                if (cfg.has_break) {
                    t_ptr += cfg.break_bars * bar;
                    ac_filter.points.push_back({t_ptr - bar * 4.0f, 0.35f, 0.3f}); // Fecha no break
                    ac_filter.points.push_back({t_ptr, 0.70f, 0.5f});
                }
                if (cfg.has_buildup2) {
                    t_ptr += cfg.buildup2_bars * bar;
                    ac_filter.points.push_back({t_ptr - bar * 0.5f, 1.00f, 0.8f}); // Pico explosivo
                    ac_filter.points.push_back({t_ptr, 1.00f, 0.0f});
                }
                if (cfg.has_drop2) {
                    t_ptr += cfg.drop2_bars * bar;
                    ac_filter.points.push_back({t_ptr, 1.00f, -0.5f});
                }
                if (cfg.has_outro) {
                    t_ptr += cfg.outro_bars * bar;
                    ac_filter.points.push_back({t_ptr, 0.25f, -0.3f}); // Fecha no fade out
                }
                cm.track_auto_clips[16].push_back(ac_filter);
            }

            // ── Automação de Reverb Washout (Track 17) ──
            if (cfg.enable_reverb_washout && total_song_sec > 10.0f) {
                AutomationClip ac_reverb;
                ac_reverb.id = cm.next_id++;
                ac_reverb.name = "Reverb Wet Washout";
                ac_reverb.color = 0xFF00E5FF; // Neon Cyan
                ac_reverb.target_node_id = "MasterReverb";
                ac_reverb.param_index = 1;
                ac_reverb.start_time_sec = 0.0f;
                ac_reverb.length_sec = total_song_sec;
                ac_reverb.points.clear();

                ac_reverb.points.push_back({0.0f, 0.05f, 0.0f}); // 5% Reverb seco

                float t_p = 0.0f;
                if (cfg.has_intro) t_p += cfg.intro_bars * bar;
                if (cfg.has_buildup1) {
                    ac_reverb.points.push_back({t_p + (cfg.buildup1_bars - 2) * bar, 0.08f, 0.6f});
                    ac_reverb.points.push_back({t_p + cfg.buildup1_bars * bar - 0.2f, 0.85f, 0.8f}); // Washout pico
                    ac_reverb.points.push_back({t_p + cfg.buildup1_bars * bar, 0.05f, 0.0f});         // Corta seco no Drop!
                    t_p += cfg.buildup1_bars * bar;
                }
                if (cfg.has_drop1) {
                    ac_reverb.points.push_back({t_p + cfg.drop1_bars * bar, 0.05f, 0.0f});
                    t_p += cfg.drop1_bars * bar;
                }
                if (cfg.has_break) {
                    ac_reverb.points.push_back({t_p + bar * 4.0f, 0.40f, 0.3f}); // Ambiente espacial no break
                    t_p += cfg.break_bars * bar;
                }
                if (cfg.has_buildup2) {
                    ac_reverb.points.push_back({t_p + (cfg.buildup2_bars - 2) * bar, 0.10f, 0.7f});
                    ac_reverb.points.push_back({t_p + cfg.buildup2_bars * bar - 0.2f, 0.90f, 0.9f}); // Climax Washout
                    ac_reverb.points.push_back({t_p + cfg.buildup2_bars * bar, 0.05f, 0.0f});         // Corta seco no Drop 2!
                    t_p += cfg.buildup2_bars * bar;
                }
                if (cfg.has_drop2) {
                    ac_reverb.points.push_back({t_p + cfg.drop2_bars * bar, 0.05f, 0.0f});
                    t_p += cfg.drop2_bars * bar;
                }
                if (cfg.has_outro) {
                    ac_reverb.points.push_back({t_p + cfg.outro_bars * bar, 0.30f, 0.2f});
                }
                cm.track_auto_clips[17].push_back(ac_reverb);
            }

            // ── Automação de Pitch Riser (Track 18) ──
            if (cfg.enable_pitch_riser && total_song_sec > 10.0f) {
                AutomationClip ac_pitch;
                ac_pitch.id = cm.next_id++;
                ac_pitch.name = "Pitch Riser (+12 Semitones)";
                ac_pitch.color = 0xFFFF00FF; // Magenta
                ac_pitch.target_node_id = "MasterPitch";
                ac_pitch.param_index = 2;
                ac_pitch.start_time_sec = 0.0f;
                ac_pitch.length_sec = total_song_sec;
                ac_pitch.points.clear();

                ac_pitch.points.push_back({0.0f, 0.0f, 0.0f});

                float t_p2 = 0.0f;
                if (cfg.has_intro) t_p2 += cfg.intro_bars * bar;
                if (cfg.has_buildup1) {
                    ac_pitch.points.push_back({t_p2 + (cfg.buildup1_bars - 4) * bar, 0.0f, 0.7f});
                    ac_pitch.points.push_back({t_p2 + cfg.buildup1_bars * bar, 1.0f, 0.0f}); // Riser sobe
                    ac_pitch.points.push_back({t_p2 + cfg.buildup1_bars * bar + 0.05f, 0.0f, 0.0f}); // Reseta no Drop
                    t_p2 += cfg.buildup1_bars * bar;
                }
                if (cfg.has_drop1) t_p2 += cfg.drop1_bars * bar;
                if (cfg.has_break) t_p2 += cfg.break_bars * bar;
                if (cfg.has_buildup2) {
                    ac_pitch.points.push_back({t_p2 + (cfg.buildup2_bars - 4) * bar, 0.0f, 0.8f});
                    ac_pitch.points.push_back({t_p2 + cfg.buildup2_bars * bar, 1.0f, 0.0f}); // Riser clímax
                    ac_pitch.points.push_back({t_p2 + cfg.buildup2_bars * bar + 0.05f, 0.0f, 0.0f});
                    t_p2 += cfg.buildup2_bars * bar;
                }
                cm.track_auto_clips[18].push_back(ac_pitch);
            }

            std::cout << "[PsySongArranger] Arranjo gerado com sucesso: " 
                      << total_song_sec << "s (" << (int)(total_song_sec / bar) << " compassos, " 
                      << cm.global_patterns.size() << " patterns, " 
                      << tl.section_markers.size() << " marcadores de secao)!\n";
        }
    };
}
