#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "../plugin_manager/ThematicSynths.h"
#include "../audio/KuroWave.h"
#include "../audio/SynthEngine.h"
#include "../core/ClipManager.h"
#include "../plugin_manager/KuroSamplerNode.h"

extern KuroDSP::ExpressiveLeadSynth g_lead_synth;
extern KuroDSP::MonkSynth g_monk_synth;
extern KuroDSP::AlienVoiceSynth g_alien_synth;
extern KuroDSP::AnalogMonsterSynth g_analog_synth;
extern KuroDSP::SynthwaveSynth g_synthwave_synth;
extern KuroDSP::AbductionFMSynth g_fm_synth;
extern KuroDSP::AcousticContrabassSynth g_contrabass_synth;
extern KuroAudio::KuroWave g_kurowave;
extern KuroAudio::SynthEngine g_piano_synth;
extern std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;

namespace KuroUI {

    struct SynthPreset {
        std::string name;
        std::string category; // Lead, Bass, Pad, FX, Pluck, Vocal, Acoustic
        std::string target_synth; // ExpressiveLead, AnalogMonster, MonkSynth, Synthwave, AbductionFM, AcousticContrabass, KuroWave
        std::string description;
        float param0 = 0.5f; // Cutoff / FM Ratio / Formant Morph
        float param1 = 0.3f; // Resonance / Mod Index / Throat Growl
        float param2 = 0.0f; // Drive / Unison / Slap Attack
        float attack = 0.01f;
        float release = 0.30f;
    };

    struct SoundBankSample {
        std::string name;
        std::string category;
        std::string filename;
        float duration_sec;
    };

    class PresetManagerUI {
    private:
        bool is_open = false;
        std::vector<SynthPreset> preset_library;
        std::vector<SoundBankSample> sample_library;
        int selected_preset_idx = 0;
        int selected_sample_idx = 0;
        int selected_category_idx = 0;
        int selected_sample_cat_idx = 0;
        int current_tab = 0; // 0: Presets de Sintetizadores, 1: Banco de Samples EDM/Psytrance
        char search_filter[128] = "";
        char sample_search_filter[128] = "";
        std::string status_feedback = "Selecione um patch ou sample para carregar.";

    public:
        PresetManagerUI() {
            initPresetLibrary();
            initSampleLibrary();
        }

        void initPresetLibrary() {
            preset_library = {
                // --- LEADS & ARPS ---
                { "Psy Screamer Saw 140BPM", "Lead", "ExpressiveLead", "Lead dente de serra agressivo com modulação FM hiper-ressonante para Psytrance", 2.0f, 4.5f, 0.6f, 0.005f, 0.20f },
                { "Acid 303 Screamer", "Lead", "ExpressiveLead", "Lead estilo 303 com ressonância ácida estridente e slide dinâmico", 3.0f, 6.0f, 0.8f, 0.002f, 0.15f },
                { "Goa Sunrise Anthem Lead", "Lead", "ExpressiveLead", "Melodia hipnótica de Goa Trance com modulação FM rica em harmônicos", 1.5f, 3.2f, 0.4f, 0.010f, 0.40f },
                { "Laser Pluck Cyber Matrix", "Pluck", "ExpressiveLead", "Pluck ultra rápido com transiente cortante para arpejos em 16ths", 4.0f, 2.0f, 0.5f, 0.001f, 0.12f },
                { "Infected Resonator", "Lead", "ExpressiveLead", "Timbre estilo Infected Mushroom com modulação cruzada e formantes alienígenas", 2.5f, 7.5f, 0.7f, 0.004f, 0.25f },
                { "Outrun 80s Neon Lead", "Lead", "Synthwave", "Sintetizador vintage dos anos 80 com chorus analógico estéreo e brilho neon", 0.75f, 0.25f, 0.3f, 0.015f, 0.35f },

                // --- BASSES ---
                { "Deep KBB Rolling Bass 140BPM", "Bass", "AnalogMonster", "Linha de baixo KBB para Psytrance com corte sub 40Hz e corpo 16th", 0.38f, 0.22f, 0.45f, 0.001f, 0.10f },
                { "Rolling Psy 16th Saw Bass", "Bass", "AnalogMonster", "Baixo dente de serra com envelope rápido e resposta precisa de ataque", 0.45f, 0.30f, 0.50f, 0.002f, 0.12f },
                { "Moog Reese Monster Sub", "Bass", "AnalogMonster", "Reese Bass estéreo distorcido com saturação analógica pesada", 0.30f, 0.15f, 0.85f, 0.020f, 0.50f },
                { "Dark Acid 303 Growl", "Bass", "AnalogMonster", "Baixo ácido com distorção de diodo e envelope de corte ressonante", 0.55f, 0.65f, 0.70f, 0.001f, 0.18f },
                { "Heavy 808 Sub Boom", "Bass", "AnalogMonster", "Sub-bass 808 limpo e profundo com saturação sutil para peso no peito", 0.25f, 0.10f, 0.30f, 0.005f, 0.60f },
                { "Warm Upright Jazz Bass", "Acoustic", "AcousticContrabass", "Contrabaixo acústico de madeira real com ressonância corporal aveludada", 0.42f, 0.18f, 0.15f, 0.020f, 0.55f },
                { "Pizzicato Slap Double Bass", "Acoustic", "AcousticContrabass", "Contrabaixo acústico com ataque de slap percussivo e estalo de corda", 0.60f, 0.35f, 0.75f, 0.003f, 0.35f },

                // --- VOCAL & MONK ---
                { "Tibetan Alien Monk Drone", "Vocal", "MonkSynth", "Canto harmônico tibetano ancestral com formante O->U e ressonância de garganta", 0.80f, 0.60f, 0.3f, 0.150f, 1.50f },
                { "Gregorian Sanctus Choir", "Vocal", "MonkSynth", "Coral sacro gregoriano com formante A->E límpido e espacial", 0.30f, 0.20f, 0.1f, 0.200f, 2.00f },
                { "Mystical Throat Chant", "Vocal", "MonkSynth", "Canto de garganta tuvano com modulação de formantes e distorção rítmica", 0.65f, 0.85f, 0.5f, 0.080f, 1.20f },
                { "Cyber Formant Morph", "Vocal", "MonkSynth", "Vocalização futurista com morphing contínuo de vogais para psy-ambient", 0.50f, 0.45f, 0.4f, 0.050f, 0.80f },

                // --- PADS & ATMOSPHERES ---
                { "Astral Dream Wavetable Pad", "Pad", "KuroWave", "Pad celestial com modulação de wavetable e ambiência estéreo flutuante", 0.65f, 0.30f, 0.1f, 0.300f, 1.80f },
                { "80s Neon Sunset Warm Pad", "Pad", "Synthwave", "Colchão harmônico analógico retrô com modulação lenta de PWM", 0.50f, 0.25f, 0.2f, 0.250f, 1.60f },
                { "Alien Mothership Atmosphere", "Pad", "AlienVoice", "Paisagem sonora sombria extraterrestre com pitch shifting caótico", 0.70f, 0.55f, 0.4f, 0.400f, 2.50f },

                // --- PLUCKS & BELLS ---
                { "Wavetable Hypnotic Pluck", "Pluck", "KuroWave", "Pluck cristalino de wavetable para melodias hipnóticas de Psy e Trance", 0.80f, 0.35f, 0.2f, 0.001f, 0.45f },
                { "Crystal FM Bell Matrix", "Pluck", "AbductionFM", "Sino metálico FM brilhante com cauda harmônica cristalina", 0.85f, 0.50f, 0.3f, 0.002f, 0.80f },
                { "Retro Miami Cyber Pluck", "Pluck", "Synthwave", "Pluck vintage com transiente estalado e chorus estéreo anos 80", 0.70f, 0.40f, 0.2f, 0.003f, 0.30f },

                // --- FX & MODULATION ---
                { "FM Cyber Zap 16th", "FX", "AbductionFM", "Laser zap percussivo para viradas e preenchimentos psicodélicos", 0.95f, 0.80f, 0.7f, 0.001f, 0.08f },
                { "Dark Warp Morpher", "FX", "AbductionFM", "Efeito de dobra espacial com modulação FM extrema e ressonância escura", 0.60f, 0.90f, 0.8f, 0.010f, 0.60f },
                { "Sub Sonic Drop Boom", "FX", "AnalogMonster", "Queda de subgrave 80Hz a 20Hz para quedas épicas de pista", 0.90f, 0.40f, 0.6f, 0.002f, 1.50f }
            };
        }

        void initSampleLibrary() {
            sample_library = {
                // --- VINI VICI & ASTRIX: ADHANA SIGNATURE KIT (STEM SLICES) ---
                { "Adhana Astrix Kick Punch 01", "Adhana Signature", "adhana_signature\\Adhana_Astrix_Kick_Punch_01.wav", 0.220f },
                { "Adhana Astrix Kick Sub 02", "Adhana Signature", "adhana_signature\\Adhana_Astrix_Kick_Sub_02.wav", 0.320f },
                { "Adhana Kick Loop 4-Beat (138BPM)", "Adhana Signature", "adhana_signature\\Adhana_Kick_Loop_138BPM.wav", 1.739f },
                { "Adhana Rolling Bass Hit 01 (16th)", "Adhana Signature", "adhana_signature\\Adhana_Rolling_Bass_Hit_01.wav", 0.109f },
                { "Adhana Rolling Bassline Loop (138BPM)", "Adhana Signature", "adhana_signature\\Adhana_Rolling_Bassline_Loop_138BPM.wav", 1.739f },
                { "Adhana Acid Bass Stab 01", "Adhana Signature", "adhana_signature\\Adhana_Acid_Bass_Stab_01.wav", 0.350f },
                { "Adhana Psy Lead Hook (2-Bars)", "Adhana Signature", "adhana_signature\\Adhana_Psy_Lead_Hook_2Bars.wav", 3.478f },
                { "Adhana Lead Stab Hit", "Adhana Signature", "adhana_signature\\Adhana_Lead_Stab_Hit.wav", 0.400f },
                { "Adhana Acid Arp Loop (138BPM)", "Adhana Signature", "adhana_signature\\Adhana_Acid_Arp_Loop_138BPM.wav", 1.739f },
                { "Adhana Vocal Mantra Chant 01", "Adhana Signature", "adhana_signature\\Adhana_Vocal_Mantra_Chant_01.wav", 2.800f },
                { "Adhana Vocal Mantra Chant 02", "Adhana Signature", "adhana_signature\\Adhana_Vocal_Mantra_Chant_02.wav", 3.200f },
                { "Adhana Tribal Vocal Chop", "Adhana Signature", "adhana_signature\\Adhana_Tribal_Vocal_Chop.wav", 0.450f },
                { "Adhana Psy Laser Zap FX", "Adhana Signature", "adhana_signature\\Adhana_Psy_Laser_Zap_FX.wav", 0.800f },
                { "Adhana Cosmic Riser Sweep FX", "Adhana Signature", "adhana_signature\\Adhana_Cosmic_Riser_FX.wav", 2.000f },

                // --- BUILT-IN EDM & PSYTRANCE DSP SAMPLES ---
                // KICKS
                { "Psytrance Kick 140BPM", "Kicks", "Psytrance_Kick_140BPM.wav", 0.220f },
                { "Hardstyle Sub Kick", "Kicks", "Hardstyle_Sub_Kick.wav", 0.350f },
                { "Clean 808 Sub Kick", "Kicks", "Clean_808_Sub_Kick.wav", 0.600f },
                { "Slap Punch Kick", "Kicks", "Slap_Punch_Kick.wav", 0.180f },

                // BASSLINES
                { "Rolling Psy Bass 16th", "Basslines", "Rolling_Psy_Bass_16th.wav", 0.107f },
                { "Acid 303 Saw Hit", "Basslines", "Acid_303_Saw_Hit.wav", 0.250f },
                { "Sub Sine 40Hz", "Basslines", "Sub_Sine_40Hz.wav", 0.500f },
                { "Reese Distortion Bass", "Basslines", "Reese_Distortion_Bass.wav", 0.400f },

                // SNARES & CLAPS
                { "909 Tight Snare", "Snares & Claps", "909_Tight_Snare.wav", 0.200f },
                { "808 Crisp Clap (Stereo)", "Snares & Claps", "808_Crisp_Clap.wav", 0.250f },
                { "EDM Smash Stadium Clap", "Snares & Claps", "EDM_Smash_Clap.wav", 0.350f },
                { "Ghost Snare Body Click", "Snares & Claps", "Ghost_Snare_Click.wav", 0.080f },

                // HIHATS & PERC
                { "Closed Metal Hat 909", "HiHats & Perc", "Closed_Metal_Hat.wav", 0.050f },
                { "Open Psy Sizzle Hat", "HiHats & Perc", "Open_Psy_Hat.wav", 0.250f },
                { "Psy Click Percussion Tick", "HiHats & Perc", "Psy_Click_Perc.wav", 0.030f },
                { "Shaker Groove High Hit", "HiHats & Perc", "Shaker_Groove_Hit.wav", 0.070f },

                // SFX & RISERS
                { "Psy Zap Laser Attack", "SFX & Risers", "Psy_Zap_Laser.wav", 0.120f },
                { "Alien Downlifter Drop", "SFX & Risers", "Alien_Downlifter.wav", 1.200f },
                { "White Noise Sweep Riser", "SFX & Risers", "White_Noise_Sweep.wav", 1.500f },
                { "Cyber Sub Impact Boom", "SFX & Risers", "Cyber_Sub_Impact.wav", 0.800f }
            };
        }

        std::string resolveSamplePath(const std::string& filename) {
            std::vector<std::string> candidates = {
                "assets\\samples\\" + filename,
                "assets\\samples\\adhana_signature\\" + filename,
                "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\assets\\samples\\" + filename,
                "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\assets\\samples\\adhana_signature\\" + filename
            };
            for (const auto& p : candidates) {
                if (std::filesystem::exists(p)) return p;
            }
            return candidates[0];
        }

        void PlaySamplePreview(const SoundBankSample& sample) {
            std::string path = resolveSamplePath(sample.filename);
            if (std::filesystem::exists(path)) {
                PlaySoundA(path.c_str(), NULL, SND_ASYNC | SND_FILENAME);
                status_feedback = "Reproduzindo sample: " + sample.name + " (" + sample.category + ")";
            } else {
                status_feedback = "Erro: Arquivo do sample não encontrado: " + path;
            }
        }

        void LoadSampleIntoGlobalSampler(const SoundBankSample& sample) {
            std::string path = resolveSamplePath(sample.filename);
            if (std::filesystem::exists(path) && g_global_sampler) {
                g_global_sampler->loadSample(path);
                status_feedback = "Sample '" + sample.name + "' carregado no Sampler Global!";
            }
        }

        bool& getOpenState() { return is_open; }
        void open() { is_open = true; }
        void close() { is_open = false; }
        void toggle() { is_open = !is_open; }

        void ApplyPreset(const SynthPreset& patch) {
            if (patch.target_synth == "ExpressiveLead") {
                g_lead_synth.setParameter(0, patch.param0); // FM Ratio
                g_lead_synth.setParameter(1, patch.param1); // Mod Index
            } else if (patch.target_synth == "AnalogMonster") {
                g_analog_synth.setParameter(0, patch.param0 * 18000.0f + 200.0f); // Cutoff
                g_analog_synth.setParameter(1, patch.param1); // Resonance
                g_analog_synth.setParameter(2, patch.param2); // Drive
            } else if (patch.target_synth == "MonkSynth") {
                g_monk_synth.setParameter(0, patch.param0); // Formant Morph
                g_monk_synth.setParameter(1, patch.param1); // Throat Growl
            } else if (patch.target_synth == "Synthwave") {
                g_synthwave_synth.setParameter(0, patch.param0 * 15000.0f + 300.0f);
                g_synthwave_synth.setParameter(1, patch.param1);
            } else if (patch.target_synth == "AbductionFM") {
                g_fm_synth.setParameter(0, patch.param0 * 8.0f);
                g_fm_synth.setParameter(1, patch.param1 * 10.0f);
            } else if (patch.target_synth == "AcousticContrabass") {
                g_contrabass_synth.setParameter(0, patch.param0);
                g_contrabass_synth.setParameter(1, patch.param1);
            } else if (patch.target_synth == "KuroWave") {
                g_kurowave.param_cutoff = patch.param0 * 18000.0f + 200.0f;
                g_kurowave.param_resonance = patch.param1 * 4.0f;
                g_kurowave.param_unison_detune = patch.param2;
            }

            status_feedback = "Patch '" + patch.name + "' carregado no " + patch.target_synth + " com sucesso!";
        }

        void PlayTestNote(const SynthPreset& patch) {
            ApplyPreset(patch);
            int note = 60; // C4
            if (patch.category == "Bass" || patch.category == "Acoustic") {
                note = 36; // C2
            } else if (patch.category == "Lead" || patch.category == "Pluck") {
                note = 69; // A4
            }

            if (patch.target_synth == "ExpressiveLead") {
                g_lead_synth.pushMidiEvent({note, note, true, 0.85f, 0.0f, 0.5f, 0.5f, 0.4f});
            } else if (patch.target_synth == "AnalogMonster") {
                g_analog_synth.pushMidiEvent({note, note, true, 0.90f, 0.0f, 0.5f, 0.5f, 0.35f});
            } else if (patch.target_synth == "MonkSynth") {
                g_monk_synth.pushMidiEvent({note, note, true, 0.85f, 0.0f, patch.param0, patch.param1, 0.8f});
            } else if (patch.target_synth == "Synthwave") {
                g_synthwave_synth.pushMidiEvent({note, note, true, 0.80f, 0.0f, 0.5f, 0.5f, 0.5f});
            } else if (patch.target_synth == "AbductionFM") {
                g_fm_synth.pushMidiEvent({note, note, true, 0.85f, 0.0f, 0.5f, 0.5f, 0.3f});
            } else if (patch.target_synth == "AcousticContrabass") {
                g_contrabass_synth.pushMidiEvent({note, note, true, 0.95f, 0.0f, patch.param0, patch.param1, 0.6f});
            } else if (patch.target_synth == "KuroWave") {
                g_kurowave.triggerNote(note, 0.6f, 0.9f);
            }
        }

        void Render() {
            if (!is_open) return;

            ImGui::SetNextWindowSize(ImVec2(840, 560), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.70f));

            if (ImGui::Begin(ICON_FA_SLIDERS " GERENCIADOR DE PRESETS & BANCO DE SONS EDM###PresetSoundBankManager", &is_open)) {
                
                // BARRA SUPERIOR DE ABAS
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                
                if (current_tab == 0) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.9f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.18f, 1.0f));
                }
                if (ImGui::Button(ICON_FA_SLIDERS " Presets de Sintetizadores Nativos", ImVec2(280, 30))) {
                    current_tab = 0;
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if (current_tab == 1) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.18f, 1.0f));
                }
                if (ImGui::Button(ICON_FA_DRUM " Banco de Samples EDM & Psytrance Matrix", ImVec2(320, 30))) {
                    current_tab = 1;
                }
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();

                ImGui::Separator();
                ImGui::Spacing();

                if (current_tab == 0) {
                    // =========================================================
                    // ABA 1: PRESETS DE SINTETIZADORES
                    // =========================================================
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_MAGNIFYING_GLASS " Filtrar:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(180);
                    ImGui::InputText("##PresetSearch", search_filter, sizeof(search_filter));

                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    const char* categories[] = { "Todas Categorias", "Lead", "Bass", "Pad", "FX", "Pluck", "Vocal", "Acoustic" };
                    ImGui::Combo("##PresetCat", &selected_category_idx, categories, IM_ARRAYSIZE(categories));

                    ImGui::SameLine();
                    if (selected_preset_idx >= 0 && selected_preset_idx < (int)preset_library.size()) {
                        const auto& cur_patch = preset_library[selected_preset_idx];
                        if (ImGui::Button(ICON_FA_VOLUME_HIGH " Testar Som", ImVec2(110, 24))) {
                            PlayTestNote(cur_patch);
                        }
                        ImGui::SameLine();
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.4f, 1.0f));
                        if (ImGui::Button(ICON_FA_BOLT " Carregar Patch", ImVec2(130, 24))) {
                            ApplyPreset(cur_patch);
                        }
                        ImGui::PopStyleColor();
                    }

                    ImGui::Spacing();
                    
                    // TABELA DE PRESETS
                    ImGui::BeginChild("##PresetListTable", ImVec2(0, 320), true);
                    {
                        ImGui::Columns(4, "PresetGridCols", true);
                        ImGui::SetColumnWidth(0, 250);
                        ImGui::SetColumnWidth(1, 100);
                        ImGui::SetColumnWidth(2, 170);

                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "NOME DO PATCH"); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "CATEGORIA"); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "SINTETIZADOR"); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "DESCRIÇÃO"); ImGui::NextColumn();
                        ImGui::Separator();

                        for (size_t i = 0; i < preset_library.size(); ++i) {
                            const auto& patch = preset_library[i];

                            if (strlen(search_filter) > 0) {
                                std::string n_low = patch.name;
                                std::string q_low = search_filter;
                                std::transform(n_low.begin(), n_low.end(), n_low.begin(), ::tolower);
                                std::transform(q_low.begin(), q_low.end(), q_low.begin(), ::tolower);
                                if (n_low.find(q_low) == std::string::npos) continue;
                            }
                            if (selected_category_idx > 0 && patch.category != categories[selected_category_idx]) {
                                continue;
                            }

                            ImGui::PushID((int)i);
                            bool is_selected = (selected_preset_idx == (int)i);
                            if (ImGui::Selectable(patch.name.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                                selected_preset_idx = (int)i;
                                ApplyPreset(patch);
                            }
                            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                                PlayTestNote(patch);
                            }
                            ImGui::NextColumn();

                            ImVec4 cat_col = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
                            if (patch.category == "Lead") cat_col = ImVec4(0.2f, 0.9f, 1.0f, 1.0f);
                            else if (patch.category == "Bass") cat_col = ImVec4(1.0f, 0.6f, 0.1f, 1.0f);
                            else if (patch.category == "Vocal") cat_col = ImVec4(0.9f, 0.4f, 1.0f, 1.0f);
                            else if (patch.category == "Acoustic") cat_col = ImVec4(0.9f, 0.8f, 0.3f, 1.0f);

                            ImGui::TextColored(cat_col, "%s", patch.category.c_str()); ImGui::NextColumn();
                            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.9f, 1.0f), "%s", patch.target_synth.c_str()); ImGui::NextColumn();
                            ImGui::TextDisabled("%s", patch.description.c_str()); ImGui::NextColumn();

                            ImGui::PopID();
                        }
                    }
                    ImGui::EndChild();

                    // DETALHES DO PRESET SELECIONADO
                    if (selected_preset_idx >= 0 && selected_preset_idx < (int)preset_library.size()) {
                        const auto& p = preset_library[selected_preset_idx];
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), ICON_FA_INFO " Parâmetros do Patch: %s (%s)", p.name.c_str(), p.target_synth.c_str());
                        ImGui::BulletText("Param 0 (Cutoff/FM/Morph): %.2f | Param 1 (Res/Mod/Growl): %.2f | Param 2 (Drive/Slap): %.2f", p.param0, p.param1, p.param2);
                    }

                } else {
                    // =========================================================
                    // ABA 2: BANCO DE SAMPLES EDM & PSYTRANCE
                    // =========================================================
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), ICON_FA_MAGNIFYING_GLASS " Filtrar Samples:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(180);
                    ImGui::InputText("##SampleSearch", sample_search_filter, sizeof(sample_search_filter));

                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(160);
                    const char* sample_cats[] = { "Todas Categorias", "Adhana Signature", "Kicks", "Basslines", "Snares & Claps", "HiHats & Perc", "SFX & Risers" };
                    ImGui::Combo("##SampleCat", &selected_sample_cat_idx, sample_cats, IM_ARRAYSIZE(sample_cats));

                    ImGui::SameLine();
                    if (selected_sample_idx >= 0 && selected_sample_idx < (int)sample_library.size()) {
                        const auto& cur_samp = sample_library[selected_sample_idx];
                        if (ImGui::Button(ICON_FA_PLAY " Ouvir Sample", ImVec2(120, 24))) {
                            PlaySamplePreview(cur_samp);
                        }
                        ImGui::SameLine();
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
                        if (ImGui::Button(ICON_FA_ARROW_RIGHT_TO_BRACKET " Carregar no Sampler", ImVec2(160, 24))) {
                            LoadSampleIntoGlobalSampler(cur_samp);
                        }
                        ImGui::PopStyleColor();
                    }

                    ImGui::Spacing();

                    // TABELA DE SAMPLES
                    ImGui::BeginChild("##SampleListTable", ImVec2(0, 320), true);
                    {
                        ImGui::Columns(4, "SampleGridCols", true);
                        ImGui::SetColumnWidth(0, 260);
                        ImGui::SetColumnWidth(1, 140);
                        ImGui::SetColumnWidth(2, 100);

                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "NOME DO SAMPLE"); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "CATEGORIA"); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "DURAÇÃO"); ImGui::NextColumn();
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "ARQUIVO WAV"); ImGui::NextColumn();
                        ImGui::Separator();

                        for (size_t i = 0; i < sample_library.size(); ++i) {
                            const auto& s = sample_library[i];

                            if (strlen(sample_search_filter) > 0) {
                                std::string n_low = s.name;
                                std::string q_low = sample_search_filter;
                                std::transform(n_low.begin(), n_low.end(), n_low.begin(), ::tolower);
                                std::transform(q_low.begin(), q_low.end(), q_low.begin(), ::tolower);
                                if (n_low.find(q_low) == std::string::npos) continue;
                            }
                            if (selected_sample_cat_idx > 0 && s.category != sample_cats[selected_sample_cat_idx]) {
                                continue;
                            }

                            ImGui::PushID((int)(i + 1000));
                            bool is_selected = (selected_sample_idx == (int)i);
                            if (ImGui::Selectable(s.name.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                                selected_sample_idx = (int)i;
                                PlaySamplePreview(s);
                            }
                            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                                LoadSampleIntoGlobalSampler(s);
                            }
                            ImGui::NextColumn();

                            ImGui::TextColored(ImVec4(0.3f, 0.9f, 1.0f, 1.0f), "%s", s.category.c_str()); ImGui::NextColumn();
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%.3fs", s.duration_sec); ImGui::NextColumn();
                            ImGui::TextDisabled("%s", s.filename.c_str()); ImGui::NextColumn();

                            ImGui::PopID();
                        }
                    }
                    ImGui::EndChild();

                    if (selected_sample_idx >= 0 && selected_sample_idx < (int)sample_library.size()) {
                        const auto& s = sample_library[selected_sample_idx];
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), ICON_FA_MUSIC " Sample Ativo: %s (assets\\samples\\%s)", s.name.c_str(), s.filename.c_str());
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "Status: %s", status_feedback.c_str());
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };
}
