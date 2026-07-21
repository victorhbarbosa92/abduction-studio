#pragma once
#include "imgui.h"
#include <string>
#include "../ai/StemSeparationEngine.h"
#include "../core/ClipManager.h"
#include "../audio/SynthEngine.h"
#include "../audio/RecordManager.h"
#include "../audio/OfflineRenderer.h"
#include "PianoRollUI.h"
#include "StepSequencerUI.h"
#include "ModulationPanelUI.h"
#include "GrossBeatUI.h"
#include "KuroDSPUI.h"
#include "KuroWaveUI.h"
#include "KuroSamplerUI.h"
#include <thread>
#include <cstdlib>
#include <atomic>
#include <fstream>
#include <windows.h> // For PlaySoundA
#include <filesystem>
#include "../plugin_manager/ClapWrapper.h"
#include "../plugin_manager/DAG.h"
#include "DawApiExplorerUI.h"

extern void setupAbductionTheme(); // FASE 23

extern bool is_playing;
extern unsigned long long global_sample_count;
extern ClipManager g_clip_manager;
extern KuroAudio::KuroWave g_kurowave;
extern KuroAudio::RecordManager g_record_manager;
extern KuroDSP::AudioGraph master_graph;
extern std::string track_names[MAX_TRACKS];
extern KuroDSP::TimelineManager timeline;
extern std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;
extern float g_master_volume;
extern float track_vu_levels[8];
extern float master_vu_level_l;
extern float master_vu_level_r;

#include "../core/DJEngine.h"
#include "../core/StemExtractorEngine.h"
extern std::unique_ptr<KuroAudio::DJEngine> g_dj_engine;
extern std::unique_ptr<KuroAudio::StemExtractorEngine> g_stem_engine;

namespace ProjectManagerBridge {
    void Save(const std::string& path);
    void Load(const std::string& path);
}

namespace KuroUI {
    extern CommandManager g_command_manager;

    enum class AppMode {
        STUDIO_MODE,
        DJ_MODE,
        STEM_MODE
    };
    static AppMode current_app_mode = AppMode::STUDIO_MODE;

    static int selected_track_idx = 0;
    static bool set_mixer_focus = false;
    static bool show_piano_roll = false;
    static bool show_step_sequencer = false;
    static bool show_modulation_panel = false;
    static bool show_cloud_downloader = false;
    static bool show_gross_beat = false;
    static char cloud_search_query[512] = "";
    static std::string cloud_status = "Pronto. Cole um link ou pesquise...";
    
    // Novas variáveis para Notification Toast & Modal de Download
    static std::atomic<bool> cloud_download_finished{false};
    static std::string cloud_last_downloaded_file = "";
    static bool show_download_toast = false;
    static bool show_download_action_modal = false;
    static float toast_timer = 0.0f;

    // FASE 28: Estado Global de Efeitos Flutuantes e Cadeias por Faixa
    struct FloatingPluginWindow {
        int track_idx;
        std::string plugin_id;
        std::string name;
        bool is_open = true;
        bool is_bypassed = false;
        ImVec2 window_pos = ImVec2(0,0);
        std::shared_ptr<KuroDSP::PluginNode> plugin;
    };
    static std::vector<FloatingPluginWindow> active_plugin_windows;
    
    static std::vector<std::string> flex_packs_cache;
    static bool flex_packs_scanned = false;
    
    bool show_sampler_settings = false;
int active_sampler_channel = 0;
bool show_flex_browser = false;
int active_flex_channel = 3;

static bool show_daw_api_explorer = false;

inline void RenderSamplerSettings(int channel_idx) {
    if (!show_sampler_settings) return;
    
    char win_title[64];
    const char* ch_names[] = { "Kick", "Snare", "HiHat", "Bassline", "Serum Chords", "Lead Synth", "Clap", "Open Hat" };
    snprintf(win_title, sizeof(win_title), "Instrument Sampler: %s (Channel %d)###SamplerSettingsWindow", ch_names[channel_idx], channel_idx);
    
    ImGui::SetNextWindowSize(ImVec2(600, 480), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(win_title, &show_sampler_settings, ImGuiWindowFlags_NoCollapse)) {
        auto& ss = g_piano_synth.sampler_settings[channel_idx];
        
        // Top Header Info
        ImGui::Checkbox("ON", &ss.reverse); // Mock ON checkbox
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::SliderFloat("PAN", &ss.pan, -1.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::SliderFloat("VOL", &ss.vol, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::DragFloat("PITCH", &ss.pitch, 0.1f, -24.0f, 24.0f, "%.1f ST");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(40);
        ImGui::InputInt("RANGE", &ss.pitch_range);
        
        ImGui::Separator();
        
        if (ImGui::BeginTabBar("SamplerTabs")) {
            // TAB 1: WAVEFORM & TIME STRETCHING
            if (ImGui::BeginTabItem("Waveform")) {
                ImGui::Columns(2, "WaveformColumns", false);
                
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Time Stretching");
                ImGui::SliderFloat("Pitch", &ss.time_pitch, -12.0f, 12.0f, "%.1f ST");
                ImGui::SliderFloat("Mul", &ss.time_mul, 0.5f, 2.0f, "%.2fx");
                ImGui::SliderFloat("Time", &ss.time_time, 0.0f, 5.0f, "%.2fs");
                
                const char* modes[] = { "Resample", "Auto", "Stretch" };
                ImGui::Combo("Mode", &ss.time_mode, modes, 3);
                
                ImGui::NextColumn();
                
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Precomputed Effects");
                ImGui::Checkbox("Remove DC offset", &ss.remove_dc);
                ImGui::Checkbox("Normalize", &ss.normalize);
                ImGui::Checkbox("Reverse", &ss.reverse);
                ImGui::Checkbox("Reverse polarity", &ss.rev_polarity);
                
                ImGui::SliderFloat("SMP Start", &ss.smp_start, 0.0f, 1.0f);
                ImGui::SliderFloat("Length", &ss.length, 0.0f, 1.0f);
                ImGui::SliderFloat("In", &ss.fade_in, 0.0f, 1.0f);
                ImGui::SliderFloat("Out", &ss.fade_out, 0.0f, 1.0f);
                
                ImGui::Columns(1);
                ImGui::Separator();
                
                // Draw Waveform Visualizer
                auto& ds = g_piano_synth.getDrumSample(channel_idx);
                if (ds.loaded && ds.total_frames > 0) {
                    ImVec2 start = ImGui::GetCursorScreenPos();
                    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 140);
                    ImGui::InvisibleButton("##waveform_draw", size);
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    dl->AddRectFilled(start, ImVec2(start.x + size.x, start.y + size.y), IM_COL32(18, 20, 24, 255), 4.0f);
                    
                    int step = ds.total_frames / (int)size.x;
                    if (step < 1) step = 1;
                    float half_h = size.y * 0.5f;
                    
                    for (int x = 0; x < (int)size.x; x++) {
                        uint64_t idx = (uint64_t)x * step;
                        if (idx < ds.total_frames) {
                            float val = ds.sample_data[idx * ds.channels];
                            dl->AddLine(
                                ImVec2(start.x + x, start.y + half_h - val * half_h * 0.9f),
                                ImVec2(start.x + x, start.y + half_h + val * half_h * 0.9f),
                                IM_COL32(90, 200, 80, 220)
                            );
                        }
                    }
                    dl->AddText(ImVec2(start.x + 10, start.y + 10), IM_COL32(180, 180, 180, 150), "Sampler Waveform Preview");
                } else {
                    ImGui::Text("Nenhum sample carregado neste canal.");
                }
                
                ImGui::EndTabItem();
            }
            
            // TAB 2: ENVELOPE / LFO / FILTER
            if (ImGui::BeginTabItem("Envelope & Filter")) {
                ImGui::Columns(3, "EnvFilterCols", false);
                
                // Envelope Column
                ImGui::Checkbox("Envelope Enabled", &ss.env_enabled);
                ImGui::SliderFloat("Delay", &ss.env_delay, 0.0f, 2.0f, "%.2fs");
                ImGui::SliderFloat("Attack", &ss.env_attack, 0.001f, 1.0f, "%.3fs");
                ImGui::SliderFloat("Hold", &ss.env_hold, 0.0f, 2.0f, "%.2fs");
                ImGui::SliderFloat("Decay", &ss.env_decay, 0.001f, 2.0f, "%.2fs");
                ImGui::SliderFloat("Sustain", &ss.env_sustain, 0.0f, 1.0f);
                ImGui::SliderFloat("Release", &ss.env_release, 0.001f, 2.0f, "%.2fs");
                
                // Envelope Graph Preview
                ImVec2 estart = ImGui::GetCursorScreenPos();
                ImVec2 esize = ImVec2(150, 70);
                ImGui::InvisibleButton("##env_graph", esize);
                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(estart, ImVec2(estart.x + esize.x, estart.y + esize.y), IM_COL32(20, 22, 26, 255));
                
                // Draw ADSR Envelope Lines
                float att_w = ss.env_attack * 20.0f;
                float dec_w = ss.env_decay * 20.0f;
                float rel_w = ss.env_release * 20.0f;
                float sus_h = esize.y * (1.0f - ss.env_sustain);
                
                ImVec2 p0 = ImVec2(estart.x + 5, estart.y + esize.y - 5);
                ImVec2 p1 = ImVec2(p0.x + att_w, estart.y + 5);
                ImVec2 p2 = ImVec2(p1.x + 10, estart.y + 5); // Hold
                ImVec2 p3 = ImVec2(p2.x + dec_w, estart.y + sus_h);
                ImVec2 p4 = ImVec2(p3.x + 30, estart.y + sus_h); // Sustain duration
                ImVec2 p5 = ImVec2(p4.x + rel_w, estart.y + esize.y - 5);
                
                dl->AddLine(p0, p1, IM_COL32(90, 200, 80, 255), 2.0f);
                dl->AddLine(p1, p2, IM_COL32(90, 200, 80, 255), 2.0f);
                dl->AddLine(p2, p3, IM_COL32(90, 200, 80, 255), 2.0f);
                dl->AddLine(p3, p4, IM_COL32(90, 200, 80, 255), 2.0f);
                dl->AddLine(p4, p5, IM_COL32(90, 200, 80, 255), 2.0f);
                
                ImGui::NextColumn();
                
                // LFO Column
                ImGui::Checkbox("LFO Enabled", &ss.lfo_enabled);
                const char* shapes[] = { "Sine", "Triangle", "Square" };
                ImGui::Combo("LFO Shape", &ss.lfo_shape, shapes, 3);
                ImGui::SliderFloat("LFO Speed", &ss.lfo_speed, 0.1f, 10.0f, "%.1f Hz");
                ImGui::SliderFloat("LFO Amount", &ss.lfo_amount, 0.0f, 1.0f);
                ImGui::SliderFloat("LFO Delay", &ss.lfo_delay, 0.0f, 2.0f);
                
                ImGui::NextColumn();
                
                // Filter Column
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Cutoff & Resonance");
                ImGui::SliderFloat("Cutoff (MOD X)", &ss.filter_cutoff, 0.01f, 1.0f);
                ImGui::SliderFloat("Res (MOD Y)", &ss.filter_res, 0.0f, 1.0f);
                const char* filter_types[] = { "Fast LP", "SVF Lowpass", "Highpass", "Bandpass" };
                ImGui::Combo("Filter Type", &ss.filter_type, filter_types, 4);
                
                ImGui::Columns(1);
                ImGui::EndTabItem();
            }
            
            // TAB 3: MISCELLANEOUS & ECHO DELAY
            if (ImGui::BeginTabItem("Miscellaneous")) {
                ImGui::Columns(3, "MiscCols", false);
                
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Level Adjustments");
                ImGui::SliderFloat("Pan Offset", &ss.misc_pan, -1.0f, 1.0f);
                ImGui::SliderFloat("Vol Offset", &ss.misc_vol, 0.0f, 2.0f);
                
                ImGui::NextColumn();
                
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Polyphony");
                ImGui::SliderInt("Max Poly", &ss.poly_max, 1, 64);
                ImGui::Checkbox("Mono", &ss.poly_mono);
                ImGui::Checkbox("Portamento", &ss.poly_porta);
                
                ImGui::NextColumn();
                
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Arpeggiator");
                const char* arp_dirs[] = { "Off", "Up", "Down", "Up-Down", "Random" };
                ImGui::Combo("Direction", &ss.arp_direction, arp_dirs, 5);
                ImGui::SliderFloat("Arp Time", &ss.arp_time, 0.05f, 1.0f);
                ImGui::SliderFloat("Arp Gate", &ss.arp_gate, 0.05f, 1.0f);
                ImGui::InputInt("Arp Range", &ss.arp_range);
                
                ImGui::Columns(1);
                ImGui::Separator();
                
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Echo Delay / Fat Mode");
                ImGui::SliderFloat("Feedback", &ss.echo_feed, 0.0f, 0.95f);
                ImGui::SliderFloat("Echo Time", &ss.echo_time, 0.05f, 2.0f);
                ImGui::SliderInt("Echo Count", &ss.echo_count, 1, 8);
                ImGui::Checkbox("Ping Pong", &ss.echo_pingpong);
                ImGui::Checkbox("Fat Mode", &ss.echo_fat);
                
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

inline void RenderFlexPresetBrowser(int channel_idx) {
    if (!show_flex_browser) return;
    
    char win_title[64];
    const char* ch_names[] = { "Kick", "Snare", "HiHat", "Bassline", "Serum Chords", "Lead Synth", "Clap", "Open Hat" };
    snprintf(win_title, sizeof(win_title), "FLEX Synth Preset Browser: %s (Channel %d)###FlexPresetWindow", ch_names[channel_idx], channel_idx);
    
    ImGui::SetNextWindowSize(ImVec2(800, 520), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(win_title, &show_flex_browser, ImGuiWindowFlags_NoCollapse)) {
        auto& fs = g_piano_synth.flex_settings[channel_idx];
        
        // --- 1. PAINEL ESQUERDO: PACKS (Largura 200px) ---
        ImGui::BeginChild("PacksPanel", ImVec2(200, 0), true);
        
        ImGui::TextColored(ImVec4(0.85f, 0.45f, 0.15f, 1.0f), "PACKS");
        ImGui::Separator();
        
        const char* packs[] = { "Arksun Cityscape", "General Midi Library", "Mobile Synth Pluck", "Mobile Tuned 808 Bass", "Olbaid Compendium" };
        for (int i = 0; i < 5; i++) {
            bool selected = (fs.selected_pack == i);
            if (ImGui::Selectable(packs[i], selected)) {
                fs.selected_pack = i;
                fs.selected_preset = 0; // Reset active preset when pack changes
            }
        }
        
        // Album art preview at the bottom left
        float packs_panel_h = ImGui::GetWindowHeight();
        ImGui::SetCursorPosY(packs_panel_h - 140);
        ImVec2 art_start = ImGui::GetCursorScreenPos();
        ImVec2 art_size = ImVec2(180, 100);
        ImGui::InvisibleButton("##album_art", art_size);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(art_start, ImVec2(art_start.x + art_size.x, art_start.y + art_size.y), IM_COL32(35, 40, 50, 255), 4.0f);
        dl->AddRect(art_start, ImVec2(art_start.x + art_size.x, art_start.y + art_size.y), IM_COL32(230, 130, 40, 255), 4.0f, 0, 1.5f);
        
        char pack_label[64];
        snprintf(pack_label, sizeof(pack_label), "%s", packs[fs.selected_pack]);
        dl->AddText(ImVec2(art_start.x + 10, art_start.y + 10), IM_COL32(230, 130, 40, 255), "FLEX PACK");
        dl->AddText(ImVec2(art_start.x + 10, art_start.y + 40), IM_COL32(240, 240, 240, 255), pack_label);
        
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // --- 2. PAINEL CENTRAL: PRESETS (Largura 180px) ---
        ImGui::BeginChild("PresetsPanel", ImVec2(180, 0), true);
        
        // Dynamic presets per pack
        const char* presets_arksun[] = { "70s Bounce", "7th Soul", "80s Theatre", "Alumin Sun", "AmbiClav", "Bite Me", "Black Sting", "Blue Cordian", "Funky Electricity" };
        const char* presets_general_midi[] = { "Acoustic Grand Piano", "Bright Acoustic Piano", "Electric Grand Piano", "Honky-tonk Piano", "Electric Piano 1", "Electric Piano 2", "Harpsichord", "Clavi", "Celesta" };
        const char* presets_mobile_pluck[] = { "Analog Pluck 1", "Bell Pluck", "Chiptune Pluck", "FM Pluck", "Glass Pluck", "Metal Pluck", "Plucky Sine", "Short Decay Lead", "Tiny Wave Pluck" };
        const char* presets_tuned_808[] = { "Clean 808 Bass", "Distorted 808", "Deep Sub Bass", "Long Release 808", "Punchy 808", "Sat Sub 808", "Glide 808 Bass", "Slide Sub 808", "Heavy Dist Bass" };
        const char* presets_olbaid[] = { "Cinematic Sweep", "Dark Drone", "Epic Brass Pad", "Lush Wave Pad", "Retro Pad", "Starlight Pad", "Cosmic Pad", "Sci-Fi FX Sweep", "Dreamy Arp Pad" };
        
        const char** active_presets = nullptr;
        int num_presets = 9;
        switch (fs.selected_pack) {
            case 0: active_presets = presets_arksun; break;
            case 1: active_presets = presets_general_midi; break;
            case 2: active_presets = presets_mobile_pluck; break;
            case 3: active_presets = presets_tuned_808; break;
            case 4: active_presets = presets_olbaid; break;
            default: active_presets = presets_arksun; break;
        }
        
        char preset_header[32];
        snprintf(preset_header, sizeof(preset_header), "PRESETS (%d)", num_presets);
        ImGui::TextColored(ImVec4(0.85f, 0.45f, 0.15f, 1.0f), "%s", preset_header);
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 90);
        if (ImGui::Button("Piano Roll", ImVec2(80, 18))) {
            selected_track_idx = channel_idx;
            show_piano_roll = true;
            ImGui::SetWindowFocus("Piano Roll");
        }
        
        ImGui::Separator();
        
        for (int i = 0; i < num_presets; i++) {
            bool selected = (fs.selected_preset == i);
            if (ImGui::Selectable(active_presets[i], selected)) {
                fs.selected_preset = i;
                
                int pitches[] = { 36, 38, 42, 48, 60, 72, 39, 46 };
                KuroAudio::MidiInstrument target_inst = KuroAudio::MidiInstrument::ACOUSTIC_PIANO;
                if (fs.selected_pack == 0) { // Arksun Cityscape
                    KuroAudio::MidiInstrument arksun_insts[] = {
                        KuroAudio::MidiInstrument::ELECTRIC_PIANO,
                        KuroAudio::MidiInstrument::VIBRAPHONE,
                        KuroAudio::MidiInstrument::CHURCH_ORGAN,
                        KuroAudio::MidiInstrument::ACCORDION,
                        KuroAudio::MidiInstrument::ELECTRIC_PIANO,
                        KuroAudio::MidiInstrument::VIBRAPHONE,
                        KuroAudio::MidiInstrument::CHURCH_ORGAN,
                        KuroAudio::MidiInstrument::ACCORDION,
                        KuroAudio::MidiInstrument::ELECTRIC_PIANO
                    };
                    target_inst = arksun_insts[fs.selected_preset];
                } else if (fs.selected_pack == 1) { // General Midi Library
                    KuroAudio::MidiInstrument general_midi_insts[] = {
                        KuroAudio::MidiInstrument::ACOUSTIC_PIANO,
                        KuroAudio::MidiInstrument::ELECTRIC_PIANO,
                        KuroAudio::MidiInstrument::HARPSICHORD,
                        KuroAudio::MidiInstrument::CELESTA,
                        KuroAudio::MidiInstrument::VIBRAPHONE,
                        KuroAudio::MidiInstrument::MARIMBA,
                        KuroAudio::MidiInstrument::NYLON_GUITAR,
                        KuroAudio::MidiInstrument::CELLO,
                        KuroAudio::MidiInstrument::STRING_ENSEMBLE
                    };
                    target_inst = general_midi_insts[fs.selected_preset];
                } else if (fs.selected_pack == 2) { // Mobile Synth Pluck
                    KuroAudio::MidiInstrument pluck_insts[] = {
                        KuroAudio::MidiInstrument::HARPSICHORD,
                        KuroAudio::MidiInstrument::CELESTA,
                        KuroAudio::MidiInstrument::MARIMBA,
                        KuroAudio::MidiInstrument::NYLON_GUITAR,
                        KuroAudio::MidiInstrument::HARPSICHORD,
                        KuroAudio::MidiInstrument::CELESTA,
                        KuroAudio::MidiInstrument::MARIMBA,
                        KuroAudio::MidiInstrument::NYLON_GUITAR,
                        KuroAudio::MidiInstrument::CELESTA
                    };
                    target_inst = pluck_insts[fs.selected_preset];
                } else if (fs.selected_pack == 3) { // Mobile Tuned 808 Bass
                    target_inst = KuroAudio::MidiInstrument::ELECTRIC_BASS;
                } else if (fs.selected_pack == 4) { // Olbaid Compendium
                    KuroAudio::MidiInstrument pad_insts[] = {
                        KuroAudio::MidiInstrument::STRING_ENSEMBLE,
                        KuroAudio::MidiInstrument::CHOIR,
                        KuroAudio::MidiInstrument::VIOLIN,
                        KuroAudio::MidiInstrument::STRING_ENSEMBLE,
                        KuroAudio::MidiInstrument::CHOIR,
                        KuroAudio::MidiInstrument::VIOLIN,
                        KuroAudio::MidiInstrument::STRING_ENSEMBLE,
                        KuroAudio::MidiInstrument::CHOIR,
                        KuroAudio::MidiInstrument::VIOLIN
                    };
                    target_inst = pad_insts[fs.selected_preset];
                }
                g_piano_synth.flex_active[channel_idx] = true;
                g_piano_synth.setInstrument(target_inst);
                g_piano_synth.triggerNote(pitches[channel_idx], 0.5f, 0.8f, channel_idx);
            }
        }
        
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // --- 3. PAINEL DIREITO: CONTROLES DO SINTETIZADOR (Resto da janela) ---
        ImGui::BeginChild("ControlsPanel", ImVec2(0, 0), true);
        
        ImGui::TextColored(ImVec4(0.85f, 0.45f, 0.15f, 1.0f), "SYNTH CONTROLS");
        ImGui::Separator();
        
        // Spectrum Visualizer
        ImVec2 spec_start = ImGui::GetCursorScreenPos();
        ImVec2 spec_size = ImVec2(ImGui::GetContentRegionAvail().x, 80);
        ImGui::InvisibleButton("##flex_spectrum", spec_size);
        
        dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(spec_start, ImVec2(spec_start.x + spec_size.x, spec_start.y + spec_size.y), IM_COL32(20, 20, 25, 255), 4.0f);
        
        float time_val = (float)ImGui::GetTime();
        float current_lvl = ::master_vu_level_l * 2.0f;
        for (int x = 0; x < (int)spec_size.x; x++) {
            float freq = x * 0.08f;
            float val = std::sin(time_val * 12.0f + freq) * 0.35f + std::sin(time_val * 5.0f - freq * 0.7f) * 0.20f;
            val *= (current_lvl * 4.0f + 0.1f);
            val = std::abs(val);
            dl->AddLine(
                ImVec2(spec_start.x + x, spec_start.y + spec_size.y),
                ImVec2(spec_start.x + x, spec_start.y + spec_size.y - val * spec_size.y * 0.85f),
                IM_COL32(230, 130, 40, 200)
            );
        }
        dl->AddText(ImVec2(spec_start.x + 10, spec_start.y + 10), IM_COL32(200, 200, 200, 150), "Active Spectrum");
        
        ImGui::Spacing();
        
        // Macros (Sliders)
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Macros");
        ImGui::SliderFloat("Filter Cutoff", &fs.macro_filter, 0.0f, 1.0f);
        ImGui::SliderFloat("Vibrato", &fs.macro_vibrato, 0.0f, 1.0f);
        ImGui::SliderFloat("Harmonic", &fs.macro_harmonic, 0.0f, 1.0f);
        ImGui::SliderFloat("Reverb Mix", &fs.macro_reverb, 0.0f, 1.0f);
        ImGui::SliderFloat("Delay Mix", &fs.macro_delay, 0.0f, 1.0f);
        
        ImGui::Spacing();
        
        // Filter & Envelope Sub-panels (Side-by-side using sub-children)
        float sub_panel_w = ImGui::GetContentRegionAvail().x * 0.48f;
        
        ImGui::BeginChild("FilterSubPanel", ImVec2(sub_panel_w, 115), false);
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Filter");
        ImGui::Separator();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 60);
        ImGui::SliderFloat("Cutoff", &fs.filter_cutoff, 0.0f, 1.0f);
        ImGui::SliderFloat("Res", &fs.filter_res, 0.0f, 1.0f);
        ImGui::SliderFloat("Env Amt", &fs.filter_env_amt, 0.0f, 1.0f);
        ImGui::PopItemWidth();
        ImGui::EndChild();
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.04f);
        
        ImGui::BeginChild("EnvelopeSubPanel", ImVec2(0, 115), false);
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Volume Envelope");
        ImGui::Separator();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 30);
        ImGui::SliderFloat("A", &fs.env_vol_a, 0.001f, 1.0f);
        ImGui::SliderFloat("D", &fs.env_vol_d, 0.001f, 2.0f);
        ImGui::SliderFloat("S", &fs.env_vol_s, 0.0f, 1.0f);
        ImGui::SliderFloat("R", &fs.env_vol_r, 0.001f, 2.0f);
        ImGui::PopItemWidth();
        ImGui::EndChild();
        
        ImGui::Separator();
        
        // Master Filter & Limiter Row (Bottom)
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Limiter & Master Effects");
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 180);
        ImGui::SliderFloat("Master Filter Cutoff", &fs.master_filter_cutoff, 0.0f, 1.0f);
        ImGui::SliderFloat("Master Filter Res", &fs.master_filter_res, 0.0f, 1.0f);
        ImGui::PopItemWidth();
        
        ImGui::EndChild();
    }
    ImGui::End();
}

static void RenderFlexBrowser() {
        if (!flex_packs_scanned) {
            flex_packs_scanned = true;
            try {
                std::string pack_dir = "C:\\Users\\USUÁRIO\\Downloads\\FL Studio 24.2.1.4526-WD-REV1\\FL Studio 24.2.1.4526-WD-REV1\\FLEX Pack\\Packs";
                if (std::filesystem::exists(pack_dir)) {
                    for (const auto& entry : std::filesystem::directory_iterator(pack_dir)) {
                        if (entry.is_directory()) {
                            flex_packs_cache.push_back(entry.path().filename().string());
                        }
                    }
                }
            } catch(...) {}
        }
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(320, 480), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("FLEX BROWSER", nullptr, ImGuiWindowFlags_NoDocking)) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "  FLEX PACKS DETECTADOS:");
            ImGui::Separator();
            
            if (flex_packs_cache.empty()) {
                ImGui::TextDisabled("   Nenhum pack encontrado ou caminho invalido.");
            } else {
                for (size_t i = 0; i < flex_packs_cache.size(); i++) {
                    ImGui::PushID((int)i);
                    bool is_open = ImGui::TreeNodeEx(flex_packs_cache[i].c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
                    if (is_open) {
                        if (ImGui::Selectable("   > Init Patch (Simulated)")) {}
                        if (ImGui::Selectable("   > Default Bass (Simulated)")) {}
                        if (ImGui::Selectable("   > Cyber Lead (Simulated)")) {}
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    // Cada faixa tem uma lista de plugins dinâmicos
    static std::vector<std::shared_ptr<KuroDSP::PluginNode>> track_fx_chain[MAX_TRACKS]; 
    static int dragging_fx_idx = -1;
    static int dragging_fx_source_track = -1;
    
    // Moved to global in StudioUI.h
} // namespace KuroUI

extern float track_pans[MAX_TRACKS];

namespace KuroUI {

    namespace KuroIcons {
        enum Type { PLAY, PAUSE, STOP, RECORD, DISK, EXPORT, TUTORIAL };
        inline bool Button(const char* id, Type icon, ImVec2 size, bool active = false) {
            bool pressed = ImGui::Button(id, size);
            ImVec2 p_min = ImGui::GetItemRectMin();
            ImVec2 p_max = ImGui::GetItemRectMax();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            
            ImVec2 center = ImVec2((p_min.x + p_max.x) * 0.5f, (p_min.y + p_max.y) * 0.5f);
            float s = (p_max.y - p_min.y) * 0.25f; // half size
            
            ImU32 col = active ? IM_COL32(50, 255, 50, 255) : IM_COL32(200, 200, 200, 255);
            if (icon == RECORD && active) col = IM_COL32(255, 50, 50, 255);
            
            if (icon == PLAY) {
                draw_list->AddTriangleFilled(
                    ImVec2(center.x - s*0.6f, center.y - s),
                    ImVec2(center.x - s*0.6f, center.y + s),
                    ImVec2(center.x + s*0.8f, center.y), col);
            } else if (icon == PAUSE) {
                draw_list->AddRectFilled(ImVec2(center.x - s*0.6f, center.y - s), ImVec2(center.x - s*0.2f, center.y + s), col);
                draw_list->AddRectFilled(ImVec2(center.x + s*0.2f, center.y - s), ImVec2(center.x + s*0.6f, center.y + s), col);
            } else if (icon == STOP) {
                draw_list->AddRectFilled(ImVec2(center.x - s, center.y - s), ImVec2(center.x + s, center.y + s), col);
            } else if (icon == RECORD) {
                col = active ? IM_COL32(255, 50, 50, 255) : IM_COL32(200, 50, 50, 255);
                draw_list->AddCircleFilled(center, s, col);
            } else if (icon == DISK) {
                draw_list->AddRectFilled(ImVec2(center.x - s, center.y - s), ImVec2(center.x + s, center.y + s), IM_COL32(100, 150, 255, 255));
                draw_list->AddRectFilled(ImVec2(center.x - s*0.5f, center.y - s), ImVec2(center.x + s*0.5f, center.y - s*0.2f), IM_COL32(255, 255, 255, 255));
            } else if (icon == EXPORT) {
                draw_list->AddTriangleFilled(ImVec2(center.x, center.y - s), ImVec2(center.x - s, center.y + s*0.2f), ImVec2(center.x + s, center.y + s*0.2f), IM_COL32(255, 200, 50, 255));
                draw_list->AddRectFilled(ImVec2(center.x - s*0.4f, center.y + s*0.2f), ImVec2(center.x + s*0.4f, center.y + s), IM_COL32(255, 200, 50, 255));
            } else if (icon == TUTORIAL) {
                // draw a '?' manually or just a badge
                draw_list->AddCircleFilled(center, s, IM_COL32(50, 150, 255, 255));
                // AddText could be used here but a filled circle is distinct enough as a help icon
            }
            return pressed;
        }
    }

    static void RenderStudioMode(StemSeparationEngine& ai_engine) {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = nullptr;
        const char* ch_track_names[] = { "Kick", "Snare", "HiHat", "Bassline", "Serum Chords", "Lead Synth", "FX 01", "Extra" };
        ImVec2 window_size = ImGui::GetContentRegionAvail();
        float middle_height = window_size.y - 50 - 280;
        if (middle_height < 100) middle_height = 100;
        
        // =============================================
        // PANEL 1: FILES/BROWSER (Left Side)
        // =============================================
        if (ImGui::Begin("Files")) {
            // Search bar
            static char search_buf[128] = "";
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.11f, 0.13f, 1.0f));
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##search", search_buf, sizeof(search_buf));
            ImGui::PopStyleColor();
            ImGui::Spacing();
            
            // Project tree
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.15f, 0.16f, 0.18f, 1.0f));
            
            if (ImGui::TreeNodeEx("[Project]", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::TreeNodeEx("  Bassline")) {
                    ImGui::Selectable("  = SERUM 01");
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("  Chords")) { ImGui::TreePop(); }
                if (ImGui::TreeNode("  Drums")) { ImGui::TreePop(); }
                if (ImGui::TreeNode("  Leads")) { ImGui::TreePop(); }
                if (ImGui::TreeNode("  FX")) { ImGui::TreePop(); }
                ImGui::Selectable("  [Plugin 01]");
                ImGui::Indent(16);
                ImGui::Selectable("= SERUM 01");
                ImGui::Unindent(16);
                ImGui::TreePop();
            }
            
            ImGui::Separator();
            ImGui::TextDisabled("  Kick");
            ImGui::TextDisabled("  Snare");
            ImGui::TextDisabled("  HiHat");
            ImGui::TextDisabled("  Bassline");
            ImGui::TextDisabled("  Serum Chords");
            ImGui::TextDisabled("  Lead Synth");
            ImGui::TextDisabled("  FX 01");
            
            ImGui::PopStyleColor();
        }
        ImGui::End();
        
        // Tabs in left panel also register to same dock
        if (ImGui::Begin("Plugins")) {
            ImGui::TextDisabled("No CLAP found.");
            ImGui::TextDisabled("C:\\Program Files\\Common Files\\CLAP");
        }
        ImGui::End();
        
        if (ImGui::Begin("Samples")) {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.f), "FL Studio 2024 — Sample Browser");
            ImGui::Separator();
            ImGui::Spacing();
            
            // ── SFX Pads ─────────────────────────────────────────────────────
            if (ImGui::CollapsingHeader("  🌀 FX & SFX  ", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent(4.f);
                int cols = std::max(1, (int)(ImGui::GetContentRegionAvail().x / 110));
                int count = 0;
                for (auto& sfx : g_piano_synth.sfx_library) {
                    if (!sfx.sample.loaded) continue;
                    ImGui::PushID(count);
                    bool clicked = ImGui::Button(sfx.name.c_str(), ImVec2(108, 22));
                    if (clicked) {
                        // Trigger SFX preview via piano synth (note 72 = high C)
                        g_piano_synth.triggerNote(72, 2.0f, 0.7f);
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clique para pré-visualizar: %s", sfx.name.c_str());
                    count++;
                    if (count % cols != 0) ImGui::SameLine(0, 4);
                    ImGui::PopID();
                }
                ImGui::Unindent(4.f);
            }
            
            ImGui::Spacing();
            
            // ── Drum Samples por categoria ────────────────────────────────────
            struct DrumCat {
                const char* label;
                int channel;
                const char* (*names)[KuroAudio::SynthEngine::MAX_DRUM_VARIANTS];
            };
            
            if (ImGui::CollapsingHeader("  🥁 Kicks")) {
                for (int v = 0; v < KuroAudio::SynthEngine::MAX_DRUM_VARIANTS; v++) {
                    auto& s = g_piano_synth.drum_variants[0][v];
                    if (!s.loaded) continue;
                    bool is_cur = (g_piano_synth.selected_variant[0] == v);
                    ImGui::PushID(v + 100);
                    ImGui::PushStyleColor(ImGuiCol_Button, is_cur ? ImVec4(0.2f,0.5f,0.2f,1.f) : ImVec4(0.18f,0.20f,0.22f,1.f));
                    if (ImGui::Button(g_piano_synth.kick_variant_names[v], ImVec2(-1, 20))) {
                        g_piano_synth.selected_variant[0] = v;
                        g_piano_synth.triggerNote(36, 0.5f, 0.8f);
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                }
            }
            
            if (ImGui::CollapsingHeader("  🥁 Snares")) {
                for (int v = 0; v < KuroAudio::SynthEngine::MAX_DRUM_VARIANTS; v++) {
                    auto& s = g_piano_synth.drum_variants[1][v];
                    if (!s.loaded) continue;
                    bool is_cur = (g_piano_synth.selected_variant[1] == v);
                    ImGui::PushID(v + 200);
                    ImGui::PushStyleColor(ImGuiCol_Button, is_cur ? ImVec4(0.2f,0.5f,0.2f,1.f) : ImVec4(0.18f,0.20f,0.22f,1.f));
                    if (ImGui::Button(g_piano_synth.snare_variant_names[v], ImVec2(-1, 20))) {
                        g_piano_synth.selected_variant[1] = v;
                        g_piano_synth.triggerNote(38, 0.4f, 0.8f);
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                }
            }
            
            if (ImGui::CollapsingHeader("  🎵 Hi-Hats")) {
                for (int v = 0; v < KuroAudio::SynthEngine::MAX_DRUM_VARIANTS; v++) {
                    auto& s = g_piano_synth.drum_variants[2][v];
                    if (!s.loaded) continue;
                    bool is_cur = (g_piano_synth.selected_variant[2] == v);
                    ImGui::PushID(v + 300);
                    ImGui::PushStyleColor(ImGuiCol_Button, is_cur ? ImVec4(0.2f,0.5f,0.2f,1.f) : ImVec4(0.18f,0.20f,0.22f,1.f));
                    if (ImGui::Button(g_piano_synth.hat_variant_names[v], ImVec2(-1, 20))) {
                        g_piano_synth.selected_variant[2] = v;
                        g_piano_synth.triggerNote(42, 0.2f, 0.6f);
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                }
            }
            
            if (ImGui::CollapsingHeader("  💥 Cymbals & Crashes")) {
                for (int v = 0; v < KuroAudio::SynthEngine::MAX_DRUM_VARIANTS; v++) {
                    auto& s = g_piano_synth.drum_variants[6][v];
                    if (!s.loaded) continue;
                    bool is_cur = (g_piano_synth.selected_variant[6] == v);
                    ImGui::PushID(v + 400);
                    ImGui::PushStyleColor(ImGuiCol_Button, is_cur ? ImVec4(0.2f,0.5f,0.2f,1.f) : ImVec4(0.18f,0.20f,0.22f,1.f));
                    if (ImGui::Button(g_piano_synth.crash_variant_names[v], ImVec2(-1, 20))) {
                        g_piano_synth.selected_variant[6] = v;
                        g_piano_synth.triggerNote(49, 1.0f, 0.7f);
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                }
            }
        }
        ImGui::End();
        
        // FLEX Browser (shows real FL Studio FLEX packs if found)
        RenderFlexBrowser();
        

        // The old simplified Piano Roll was removed. We now use the premium PianoRollUI.h version.
        
        if (ImGui::Begin("Project Settings")) {
            ImGui::TextColored(ImVec4(0.6f,0.6f,0.6f,1.f), "Project Settings");
        }
        ImGui::End();

        // =============================================
        // MASTER FADER (Right side panel)
        // =============================================
        if (ImGui::Begin("Master")) {
            ImDrawList* dl2 = ImGui::GetWindowDrawList();
            ImVec2 avail2 = ImGui::GetContentRegionAvail();
            ImVec2 sp2 = ImGui::GetCursorScreenPos();
            
            // VU Meter
            float mt = sp2.y + 20;
            float mb = mt + avail2.y * 0.4f;
            float mlx = sp2.x + avail2.x * 0.25f;
            float mrx = sp2.x + avail2.x * 0.55f;
            float bar_w2 = 10.0f;
            
            static float mv_lvl_l = 0.0f;
            static float mv_lvl_r = 0.0f;
            static float mv_peak_l = 0.0f;
            static float mv_peak_r = 0.0f;

            mv_lvl_l = ::master_vu_level_l * 2.0f;
            mv_lvl_r = ::master_vu_level_r * 2.0f;

            mv_lvl_l = std::max(0.01f, std::min(mv_lvl_l, 1.0f));
            mv_lvl_r = std::max(0.01f, std::min(mv_lvl_r, 1.0f));

            if (mv_lvl_l > mv_peak_l) mv_peak_l = mv_lvl_l;
            else mv_peak_l = std::max(mv_lvl_l, mv_peak_l - 0.005f);

            if (mv_lvl_r > mv_peak_r) mv_peak_r = mv_lvl_r;
            else mv_peak_r = std::max(mv_lvl_r, mv_peak_r - 0.005f);
            
            dl2->AddRectFilled(ImVec2(mlx, mt), ImVec2(mlx+bar_w2, mb), IM_COL32(25,30,25,255));
            dl2->AddRectFilled(ImVec2(mlx, mt + (mb-mt)*(1.f-mv_lvl_l)), ImVec2(mlx+bar_w2, mb), IM_COL32(90,195,80,255));
            dl2->AddLine(ImVec2(mlx, mt+(mb-mt)*(1.f-mv_peak_l)), ImVec2(mlx+bar_w2, mt+(mb-mt)*(1.f-mv_peak_l)), IM_COL32(220,255,200,255), 2.f);
            dl2->AddRectFilled(ImVec2(mrx, mt), ImVec2(mrx+bar_w2, mb), IM_COL32(25,30,25,255));
            dl2->AddRectFilled(ImVec2(mrx, mt + (mb-mt)*(1.f-mv_lvl_r)), ImVec2(mrx+bar_w2, mb), IM_COL32(90,195,80,255));
            dl2->AddLine(ImVec2(mrx, mt+(mb-mt)*(1.f-mv_peak_r)), ImVec2(mrx+bar_w2, mt+(mb-mt)*(1.f-mv_peak_r)), IM_COL32(220,255,200,255), 2.f);
            dl2->AddText(ImVec2(sp2.x+2, sp2.y+4), IM_COL32(160,165,175,255), "Master");
            
            // Master fader
            float ft = mb + 8;
            float fb = ft + avail2.y * 0.4f;
            float fdr_x = sp2.x + avail2.x/2 - 3;
            dl2->AddRectFilled(ImVec2(fdr_x, ft), ImVec2(fdr_x+6, fb), IM_COL32(30,32,38,255), 2.f);
            float fh = ft + (fb-ft)*(1.f-g_master_volume) - 8;
            ImGui::SetCursorScreenPos(ImVec2(sp2.x+2, fh));
            ImGui::InvisibleButton("##mfdr", ImVec2(avail2.x-4, 16));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
                g_master_volume -= ImGui::GetIO().MouseDelta.y / (fb - ft);
                g_master_volume = std::max(0.f, std::min(g_master_volume, 1.f));
            }
            dl2->AddRectFilled(ImVec2(sp2.x+4, fh), ImVec2(sp2.x+avail2.x-4, fh+16),
                ImGui::IsItemHovered() ? IM_COL32(80,90,105,255) : IM_COL32(60,65,78,255), 2.f);
            dl2->AddLine(ImVec2(sp2.x+4, fh+8), ImVec2(sp2.x+avail2.x-4, fh+8), IM_COL32(100,108,122,255), 1.5f);
            dl2->AddText(ImVec2(sp2.x+2, fb+6), IM_COL32(120,125,135,255), "M");
        }
        ImGui::End();

        if (ImGui::Begin("Mixer Panel")) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 avail = ImGui::GetContentRegionAvail();
            
            // Mixer title tabs
            if (ImGui::BeginTabBar("##mixer_tabs")) {
                if (ImGui::BeginTabItem("Mixer Panel")) {
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            
            const char* channel_names[] = {"Master", "1: Kick", "2: Snare", "3: Hat", "4: Bass", "5: Chord", "6: Lead", "7: FX 01", "8: Extra"};
            ImVec4 ch_colors[] = {
                ImVec4(0.5f, 0.5f, 0.5f, 1.f),
                ImVec4(0.85f, 0.25f, 0.25f, 1.f),
                ImVec4(0.85f, 0.85f, 0.25f, 1.f),
                ImVec4(0.25f, 0.75f, 0.25f, 1.f),
                ImVec4(0.25f, 0.25f, 0.85f, 1.f),
                ImVec4(0.65f, 0.25f, 0.85f, 1.f),
                ImVec4(0.85f, 0.55f, 0.25f, 1.f),
                ImVec4(0.25f, 0.85f, 0.85f, 1.f),
                ImVec4(0.5f, 0.5f, 0.85f, 1.f),
            };
            int num_ch = 9;
            float strip_w = (avail.x - 16) / num_ch;
            if (strip_w < 55) strip_w = 55;
            float meter_h = avail.y * 0.28f;
            float fader_h = avail.y * 0.28f;
            float eq_h = avail.y * 0.20f;
            
            ImGui::BeginChild("##mixer_strips", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
            ImVec2 strip_origin = ImGui::GetCursorScreenPos();
            
            for (int ch = 0; ch < num_ch; ch++) {
                ImGui::PushID(ch);
                ImVec2 sp = ImVec2(strip_origin.x + ch * (strip_w + 2), strip_origin.y);
                
                // Strip background
                bool is_selected_ch = (ch == selected_track_idx || (ch == 0));
                ImU32 strip_bg = is_selected_ch ? IM_COL32(22,25,22,255) : IM_COL32(18,19,22,255);
                dl->AddRectFilled(sp, ImVec2(sp.x + strip_w, sp.y + avail.y - 4), strip_bg);
                dl->AddRect(sp, ImVec2(sp.x + strip_w, sp.y + avail.y - 4), IM_COL32(40,42,48,255));
                
                // Channel name at top
                dl->AddText(ImVec2(sp.x + 3, sp.y + 3), IM_COL32(160,165,175,255), channel_names[ch]);
                
                // VU Meters — animated when playing
                float meter_top = sp.y + 22;
                float meter_bottom = meter_top + meter_h;
                float meter_cx = sp.x + strip_w / 2;
                float bar_w = 6.0f;
                float gap = 3.0f;
                
                // Animated meter levels (real peak tracking)
                static float meter_lvl[9] = { 0.0f };
                static float meter_peak[9] = { 0.0f };
                
                float raw_lvl = 0.0f;
                if (ch == 0) {
                    raw_lvl = (::master_vu_level_l + ::master_vu_level_r) * 0.5f * 2.0f;
                } else if (ch - 1 < 8) {
                    raw_lvl = ::track_vu_levels[ch - 1] * 2.5f;
                }
                
                meter_lvl[ch] = std::max(0.01f, std::min(raw_lvl, 1.0f));
                if (meter_lvl[ch] > meter_peak[ch]) {
                    meter_peak[ch] = meter_lvl[ch];
                } else {
                    meter_peak[ch] = std::max(meter_lvl[ch], meter_peak[ch] - 0.005f);
                }

                float lvl = (ch > 0 && ::track_mutes[ch - 1]) ? 0.0f : meter_lvl[ch];
                
                // Left meter
                float lx = meter_cx - bar_w - gap;
                dl->AddRectFilled(ImVec2(lx, meter_top), ImVec2(lx + bar_w, meter_bottom), IM_COL32(25,30,25,255));
                dl->AddRectFilled(ImVec2(lx, meter_top + meter_h * (1.0f - lvl)), ImVec2(lx + bar_w, meter_bottom), IM_COL32(90, 195, 80, 255));
                // Peak indicator
                float peak_y = meter_top + meter_h * (1.0f - meter_peak[ch]);
                dl->AddLine(ImVec2(lx, peak_y), ImVec2(lx + bar_w, peak_y), IM_COL32(200,255,180,255), 1.5f);
                
                // Right meter
                float rx = meter_cx + gap;
                dl->AddRectFilled(ImVec2(rx, meter_top), ImVec2(rx + bar_w, meter_bottom), IM_COL32(25,30,25,255));
                dl->AddRectFilled(ImVec2(rx, meter_top + meter_h * (1.0f - lvl * 0.9f)), ImVec2(rx + bar_w, meter_bottom), IM_COL32(90, 195, 80, 255));
                dl->AddLine(ImVec2(rx, peak_y), ImVec2(rx + bar_w, peak_y), IM_COL32(200,255,180,255), 1.5f);
                
                // dB labels on first strip
                if (ch == 0) {
                    const char* db_labels[] = {"0", "-10", "-20", "-30", "-40"};
                    for (int di = 0; di < 5; di++) {
                        float dy = meter_top + meter_h * (di / 4.0f);
                        dl->AddText(ImVec2(sp.x + strip_w + 2, dy - 6), IM_COL32(100,100,110,255), db_labels[di]);
                    }
                }
                
                // EQ Curve placeholder
                float eq_top = meter_bottom + 4;
                dl->AddRectFilled(ImVec2(sp.x + 2, eq_top), ImVec2(sp.x + strip_w - 2, eq_top + eq_h), IM_COL32(15,17,20,255));
                // Draw a simple EQ curve
                float eq_step = (strip_w - 4) / 10.0f;
                float eq_pts_y[] = {0.5f, 0.4f, 0.45f, 0.35f, 0.3f, 0.4f, 0.5f, 0.45f, 0.4f, 0.5f, 0.5f};
                for (int ei = 0; ei < 10; ei++) {
                    float ex0 = sp.x + 2 + ei * eq_step;
                    float ex1 = ex0 + eq_step;
                    float ey0 = eq_top + eq_h * eq_pts_y[ei];
                    float ey1 = eq_top + eq_h * eq_pts_y[ei+1];
                    dl->AddLine(ImVec2(ex0, ey0), ImVec2(ex1, ey1), IM_COL32(80, 180, 70, 200), 1.5f);
                }
                dl->AddText(ImVec2(sp.x + 3, eq_top + eq_h - 14), IM_COL32(100,105,115,255), "EQ");
                
                // Mute/Solo buttons
                float btn_y = eq_top + eq_h + 4;
                float btn_h = 16.0f;
                float btn_w = (strip_w - 8) / 2;
                
                // Mute
                ImGui::SetCursorScreenPos(ImVec2(sp.x + 2, btn_y));
                bool muted = (ch == 0) ? false : ::track_mutes[ch - 1];
                ImGui::PushStyleColor(ImGuiCol_Button, muted ? ImVec4(0.7f,0.1f,0.1f,1.f) : ImVec4(0.2f,0.2f,0.23f,1.f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f,0.7f,0.75f,1.f));
                if (ImGui::Button("Mute", ImVec2(btn_w, btn_h)) && ch > 0) ::track_mutes[ch - 1] = !::track_mutes[ch - 1];
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 2);
                
                bool soloed = (ch == 0) ? false : ::track_solos[ch - 1];
                ImGui::PushStyleColor(ImGuiCol_Button, soloed ? ImVec4(0.65f,0.65f,0.1f,1.f) : ImVec4(0.2f,0.2f,0.23f,1.f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f,0.7f,0.75f,1.f));
                if (ImGui::Button("Solo", ImVec2(btn_w, btn_h)) && ch > 0) ::track_solos[ch - 1] = !::track_solos[ch - 1];
                ImGui::PopStyleColor(2);
                
                // FL-style sub-labels ("Mts", "Iln")
                ImVec2 after_btn = ImVec2(sp.x + 2, btn_y + btn_h + 2);
                dl->AddText(ImVec2(sp.x + 2, btn_y + btn_h + 2), IM_COL32(80,85,90,255), "Mts");
                ImGui::SameLine(0,2);
                dl->AddText(ImVec2(sp.x + btn_w + 6, btn_y + btn_h + 2), IM_COL32(80,85,90,255), "Iln");
                
                // Pan knob area
                float knob_y = btn_y + btn_h + 18;
                float knob_cx = sp.x + strip_w / 2;
                float knob_r = 10.0f;
                dl->AddCircle(ImVec2(knob_cx, knob_y + knob_r), knob_r, IM_COL32(50,55,60,255), 16, 2.0f);
                dl->AddLine(ImVec2(knob_cx, knob_y + 2), ImVec2(knob_cx, knob_y + knob_r - 2), IM_COL32(100, 200, 90, 255), 2.0f);
                
                // Volume fader — interactive drag
                float fader_top = knob_y + knob_r * 2 + 6;
                float fader_bottom = fader_top + fader_h;
                float fader_x = sp.x + strip_w / 2 - 3;
                static float fader_vals[9] = {0.80f, 0.75f, 0.75f, 0.70f, 0.75f, 0.70f, 0.72f, 0.68f, 0.70f};
                
                // Fader track
                dl->AddRectFilled(ImVec2(fader_x, fader_top), ImVec2(fader_x + 6, fader_bottom), IM_COL32(30,32,38,255), 2.0f);
                dl->AddLine(ImVec2(fader_x + 3, fader_top), ImVec2(fader_x + 3, fader_bottom), IM_COL32(60,65,75,255), 2.0f);
                
                // Fader handle (draggable)
                float fader_val = (ch == 0) ? g_master_volume : fader_vals[ch];
                float fh_y = fader_top + fader_h * (1.0f - fader_val) - 8;
                ImGui::SetCursorScreenPos(ImVec2(sp.x + 2, fh_y));
                ImGui::InvisibleButton(("##fdr" + std::to_string(ch)).c_str(), ImVec2(strip_w - 6, 16));
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
                    float delta = ImGui::GetIO().MouseDelta.y;
                    fader_val -= delta / fader_h;
                    fader_val = std::max(0.0f, std::min(fader_val, 1.0f));
                    if (ch == 0) {
                        g_master_volume = fader_val;
                    } else {
                        fader_vals[ch] = fader_val;
                        int trk = ch - 1;
                        if (trk >= 0 && trk < MAX_TRACKS) {
                            ::track_volumes[trk] = (fader_val <= 0.001f) ? -60.0f : 20.0f * std::log10(fader_val);
                        }
                    }
                }
                dl->AddRectFilled(ImVec2(sp.x + 4, fh_y), ImVec2(sp.x + strip_w - 4, fh_y + 16),
                    ImGui::IsItemHovered() ? IM_COL32(75,82,95,255) : IM_COL32(55,60,70,255), 2.0f);
                dl->AddLine(ImVec2(sp.x + 4, fh_y + 8), ImVec2(sp.x + strip_w - 4, fh_y + 8), IM_COL32(90,98,112,255), 1.5f);
                
                // Channel number at bottom
                char ch_num[8];
                snprintf(ch_num, sizeof(ch_num), ch == 0 ? "M" : "%d", ch);
                dl->AddText(ImVec2(sp.x + strip_w/2 - 4, fader_bottom + 4), IM_COL32(110,115,125,255), ch_num);
                
                ImGui::PopID();
            }
            
            ImGui::Dummy(ImVec2(num_ch * (strip_w + 2), avail.y));
            ImGui::EndChild();
        }
        ImGui::End();
        
        // =============================================
        // PANEL 5: PLAYLIST (Bottom)
        // =============================================
        ImVec4 track_colors[MAX_TRACKS] = {
            ImVec4(0.8f, 0.2f, 0.2f, 1.0f), ImVec4(0.2f, 0.8f, 0.2f, 1.0f),
            ImVec4(0.2f, 0.2f, 0.8f, 1.0f), ImVec4(0.8f, 0.8f, 0.2f, 1.0f),
            ImVec4(0.2f, 0.8f, 0.8f, 1.0f), ImVec4(0.8f, 0.2f, 0.8f, 1.0f),
            ImVec4(0.5f, 0.5f, 0.5f, 1.0f), ImVec4(0.3f, 0.7f, 0.4f, 1.0f),
        };
        
        if (ImGui::Begin("[Playlist]")) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            
            // Cores das faixas (Tons cibernéticos e alienígenas)
            ImVec4 timeline_track_colors[MAX_TRACKS] = {
                ImVec4(0.8f, 0.2f, 0.2f, 1.0f), // KICK/BASS
                ImVec4(0.2f, 0.8f, 0.2f, 1.0f), // LEADS
                ImVec4(0.2f, 0.2f, 0.8f, 1.0f), // VOX
                ImVec4(0.8f, 0.8f, 0.2f, 1.0f), // FX
                ImVec4(0.2f, 0.8f, 0.8f, 1.0f), // DRUMS
                ImVec4(0.8f, 0.2f, 0.8f, 1.0f), // SYNTH
                ImVec4(0.5f, 0.5f, 0.5f, 1.0f), // PADS
                ImVec4(0.3f, 0.7f, 0.4f, 1.0f),  // EXTRA
                ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
                ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
                ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
                ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
                ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
                ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f)
            };

            float header_width = 180.0f;
            ImGui::BeginChild("TimelineContent", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
            
            static float pixels_per_second = 15.0f;
            ImGuiIO& io = ImGui::GetIO();
            if (ImGui::IsWindowHovered() && io.KeyCtrl && io.MouseWheel != 0.0f) {
                pixels_per_second += io.MouseWheel * 1.5f;
                if (pixels_per_second < 4.0f) pixels_per_second = 4.0f;
                if (pixels_per_second > 200.0f) pixels_per_second = 200.0f;
            }
            int num_tracks = 8;
            float track_height = 55.0f;
            
            float playhead_x_offset = ((float)timeline.getMasterFrame() / 44100.0f) * pixels_per_second;

            float total_timeline_width = ImGui::GetWindowWidth();
            if (playhead_x_offset + header_width + 500.0f > total_timeline_width) {
                total_timeline_width = playhead_x_offset + header_width + 500.0f;
            }
            for (int i = 0; i < MAX_TRACKS; i++) {
                for (const auto& clip : g_clip_manager.getClips(i)) {
                    float clip_end = (clip.start_time_sec + clip.length_sec) * pixels_per_second;
                    if (clip_end + header_width + 500.0f > total_timeline_width) {
                        total_timeline_width = clip_end + header_width + 500.0f;
                    }
                }
                for (const auto& clip : g_clip_manager.getMidiClips(i)) {
                    float clip_end = (clip.start_time_sec + clip.length_sec) * pixels_per_second;
                    if (clip_end + header_width + 500.0f > total_timeline_width) {
                        total_timeline_width = clip_end + header_width + 500.0f;
                    }
                }
            }

            // --- Régua da Timeline ---
            ImVec2 ruler_pos = ImGui::GetCursorScreenPos();
            float ruler_height = 20.0f;
            
            draw_list->AddRectFilled(ruler_pos, ImVec2(ruler_pos.x + total_timeline_width, ruler_pos.y + ruler_height), IM_COL32(40, 40, 45, 255));
            draw_list->AddLine(ImVec2(ruler_pos.x, ruler_pos.y + ruler_height), ImVec2(ruler_pos.x + total_timeline_width, ruler_pos.y + ruler_height), IM_COL32(60, 60, 70, 255), 1.0f);
            
            float scroll_x = ImGui::GetScrollX();
            float window_width = ImGui::GetWindowWidth();
            float tick_spacing = 6.0f * pixels_per_second;
            
            int start_tick = (int)(scroll_x / tick_spacing);
            int end_tick = (int)((scroll_x + window_width) / tick_spacing) + 2;
            
            float visible_min_x = ImGui::GetWindowPos().x + header_width + 10;
            float visible_max_x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
            
            draw_list->PushClipRect(ImVec2(visible_min_x, ruler_pos.y), ImVec2(visible_max_x, ruler_pos.y + ruler_height), true);
            for (int t = start_tick; t <= end_tick; t++) {
                float tx = ruler_pos.x + header_width + t * tick_spacing;
                if (t % 5 == 0) {
                    draw_list->AddLine(ImVec2(tx, ruler_pos.y + 4.0f), ImVec2(tx, ruler_pos.y + ruler_height), IM_COL32(200, 200, 200, 255), 1.5f);
                    char label[16];
                    snprintf(label, sizeof(label), "%d", t);
                    draw_list->AddText(ImVec2(tx + 4, ruler_pos.y + 2), IM_COL32(200, 200, 200, 255), label);
                } else {
                    draw_list->AddLine(ImVec2(tx, ruler_pos.y + 12.0f), ImVec2(tx, ruler_pos.y + ruler_height), IM_COL32(120, 120, 120, 255), 1.0f);
                }
            }
            draw_list->PopClipRect();
            
            ImVec2 header_ruler_min = ImVec2(ruler_pos.x + scroll_x, ruler_pos.y);
            ImVec2 header_ruler_max = ImVec2(header_ruler_min.x + header_width, ruler_pos.y + ruler_height);
            draw_list->AddRectFilled(header_ruler_min, header_ruler_max, IM_COL32(25, 25, 30, 255));
            draw_list->AddLine(ImVec2(header_ruler_max.x, header_ruler_min.y), ImVec2(header_ruler_max.x, header_ruler_max.y), IM_COL32(60, 60, 70, 255), 2.0f);
            
            ImGui::Dummy(ImVec2(total_timeline_width, ruler_height));

            for (int i = 0; i < num_tracks; i++) {
                ImGui::PushID(i);
                ImVec2 p_min = ImGui::GetCursorScreenPos();
                ImVec2 p_max_track = ImVec2(p_min.x + total_timeline_width, p_min.y + track_height);
                
                draw_list->AddRectFilled(p_min, p_max_track, selected_track_idx == i ? IM_COL32(30, 40, 30, 255) : IM_COL32(15, 15, 18, 255));
                draw_list->AddRect(p_min, p_max_track, IM_COL32(40, 40, 50, 255));
                
                ImVec2 header_p_min = ImVec2(p_min.x + scroll_x, p_min.y);
                ImVec2 header_p_max = ImVec2(header_p_min.x + header_width, p_min.y + track_height);
                
                draw_list->AddRectFilled(header_p_min, header_p_max, IM_COL32(25, 25, 30, 255));
                draw_list->AddLine(ImVec2(header_p_max.x, header_p_min.y), ImVec2(header_p_max.x, header_p_max.y), IM_COL32(60, 60, 70, 255), 2.0f);
                draw_list->AddRectFilled(header_p_min, ImVec2(header_p_min.x + 6, header_p_max.y), ImColor(timeline_track_colors[i]));
                
                ImGui::SetCursorScreenPos(ImVec2(header_p_min.x + 12, p_min.y + 5));
                if (ImGui::Selectable((track_names[i] + "##sel" + std::to_string(i)).c_str(), selected_track_idx == i, 0, ImVec2(100, 15))) {
                    selected_track_idx = i;
                }
                
                ImGui::SetCursorScreenPos(ImVec2(header_p_min.x + 12, p_min.y + 28));
                if (::track_mutes[i]) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                }
                if (ImGui::Button((std::string("M##m") + std::to_string(i)).c_str(), ImVec2(24, 20))) { ::track_mutes[i] = !::track_mutes[i]; }
                ImGui::PopStyleColor();
                
                ImGui::SameLine();
                if (::track_solos[i]) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.2f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                }
                if (ImGui::Button((std::string("S##s") + std::to_string(i)).c_str(), ImVec2(24, 20))) { ::track_solos[i] = !::track_solos[i]; }
                ImGui::PopStyleColor();

                ImU32 col = ImColor(timeline_track_colors[i]);
                float mid_y = p_min.y + (track_height / 2);
                float start_x = p_min.x + header_width + 10;
                
                const auto& wave_data = ai_engine.getWaveformOverview(i);
                auto clips = g_clip_manager.getClips(i);
                float screen_view_min_x = p_min.x + scroll_x + header_width;
                float screen_view_max_x = p_min.x + scroll_x + ImGui::GetWindowWidth();
                
                for (size_t c_idx = 0; c_idx < clips.size(); c_idx++) {
                    const auto& clip = clips[c_idx];
                    float clip_start_x = start_x + (clip.start_time_sec * pixels_per_second);
                    float clip_width = clip.length_sec * pixels_per_second;
                    float clip_end_x = clip_start_x + clip_width;
                    
                    if (clip_end_x > screen_view_min_x && clip_start_x < screen_view_max_x) {
                        float draw_start = std::max(clip_start_x, screen_view_min_x);
                        float draw_end = std::min(clip_end_x, screen_view_max_x);
                        
                        ImU32 fill_col = ImColor(timeline_track_colors[i].x, timeline_track_colors[i].y, timeline_track_colors[i].z, clip.is_selected ? 0.4f : 0.15f);
                        ImU32 outline_col = ImColor(timeline_track_colors[i].x, timeline_track_colors[i].y, timeline_track_colors[i].z, 0.8f);
                        
                        draw_list->AddRectFilled(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), fill_col, 4.0f);
                        draw_list->AddRect(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), outline_col, 4.0f);
                        
                        if (wave_data.size() > 0) {
                            float source_px_start = clip.source_offset_sec * pixels_per_second;
                            for (float px = 0; px < clip_width; px++) {
                                float point_x = clip_start_x + px;
                                if (point_x >= draw_start && point_x <= draw_end) {
                                    size_t wave_idx = (size_t)(source_px_start + px);
                                    if (wave_idx < wave_data.size()) {
                                        float amp = wave_data[wave_idx] * ((track_height / 2) - 4); 
                                        draw_list->AddLine(ImVec2(point_x, mid_y - amp), ImVec2(point_x, mid_y + amp), col, 1.0f);
                                    }
                                }
                            }
                        }
                        
                        ImGui::SetCursorScreenPos(ImVec2(draw_start, p_min.y));
                        ImGui::InvisibleButton("##clip_btn", ImVec2(draw_end - draw_start, track_height));
                    }
                }
                
                auto mclips = g_clip_manager.getMidiClips(i);
                for (size_t c_idx = 0; c_idx < mclips.size(); c_idx++) {
                    const auto& clip = mclips[c_idx];
                    float clip_start_x = start_x + (clip.start_time_sec * pixels_per_second);
                    float clip_width = clip.length_sec * pixels_per_second;
                    float clip_end_x = clip_start_x + clip_width;
                    
                    if (clip_end_x > screen_view_min_x && clip_start_x < screen_view_max_x) {
                        float draw_start = std::max(clip_start_x, screen_view_min_x);
                        float draw_end = std::min(clip_end_x, screen_view_max_x);
                        
                        ImU32 fill_col = ImColor(timeline_track_colors[i].x * 0.8f, timeline_track_colors[i].y * 0.8f, timeline_track_colors[i].z * 0.8f, clip.is_selected ? 0.6f : 0.25f);
                        ImU32 outline_col = ImColor(timeline_track_colors[i].x, timeline_track_colors[i].y, timeline_track_colors[i].z, 1.0f);
                        
                        draw_list->AddRectFilled(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), fill_col, 4.0f);
                        draw_list->AddRect(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), outline_col, 4.0f);
                        
                        draw_list->PushClipRect(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), true);
                        Pattern* p = nullptr;
                        for(auto& pat : g_clip_manager.global_patterns) {
                            if(pat.id == clip.pattern_id) { p = &pat; break; }
                        }
                        if(p) {
                            draw_list->AddText(ImVec2(draw_start + 4, p_min.y + 4), IM_COL32(255,255,255,255), p->name.c_str());
                            for (const auto& n : p->notes) {
                                float n_x = clip_start_x + (n.start_time * pixels_per_second);
                                float n_w = n.duration * pixels_per_second;
                                float n_y = p_min.y + track_height - 6.0f - ((n.pitch / 127.0f) * (track_height - 24.0f));
                                draw_list->AddRectFilled(ImVec2(n_x, n_y - 2), ImVec2(n_x + n_w, n_y + 2), IM_COL32(255, 255, 255, 200));
                            }
                        }
                        draw_list->PopClipRect();
                        
                        ImGui::SetCursorScreenPos(ImVec2(draw_start, p_min.y));
                        ImGui::PushID(clip.id);
                        ImGui::InvisibleButton("##mclip_btn", ImVec2(draw_end - draw_start, track_height));
                        ImGui::PopID();
                    }
                }

                // Interactive placing/removing clips inside playlist tracks
                ImGui::SetCursorScreenPos(ImVec2(p_min.x + header_width, p_min.y));
                ImGui::InvisibleButton(("##plgrid" + std::to_string(i)).c_str(), ImVec2(total_timeline_width - header_width, track_height));
                if (ImGui::IsItemHovered()) {
                    ImVec2 mouse_pos = ImGui::GetMousePos();
                    float click_x = mouse_pos.x - (p_min.x + header_width);
                    float click_time_sec = click_x / pixels_per_second;
                    
                    float beat_dur = 60.0f / timeline.getBPM();
                    float snap_sec = beat_dur * 4.0f; // Snap to 4 beats (1 bar)
                    float snapped_time = std::round(click_time_sec / snap_sec) * snap_sec;
                    if (snapped_time < 0.0f) snapped_time = 0.0f;
                    
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        float len_sec = beat_dur * 16.0f; // 4 bars pattern length
                        g_clip_manager.addPatternClip(i, snapped_time, len_sec, g_clip_manager.global_patterns[g_clip_manager.current_pattern_idx].id);
                    }
                    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                        g_clip_manager.removeMidiClipAt(i, click_time_sec);
                    }
                }
                
                // --- FASE 10: AUTOMATION LANES ---
                std::string target_node = "TrackOut_" + std::to_string(i);
                int target_param = 0; // Volume
                KuroDSP::TimelineManager::AutomationLane* current_lane = nullptr;
                for (auto& lane : timeline.automation_lanes) {
                    if (lane.target_node_id == target_node && lane.param_index == target_param) {
                        current_lane = &lane;
                        break;
                    }
                }
                if (current_lane && !current_lane->points.empty()) {
                    for (size_t pt_idx = 0; pt_idx < current_lane->points.size(); pt_idx++) {
                        const auto& pt = current_lane->points[pt_idx];
                        float px = start_x + (pt.time_sec * pixels_per_second);
                        float py = p_min.y + track_height * (1.0f - pt.value);
                        
                        if (px > screen_view_min_x && px < screen_view_max_x) {
                            draw_list->AddCircleFilled(ImVec2(px, py), 4.0f, IM_COL32(230, 230, 50, 255));
                            if (pt_idx > 0) {
                                const auto& prev_pt = current_lane->points[pt_idx - 1];
                                float prev_px = start_x + (prev_pt.time_sec * pixels_per_second);
                                float prev_py = p_min.y + track_height * (1.0f - prev_pt.value);
                                draw_list->AddLine(ImVec2(prev_px, prev_py), ImVec2(prev_px, py), IM_COL32(230, 230, 50, 200), 1.5f);
                            }
                        }
                    }
                }
                
                ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_min.y + track_height));
                ImGui::Dummy(ImVec2(1, 0));
                ImGui::PopID();
            }

            // Playhead
            float playhead_x = ImGui::GetWindowPos().x + header_width + 10 - ImGui::GetScrollX() + playhead_x_offset; 
            float min_playhead_x = ImGui::GetWindowPos().x + header_width + 10;
            
            float playhead_y_min = ImGui::GetWindowPos().y; 
            float playhead_y_max = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
            
            draw_list->PushClipRect(ImVec2(min_playhead_x, playhead_y_min), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, playhead_y_max), true);
            if (playhead_x >= min_playhead_x) {
                draw_list->AddLine(ImVec2(playhead_x, playhead_y_min), ImVec2(playhead_x, playhead_y_max), IM_COL32(57, 255, 20, 200), 2.0f);
                draw_list->AddTriangleFilled(ImVec2(playhead_x - 6, playhead_y_min), ImVec2(playhead_x + 6, playhead_y_min), ImVec2(playhead_x, playhead_y_min + 8), IM_COL32(57, 255, 20, 255));
            }
            draw_list->PopClipRect();
            
            ImGui::EndChild();
        }
        ImGui::End();
        
        if (ImGui::Begin("[Channel Rack]")) {
            KuroUI::RenderStepSequencer(nullptr, ::timeline, timeline.getBPM(), g_clip_manager);
        }
        ImGui::End();
        
        if (ImGui::Begin("[Automation Clip]")) {
            ImGui::TextColored(ImVec4(0.6f,0.6f,0.6f,1.f), "Automation Clip Editor");
        }
        ImGui::End();
        
        if (ImGui::Begin("[Audio Editor]")) {
            ImGui::TextColored(ImVec4(0.6f,0.6f,0.6f,1.f), "Audio Editor");
        }
        ImGui::End();


        
        // ==========================================
        // 2. PAINEL CENTRAL (BROWSER + TIMELINE)
        // ==========================================
        
        // --- BROWSER PANEL (Left) ---
        ImGui::Begin("Browser");
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "🛸 BROWSER");
        ImGui::Separator();
        if (ImGui::TreeNodeEx("Samples & Loops", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BulletText("Kick_Alien_01.wav");
            ImGui::BulletText("Bass_FM_C.wav");
            ImGui::BulletText("Vocal_Abduction.wav");
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Plugins (VST3/CLAP)", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BulletText("Serum (VST3)");
            ImGui::BulletText("Vital (CLAP)");
            ImGui::BulletText("Kuro Synth (Native)");
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Projetos")) {
            ImGui::BulletText("Area51_Jam.abduct");
            ImGui::TreePop();
        }
        ImGui::End();
        
        // --- TIMELINE (inside Piano Roll docked window) ---
        // The DockSpace handles placement - no SameLine needed

        // Cores das faixas (Tons cibernéticos e alienígenas)
        ImVec4 timeline_track_colors[MAX_TRACKS] = {
            ImVec4(0.8f, 0.2f, 0.2f, 1.0f), // KICK/BASS
            ImVec4(0.2f, 0.8f, 0.2f, 1.0f), // LEADS
            ImVec4(0.2f, 0.2f, 0.8f, 1.0f), // VOX
            ImVec4(0.8f, 0.8f, 0.2f, 1.0f), // FX
            ImVec4(0.2f, 0.8f, 0.8f, 1.0f), // DRUMS
            ImVec4(0.8f, 0.2f, 0.8f, 1.0f), // SYNTH
            ImVec4(0.5f, 0.5f, 0.5f, 1.0f), // PADS
            ImVec4(0.3f, 0.7f, 0.4f, 1.0f),  // EXTRA
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f), ImVec4(0.4f, 0.4f, 0.4f, 1.0f)
        };

        float header_width = 180.0f;
        ImGui::BeginChild("Timeline", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        draw_list = ImGui::GetWindowDrawList(); // Pegar o draw_list AQUI garante o clipping do ChildWindow!
        
        static float pixels_per_second = 15.0f;
        
        if (ImGui::IsWindowHovered() && io.KeyCtrl && io.MouseWheel != 0.0f) {
            pixels_per_second += io.MouseWheel * 1.5f;
            if (pixels_per_second < 4.0f) pixels_per_second = 4.0f;
            if (pixels_per_second > 200.0f) pixels_per_second = 200.0f;
        }
        int num_tracks = 8;
        float track_height = 55.0f; // Um pouco mais alto para ficar espaçoso
        
        float playhead_x_offset = ((float)timeline.getMasterFrame() / 44100.0f) * pixels_per_second;

        // Calcular a largura total da linha do tempo com base nos clipes e na agulha
        float total_timeline_width = ImGui::GetWindowWidth();
        if (playhead_x_offset + header_width + 500.0f > total_timeline_width) {
            total_timeline_width = playhead_x_offset + header_width + 500.0f;
        }
        for (int i = 0; i < MAX_TRACKS; i++) {
            for (const auto& clip : g_clip_manager.getClips(i)) {
                float clip_end = (clip.start_time_sec + clip.length_sec) * pixels_per_second;
                if (clip_end + header_width + 500.0f > total_timeline_width) {
                    total_timeline_width = clip_end + header_width + 500.0f;
                }
            }
            for (const auto& clip : g_clip_manager.getMidiClips(i)) {
                float clip_end = (clip.start_time_sec + clip.length_sec) * pixels_per_second;
                if (clip_end + header_width + 500.0f > total_timeline_width) {
                    total_timeline_width = clip_end + header_width + 500.0f;
                }
            }
        }

        // --- Régua da Timeline ---
        ImVec2 ruler_pos = ImGui::GetCursorScreenPos();
        float ruler_height = 20.0f;
        
        // Desenha o fundo da régua (cinza escuro)
        draw_list->AddRectFilled(ruler_pos, ImVec2(ruler_pos.x + total_timeline_width, ruler_pos.y + ruler_height), IM_COL32(40, 40, 45, 255));
        draw_list->AddLine(ImVec2(ruler_pos.x, ruler_pos.y + ruler_height), ImVec2(ruler_pos.x + total_timeline_width, ruler_pos.y + ruler_height), IM_COL32(60, 60, 70, 255), 1.0f);
        
        // Desenhar os ticks da régua
        // Cada tick representa 6 segundos (0, 1, 2, 3, 4, 5... na régua correspondem a 0s, 6s, 12s, 18s, 24s, 30s...)
        // Lembrete: 6 segundos = 6 * pixels_per_second = 300 pixels.
        float scroll_x = ImGui::GetScrollX();
        float window_width = ImGui::GetWindowWidth();
        float tick_spacing = 6.0f * pixels_per_second; // 300.0f
        
        // Find visible ticks range
        int start_tick = (int)(scroll_x / tick_spacing);
        int end_tick = (int)((scroll_x + window_width) / tick_spacing) + 2;
        
        float visible_min_x = ImGui::GetWindowPos().x + header_width + 10;
        float visible_max_x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
        
        // Clip ticks to visible grid area
        draw_list->PushClipRect(ImVec2(visible_min_x, ruler_pos.y), ImVec2(visible_max_x, ruler_pos.y + ruler_height), true);
        
        for (int t = start_tick; t <= end_tick; t++) {
            float tx = ruler_pos.x + header_width + 10 + t * tick_spacing;
            
            if (t % 5 == 0) {
                // Major tick: maior, com número
                draw_list->AddLine(ImVec2(tx, ruler_pos.y + 4.0f), ImVec2(tx, ruler_pos.y + ruler_height), IM_COL32(200, 200, 200, 255), 1.5f);
                
                char label[16];
                snprintf(label, sizeof(label), "%d", t); // 0, 5, 10, 15...
                draw_list->AddText(ImVec2(tx + 4, ruler_pos.y + 2), IM_COL32(200, 200, 200, 255), label);
            } else {
                // Minor tick
                draw_list->AddLine(ImVec2(tx, ruler_pos.y + 12.0f), ImVec2(tx, ruler_pos.y + ruler_height), IM_COL32(120, 120, 120, 255), 1.0f);
            }
        }
        draw_list->PopClipRect();
        
        // Cabeçalho da Régua (Fixo na esquerda)
        scroll_x = ImGui::GetScrollX();
        ImVec2 header_ruler_min = ImVec2(ruler_pos.x + scroll_x, ruler_pos.y);
        ImVec2 header_ruler_max = ImVec2(header_ruler_min.x + header_width, ruler_pos.y + ruler_height);
        draw_list->AddRectFilled(header_ruler_min, header_ruler_max, IM_COL32(25, 25, 30, 255));
        draw_list->AddLine(ImVec2(header_ruler_max.x, header_ruler_min.y), ImVec2(header_ruler_max.x, header_ruler_max.y), IM_COL32(60, 60, 70, 255), 2.0f);
        
        // Pula o cursor da UI para baixo da régua
        ImGui::Dummy(ImVec2(total_timeline_width, ruler_height));

        for (int i = 0; i < num_tracks; i++) {
            ImGui::PushID(i);
            
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            // p_min.x já considera o scroll negativo da janela do ImGui!
            // Então, a largura visual da track precisa ser total_timeline_width real.
            ImVec2 p_max_track = ImVec2(p_min.x + total_timeline_width, p_min.y + track_height);
            
            // Fundo da Faixa
            draw_list->AddRectFilled(p_min, p_max_track, selected_track_idx == i ? IM_COL32(30, 40, 30, 255) : IM_COL32(15, 15, 18, 255));
            draw_list->AddRect(p_min, p_max_track, IM_COL32(40, 40, 50, 255));
            
            // Cabeçalho da Faixa (Acompanha o scroll na visualização, mas vamos prender ele na esquerda da tela subtraindo o scrollX)
            float scroll_x = ImGui::GetScrollX();
            ImVec2 header_p_min = ImVec2(p_min.x + scroll_x, p_min.y);
            ImVec2 header_p_max = ImVec2(header_p_min.x + header_width, p_min.y + track_height);
            
            draw_list->AddRectFilled(header_p_min, header_p_max, IM_COL32(25, 25, 30, 255));
            draw_list->AddLine(ImVec2(header_p_max.x, header_p_min.y), ImVec2(header_p_max.x, header_p_max.y), IM_COL32(60, 60, 70, 255), 2.0f);
            
            // Linha colorida de destaque no cabeçalho
            draw_list->AddRectFilled(header_p_min, ImVec2(header_p_min.x + 6, header_p_max.y), ImColor(timeline_track_colors[i]));
            
            ImGui::SetCursorScreenPos(ImVec2(header_p_min.x + 12, p_min.y + 5));
            
            // Seleção de Faixa
            if (ImGui::Selectable((track_names[i] + "##sel" + std::to_string(i)).c_str(), selected_track_idx == i, 0, ImVec2(100, 15))) {
                selected_track_idx = i;
            }
            
            ImGui::SetCursorScreenPos(ImVec2(header_p_min.x + 12, p_min.y + 28));
            
            if (::track_mutes[i]) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            }
            if (ImGui::Button((std::string("M##m") + std::to_string(i)).c_str(), ImVec2(24, 20))) { ::track_mutes[i] = !::track_mutes[i]; }
            ImGui::PopStyleColor();
            
            ImGui::SameLine();
            if (::track_solos[i]) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.2f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            }
            if (ImGui::Button((std::string("S##s") + std::to_string(i)).c_str(), ImVec2(24, 20))) { ::track_solos[i] = !::track_solos[i]; }
            ImGui::PopStyleColor();

            // FASE 28: FX RACK movido para o GLOBAL MIXER

            // Timeline Waveforms
            ImU32 col = ImColor(timeline_track_colors[i]);
            float mid_y = p_min.y + (track_height / 2);
            float start_x = p_min.x + header_width + 10;
            
            const auto& wave_data = ai_engine.getWaveformOverview(i);
            auto clips = g_clip_manager.getClips(i);
            
            // Visibilidade real da tela
            float screen_view_min_x = p_min.x + scroll_x + header_width;
            float screen_view_max_x = p_min.x + scroll_x + ImGui::GetWindowWidth();
            
            for (size_t c_idx = 0; c_idx < clips.size(); c_idx++) {
                const auto& clip = clips[c_idx];
                float clip_start_x = start_x + (clip.start_time_sec * pixels_per_second);
                float clip_width = clip.length_sec * pixels_per_second;
                float clip_end_x = clip_start_x + clip_width;
                
                if (clip_end_x > screen_view_min_x && clip_start_x < screen_view_max_x) {
                    float draw_start = std::max(clip_start_x, screen_view_min_x);
                    float draw_end = std::min(clip_end_x, screen_view_max_x);
                    
                    ImU32 fill_col = ImColor(timeline_track_colors[i].x, timeline_track_colors[i].y, timeline_track_colors[i].z, clip.is_selected ? 0.4f : 0.15f);
                    ImU32 outline_col = ImColor(timeline_track_colors[i].x, timeline_track_colors[i].y, timeline_track_colors[i].z, 0.8f);
                    
                    draw_list->AddRectFilled(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), fill_col, 4.0f);
                    draw_list->AddRect(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), outline_col, 4.0f);
                    
                    if (wave_data.size() > 0) {
                        float source_px_start = clip.source_offset_sec * pixels_per_second;
                        for (float px = 0; px < clip_width; px++) {
                            float point_x = clip_start_x + px;
                            if (point_x >= draw_start && point_x <= draw_end) {
                                size_t wave_idx = (size_t)(source_px_start + px);
                                if (wave_idx < wave_data.size()) {
                                    float amp = wave_data[wave_idx] * ((track_height / 2) - 4); 
                                    draw_list->AddLine(ImVec2(point_x, mid_y - amp), ImVec2(point_x, mid_y + amp), col, 1.0f);
                                }
                            }
                        }
                    }
                    
                    ImGui::SetCursorScreenPos(ImVec2(draw_start, p_min.y));
                    ImGui::InvisibleButton("##clip_btn", ImVec2(draw_end - draw_start, track_height));
                }
            }
            
            auto mclips = g_clip_manager.getMidiClips(i);
            for (size_t c_idx = 0; c_idx < mclips.size(); c_idx++) {
                const auto& clip = mclips[c_idx];
                float clip_start_x = start_x + (clip.start_time_sec * pixels_per_second);
                float clip_width = clip.length_sec * pixels_per_second;
                float clip_end_x = clip_start_x + clip_width;
                
                if (clip_end_x > screen_view_min_x && clip_start_x < screen_view_max_x) {
                    float draw_start = std::max(clip_start_x, screen_view_min_x);
                    float draw_end = std::min(clip_end_x, screen_view_max_x);
                    
                    ImU32 fill_col = ImColor(timeline_track_colors[i].x * 0.8f, timeline_track_colors[i].y * 0.8f, timeline_track_colors[i].z * 0.8f, clip.is_selected ? 0.6f : 0.25f);
                    ImU32 outline_col = ImColor(timeline_track_colors[i].x, timeline_track_colors[i].y, timeline_track_colors[i].z, 1.0f);
                    
                    draw_list->AddRectFilled(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), fill_col, 4.0f);
                    draw_list->AddRect(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), outline_col, 4.0f);
                    
                    draw_list->PushClipRect(ImVec2(draw_start, p_min.y + 2), ImVec2(draw_end, p_min.y + track_height - 2), true);
                    
                    // Render pattern notes
                    Pattern* p = nullptr;
                    for(auto& pat : g_clip_manager.global_patterns) {
                        if(pat.id == clip.pattern_id) { p = &pat; break; }
                    }
                    if(p) {
                        // Title of Pattern
                        draw_list->AddText(ImVec2(draw_start + 4, p_min.y + 4), IM_COL32(255,255,255,255), p->name.c_str());
                        for (const auto& n : p->notes) {
                            float n_x = clip_start_x + (n.start_time * pixels_per_second);
                            float n_w = n.duration * pixels_per_second;
                            float n_y = p_min.y + track_height - 6.0f - ((n.pitch / 127.0f) * (track_height - 24.0f));
                            draw_list->AddRectFilled(ImVec2(n_x, n_y - 2), ImVec2(n_x + n_w, n_y + 2), IM_COL32(255, 255, 255, 200));
                        }
                    }
                    draw_list->PopClipRect();
                    
                    ImGui::SetCursorScreenPos(ImVec2(draw_start, p_min.y));
                    ImGui::PushID(clip.id);
                    ImGui::InvisibleButton("##mclip_btn", ImVec2(draw_end - draw_start, track_height));
                    ImGui::PopID();
                }
            }
            // --- FASE 10: AUTOMATION LANES ---
            std::string target_node = "TrackOut_" + std::to_string(i);
            int target_param = 0; // Volume
            KuroDSP::TimelineManager::AutomationLane* current_lane = nullptr;
            for (auto& lane : timeline.automation_lanes) {
                if (lane.target_node_id == target_node && lane.param_index == target_param) {
                    current_lane = &lane;
                    break;
                }
            }

            ImGui::SetCursorScreenPos(ImVec2(start_x, p_min.y)); 
            ImGui::InvisibleButton(("##track_bg_" + std::to_string(i)).c_str(), ImVec2(total_timeline_width, track_height));
            
            // FASE 30: Drag and Drop Target for Patterns
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PATTERN_PAYLOAD")) {
                    int p_idx = *(const int*)payload->Data;
                    float drop_x = ImGui::GetMousePos().x - start_x;
                    float drop_time_sec = drop_x / pixels_per_second;
                    float snap_sec = 60.0f / timeline.getBPM(); // Snap to beat
                    drop_time_sec = std::round(drop_time_sec / snap_sec) * snap_sec;
                    
                    int pat_id = g_clip_manager.global_patterns[p_idx].id;
                    float def_len = g_clip_manager.global_patterns[p_idx].default_length_sec;
                    g_clip_manager.addPatternClip(i, drop_time_sec, def_len, pat_id);
                }
                ImGui::EndDragDropTarget();
            }
            
            if (current_lane && !current_lane->points.empty()) {
                ImU32 auto_col = IM_COL32(255, 100, 100, 255); // Red for volume
                std::vector<ImVec2> draw_points;
                for (const auto& pt : current_lane->points) {
                    float px_x = start_x + (pt.time_sec * pixels_per_second);
                    // Value is -60 to 6
                    float norm_val = (pt.value + 60.0f) / 66.0f;
                    norm_val = std::clamp(norm_val, 0.0f, 1.0f);
                    float px_y = p_min.y + track_height - (norm_val * track_height);
                    draw_points.push_back(ImVec2(px_x, px_y));
                }
                
                // Draw lines
                for (size_t pt_idx = 0; pt_idx < draw_points.size() - 1; pt_idx++) {
                    draw_list->AddLine(draw_points[pt_idx], draw_points[pt_idx+1], auto_col, 2.0f);
                }
                // Draw circles
                for (const auto& dp : draw_points) {
                    draw_list->AddCircleFilled(dp, 4.0f, auto_col);
                }
            }

            // Interactions for Automation
            if (ImGui::IsItemHovered()) {
                float mouse_x = ImGui::GetMousePos().x - start_x;
                float mouse_time_sec = mouse_x / pixels_per_second;
                float mouse_y = ImGui::GetMousePos().y - p_min.y;
                float norm_y = 1.0f - (mouse_y / track_height);
                float val = -60.0f + (norm_y * 66.0f);
                
                if (io.KeyAlt && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    // Alt+Click = Add point
                    timeline.addAutomationPoint(target_node, target_param, mouse_time_sec, val);
                } else if (io.KeyAlt && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                    // Alt+RightClick = Delete point
                    timeline.removeAutomationPoint(target_node, target_param, mouse_time_sec);
                } else if (io.KeyAlt && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    // Alt+Drag = Move point
                    timeline.addAutomationPoint(target_node, target_param, mouse_time_sec, val);
                }
            }

            ImGui::PopID();
            ImGui::Spacing();
        }

        // Agulha (Playhead) Verde Neon e Interação
        if (is_playing) {
            float target_scroll = playhead_x_offset - (ImGui::GetWindowWidth() * 0.5f);
            if (target_scroll < 0) target_scroll = 0;
            ImGui::SetScrollX(target_scroll);
        }
        
        // Interação de Clique/Arraste na Timeline
        static bool is_dragging_playhead = false;
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            is_dragging_playhead = true;
        }
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            is_dragging_playhead = false;
        }

        if (is_dragging_playhead) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            if (mouse_pos.x > ImGui::GetWindowPos().x + header_width) { // Clicou depois do cabeçalho
                float click_x_in_timeline = mouse_pos.x - (ImGui::GetWindowPos().x + header_width + 10 - ImGui::GetScrollX());
                if (click_x_in_timeline < 0) click_x_in_timeline = 0;
                float grid_start_x = ImGui::GetWindowPos().x + header_width + 10;
                float clicked_time_sec = (io.MousePos.x - grid_start_x + ImGui::GetScrollX()) / pixels_per_second;
                timeline.setMasterFrame((uint64_t)(clicked_time_sec * 44100.0f));
                playhead_x_offset = click_x_in_timeline; // Update instantâneo visual
            }
        }
        
        float playhead_x = ImGui::GetWindowPos().x + header_width + 10 - ImGui::GetScrollX() + playhead_x_offset; 
        float min_playhead_x = ImGui::GetWindowPos().x + header_width + 10;
        
        // Clip playhead line and triangle so it doesn't leak into the track headers
        float playhead_y_min = ImGui::GetWindowPos().y; 
        float playhead_y_max = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
        
        draw_list->PushClipRect(ImVec2(min_playhead_x, playhead_y_min), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, playhead_y_max), true);
        if (playhead_x >= min_playhead_x) {
            draw_list->AddLine(ImVec2(playhead_x, playhead_y_min), ImVec2(playhead_x, playhead_y_max), IM_COL32(57, 255, 20, 200), 2.0f);
            draw_list->AddTriangleFilled(ImVec2(playhead_x - 6, playhead_y_min), ImVec2(playhead_x + 6, playhead_y_min), ImVec2(playhead_x, playhead_y_min + 8), IM_COL32(57, 255, 20, 255));
        }
        draw_list->PopClipRect();

        ImGui::EndChild(); // Timeline

        // ==========================================
        // 3. BOTTOM RACK (DEVICE VIEW)
        // ==========================================
        ImGui::BeginChild("BottomPanel", ImVec2(0, 0), true);
        
        // Header do Device View
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "DEVICE RACK: %s", track_names[selected_track_idx].c_str());
        
        // Quebra de linha (sem SameLine) para os botões caberem perfeitamente
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
        if (ImGui::Button("[PR] PIANO ROLL", ImVec2(120, 30))) show_piano_roll = true;
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
        if (ImGui::Button("[DL] CLOUD DOWN", ImVec2(120, 30))) show_cloud_downloader = true;
        ImGui::PopStyleColor();
        
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("DeviceTabs")) {
            // O FX RACK foi removido daqui e transferido para o Track Header e Viewports (Fase 28)
            if (ImGui::BeginTabItem("SYNTH (KuroWave)")) {
                ImGui::Spacing();
                KuroUI::RenderKuroWaveSynth(g_kurowave);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("SAMPLER")) {
                ImGui::Spacing();
                KuroUI::RenderKuroSampler(g_global_sampler);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("STEP SEQUENCER")) {
                ImGui::Spacing();
                KuroUI::RenderStepSequencer(nullptr, ::timeline, timeline.getBPM(), g_clip_manager);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("LFO MATRIX")) {
                ImGui::Spacing();
                KuroUI::RenderModulationPanel(nullptr);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("GROSS BEAT")) {
                ImGui::Spacing();
                KuroUI::GrossBeatUI::Render(::g_gross_beat, nullptr);
                ImGui::EndTabItem();
            }
            // Aba PIANO ROLL removida a pedido do usuario (virou botao no topo e janela flutuante)
            
            ImGuiTabItemFlags mixer_flags = set_mixer_focus ? ImGuiTabItemFlags_SetSelected : 0;
            if (ImGui::BeginTabItem("GLOBAL MIXER", nullptr, mixer_flags)) {
                ImGui::Spacing();
                
                // --- FL Studio Style Mixer ---
                float mixer_height = 220.0f; // fixed height for tracks
                ImGui::BeginChild("MixerTracks", ImVec2(ImGui::GetContentRegionAvail().x - 220, mixer_height), false, ImGuiWindowFlags_HorizontalScrollbar);
                
                // 1. MASTER TRACK
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.2f, 1.0f));
                ImGui::BeginChild("Master", ImVec2(80, 200), true);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
                ImGui::Text("MASTER");
                ImGui::PopStyleColor();
                
                ImGui::Spacing();
                ImGui::PushItemWidth(60);
                static float master_vol = 1.0f;
                static float master_pan = 0.0f;
                ImGui::SliderFloat("##mpan", &master_pan, -1.0f, 1.0f, "");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Master Pan");
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
                ImGui::VSliderFloat("##mvol", ImVec2(60, 110), &master_vol, 0.0f, 1.2f, "");
                ImGui::PopStyleColor();
                ImGui::PopItemWidth();
                ImGui::EndChild();
                ImGui::PopStyleColor(); // childbg
                
                ImGui::SameLine();
                ImGui::TextDisabled("|");
                ImGui::SameLine();
                
                // 2. INSERT TRACKS
                for (int i = 0; i < MAX_TRACKS; i++) {
                    ImGui::PushID(i);
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.16f, 0.17f, 1.0f));
                    ImGui::BeginChild("Insert", ImVec2(60, 200), true);
                    
                    // Track Color Header
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    draw_list->AddRectFilled(p, ImVec2(p.x + 45, p.y + 15), IM_COL32(80, 120, 160, 255), 2.0f);
                    ImGui::SetCursorScreenPos(ImVec2(p.x + 5, p.y + 1));
                    ImGui::TextColored(ImVec4(0,0,0,1), "Ins %d", i+1);
                    ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + 20));
                    
                    // Mute/Solo
                    bool is_muted = false; // mock
                    ImU32 led_color = is_muted ? IM_COL32(50, 50, 50, 255) : IM_COL32(50, 255, 50, 255);
                    p = ImGui::GetCursorScreenPos();
                    draw_list->AddCircleFilled(ImVec2(p.x + 22, p.y + 6), 6.0f, led_color);
                    ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + 16));
                    
                    ImGui::PushItemWidth(45);
                    // Panner
                    ImGui::SliderFloat("##pan", &track_pans[i], -1.0f, 1.0f, "");
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pan");
                    
                    // Fader
                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.4f, 0.5f, 0.4f, 1.0f));
                    ImGui::VSliderFloat("##vol", ImVec2(45, 95), &::track_volumes[i], -60.0f, 6.0f, "");
                    ImGui::PopStyleColor();
                    ImGui::PopItemWidth();
                    
                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::PopID();
                }
                ImGui::EndChild(); // MixerTracks
                
                ImGui::SameLine();
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                ImGui::SameLine();
                
                // 3. INSPECTOR (FX Slots)
                ImGui::BeginChild("Inspector", ImVec2(0, mixer_height), true);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
                ImGui::Text("FX SLOTS (Ins 1)");
                ImGui::PopStyleColor();
                ImGui::Separator();
                
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
                for(int slot = 0; slot < 10; slot++) {
                    ImGui::PushID(slot);
                    // FL Studio style FX slots
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
                    if (ImGui::Button(slot == 0 ? "Parametric EQ 2" : "Slot", ImVec2(ImGui::GetContentRegionAvail().x - 20, 16))) {
                        // open plugin
                    }
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    // Mix level knob mock
                    static float mix_lvl = 1.0f;
                    ImGui::PushItemWidth(15);
                    ImGui::SliderFloat("##mix", &mix_lvl, 0, 1, "");
                    ImGui::PopItemWidth();
                    ImGui::PopID();
                }
                ImGui::PopStyleVar();
                ImGui::EndChild(); // Inspector
                
                ImGui::EndTabItem();
            }
            if (set_mixer_focus) set_mixer_focus = false;
            
            ImGui::EndTabBar();
        }

        ImGui::EndChild(); // BottomPanel

        // ==========================================
        // 4. ATALHOS GLOBAIS (FL STUDIO STYLE)
        // ==========================================
        if (!io.WantCaptureKeyboard) {
            if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
                KuroUI::TransportController::TogglePlayPause(timeline);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_S) && io.KeyCtrl) {
                g_record_manager.saveToWav("Kuro_Export_Master.wav");
            }
            if (ImGui::IsKeyPressed(ImGuiKey_M)) {
                ::track_mutes[selected_track_idx] = !::track_mutes[selected_track_idx];
            }
            if (ImGui::IsKeyPressed(ImGuiKey_S) && !io.KeyCtrl) {
                ::track_solos[selected_track_idx] = !::track_solos[selected_track_idx];
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
                set_mixer_focus = true;
            }
        }
    }

    // FASE 28: Renderiza Janelas Flutuantes (Fora da Janela Principal)
    inline void RenderFloatingWindows() {
        RenderSamplerSettings(active_sampler_channel);
        RenderFlexPresetBrowser(active_flex_channel);
        if (show_cloud_downloader) {
            ImGui::SetNextWindowSize(ImVec2(500, 160), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("☁️ Cloud Downloader (Psycore Project)", &show_cloud_downloader)) {
                ImGui::Text("Pesquisar (Nome da musica / Link Spotify ou YouTube):");
                ImGui::InputText("##CloudQuery", cloud_search_query, IM_ARRAYSIZE(cloud_search_query));
                
                if (ImGui::Button("BAIXAR (.WAV)", ImVec2(-1, 40))) {
                    std::string query = cloud_search_query;
                    if (!query.empty()) {
                        cloud_status = "Baixando: " + query + "... (Aguarde)";
                        std::thread([query]() {
                            std::string cmd = "python \"C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\.agents\\scripts\\download_track.py\" \"" + query + "\"";
                            int ret = std::system(cmd.c_str());
                            if (ret == 0) {
                                cloud_status = "Sucesso! Aguardando ação...";
                                cloud_download_finished = true;
                            } else {
                                cloud_status = "Erro no download. Verifique o console.";
                            }
                        }).detach();
                    }
                }
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: %s", cloud_status.c_str());
            }
            ImGui::End();
        }

        static bool show_piano_roll_prev = false;
        
        if (show_piano_roll && !show_piano_roll_prev) {
            // FASE 30: Patterns are always edited directly.
        }

        // --- CLOUD DOWNLOAD TOAST E MODAL ---
        if (cloud_download_finished) {
            cloud_download_finished = false;
            
            // Lê o caminho salvo pelo python
            std::ifstream file("C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\last_download.txt");
            if (file.is_open()) {
                std::getline(file, cloud_last_downloaded_file);
                file.close();
            }
            
            // Toca a notificação divina
            PlaySoundA("C:\\NovaDAW\\assets\\divine.wav", NULL, SND_FILENAME | SND_ASYNC);
            
            show_download_toast = true;
            toast_timer = 10.0f; // Duração do toast em segundos (aproximado se usássemos delta_time, mas vamos decrementar no frame de forma simples)
        }
        
        if (show_download_toast) {
            // Posiciona no canto inferior direito
            ImGuiIO& io = ImGui::GetIO();
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 320, io.DisplaySize.y - 120), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
            
            ImGuiWindowFlags toastFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
            
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.3f, 0.6f, 0.9f));
            if (ImGui::Begin("DownloadToast", nullptr, toastFlags)) {
                ImGui::TextColored(ImVec4(1,1,1,1), "☁️ MENSAGEM DIVINA");
                ImGui::Separator();
                ImGui::TextWrapped("Download concluído com sucesso!");
                
                if (ImGui::Button("TOMAR UMA AÇÃO", ImVec2(-1, 30))) {
                    show_download_toast = false;
                    show_download_action_modal = true;
                }
            }
            ImGui::End();
            ImGui::PopStyleColor();
        }
        
        if (show_download_action_modal) {
            ImGui::OpenPopup("Download Action");
            show_download_action_modal = false; // OpenPopup cuida do estado
        }
        
        // Modal no centro
        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("Download Action", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Arquivo: %s", cloud_last_downloaded_file.c_str());
            ImGui::Separator();
            
            if (ImGui::Button("Importar para o Sampler", ImVec2(-1, 40))) {
                // Adiciona no sampler global
                if (g_global_sampler) {
                    g_global_sampler->loadSample(cloud_last_downloaded_file);
                }
                ImGui::CloseCurrentPopup();
            }
            
            if (ImGui::Button("Abrir Local do Arquivo", ImVec2(-1, 40))) {
                std::string cmd = "explorer /select,\"" + cloud_last_downloaded_file + "\"";
                std::system(cmd.c_str());
                ImGui::CloseCurrentPopup();
            }
            
            if (ImGui::Button("Fechar", ImVec2(-1, 30))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (show_piano_roll) {
            unsigned long long curr_frame = timeline.getMasterFrame();
            KuroUI::RenderPianoRoll(&show_piano_roll, ::timeline, selected_track_idx, timeline.getBPM(), &curr_frame, ::is_playing, g_clip_manager, nullptr);
        }
        
        static bool show_export_modal = false;
        show_piano_roll_prev = show_piano_roll;

        
        // Dangling modal syntax removed
        if (show_step_sequencer) {
            ImGui::SetNextWindowSize(ImVec2(685, 380), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Channel Rack", &show_step_sequencer)) {
                KuroUI::RenderStepSequencer(&show_step_sequencer, ::timeline, timeline.getBPM(), g_clip_manager);
            }
            ImGui::End();
        }
        if (show_modulation_panel) {
            KuroUI::RenderModulationPanel(&show_modulation_panel);
        }
        
        if (show_gross_beat) {
            KuroUI::GrossBeatUI::Render(::g_gross_beat, &show_gross_beat);
        }
    }

    // Renderiza Modos Mockados para DJ e Stems (Fase 24)
    static void RenderDJMode() {
        ImGui::BeginChild("DJMode", ImVec2(0, 0), true);
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "🎛️ MODO DJ: EM CONSTRUÇÃO");
        ImGui::Separator();
        
        if (g_dj_engine) {
            ImGui::Spacing();
            ImGui::Columns(2, "Decks", true);
            // Deck A
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "DECK A");
            ImGui::TextWrapped("Faixa: %s", g_dj_engine->getDeckATrack().c_str());
            if (ImGui::Button("Carregar A", ImVec2(100, 30))) {
                std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                if (!path.empty()) g_dj_engine->loadDeckA(path);
            }
            ImGui::SameLine();
            if (ImGui::Button(g_dj_engine->isDeckAPlaying() ? "Pause A" : "Play A", ImVec2(100, 30))) {
                if (g_dj_engine->isDeckAPlaying()) g_dj_engine->pauseDeckA();
                else g_dj_engine->playDeckA();
            }
            ImGui::NextColumn();
            
            // Deck B
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "DECK B");
            ImGui::TextWrapped("Faixa: %s", g_dj_engine->getDeckBTrack().c_str());
            if (ImGui::Button("Carregar B", ImVec2(100, 30))) {
                std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                if (!path.empty()) g_dj_engine->loadDeckB(path);
            }
            ImGui::SameLine();
            if (ImGui::Button(g_dj_engine->isDeckBPlaying() ? "Pause B" : "Play B", ImVec2(100, 30))) {
                if (g_dj_engine->isDeckBPlaying()) g_dj_engine->pauseDeckB();
                else g_dj_engine->playDeckB();
            }
            ImGui::Columns(1);
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Mixer Central Placeholder
            ImGui::Text("CROSSFADER");
            float cf = g_dj_engine->getCrossfader();
            if (ImGui::SliderFloat("##crossfader", &cf, 0.0f, 1.0f, "%.2f")) {
                g_dj_engine->setCrossfader(cf);
            }
            if (ImGui::Button("SYNC BPM", ImVec2(200, 40))) {
                g_dj_engine->syncBPM();
            }
        }
        ImGui::EndChild();
    }

    static void RenderStemMode() {
        ImGui::BeginChild("StemMode", ImVec2(0, 0), true);
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "🧬 MODO EXTRAÇÃO DE STEMS: EM CONSTRUÇÃO");
        ImGui::Separator();
        
        if (g_stem_engine) {
            ImGui::TextWrapped("Arquivo Isolado Atual: %s", g_stem_engine->getCurrentFile().c_str());
            if (ImGui::Button("Carregar Faixa para Isolar", ImVec2(300, 40))) {
                std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                if (!path.empty()) g_stem_engine->dropFileToIsolate(path);
            }
            
            ImGui::Spacing();
            if (g_stem_engine->isProcessing()) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Status: %s", g_stem_engine->getStatus().c_str());
                ImGui::ProgressBar(g_stem_engine->getProgress(), ImVec2(-1, 0));
            } else if (g_stem_engine->isFinished()) {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Pronto para exportar!");
                ImGui::Spacing();
                if (ImGui::Button("Baixar Kick/Bass", ImVec2(200, 30))) g_stem_engine->exportSingleStem(0);
                if (ImGui::Button("Baixar Leads/Drums", ImVec2(200, 30))) g_stem_engine->exportSingleStem(1);
                if (ImGui::Button("Baixar Vocals", ImVec2(200, 30))) g_stem_engine->exportSingleStem(2);
                if (ImGui::Button("Baixar FX", ImVec2(200, 30))) g_stem_engine->exportSingleStem(3);
            }
        }
        ImGui::EndChild();
    }

#include <GLFW/glfw3.h> // Para controle nativo da janela (Min/Max/Close)

    static void RenderMainMenu(StemSeparationEngine& ai_engine) {
        if (ImGui::BeginMenuBar()) {
            
            // Drag logic for borderless window
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                GLFWwindow* win = glfwGetCurrentContext();
                if (win) {
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    int wx, wy;
                    glfwGetWindowPos(win, &wx, &wy);
                    glfwSetWindowPos(win, wx + (int)delta.x, wy + (int)delta.y);
                }
            }

            // ── TRANSPORT BAR ──────────────────────────────────────────────
            // Play / Pause
            ImGui::PushStyleColor(ImGuiCol_Button,
                is_playing ? ImVec4(0.20f,0.55f,0.20f,1.f) : ImVec4(0.12f,0.13f,0.15f,1.f));
            if (KuroIcons::Button("##TPlay", is_playing ? KuroIcons::PAUSE : KuroIcons::PLAY,
                                  ImVec2(30,20), is_playing)) {
                KuroUI::TransportController::TogglePlayPause(timeline);
            }
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 2);

            // Stop
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f,0.13f,0.15f,1.f));
            if (KuroIcons::Button("##TStop", KuroIcons::STOP, ImVec2(30,20))) {
                KuroUI::TransportController::Stop(timeline);
            }
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 2);

            // Record
            bool rec = g_record_manager.isRecording();
            ImGui::PushStyleColor(ImGuiCol_Button,
                rec ? ImVec4(0.65f,0.10f,0.10f,1.f) : ImVec4(0.12f,0.13f,0.15f,1.f));
            if (KuroIcons::Button("##TRec", KuroIcons::RECORD, ImVec2(30,20), rec)) {
                if (rec) g_record_manager.setRecording(false);
                else     g_record_manager.setRecording(true, false);
            }
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 8);

            // Timecode + BPM display
            {
                size_t sr = ai_engine.getSampleRate(); if (sr == 0) sr = 44100;
                size_t mf = timeline.getMasterFrame();
                int cs = (int)(mf / sr);
                int ms = (int)((mf % sr) * 10 / sr);
                float bpm = ai_engine.getBPM() > 0 ? ai_engine.getBPM() : 140.0f;
                char hud[64];
                snprintf(hud, sizeof(hud), " %02d:%02d.%1d  |  %.0f BPM ", cs/60, cs%60, ms, bpm);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f,0.06f,0.07f,1.f));
                ImGui::PushStyleColor(ImGuiCol_Text,   ImVec4(0.33f,1.00f,0.20f,1.f));
                ImGui::Button(hud, ImVec2(0, 20));
                ImGui::PopStyleColor(2);
            }
            ImGui::SameLine(0, 8);
            // ───────────────────────────────────────────────────────────────

            if (ImGui::BeginMenu("Arquivo")) {
                if (ImGui::Button("NEW PROJECT (Reset)", ImVec2(150, 30))) {
                    ai_engine.reset();
                    g_clip_manager.reset();
                    timeline.setMasterFrame(0);
                    timeline.setPlaying(false);
                }
                if (ImGui::MenuItem("Abrir Projeto...")) {
                    std::string path = KuroUI::FileDialog::OpenFile("Abduction Project (*.kuro)\0*.kuro\0");
                    if (!path.empty()) {
                        ProjectManagerBridge::Load(path);
                    }
                }
                if (ImGui::MenuItem("Salvar Projeto...")) {
                    std::string path = KuroUI::FileDialog::SaveFile("Abduction Project (*.kuro)\0*.kuro\0");
                    if (!path.empty()) {
                        if (path.find(".kuro") == std::string::npos) path += ".kuro";
                        ProjectManagerBridge::Save(path);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Importar Áudio...")) {
                    std::string path = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3)\0*.wav;*.mp3\0");
                    if (!path.empty()) {
                        if (g_global_sampler && (path.find(".wav") != std::string::npos || path.find(".WAV") != std::string::npos)) {
                            g_global_sampler->loadSample(path);
                        }
                        if (!ai_engine.isRunning()) {
                            ai_engine.startProcessing(path);
                        }
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exportar Stems...")) {
                    KuroAudio::OfflineRenderer::RenderStems(master_graph, 10.0f, ".");
                }
                if (ImGui::MenuItem("Exportar Master (.wav)...")) {
                    g_record_manager.saveToWav("Kuro_Export_Master.wav");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Configurações da DAW")) {}
                if (ImGui::MenuItem("Sair")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Editar")) {
                if (ImGui::MenuItem("Desfazer", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Refazer", "Ctrl+Y")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Cortar", "Ctrl+X")) {}
                if (ImGui::MenuItem("Copiar", "Ctrl+C")) {}
                if (ImGui::MenuItem("Colar", "Ctrl+V")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Exibir")) {
                if (ImGui::MenuItem("Mostrar Browser", "", true)) {}
                if (ImGui::MenuItem("Mostrar Mixer", "", true)) {}
                if (ImGui::MenuItem("DAW API Explorer", "")) {
                    show_daw_api_explorer = true;
                }
                ImGui::Separator();
                if (ImGui::BeginMenu("Temas")) {
                    if (ImGui::MenuItem("Abduction (Nave)", "", true)) {}
                    if (ImGui::MenuItem("Gótico Kuro")) {}
                    if (ImGui::MenuItem("Ableton Clássico")) {}
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Modos")) {
                if (ImGui::MenuItem("Modo Studio", "", current_app_mode == AppMode::STUDIO_MODE)) {
                    current_app_mode = AppMode::STUDIO_MODE;
                }
                if (ImGui::MenuItem("Modo DJ", "", current_app_mode == AppMode::DJ_MODE)) {
                    current_app_mode = AppMode::DJ_MODE;
                }
                if (ImGui::MenuItem("Modo Extração (Stems)", "", current_app_mode == AppMode::STEM_MODE)) {
                    current_app_mode = AppMode::STEM_MODE;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Ajuda")) {
                if (ImGui::MenuItem("Documentação")) {}
                if (ImGui::MenuItem("Atalhos do Teclado")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Sobre o Abduction Studio")) {}
                ImGui::EndMenu();
            }
            
            // Window Controls (Min, Max, Close) na direita
            float menu_width = ImGui::GetWindowWidth();
            ImGui::SameLine(menu_width - 120);
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
            if (ImGui::Button("_", ImVec2(30, 20))) {
                GLFWwindow* win = glfwGetCurrentContext();
                if (win) glfwIconifyWindow(win);
            }
            ImGui::SameLine();
            if (ImGui::Button("[ ]", ImVec2(30, 20))) {
                GLFWwindow* win = glfwGetCurrentContext();
                if (win) {
                    if (glfwGetWindowAttrib(win, GLFW_MAXIMIZED)) glfwRestoreWindow(win);
                    else glfwMaximizeWindow(win);
                }
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("X", ImVec2(30, 20))) {
                GLFWwindow* win = glfwGetCurrentContext();
                if (win) glfwSetWindowShouldClose(win, GLFW_TRUE);
            }
            ImGui::PopStyleColor(); // text
            ImGui::PopStyleColor(); // bg

            ImGui::EndMenuBar();
        }
    }

    inline void RenderModulationOverlay(float active_val, float min_val, float max_val) {
        ImVec2 rect_min = ImGui::GetItemRectMin();
        ImVec2 rect_max = ImGui::GetItemRectMax();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        
        float active_normalized = (active_val - min_val) / (max_val - min_val);
        active_normalized = std::max(0.0f, std::min(active_normalized, 1.0f));
        
        float active_x = rect_min.x + active_normalized * (rect_max.x - rect_min.x);
        
        // Desenhar indicador neon ciano da modulação ativa
        draw_list->AddLine(ImVec2(active_x, rect_min.y), ImVec2(active_x, rect_max.y), IM_COL32(0, 255, 255, 200), 2.0f);
        draw_list->AddCircleFilled(ImVec2(active_x, rect_min.y + 2), 2.5f, IM_COL32(0, 255, 255, 255));
    }

    // FASE 28: Renderizar Janelas Flutuantes e Sincronizar DSP
    static void RenderFloatingPluginWindows() {
        // Usar a declaração global de KuroDSPUI.h
        
        // Sincronizar UI com o C++ DSP (Pedalboard)
        for (int i = 0; i < MAX_TRACKS; i++) {
            auto& board = ::track_pedalboards[i];
            board.preset_dark = false;
            board.preset_alien = false;
            board.preset_ritual = false;
            board.preset_psych = false;
            board.enable_abyss_pitch = false;
            
            for (auto& node : track_fx_chain[i]) {
                std::string fx_id = node->getId();
                bool bypassed = false;
                for (auto& win : active_plugin_windows) {
                    if (win.track_idx == i && win.plugin_id == fx_id) { bypassed = win.is_bypassed; break; }
                }
                if (!bypassed) {
                    if (fx_id == "1") board.preset_dark = true;
                    if (fx_id == "2") board.preset_alien = true;
                    if (fx_id == "3") board.preset_ritual = true;
                    if (fx_id == "4") board.preset_psych = true;
                    if (fx_id == "5") board.enable_abyss_pitch = true;
                }
            }
        }

        // Renderizar Janelas
        for (auto& win : active_plugin_windows) {
            if (!win.is_open) continue;
            
            ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
            std::string window_id = win.name + "##win" + std::to_string(win.track_idx) + "_" + win.plugin_id;
            
            if (ImGui::Begin(window_id.c_str(), &win.is_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing)) {
                auto& board = ::track_pedalboards[win.track_idx];

                // Header customizado na janela flutuante
                if (win.is_bypassed) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 1.0f, 0.08f, 1.0f));
                }
                if (ImGui::Button("BYPASS", ImVec2(80, 25))) { win.is_bypassed = !win.is_bypassed; }
                ImGui::PopStyleColor();
                
                ImGui::SameLine(ImGui::GetWindowWidth() - 90);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("EXCLUIR", ImVec2(80, 25))) {
                    auto& chain = track_fx_chain[win.track_idx];
                    for (auto it = chain.begin(); it != chain.end(); ++it) {
                        if ((*it)->getId() == win.plugin_id) { chain.erase(it); break; }
                    }
                    win.is_open = false; 
                }
                ImGui::PopStyleColor();
                
                ImGui::Separator();
                ImGui::Spacing();
                
                // Controles de Efeito (DSP)
                if (win.plugin_id == "1") {
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "GOTHIC OVERDRIVE");
                    ImGui::SliderFloat("Drive", &board.param_dark_drive, 1.0f, 10.0f);
                    RenderModulationOverlay(board.active_dark_drive, 1.0f, 10.0f);
                } else if (win.plugin_id == "2") {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "RING MODULATOR");
                    ImGui::SliderFloat("Frequency", &board.param_alien_freq, 20.0f, 1000.0f);
                    RenderModulationOverlay(board.active_alien_freq, 20.0f, 1000.0f);
                } else if (win.plugin_id == "3") {
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.8f, 1.0f), "TREMOLO RITUAL");
                    ImGui::SliderFloat("Rate", &board.param_ritual_rate, 0.1f, 5.0f);
                    RenderModulationOverlay(board.active_ritual_rate, 0.1f, 5.0f);
                    ImGui::SliderFloat("Depth", &board.param_ritual_depth, 0.0f, 1.0f);
                    RenderModulationOverlay(board.active_ritual_depth, 0.0f, 1.0f);
                } else if (win.plugin_id == "4") {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.8f, 1.0f), "PSYCH FLANGER");
                    ImGui::SliderFloat("Speed", &board.param_psych_speed, 0.01f, 2.0f);
                    ImGui::SliderFloat("Depth", &board.param_psych_depth, 0.0f, 1.0f);
                } else if (win.plugin_id == "5") {
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "ABYSS PITCH");
                    ImGui::SliderFloat("Pitch Ratio", &board.abyss_pitch_factor, 0.5f, 2.0f);
                }
            }
            ImGui::End();
        }
    }

    bool show_tutorial = false;

    static void RenderTutorialWindow() {
        if (!show_tutorial) return;

        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.08f, 0.98f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 1.0f, 0.08f, 1.0f));
        
        if (ImGui::Begin("Guia Interativo: Abduction Studio V2", &show_tutorial, ImGuiWindowFlags_NoCollapse)) {
            
            ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "BEM-VINDO AO ABDUCTION STUDIO V2");
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::BeginTabBar("TutorialTabs")) {
                
                if (ImGui::BeginTabItem("1. Visão Geral (Motor DSP)")) {
                    ImGui::Spacing();
                    ImGui::TextWrapped(
                        "O Abduction Studio V2 é alimentado por um poderoso motor de áudio em Grafo (DAG - Directed Acyclic Graph).\n"
                        "Diferente de DAWs antigas que processam pistas em linha reta, nosso motor calcula conexões dinâmicas. "
                        "Isso permite roteamento paralelo invisível e latência ultra-baixa.\n\n"
                        "O motor utiliza cálculos Lock-Free na Thread de Áudio e lida com plugins CLAP nativamente."
                    );
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("2. Mixer e Faixas")) {
                    ImGui::Spacing();
                    ImGui::TextWrapped(
                        "Na Timeline principal, cada faixa possui um cabeçalho completo:\n\n"
                        "- [M] Mute: Silencia a faixa.\n"
                        "- [S] Solo: Isola a faixa.\n"
                        "- [Pan]: Distribui o som entre o fone esquerdo e direito.\n"
                        "- [Vol]: Volume central da faixa.\n\n"
                        "Estes mesmos controles (em formato de faders verticais) podem ser encontrados no Rack inferior na aba 'GLOBAL MIXER'."
                    );
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("3. Roteamento Paralelo (Sends)")) {
                    ImGui::Spacing();
                    ImGui::TextWrapped(
                        "O Abduction Studio já possui Canais de Retorno embutidos (Reverb e Delay).\n"
                        "Em vez de colocar um Reverb em cada pista (o que pesa a CPU), você pode enviar uma cópia do sinal para o canal de retorno global:\n\n"
                        "- [Snd A]: Envia o som desta faixa para a sala global de Reverb.\n"
                        "- [Snd B]: Envia o som desta faixa para a máquina global de Delay.\n\n"
                        "Dica: Durante a reprodução, girar estes controles aplicará uma interpolação matemática suave (Anti-Zipper) para evitar estalos no áudio."
                    );
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("4. Plugins CLAP")) {
                    ImGui::Spacing();
                    ImGui::TextWrapped(
                        "O suporte a VST3 e formatos antigos foi substituído pelo novíssimo padrão CLAP (Clever Audio Plug-in).\n\n"
                        "Como usar:\n"
                        "1. Abra a aba 'Plugins (CLAP)' no painel esquerdo.\n"
                        "2. O sistema buscará arquivos .clap automaticamente nas pastas padrão do seu sistema.\n"
                        "3. Arraste um plugin e solte no botão verde de '+' (Insert Slot) no cabeçalho de qualquer faixa.\n"
                        "4. O motor em Grafo re-roteará o sinal automaticamente para passar pelo plugin!"
                    );
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("5. Automação de Curvas")) {
                    ImGui::Spacing();
                    ImGui::TextWrapped(
                        "O motor suporta desenho vetorial de curvas de automação que controlam os parâmetros em tempo real, em sincronia perfeita com a agulha de reprodução.\n\n"
                        "Como Automar o Volume da Faixa:\n"
                        "- Segure a tecla [ALT] e clique com o Botão Esquerdo no espaço vazio de uma faixa na Timeline para adicionar um Ponto.\n"
                        "- Segure [ALT] e arraste o mouse (Left Click) para mover pontos existentes livremente.\n"
                        "- Segure [ALT] e clique com o Botão Direito sobre um ponto para excluí-lo.\n\n"
                        "As curvas são suavizadas automaticamente pelo motor."
                    );
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("6. Atalhos Essenciais")) {
                    ImGui::Spacing();
                    ImGui::TextWrapped(
                        "[ ESPAÇO ] : Iniciar / Pausar Reprodução (Play/Pause)\n"
                        "[ CTRL + ESPAÇO ] : Stop (Volta a agulha para o início)\n"
                        "[ R ] : Armar gravação de microfone (Recording)\n"
                        "[ L ] : Alternar modo (Song / Pattern)\n\n"
                        "HUD / Janelas:\n"
                        "[ F5 ] : Mostrar/Esconder Playlist (Esquerda)\n"
                        "[ F6 ] : Mostrar/Esconder Step Sequencer\n"
                        "[ F8 ] : Mostrar/Esconder Browser\n"
                        "[ F9 ] : Focar no Global Mixer\n"
                    );
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
        
        ImGui::PopStyleColor(2);
    }

    // Função Raiz que gerencia a janela principal (Fase 24)
    static void RenderMainApp(StemSeparationEngine& ai_engine) {
        RenderMainMenu(ai_engine);

        switch (current_app_mode) {
            case AppMode::STUDIO_MODE:
                RenderStudioMode(ai_engine);
                RenderFloatingPluginWindows();
                break;
            case AppMode::DJ_MODE:
                RenderDJMode();
                break;
            case AppMode::STEM_MODE:
                RenderStemMode();
                break;
        }

        RenderTutorialWindow();
        DawApiExplorerUI::Render(show_daw_api_explorer);

        // ==========================================
        // 5. HUD MODAL DE CARREGAMENTO (FASE 26)
        // ==========================================
        static bool was_running = false;
        if (ai_engine.isRunning() && !was_running) {
            ImGui::OpenPopup("LoadingModal");
        }
        was_running = ai_engine.isRunning();

        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 180));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.05f, 0.15f, 0.05f, 0.95f)); // Transparente esverdeado
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 1.0f, 0.08f, 1.0f));
        if (ImGui::BeginPopupModal("LoadingModal", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "🛸 PROCESSANDO ÁUDIO...");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextWrapped("%s", ai_engine.getStatus().c_str());
            ImGui::Spacing();
            ImGui::ProgressBar(ai_engine.getProgress(), ImVec2(-1, 0));
            if (!ai_engine.isRunning()) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
    }
}
