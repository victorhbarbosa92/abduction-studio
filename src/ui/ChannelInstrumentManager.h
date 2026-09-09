#pragma once
#include "imgui.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include "../core/ClipManager.h"
#include "../audio/SynthEngine.h"

extern std::string track_names[MAX_TRACKS];
extern KuroAudio::SynthEngine g_piano_synth;

namespace KuroUI {

    extern int selected_track_idx;
    extern bool show_piano_roll;
    extern bool show_sampler_settings;
    extern bool show_flex_browser;
    extern bool show_contrabass_window;
    extern bool show_delay_lama;
    extern bool focus_delay_lama;
    extern int active_flex_channel;
    extern int active_sampler_channel;

    enum class ChannelInstrumentType {
        KURO_RHYTHM_BASS = 0,    // 🛸 Kuro Psytrance Rhythm & Bass Workstation
        FL_FLEX_SYNTH,           // 🎹 FL Flex Synth (Psytrance Leads & Arps)
        DIRECTWAVE,              // 🌊 FL DirectWave (Multi-Sampler)
        SYTRUS_FM,               // 📻 Sytrus (6-Op FM Synth)
        HARMOR_ADDITIVE,         // 🔮 Harmor (Additive Resynthesis)
        FRUITY_GRANULIZER,       // ☁️ Fruity Granulizer (Grain Cloud)
        SLICEX,                  // ✂️ SliceX (Beat Slicer)
        ALIEN_LED,               // 👾 Alien LED Synth (Sci-Fi)
        DELAY_LAMA,              // 🧘 Delay Lama (Monge 3D)
        CONTRABASS,              // 🎻 Contrabaixo Acústico Real
        SOUNDFONT_PLAYER,        // 🎼 Soundfont Player (.SF2)
        OSC_3X,                  // 🎛️ 3x Osc (Triple Oscillator)
        AUDIO_SAMPLE,            // 🥁 Audio Sample (Kick / Snare / Hat / Perc)
        VST3_CLAP_PLUGIN         // 🔌 Gerenciador VST3 / CLAP
    };

    struct ChannelSlotConfig {
        ChannelInstrumentType type = ChannelInstrumentType::AUDIO_SAMPLE;
        std::string custom_title = "";
        std::string sample_name = "";
        ImU32 custom_color = 0; // 0 = default
        int kuro_layer_tab = 0; // 0=Full, 1=Kick, 2=Bass, 3=Snare
    };

    // Configuração dos 16 canais de instrumentos
    inline ChannelSlotConfig g_channel_slots[MAX_TRACKS] = {
        { ChannelInstrumentType::KURO_RHYTHM_BASS, "Track 1", "Kick 4x4", IM_COL32(255, 140, 30, 255), 1 },       // Ch 0: Kick
        { ChannelInstrumentType::KURO_RHYTHM_BASS, "Track 2", "Rolling Bass", IM_COL32(0, 229, 255, 255), 2 },   // Ch 1: Bass
        { ChannelInstrumentType::KURO_RHYTHM_BASS, "Track 3", "Snare / Clap", IM_COL32(230, 70, 200, 255), 3 },  // Ch 2: Snare
        { ChannelInstrumentType::AUDIO_SAMPLE,     "Track 4", "808 HiHat", 0, 0 },                                  // Ch 3: Hi-Hat
        { ChannelInstrumentType::FL_FLEX_SYNTH,    "Track 5", "Psy Lead", 0, 0 },                                   // Ch 4: Flex Synth
        { ChannelInstrumentType::SYTRUS_FM,        "Track 6", "FM Squelch", 0, 0 },                                 // Ch 5: Sytrus FM
        { ChannelInstrumentType::DIRECTWAVE,       "Track 7", "Tribal Perc", 0, 0 },                                // Ch 6: DirectWave
        { ChannelInstrumentType::HARMOR_ADDITIVE,  "Track 8", "Cosmic Pad", 0, 0 },                                 // Ch 7: Harmor
        { ChannelInstrumentType::ALIEN_LED,        "Track 9", "Sci-Fi Lead", 0, 0 },                                // Ch 8: Alien LED
        { ChannelInstrumentType::DELAY_LAMA,       "Track 10", "Monk Chant", 0, 0 },                                // Ch 9: Delay Lama
        { ChannelInstrumentType::CONTRABASS,       "Track 11", "Upright Bass", 0, 0 },                              // Ch 10: Contrabass
        { ChannelInstrumentType::SOUNDFONT_PLAYER, "Track 12", "Strings SF2", 0, 0 },                               // Ch 11: Soundfont
        { ChannelInstrumentType::OSC_3X,           "Track 13", "Triple Saw", 0, 0 },                                // Ch 12: 3x Osc
        { ChannelInstrumentType::FRUITY_GRANULIZER,"Track 14", "Grain Cloud", 0, 0 },                               // Ch 13: Granulizer
        { ChannelInstrumentType::SLICEX,           "Track 15", "Beat Slicer", 0, 0 },                               // Ch 14: SliceX
        { ChannelInstrumentType::VST3_CLAP_PLUGIN, "Track 16", "External VST", 0, 0 }                                // Ch 15: VST/CLAP
    };

    inline const char* GetChannelIcon(int ch_idx) {
        if (ch_idx < 0 || ch_idx >= MAX_TRACKS) return "🎛️";
        switch (g_channel_slots[ch_idx].type) {
            case ChannelInstrumentType::KURO_RHYTHM_BASS:
                if (g_channel_slots[ch_idx].kuro_layer_tab == 1) return "🥊";
                if (g_channel_slots[ch_idx].kuro_layer_tab == 2) return "⚡";
                if (g_channel_slots[ch_idx].kuro_layer_tab == 3) return "💥";
                return "🛸";
            case ChannelInstrumentType::FL_FLEX_SYNTH:     return "🎹";
            case ChannelInstrumentType::DIRECTWAVE:        return "🌊";
            case ChannelInstrumentType::SYTRUS_FM:         return "📻";
            case ChannelInstrumentType::HARMOR_ADDITIVE:   return "🔮";
            case ChannelInstrumentType::FRUITY_GRANULIZER: return "☁️";
            case ChannelInstrumentType::SLICEX:            return "✂️";
            case ChannelInstrumentType::ALIEN_LED:         return "👾";
            case ChannelInstrumentType::DELAY_LAMA:        return "🧘";
            case ChannelInstrumentType::CONTRABASS:        return "🎻";
            case ChannelInstrumentType::SOUNDFONT_PLAYER:  return "🎼";
            case ChannelInstrumentType::OSC_3X:            return "🎛️";
            case ChannelInstrumentType::AUDIO_SAMPLE:      return "🥁";
            case ChannelInstrumentType::VST3_CLAP_PLUGIN:  return "🔌";
            default: return "🎛️";
        }
    }

    inline const char* GetChannelInstrumentName(int ch_idx) {
        if (ch_idx < 0 || ch_idx >= MAX_TRACKS) return "Sampler";
        switch (g_channel_slots[ch_idx].type) {
            case ChannelInstrumentType::KURO_RHYTHM_BASS:
                if (g_channel_slots[ch_idx].kuro_layer_tab == 1) return "Kuro Kick Drum";
                if (g_channel_slots[ch_idx].kuro_layer_tab == 2) return "Kuro Rolling Bass";
                if (g_channel_slots[ch_idx].kuro_layer_tab == 3) return "Kuro Snare/Clap";
                return "Kuro Rhythm & Bass";
            case ChannelInstrumentType::FL_FLEX_SYNTH:     return "FL Flex Synth";
            case ChannelInstrumentType::DIRECTWAVE:        return "FL DirectWave";
            case ChannelInstrumentType::SYTRUS_FM:         return "Sytrus FM Synth";
            case ChannelInstrumentType::HARMOR_ADDITIVE:   return "Harmor Resynthesis";
            case ChannelInstrumentType::FRUITY_GRANULIZER: return "Fruity Granulizer";
            case ChannelInstrumentType::SLICEX:            return "SliceX Beat Slicer";
            case ChannelInstrumentType::ALIEN_LED:         return "Alien LED Synth";
            case ChannelInstrumentType::DELAY_LAMA:        return "Delay Lama Monk";
            case ChannelInstrumentType::CONTRABASS:        return "Contrabaixo Acústico";
            case ChannelInstrumentType::SOUNDFONT_PLAYER:  return "Soundfont Player";
            case ChannelInstrumentType::OSC_3X:            return "3x Osc Synth";
            case ChannelInstrumentType::AUDIO_SAMPLE:      return (!g_channel_slots[ch_idx].sample_name.empty()) ? g_channel_slots[ch_idx].sample_name.c_str() : "Audio Sampler";
            case ChannelInstrumentType::VST3_CLAP_PLUGIN:  return "Plugin VST3/CLAP";
            default: return "Instrument";
        }
    }

    // Sinal de requisição de abertura de interface de instrumento com 1 clique
    inline int g_request_open_instrument_ch = -1;
    inline int g_force_open_popup_ch = -1;
    inline void TriggerOpenInstrument(int ch_idx) {
        g_request_open_instrument_ch = ch_idx;
    }

    // Modal de renomeação rápida
    inline int g_rename_track_target = -1;
    inline char g_rename_track_buf[64] = "";

    inline void StartTrackRename(int ch_idx) {
        g_rename_track_target = ch_idx;
        const char* cur_n = (!::track_names[ch_idx].empty()) ? ::track_names[ch_idx].c_str() : ("Track " + std::to_string(ch_idx + 1)).c_str();
        strncpy(g_rename_track_buf, cur_n, sizeof(g_rename_track_buf) - 1);
        g_rename_track_buf[sizeof(g_rename_track_buf) - 1] = '\0';
    }

    // Renderizador do Menu Compartilhado de 4 Pontinhos (Channel Rack e Piano Roll)
    inline void RenderChannelInstrumentMenu(int ch_idx, ClipManager& clip_manager, float snap_step, const int* track_pitches) {
        if (ch_idx < 0 || ch_idx >= MAX_TRACKS) return;

        const char* ch_name = (!::track_names[ch_idx].empty()) ? ::track_names[ch_idx].c_str() : ("Track " + std::to_string(ch_idx + 1)).c_str();

        ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "🎛️ %s %s [%s]", GetChannelIcon(ch_idx), ch_name, GetChannelInstrumentName(ch_idx));
        ImGui::Separator();

        // 1. AÇÃO RÁPIDA: ABRIR INTERFACE COM 1 CLIQUE
        if (ImGui::MenuItem("🚀 Abrir Interface Gráfica (Editor)")) {
            selected_track_idx = ch_idx;
            TriggerOpenInstrument(ch_idx);
        }
        ImGui::Separator();

        // 2. SUBMENU: SINTETIZADORES & ENGINES
        if (ImGui::BeginMenu("🎹 Substituir por Sintetizador / Engine")) {
            if (ImGui::MenuItem("🛸 Kuro Rhythm & Bass (Full Workstation)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::KURO_RHYTHM_BASS;
                g_channel_slots[ch_idx].kuro_layer_tab = 0;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("🥊 Kuro Kick Drum Engine")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::KURO_RHYTHM_BASS;
                g_channel_slots[ch_idx].kuro_layer_tab = 1;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("⚡ Kuro Rolling Bass Engine")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::KURO_RHYTHM_BASS;
                g_channel_slots[ch_idx].kuro_layer_tab = 2;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("💥 Kuro Snare / Clap Engine")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::KURO_RHYTHM_BASS;
                g_channel_slots[ch_idx].kuro_layer_tab = 3;
                TriggerOpenInstrument(ch_idx);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("🎹 FL Flex Synth (Psytrance Leads & Arps)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::FL_FLEX_SYNTH;
                g_piano_synth.flex_active[ch_idx] = true;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("🌊 FL DirectWave (Multi-Sampler)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::DIRECTWAVE;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("📻 Sytrus (6-Op FM Synth)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::SYTRUS_FM;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("🔮 Harmor (Additive Resynthesis)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::HARMOR_ADDITIVE;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("🎛️ 3x Osc (Triple Oscillator)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::OSC_3X;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("☁️ Fruity Granulizer (Grain Cloud)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::FRUITY_GRANULIZER;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("✂️ SliceX (Beat Slicer)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::SLICEX;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("👾 Alien LED Synth (Sci-Fi)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::ALIEN_LED;
                g_piano_synth.flex_channel_instrument[ch_idx] = KuroAudio::MidiInstrument::ALIEN_LED_SYNTH;
                g_piano_synth.triggerNote(60, 0.5f, 0.85f, ch_idx);
            }
            if (ImGui::MenuItem("🧘 Delay Lama (Monge 3D)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::DELAY_LAMA;
                g_piano_synth.flex_channel_instrument[ch_idx] = KuroAudio::MidiInstrument::DELAY_LAMA;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("🎻 Contrabaixo Acústico Real")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::CONTRABASS;
                TriggerOpenInstrument(ch_idx);
            }
            if (ImGui::MenuItem("🎼 Soundfont Player (.SF2)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::SOUNDFONT_PLAYER;
                TriggerOpenInstrument(ch_idx);
            }
            ImGui::EndMenu();
        }

        // 3. SUBMENU: CARREGAR SAMPLES DE ÁUDIO (KICK, BASS, SNARE, ETC.)
        if (ImGui::BeginMenu("🥁 Carregar Amostra / Bateria (Sample)")) {
            auto load_bank_sample = [&](const std::string& display_name, const std::string& rel_path, int audition_note) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = display_name;
                std::vector<std::string> paths = {
                    "assets/samples/Electronic_Soundbanks/" + rel_path,
                    "assets\\samples\\Electronic_Soundbanks\\" + rel_path,
                    "assets/samples/" + rel_path,
                    "assets\\samples\\" + rel_path,
                    "assets/samples/Sonicspore_PRYZMA/" + rel_path,
                    "assets\\samples\\Sonicspore_PRYZMA\\" + rel_path
                };
                {
                    std::lock_guard<std::mutex> lock(g_piano_synth.getMutex());
                    for (const auto& p : paths) {
                        if (std::filesystem::exists(p)) {
                            g_piano_synth.getDrumSample(ch_idx).load(p);
                            if (g_piano_synth.getDrumSample(ch_idx).loaded) break;
                        }
                    }
                }
                g_piano_synth.triggerNote(audition_note, 0.35f, 0.95f, ch_idx);
            };

            // 🌀 SONICSPORE - PRYZMA PSYTRANCE PACK (239 SAMPLES)
            if (ImGui::BeginMenu("🌀 Sonicspore PRYZMA (FREE Psytrance Pack)")) {
                if (ImGui::BeginMenu("🥊 Kicks (Punch & Click)")) {
                    if (ImGui::MenuItem("🥊 Kick 01 (144 BPM)")) load_bank_sample("PRYZMA Kick 01", "01_Kick/Sonicspore _ FPS _ Kick 01 - 144 BPM.wav", 36);
                    if (ImGui::MenuItem("🥊 Kick 02 (146 BPM)")) load_bank_sample("PRYZMA Kick 02", "01_Kick/Sonicspore _ FPS _ Kick 02 - 146 BPM.wav", 36);
                    if (ImGui::MenuItem("🥊 Kick 03 (148 BPM)")) load_bank_sample("PRYZMA Kick 03", "01_Kick/Sonicspore _ FPS _ Kick 03 - 148 BPM.wav", 36);
                    if (ImGui::MenuItem("🥊 Kick 04 (144 BPM)")) load_bank_sample("PRYZMA Kick 04", "01_Kick/Sonicspore _ FPS _ Kick 04 - 144 BPM.wav", 36);
                    if (ImGui::MenuItem("🥊 Kick 05 (151 BPM)")) load_bank_sample("PRYZMA Kick 05", "01_Kick/Sonicspore _ FPS _ Kick 05 - 151 BPM-1.wav", 36);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("🎸 Basses (Rolling & Offbeat)")) {
                    if (ImGui::MenuItem("🎸 Bass 01 (G - 144 BPM)")) load_bank_sample("PRYZMA Bass G", "02_Bass/Sonicspore - FPS _ Bass 01_ G - 144 BPM.wav", 43);
                    if (ImGui::MenuItem("🎸 Bass 02 (G# - 146 BPM)")) load_bank_sample("PRYZMA Bass G#", "02_Bass/Sonicspore _ FPS _ Bass 02_G# - 146 BPM.wav", 44);
                    if (ImGui::MenuItem("🎸 Bass 03 (A# - 148 BPM)")) load_bank_sample("PRYZMA Bass A#", "02_Bass/Sonicspore _ FPS _ Bass 03_A# - 148 BPM.wav", 46);
                    if (ImGui::MenuItem("🎸 Bass 04 (F - 144 BPM)")) load_bank_sample("PRYZMA Bass F", "02_Bass/Sonicspore _ FPS _ Bass 04_F - 144 BPM.wav", 41);
                    if (ImGui::MenuItem("🎸 Bass 05 (C - 151 BPM)")) load_bank_sample("PRYZMA Bass C", "02_Bass/Sonicspore _ FPS _ Bass 05_C - 151 BPM-1.wav", 36);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("💥 Snares (Crisp & Transient)")) {
                    if (ImGui::MenuItem("💥 Snare 01")) load_bank_sample("PRYZMA Snare 01", "03_Snare/Sonicspore FPS _ Snare 01 .wav", 38);
                    if (ImGui::MenuItem("💥 Snare 02")) load_bank_sample("PRYZMA Snare 02", "03_Snare/Sonicspore FPS _ Snare 02 .wav", 38);
                    if (ImGui::MenuItem("💥 Snare 03")) load_bank_sample("PRYZMA Snare 03", "03_Snare/Sonicspore FPS _ Snare 03 .wav", 38);
                    if (ImGui::MenuItem("💥 Snare 04")) load_bank_sample("PRYZMA Snare 04", "03_Snare/Sonicspore FPS _ Snare 04 .wav", 38);
                    if (ImGui::MenuItem("💥 Snare 05")) load_bank_sample("PRYZMA Snare 05", "03_Snare/Sonicspore FPS _ Snare 05 .wav", 38);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("⚡ Synths & Leads")) {
                    if (ImGui::MenuItem("⚡ Synth 01")) load_bank_sample("PRYZMA Synth 01", "08_Synths/Sonicspore FPS _ Synth 01.wav", 60);
                    if (ImGui::MenuItem("⚡ Synth 02")) load_bank_sample("PRYZMA Synth 02", "08_Synths/Sonicspore FPS _ Synth 02.wav", 60);
                    if (ImGui::MenuItem("⚡ Lead 01")) load_bank_sample("PRYZMA Lead 01", "07_Leads/Sonicspore FPS _ Lead 01 _ 146 BPM.wav", 60);
                    if (ImGui::MenuItem("⚡ Lead 02")) load_bank_sample("PRYZMA Lead 02", "07_Leads/Sonicspore FPS _ Lead 02 _ 147 BPM.wav", 60);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("🌌 SFX & Glitches")) {
                    if (ImGui::MenuItem("🌌 SFX 01 (145 BPM)")) load_bank_sample("PRYZMA SFX 01", "05_Sfx/Sonicspore FPS _ SFX 01 _ 145 BPM.wav", 60);
                    if (ImGui::MenuItem("🌌 SFX 02 (144 BPM)")) load_bank_sample("PRYZMA SFX 02", "05_Sfx/Sonicspore FPS _ SFX 02 _ 144 BPM.wav", 60);
                    if (ImGui::MenuItem("🌌 SFX 03 (145 BPM)")) load_bank_sample("PRYZMA SFX 03", "05_Sfx/Sonicspore FPS _ SFX 03 _ 145 BPM.wav", 60);
                    if (ImGui::MenuItem("🌌 SFX 04 (145 BPM)")) load_bank_sample("PRYZMA SFX 04", "05_Sfx/Sonicspore FPS _ SFX 04 _ 145 BPM.wav", 60);
                    if (ImGui::MenuItem("🌌 SFX 05 (147 BPM)")) load_bank_sample("PRYZMA SFX 05", "05_Sfx/Sonicspore FPS _ SFX 05 _ 147 BPM.wav", 60);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();

            // 🌀 PSYTRANCE SOUNDBANK
            if (ImGui::BeginMenu("🌀 Psytrance Soundbank (138-145 BPM)")) {
                if (ImGui::MenuItem("🥊 Psy Punch Kick 140BPM")) load_bank_sample("Psy Punch Kick 140", "01_Psytrance/01_Kicks/Psy_Punch_Kick_140BPM.wav", 36);
                if (ImGui::MenuItem("🥊 HiTech Laser Kick 145BPM")) load_bank_sample("HiTech Laser Kick", "01_Psytrance/01_Kicks/HiTech_Laser_Kick_145BPM.wav", 36);
                if (ImGui::MenuItem("🥊 Deep Psy Sub Kick")) load_bank_sample("Deep Psy Sub Kick", "01_Psytrance/01_Kicks/Deep_Psy_Sub_Kick.wav", 36);
                if (ImGui::MenuItem("🥊 FullOn Tok Kick")) load_bank_sample("FullOn Tok Kick", "01_Psytrance/01_Kicks/FullOn_Tok_Kick.wav", 36);
                ImGui::Separator();
                if (ImGui::MenuItem("🎸 Rolling Psy Bass C1")) load_bank_sample("Rolling Psy Bass C1", "01_Psytrance/02_Rolling_Basses/Rolling_Psy_Bass_C1.wav", 36);
                if (ImGui::MenuItem("🎸 Rolling Psy Bass D#1")) load_bank_sample("Rolling Psy Bass D#1", "01_Psytrance/02_Rolling_Basses/Rolling_Psy_Bass_Ds1.wav", 39);
                if (ImGui::MenuItem("🎸 Offbeat Psy Bass F1")) load_bank_sample("Offbeat Psy Bass F1", "01_Psytrance/02_Rolling_Basses/Offbeat_Psy_Bass_F1.wav", 41);
                if (ImGui::MenuItem("💣 Sub Sine Pure Psy 42Hz")) load_bank_sample("Psy Sub Sine 42Hz", "01_Psytrance/02_Rolling_Basses/Sub_Sine_Pure_Psy_42Hz.wav", 36);
                if (ImGui::MenuItem("🛸 Reverse Psy Suck Bass")) load_bank_sample("Reverse Psy Bass", "01_Psytrance/02_Rolling_Basses/Reverse_Psy_Suck_Bass.wav", 36);
                ImGui::Separator();
                if (ImGui::MenuItem("⚡ Goa Squawk Lead Hit")) load_bank_sample("Goa Squawk Lead", "01_Psytrance/03_Leads_and_Arps/Goa_Squawk_Lead_Hit.wav", 60);
                if (ImGui::MenuItem("⚡ Psy Saw Arp Stab C5")) load_bank_sample("Psy Saw Arp Stab", "01_Psytrance/03_Leads_and_Arps/Psy_Saw_Arp_Stab_C5.wav", 72);
                if (ImGui::MenuItem("⚡ Cosmic Alien Lead Pluck")) load_bank_sample("Cosmic Alien Pluck", "01_Psytrance/03_Leads_and_Arps/Cosmic_Alien_Lead_Pluck.wav", 67);
                ImGui::Separator();
                if (ImGui::MenuItem("💥 Psy Snare Snap")) load_bank_sample("Psy Snare Snap", "01_Psytrance/04_Percussion_and_Hats/Psy_Snare_Snap.wav", 38);
                if (ImGui::MenuItem("💥 Psy Tight Clap")) load_bank_sample("Psy Tight Clap", "01_Psytrance/04_Percussion_and_Hats/Psy_Tight_Clap.wav", 39);
                if (ImGui::MenuItem("🎩 Psy Closed Hat 16th")) load_bank_sample("Psy Closed Hat", "01_Psytrance/04_Percussion_and_Hats/Psy_Closed_Hat_16th.wav", 42);
                if (ImGui::MenuItem("🎩 Psy Open Hat Bright")) load_bank_sample("Psy Open Hat", "01_Psytrance/04_Percussion_and_Hats/Psy_Open_Hat_Bright.wav", 46);
                if (ImGui::MenuItem("🪘 Tribal Tom Hit Psy")) load_bank_sample("Tribal Tom Hit", "01_Psytrance/04_Percussion_and_Hats/Tribal_Tom_Hit_Psy.wav", 45);
                ImGui::Separator();
                if (ImGui::MenuItem("🛸 Psy Laser Zap FX")) load_bank_sample("Psy Laser Zap", "01_Psytrance/05_FX_and_Vocals/Psy_Laser_Zap_FX.wav", 60);
                if (ImGui::MenuItem("🌌 Cosmic Downlifter FX")) load_bank_sample("Cosmic Downlifter", "01_Psytrance/05_FX_and_Vocals/Cosmic_Downlifter_FX.wav", 60);
                if (ImGui::MenuItem("🧘 Shamanic Mantra Chant")) load_bank_sample("Shamanic Mantra", "01_Psytrance/05_FX_and_Vocals/Shamanic_Mantra_Chant.wav", 55);
                ImGui::EndMenu();
            }

            // 🏭 PEAK-TIME & RAW TECHNO
            if (ImGui::BeginMenu("🏭 Peak-Time & Raw Techno (130-136 BPM)")) {
                if (ImGui::MenuItem("🥊 PeakTime Rumble Kick 132BPM")) load_bank_sample("PeakTime Rumble Kick", "02_Peak_Time_Techno/01_Rumble_Kicks/PeakTime_Rumble_Kick_132BPM.wav", 36);
                if (ImGui::MenuItem("🥊 Industrial Distorted Kick")) load_bank_sample("Industrial Dist Kick", "02_Peak_Time_Techno/01_Rumble_Kicks/Industrial_Distorted_Kick.wav", 36);
                if (ImGui::MenuItem("🥊 Berlin Raw 909 Kick")) load_bank_sample("Berlin Raw Kick", "02_Peak_Time_Techno/01_Rumble_Kicks/Berlin_Raw_Kick.wav", 36);
                if (ImGui::MenuItem("🥊 Dark Sub Impact Kick")) load_bank_sample("Dark Sub Impact Kick", "02_Peak_Time_Techno/01_Rumble_Kicks/Dark_Sub_Impact_Kick.wav", 36);
                ImGui::Separator();
                if (ImGui::MenuItem("🎸 Industrial Monolith Bass C1")) load_bank_sample("Monolith Bass C1", "02_Peak_Time_Techno/02_Industrial_Basses/Industrial_Monolith_Bass_C1.wav", 36);
                if (ImGui::MenuItem("🎸 Techno Rumble Sub Tail")) load_bank_sample("Techno Rumble Tail", "02_Peak_Time_Techno/02_Industrial_Basses/Techno_Rumble_Sub_Tail.wav", 36);
                if (ImGui::MenuItem("🎸 Dark Modular Bass Stab")) load_bank_sample("Modular Bass Stab", "02_Peak_Time_Techno/02_Industrial_Basses/Dark_Modular_Bass_Stab.wav", 36);
                ImGui::Separator();
                if (ImGui::MenuItem("⚡ Dark Techno Minor Chord Stab")) load_bank_sample("Techno Minor Stab", "02_Peak_Time_Techno/03_Stabs_and_Synths/Dark_Techno_Minor_Chord_Stab.wav", 60);
                if (ImGui::MenuItem("⚡ Rave Hoover Stab")) load_bank_sample("Rave Hoover Stab", "02_Peak_Time_Techno/03_Stabs_and_Synths/Rave_Hoover_Stab.wav", 57);
                if (ImGui::MenuItem("⚡ Minimal Techno Beep Pluck")) load_bank_sample("Minimal Beep Pluck", "02_Peak_Time_Techno/03_Stabs_and_Synths/Minimal_Techno_Beep_Pluck.wav", 84);
                ImGui::Separator();
                if (ImGui::MenuItem("💥 Warehouse Reverb Clap")) load_bank_sample("Warehouse Clap", "02_Peak_Time_Techno/04_Percussion_and_Rides/Warehouse_Reverb_Clap.wav", 39);
                if (ImGui::MenuItem("💥 Industrial Metallic Snare")) load_bank_sample("Metallic Snare", "02_Peak_Time_Techno/04_Percussion_and_Rides/Industrial_Metallic_Snare.wav", 38);
                if (ImGui::MenuItem("🎩 Techno 909 Ride Cymbal")) load_bank_sample("909 Ride Cymbal", "02_Peak_Time_Techno/04_Percussion_and_Rides/Techno_Ride_Cymbal_909.wav", 51);
                if (ImGui::MenuItem("🎩 Raw Closed Hat")) load_bank_sample("Raw Closed Hat", "02_Peak_Time_Techno/04_Percussion_and_Rides/Raw_Closed_Hat.wav", 42);
                ImGui::Separator();
                if (ImGui::MenuItem("💨 Factory Steam Exhaust FX")) load_bank_sample("Steam Exhaust FX", "02_Peak_Time_Techno/05_Drones_and_FX/Factory_Steam_Exhaust_FX.wav", 60);
                if (ImGui::MenuItem("💣 Sub Boom Drop FX")) load_bank_sample("Sub Boom Drop", "02_Peak_Time_Techno/05_Drones_and_FX/Sub_Boom_Drop_FX.wav", 36);
                ImGui::EndMenu();
            }

            // 🧪 ACID TECHNO 303
            if (ImGui::BeginMenu("🧪 Acid Techno 303")) {
                if (ImGui::MenuItem("⚡ TB-303 Resonant Saw C1")) load_bank_sample("TB303 Resonant Saw", "03_Acid_Techno_303/01_Acid_303_Bass_and_Squelch/TB303_Resonant_Saw_C1.wav", 36);
                if (ImGui::MenuItem("⚡ TB-303 Acid Squelch Stab")) load_bank_sample("TB303 Acid Squelch", "03_Acid_Techno_303/01_Acid_303_Bass_and_Squelch/TB303_Acid_Squelch_Stab.wav", 43);
                if (ImGui::MenuItem("⚡ TB-303 Distorted Slide Note")) load_bank_sample("TB303 Dist Slide", "03_Acid_Techno_303/01_Acid_303_Bass_and_Squelch/TB303_Distorted_Slide_Note.wav", 36);
                if (ImGui::MenuItem("⚡ TB-303 Square Acid Hit")) load_bank_sample("TB303 Square Acid", "03_Acid_Techno_303/01_Acid_303_Bass_and_Squelch/TB303_Square_Acid_Hit.wav", 36);
                ImGui::Separator();
                if (ImGui::MenuItem("🥊 Acid Hard Click Kick")) load_bank_sample("Acid Hard Kick", "03_Acid_Techno_303/02_Hard_Kicks/Acid_Hard_Click_Kick.wav", 36);
                if (ImGui::MenuItem("🥊 Acid Distorted Punch Kick")) load_bank_sample("Acid Dist Punch", "03_Acid_Techno_303/02_Hard_Kicks/Acid_Distorted_Punch_Kick.wav", 36);
                ImGui::Separator();
                if (ImGui::MenuItem("💥 Acid Saturated Clap")) load_bank_sample("Acid Sat Clap", "03_Acid_Techno_303/03_Aggressive_Percussion/Acid_Saturated_Clap.wav", 39);
                if (ImGui::MenuItem("💥 Acid Snare Roll Hit")) load_bank_sample("Acid Snare Hit", "03_Acid_Techno_303/03_Aggressive_Percussion/Acid_Snare_Roll_Hit.wav", 38);
                if (ImGui::MenuItem("🎩 Acid 909 Open Hat")) load_bank_sample("Acid 909 Open Hat", "03_Acid_Techno_303/03_Aggressive_Percussion/Acid_909_Open_Hat.wav", 46);
                ImGui::Separator();
                if (ImGui::MenuItem("🔊 Acid Filter Scream FX")) load_bank_sample("Acid Scream FX", "03_Acid_Techno_303/04_Resonant_Sweeps/Acid_Resonance_Filter_Scream_FX.wav", 60);
                ImGui::EndMenu();
            }

            // 🌌 MELODIC TECHNO & PROGRESSIVE
            if (ImGui::BeginMenu("🌌 Melodic Techno & Progressive")) {
                if (ImGui::MenuItem("🥊 Melodic Deep Warm Kick")) load_bank_sample("Melodic Deep Kick", "04_Melodic_Techno/01_Deep_Kicks/Melodic_Deep_Warm_Kick.wav", 36);
                if (ImGui::MenuItem("🥊 Melodic Punch Kick")) load_bank_sample("Melodic Punch Kick", "04_Melodic_Techno/01_Deep_Kicks/Melodic_Punch_Kick.wav", 36);
                ImGui::Separator();
                if (ImGui::MenuItem("🎸 Moog Analog Pluck Bass C1")) load_bank_sample("Moog Pluck Bass C1", "04_Melodic_Techno/02_Pluck_and_Moog_Basses/Moog_Analog_Pluck_Bass_C1.wav", 36);
                if (ImGui::MenuItem("🎸 Rolling Progressive Sub D1")) load_bank_sample("Progressive Sub D1", "04_Melodic_Techno/02_Pluck_and_Moog_Basses/Rolling_Progressive_Sub_D1.wav", 38);
                ImGui::Separator();
                if (ImGui::MenuItem("⚡ Afterlife Saw Pluck Lead")) load_bank_sample("Afterlife Saw Pluck", "04_Melodic_Techno/03_Analog_Chords_and_Leads/Afterlife_Saw_Pluck_Lead.wav", 57);
                if (ImGui::MenuItem("⚡ Lush Analog Pad Chord Hit")) load_bank_sample("Lush Pad Chord", "04_Melodic_Techno/03_Analog_Chords_and_Leads/Lush_Analog_Pad_Chord_Hit.wav", 57);
                ImGui::Separator();
                if (ImGui::MenuItem("🪵 Organic Wood Click")) load_bank_sample("Wood Click", "04_Melodic_Techno/04_Organic_Percussion/Organic_Wood_Click.wav", 76);
                if (ImGui::MenuItem("🪘 Smooth Progressive Shaker")) load_bank_sample("Smooth Shaker", "04_Melodic_Techno/04_Organic_Percussion/Smooth_Progressive_Shaker.wav", 69);
                ImGui::EndMenu();
            }

            // 🕺 TECH HOUSE & CLUB
            if (ImGui::BeginMenu("🕺 Tech House & Club Grooves")) {
                if (ImGui::MenuItem("🥊 Tech House Slap Punch Kick")) load_bank_sample("TechHouse Slap Kick", "05_Tech_House/01_Slap_Kicks/TechHouse_Slap_Punch_Kick.wav", 36);
                if (ImGui::MenuItem("🎸 FM Slap Donk Bass C1")) load_bank_sample("FM Donk Bass C1", "05_Tech_House/02_FM_and_Donk_Basses/FM_Slap_Donk_Bass_C1.wav", 36);
                if (ImGui::MenuItem("🎸 Deep House Organ Bass")) load_bank_sample("Deep Organ Bass", "05_Tech_House/02_FM_and_Donk_Basses/Deep_House_Organ_Bass.wav", 36);
                if (ImGui::MenuItem("💥 Crisp Layered House Clap")) load_bank_sample("House Layered Clap", "05_Tech_House/03_House_Claps_and_Perc/Crisp_Layered_House_Clap.wav", 39);
                if (ImGui::MenuItem("🗣️ House Vocal Chop 'Yeah!'")) load_bank_sample("Vocal Chop Yeah", "05_Tech_House/04_Vocal_Chops_and_Shakers/House_Vocal_Chop_Yeah.wav", 60);
                ImGui::EndMenu();
            }

            // ⚡ HARDSTYLE & RAWSTYLE
            if (ImGui::BeginMenu("⚡ Hardstyle & Rawstyle")) {
                if (ImGui::MenuItem("🥊 Rawstyle Distorted Kick")) load_bank_sample("Raw Dist Kick", "06_Hardstyle_and_Raw/01_Distorted_Kicks/Rawstyle_Distorted_Screaming_Kick.wav", 36);
                if (ImGui::MenuItem("🥊 Reverse Bass Punch 150BPM")) load_bank_sample("Reverse Bass 150", "06_Hardstyle_and_Raw/02_Reverse_Basses/Reverse_Bass_Punch_150BPM.wav", 36);
                if (ImGui::MenuItem("🔊 Hardstyle Screech Hit")) load_bank_sample("Hard Screech Hit", "06_Hardstyle_and_Raw/03_Screeches_and_FX/Hardstyle_Screech_Hit.wav", 72);
                ImGui::EndMenu();
            }

            // 🦾 CYBERPUNK & DARKSYNTH
            if (ImGui::BeginMenu("🦾 Cyberpunk & Darksynth")) {
                if (ImGui::MenuItem("🥊 Cyber Sub Impact Kick")) load_bank_sample("Cyber Sub Kick", "07_Cyberpunk_and_Darksynth/01_Cyber_Kicks/Cyber_Sub_Impact_Kick.wav", 36);
                if (ImGui::MenuItem("🎸 Cyber Reese Distortion Bass C1")) load_bank_sample("Cyber Reese Bass", "07_Cyberpunk_and_Darksynth/02_Reese_Basses/Cyber_Reese_Distortion_Bass_C1.wav", 36);
                if (ImGui::MenuItem("⚡ Neon Darksynth Brass Hit")) load_bank_sample("Neon Darksynth Brass", "07_Cyberpunk_and_Darksynth/03_Neon_Stabs_and_Swells/Neon_Darksynth_Brass_Hit.wav", 48);
                ImGui::EndMenu();
            }

            ImGui::Separator();

            // KICKS TRADICIONAIS
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "── KICKS (BUMBOS) ──");
            if (ImGui::MenuItem("🥊 808 Kick Drum")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "808 Kick";
                g_piano_synth.selected_variant[0] = 0;
                g_piano_synth.triggerNote(36, 0.25f, 0.95f, ch_idx);
            }
            if (ImGui::MenuItem("🥊 909 Kick Drum")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "909 Kick";
                g_piano_synth.selected_variant[0] = 1;
                g_piano_synth.triggerNote(36, 0.25f, 0.95f, ch_idx);
            }
            if (ImGui::MenuItem("🥊 Psy Punch Kick (Hard Hit)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "Psy Punch Kick";
                g_piano_synth.selected_variant[0] = 3;
                g_piano_synth.triggerNote(36, 0.25f, 0.95f, ch_idx);
            }
            if (ImGui::MenuItem("🥊 Acoustic / Tok Kick")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "Acoustic Kick";
                g_piano_synth.selected_variant[0] = 2;
                g_piano_synth.triggerNote(36, 0.25f, 0.95f, ch_idx);
            }
            ImGui::Separator();

            // BASS SAMPLES
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "── SAMPLES DE BAIXO (BASS) ──");
            if (ImGui::MenuItem("🎸 Psytrance Rolling Sub (Sample C1)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "Psy Rolling Sub";
                g_piano_synth.triggerNote(36, 0.35f, 0.95f, ch_idx);
            }
            if (ImGui::MenuItem("💣 808 Deep Sub Bass (Sample C1)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "808 Deep Sub";
                g_piano_synth.triggerNote(36, 0.45f, 0.95f, ch_idx);
            }
            if (ImGui::MenuItem("⚡ Acid Saw Bass (Sample C1)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "Acid Saw Bass";
                g_piano_synth.triggerNote(36, 0.35f, 0.90f, ch_idx);
            }
            if (ImGui::MenuItem("🛸 Alien Reese Bass (Sample C1)")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "Reese Bass";
                g_piano_synth.triggerNote(36, 0.50f, 0.90f, ch_idx);
            }
            ImGui::Separator();

            // SNARES & CLAPS
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.8f, 1.0f), "── CAIXAS & CLAPS (SNARES) ──");
            if (ImGui::MenuItem("💥 808 Snare Drum")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "808 Snare";
                g_piano_synth.selected_variant[1] = 0;
                g_piano_synth.triggerNote(38, 0.25f, 0.90f, ch_idx);
            }
            if (ImGui::MenuItem("💥 909 Snare Drum")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "909 Snare";
                g_piano_synth.selected_variant[1] = 1;
                g_piano_synth.triggerNote(38, 0.25f, 0.90f, ch_idx);
            }
            if (ImGui::MenuItem("💥 Tight Psy Clap")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "Psy Clap";
                g_piano_synth.selected_variant[1] = 0;
                g_piano_synth.triggerNote(39, 0.25f, 0.90f, ch_idx);
            }
            if (ImGui::MenuItem("💥 FPC Attack Snare")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "FPC Snare";
                g_piano_synth.selected_variant[1] = 2;
                g_piano_synth.triggerNote(38, 0.25f, 0.90f, ch_idx);
            }
            ImGui::Separator();

            // HI-HATS & PERCS
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "── PRATOS & PERCUSSÃO ──");
            if (ImGui::MenuItem("🎩 808 Closed Hat")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "808 Closed Hat";
                g_piano_synth.selected_variant[2] = 0;
                g_piano_synth.triggerNote(42, 0.20f, 0.85f, ch_idx);
            }
            if (ImGui::MenuItem("🎩 909 Open Hat")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "909 Open Hat";
                g_piano_synth.selected_variant[2] = 1;
                g_piano_synth.triggerNote(46, 0.35f, 0.85f, ch_idx);
            }
            if (ImGui::MenuItem("🪘 Tribal Wood Percussion")) {
                g_channel_slots[ch_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                g_channel_slots[ch_idx].sample_name = "Tribal Perc";
                g_piano_synth.triggerNote(65, 0.25f, 0.85f, ch_idx);
            }
            ImGui::Separator();

            // ABRIR EDITOR COMPLETO DO SAMPLE
            if (ImGui::MenuItem("🎛️ Abrir Editor Completo do Sample (ADSR, Pitch & Filtro)...")) {
                selected_track_idx = ch_idx;
                active_sampler_channel = ch_idx;
                show_sampler_settings = true;
            }
            ImGui::EndMenu();
        }

        // 4. PLUGINS EXTERNOS VST3 / CLAP
        if (ImGui::MenuItem("🔌 Gerenciador de Plugins VST3 / CLAP...")) {
            selected_track_idx = ch_idx;
            g_channel_slots[ch_idx].type = ChannelInstrumentType::VST3_CLAP_PLUGIN;
            TriggerOpenInstrument(ch_idx);
        }
        ImGui::Separator();

        // 5. AÇÕES RÁPIDAS DA FAIXA
        if (ImGui::MenuItem("✏️ Renomear Faixa...")) {
            StartTrackRename(ch_idx);
        }

        if (ImGui::BeginMenu("🎨 Alterar Cor da Faixa")) {
            if (ImGui::MenuItem("🟦 Ciano Neon")) {
                g_channel_slots[ch_idx].custom_color = IM_COL32(0, 229, 255, 255);
            }
            if (ImGui::MenuItem("🟩 Verde Neon FL")) {
                g_channel_slots[ch_idx].custom_color = IM_COL32(57, 255, 140, 255);
            }
            if (ImGui::MenuItem("🟧 Laranja / Âmbar")) {
                g_channel_slots[ch_idx].custom_color = IM_COL32(255, 140, 30, 255);
            }
            if (ImGui::MenuItem("🟪 Magenta / Roxo")) {
                g_channel_slots[ch_idx].custom_color = IM_COL32(230, 70, 200, 255);
            }
            if (ImGui::MenuItem("🟥 Vermelho Coral")) {
                g_channel_slots[ch_idx].custom_color = IM_COL32(255, 75, 75, 255);
            }
            if (ImGui::MenuItem("⚪ Padrão (Sem Cor)")) {
                g_channel_slots[ch_idx].custom_color = 0;
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("🎹 Abrir no Piano Roll (F7)")) {
            selected_track_idx = ch_idx;
            show_piano_roll = true;
        }

        ImGui::Separator();
        int cur_p = clip_manager.current_pattern_idx;
        if (cur_p >= 0 && cur_p < (int)clip_manager.global_patterns.size()) {
            auto& active_pat = clip_manager.global_patterns[cur_p];
            if (ImGui::MenuItem("⚡ Preencher a cada 2 passos (8 avos)")) {
                auto& notes = active_pat.getChannelNotes(ch_idx);
                notes.clear();
                int pitch = (track_pitches != nullptr) ? track_pitches[ch_idx % 16] : 36;
                for (int s = 0; s < 16; s += 2) {
                    notes.push_back(KuroDSP::MidiNote(pitch, s * snap_step, snap_step * 0.85f, 0.90f, 1.0f, ch_idx));
                }
            }
            if (ImGui::MenuItem("⚡ Preencher a cada 4 passos (4-on-the-Floor)")) {
                auto& notes = active_pat.getChannelNotes(ch_idx);
                notes.clear();
                int pitch = (track_pitches != nullptr) ? track_pitches[ch_idx % 16] : 36;
                for (int s = 0; s < 16; s += 4) {
                    notes.push_back(KuroDSP::MidiNote(pitch, s * snap_step, snap_step * 0.85f, 0.95f, 1.0f, ch_idx));
                }
            }
            if (ImGui::MenuItem("🧹 Limpar todos os passos")) {
                active_pat.getChannelNotes(ch_idx).clear();
            }
        }
    }

    // Modal de renomeação de faixa
    inline void RenderTrackRenameModal() {
        if (g_rename_track_target < 0 || g_rename_track_target >= MAX_TRACKS) return;

        ImGui::OpenPopup("Renomear Faixa##cr_rename_modal");
        ImGui::SetNextWindowSize(ImVec2(340, 140));
        if (ImGui::BeginPopupModal("Renomear Faixa##cr_rename_modal", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::Text("Digite o novo nome para a Faixa %d:", g_rename_track_target + 1);
            ImGui::Spacing();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
            bool enter_pressed = ImGui::InputText("##rename_input", g_rename_track_buf, sizeof(g_rename_track_buf), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Salvar", ImVec2(100, 24)) || enter_pressed) {
                if (strlen(g_rename_track_buf) > 0) {
                    ::track_names[g_rename_track_target] = g_rename_track_buf;
                    g_channel_slots[g_rename_track_target].custom_title = g_rename_track_buf;
                }
                g_rename_track_target = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancelar", ImVec2(100, 24))) {
                g_rename_track_target = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Resetar", ImVec2(80, 24))) {
                ::track_names[g_rename_track_target] = "";
                g_channel_slots[g_rename_track_target].custom_title = "";
                g_rename_track_target = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}
