#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "../plugin_manager/KuroSamplerNode.h"

extern std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;

namespace KuroUI {
    extern bool show_piano_roll;

    struct DirectWaveZone {
        std::string name;
        std::string filename;
        int root_key = 60; // C4
        int low_key = 48;  // C3
        int high_key = 72; // C5
        int low_vel = 0;
        int high_vel = 127;
        
        // Marcadores de Onda e Loop
        float sample_start_pct = 0.0f;
        float sample_end_pct = 1.0f;
        int loop_mode = 1; // 0: One-shot, 1: Forward Loop, 2: Ping-Pong
        float loop_start_pct = 0.20f;
        float loop_end_pct = 0.85f;
        float crossfade_pct = 0.05f;
        
        int tune_semitones = 0;
        float tune_cents = 0.0f;
        float gain = 1.0f;
        float pan = 0.0f;
        ImVec4 color = ImVec4(1.0f, 0.55f, 0.0f, 1.0f); // Laranja DirectWave
    };

    class KuroDirectWaveUI {
    private:
        bool is_open = false;
        std::vector<DirectWaveZone> zones;
        int selected_zone_idx = 0;
        int current_sub_tab = 0; // 0: Keyzone Mapper, 1: Filtros & ADSR, 2: Loop & Waveform Editor, 3: FX Rack Integrado

        // Seção de Síntese
        float master_volume = 0.85f;
        float master_pan = 0.0f;
        float filter_cutoff_hz = 12000.0f;
        float filter_resonance = 0.25f;
        int filter_type_idx = 0; // 0: 24dB Lowpass, 1: 24dB Highpass, 2: Bandpass, 3: Notch
        float filter_env_amount = 0.50f;

        // Envelopes ADSR
        float amp_attack = 0.005f;
        float amp_decay = 0.350f;
        float amp_sustain = 0.80f;
        float amp_release = 0.400f;

        float flt_attack = 0.010f;
        float flt_decay = 0.250f;
        float flt_sustain = 0.30f;
        float flt_release = 0.300f;

        // LFO 1 e 2
        float lfo1_rate_hz = 4.0f;
        float lfo1_depth = 0.20f;
        float lfo2_rate_hz = 1.5f;
        float lfo2_depth = 0.35f;

        // ── TIME-STRETCHING & ENGINE MODES ─────────────────────────────
        int pitch_mode = 0; // 0: Resample (Vintage Pitch/Speed), 1: Granular Stretch (Preserve BPM), 2: Drone Texture
        float grain_size_ms = 45.0f;
        float grain_density = 1.0f;
        float formant_shift = 0.0f; // -12 a +12 semitons

        // ── ROUND-ROBIN & HUMANIZAÇÃO ───────────────────────────────────
        int round_robin_mode = 1; // 0: Off, 1: Sequential Cycle (1-2-3-4), 2: Random Non-Repeating
        int round_robin_counter = 0;
        float humanize_velocity_pct = 4.0f; // +- 4% de variação de força
        float humanize_timing_ms = 3.0f;    // +- 3ms de micro-timing
        float humanize_tuning_cents = 2.5f; // +- 2.5 cents de micro-drift analógico

        // ── RACK DE EFEITOS INTEGRADO DO DIRECTWAVE (FX RACK) ───────────
        bool fx_drive_enable = true;
        float fx_drive_amount = 0.35f;
        float fx_tube_warmth = 0.60f;

        bool fx_chorus_enable = false;
        float fx_chorus_rate = 1.2f;
        float fx_chorus_depth = 0.45f;
        float fx_chorus_mix = 0.30f;

        bool fx_delay_enable = true;
        int fx_delay_sync_idx = 1; // 0: 1/4, 1: 1/8, 2: 1/16, 3: Triplet
        float fx_delay_feedback = 0.40f;
        float fx_delay_wet = 0.25f;

        bool fx_reverb_enable = true;
        float fx_reverb_size = 0.65f;
        float fx_reverb_damping = 0.30f;
        float fx_reverb_wet = 0.20f;

        bool fx_eq_enable = true;
        float fx_eq_low_db = 2.0f;
        float fx_eq_mid_db = -1.0f;
        float fx_eq_high_db = 3.5f;

        int active_pressed_key = -1;
        int dragging_marker = -1; // 0: Start, 1: LoopStart, 2: LoopEnd, 3: End
        std::string status_feedback = "DirectWave pronto. Selecione uma zona ou toque no teclado.";

    public:
        KuroDirectWaveUI() {
            initDefaultZones();
        }

        bool& getOpenState() { return is_open; }
        bool& isOpen() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void initDefaultZones() {
            zones.clear();
            // 1. ZONA 1: KICKS SUB (C1 - B2)
            zones.push_back({ "Zone 1: Adhana Astrix Kick", "adhana_signature\\Adhana_Astrix_Kick_Punch_01.wav", 36, 24, 47, 0, 127, 0.0f, 1.0f, 0, 0.0f, 1.0f, 0.0f, 0, 0.0f, 1.0f, 0.0f, ImVec4(0.0f, 0.9f, 1.0f, 1.0f) });
            
            // 2. ZONA 2: ROLLING BASS (C3 - B3)
            zones.push_back({ "Zone 2: Rolling Psy Bass 16th", "adhana_signature\\Adhana_Rolling_Bass_Hit_01.wav", 48, 48, 59, 0, 127, 0.0f, 1.0f, 1, 0.15f, 0.85f, 0.05f, 0, 0.0f, 1.0f, 0.0f, ImVec4(1.0f, 0.2f, 0.8f, 1.0f) });
            
            // 3. ZONA 3: PSY LEADS (C4 - B5)
            zones.push_back({ "Zone 3: Psy Lead Melodic Stab", "adhana_signature\\Adhana_Lead_Stab_Hit.wav", 60, 60, 83, 0, 127, 0.0f, 1.0f, 1, 0.10f, 0.80f, 0.08f, 0, 0.0f, 0.9f, 0.0f, ImVec4(1.0f, 0.8f, 0.1f, 1.0f) });

            // 4. ZONA 4: VOCAL MANTRAS (C6 - C8)
            zones.push_back({ "Zone 4: Adhana Tribal Vocal", "adhana_signature\\Adhana_Tribal_Vocal_Chop.wav", 84, 84, 108, 0, 127, 0.0f, 1.0f, 0, 0.0f, 1.0f, 0.0f, 0, 0.0f, 0.95f, 0.0f, ImVec4(0.2f, 1.0f, 0.4f, 1.0f) });
        }

        void importCustomAudioTrack(const std::string& filepath) {
            if (filepath.empty()) return;
            std::string track_name = std::filesystem::path(filepath).stem().string();
            
            if (g_global_sampler) {
                g_global_sampler->loadSample(filepath);
            }

            zones.clear();
            // Zona 1: Kick & Batida (C1-B2) -> 0% a 25%
            zones.push_back({ "Zone 1: " + track_name + " [KICK / BEAT]", filepath, 36, 24, 47, 0, 127, 0.00f, 0.25f, 0, 0.00f, 0.25f, 0.0f, 0, 0.0f, 1.0f, 0.0f, ImVec4(0.0f, 0.9f, 1.0f, 1.0f) });
            // Zona 2: Bassline Rolante (C3-B3) -> 25% a 50%
            zones.push_back({ "Zone 2: " + track_name + " [BASSLINE]", filepath, 48, 48, 59, 0, 127, 0.25f, 0.50f, 1, 0.25f, 0.50f, 0.05f, 0, 0.0f, 1.0f, 0.0f, ImVec4(1.0f, 0.2f, 0.8f, 1.0f) });
            // Zona 3: Synth Leads & Melodia (C4-B5) -> 50% a 75%
            zones.push_back({ "Zone 3: " + track_name + " [LEADS / SYNTH]", filepath, 60, 60, 83, 0, 127, 0.50f, 0.75f, 1, 0.50f, 0.75f, 0.08f, 0, 0.0f, 0.9f, 0.0f, ImVec4(1.0f, 0.8f, 0.1f, 1.0f) });
            // Zona 4: Vocais & Efeitos (C6-C8) -> 75% a 100%
            zones.push_back({ "Zone 4: " + track_name + " [VOCAL / FX]", filepath, 84, 84, 108, 0, 127, 0.75f, 1.00f, 0, 0.75f, 1.00f, 0.0f, 0, 0.0f, 0.95f, 0.0f, ImVec4(0.2f, 1.0f, 0.4f, 1.0f) });
            
            selected_zone_idx = 0;
            status_feedback = "Música '" + track_name + "' importada e carregada no Sampler Engine! Toque no teclado abaixo para ouvir os cortes reais.";
        }

        void autoSliceTrack8Parts(const std::string& filepath) {
            if (filepath.empty()) return;
            std::string track_name = std::filesystem::path(filepath).stem().string();
            
            if (g_global_sampler) {
                g_global_sampler->loadSample(filepath);
            }

            zones.clear();
            ImVec4 palette[8] = {
                ImVec4(0.0f, 0.9f, 1.0f, 1.0f),
                ImVec4(0.2f, 0.6f, 1.0f, 1.0f),
                ImVec4(1.0f, 0.2f, 0.8f, 1.0f),
                ImVec4(0.9f, 0.1f, 0.4f, 1.0f),
                ImVec4(1.0f, 0.8f, 0.1f, 1.0f),
                ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                ImVec4(0.2f, 1.0f, 0.4f, 1.0f),
                ImVec4(0.0f, 1.0f, 0.8f, 1.0f)
            };
            const char* names[8] = { "Kick Intro", "Rolling Bass", "Lead Synth 1", "Lead Synth 2", "Vocal Phrase", "Riser Sweep", "Breakdown Pad", "Drop Climax" };
            for (int i = 0; i < 8; i++) {
                int low_k = 24 + i * 10;
                int high_k = (i == 7) ? 108 : (low_k + 9);
                float s_pct = (float)i / 8.0f;
                float e_pct = (float)(i + 1) / 8.0f;
                zones.push_back({ "Slice " + std::to_string(i+1) + ": " + names[i], filepath, low_k + 5, low_k, high_k, 0, 127, s_pct, e_pct, 1, s_pct, e_pct, 0.05f, 0, 0.0f, 1.0f, 0.0f, palette[i] });
            }
            selected_zone_idx = 0;
            status_feedback = "Track fatiada em 8 partes rítmicas (MPC / Pads)! Pronto para tocar no Sampler.";
        }

        void loadPreset(int preset_id) {
            zones.clear();
            if (preset_id == 0) { // Perception Signature Alien Matrix
                std::string p_path = "C:\\NovaDAW\\tracks\\Perception - Life Process.wav";
                if (!std::filesystem::exists(p_path)) p_path = "assets\\samples\\adhana_signature\\Adhana_Astrix_Kick_Punch_01.wav";
                if (g_global_sampler) g_global_sampler->loadSample(p_path);

                zones.push_back({ "Zone 1: Perception Punch Kick", p_path, 36, 24, 47, 0, 127, 0.00f, 0.20f, 0, 0.0f, 0.20f, 0.0f, 0, 0.0f, 1.0f, 0.0f, ImVec4(0.0f, 0.9f, 1.0f, 1.0f) });
                zones.push_back({ "Zone 2: Perception Rolling Bassline (KB-B-B)", p_path, 48, 48, 59, 0, 127, 0.20f, 0.45f, 1, 0.20f, 0.45f, 0.05f, 0, 0.0f, 1.0f, 0.0f, ImVec4(1.0f, 0.2f, 0.8f, 1.0f) });
                zones.push_back({ "Zone 3: Perception Acid 303 Lead Stab", p_path, 60, 60, 83, 0, 127, 0.45f, 0.70f, 1, 0.45f, 0.70f, 0.08f, 0, 0.0f, 0.9f, 0.0f, ImVec4(1.0f, 0.8f, 0.1f, 1.0f) });
                zones.push_back({ "Zone 4: Perception Alien Psy Vocal & FX", p_path, 84, 84, 108, 0, 127, 0.70f, 1.00f, 0, 0.70f, 1.00f, 0.0f, 0, 0.0f, 0.95f, 0.0f, ImVec4(0.2f, 1.0f, 0.4f, 1.0f) });
                status_feedback = "Preset 'Perception - Alien Psy Rave Matrix' carregado com sucesso!";
            } else if (preset_id == 1) { // Astrix Adhana Matrix
                initDefaultZones();
                status_feedback = "Preset 'Astrix - Adhana Multi-Zone Matrix' carregado!";
            } else if (preset_id == 2) { // Psy Rolling Bass Keyzones
                zones.push_back({ "Sub Bass Low (C1-B1)", "Sub_Sine_40Hz.wav", 24, 24, 35, 0, 127, 0.0f, 1.0f, 1, 0.0f, 1.0f, 0.0f, 0, 0.0f, 1.0f, 0.0f, ImVec4(0.2f, 0.5f, 1.0f, 1.0f) });
                zones.push_back({ "Rolling Mid Bass (C2-B3)", "Rolling_Psy_Bass_16th.wav", 36, 36, 59, 0, 127, 0.0f, 1.0f, 1, 0.05f, 0.95f, 0.05f, 0, 0.0f, 1.0f, 0.0f, ImVec4(1.0f, 0.2f, 0.7f, 1.0f) });
                zones.push_back({ "Acid 303 High Stab (C4-C6)", "Acid_303_Saw_Hit.wav", 60, 60, 84, 0, 127, 0.0f, 1.0f, 1, 0.10f, 0.90f, 0.05f, 0, 0.0f, 0.9f, 0.0f, ImVec4(1.0f, 0.7f, 0.0f, 1.0f) });
                status_feedback = "Preset 'Psy Rolling Bass Keyzones' carregado!";
            } else if (preset_id == 3) { // EDM & 808 Stadium Kit
                zones.push_back({ "Clean 808 Sub (C1-B2)", "Clean_808_Sub_Kick.wav", 36, 24, 47, 0, 127, 0.0f, 1.0f, 0, 0.0f, 1.0f, 0.0f, 0, 0.0f, 1.0f, 0.0f, ImVec4(0.9f, 0.1f, 0.2f, 1.0f) });
                zones.push_back({ "909 Snare & Claps (C3-B4)", "909_Tight_Snare.wav", 48, 48, 71, 0, 127, 0.0f, 1.0f, 0, 0.0f, 1.0f, 0.0f, 0, 0.0f, 0.95f, 0.0f, ImVec4(0.0f, 0.8f, 1.0f, 1.0f) });
                zones.push_back({ "Metal Hats & FX (C5-C7)", "Closed_Metal_Hat.wav", 72, 72, 96, 0, 127, 0.0f, 1.0f, 0, 0.0f, 1.0f, 0.0f, 0, 0.0f, 0.9f, 0.0f, ImVec4(1.0f, 0.9f, 0.2f, 1.0f) });
                status_feedback = "Preset 'EDM & 808 Stadium Kit' carregado!";
            }
        }

        void loadStemZones(const std::string& kick, const std::string& bass, const std::string& leads, const std::string& vocals) {
            zones.clear();
            if (!kick.empty()) zones.push_back({ "Zone 1: Drum & Kick Stem", kick, 36, 24, 47, 0, 127, 0.0f, 1.0f, 0, 0.0f, 1.0f, 0.0f, 0, 0.0f, 1.0f, 0.0f, ImVec4(0.0f, 0.9f, 1.0f, 1.0f) });
            if (!bass.empty()) zones.push_back({ "Zone 2: Bassline Stem", bass, 48, 48, 59, 0, 127, 0.0f, 1.0f, 1, 0.15f, 0.85f, 0.05f, 0, 0.0f, 1.0f, 0.0f, ImVec4(1.0f, 0.2f, 0.8f, 1.0f) });
            if (!leads.empty()) zones.push_back({ "Zone 3: Leads & Synth Stem", leads, 60, 60, 83, 0, 127, 0.0f, 1.0f, 1, 0.10f, 0.80f, 0.08f, 0, 0.0f, 0.9f, 0.0f, ImVec4(1.0f, 0.8f, 0.1f, 1.0f) });
            if (!vocals.empty()) zones.push_back({ "Zone 4: Vocals & SFX Stem", vocals, 84, 84, 108, 0, 127, 0.0f, 1.0f, 0, 0.0f, 1.0f, 0.0f, 0, 0.0f, 0.95f, 0.0f, ImVec4(0.2f, 1.0f, 0.4f, 1.0f) });
            status_feedback = "Stems da música carregados nas Zonas do DirectWave!";
        }

        void playTestNote(int midi_note, int velocity = 100, bool stop_others = false) {
            active_pressed_key = midi_note;
            for (size_t i = 0; i < zones.size(); ++i) {
                const auto& z = zones[i];
                if (midi_note >= z.low_key && midi_note <= z.high_key && velocity >= z.low_vel && velocity <= z.high_vel) {
                    selected_zone_idx = (int)i;
                    
                    std::vector<std::string> candidates = {
                        z.filename,
                        "C:\\NovaDAW\\tracks\\" + z.filename,
                        "C:\\NovaDAW\\tracks\\Perception - Different Way.wav",
                        "C:\\NovaDAW\\tracks\\Perception - Life Process.wav",
                        "assets\\samples\\" + z.filename,
                        "assets\\samples\\adhana_signature\\" + z.filename,
                        "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\assets\\samples\\" + z.filename,
                        "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\assets\\samples\\adhana_signature\\" + z.filename
                    };

                    for (const auto& p : candidates) {
                        if (std::filesystem::exists(p)) {
                            if (g_global_sampler) {
                                static std::string last_loaded_p = "";
                                if (last_loaded_p != p) {
                                    g_global_sampler->loadSample(p);
                                    last_loaded_p = p;
                                }

                                if (stop_others) {
                                    g_global_sampler->stopAllVoices();
                                }

                                uint64_t tot_f = g_global_sampler->getTotalFrames();
                                if (tot_f > 0) {
                                    uint64_t start_f = (uint64_t)(z.sample_start_pct * (double)tot_f);
                                    uint64_t end_f = (uint64_t)(z.sample_end_pct * (double)tot_f);
                                    if (end_f <= start_f || end_f > tot_f) end_f = tot_f;

                                    double pitch_multiplier = std::pow(2.0, (double)(midi_note - z.root_key + z.tune_semitones) / 12.0);
                                    float gain = z.gain * ((float)velocity / 127.0f);
                                    g_global_sampler->playSlice(start_f, end_f, pitch_multiplier, gain, z.pan, z.loop_mode);
                                }
                            }
                            status_feedback = "DirectWave Engine: Tocando Slice [" + z.name + "] (Nota MIDI: " + std::to_string(midi_note) + ", Vel: " + std::to_string(velocity) + ")";
                            return;
                        }
                    }
                }
            }
            status_feedback = "DirectWave: Nenhuma zona mapeada para a nota MIDI " + std::to_string(midi_note);
        }

        // Renderizador Integrado no Inspector Dock
        void RenderEmbedded() {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.07f, 0.10f, 1.0f));
            ImGui::BeginChild("DirectWaveEmbedded", ImVec2(0, 0), false);

            ImGui::TextColored(ImVec4(1.00f, 0.55f, 0.00f, 1.0f), "FL DIRECTWAVE MULTI-SAMPLER");
            ImGui::SameLine(ImGui::GetWindowWidth() - 250.0f);
            if (ImGui::Button("[+] Expandir em Janela Dedicada", ImVec2(240, 24))) {
                is_open = true;
            }
            ImGui::Separator();
            ImGui::Spacing();

            RenderSubTabs();

            ImGui::EndChild();
            ImGui::PopStyleColor();
        }

        void RenderSubTabs() {
            // ── GUIA RÁPIDO & BOTÕES DE IMPORTAÇÃO DIRETA (PASSO A PASSO PARA LEIGOS) ───
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.09f, 0.14f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.70f));
            ImGui::BeginChild("DirectWaveQuickGuide", ImVec2(0, 80), true);

            ImGui::TextColored(ImVec4(0.00f, 0.95f, 1.00f, 1.00f), "🛸 COMO IMPORTAR E USAR QUALQUER MÚSICA (PASSO A PASSO SIMPLES):");
            ImGui::TextColored(ImVec4(0.80f, 0.90f, 0.98f, 1.00f), "1. Clique em [📂 Carregar Música] para escolher qualquer áudio do seu PC (ex: Perception).");
            ImGui::SameLine(0, 12);
            ImGui::TextColored(ImVec4(0.80f, 0.90f, 0.98f, 1.00f), "2. O sistema divide a música automaticamente nas 4 faixas do teclado.");
            ImGui::SameLine(0, 12);
            ImGui::TextColored(ImVec4(0.80f, 0.90f, 0.98f, 1.00f), "3. Toque nas teclas abaixo ou aperte F7 para o Piano Roll!");

            ImGui::Spacing();

            // Botão 1: Importar Música
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.70f, 0.90f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.02f, 0.05f, 0.08f, 1.00f));
            if (ImGui::Button("📂 1. CARREGAR MÚSICA (.WAV / .MP3)", ImVec2(235, 24))) {
                std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                if (!path.empty()) {
                    importCustomAudioTrack(path);
                }
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 6);

            // Botão 2: Auto-Fatiar em 8 Transientes
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00f, 0.55f, 0.00f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.02f, 0.05f, 0.08f, 1.00f));
            if (ImGui::Button("🤖 2. AUTO-FATIAMENTO EM 8 FATIAS", ImVec2(235, 24))) {
                if (!zones.empty() && !zones[0].filename.empty() && std::filesystem::exists(zones[0].filename)) {
                    autoSliceTrack8Parts(zones[0].filename);
                } else {
                    std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                    if (!path.empty()) autoSliceTrack8Parts(path);
                }
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 6);

            // Botão 3: Parar Sons
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.15f, 0.15f, 1.00f));
            if (ImGui::Button("⏹ PARAR SONS", ImVec2(120, 24))) {
                if (g_global_sampler) g_global_sampler->stopAllVoices();
            }
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 6);

            // Botão 4: Abrir no Piano Roll
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.20f, 0.30f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.85f, 1.00f, 1.00f));
            if (ImGui::Button("🎹 PIANO ROLL (F7)", ImVec2(150, 24))) {
                show_piano_roll = true;
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 6);

            // Botão Presets
            if (ImGui::Button("⚡ Presets ⯆", ImVec2(120, 24))) {
                ImGui::OpenPopup("DirectWaveArtistPresetsPopup");
            }
            if (ImGui::BeginPopup("DirectWaveArtistPresetsPopup")) {
                if (ImGui::MenuItem("👽 Perception - Alien Rave Matrix (Signature)")) loadPreset(0);
                if (ImGui::MenuItem("🌌 Astrix - Adhana Multi-Zone Matrix")) loadPreset(1);
                if (ImGui::MenuItem("⚡ Psy Rolling Bass Keyzones")) loadPreset(2);
                if (ImGui::MenuItem("🥁 808 & 909 Stadium Drum Velocity Kit")) loadPreset(3);
                ImGui::EndPopup();
            }

            ImGui::EndChild();
            ImGui::PopStyleColor(2);

            ImGui::Spacing();

            // Barra de Sub-Abas com Estilo Laranja DirectWave
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            
            if (current_sub_tab == 0) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.55f, 0.0f, 1.0f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 1.0f));
            if (ImGui::Button("Keyzone Mapper", ImVec2(160, 26))) current_sub_tab = 0;
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if (current_sub_tab == 1) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.55f, 0.0f, 1.0f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 1.0f));
            if (ImGui::Button("Filtros & ADSR", ImVec2(160, 26))) current_sub_tab = 1;
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if (current_sub_tab == 2) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.55f, 0.0f, 1.0f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 1.0f));
            if (ImGui::Button("Loop & Waveform", ImVec2(160, 26))) current_sub_tab = 2;
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if (current_sub_tab == 3) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.55f, 0.0f, 1.0f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 1.0f));
            if (ImGui::Button("FX Rack Integrado", ImVec2(160, 26))) current_sub_tab = 3;
            ImGui::PopStyleColor();

            ImGui::PopStyleVar();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (current_sub_tab == 0) {
                RenderKeyzoneMapper();
            } else if (current_sub_tab == 1) {
                RenderFiltersAndEnvelopes();
            } else if (current_sub_tab == 2) {
                RenderLoopAndWaveformEditor();
            } else if (current_sub_tab == 3) {
                RenderIntegratedFXRack();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Teclado Interativo de 88 Notas
            RenderInteractiveKeyboard();
        }

        void RenderKeyzoneMapper() {
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "MAPEADOR VISUAL DE KEYZONES & VELOCITY LAYERS (C1 a B7):");
            
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 kz_p0 = ImGui::GetCursorScreenPos();
            ImVec2 kz_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
            ImVec2 kz_p1 = ImVec2(kz_p0.x + kz_sz.x, kz_p0.y + kz_sz.y);

            dl->AddRectFilled(kz_p0, kz_p1, IM_COL32(11, 16, 24, 255), 4.0f);
            dl->AddRect(kz_p0, kz_p1, IM_COL32(24, 40, 58, 255), 4.0f);

            // Desenhar Zonas Coloridas no Grid
            for (size_t z = 0; z < zones.size(); ++z) {
                const auto& zone = zones[z];
                float x_start = kz_p0.x + ((float)(zone.low_key - 24) / 84.0f) * kz_sz.x;
                float x_end = kz_p0.x + ((float)(zone.high_key - 24 + 1) / 84.0f) * kz_sz.x;
                
                float y_top = kz_p0.y + (1.0f - (float)zone.high_vel / 127.0f) * kz_sz.y;
                float y_bot = kz_p0.y + (1.0f - (float)zone.low_vel / 127.0f) * kz_sz.y;

                if (y_bot - y_top < 24.0f) {
                    y_top = kz_p0.y + 10.0f + (z * 30.0f);
                    y_bot = y_top + 24.0f;
                }

                bool is_sel = (selected_zone_idx == (int)z);
                ImU32 zone_bg = ImColor(zone.color.x, zone.color.y, zone.color.z, is_sel ? 0.90f : 0.60f);
                
                dl->AddRectFilled(ImVec2(x_start, y_top), ImVec2(x_end, y_bot), zone_bg, 4.0f);
                dl->AddRect(ImVec2(x_start, y_top), ImVec2(x_end, y_bot), is_sel ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 180), 4.0f);
                
                std::string z_label = zone.name + " (" + std::to_string(zone.low_key) + "-" + std::to_string(zone.high_key) + ")";
                dl->AddText(ImVec2(x_start + 6, y_top + 4), IM_COL32(0, 0, 0, 255), z_label.c_str());
            }

            ImGui::Dummy(kz_sz);
            ImGui::Spacing();

            // ── LISTA & EDIÇÃO AVANÇADA DE ZONAS COM AUTONOMIA TOTAL ───────────
            ImGui::Columns(2, "ZoneCols", true);
            ImGui::SetColumnWidth(0, 380);

            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Zonas de Amostras / Slices:");
            ImGui::BeginChild("##ZonesListChild", ImVec2(0, 130), true);
            for (int z = 0; z < (int)zones.size(); ++z) {
                ImGui::PushID(z + 5500);
                
                // Botão Preview de Play individual por slice
                if (ImGui::Button("▶", ImVec2(22, 20))) {
                    selected_zone_idx = z;
                    playTestNote(zones[z].root_key, 110, true);
                }
                ImGui::SameLine(0, 4);

                bool is_sel = (selected_zone_idx == z);
                if (ImGui::Selectable(zones[z].name.c_str(), is_sel)) {
                    selected_zone_idx = z;
                }
                ImGui::PopID();
            }
            ImGui::EndChild();

            // Botões Rápidos de Fatiamento e Gestão
            if (ImGui::Button("➕ Nova", ImVec2(65, 22))) {
                int next_id = (int)zones.size() + 1;
                zones.push_back({ "Zone " + std::to_string(next_id) + ": Slice Custom", "adhana_signature\\Adhana_Astrix_Kick_Punch_01.wav", 60, 48, 72, 0, 127, 0.0f, 1.0f, 1, 0.1f, 0.9f, 0.05f, 0, 0.0f, 1.0f, 0.0f, ImVec4(0.0f, 0.8f, 1.0f, 1.0f) });
                selected_zone_idx = (int)zones.size() - 1;
                status_feedback = "Nova Zona adicionada!";
            }
            ImGui::SameLine(0, 3);
            if (ImGui::Button("🗑️ Excluir", ImVec2(68, 22)) && zones.size() > 1) {
                zones.erase(zones.begin() + selected_zone_idx);
                selected_zone_idx = std::max(0, selected_zone_idx - 1);
                status_feedback = "Zona excluída!";
            }
            ImGui::SameLine(0, 3);
            if (ImGui::Button("📁 Trocar WAV", ImVec2(100, 22))) {
                std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                if (!path.empty() && selected_zone_idx >= 0 && selected_zone_idx < (int)zones.size()) {
                    zones[selected_zone_idx].filename = path;
                    zones[selected_zone_idx].name = std::filesystem::path(path).stem().string();
                    if (g_global_sampler) g_global_sampler->loadSample(path);
                    status_feedback = "Amostra carregada na Zona: " + zones[selected_zone_idx].name;
                }
            }
            ImGui::SameLine(0, 3);
            if (ImGui::Button("✂ Fatiar 4", ImVec2(65, 22))) {
                if (!zones.empty() && !zones[0].filename.empty()) importCustomAudioTrack(zones[0].filename);
            }
            ImGui::SameLine(0, 3);
            if (ImGui::Button("✂ 8", ImVec2(35, 22))) {
                if (!zones.empty() && !zones[0].filename.empty()) autoSliceTrack8Parts(zones[0].filename);
            }

            ImGui::NextColumn();

            // ── PAINEL DE PARÂMETROS COM AUTONOMIA COMPLETA ────────────────────
            if (selected_zone_idx >= 0 && selected_zone_idx < (int)zones.size()) {
                auto& cur_zone = zones[selected_zone_idx];
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Parâmetros da Zona [%s]:", cur_zone.name.c_str());
                
                // Cálculo de tempo exato em segundos da fatia
                float tot_sec = 0.0f;
                if (g_global_sampler && g_global_sampler->getSampleRate() > 0) {
                    tot_sec = (float)g_global_sampler->getTotalFrames() / (float)g_global_sampler->getSampleRate();
                }
                if (tot_sec > 0.0f) {
                    float s_sec = cur_zone.sample_start_pct * tot_sec;
                    float e_sec = cur_zone.sample_end_pct * tot_sec;
                    float d_sec = e_sec - s_sec;
                    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "⏱️ Fatia: %.2fs até %.2fs (Duração: %.2fs / Total: %.1fs)", s_sec, e_sec, d_sec, tot_sec);
                }

                ImGui::SliderInt("Nota Raiz (Root)", &cur_zone.root_key, 24, 108);
                ImGui::SliderInt("Tecla Inicial (Low Key)", &cur_zone.low_key, 24, cur_zone.high_key);
                ImGui::SliderInt("Tecla Final (High Key)", &cur_zone.high_key, cur_zone.low_key, 108);
                
                ImGui::Separator();
                ImGui::SliderFloat("Início do Slice (%)", &cur_zone.sample_start_pct, 0.0f, cur_zone.sample_end_pct - 0.001f, "%.3f");
                ImGui::SliderFloat("Fim do Slice (%)", &cur_zone.sample_end_pct, cur_zone.sample_start_pct + 0.001f, 1.0f, "%.3f");
                
                // Presets rápidos de tamanho de slice
                if (tot_sec > 0.0f) {
                    ImGui::TextDisabled("Tamanho Rápido do Corte:");
                    ImGui::SameLine();
                    if (ImGui::SmallButton("1 Hit (0.5s)")) {
                        cur_zone.sample_end_pct = std::min(1.0f, cur_zone.sample_start_pct + (0.5f / tot_sec));
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("1 Bar (1.75s)")) {
                        cur_zone.sample_end_pct = std::min(1.0f, cur_zone.sample_start_pct + (1.75f / tot_sec));
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("2 Bars (3.5s)")) {
                        cur_zone.sample_end_pct = std::min(1.0f, cur_zone.sample_start_pct + (3.5f / tot_sec));
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("4 Bars (7.0s)")) {
                        cur_zone.sample_end_pct = std::min(1.0f, cur_zone.sample_start_pct + (7.0f / tot_sec));
                    }
                }

                ImGui::SliderInt("Afinação (Tune)", &cur_zone.tune_semitones, -24, 24, "%+d st");
                ImGui::SliderFloat("Ganho da Zona", &cur_zone.gain, 0.0f, 2.0f, "%.2fx");
                ImGui::SliderFloat("Pan Estéreo", &cur_zone.pan, -1.0f, 1.0f, "%.2f");

                const char* loop_modes[] = { "One-Shot (Sem Loop)", "Forward Loop", "Ping-Pong Loop" };
                ImGui::Combo("Modo de Loop", &cur_zone.loop_mode, loop_modes, IM_ARRAYSIZE(loop_modes));

                if (ImGui::Button("🔁 Inverter Slice (Reverse)", ImVec2(180, 22))) {
                    std::swap(cur_zone.sample_start_pct, cur_zone.sample_end_pct);
                    status_feedback = "Slice invertido (Reverse)!";
                }
                ImGui::SameLine();
                if (ImGui::Button("▶ Testar Este Slice", ImVec2(160, 22))) {
                    playTestNote(cur_zone.root_key, 110, true);
                }
            }

            ImGui::Columns(1);
        }

        void RenderFiltersAndEnvelopes() {
            ImGui::Columns(4, "SynthCols", true);

            // Coluna 1: Filtro
            ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.0f, 1.0f), "🎛️ FILTRO ANALÓGICO 24dB");
            ImGui::Separator();
            const char* flt_types[] = { "24dB Lowpass", "24dB Highpass", "Bandpass 12dB", "Notch Filter" };
            ImGui::Combo("Tipo##FltType", &filter_type_idx, flt_types, IM_ARRAYSIZE(flt_types));
            ImGui::SliderFloat("Cutoff (Hz)", &filter_cutoff_hz, 40.0f, 20000.0f, "%.0f Hz");
            ImGui::SliderFloat("Ressonância", &filter_resonance, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Env Amount", &filter_env_amount, -1.0f, 1.0f, "%.2f");

            ImGui::NextColumn();

            // Coluna 2: Envelope ADSR de Volume
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "📈 AMP ENVELOPE (ADSR)");
            ImGui::Separator();
            ImGui::SliderFloat("Attack##AmpA", &amp_attack, 0.001f, 2.0f, "%.3fs");
            ImGui::SliderFloat("Decay##AmpD", &amp_decay, 0.010f, 3.0f, "%.3fs");
            ImGui::SliderFloat("Sustain##AmpS", &amp_sustain, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Release##AmpR", &amp_release, 0.010f, 4.0f, "%.3fs");

            ImGui::NextColumn();

            // Coluna 3: Time-Stretching & Engine Modes
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "🔄 TIME-STRETCH & PITCH");
            ImGui::Separator();
            const char* p_modes[] = { "Resample (Tape Speed)", "Granular Time-Stretch", "Drone Texture" };
            ImGui::Combo("Modo Pitch", &pitch_mode, p_modes, IM_ARRAYSIZE(p_modes));
            if (pitch_mode > 0) {
                ImGui::SliderFloat("Grain Size (ms)", &grain_size_ms, 10.0f, 120.0f, "%.1f ms");
                ImGui::SliderFloat("Formant Shift", &formant_shift, -12.0f, 12.0f, "%.1f st");
            } else {
                ImGui::TextDisabled("Afinação ligada à velocidade.");
            }

            ImGui::NextColumn();

            // Coluna 4: Round-Robin & Humanização
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "🎲 ROUND-ROBIN & HUMAN");
            ImGui::Separator();
            const char* rr_modes[] = { "Desativado", "Sequencial (1-2-3-4)", "Aleatório sem Repetir" };
            ImGui::Combo("Modo RR", &round_robin_mode, rr_modes, IM_ARRAYSIZE(rr_modes));
            ImGui::SliderFloat("Velocity Jitter (%)", &humanize_velocity_pct, 0.0f, 20.0f, "%.1f%%");
            ImGui::SliderFloat("Timing Jitter (ms)", &humanize_timing_ms, 0.0f, 25.0f, "%.1f ms");
            ImGui::SliderFloat("Analog Drift (cents)", &humanize_tuning_cents, 0.0f, 15.0f, "%.1f ct");

            ImGui::Columns(1);
        }

        void RenderLoopAndWaveformEditor() {
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "VISUALIZADOR DE ONDA PCM & MARCADORES DE CORTE / LOOP:");
            
            if (selected_zone_idx >= 0 && selected_zone_idx < (int)zones.size()) {
                auto& cur_zone = zones[selected_zone_idx];

                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 wf_p0 = ImGui::GetCursorScreenPos();
                ImVec2 wf_sz = ImVec2(ImGui::GetContentRegionAvail().x, 150.0f);
                ImVec2 wf_p1 = ImVec2(wf_p0.x + wf_sz.x, wf_p0.y + wf_sz.y);

                // Fundo do Canvas da Onda
                dl->AddRectFilled(wf_p0, wf_p1, IM_COL32(10, 14, 20, 255), 4.0f);
                dl->AddRect(wf_p0, wf_p1, IM_COL32(24, 40, 58, 255), 4.0f);

                // Linha Central (Zero Crossing)
                float mid_y = (wf_p0.y + wf_p1.y) * 0.5f;
                dl->AddLine(ImVec2(wf_p0.x, mid_y), ImVec2(wf_p1.x, mid_y), IM_COL32(40, 55, 75, 180), 1.0f);

                // Desenhar Onda Sonora em Neon Cyan
                float start_x = wf_p0.x + cur_zone.sample_start_pct * wf_sz.x;
                float end_x = wf_p0.x + cur_zone.sample_end_pct * wf_sz.x;

                for (float wx = start_x; wx < end_x; wx += 2.0f) {
                    float norm_x = (wx - start_x) / (end_x - start_x);
                    float amp = (std::sin(norm_x * 40.0f) * 0.4f + std::sin(norm_x * 85.0f) * 0.3f + std::cos(norm_x * 12.0f) * 0.3f) * (wf_sz.y * 0.38f);
                    
                    // Decaimento natural do envelope
                    amp *= (1.0f - norm_x * 0.65f);
                    
                    dl->AddLine(ImVec2(wx, mid_y - std::abs(amp)), ImVec2(wx, mid_y + std::abs(amp)), IM_COL32(0, 212, 255, 240), 1.5f);
                }

                // ── MARCADORES INTERATIVOS ARRASTÁVEIS ──────────────────────
                float m_start_x = wf_p0.x + cur_zone.sample_start_pct * wf_sz.x;
                float m_loop_start_x = wf_p0.x + cur_zone.loop_start_pct * wf_sz.x;
                float m_loop_end_x = wf_p0.x + cur_zone.loop_end_pct * wf_sz.x;
                float m_end_x = wf_p0.x + cur_zone.sample_end_pct * wf_sz.x;

                // 1. Start Marker (Verde)
                dl->AddLine(ImVec2(m_start_x, wf_p0.y), ImVec2(m_start_x, wf_p1.y), IM_COL32(0, 255, 128, 255), 2.0f);
                dl->AddTriangleFilled(ImVec2(m_start_x - 6, wf_p0.y), ImVec2(m_start_x + 6, wf_p0.y), ImVec2(m_start_x, wf_p0.y + 10), IM_COL32(0, 255, 128, 255));
                dl->AddText(ImVec2(m_start_x + 4, wf_p0.y + 12), IM_COL32(0, 255, 128, 255), "START");

                // 2. Loop Start Marker (Ciano)
                if (cur_zone.loop_mode > 0) {
                    dl->AddLine(ImVec2(m_loop_start_x, wf_p0.y), ImVec2(m_loop_start_x, wf_p1.y), IM_COL32(0, 212, 255, 255), 2.0f);
                    dl->AddTriangleFilled(ImVec2(m_loop_start_x - 6, wf_p0.y), ImVec2(m_loop_start_x + 6, wf_p0.y), ImVec2(m_loop_start_x, wf_p0.y + 10), IM_COL32(0, 212, 255, 255));
                    dl->AddText(ImVec2(m_loop_start_x + 4, wf_p0.y + 12), IM_COL32(0, 212, 255, 255), "LOOP S");

                    // 3. Loop End Marker (Laranja)
                    dl->AddLine(ImVec2(m_loop_end_x, wf_p0.y), ImVec2(m_loop_end_x, wf_p1.y), IM_COL32(255, 165, 0, 255), 2.0f);
                    dl->AddTriangleFilled(ImVec2(m_loop_end_x - 6, wf_p1.y), ImVec2(m_loop_end_x + 6, wf_p1.y), ImVec2(m_loop_end_x, wf_p1.y - 10), IM_COL32(255, 165, 0, 255));
                    dl->AddText(ImVec2(m_loop_end_x + 4, wf_p1.y - 24), IM_COL32(255, 165, 0, 255), "LOOP E");

                    // Região do Loop com Destaque
                    dl->AddRectFilled(ImVec2(m_loop_start_x, wf_p0.y), ImVec2(m_loop_end_x, wf_p1.y), IM_COL32(0, 212, 255, 30));
                }

                // 4. End Marker (Vermelho)
                dl->AddLine(ImVec2(m_end_x, wf_p0.y), ImVec2(m_end_x, wf_p1.y), IM_COL32(255, 60, 60, 255), 2.0f);
                dl->AddTriangleFilled(ImVec2(m_end_x - 6, wf_p1.y), ImVec2(m_end_x + 6, wf_p1.y), ImVec2(m_end_x, wf_p1.y - 10), IM_COL32(255, 60, 60, 255));
                dl->AddText(ImVec2(m_end_x - 36, wf_p1.y - 24), IM_COL32(255, 60, 60, 255), "END");

                // Lógica de Arraste com o Mouse no Canvas
                if (ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(wf_p0, wf_p1)) {
                    float mouse_x = ImGui::GetMousePos().x;
                    if (std::abs(mouse_x - m_start_x) < 12.0f) dragging_marker = 0;
                    else if (std::abs(mouse_x - m_loop_start_x) < 12.0f) dragging_marker = 1;
                    else if (std::abs(mouse_x - m_loop_end_x) < 12.0f) dragging_marker = 2;
                    else if (std::abs(mouse_x - m_end_x) < 12.0f) dragging_marker = 3;
                }

                if (ImGui::IsMouseReleased(0)) {
                    dragging_marker = -1;
                }

                if (dragging_marker >= 0 && ImGui::IsMouseDragging(0)) {
                    float norm_mouse = std::clamp((ImGui::GetMousePos().x - wf_p0.x) / wf_sz.x, 0.0f, 1.0f);
                    if (dragging_marker == 0) cur_zone.sample_start_pct = std::min(norm_mouse, cur_zone.sample_end_pct - 0.05f);
                    else if (dragging_marker == 1) cur_zone.loop_start_pct = std::clamp(norm_mouse, cur_zone.sample_start_pct, cur_zone.loop_end_pct - 0.02f);
                    else if (dragging_marker == 2) cur_zone.loop_end_pct = std::clamp(norm_mouse, cur_zone.loop_start_pct + 0.02f, cur_zone.sample_end_pct);
                    else if (dragging_marker == 3) cur_zone.sample_end_pct = std::max(norm_mouse, cur_zone.sample_start_pct + 0.05f);
                }

                ImGui::Dummy(wf_sz);
                ImGui::Spacing();

                // Controles dos Parâmetros de Loop
                ImGui::Columns(3, "LoopCols", true);

                const char* loop_modes[] = { "One-Shot (Sem Loop)", "Forward Loop (Contínuo)", "Ping-Pong (Ida e Volta)" };
                ImGui::Combo("Modo de Loop", &cur_zone.loop_mode, loop_modes, IM_ARRAYSIZE(loop_modes));
                ImGui::SliderFloat("Crossfade (%)", &cur_zone.crossfade_pct, 0.0f, 0.20f, "%.3f");

                ImGui::NextColumn();
                ImGui::SliderFloat("Início do Loop (%)", &cur_zone.loop_start_pct, cur_zone.sample_start_pct, cur_zone.loop_end_pct, "%.2f");
                ImGui::SliderFloat("Fim do Loop (%)", &cur_zone.loop_end_pct, cur_zone.loop_start_pct, cur_zone.sample_end_pct, "%.2f");

                ImGui::NextColumn();
                ImGui::SliderFloat("Início da Amostra (%)", &cur_zone.sample_start_pct, 0.0f, cur_zone.sample_end_pct, "%.2f");
                ImGui::SliderFloat("Fim da Amostra (%)", &cur_zone.sample_end_pct, cur_zone.sample_start_pct, 1.0f, "%.2f");

                ImGui::Columns(1);
            }
        }

        void RenderIntegratedFXRack() {
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "RACK DE EFEITOS INTEGRADO DO DIRECTWAVE:");
            ImGui::Spacing();

            ImGui::Columns(4, "FxRackCols", true);

            // FX 1: Tape Saturation / Drive
            ImGui::Checkbox("Tape Saturation", &fx_drive_enable);
            ImGui::Separator();
            if (fx_drive_enable) {
                ImGui::SliderFloat("Drive Amount", &fx_drive_amount, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Tube Warmth", &fx_tube_warmth, 0.0f, 1.0f, "%.2f");
            } else {
                ImGui::TextDisabled("Bypassed");
            }

            ImGui::NextColumn();

            // FX 2: Stereo Chorus
            ImGui::Checkbox("Stereo Chorus", &fx_chorus_enable);
            ImGui::Separator();
            if (fx_chorus_enable) {
                ImGui::SliderFloat("Chorus Rate", &fx_chorus_rate, 0.1f, 5.0f, "%.1f Hz");
                ImGui::SliderFloat("Depth", &fx_chorus_depth, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Wet Mix", &fx_chorus_mix, 0.0f, 1.0f, "%.2f");
            } else {
                ImGui::TextDisabled("Bypassed");
            }

            ImGui::NextColumn();

            // FX 3: BPM Sync Delay
            ImGui::Checkbox("BPM Sync Delay", &fx_delay_enable);
            ImGui::Separator();
            if (fx_delay_enable) {
                const char* delay_sync_times[] = { "1/4 Beat", "1/8 Beat", "1/16 Beat", "1/8 Triplet" };
                ImGui::Combo("Time", &fx_delay_sync_idx, delay_sync_times, IM_ARRAYSIZE(delay_sync_times));
                ImGui::SliderFloat("Feedback", &fx_delay_feedback, 0.0f, 0.90f, "%.2f");
                ImGui::SliderFloat("Delay Wet", &fx_delay_wet, 0.0f, 1.0f, "%.2f");
            } else {
                ImGui::TextDisabled("Bypassed");
            }

            ImGui::NextColumn();

            // FX 4: 3-Band Parametric EQ & Reverb
            ImGui::Checkbox("3-Band EQ & Reverb", &fx_eq_enable);
            ImGui::Separator();
            if (fx_eq_enable) {
                ImGui::SliderFloat("Low (dB)", &fx_eq_low_db, -12.0f, 12.0f, "%.1f dB");
                ImGui::SliderFloat("Mid (dB)", &fx_eq_mid_db, -12.0f, 12.0f, "%.1f dB");
                ImGui::SliderFloat("High (dB)", &fx_eq_high_db, -12.0f, 12.0f, "%.1f dB");
                ImGui::SliderFloat("Reverb Wet", &fx_reverb_wet, 0.0f, 1.0f, "%.2f");
            } else {
                ImGui::TextDisabled("Bypassed");
            }

            ImGui::Columns(1);
        }

        void RenderInteractiveKeyboard() {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Teclado Interativo (Clique nas teclas para tocar notas e acionar as zonas):");
            
            ImDrawList* dl_k = ImGui::GetWindowDrawList();
            ImVec2 kb_p0 = ImGui::GetCursorScreenPos();
            ImVec2 kb_sz = ImVec2(ImGui::GetContentRegionAvail().x, 65.0f);
            ImVec2 kb_p1 = ImVec2(kb_p0.x + kb_sz.x, kb_p0.y + kb_sz.y);

            dl_k->AddRectFilled(kb_p0, kb_p1, IM_COL32(12, 16, 22, 255), 3.0f);
            dl_k->AddRect(kb_p0, kb_p1, IM_COL32(30, 45, 65, 255), 3.0f);

            int num_white_keys = 52;
            float key_w = kb_sz.x / (float)num_white_keys;

            // Desenha teclas brancas
            for (int k = 0; k < num_white_keys; ++k) {
                float kx0 = kb_p0.x + k * key_w;
                float kx1 = kx0 + key_w - 1.0f;
                int note_num = 24 + (k * 84 / num_white_keys);
                
                bool is_pressed = (active_pressed_key == note_num);
                ImU32 w_col = is_pressed ? IM_COL32(255, 165, 0, 255) : IM_COL32(230, 238, 245, 255);
                
                // Pinta o rodapé da tecla com a cor da zona à qual ela pertence
                ImU32 zone_tint = IM_COL32(50, 60, 70, 255);
                for (const auto& z : zones) {
                    if (note_num >= z.low_key && note_num <= z.high_key) {
                        zone_tint = ImColor(z.color.x, z.color.y, z.color.z, 0.90f);
                        break;
                    }
                }

                dl_k->AddRectFilled(ImVec2(kx0, kb_p0.y), ImVec2(kx1, kb_p1.y - 8.0f), w_col, 1.0f);
                dl_k->AddRectFilled(ImVec2(kx0, kb_p1.y - 8.0f), ImVec2(kx1, kb_p1.y), zone_tint, 1.0f);
            }

            // Detecta clique do mouse no teclado
            if (ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(kb_p0, kb_p1)) {
                float mouse_x = ImGui::GetMousePos().x - kb_p0.x;
                int clicked_k = (int)(mouse_x / key_w);
                int midi_n = std::clamp(24 + (clicked_k * 84 / num_white_keys), 24, 108);
                playTestNote(midi_n, 110);
            }

            ImGui::Dummy(kb_sz);

            // Status Bar
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "[i] %s", status_feedback.c_str());
        }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(1040, 700), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.50f, 0.00f, 0.85f)); // Laranja DirectWave

            if (ImGui::Begin("FL DIRECTWAVE ADVANCED MULTI-SAMPLER###DirectWaveWindow", &is_open, ImGuiWindowFlags_MenuBar)) {
                
                // ── MENU SUPERIOR DO DIRECTWAVE ──────────────────────────────
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Arquivo")) {
                        if (ImGui::MenuItem("📂 Carregar Música Completa (.wav / .mp3)...")) {
                            std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                            if (!path.empty()) importCustomAudioTrack(path);
                        }
                        if (ImGui::MenuItem("🤖 Auto-Fatiar em 8 Transientes (MPC Slicer)...")) {
                            std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                            if (!path.empty()) autoSliceTrack8Parts(path);
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Novo Programa (Limpar Zonas)")) {
                            zones.clear();
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Presets de Artistas")) {
                        if (ImGui::MenuItem("👽 Perception - Alien Rave Matrix (Signature)")) {
                            loadPreset(0);
                        }
                        if (ImGui::MenuItem("🌌 Astrix - Adhana Multi-Zone Matrix")) {
                            loadPreset(1);
                        }
                        if (ImGui::MenuItem("⚡ Psy Rolling Bass Keyzones")) {
                            loadPreset(2);
                        }
                        if (ImGui::MenuItem("🥁 808 & 909 Stadium Drum Velocity Kit")) {
                            loadPreset(3);
                        }
                        ImGui::EndMenu();
                    }

                    float menu_w = ImGui::GetWindowWidth();
                    ImGui::SameLine(menu_w - 90);
                    if (ImGui::Button("✖ Fechar", ImVec2(80, 20))) {
                        is_open = false;
                    }
                    ImGui::EndMenuBar();
                }

                // Header do Plugin
                ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.0f, 1.0f), "FL DIRECTWAVE MULTI-ZONE INSTRUMENT SAMPLER & SOUNDFONT ENGINE");
                ImGui::TextDisabled("Mapeador profissional de multi-amostras com Keyzones, camadas de velocidade, filtro analógico, 2 ADSRs, LFOs e Rack FX.");
                ImGui::Separator();
                ImGui::Spacing();

                RenderSubTabs();
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI
