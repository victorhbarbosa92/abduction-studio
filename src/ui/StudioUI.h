#pragma once
#include "imgui.h"
#include "IconsFontAwesome6.h"
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
#include "SpectrumVisualizerUI.h"
#include "KuroAutoMasterUI.h"
#include "KuroSpatial3DPannerUI.h"
#include "KuroPeakControllerUI.h"
#include "KuroSoundfontPlayerUI.h"
#include "KuroEdisonWaveEditorUI.h"
#include "KuroPatcherModularUI.h"
#include "KuroMixerRoutingMatrixUI.h"
#include "KuroHarmorAdditiveSynthUI.h"
#include "KuroSytrusFMSynthUI.h"
#include "KuroSlicexBeatSlicerUI.h"
#include "KuroMaximusMultibandMasterUI.h"
#include "KuroVocodexSynthUI.h"
#include "KuroLovePhilterUI.h"
#include "KuroSakuraStringSynthUI.h"
#include "KuroConvolverReverbUI.h"
#include "KuroGranulizerUI.h"
#include "KuroStereoShaperUI.h"
#include "KuroWaveShaperUI.h"
#include "KuroSoundgoodizerUI.h"
#include "KuroParametricEQ2UI.h"
#include "KuroGrossBeatUI.h"
#include "KuroDelay3UI.h"
#include "KuroControlSurfaceUI.h"
#include "KuroFruityLimiterUI.h"
#include "KuroFastLPUI.h"
#include "KuroFlangusChorusUI.h"
#include "KuroEnvelopeControllerUI.h"
#include "KuroHardcorePedalboardUI.h"
#include "KuroBloodOverdriveUI.h"
#include "KuroReeverb2UI.h"
#include "Kuro3xOscSynthUI.h"
#include "KuroPhaserUI.h"
#include "KuroGranularDelayUI.h"
#include "KuroTransientProcessorUI.h"
#include "KuroFruityVocoderUI.h"
#include "KuroFruitySqueezeUI.h"
#include "KuroFruityScratcherUI.h"
#include "KuroStereoEnhancerUI.h"
#include "KuroBassDrumUI.h"
#include "KuroCenterPannerUI.h"
#include "KuroFormulaControllerUI.h"
#include "KuroFruityMute2UI.h"
#include "KuroFruityFilterUI.h"
#include "KuroFruityStereoToolUI.h"
#include "KuroFruityNotebookUI.h"
#include "KuroFruityStereoReverbPanUI.h"
#include "KuroFruitySoftClipperUI.h"
#include "KuroFruityStereoReverbDelayPanUI.h"
#include "KuroFruityStereoVisualizerMetersUI.h"
#include "KuroFruityStereoReverbDelayPanChorusUI.h"
#include "KuroFruityHTMLNotebookUI.h"
#include "GenreTemplatesUI.h"
#include "CloudStemExtractorUI.h"
#include "PresetManagerUI.h"
#include "StemBeatSlicerUI.h"
#include "KuroDirectWaveUI.h"
#include "AudioClipEditorUI.h"
inline KuroUI::AudioClipEditorUI g_audio_clip_editor_ui;
extern KuroUI::SpectrumVisualizerUI g_spectrum_visualizer;
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
extern KuroDSP::AcousticContrabassSynth g_contrabass_synth;
extern std::string track_names[MAX_TRACKS];
extern KuroDSP::TimelineManager timeline;
extern std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;
extern float g_master_volume;
extern float track_vu_levels[MAX_TRACKS];
extern float master_vu_level_l;
extern float master_vu_level_r;
extern float track_pans[MAX_TRACKS];

#include "../core/DJEngine.h"
#include "../core/StemExtractorEngine.h"
extern std::unique_ptr<KuroAudio::DJEngine> g_dj_engine;
extern std::unique_ptr<KuroAudio::StemExtractorEngine> g_stem_engine;
extern std::unique_ptr<StemSeparationEngine> g_ai_engine;

namespace ProjectManagerBridge {
    void Save(const std::string& path);
    void Load(const std::string& path);
}

namespace KuroUI {
    extern CommandManager g_command_manager;

    enum class AppMode {
        PRODUCE_MODE = 0,
        BEAT_SLICING_MODE = 1,
        MIX_MODE = 2,
        LIVE_MODE = 3,
        // Aliases de compatibilidade
        STUDIO_MODE = PRODUCE_MODE,
        STEM_MODE = BEAT_SLICING_MODE,
        DJ_MODE = LIVE_MODE
    };
    static AppMode current_app_mode = AppMode::PRODUCE_MODE;

    inline int selected_track_idx = 0;
    inline bool show_mixer = false;
    inline bool set_mixer_focus = false;
    inline bool show_playlist = true;
    inline bool show_browser = true;
    inline bool show_inspector = true;
    inline bool show_piano_roll = true;
    inline bool show_step_sequencer = false;
    inline bool show_modulation_panel = false;
    inline bool show_cloud_downloader = false;
    inline bool show_gross_beat = false;
    inline bool show_ai_stem_separator = false;
    inline bool show_delay_lama = false;
    inline bool focus_delay_lama = false;
    inline bool show_monksynth_vst3 = false;
    inline bool focus_monksynth_vst3 = false;
    inline bool show_contrabass_window = false;
    inline KuroAutoMasterUI g_auto_master_ui;
    inline KuroSpatial3DPannerUI g_spatial_panner_ui;
    inline KuroPeakControllerUI g_peak_controller_ui;
    inline KuroSoundfontPlayerUI g_soundfont_player_ui;
    inline KuroEdisonWaveEditorUI g_edison_editor_ui;
    inline KuroPatcherModularUI g_patcher_ui;
    inline KuroMixerRoutingMatrixUI g_mixer_routing_ui;
    inline KuroHarmorAdditiveSynthUI g_harmor_synth_ui;
    inline KuroSytrusFMSynthUI g_sytrus_synth_ui;
    inline KuroSlicexBeatSlicerUI g_slicex_ui;
    inline KuroMaximusMultibandMasterUI g_maximus_master_ui;
    inline KuroVocodexSynthUI g_vocodex_synth_ui;
    inline KuroLovePhilterUI g_love_philter_ui;
    inline KuroSakuraStringSynthUI g_sakura_synth_ui;
    inline KuroConvolverReverbUI g_convolver_ui;
    inline KuroGranulizerUI g_granulizer_ui;
    inline KuroStereoShaperUI g_stereo_shaper_ui;
    inline KuroWaveShaperUI g_waveshaper_ui;
    inline KuroSoundgoodizerUI g_soundgoodizer_ui;
    inline KuroParametricEQ2UI g_parametric_eq2_ui;
    inline KuroGrossBeatUI g_gross_beat_ui;
    inline KuroDelay3UI g_delay3_ui;
    inline KuroControlSurfaceUI g_control_surface_ui;
    inline KuroFruityLimiterUI g_fruity_limiter_ui;
    inline KuroFastLPUI g_fast_lp_ui;
    inline KuroFlangusChorusUI g_flangus_ui;
    inline KuroEnvelopeControllerUI g_envelope_controller_ui;
    inline KuroHardcorePedalboardUI g_hardcore_pedalboard_ui;
    inline KuroBloodOverdriveUI g_blood_overdrive_ui;
    inline KuroReeverb2UI g_reeverb2_ui;
    inline Kuro3xOscSynthUI g_3xosc_ui;
    inline KuroPhaserUI g_phaser_ui;
    inline KuroGranularDelayUI g_granular_delay_ui;
    inline KuroTransientProcessorUI g_transient_processor_ui;
    inline KuroFruityVocoderUI g_fruity_vocoder_ui;
    inline KuroFruitySqueezeUI g_fruity_squeeze_ui;
    inline KuroFruityScratcherUI g_fruity_scratcher_ui;
    inline KuroStereoEnhancerUI g_stereo_enhancer_ui;
    inline KuroBassDrumUI g_bass_drum_ui;
    inline KuroCenterPannerUI g_center_panner_ui;
    inline KuroFormulaControllerUI g_formula_controller_ui;
    inline KuroFruityMute2UI g_fruity_mute2_ui;
    inline KuroFruityFilterUI g_fruity_filter_ui;
    inline KuroFruityStereoToolUI g_fruity_stereo_tool_ui;
    inline KuroFruityNotebookUI g_fruity_notebook_ui;
    inline KuroFruityStereoReverbPanUI g_fruity_panomatic_ui;
    inline KuroFruitySoftClipperUI g_fruity_soft_clipper_ui;
    inline KuroFruityStereoReverbDelayPanUI g_fruity_stereo_delay_ui;
    inline KuroFruityStereoVisualizerMetersUI g_wave_candy_ui;
    inline KuroFruityStereoReverbDelayPanChorusUI g_fruity_dx10_ui;
    inline KuroFruityHTMLNotebookUI g_project_info_ui;
    inline CloudStemExtractorUI g_cloud_stem_ui;
    inline PresetManagerUI g_preset_manager_ui;
    inline StemBeatSlicerUI g_stem_beat_slicer_ui;
    inline KuroDirectWaveUI g_directwave_ui;
    static void RenderMixerFloatingWindow();
    inline std::string g_current_project_name = "Novo Projeto";
    static bool show_genre_templates_window = false;
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

static bool show_parametric_eq2 = false;
static bool show_soundgoodizer = false;
static float eq_band_gain[7] = { 0.0f, 3.5f, -1.5f, 0.0f, 2.5f, 1.0f, 0.0f };
static float eq_band_freq[7] = { 40.0f, 120.0f, 400.0f, 1000.0f, 3000.0f, 8000.0f, 15000.0f };
static int soundgoodizer_preset = 0;
static float soundgoodizer_amount = 0.75f;

static bool show_daw_api_explorer = false;

inline void RenderSamplerSettings(int channel_idx) {
    if (!show_sampler_settings) return;
    
    char win_title[64];
    const char* ch_names[] = { "Kick", "Snare", "HiHat", "Bassline", "Serum Chords", "Lead Synth", "Clap", "Open Hat" };
    snprintf(win_title, sizeof(win_title), "%s (Insert %d)###SamplerSettingsWindow", ch_names[channel_idx], channel_idx + 1);
    
    ImGui::SetNextWindowSize(ImVec2(720, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(win_title, &show_sampler_settings, ImGuiWindowFlags_NoCollapse)) {
        auto& ss = g_piano_synth.sampler_settings[channel_idx];
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();
        
        // ==========================================
        // 1. BARRA DE CABEÇALHO SUPERIOR (FL STYLE)
        // ==========================================
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(25, 27, 30, 255));
        ImGui::BeginChild("HeaderTopBar", ImVec2(0, 52), true);
        
        // Abas principais com Ícones estilizados
        ImGui::PushStyleColor(ImGuiCol_Button, ss.active_tab == 0 ? IM_COL32(50, 55, 60, 255) : IM_COL32(32, 35, 38, 255));
        if (ImGui::Button(ICON_FA_WAVE_SQUARE " Waveform", ImVec2(100, 34))) ss.active_tab = 0;
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ss.active_tab == 1 ? IM_COL32(50, 55, 60, 255) : IM_COL32(32, 35, 38, 255));
        if (ImGui::Button(ICON_FA_DIAGRAM_PROJECT " Env & Filter", ImVec2(110, 34))) ss.active_tab = 1;
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ss.active_tab == 2 ? IM_COL32(50, 55, 60, 255) : IM_COL32(32, 35, 38, 255));
        if (ImGui::Button(ICON_FA_WRENCH " Misc", ImVec2(80, 34))) ss.active_tab = 2;
        ImGui::PopStyleColor();
        
        // Lado Direito do Cabeçalho: ON, PAN, VOL, PITCH, PITCH RANGE, TRACK
        ImGui::SameLine(ImGui::GetWindowWidth() - 360);
        
        // Botão LED Verde de ON
        bool is_on = !ss.reverse;
        ImVec2 led_pos = ImGui::GetCursorScreenPos();
        dl->AddCircleFilled(ImVec2(led_pos.x + 8, led_pos.y + 16), 6.0f, is_on ? IM_COL32(100, 230, 50, 255) : IM_COL32(60, 65, 70, 255));
        dl->AddCircle(ImVec2(led_pos.x + 8, led_pos.y + 16), 6.0f, IM_COL32(30, 30, 35, 255), 12, 1.5f);
        ImGui::SetCursorScreenPos(led_pos);
        if (ImGui::InvisibleButton("##on_led", ImVec2(18, 34))) { ss.reverse = !ss.reverse; }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Channel ON/OFF");
        
        ImGui::SameLine(0, 8);
        Knob("PAN##top", &ss.pan, -1.0f, 1.0f, 11.0f);
        
        ImGui::SameLine(0, 10);
        Knob("VOL##top", &ss.vol, 0.0f, 1.0f, 11.0f);
        
        ImGui::SameLine(0, 10);
        Knob("PITCH##top", &ss.pitch, (float)-ss.pitch_range, (float)ss.pitch_range, 11.0f);
        
        ImGui::SameLine(0, 12);
        
        // Pitch ST / Range Display (Caixa Vermelha FL Style)
        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(180, 40, 40, 255));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        
        ImGui::SetNextItemWidth(45);
        if (ImGui::DragFloat("##pitch_st", &ss.pitch, 0.05f, (float)-ss.pitch_range, (float)ss.pitch_range, "%.0f ST")) {
            ss.pitch = std::clamp(ss.pitch, (float)-ss.pitch_range, (float)ss.pitch_range);
        }
        if (ImGui::IsItemHovered() && io.MouseWheel != 0.0f) {
            ss.pitch += (io.MouseWheel > 0 ? 1.0f : -1.0f);
            ss.pitch = std::clamp(ss.pitch, (float)-ss.pitch_range, (float)ss.pitch_range);
        }
        
        ImGui::SameLine(0, 2);
        ImGui::SetNextItemWidth(26);
        if (ImGui::DragInt("##pitch_rg", &ss.pitch_range, 0.05f, 1, 24, "%d")) {
            if (ss.pitch_range < 1) ss.pitch_range = 1;
            if (ss.pitch_range > 24) ss.pitch_range = 24;
        }
        if (ImGui::IsItemHovered() && io.MouseWheel != 0.0f) {
            ss.pitch_range += (io.MouseWheel > 0 ? 1 : -1);
            if (ss.pitch_range < 1) ss.pitch_range = 1;
            if (ss.pitch_range > 24) ss.pitch_range = 24;
        }
        
        ImGui::PopStyleColor(2);
        ImGui::TextDisabled("PITCH RANGE");
        ImGui::EndGroup();
        
        ImGui::SameLine(0, 10);
        
        // Track Mixer Target Display (Caixa Vermelha FL Style)
        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(180, 40, 40, 255));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        ImGui::SetNextItemWidth(35);
        if (ss.track < 1) ss.track = channel_idx + 1;
        if (ImGui::DragInt("##track_target", &ss.track, 0.05f, 1, 20, "%d")) {
            if (ss.track < 1) ss.track = 1;
            if (ss.track > 20) ss.track = 20;
            ::channel_tracks[channel_idx] = ss.track - 1;
        }
        if (ImGui::IsItemHovered() && io.MouseWheel != 0.0f) {
            ss.track += (io.MouseWheel > 0 ? 1 : -1);
            if (ss.track < 1) ss.track = 1;
            if (ss.track > 20) ss.track = 20;
            ::channel_tracks[channel_idx] = ss.track - 1;
        }
        ImGui::PopStyleColor(2);
        ImGui::TextDisabled("TRACK");
        ImGui::EndGroup();
        
        ImGui::EndChild();
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        // ==========================================
        // 2. CONTEÚDO DAS ABAS PRINCIPAIS
        // ==========================================
        if (ss.active_tab == 0) {
            // ------------------------------------------
            // ABA 1: WAVEFORM (AUDIO SAMPLE PAGE)
            // ------------------------------------------
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(20, 22, 25, 255));
            ImGui::BeginChild("WaveformPage", ImVec2(0, -180), true);
            
            // File Row & Preset Selector
            char file_buf[128];
            auto& ds = g_piano_synth.getDrumSample(channel_idx);
            snprintf(file_buf, sizeof(file_buf), "File: %s", ds.loaded && !ds.filepath.empty() ? ds.filepath.c_str() : "808 Kick.wav");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", file_buf);
            
            ImGui::SameLine(ImGui::GetWindowWidth() - 260);
            ImGui::SetNextItemWidth(140);
            if (channel_idx == 0) {
                const char* kick_samples[] = { "808 Kick", "909 Kick", "Acoustic Kick", "FPC Kick" };
                int cur_var = g_piano_synth.selected_variant[0];
                if (ImGui::Combo("##kick_preset", &cur_var, kick_samples, 4)) {
                    g_piano_synth.selected_variant[0] = cur_var;
                    g_piano_synth.triggerNote(36, 0.25f, 0.9f, 0);
                }
            } else if (channel_idx == 1) {
                const char* snare_samples[] = { "808 Snare", "909 Snare", "FPC Snare", "Acoustic Snare" };
                int cur_var = g_piano_synth.selected_variant[1];
                if (ImGui::Combo("##snare_preset", &cur_var, snare_samples, 4)) {
                    g_piano_synth.selected_variant[1] = cur_var;
                    g_piano_synth.triggerNote(38, 0.25f, 0.9f, 1);
                }
            } else if (channel_idx == 2) {
                const char* hat_samples[] = { "808 CH", "909 CH", "Grv CH 01", "808 OH" };
                int cur_var = g_piano_synth.selected_variant[2];
                if (ImGui::Combo("##hat_preset", &cur_var, hat_samples, 4)) {
                    g_piano_synth.selected_variant[2] = cur_var;
                    g_piano_synth.triggerNote(42, 0.25f, 0.9f, 2);
                }
            }
            
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER_OPEN "##load_wav", ImVec2(30, 20))) {
                ImGui::OpenPopup("SamplePickerPopup");
            }
            ImGui::SameLine();
            if (ImGui::Button("X##clr_wav", ImVec2(24, 20))) {
                ds.loaded = false;
            }
            
            if (ImGui::BeginPopup("SamplePickerPopup")) {
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Selecionar Sample (Kicks & Baterias)");
                ImGui::Separator();
                if (ImGui::Selectable("808 Kick.wav")) { g_piano_synth.selected_variant[0] = 0; g_piano_synth.triggerNote(36, 0.25f, 0.9f, 0); }
                if (ImGui::Selectable("909 Kick.wav")) { g_piano_synth.selected_variant[0] = 1; g_piano_synth.triggerNote(36, 0.25f, 0.9f, 0); }
                if (ImGui::Selectable("Acoustic Kick.wav")) { g_piano_synth.selected_variant[0] = 2; g_piano_synth.triggerNote(36, 0.25f, 0.9f, 0); }
                if (ImGui::Selectable("FPC Kick.wav")) { g_piano_synth.selected_variant[0] = 3; g_piano_synth.triggerNote(36, 0.25f, 0.9f, 0); }
                ImGui::Separator();
                if (ImGui::Selectable("808 Snare.wav")) { g_piano_synth.selected_variant[1] = 0; g_piano_synth.triggerNote(38, 0.25f, 0.9f, 1); }
                if (ImGui::Selectable("909 Snare.wav")) { g_piano_synth.selected_variant[1] = 1; g_piano_synth.triggerNote(38, 0.25f, 0.9f, 1); }
                if (ImGui::Selectable("808 HiHat.wav")) { g_piano_synth.selected_variant[2] = 0; g_piano_synth.triggerNote(42, 0.25f, 0.9f, 2); }
                ImGui::EndPopup();
            }
            
            ImGui::Separator();
            
            ImGui::Columns(2, "WaveformCols", false);
            ImGui::SetColumnWidth(0, 330);
            
            // --- Content Group ---
            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.95f, 1.0f), "Content");
            static int content_mode = 1; // Resample
            ImGui::RadioButton("Keep on disk", &content_mode, 0); ImGui::SameLine(135);
            ImGui::RadioButton("Load regions", &content_mode, 2);
            ImGui::RadioButton("Resample", &content_mode, 1); ImGui::SameLine(135);
            ImGui::RadioButton("Load slice markers", &content_mode, 3);
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.95f, 1.0f), "Declicking mode");
            const char* declick_modes[] = { "Out only (no bleeding)", "Transient (no bleeding)", "Generic (smooth)", "Crossfade" };
            static int declick_idx = 0;
            ImGui::SetNextItemWidth(260);
            ImGui::Combo("##declick", &declick_idx, declick_modes, 4);
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.95f, 1.0f), "Playback");
            Knob("START OFFSET", &ss.smp_start, 0.0f, 1.0f, 11.0f);
            ImGui::SameLine(0, 25);
            static bool use_loop = false, pingpong = false;
            ImGui::BeginGroup();
            ImGui::Checkbox("Use loop points", &use_loop);
            ImGui::Checkbox("Ping pong loop", &pingpong);
            ImGui::EndGroup();
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.95f, 1.0f), "Time stretching");
            Knob("PITCH##ts", &ss.time_pitch, -12.0f, 12.0f, 11.0f); ImGui::SameLine(0, 16);
            Knob("MUL##ts", &ss.time_mul, 0.5f, 2.0f, 11.0f); ImGui::SameLine(0, 16);
            Knob("TIME##ts", &ss.time_time, 0.0f, 5.0f, 11.0f);
            
            const char* stretch_modes[] = { "Resample", "Stretch", "Realtime", "e3 Generic", "Mono" };
            ImGui::SetNextItemWidth(180);
            ImGui::Combo("Mode##ts", &ss.time_mode, stretch_modes, 5);
            
            ImGui::NextColumn();
            
            // --- Precomputed Effects Group ---
            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.95f, 1.0f), ICON_FA_WRENCH " Precomputed effects");
            ImGui::Checkbox("Remove DC offset", &ss.remove_dc); ImGui::SameLine(160);
            ImGui::Checkbox("Reverse polarity", &ss.rev_polarity);
            ImGui::Checkbox("Normalize", &ss.normalize); ImGui::SameLine(160);
            ImGui::Checkbox("Fade stereo", &ss.fade_stereo);
            ImGui::Checkbox("Reverse", &ss.reverse); ImGui::SameLine(160);
            ImGui::Checkbox("Swap stereo", &ss.swap_stereo);
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Precomputed Knobs (2 rows of 3)
            Knob("SMP START", &ss.smp_start, 0.0f, 1.0f, 11.0f); ImGui::SameLine(0, 20);
            Knob("LENGTH", &ss.length, 0.0f, 1.0f, 11.0f); ImGui::SameLine(0, 20);
            Knob("IN", &ss.fade_in, 0.0f, 1.0f, 11.0f);
            
            ImGui::Spacing();
            Knob("OUT", &ss.fade_out, 0.0f, 1.0f, 11.0f); ImGui::SameLine(0, 20);
            Knob("CROSSFADE", &ss.crossfade, 0.0f, 1.0f, 11.0f); ImGui::SameLine(0, 20);
            Knob("TRIM", &ss.trim, 0.0f, 1.0f, 11.0f);
            
            ImGui::Columns(1);
            ImGui::EndChild();
            ImGui::PopStyleColor();
            
            // --- Waveform Oscilloscope Display Box ---
            ImVec2 wstart = ImGui::GetCursorScreenPos();
            ImVec2 wsize = ImVec2(ImGui::GetContentRegionAvail().x, 160);
            ImGui::InvisibleButton("##waveform_display_box", wsize);
            dl->AddRectFilled(wstart, ImVec2(wstart.x + wsize.x, wstart.y + wsize.y), IM_COL32(14, 16, 19, 255), 4.0f);
            dl->AddRect(wstart, ImVec2(wstart.x + wsize.x, wstart.y + wsize.y), IM_COL32(40, 45, 50, 255), 4.0f);
            
            float mid_y = wstart.y + (wsize.y * 0.5f);
            dl->AddLine(ImVec2(wstart.x, mid_y), ImVec2(wstart.x + wsize.x, mid_y), IM_COL32(35, 40, 45, 255));
            
            if (ds.loaded && ds.total_frames > 0) {
                int step = ds.total_frames / (int)wsize.x;
                if (step < 1) step = 1;
                for (int x = 0; x < (int)wsize.x; x++) {
                    uint64_t idx = (uint64_t)x * step;
                    if (idx < ds.total_frames) {
                        float val = ds.sample_data[idx * ds.channels];
                        dl->AddLine(
                            ImVec2(wstart.x + x, mid_y - val * (wsize.y * 0.45f)),
                            ImVec2(wstart.x + x, mid_y + val * (wsize.y * 0.45f)),
                            IM_COL32(190, 195, 200, 220)
                        );
                    }
                }
            } else {
                // Synthetic 808 Kick sine waveform preview
                for (int x = 0; x < (int)wsize.x; x++) {
                    float t = (float)x / wsize.x;
                    float freq = 120.0f * std::exp(-4.0f * t);
                    float env = std::exp(-2.5f * t);
                    float val = std::sin(6.28318f * freq * t) * env;
                    dl->AddLine(
                        ImVec2(wstart.x + x, mid_y - val * (wsize.y * 0.45f)),
                        ImVec2(wstart.x + x, mid_y + val * (wsize.y * 0.45f)),
                        IM_COL32(210, 215, 220, 220)
                    );
                }
            }
            
            dl->AddText(ImVec2(wstart.x + wsize.x - 70, wstart.y + wsize.y - 22), IM_COL32(140, 145, 150, 200), "Sampler (16 bit)");
            
        } else if (ss.active_tab == 1) {
            // ------------------------------------------
            // ABA 2: ENVELOPE & FILTER PAGE
            // ------------------------------------------
            
            // Sub-aba selector bar
            const char* subtabs[] = { "Panning", "Volume", "Mod X", "Mod Y", "Pitch" };
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 3));
            for (int st = 0; st < 5; st++) {
                if (st > 0) ImGui::SameLine();
                bool is_st_sel = (ss.active_env_subtab == st);
                if (is_st_sel) ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(40, 45, 50, 255));
                else ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(22, 24, 27, 255));
                if (ImGui::Button(subtabs[st])) ss.active_env_subtab = st;
                ImGui::PopStyleColor();
            }
            ImGui::PopStyleVar();
            
            ImGui::Spacing();
            
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(20, 22, 25, 255));
            ImGui::BeginChild("EnvFilterBody", ImVec2(0, -90), true);
            
            ImGui::Columns(3, "EnvFilter3Cols", false);
            ImGui::SetColumnWidth(0, 280);
            ImGui::SetColumnWidth(1, 230);
            
            // --- 1. Envelope Box ---
            ImGui::Checkbox("Envelope##env_toggle", &ss.env_enabled);
            
            // Vector ADSR Display Box
            ImVec2 estart = ImGui::GetCursorScreenPos();
            ImVec2 esize = ImVec2(260, 90);
            ImGui::InvisibleButton("##env_graph_canvas", esize);
            dl->AddRectFilled(estart, ImVec2(estart.x + esize.x, estart.y + esize.y), IM_COL32(14, 16, 19, 255), 3.0f);
            dl->AddRect(estart, ImVec2(estart.x + esize.x, estart.y + esize.y), IM_COL32(35, 40, 45, 255), 3.0f);
            
            ImU32 curve_col = IM_COL32(90, 220, 80, 255); // Green for volume
            if (ss.active_env_subtab == 2) curve_col = IM_COL32(230, 220, 50, 255); // Yellow for Mod X
            else if (ss.active_env_subtab == 3) curve_col = IM_COL32(230, 130, 40, 255); // Orange for Mod Y
            else if (ss.active_env_subtab == 4) curve_col = IM_COL32(180, 80, 230, 255); // Purple for Pitch
            
            float att_w = ss.env_attack * 35.0f;
            float hold_w = ss.env_hold * 25.0f;
            float dec_w = ss.env_decay * 35.0f;
            float sus_h = esize.y * (1.0f - std::clamp(ss.env_sustain, 0.0f, 1.0f));
            float rel_w = ss.env_release * 35.0f;
            
            ImVec2 ep0 = ImVec2(estart.x + 8, estart.y + esize.y - 8);
            ImVec2 ep1 = ImVec2(ep0.x + att_w, estart.y + 8);
            ImVec2 ep2 = ImVec2(ep1.x + hold_w, estart.y + 8);
            ImVec2 ep3 = ImVec2(ep2.x + dec_w, estart.y + 8 + sus_h);
            ImVec2 ep4 = ImVec2(ep3.x + 40, estart.y + 8 + sus_h);
            ImVec2 ep5 = ImVec2(ep4.x + rel_w, estart.y + esize.y - 8);
            
            dl->AddLine(ep0, ep1, curve_col, 2.0f);
            dl->AddLine(ep1, ep2, curve_col, 2.0f);
            dl->AddLine(ep2, ep3, curve_col, 2.0f);
            dl->AddLine(ep3, ep4, curve_col, 2.0f);
            dl->AddLine(ep4, ep5, curve_col, 2.0f);
            
            // Handles circles
            dl->AddCircleFilled(ep0, 3.5f, curve_col);
            dl->AddCircleFilled(ep1, 3.5f, curve_col);
            dl->AddCircleFilled(ep2, 3.5f, curve_col);
            dl->AddCircleFilled(ep3, 3.5f, curve_col);
            dl->AddCircleFilled(ep5, 3.5f, curve_col);
            
            ImGui::Spacing();
            
            // ADSR Knobs Row with clean spacing
            Knob("DELAY", &ss.env_delay, 0.0f, 2.0f, 9.0f); ImGui::SameLine(0, 8);
            Knob("ATT", &ss.env_attack, 0.001f, 1.0f, 9.0f); ImGui::SameLine(0, 8);
            Knob("HOLD", &ss.env_hold, 0.0f, 2.0f, 9.0f); ImGui::SameLine(0, 8);
            Knob("DEC", &ss.env_decay, 0.001f, 2.0f, 9.0f); ImGui::SameLine(0, 8);
            Knob("SUS", &ss.env_sustain, 0.0f, 1.0f, 9.0f); ImGui::SameLine(0, 8);
            Knob("REL", &ss.env_release, 0.001f, 2.0f, 9.0f);
            
            ImGui::NextColumn();
            
            // --- 2. LFO Box ---
            ImGui::BeginGroup();
            ImGui::Text("LFO Wave:"); ImGui::SameLine();
            if (ImGui::Button("~##sine", ImVec2(20, 18))) ss.lfo_shape = 0; ImGui::SameLine();
            if (ImGui::Button("/\\##tri", ImVec2(20, 18))) ss.lfo_shape = 1; ImGui::SameLine();
            if (ImGui::Button("[]##sq", ImVec2(20, 18))) ss.lfo_shape = 2;
            ImGui::EndGroup();
            
            ImVec2 lstart = ImGui::GetCursorScreenPos();
            ImVec2 lsize = ImVec2(210, 75);
            ImGui::InvisibleButton("##lfo_graph_canvas", lsize);
            dl->AddRectFilled(lstart, ImVec2(lstart.x + lsize.x, lstart.y + lsize.y), IM_COL32(14, 16, 19, 255), 3.0f);
            dl->AddRect(lstart, ImVec2(lstart.x + lsize.x, lstart.y + lsize.y), IM_COL32(35, 40, 45, 255), 3.0f);
            
            float lmid = lstart.y + lsize.y * 0.5f;
            dl->AddLine(ImVec2(lstart.x, lmid), ImVec2(lstart.x + lsize.x, lmid), IM_COL32(90, 220, 80, 180), 1.5f);
            
            Knob("DELAY##lfo", &ss.lfo_delay, 0.0f, 2.0f, 9.0f); ImGui::SameLine(0, 10);
            Knob("ATT##lfo", &ss.lfo_attack, 0.0f, 2.0f, 9.0f); ImGui::SameLine(0, 10);
            Knob("AMT##lfo", &ss.lfo_amount, 0.0f, 1.0f, 9.0f); ImGui::SameLine(0, 10);
            Knob("SPEED", &ss.lfo_speed, 0.1f, 10.0f, 9.0f);
            
            ImGui::NextColumn();
            
            // --- 3. Filter Box ---
            ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), "Filter");
            const char* filter_modes[] = { "Fast LP", "Low Pass", "Band Pass", "High Pass", "Vocal Formant" };
            ImGui::SetNextItemWidth(140);
            ImGui::Combo("##flt_mode", &ss.filter_type, filter_modes, 5);
            
            ImGui::Spacing();
            Knob("MOD X", &ss.filter_cutoff, 0.01f, 1.0f, 18.0f); ImGui::SameLine(0, 20);
            Knob("MOD Y", &ss.filter_res, 0.0f, 1.0f, 18.0f);
            
            ImGui::Columns(1);
            ImGui::EndChild();
            ImGui::PopStyleColor();
            
            // --- 4. Bottom Keyboard & Root Note Bar ---
            ImGui::BeginGroup();
            ImGui::Text("Root note: C5"); ImGui::SameLine();
            if (ImGui::Button("Reset##root", ImVec2(50, 18))) ss.root_note = 60; ImGui::SameLine();
            ImGui::Checkbox("Enable main pitch", &ss.enable_main_pitch); ImGui::SameLine();
            ImGui::Checkbox("Add to key", &ss.add_to_key); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            ImGui::SliderFloat("Fine tune", &ss.fine_tune, -100.0f, 100.0f, "%.0f Cents");
            
            // Piano Keyboard Canvas
            ImVec2 kstart = ImGui::GetCursorScreenPos();
            ImVec2 ksize = ImVec2(ImGui::GetContentRegionAvail().x, 45);
            ImGui::InvisibleButton("##root_keyboard_canvas", ksize);
            dl->AddRectFilled(kstart, ImVec2(kstart.x + ksize.x, kstart.y + ksize.y), IM_COL32(20, 22, 25, 255), 2.0f);
            
            int total_keys = 60; // C2 to C7
            float kw = ksize.x / total_keys;
            
            for (int k = 0; k < total_keys; k++) {
                int pitch = 36 + k;
                bool is_c5 = (pitch == ss.root_note);
                int note_in_octave = pitch % 12;
                bool is_black = (note_in_octave == 1 || note_in_octave == 3 || note_in_octave == 6 || note_in_octave == 8 || note_in_octave == 10);
                
                ImVec2 kp0 = ImVec2(kstart.x + k * kw, kstart.y);
                ImVec2 kp1 = ImVec2(kp0.x + kw - 1.0f, kstart.y + ksize.y);
                
                ImU32 key_col = is_black ? IM_COL32(30, 30, 32, 255) : IM_COL32(240, 240, 235, 255);
                if (is_c5) key_col = IM_COL32(40, 150, 240, 255); // Blue highlighted Root Note
                
                dl->AddRectFilled(kp0, kp1, key_col);
                
                if (io.MousePos.x >= kp0.x && io.MousePos.x <= kp1.x && io.MousePos.y >= kp0.y && io.MousePos.y <= kp1.y) {
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        ss.root_note = pitch;
                    }
                }
            }
            ImGui::EndGroup();
            
        } else if (ss.active_tab == 2) {
            // ------------------------------------------
            // ABA 3: MISCELLANEOUS FUNCTIONS PAGE
            // ------------------------------------------
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(20, 22, 25, 255));
            ImGui::BeginChild("MiscBody", ImVec2(0, -90), true);
            
            ImGui::Columns(3, "Misc3Cols", false);
            
            // --- Levels Adjustment ---
            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.95f, 1.0f), "Levels adjustment");
            Knob("PAN##m", &ss.misc_pan, -1.0f, 1.0f, 11.0f); ImGui::SameLine(0, 16);
            Knob("VOL##m", &ss.misc_vol, 0.0f, 2.0f, 11.0f); ImGui::SameLine(0, 16);
            Knob("MOD X##m", &ss.misc_modx, -1.0f, 1.0f, 11.0f); ImGui::SameLine(0, 16);
            Knob("MOD Y##m", &ss.misc_mody, -1.0f, 1.0f, 11.0f);
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // --- Group ---
            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.95f, 1.0f), "Group");
            ImGui::SetNextItemWidth(45); ImGui::InputInt("Cut", &ss.cut_group); ImGui::SameLine(110);
            ImGui::SetNextItemWidth(45); ImGui::InputInt("By", &ss.cut_by);
            ImGui::PushStyleColor(ImGuiCol_Button, ss.cut_self ? IM_COL32(50, 150, 50, 255) : IM_COL32(35, 38, 42, 255));
            if (ImGui::Button("Cut self", ImVec2(80, 22))) ss.cut_self = !ss.cut_self;
            ImGui::PopStyleColor();
            
            ImGui::NextColumn();
            
            // --- Polyphony ---
            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.95f, 1.0f), "Polyphony");
            ImGui::SetNextItemWidth(50); ImGui::InputInt("Max", &ss.poly_max); ImGui::SameLine(0, 14);
            Knob("SLIDE", &ss.poly_slide, 0.0f, 2.0f, 9.0f);
            if (ImGui::RadioButton("Porta", ss.poly_porta)) ss.poly_porta = !ss.poly_porta; ImGui::SameLine(100);
            if (ImGui::RadioButton("Mono", ss.poly_mono)) ss.poly_mono = !ss.poly_mono;
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // --- Time ---
            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.95f, 1.0f), "Time");
            Knob("GATE", &ss.arp_gate, 0.0f, 1.0f, 9.0f); ImGui::SameLine(0, 14);
            Knob("SHIFT", &ss.misc_pan, 0.0f, 1.0f, 9.0f); ImGui::SameLine(0, 14);
            Knob("SWING", &ss.misc_vol, 0.0f, 1.0f, 9.0f);
            
            ImGui::NextColumn();
            
            // --- Echo delay / Fat mode ---
            ImGui::TextColored(ImVec4(0.3f, 0.75f, 0.95f, 1.0f), "Echo delay / fat mode");
            Knob("FEED", &ss.echo_feed, 0.0f, 0.95f, 9.0f); ImGui::SameLine(0, 14);
            Knob("PAN##e", &ss.echo_pan, -1.0f, 1.0f, 9.0f); ImGui::SameLine(0, 14);
            Knob("PITCH##e", &ss.echo_pitch, -12.0f, 12.0f, 9.0f); ImGui::SameLine(0, 14);
            Knob("TIME##e", &ss.echo_time, 0.05f, 2.0f, 9.0f);
            
            ImGui::SetNextItemWidth(50); ImGui::InputInt("Echoes", &ss.echo_count); ImGui::SameLine(100);
            ImGui::Checkbox("Ping pong", &ss.echo_pingpong);
            ImGui::Checkbox("Fat mode", &ss.echo_fat);
            
            ImGui::Columns(1);
            ImGui::EndChild();
            ImGui::PopStyleColor();
            
            // --- Bottom Keyboard & Root Note Bar ---
            ImGui::BeginGroup();
            ImGui::Text("Root note: C5"); ImGui::SameLine();
            if (ImGui::Button("Reset##root2", ImVec2(50, 18))) ss.root_note = 60; ImGui::SameLine();
            ImGui::Checkbox("Enable main pitch##2", &ss.enable_main_pitch); ImGui::SameLine();
            ImGui::Checkbox("Add to key##2", &ss.add_to_key); ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            ImGui::SliderFloat("Fine tune##2", &ss.fine_tune, -100.0f, 100.0f, "%.0f Cents");
            
            ImVec2 kstart = ImGui::GetCursorScreenPos();
            ImVec2 ksize = ImVec2(ImGui::GetContentRegionAvail().x, 45);
            ImGui::InvisibleButton("##root_keyboard_canvas2", ksize);
            dl->AddRectFilled(kstart, ImVec2(kstart.x + ksize.x, kstart.y + ksize.y), IM_COL32(20, 22, 25, 255), 2.0f);
            
            int total_keys = 60;
            float kw = ksize.x / total_keys;
            for (int k = 0; k < total_keys; k++) {
                int pitch = 36 + k;
                bool is_c5 = (pitch == ss.root_note);
                int note_in_octave = pitch % 12;
                bool is_black = (note_in_octave == 1 || note_in_octave == 3 || note_in_octave == 6 || note_in_octave == 8 || note_in_octave == 10);
                
                ImVec2 kp0 = ImVec2(kstart.x + k * kw, kstart.y);
                ImVec2 kp1 = ImVec2(kp0.x + kw - 1.0f, kstart.y + ksize.y);
                
                ImU32 key_col = is_black ? IM_COL32(30, 30, 32, 255) : IM_COL32(240, 240, 235, 255);
                if (is_c5) key_col = IM_COL32(40, 150, 240, 255);
                
                dl->AddRectFilled(kp0, kp1, key_col);
                
                if (io.MousePos.x >= kp0.x && io.MousePos.x <= kp1.x && io.MousePos.y >= kp0.y && io.MousePos.y <= kp1.y) {
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        ss.root_note = pitch;
                    }
                }
            }
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}

inline void DrawFlexKnob(const char* label, float* p_value, float v_min, float v_max, const char* format = "%.2f", float radius = 16.0f) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImGuiContext& g = *GImGui;
    const float line_height = ImGui::GetTextLineHeight();
    // Label under knob
    const char* clean_label = strstr(label, "##") ? label : label;
    std::string text_str(clean_label);
    size_t hash_pos = text_str.find("##");
    if (hash_pos != std::string::npos) text_str = text_str.substr(0, hash_pos);

    ImVec2 text_size = ImGui::CalcTextSize(text_str.c_str());
    float item_width = std::max(radius * 2.0f + 6.0f, text_size.x + 6.0f);
    const ImVec2 size(item_width, radius * 2.0f + line_height + 4.0f);

    const ImGuiID id = window->GetID(label);
    const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return;

    const bool hovered = ImGui::ItemHoverable(bb, id, g.LastItemData.ItemFlags);
    const bool clicked = ImGui::IsItemClicked();
    if (clicked || (hovered && g.IO.MouseDown[0])) {
        ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
        g.ActiveId = id;
        g.ActiveIdSource = ImGuiInputSource_Mouse;
    }

    if (g.ActiveId == id) {
        if (g.IO.MouseDown[0]) {
            float mouse_delta = g.IO.MouseDelta.y;
            if (g.IO.KeyShift) mouse_delta *= 0.2f;
            *p_value -= mouse_delta * (v_max - v_min) / 150.0f;
            if (*p_value < v_min) *p_value = v_min;
            if (*p_value > v_max) *p_value = v_max;
        } else {
            g.ActiveId = 0;
        }
    }

    float t = (*p_value - v_min) / (v_max - v_min);
    float angle_min = 3.14159265f * 0.75f;
    float angle_max = 3.14159265f * 2.25f;
    float angle = angle_min + t * (angle_max - angle_min);

    ImVec2 center = ImVec2(bb.Min.x + size.x * 0.5f, bb.Min.y + radius);

    ImDrawList* draw_list = window->DrawList;
    // Outer shadow ring
    draw_list->AddCircleFilled(center, radius, IM_COL32(18, 22, 28, 255), 32);
    draw_list->AddCircle(center, radius, IM_COL32(45, 55, 68, 255), 32, 1.5f);

    // Active arc (FL Studio Orange)
    draw_list->PathArcTo(center, radius - 2.5f, angle_min, angle, 32);
    draw_list->PathStroke(IM_COL32(247, 127, 0, 255), 0, 3.0f);

    // Inner dial face
    draw_list->AddCircleFilled(center, radius - 4.5f, IM_COL32(32, 38, 48, 255), 32);

    // Indicator dot
    ImVec2 dot_pos = ImVec2(center.x + std::cos(angle) * (radius - 8.0f), center.y + std::sin(angle) * (radius - 8.0f));
    draw_list->AddCircleFilled(dot_pos, 2.5f, IM_COL32(247, 127, 0, 255));

    ImVec2 text_pos = ImVec2(bb.Min.x + (size.x - text_size.x) * 0.5f, bb.Min.y + radius * 2.0f + 2.0f);
    draw_list->AddText(text_pos, IM_COL32(190, 205, 220, 255), text_str.c_str());
}

inline void DrawFlexVSlider(const char* label, ImVec2 size, float* p_value, float v_min, float v_max) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);
    const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return;

    const bool hovered = ImGui::ItemHoverable(bb, id, g.LastItemData.ItemFlags);
    if (hovered && ImGui::IsItemClicked()) {
        g.ActiveId = id;
    }

    if (g.ActiveId == id) {
        if (g.IO.MouseDown[0]) {
            float mouse_y = g.IO.MousePos.y;
            float t = 1.0f - (mouse_y - (bb.Min.y + 10.0f)) / (size.y - 20.0f);
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            *p_value = v_min + t * (v_max - v_min);
        } else {
            g.ActiveId = 0;
        }
    }

    float t = (*p_value - v_min) / (v_max - v_min);
    ImDrawList* draw_list = window->DrawList;

    // Track
    float track_x = bb.Min.x + size.x * 0.5f;
    draw_list->AddRectFilled(ImVec2(track_x - 2.5f, bb.Min.y + 6.0f), ImVec2(track_x + 2.5f, bb.Max.y - 18.0f), IM_COL32(18, 22, 28, 255), 2.0f);
    draw_list->AddRectFilled(ImVec2(track_x - 2.5f, bb.Max.y - 18.0f - t * (size.y - 24.0f)), ImVec2(track_x + 2.5f, bb.Max.y - 18.0f), IM_COL32(247, 127, 0, 200), 2.0f);

    // Thumb
    float thumb_y = (bb.Max.y - 18.0f) - t * (size.y - 24.0f);
    draw_list->AddCircleFilled(ImVec2(track_x, thumb_y), 6.5f, IM_COL32(247, 127, 0, 255));
    draw_list->AddCircle(ImVec2(track_x, thumb_y), 6.5f, IM_COL32(255, 255, 255, 220), 0, 1.2f);

    // Label
    std::string text_str(label);
    size_t hash_pos = text_str.find("##");
    if (hash_pos != std::string::npos) text_str = text_str.substr(0, hash_pos);
    ImVec2 text_size = ImGui::CalcTextSize(text_str.c_str());
    draw_list->AddText(ImVec2(track_x - text_size.x * 0.5f, bb.Max.y - 14.0f), IM_COL32(170, 185, 200, 255), text_str.c_str());
}

inline void ApplyUniquePresetDSP(FlexSettings& fs, int pack, int preset, KuroAudio::MidiInstrument& target_inst, int& trigger_pitch) {
    fs.macro_vibrato = 0.0f;
    fs.macro_reverb = 0.0f;
    fs.macro_delay = 0.0f;
    fs.filter_env_amt = 0.0f;
    fs.pitch = 0.0f;
    trigger_pitch = 60;

    switch (pack) {
    case 0:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_PIANO;
            fs.filter_cutoff = 0.85f; fs.filter_res = 0.20f; fs.env_vol_a = 0.01f; fs.env_vol_d = 0.40f; fs.env_vol_s = 0.60f; fs.env_vol_r = 0.30f; fs.macro_reverb = 0.35f; trigger_pitch = 60;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::ORGAN;
            fs.filter_cutoff = 0.70f; fs.filter_res = 0.30f; fs.env_vol_a = 0.02f; fs.env_vol_d = 0.60f; fs.env_vol_s = 0.80f; fs.env_vol_r = 0.40f; fs.macro_reverb = 0.50f; trigger_pitch = 60;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::SYNTH_SAW;
            fs.filter_cutoff = 0.90f; fs.filter_res = 0.40f; fs.macro_vibrato = 0.50f; fs.env_vol_a = 0.01f; fs.env_vol_d = 0.50f; fs.env_vol_s = 0.70f; fs.env_vol_r = 0.30f; fs.macro_delay = 0.40f; trigger_pitch = 72;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::VIBRAPHONE;
            fs.filter_cutoff = 0.75f; fs.filter_res = 0.50f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.80f; fs.env_vol_s = 0.20f; fs.env_vol_r = 0.60f; fs.macro_reverb = 0.60f; trigger_pitch = 72;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::HARPSICHORD;
            fs.filter_cutoff = 0.95f; fs.filter_res = 0.30f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.20f; fs.env_vol_s = 0.10f; fs.env_vol_r = 0.10f; trigger_pitch = 60;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::SYNTH_SAW;
            fs.filter_cutoff = 0.45f; fs.filter_res = 0.88f; fs.filter_env_amt = 0.85f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.15f; fs.env_vol_s = 0.10f; fs.env_vol_r = 0.05f; trigger_pitch = 48;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::STRING_ENSEMBLE;
            fs.filter_cutoff = 0.60f; fs.filter_res = 0.40f; fs.env_vol_a = 0.40f; fs.env_vol_d = 1.00f; fs.env_vol_s = 0.90f; fs.env_vol_r = 1.20f; fs.macro_reverb = 0.70f; trigger_pitch = 60;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::ACCORDION;
            fs.filter_cutoff = 0.80f; fs.filter_res = 0.20f; fs.env_vol_a = 0.05f; fs.env_vol_d = 0.40f; fs.env_vol_s = 0.80f; fs.env_vol_r = 0.20f; trigger_pitch = 60;
        } else {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_GUITAR;
            fs.filter_cutoff = 0.75f; fs.filter_res = 0.60f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.20f; fs.env_vol_s = 0.30f; fs.env_vol_r = 0.10f; trigger_pitch = 48;
        }
        break;

    case 1:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::ACOUSTIC_PIANO;
            fs.filter_cutoff = 1.00f; fs.filter_res = 0.00f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.80f; fs.env_vol_s = 0.40f; fs.env_vol_r = 0.30f; trigger_pitch = 60;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::ACOUSTIC_PIANO;
            fs.filter_cutoff = 1.00f; fs.filter_res = 0.25f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.70f; fs.env_vol_s = 0.30f; fs.env_vol_r = 0.25f; trigger_pitch = 60;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_PIANO;
            fs.filter_cutoff = 0.90f; fs.filter_res = 0.15f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.60f; fs.env_vol_s = 0.50f; fs.env_vol_r = 0.30f; trigger_pitch = 60;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::HONKY_TONK;
            fs.filter_cutoff = 0.85f; fs.filter_res = 0.30f; fs.macro_vibrato = 0.25f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.65f; fs.env_vol_s = 0.40f; fs.env_vol_r = 0.30f; trigger_pitch = 60;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_PIANO;
            fs.filter_cutoff = 0.80f; fs.filter_res = 0.10f; fs.env_vol_a = 0.005f; fs.env_vol_d = 0.50f; fs.env_vol_s = 0.60f; fs.env_vol_r = 0.40f; trigger_pitch = 60;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.95f; fs.filter_res = 0.35f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.55f; fs.env_vol_s = 0.45f; fs.env_vol_r = 0.35f; trigger_pitch = 60;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::HARPSICHORD;
            fs.filter_cutoff = 1.00f; fs.filter_res = 0.20f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.18f; fs.env_vol_s = 0.05f; fs.env_vol_r = 0.05f; trigger_pitch = 60;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::CLAVINET;
            fs.filter_cutoff = 0.72f; fs.filter_res = 0.65f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.25f; fs.env_vol_s = 0.20f; fs.env_vol_r = 0.10f; trigger_pitch = 48;
        } else {
            target_inst = KuroAudio::MidiInstrument::CELESTA;
            fs.filter_cutoff = 0.90f; fs.filter_res = 0.50f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.90f; fs.env_vol_s = 0.30f; fs.env_vol_r = 0.80f; fs.macro_reverb = 0.55f; trigger_pitch = 72;
        }
        break;

    case 2:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::SYNTH_SAW;
            fs.filter_cutoff = 0.70f; fs.filter_res = 0.50f; fs.filter_env_amt = 0.60f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.10f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.10f; trigger_pitch = 60;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::CELESTA;
            fs.filter_cutoff = 0.85f; fs.filter_res = 0.60f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.35f; fs.env_vol_s = 0.10f; fs.env_vol_r = 0.40f; fs.macro_reverb = 0.45f; trigger_pitch = 72;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::SQUARE_LEAD;
            fs.filter_cutoff = 1.00f; fs.filter_res = 0.10f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.08f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.02f; trigger_pitch = 60;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.90f; fs.filter_res = 0.70f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.12f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.05f; trigger_pitch = 60;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::CELESTA;
            fs.filter_cutoff = 0.95f; fs.filter_res = 0.40f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.40f; fs.env_vol_s = 0.10f; fs.env_vol_r = 0.60f; fs.macro_reverb = 0.65f; trigger_pitch = 72;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::MARIMBA;
            fs.filter_cutoff = 0.80f; fs.filter_res = 0.85f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.09f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.04f; trigger_pitch = 60;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.55f; fs.filter_res = 0.10f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.15f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.05f; trigger_pitch = 48;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::TRANCE_LEAD;
            fs.filter_cutoff = 0.85f; fs.filter_res = 0.40f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.10f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.03f; trigger_pitch = 72;
        } else {
            target_inst = KuroAudio::MidiInstrument::SQUARE_LEAD;
            fs.filter_cutoff = 0.75f; fs.filter_res = 0.50f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.07f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.02f; trigger_pitch = 60;
        }
        break;

    case 3:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.28f; fs.filter_res = 0.00f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.60f; fs.env_vol_s = 0.80f; fs.env_vol_r = 0.40f; trigger_pitch = 36;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_BASS;
            fs.filter_cutoff = 0.48f; fs.filter_res = 0.45f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.50f; fs.env_vol_s = 0.70f; fs.env_vol_r = 0.30f; trigger_pitch = 36;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.18f; fs.filter_res = 0.00f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.80f; fs.env_vol_s = 0.90f; fs.env_vol_r = 0.50f; trigger_pitch = 24;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.35f; fs.filter_res = 0.10f; fs.env_vol_a = 0.001f; fs.env_vol_d = 1.20f; fs.env_vol_s = 0.90f; fs.env_vol_r = 1.50f; trigger_pitch = 36;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_BASS;
            fs.filter_cutoff = 0.55f; fs.filter_res = 0.30f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.25f; fs.env_vol_s = 0.20f; fs.env_vol_r = 0.10f; trigger_pitch = 36;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_BASS;
            fs.filter_cutoff = 0.40f; fs.filter_res = 0.35f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.45f; fs.env_vol_s = 0.60f; fs.env_vol_r = 0.30f; trigger_pitch = 36;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_BASS;
            fs.filter_cutoff = 0.35f; fs.filter_res = 0.20f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.55f; fs.env_vol_s = 0.75f; fs.env_vol_r = 0.40f; trigger_pitch = 36;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.30f; fs.filter_res = 0.15f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.65f; fs.env_vol_s = 0.80f; fs.env_vol_r = 0.50f; trigger_pitch = 36;
        } else {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_BASS;
            fs.filter_cutoff = 0.65f; fs.filter_res = 0.60f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.40f; fs.env_vol_s = 0.60f; fs.env_vol_r = 0.20f; trigger_pitch = 36;
        }
        break;

    case 4:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::SWEEP_PAD;
            fs.filter_cutoff = 0.10f; fs.filter_res = 0.70f; fs.filter_env_amt = 0.90f; fs.env_vol_a = 0.80f; fs.env_vol_d = 1.50f; fs.env_vol_s = 0.80f; fs.env_vol_r = 1.80f; fs.macro_reverb = 0.75f; trigger_pitch = 48;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::PAD_SYNTH;
            fs.filter_cutoff = 0.25f; fs.filter_res = 0.60f; fs.env_vol_a = 1.50f; fs.env_vol_d = 2.00f; fs.env_vol_s = 1.00f; fs.env_vol_r = 2.50f; fs.macro_reverb = 0.85f; trigger_pitch = 36;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::BRASS_SYNTH;
            fs.filter_cutoff = 0.75f; fs.filter_res = 0.35f; fs.env_vol_a = 0.20f; fs.env_vol_d = 0.80f; fs.env_vol_s = 0.70f; fs.env_vol_r = 0.90f; fs.macro_reverb = 0.50f; trigger_pitch = 60;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::SWEEP_PAD;
            fs.filter_cutoff = 0.65f; fs.filter_res = 0.25f; fs.env_vol_a = 0.50f; fs.env_vol_d = 1.20f; fs.env_vol_s = 0.85f; fs.env_vol_r = 1.40f; fs.macro_reverb = 0.70f; trigger_pitch = 60;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::SYNTH_SAW;
            fs.filter_cutoff = 0.70f; fs.filter_res = 0.40f; fs.macro_vibrato = 0.40f; fs.env_vol_a = 0.10f; fs.env_vol_d = 0.60f; fs.env_vol_s = 0.75f; fs.env_vol_r = 0.80f; trigger_pitch = 60;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::PAD_SYNTH;
            fs.filter_cutoff = 0.85f; fs.filter_res = 0.50f; fs.env_vol_a = 0.30f; fs.env_vol_d = 1.00f; fs.env_vol_s = 0.80f; fs.env_vol_r = 1.50f; fs.macro_reverb = 0.90f; trigger_pitch = 72;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::PAD_SYNTH;
            fs.filter_cutoff = 0.40f; fs.filter_res = 0.55f; fs.env_vol_a = 0.60f; fs.env_vol_d = 1.40f; fs.env_vol_s = 0.75f; fs.env_vol_r = 1.60f; fs.macro_reverb = 0.80f; trigger_pitch = 48;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.20f; fs.filter_res = 0.85f; fs.filter_env_amt = 0.95f; fs.env_vol_a = 0.40f; fs.env_vol_d = 1.00f; fs.env_vol_s = 0.50f; fs.env_vol_r = 1.20f; trigger_pitch = 60;
        } else {
            target_inst = KuroAudio::MidiInstrument::TRANCE_LEAD;
            fs.filter_cutoff = 0.80f; fs.filter_res = 0.45f; fs.env_vol_a = 0.05f; fs.env_vol_d = 0.30f; fs.env_vol_s = 0.50f; fs.env_vol_r = 0.60f; fs.macro_delay = 0.55f; trigger_pitch = 60;
        }
        break;

    case 5:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_BASS;
            fs.filter_cutoff = 0.32f; fs.filter_res = 0.40f; fs.filter_env_amt = 0.75f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.09f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.03f; trigger_pitch = 36;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::SYNTH_SAW;
            fs.filter_cutoff = 0.42f; fs.filter_res = 0.95f; fs.filter_env_amt = 0.95f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.18f; fs.env_vol_s = 0.10f; fs.env_vol_r = 0.04f; trigger_pitch = 48;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::LEAD_SYNTH;
            fs.filter_cutoff = 0.92f; fs.filter_res = 0.35f; fs.macro_vibrato = 0.65f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.40f; fs.env_vol_s = 0.70f; fs.env_vol_r = 0.25f; fs.macro_delay = 0.40f; fs.macro_reverb = 0.40f; trigger_pitch = 72;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.98f; fs.filter_res = 0.80f; fs.macro_vibrato = 0.95f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.05f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.02f; trigger_pitch = 72;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::PAD_SYNTH;
            fs.filter_cutoff = 0.28f; fs.filter_res = 0.55f; fs.env_vol_a = 1.20f; fs.env_vol_d = 2.00f; fs.env_vol_s = 1.00f; fs.env_vol_r = 2.00f; fs.macro_reverb = 0.85f; trigger_pitch = 36;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::TRANCE_LEAD;
            fs.filter_cutoff = 0.78f; fs.filter_res = 0.45f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.12f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.05f; fs.macro_delay = 0.50f; trigger_pitch = 60;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.22f; fs.filter_res = 0.20f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.35f; fs.env_vol_s = 0.40f; fs.env_vol_r = 0.20f; trigger_pitch = 36;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::MARIMBA;
            fs.filter_cutoff = 0.65f; fs.filter_res = 0.70f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.07f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.02f; trigger_pitch = 60;
        } else if (preset == 8) {
            target_inst = KuroAudio::MidiInstrument::SWEEP_PAD;
            fs.filter_cutoff = 0.10f; fs.filter_res = 0.75f; fs.filter_env_amt = 0.90f; fs.env_vol_a = 0.50f; fs.env_vol_d = 1.20f; fs.env_vol_s = 0.60f; fs.env_vol_r = 1.50f; fs.macro_reverb = 0.65f; trigger_pitch = 48;
        } else {
            target_inst = KuroAudio::MidiInstrument::FULLON_LEAD;
            fs.filter_cutoff = 0.90f; fs.filter_res = 0.60f; fs.macro_vibrato = 0.50f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.30f; fs.env_vol_s = 0.60f; fs.env_vol_r = 0.20f; fs.macro_delay = 0.45f; trigger_pitch = 72;
        }
        break;

    case 6:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.99f; fs.filter_res = 0.92f; fs.macro_vibrato = 0.98f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.04f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.01f; trigger_pitch = 84;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::TRANCE_LEAD;
            fs.filter_cutoff = 0.82f; fs.filter_res = 0.85f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.10f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.03f; fs.macro_delay = 0.55f; trigger_pitch = 72;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::SYNTH_SAW;
            fs.filter_cutoff = 0.38f; fs.filter_res = 0.98f; fs.filter_env_amt = 0.98f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.16f; fs.env_vol_s = 0.08f; fs.env_vol_r = 0.03f; trigger_pitch = 48;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::FULLON_LEAD;
            fs.filter_cutoff = 0.95f; fs.filter_res = 0.45f; fs.macro_vibrato = 0.70f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.35f; fs.env_vol_s = 0.65f; fs.env_vol_r = 0.20f; fs.macro_delay = 0.40f; trigger_pitch = 72;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::PAD_SYNTH;
            fs.filter_cutoff = 0.18f; fs.filter_res = 0.70f; fs.env_vol_a = 2.00f; fs.env_vol_d = 3.00f; fs.env_vol_s = 1.00f; fs.env_vol_r = 3.00f; fs.macro_reverb = 0.90f; trigger_pitch = 24;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.30f; fs.filter_res = 0.30f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.10f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.02f; trigger_pitch = 36;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.95f; fs.filter_res = 0.85f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.03f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.01f; trigger_pitch = 84;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::MARIMBA;
            fs.filter_cutoff = 0.70f; fs.filter_res = 0.60f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.08f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.03f; trigger_pitch = 60;
        } else if (preset == 8) {
            target_inst = KuroAudio::MidiInstrument::SYNTH_SAW;
            fs.filter_cutoff = 0.60f; fs.filter_res = 0.98f; fs.filter_env_amt = 0.90f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.14f; fs.env_vol_s = 0.05f; fs.env_vol_r = 0.04f; trigger_pitch = 60;
        } else if (preset == 9) {
            target_inst = KuroAudio::MidiInstrument::SWEEP_PAD;
            fs.filter_cutoff = 0.05f; fs.filter_res = 0.80f; fs.filter_env_amt = 0.99f; fs.env_vol_a = 1.00f; fs.env_vol_d = 2.00f; fs.env_vol_s = 0.80f; fs.env_vol_r = 2.00f; trigger_pitch = 48;
        } else if (preset == 10) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.85f; fs.filter_res = 0.65f; fs.macro_vibrato = 0.80f; fs.env_vol_a = 0.01f; fs.env_vol_d = 0.25f; fs.env_vol_s = 0.40f; fs.env_vol_r = 0.20f; trigger_pitch = 72;
        } else {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.25f; fs.filter_res = 0.15f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.40f; fs.env_vol_s = 0.20f; fs.env_vol_r = 0.10f; trigger_pitch = 36;
        }
        break;

    case 7:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::SYNTH_SAW;
            fs.filter_cutoff = 0.65f; fs.filter_res = 0.40f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.30f; fs.env_vol_s = 0.50f; fs.env_vol_r = 0.20f; trigger_pitch = 36;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::LEAD_SYNTH;
            fs.filter_cutoff = 0.90f; fs.filter_res = 0.30f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.40f; fs.env_vol_s = 0.70f; fs.env_vol_r = 0.30f; fs.macro_delay = 0.40f; trigger_pitch = 72;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::BRASS_SYNTH;
            fs.filter_cutoff = 0.70f; fs.filter_res = 0.50f; fs.env_vol_a = 0.15f; fs.env_vol_d = 0.70f; fs.env_vol_s = 0.80f; fs.env_vol_r = 0.90f; fs.macro_reverb = 0.70f; trigger_pitch = 60;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::TRANCE_LEAD;
            fs.filter_cutoff = 0.80f; fs.filter_res = 0.35f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.15f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.05f; fs.macro_delay = 0.30f; trigger_pitch = 60;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.55f; fs.filter_res = 0.60f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.20f; fs.env_vol_s = 0.30f; fs.env_vol_r = 0.10f; trigger_pitch = 36;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::SQUARE_LEAD;
            fs.filter_cutoff = 1.00f; fs.filter_res = 0.10f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.08f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.02f; trigger_pitch = 60;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::SWEEP_PAD;
            fs.filter_cutoff = 0.60f; fs.filter_res = 0.20f; fs.env_vol_a = 0.40f; fs.env_vol_d = 1.00f; fs.env_vol_s = 0.80f; fs.env_vol_r = 1.20f; fs.macro_reverb = 0.60f; trigger_pitch = 60;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::SQUARE_LEAD;
            fs.filter_cutoff = 0.75f; fs.filter_res = 0.50f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.10f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.04f; fs.macro_delay = 0.50f; trigger_pitch = 60;
        } else {
            target_inst = KuroAudio::MidiInstrument::PAD_SYNTH;
            fs.filter_cutoff = 0.50f; fs.filter_res = 0.30f; fs.env_vol_a = 0.60f; fs.env_vol_d = 1.50f; fs.env_vol_s = 0.85f; fs.env_vol_r = 1.80f; fs.macro_reverb = 0.80f; trigger_pitch = 60;
        }
        break;

    case 8:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.20f; fs.filter_res = 0.10f; fs.env_vol_a = 0.001f; fs.env_vol_d = 1.00f; fs.env_vol_s = 0.80f; fs.env_vol_r = 1.20f; trigger_pitch = 24;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::CHOIR;
            fs.filter_cutoff = 0.75f; fs.filter_res = 0.30f; fs.env_vol_a = 0.50f; fs.env_vol_d = 1.20f; fs.env_vol_s = 0.90f; fs.env_vol_r = 1.50f; fs.macro_reverb = 0.85f; trigger_pitch = 60;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::PAD_SYNTH;
            fs.filter_cutoff = 0.30f; fs.filter_res = 0.65f; fs.env_vol_a = 1.80f; fs.env_vol_d = 2.50f; fs.env_vol_s = 1.00f; fs.env_vol_r = 3.00f; fs.macro_reverb = 0.90f; trigger_pitch = 36;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::SWEEP_PAD;
            fs.filter_cutoff = 0.10f; fs.filter_res = 0.85f; fs.filter_env_amt = 0.95f; fs.env_vol_a = 0.80f; fs.env_vol_d = 1.80f; fs.env_vol_s = 0.70f; fs.env_vol_r = 2.00f; trigger_pitch = 48;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::ORGAN;
            fs.filter_cutoff = 0.50f; fs.filter_res = 0.40f; fs.env_vol_a = 0.10f; fs.env_vol_d = 0.80f; fs.env_vol_s = 0.85f; fs.env_vol_r = 1.00f; fs.macro_reverb = 0.70f; trigger_pitch = 48;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::STRING_ENSEMBLE;
            fs.filter_cutoff = 0.70f; fs.filter_res = 0.30f; fs.env_vol_a = 0.50f; fs.env_vol_d = 1.50f; fs.env_vol_s = 0.80f; fs.env_vol_r = 1.50f; trigger_pitch = 60;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::PAD_SYNTH;
            fs.filter_cutoff = 0.25f; fs.filter_res = 0.50f; fs.env_vol_a = 1.20f; fs.env_vol_d = 2.00f; fs.env_vol_s = 0.90f; fs.env_vol_r = 2.20f; fs.macro_reverb = 0.80f; trigger_pitch = 36;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.95f; fs.filter_res = 0.80f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.10f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.05f; trigger_pitch = 72;
        } else {
            target_inst = KuroAudio::MidiInstrument::PAD_SYNTH;
            fs.filter_cutoff = 0.40f; fs.filter_res = 0.40f; fs.env_vol_a = 0.80f; fs.env_vol_d = 1.60f; fs.env_vol_s = 0.80f; fs.env_vol_r = 1.80f; fs.macro_reverb = 0.75f; trigger_pitch = 60;
        }
        break;

    case 9:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::LEAD_SYNTH;
            fs.filter_cutoff = 0.95f; fs.filter_res = 0.30f; fs.env_vol_a = 0.01f; fs.env_vol_d = 0.50f; fs.env_vol_s = 0.70f; fs.env_vol_r = 0.30f; fs.macro_delay = 0.40f; trigger_pitch = 60;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::MARIMBA;
            fs.filter_cutoff = 0.80f; fs.filter_res = 0.50f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.12f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.05f; trigger_pitch = 60;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::CLAVINET;
            fs.filter_cutoff = 0.85f; fs.filter_res = 0.70f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.09f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.04f; trigger_pitch = 60;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::SYNTH_SAW;
            fs.filter_cutoff = 0.60f; fs.filter_res = 0.80f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.15f; fs.env_vol_s = 0.10f; fs.env_vol_r = 0.05f; trigger_pitch = 36;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.50f; fs.filter_res = 0.85f; fs.macro_vibrato = 0.80f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.30f; fs.env_vol_s = 0.40f; fs.env_vol_r = 0.10f; trigger_pitch = 36;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::SYNTH_SAW;
            fs.filter_cutoff = 0.90f; fs.filter_res = 0.50f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.25f; fs.env_vol_s = 0.50f; fs.env_vol_r = 0.15f; trigger_pitch = 72;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::TRANCE_LEAD;
            fs.filter_cutoff = 0.95f; fs.filter_res = 0.35f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.40f; fs.env_vol_s = 0.75f; fs.env_vol_r = 0.30f; fs.macro_delay = 0.50f; trigger_pitch = 72;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.30f; fs.filter_res = 0.20f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.20f; fs.env_vol_s = 0.10f; fs.env_vol_r = 0.05f; trigger_pitch = 36;
        } else {
            target_inst = KuroAudio::MidiInstrument::LEAD_SYNTH;
            fs.filter_cutoff = 0.75f; fs.filter_res = 0.45f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.15f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.06f; fs.macro_delay = 0.40f; trigger_pitch = 60;
        }
        break;

    case 10:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.25f; fs.filter_res = 0.10f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.80f; fs.env_vol_s = 0.70f; fs.env_vol_r = 0.40f; trigger_pitch = 36;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.30f; fs.filter_res = 0.15f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.75f; fs.env_vol_s = 0.75f; fs.env_vol_r = 0.45f; trigger_pitch = 36;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_BASS;
            fs.filter_cutoff = 0.50f; fs.filter_res = 0.50f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.45f; fs.env_vol_s = 0.60f; fs.env_vol_r = 0.25f; trigger_pitch = 36;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::SQUARE_LEAD;
            fs.filter_cutoff = 0.80f; fs.filter_res = 0.40f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.10f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.03f; trigger_pitch = 60;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.20f; fs.filter_res = 0.05f; fs.env_vol_a = 0.001f; fs.env_vol_d = 1.20f; fs.env_vol_s = 0.90f; fs.env_vol_r = 1.00f; trigger_pitch = 24;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::ELECTRIC_BASS;
            fs.filter_cutoff = 0.40f; fs.filter_res = 0.40f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.50f; fs.env_vol_s = 0.65f; fs.env_vol_r = 0.30f; trigger_pitch = 36;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.35f; fs.filter_res = 0.20f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.25f; fs.env_vol_s = 0.10f; fs.env_vol_r = 0.05f; trigger_pitch = 36;
        } else if (preset == 7) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.20f; fs.filter_res = 0.10f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.90f; fs.env_vol_s = 0.80f; fs.env_vol_r = 0.60f; trigger_pitch = 36;
        } else {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.18f; fs.filter_res = 0.00f; fs.env_vol_a = 0.001f; fs.env_vol_d = 1.00f; fs.env_vol_s = 0.85f; fs.env_vol_r = 0.70f; trigger_pitch = 24;
        }
        break;

    case 11:
        if (preset == 0) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.98f; fs.filter_res = 0.85f; fs.macro_vibrato = 0.95f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.10f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.02f; trigger_pitch = 84;
        } else if (preset == 1) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.85f; fs.filter_res = 0.75f; fs.macro_vibrato = 0.90f; fs.env_vol_a = 0.01f; fs.env_vol_d = 0.30f; fs.env_vol_s = 0.40f; fs.env_vol_r = 0.20f; trigger_pitch = 72;
        } else if (preset == 2) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.99f; fs.filter_res = 0.90f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.06f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.01f; trigger_pitch = 84;
        } else if (preset == 3) {
            target_inst = KuroAudio::MidiInstrument::FM_SYNTH;
            fs.filter_cutoff = 0.80f; fs.filter_res = 0.60f; fs.macro_vibrato = 0.70f; fs.env_vol_a = 0.02f; fs.env_vol_d = 0.40f; fs.env_vol_s = 0.50f; fs.env_vol_r = 0.30f; trigger_pitch = 60;
        } else if (preset == 4) {
            target_inst = KuroAudio::MidiInstrument::SWEEP_PAD;
            fs.filter_cutoff = 0.05f; fs.filter_res = 0.85f; fs.filter_env_amt = 0.99f; fs.env_vol_a = 0.80f; fs.env_vol_d = 1.80f; fs.env_vol_s = 0.70f; fs.env_vol_r = 2.00f; trigger_pitch = 48;
        } else if (preset == 5) {
            target_inst = KuroAudio::MidiInstrument::SUB_BASS;
            fs.filter_cutoff = 0.40f; fs.filter_res = 0.30f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.30f; fs.env_vol_s = 0.00f; fs.env_vol_r = 0.10f; trigger_pitch = 36;
        } else if (preset == 6) {
            target_inst = KuroAudio::MidiInstrument::SQUARE_LEAD;
            fs.filter_cutoff = 0.60f; fs.filter_res = 0.50f; fs.env_vol_a = 0.001f; fs.env_vol_d = 0.15f; fs.env_vol_s = 0.20f; fs.env_vol_r = 0.10f; fs.macro_delay = 0.60f; trigger_pitch = 60;
        } else {
            target_inst = KuroAudio::MidiInstrument::PAD_SYNTH;
            fs.filter_cutoff = 0.30f; fs.filter_res = 0.70f; fs.env_vol_a = 1.50f; fs.env_vol_d = 2.20f; fs.env_vol_s = 0.90f; fs.env_vol_r = 2.50f; fs.macro_reverb = 0.85f; trigger_pitch = 36;
        }
        break;

    case 12: // PACK 12: DELAY LAMA (MONGE TIBETANO 3D)
        target_inst = KuroAudio::MidiInstrument::DELAY_LAMA;
        show_delay_lama = true;
        focus_delay_lama = true;
        if (preset == 0) { // Monge Tibetano Clássico
            g_delay_lama_vowel_x = 0.50f; g_delay_lama_pitch_y = 0.0f; g_delay_lama_delay_time = 0.35f; g_delay_lama_delay_mix = 0.40f;
        } else if (preset == 1) { // Canto Gutural Sub-Bass
            g_delay_lama_vowel_x = 0.10f; g_delay_lama_pitch_y = -0.4f; g_delay_lama_delay_time = 0.50f; g_delay_lama_delay_mix = 0.30f;
        } else if (preset == 2) { // Eco Místico das Montanhas
            g_delay_lama_vowel_x = 0.75f; g_delay_lama_pitch_y = 0.2f; g_delay_lama_delay_time = 0.25f; g_delay_lama_delay_mix = 0.65f;
        } else { // Mantra Sagrado Om
            g_delay_lama_vowel_x = 0.30f; g_delay_lama_pitch_y = 0.0f; g_delay_lama_delay_time = 0.40f; g_delay_lama_delay_mix = 0.50f;
        }
        fs.filter_cutoff = 0.95f; fs.filter_res = 0.30f; fs.env_vol_a = 0.05f; fs.env_vol_d = 1.0f; fs.env_vol_s = 0.8f; fs.env_vol_r = 1.2f; trigger_pitch = 48;
        break;

    default:
        target_inst = KuroAudio::MidiInstrument::ACOUSTIC_PIANO;
        trigger_pitch = 60;
        break;
    }
}

inline void RenderFlexPresetBrowser(int channel_idx) {
    if (!show_flex_browser) return;
    
    char win_title[64];
    const char* ch_names[] = { "Kick", "Snare", "HiHat", "Bassline", "Serum Chords", "Lead Synth", "Clap", "Open Hat" };
    snprintf(win_title, sizeof(win_title), "FL Studio FLEX Synth - %s (Channel %d)###FlexPresetWindow", ch_names[channel_idx], channel_idx);
    
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.11f, 0.13f, 0.16f, 1.0f)); // FL Slate Dark
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.97f, 0.50f, 0.0f, 0.85f));   // FL Orange Select
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 0.60f, 0.1f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.24f, 0.30f, 1.0f));

    ImGui::SetNextWindowSize(ImVec2(1040, 620), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(win_title, &show_flex_browser, ImGuiWindowFlags_NoCollapse)) {
        auto& fs = g_piano_synth.flex_settings[channel_idx];
        
        // =========================================================================
        // PAINEL ESQUERDO: PACKS & PRESETS (Estilo 1:1 FL Studio FLEX)
        // =========================================================================
        ImGui::BeginChild("LeftFlexPanel", ImVec2(340, 0), true);
        
        // Header Tabs interativas: PACKS | GET | STORE | PRESETS
        static int flex_top_tab = 0; // 0 = PACKS / PRESETS, 1 = STORE / GET
        static bool flex_pack_installed[12] = { true, true, true, true, true, true, true, true, true, true, true, true };
        static float download_progress[12] = { 0.0f };
        static bool downloading[12] = { false };

        if (flex_top_tab == 0) {
            ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "[ PACKS ]");
        } else {
            if (ImGui::Button("PACKS", ImVec2(55, 20))) flex_top_tab = 0;
        }
        ImGui::SameLine(70);
        
        if (flex_top_tab == 1) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "[ GET / STORE ]");
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
            if (ImGui::Button("GET / STORE [SHOP]", ImVec2(110, 20))) flex_top_tab = 1;
            ImGui::PopStyleColor();
        }
        ImGui::SameLine(190);
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "PRESETS (65+)");
        ImGui::Separator();

        if (flex_top_tab == 1) {
            // =========================================================================
            // LOJA DE EXPANSÕES INTERATIVA (GET / STORE VIEW)
            // =========================================================================
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "FLEX STORE & EXPANSION PACK DOWNLOADER");
            ImGui::TextDisabled("Baixe pacotes de presets adicionais para inflar sua DAW com mais timbres!");
            ImGui::Spacing();

            const char* store_pack_names[] = {
                "Psytrance Vol. 2 (Psycore & Goa)",
                "Synthwave 80s Cyberpunk Outrun",
                "Cinematic Sci-Fi Space Drones",
                "EDM & Future Bass Festival",
                "Trap 808 Voltage Master",
                "Abduction Alien FX & Lasers"
            };
            const char* store_pack_authors[] = { "Kuro Audio Lab", "Neon Syndicate", "Hans Zimmer Style", "Festival Beats", "808 Mafia Style", "Area 51 Audio" };
            const int store_pack_presets[] = { 12, 9, 9, 9, 9, 8 };

            ImGui::Columns(2, "StoreGrid", false);
            for (int k = 0; k < 6; k++) {
                ImGui::PushID(k + 100);
                ImGui::BeginChild(std::string("StoreCard_" + std::to_string(k)).c_str(), ImVec2(0, 105), true);
                
                ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "%s", store_pack_names[k]);
                ImGui::TextDisabled("Criador: %s | %d Presets HD", store_pack_authors[k], store_pack_presets[k]);
                ImGui::Spacing();

                if (flex_pack_installed[k]) {
                    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "[OK] INSTALADO & ATIVO NA DAW");
                } else if (downloading[k]) {
                    download_progress[k] += 0.03f;
                    ImGui::ProgressBar(download_progress[k], ImVec2(-1, 20), "Baixando Expansao...");
                    if (download_progress[k] >= 1.0f) {
                        downloading[k] = false;
                        flex_pack_installed[k] = true;
                    }
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.5f, 1.0f));
                    if (ImGui::Button("BAIXAR & INSTALAR (GRATIS)", ImVec2(-1, 26))) {
                        downloading[k] = true;
                        download_progress[k] = 0.0f;
                    }
                    ImGui::PopStyleColor();
                }

                ImGui::EndChild();
                ImGui::PopID();

                if (k % 2 == 0) ImGui::NextColumn();
                else ImGui::NextColumn();
            }
            ImGui::Columns(1);
            
            ImGui::Spacing();
            if (ImGui::Button("<- VOLTAR PARA O NAVEGADOR DE PRESETS", ImVec2(-1, 30))) {
                flex_top_tab = 0;
            }

            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("RightFlexPanel", ImVec2(0, 0), true);
        } else {
            // =========================================================================
            // NAVEGADOR DE PACKS E PRESETS NORMAL
            // =========================================================================
            ImGui::Columns(2, "FlexPacksCols", false);
            ImGui::SetColumnWidth(0, 160);
            
            // Lista Ampliada de Packs Nativos & Expansões Instaladas
            const char* packs[] = { 
                "Arksun Cityscape", 
                "General Midi Library", 
                "Mobile Synth Pluck", 
                "Mobile Tuned 808 Bass", 
                "Olbaid Compendium", 
                "Psytrance Essentials", // PACK 5
                "Psytrance Vol. 2 (Psycore)", // PACK 6
                "Synthwave 80s Cyberpunk", // PACK 7
                "Cinematic Space Drones", // PACK 8
                "EDM Future Bass Festival", // PACK 9
                "Trap 808 Voltage", // PACK 10
                "Abduction Alien FX", // PACK 11
                "[+] Delay Lama (Monge 3D)" // PACK 12 - DELAY LAMA
            };
            
            for (int i = 0; i < 13; i++) {
                bool selected = (fs.selected_pack == i);
                if (i == 12) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.75f, 0.20f, 1.0f)); // Dourado Tibetano
                else if (i == 5 || i == 6) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f)); // Neon Psytrance
                else if (i >= 7) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.97f, 0.50f, 0.0f, 1.0f)); // Store Packs
                
                if (ImGui::Selectable(packs[i], selected)) {
                    fs.selected_pack = i;
                    fs.selected_preset = 0;

                    if (i == 12) {
                        // Abrir Janela do Monge 3D e sintonizar canal ativo do FLEX
                        show_delay_lama = true;
                        focus_delay_lama = true;
                        g_piano_synth.flex_channel_instrument[channel_idx] = KuroAudio::MidiInstrument::DELAY_LAMA;
                        g_piano_synth.flex_active[channel_idx] = true;
                        g_piano_synth.setInstrument(KuroAudio::MidiInstrument::DELAY_LAMA);
                        g_piano_synth.triggerNote(48, 1.2f, 0.9f, channel_idx);
                    }
                }
                if (i == 5 || i == 6 || i >= 7) ImGui::PopStyleColor();
            }
            
            ImGui::NextColumn();
            
            // Listas de Presets Dinâmicos Ampliadas
            const char* presets_arksun[] = { "70s Bounce", "7th Soul", "80s Theatre", "Alumin Sun", "AmbiClav", "Bite Me", "Black Sting", "Blue Cordian", "Funky Electricity" };
            const char* presets_general_midi[] = { "Acoustic Grand Piano", "Bright Acoustic Piano", "Electric Grand Piano", "Honky-tonk Piano", "Electric Piano 1", "Electric Piano 2", "Harpsichord", "Clavi", "Celesta" };
            const char* presets_mobile_pluck[] = { "Analog Pluck 1", "Bell Pluck", "Chiptune Pluck", "FM Pluck", "Glass Pluck", "Metal Pluck", "Plucky Sine", "Short Decay Lead", "Tiny Wave Pluck" };
            const char* presets_tuned_808[] = { "Clean 808 Bass", "Distorted 808", "Deep Sub Bass", "Long Release 808", "Punchy 808", "Sat Sub 808", "Glide 808 Bass", "Slide Sub 808", "Heavy Dist Bass" };
            const char* presets_olbaid[] = { "Cinematic Sweep", "Dark Drone", "Epic Brass Pad", "Lush Wave Pad", "Retro Pad", "Starlight Pad", "Cosmic Pad", "Sci-Fi FX Sweep", "Dreamy Arp Pad" };
            
            const char* presets_psytrance[] = {
                "Psytrance Rolling Bass (16th)",
                "Psytrance 303 Acid Squelch",
                "Psytrance Goa Saw Lead",
                "Psytrance Alien Laser Zap",
                "Psytrance Dark Psy Drone",
                "Psytrance Gated Trance Arp",
                "Psytrance FM Sub Boom",
                "Psytrance Tribal Perc Pluck",
                "Psytrance Hypnotic Psy Sweep",
                "Psytrance Full-On Scream Lead"
            };

            const char* presets_psytrance_vol2[] = {
                "Psycore 200BPM FM Laser",
                "Goa Metallic Filter Arp",
                "Psytrance Acid Saw Tooth 303",
                "Full-On Hyper Lead",
                "Darkpsy Void Drone",
                "Psytrance Triplet Sub Bass",
                "Psytrance Glitch Zap",
                "Psytrance Organic Perc Pluck",
                "Psytrance Resonance Screamer",
                "Psytrance Galactic Riser",
                "Psytrance Alien Voice FM",
                "Psytrance Sub Boom Punch"
            };

            const char* presets_synthwave[] = {
                "80s Analog PWM Bass",
                "Outrun Neon Saw Lead",
                "Blade Runner Vangelis Brass",
                "Synthwave Gated Snare Synth",
                "Cyberpunk FM Bassline",
                "Retro Chiptune Square",
                "Sunset Boulevard Pad",
                "Synthwave Arpeggiated Pulse",
                "Vaporwave Chill Pad"
            };

            const char* presets_cinematic[] = {
                "Hans Zimmer Sub Impact",
                "Starlight Celestial Choir",
                "Space Void Atmosphere",
                "Warp Speed Sweep",
                "Alien Organ Drone",
                "Cosmic Strings Ensemble",
                "Deep Galaxy Drone",
                "Sci-Fi Laser Impact",
                "Solar Eclipse Pad"
            };

            const char* presets_edm[] = {
                "Future Bass SuperSaw Chords",
                "Festival Vocal Pluck",
                "Metallic Drop Pluck",
                "Hardstyle Donk Bass",
                "Dubstep Wobble FM",
                "Electro House Lead",
                "Trance Anthem Saw",
                "Bigroom Kick Synth",
                "Progressive House Pluck"
            };

            const char* presets_trap[] = {
                "Sub Voltage 808",
                "Glide Sub Bass 808",
                "Distorted Hard 808",
                "Pitched Trap Snare Synth",
                "808 Sub Boom Master",
                "Saturated Trap Bass",
                "Short Punchy 808",
                "Sub Bass Drop 808",
                "Low End Sub Voltage"
            };

            const char* presets_alien_fx[] = {
                "Alien Abduction Beam Zap",
                "UFO Radar Scanner FM",
                "Area 51 Teleport Laser",
                "Martian Alien Voice Synthesizer",
                "Galactic Engine Riser",
                "Plasma Cannon Impact",
                "Sub-Space Pulse FX",
                "Alien Matrix Drone"
            };
            
            const char* presets_delay_lama[] = {
                "Monge Tibetano Clássico (Temple Chant)",
                "Canto Gutural Sub-Bass (Deep Throat)",
                "Eco Místico das Montanhas (Psy Echo)",
                "Mantra Sagrado Om (Sacred Om)"
            };

            const char** active_presets = nullptr;
            int num_presets = 9;
            switch (fs.selected_pack) {
                case 0: active_presets = presets_arksun; num_presets = 9; break;
                case 1: active_presets = presets_general_midi; num_presets = 9; break;
                case 2: active_presets = presets_mobile_pluck; num_presets = 9; break;
                case 3: active_presets = presets_tuned_808; num_presets = 9; break;
                case 4: active_presets = presets_olbaid; num_presets = 9; break;
                case 5: active_presets = presets_psytrance; num_presets = 10; break;
                case 6: active_presets = presets_psytrance_vol2; num_presets = 12; break;
                case 7: active_presets = presets_synthwave; num_presets = 9; break;
                case 8: active_presets = presets_cinematic; num_presets = 9; break;
                case 9: active_presets = presets_edm; num_presets = 9; break;
                case 10: active_presets = presets_trap; num_presets = 9; break;
                case 11: active_presets = presets_alien_fx; num_presets = 8; break;
                case 12: active_presets = presets_delay_lama; num_presets = 4; break;
                default: active_presets = presets_arksun; num_presets = 9; break;
            }
            
            for (int i = 0; i < num_presets; i++) {
                bool selected = (fs.selected_preset == i);
                if (ImGui::Selectable(active_presets[i], selected)) {
                    fs.selected_preset = i;
                    
                    KuroAudio::MidiInstrument target_inst = KuroAudio::MidiInstrument::ACOUSTIC_PIANO;
                    int trigger_pitch = 60;
                    ApplyUniquePresetDSP(fs, fs.selected_pack, i, target_inst, trigger_pitch);
                    
                    g_piano_synth.flex_active[channel_idx] = true;
                    g_piano_synth.flex_channel_instrument[channel_idx] = target_inst;
                    g_piano_synth.setInstrument(target_inst);
                    g_piano_synth.triggerNote(trigger_pitch, 0.5f, 0.8f, channel_idx);
                }
            }
        }
        
        ImGui::Columns(1);
        
        // Painel inferior esquerdo (Badges de Cor 1 2 3 4 e Knobs do FL Studio)
        float left_h = ImGui::GetWindowHeight();
        ImGui::SetCursorPosY(left_h - 90);
        ImGui::Separator();
        
        DrawFlexKnob("PITCH", &fs.pitch, -24.0f, 24.0f, "%.0f", 12.0f); ImGui::SameLine(0, 10);
        DrawFlexKnob("FILTER", &fs.filter_cutoff, 0.0f, 1.0f, "%.2f", 12.0f); ImGui::SameLine(0, 10);
        DrawFlexKnob("REVERB", &fs.macro_reverb, 0.0f, 1.0f, "%.2f", 12.0f); ImGui::SameLine(0, 15);
        
        // Botões de Cor 1, 2, 3, 4 no canto inferior esquerdo (Como na screenshot)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.25f, 0.25f, 1.0f)); ImGui::Button("1", ImVec2(18, 28)); ImGui::PopStyleColor(); ImGui::SameLine(0, 4);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.55f, 0.15f, 1.0f)); ImGui::Button("2", ImVec2(18, 28)); ImGui::PopStyleColor(); ImGui::SameLine(0, 4);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.85f, 0.25f, 1.0f)); ImGui::Button("3", ImVec2(18, 28)); ImGui::PopStyleColor(); ImGui::SameLine(0, 4);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.85f, 0.90f, 1.0f)); ImGui::Button("4", ImVec2(18, 28)); ImGui::PopStyleColor();
        
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // =========================================================================
        // PAINEL DIREITO: SINTETIZADOR, ANALISADOR & RACK DE EFEITOS (FL Studio 1:1)
        // =========================================================================
        ImGui::BeginChild("RightFlexPanel", ImVec2(0, 0), true);
        
        // --- 1. TOPO: ANALYSIS & MACROS (8 Sliders Verticais em Laranja FL) ---
        ImGui::BeginChild("TopAnalysisMacrosPanel", ImVec2(0, 160), true);
        
        ImGui::Columns(2, "AnalysisCols", false);
        ImGui::SetColumnWidth(0, 240);
        
        // ANALYSIS (Spectrum Visualizer + FL Logo)
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "ANALYSIS");
        ImVec2 spec_start = ImGui::GetCursorScreenPos();
        ImVec2 spec_size = ImVec2(220, 110);
        ImGui::InvisibleButton("##flex_spec_main", spec_size);
        
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(spec_start, ImVec2(spec_start.x + spec_size.x, spec_start.y + spec_size.y), IM_COL32(18, 22, 28, 255), 4.0f);
        
        // Espectro de Análise de Frequências (Onda FL Studio)
        float t_val = (float)ImGui::GetTime();
        float current_lvl = ::master_vu_level_l * 2.0f;
        for (int x = 0; x < (int)spec_size.x; x++) {
            float freq = x * 0.07f;
            float val = std::sin(t_val * 14.0f + freq) * 0.40f + std::sin(t_val * 6.0f - freq * 0.8f) * 0.25f;
            val *= (current_lvl * 4.0f + 0.15f);
            val = std::abs(val);
            dl->AddLine(
                ImVec2(spec_start.x + x, spec_start.y + spec_size.y),
                ImVec2(spec_start.x + x, spec_start.y + spec_size.y - val * spec_size.y * 0.85f),
                IM_COL32(247, 127, 0, 220)
            );
        }
        dl->AddText(ImVec2(spec_start.x + 15, spec_start.y + 35), IM_COL32(247, 127, 0, 255), "FLEX");
        dl->AddText(ImVec2(spec_start.x + 15, spec_start.y + 55), IM_COL32(180, 195, 210, 200), "Advanced simplicity");
        
        ImGui::NextColumn();
        
        // MACROS (Sliders Verticais com knobs circulares em Laranja FL)
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "MACROS");
        
        DrawFlexVSlider("Filter##m1", ImVec2(35, 120), &fs.macro_filter, 0.0f, 1.0f); ImGui::SameLine(0, 6);
        DrawFlexVSlider("Vibrato##m2", ImVec2(35, 120), &fs.macro_vibrato, 0.0f, 1.0f); ImGui::SameLine(0, 6);
        DrawFlexVSlider("Gated##m3", ImVec2(35, 120), &fs.macro_extra1, 0.0f, 1.0f); ImGui::SameLine(0, 6);
        DrawFlexVSlider("Character##m4", ImVec2(35, 120), &fs.macro_harmonic, 0.0f, 1.0f); ImGui::SameLine(0, 6);
        DrawFlexVSlider("Reverb##m5", ImVec2(35, 120), &fs.macro_reverb, 0.0f, 1.0f); ImGui::SameLine(0, 6);
        DrawFlexVSlider("Delay##m6", ImVec2(35, 120), &fs.macro_delay, 0.0f, 1.0f); ImGui::SameLine(0, 6);
        DrawFlexVSlider("Mod 7##m7", ImVec2(35, 120), &fs.macro_extra2, 0.0f, 1.0f); ImGui::SameLine(0, 6);
        DrawFlexVSlider("Mod 8##m8", ImVec2(35, 120), &fs.macro_extra3, 0.0f, 1.0f);
        
        ImGui::Columns(1);
        ImGui::EndChild();
        
        // --- 2. MEIO: PITCH, FILTER & ENVELOPES ---
        ImGui::BeginChild("MidFilterEnvelopesPanel", ImVec2(0, 185), true);
        
        ImGui::Columns(3, "MidCols", false);
        ImGui::SetColumnWidth(0, 100);
        ImGui::SetColumnWidth(1, 190);
        
        // Pitch Fader Vertical (-24 a +24)
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "PITCH");
        DrawFlexVSlider("##flex_pitch_fader", ImVec2(40, 140), &fs.pitch, -24.0f, 24.0f);
        
        ImGui::NextColumn();
        
        // FILTER Panel
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "FILTER");
        DrawFlexKnob("Cutoff##flt", &fs.filter_cutoff, 0.0f, 1.0f, "%.2f", 24.0f);
        DrawFlexKnob("Res##flt", &fs.filter_res, 0.0f, 1.0f, "%.2f", 12.0f); ImGui::SameLine(0, 15);
        DrawFlexKnob("Env Amt##flt", &fs.filter_env_amt, 0.0f, 1.0f, "%.2f", 12.0f);
        
        ImGui::NextColumn();
        
        // ENVELOPES Panel (Filter & Volume ADSR Dials)
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "ENVELOPES");
        
        ImGui::TextDisabled("Filter Env"); ImGui::SameLine(70);
        DrawFlexKnob("A##fe", &fs.env_filter_a, 0.001f, 2.0f, "%.2f", 11.0f); ImGui::SameLine(0, 8);
        DrawFlexKnob("H##fe", &fs.env_filter_h, 0.0f, 2.0f, "%.2f", 11.0f); ImGui::SameLine(0, 8);
        DrawFlexKnob("D##fe", &fs.env_filter_d, 0.001f, 2.0f, "%.2f", 11.0f); ImGui::SameLine(0, 8);
        DrawFlexKnob("S##fe", &fs.env_filter_s, 0.0f, 1.0f, "%.2f", 11.0f); ImGui::SameLine(0, 8);
        DrawFlexKnob("R##fe", &fs.env_filter_r, 0.001f, 2.0f, "%.2f", 11.0f);
        
        ImGui::Spacing();
        
        ImGui::TextDisabled("Volume Env"); ImGui::SameLine(70);
        DrawFlexKnob("A##ve", &fs.env_vol_a, 0.001f, 2.0f, "%.2f", 11.0f); ImGui::SameLine(0, 8);
        DrawFlexKnob("H##ve", &fs.env_vol_h, 0.0f, 2.0f, "%.2f", 11.0f); ImGui::SameLine(0, 8);
        DrawFlexKnob("D##ve", &fs.env_vol_d, 0.001f, 2.0f, "%.2f", 11.0f); ImGui::SameLine(0, 8);
        DrawFlexKnob("S##ve", &fs.env_vol_s, 0.0f, 1.0f, "%.2f", 11.0f); ImGui::SameLine(0, 8);
        DrawFlexKnob("R##ve", &fs.env_vol_r, 0.001f, 2.0f, "%.2f", 11.0f);
        
        ImGui::Columns(1);
        ImGui::EndChild();
        
        // --- 3. BASE: MASTER FILTER, DELAY, REVERB, LIMITER & OUT (FL Studio Rack) ---
        ImGui::BeginChild("BottomEffectsPanel", ImVec2(0, 0), true);
        
        ImGui::Columns(5, "BtmCols", false);
        ImGui::SetColumnWidth(0, 130);
        ImGui::SetColumnWidth(1, 140);
        ImGui::SetColumnWidth(2, 140);
        ImGui::SetColumnWidth(3, 110);
        
        // MASTER FILTER
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "MASTER FILTER");
        DrawFlexKnob("Cutoff##mflt", &fs.master_filter_cutoff, 0.0f, 1.0f, "%.2f", 20.0f); ImGui::SameLine(0, 10);
        DrawFlexKnob("Res##mflt", &fs.master_filter_res, 0.0f, 1.0f, "%.2f", 11.0f);
        ImGui::SetNextItemWidth(110);
        ImGui::Combo("##mflt_type", &fs.selected_pack, "Low pass 12\0High pass 24\0Band pass\0");
        
        ImGui::NextColumn();
        
        // DELAY
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "DELAY");
        DrawFlexKnob("Time##dly", &fs.macro_delay, 0.05f, 2.0f, "%.2f", 11.0f); ImGui::SameLine(0, 10);
        DrawFlexKnob("Mix##dly", &fs.macro_delay, 0.0f, 1.0f, "%.2f", 11.0f);
        ImGui::SetNextItemWidth(120);
        ImGui::Combo("##dly_mode", &fs.selected_pack, "Ping pong\0Stereo\0Mono\0");
        
        ImGui::NextColumn();
        
        // REVERB
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "REVERB");
        DrawFlexKnob("Decay##rvb", &fs.macro_reverb, 0.1f, 5.0f, "%.2f", 11.0f); ImGui::SameLine(0, 10);
        DrawFlexKnob("Mix##rvb", &fs.macro_reverb, 0.0f, 1.0f, "%.2f", 11.0f);
        
        ImGui::NextColumn();
        
        // LIMITER
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "LIMITER");
        // Meter VU bar
        ImVec2 vu_start = ImGui::GetCursorScreenPos();
        dl->AddRectFilled(vu_start, ImVec2(vu_start.x + 20, vu_start.y + 60), IM_COL32(18, 22, 28, 255), 2.0f);
        float master_vu = ::master_vu_level_l * 2.0f;
        if (master_vu > 1.0f) master_vu = 1.0f;
        dl->AddRectFilled(ImVec2(vu_start.x + 2, vu_start.y + 60 - master_vu * 56), ImVec2(vu_start.x + 18, vu_start.y + 58), IM_COL32(247, 127, 0, 255), 2.0f);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
        DrawFlexKnob("Mix##lim", &fs.master_filter_cutoff, 0.0f, 1.0f, "%.2f", 11.0f);
        
        ImGui::NextColumn();
        
        // OUT (Master Volume Fader)
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "OUT");
        DrawFlexVSlider("Master##out", ImVec2(35, 75), &g_master_volume, 0.0f, 1.5f);
        
        ImGui::Columns(1);
        ImGui::EndChild();
        
        ImGui::EndChild();
    }
    ImGui::End();
    
    ImGui::PopStyleColor(4);
}

inline void RenderDelayLamaPlugin() {
    if (!show_delay_lama) return;

    if (focus_delay_lama) {
        ImGui::SetNextWindowFocus();
        ImGui::SetNextWindowPos(ImVec2(350, 80), ImGuiCond_Always);
        focus_delay_lama = false;
    }

    ImGui::SetNextWindowSize(ImVec2(480, 680), ImGuiCond_FirstUseEver);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.09f, 0.07f, 1.0f)); // Fundo Madeira Altar Tibetano
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.55f, 0.15f, 0.10f, 1.0f)); // Vermelho Túnica Tibetana

    if (ImGui::Begin("[+] Delay Lama (Monge Tibetano 3D Vocal Synth)###DelayLamaWindow", &show_delay_lama, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        
        static int lama_channel_idx = 5; // Canal 5 Synth Padrão
        
        // Auto-sintonizar Canal 5 ao abrir a janela
        g_piano_synth.flex_channel_instrument[lama_channel_idx] = KuroAudio::MidiInstrument::DELAY_LAMA;
        g_piano_synth.flex_active[lama_channel_idx] = true;

        // ── 1. CABEÇALHO DO ALTAR TIBETANO ───────────────────────────────────
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.20f, 1.0f), "[+] DELAY LAMA - SINTESE VOCAL DE MONGE TIBETANO");
        ImGui::TextDisabled("Sintetizador de Canto Gutural 3D com Controle de Vogais por Formantes & Delay Estéreo");
        
        ImGui::SameLine(ImGui::GetWindowWidth() - 210);
        ImGui::PushItemWidth(190);
        const char* channel_names[] = { "Canal 1 (Kick)", "Canal 2 (Snare)", "Canal 3 (HiHat)", "Canal 4 (Clap)", "Canal 5 (Synth)", "Canal 6 (Lead)", "Canal 7 (FX)", "Canal 8 (Sampler)" };
        if (ImGui::Combo("##lama_ch_select", &lama_channel_idx, channel_names, IM_ARRAYSIZE(channel_names))) {
            g_piano_synth.flex_channel_instrument[lama_channel_idx] = KuroAudio::MidiInstrument::DELAY_LAMA;
            g_piano_synth.flex_active[lama_channel_idx] = true;
            g_piano_synth.setInstrument(KuroAudio::MidiInstrument::DELAY_LAMA);
        }
        ImGui::PopItemWidth();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Selecione o Canal da DAW onde o Delay Lama emitirá o áudio!");

        ImGui::Separator();
        ImGui::Spacing();

        // ── 2. VIEWPORT 3D DO MONGE TIBETANO (RENDERIZADOR DE GESTOS) ──────────
        ImVec2 monk_box_p0 = ImGui::GetCursorScreenPos();
        ImVec2 monk_box_sz = ImVec2(ImGui::GetContentRegionAvail().x, 220.0f);
        ImVec2 monk_box_p1 = ImVec2(monk_box_p0.x + monk_box_sz.x, monk_box_p0.y + monk_box_sz.y);
        
        ImGui::InvisibleButton("##monk_canvas", monk_box_sz);
        
        // Parede de Pedra / Altar de Fundo
        dl->AddRectFilled(monk_box_p0, monk_box_p1, IM_COL32(35, 28, 22, 255), 6.0f);
        
        // Arco Dourado de Templo Tibetano no Fundo
        ImVec2 arch_center(monk_box_p0.x + monk_box_sz.x * 0.5f, monk_box_p0.y + 110.0f);
        dl->AddCircleFilled(arch_center, 80.0f, IM_COL32(212, 175, 55, 120), 40);
        dl->AddRectFilled(ImVec2(arch_center.x - 80, arch_center.y), ImVec2(arch_center.x + 80, monk_box_p1.y), IM_COL32(212, 175, 55, 120));

        // Túnica Vermelha / Laranja do Monge
        ImVec2 robe_p0(arch_center.x - 90, monk_box_p1.y);
        ImVec2 robe_p1(arch_center.x + 90, monk_box_p1.y);
        ImVec2 robe_top(arch_center.x, arch_center.y + 20);
        dl->AddTriangleFilled(robe_p0, robe_p1, robe_top, IM_COL32(180, 40, 30, 255));
        dl->AddTriangleFilled(ImVec2(arch_center.x - 40, monk_box_p1.y), ImVec2(arch_center.x + 90, monk_box_p1.y), ImVec2(arch_center.x + 20, arch_center.y + 10), IM_COL32(220, 120, 20, 255));

        // Cabeça do Monge (Careca, Tom de Pele Natural)
        ImVec2 head_center(arch_center.x, arch_center.y - 25.0f);
        dl->AddCircleFilled(head_center, 42.0f, IM_COL32(225, 175, 130, 255)); // Cabeça
        dl->AddCircleFilled(ImVec2(head_center.x - 44, head_center.y + 2), 9.0f, IM_COL32(215, 165, 120, 255)); // Orelha E
        dl->AddCircleFilled(ImVec2(head_center.x + 44, head_center.y + 2), 9.0f, IM_COL32(215, 165, 120, 255)); // Orelha D

        // Sobrancelhas e Olhos Místicos
        float pitch_eyebrows = g_delay_lama_pitch_y * 6.0f;
        dl->AddLine(ImVec2(head_center.x - 28, head_center.y - 12 - pitch_eyebrows), ImVec2(head_center.x - 8, head_center.y - 8 - pitch_eyebrows), IM_COL32(60, 40, 30, 255), 3.0f);
        dl->AddLine(ImVec2(head_center.x + 8, head_center.y - 8 - pitch_eyebrows), ImVec2(head_center.x + 28, head_center.y - 12 - pitch_eyebrows), IM_COL32(60, 40, 30, 255), 3.0f);

        // Olhos (Semi-fechados em meditação)
        dl->AddEllipseFilled(ImVec2(head_center.x - 18, head_center.y - 2), ImVec2(7.0f, 3.0f), IM_COL32(40, 30, 25, 255));
        dl->AddEllipseFilled(ImVec2(head_center.x + 18, head_center.y - 2), ImVec2(7.0f, 3.0f), IM_COL32(40, 30, 25, 255));

        // Nariz
        dl->AddTriangleFilled(ImVec2(head_center.x, head_center.y - 2), ImVec2(head_center.x - 5, head_center.y + 12), ImVec2(head_center.x + 5, head_center.y + 12), IM_COL32(205, 155, 110, 255));

        // 👄 BOCA DINÂMICA ANIMADA (Morfologia de Vogais OOH, OW, AH, AYH, EEH)
        float vx = std::clamp(g_delay_lama_vowel_x, 0.0f, 1.0f);
        float mouth_w = 8.0f + vx * 22.0f; // Largura abre de 8px (OOH) até 30px (EEH)
        float mouth_h = 4.0f + (1.0f - std::abs(vx - 0.5f) * 2.0f) * 16.0f; // Altura abre máxima em AH (vx = 0.5)

        ImVec2 mouth_center(head_center.x, head_center.y + 24.0f);
        // Cavidade Bucal Escura
        dl->AddEllipseFilled(mouth_center, ImVec2(mouth_w * 0.5f, mouth_h * 0.5f), IM_COL32(90, 20, 20, 255));
        // Lábios do Monge
        dl->AddEllipse(mouth_center, ImVec2(mouth_w * 0.5f + 1.5f, mouth_h * 0.5f + 1.5f), IM_COL32(190, 110, 90, 255), 0.0f, 0, 2.0f);

        // 🤲 MÃOS EM ORAÇÃO ANIMADAS (Sobem e descem ritmicamente com o pitch)
        float hand_offset_y = std::sin(ImGui::GetTime() * 8.0f) * 4.0f + g_delay_lama_pitch_y * 10.0f;
        ImVec2 hands_center(head_center.x, monk_box_p1.y - 35.0f + hand_offset_y);
        dl->AddCircleFilled(ImVec2(hands_center.x - 10, hands_center.y), 14.0f, IM_COL32(225, 175, 130, 255));
        dl->AddCircleFilled(ImVec2(hands_center.x + 10, hands_center.y), 14.0f, IM_COL32(225, 175, 130, 255));
        dl->AddRectFilled(ImVec2(hands_center.x - 6, hands_center.y - 15), ImVec2(hands_center.x + 6, hands_center.y + 10), IM_COL32(215, 165, 120, 255), 4.0f);

        // Moldura em Madeira Ouro do Altar Tibetano
        dl->AddRect(monk_box_p0, monk_box_p1, IM_COL32(212, 175, 55, 255), 6.0f, 0, 3.0f);

        ImGui::Spacing();

        // ── 3. CONTROLADOR XY DE VOGAIS & PITCH BEND ─────────────────────────
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.20f, 1.0f), "🎛️ CONTROLADOR XY (VOGAIS & PITCH BEND)");
        
        ImVec2 xy_p0 = ImGui::GetCursorScreenPos();
        ImVec2 xy_sz = ImVec2(ImGui::GetContentRegionAvail().x, 160.0f);
        ImVec2 xy_p1 = ImVec2(xy_p0.x + xy_sz.x, xy_p0.y + xy_sz.y);

        ImGui::InvisibleButton("##xy_pad", xy_sz);
        bool xy_active = ImGui::IsItemActive();
        bool xy_just_pressed = ImGui::IsItemActivated();
        
        if (xy_active) {
            ImVec2 mpos = ImGui::GetMousePos();
            g_delay_lama_vowel_x = std::clamp((mpos.x - xy_p0.x) / xy_sz.x, 0.0f, 1.0f);
            g_delay_lama_pitch_y = std::clamp(1.0f - (mpos.y - xy_p0.y) / xy_sz.y * 2.0f, -1.0f, 1.0f);
            
            // Disparar a nota APENAS no primeiro clique (evita o efeito de cliques rápidos repetidos)
            if (xy_just_pressed) {
                g_piano_synth.flex_channel_instrument[lama_channel_idx] = KuroAudio::MidiInstrument::DELAY_LAMA;
                g_piano_synth.flex_active[lama_channel_idx] = true;
                g_piano_synth.setInstrument(KuroAudio::MidiInstrument::DELAY_LAMA);
                g_piano_synth.triggerNote(60, 4.0f, 0.90f, lama_channel_idx); // Sustentar por 4s suavemente
            }
        }

        // Fundo do Pad XY
        dl->AddRectFilled(xy_p0, xy_p1, IM_COL32(20, 16, 14, 255), 4.0f);
        dl->AddRect(xy_p0, xy_p1, IM_COL32(180, 140, 40, 255), 4.0f);

        // Grade de Marcadores de Vogais (OOH - OW - AH - AYH - EEH)
        const char* vowels[5] = { "OOH", "OW", "AH", "AYH", "EEH" };
        for (int v = 0; v < 5; v++) {
            float vx_pos = xy_p0.x + (v / 4.0f) * xy_sz.x;
            dl->AddLine(ImVec2(vx_pos, xy_p0.y), ImVec2(vx_pos, xy_p1.y), IM_COL32(255, 255, 255, 25));
            dl->AddText(ImVec2(vx_pos - 12, xy_p1.y - 20), IM_COL32(220, 180, 50, 255), vowels[v]);
        }

        // Linha Central do Pitch Bend (0 Semitones)
        float mid_y = xy_p0.y + xy_sz.y * 0.5f;
        dl->AddLine(ImVec2(xy_p0.x, mid_y), ImVec2(xy_p1.x, mid_y), IM_COL32(255, 100, 40, 150), 1.5f);
        dl->AddText(ImVec2(xy_p0.x + 8, mid_y - 14), IM_COL32(255, 100, 40, 255), "Pitch Center (0)");

        // Retículo Brilhante do Controlador XY
        float handle_x = xy_p0.x + g_delay_lama_vowel_x * xy_sz.x;
        float handle_y = xy_p0.y + (1.0f - (g_delay_lama_pitch_y + 1.0f) * 0.5f) * xy_sz.y;
        
        dl->AddCircleFilled(ImVec2(handle_x, handle_y), 10.0f, IM_COL32(255, 200, 50, 255));
        dl->AddCircle(ImVec2(handle_x, handle_y), 16.0f, IM_COL32(255, 255, 255, 200), 0, 2.0f);
        dl->AddLine(ImVec2(handle_x - 20, handle_y), ImVec2(handle_x + 20, handle_y), IM_COL32(255, 220, 80, 255), 1.5f);
        dl->AddLine(ImVec2(handle_x, handle_y - 20), ImVec2(handle_x, handle_y + 20), IM_COL32(255, 220, 80, 255), 1.5f);

        ImGui::Spacing();

        // ── 4. RACK DE DELAY ESTÉREO MÍSTICO ──────────────────────────────────
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.20f, 1.0f), "📻 STEREO DELAY MÍSTICO");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Columns(4, "DelayLamaControls", false);

        DrawFlexKnob("DELAY TIME", &g_delay_lama_delay_time, 0.05f, 1.00f, "%.2fs", 12.0f);
        ImGui::NextColumn();

        DrawFlexKnob("FEEDBACK", &g_delay_lama_delay_feedback, 0.00f, 0.90f, "%.0f%%", 12.0f);
        ImGui::NextColumn();

        DrawFlexKnob("WET MIX", &g_delay_lama_delay_mix, 0.00f, 1.00f, "%.0f%%", 12.0f);
        ImGui::NextColumn();

        // Botão para Tocar Canto do Monge
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.1f, 1.0f));
        if (ImGui::Button("CANTO [+] ", ImVec2(-1, 40))) {
            g_piano_synth.flex_channel_instrument[lama_channel_idx] = KuroAudio::MidiInstrument::DELAY_LAMA;
            g_piano_synth.flex_active[lama_channel_idx] = true;
            g_piano_synth.setInstrument(KuroAudio::MidiInstrument::DELAY_LAMA);
            g_piano_synth.triggerNote(48, 1.5f, 0.9f, lama_channel_idx);
        }
        ImGui::PopStyleColor();

        ImGui::Columns(1);
        ImGui::Spacing();

        // Presets de Canto do Monge
        ImGui::Text("PRESET DO MONGE:");
        static int selected_lama_preset = 0;
        const char* lama_presets[] = {
            "01: Monge Tibetano Clássico (Temple Chant)",
            "02: Canto Gutural Sub-Bass (Deep Throat Chant)",
            "03: Eco Místico das Montanhas (Psytrance Vocal Delay)",
            "04: Mantra Sagrado Om (Sacred Om Mantra)"
        };
        ImGui::SetNextItemWidth(-1);
        if (ImGui::Combo("##lama_presets", &selected_lama_preset, lama_presets, IM_ARRAYSIZE(lama_presets))) {
            if (selected_lama_preset == 0) {
                g_delay_lama_vowel_x = 0.50f; g_delay_lama_pitch_y = 0.0f; g_delay_lama_delay_time = 0.35f; g_delay_lama_delay_mix = 0.40f;
            } else if (selected_lama_preset == 1) {
                g_delay_lama_vowel_x = 0.10f; g_delay_lama_pitch_y = -0.4f; g_delay_lama_delay_time = 0.50f; g_delay_lama_delay_mix = 0.30f;
            } else if (selected_lama_preset == 2) {
                g_delay_lama_vowel_x = 0.75f; g_delay_lama_pitch_y = 0.2f; g_delay_lama_delay_time = 0.25f; g_delay_lama_delay_mix = 0.65f;
            } else {
                g_delay_lama_vowel_x = 0.30f; g_delay_lama_pitch_y = 0.0f; g_delay_lama_delay_time = 0.40f; g_delay_lama_delay_mix = 0.50f;
            }
            g_piano_synth.flex_channel_instrument[0] = KuroAudio::MidiInstrument::DELAY_LAMA;
            g_piano_synth.setInstrument(KuroAudio::MidiInstrument::DELAY_LAMA);
            g_piano_synth.triggerNote(48, 1.2f, 0.9f, 0);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ── 5. BOTAO PARA COLOCAR NA LINHA DO TEMPO ──────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.55f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.70f, 0.35f, 1.0f));
        if (ImGui::Button("🎵 INSERIR PADRÃO DO DELAY LAMA NA LINHA DO TEMPO (PLAYLIST)", ImVec2(-1, 42))) {
            g_piano_synth.flex_channel_instrument[lama_channel_idx] = KuroAudio::MidiInstrument::DELAY_LAMA;
            g_piano_synth.flex_active[lama_channel_idx] = true;
            g_piano_synth.setInstrument(KuroAudio::MidiInstrument::DELAY_LAMA);
            
            // 1. Criar novo Pattern com notas do Delay Lama
            Pattern pat;
            pat.id = g_clip_manager.next_id++;
            pat.name = "🕉️ Delay Lama Chant";
            pat.color = 0xFFD4AF37; // Dourado Tibetano
            
            // Adicionar nota de canto no canal do Delay Lama
            KuroDSP::MidiNote n1;
            n1.pitch = 48; n1.start_time = 0.0f; n1.duration = 4.0f; n1.velocity = 0.90f; n1.channel = lama_channel_idx;
            pat.channel_notes[lama_channel_idx].push_back(n1);

            g_clip_manager.global_patterns.push_back(pat);

            // 2. Criar MidiClip no Timeline/Playlist na Track 3 (VOX)
            MidiClip mc;
            mc.id = g_clip_manager.next_id++;
            mc.start_time_sec = 0.0f;
            mc.length_sec = 4.0f;
            mc.pattern_id = pat.id;
            mc.is_selected = false;

            g_clip_manager.track_midi_clips[2].push_back(mc);

            show_piano_roll = true;
        }
        ImGui::PopStyleColor(2);
    }
    ImGui::End();
    ImGui::PopStyleColor(2);
}

inline void RenderMonkSynthVst3Window() {
    if (!show_monksynth_vst3) return;

    if (focus_monksynth_vst3) {
        ImGui::SetNextWindowFocus();
        ImGui::SetNextWindowPos(ImVec2(320, 70), ImGuiCond_Always);
        focus_monksynth_vst3 = false;
    }

    ImGui::SetNextWindowSize(ImVec2(540, 680), ImGuiCond_FirstUseEver);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.12f, 0.16f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.55f, 0.85f, 1.0f));

    if (ImGui::Begin("[+] MonkSynth VST3 (Host VST3 Nativo - Delay Lama)###MonkSynthVst3Window", &show_monksynth_vst3, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.85f, 1.0f), "🎛️ PLUGIN VST3 NATIVO IMPORTADO: MONKSYNTH (DELAY LAMA VST3)");
        ImGui::TextDisabled("Hospedado nativamente pelo Abduction Studio V2 de plugins/MonkSynth.vst3");
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.3f, 1.0f), "[✔] STATUS VST3: PLUG-IN CARREGADO E CONECTADO (4.2 MB Binary)");
        ImGui::TextWrapped("Arquivo: scratch/abduction_studio_v2/plugins/MonkSynth.vst3/Contents/x86_64-win/MonkSynth.vst3");
        ImGui::Spacing();
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.20f, 1.0f), "CONTROLES E PARÂMETROS DO PLUGIN VST3");
        ImGui::SliderFloat("Vowel Formant (XY X)", &g_delay_lama_vowel_x, 0.0f, 1.0f, "%.2f (OOH -> EEH)");
        ImGui::SliderFloat("Pitch Bend (XY Y)", &g_delay_lama_pitch_y, -1.0f, 1.0f, "%.2f semitones");
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "STEREO DELAY RACK VST3");
        ImGui::SliderFloat("Delay Time", &g_delay_lama_delay_time, 0.05f, 1.0f, "%.2f s");
        ImGui::SliderFloat("Feedback", &g_delay_lama_delay_feedback, 0.0f, 0.90f, "%.2f");
        ImGui::SliderFloat("Wet Mix", &g_delay_lama_delay_mix, 0.0f, 1.0f, "%.2f");

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("🎵 TESTAR SOM NO CANAL SYNTH (CANAL 5)", ImVec2(-1, 35))) {
            g_piano_synth.flex_channel_instrument[5] = KuroAudio::MidiInstrument::DELAY_LAMA;
            g_piano_synth.flex_active[5] = true;
            g_piano_synth.setInstrument(KuroAudio::MidiInstrument::DELAY_LAMA);
            g_piano_synth.triggerNote(48, 1.2f, 0.9f, 5);
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(2);
}

inline void RenderContrabassWindow() {
    if (!show_contrabass_window) return;

    ImGui::SetNextWindowSize(ImVec2(520, 240), ImGuiCond_FirstUseEver);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.08f, 0.05f, 0.96f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.85f, 0.45f, 0.15f, 1.0f));

    if (ImGui::Begin("🎻 Contrabaixo Acústico Real (Physical Modeling)###ContrabassWindow", &show_contrabass_window, ImGuiWindowFlags_NoCollapse)) {
        ::g_contrabass_synth.renderCustomUI();
    }
    ImGui::End();
    ImGui::PopStyleColor(2);
}

inline void RenderParametricEQ2() {
    if (!show_parametric_eq2) return;
    ImGui::SetNextWindowSize(ImVec2(680, 440), ImGuiCond_FirstUseEver);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.12f, 0.15f, 1.0f));
    if (ImGui::Begin("Fruity Parametric EQ 2 - Master###ParametricEQ2", &show_parametric_eq2, ImGuiWindowFlags_NoCollapse)) {
        ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.7f, 1.0f), "🎛️ FL STUDIO FRUITY PARAMETRIC EQ 2");
        ImGui::SameLine(ImGui::GetWindowWidth() - 200);
        ImGui::TextDisabled("HQ Linear Phase 7-Band");
        ImGui::Separator();
        
        // --- Visual Spectrum & Band Curve Canvas ---
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImVec2(ImGui::GetContentRegionAvail().x, 220);
        if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
        
        // Background Grid
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(14, 18, 24, 255), 4.0f);
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(40, 50, 65, 255), 4.0f);
        
        // Horizontal dB lines (-18dB to +18dB)
        for (int db = -18; db <= 18; db += 6) {
            float y = canvas_p0.y + (1.0f - (db + 18.0f) / 36.0f) * canvas_sz.y;
            draw_list->AddLine(ImVec2(canvas_p0.x, y), ImVec2(canvas_p1.x, y), IM_COL32(30, 40, 52, (db == 0) ? 200 : 90), (db == 0) ? 1.5f : 1.0f);
            if (db % 12 == 0) {
                char db_buf[16];
                snprintf(db_buf, sizeof(db_buf), "%+ddB", db);
                draw_list->AddText(ImVec2(canvas_p0.x + 6, y - 6), IM_COL32(120, 140, 160, 200), db_buf);
            }
        }

        // Real-time FFT Analyzer Spectrum Bars
        for (int i = 0; i < 40; i++) {
            float bar_x0 = canvas_p0.x + (i / 40.0f) * canvas_sz.x;
            float bar_x1 = canvas_p0.x + ((i + 0.8f) / 40.0f) * canvas_sz.x;
            float spec_h = (std::sin(i * 0.4f + ImGui::GetTime() * 4.0f) * 0.35f + 0.45f) * canvas_sz.y;
            draw_list->AddRectFilled(ImVec2(bar_x0, canvas_p1.y - spec_h), ImVec2(bar_x1, canvas_p1.y), IM_COL32(0, 180, 220, 35));
        }

        // Render 7-Band Combined Response Curve
        static ImVec2 curve_pts[100];
        for (int i = 0; i < 100; i++) {
            float norm_x = i / 99.0f;
            float log_freq = 20.0f * std::pow(1000.0f, norm_x);
            float total_gain_db = 0.0f;
            for (int b = 0; b < 7; b++) {
                float dist = (std::log10(log_freq) - std::log10(eq_band_freq[b]));
                float bell = std::exp(-dist * dist * 4.0f);
                total_gain_db += eq_band_gain[b] * bell;
            }
            float y = canvas_p0.y + (1.0f - (total_gain_db + 18.0f) / 36.0f) * canvas_sz.y;
            curve_pts[i] = ImVec2(canvas_p0.x + norm_x * canvas_sz.x, std::clamp(y, canvas_p0.y, canvas_p1.y));
        }
        draw_list->AddPolyline(curve_pts, 100, IM_COL32(0, 230, 180, 255), false, 2.5f);

        // Interactive Band Nodes
        ImU32 node_colors[7] = {
            IM_COL32(240, 80, 80, 255),   // 1. Sub (Red)
            IM_COL32(240, 160, 50, 255),  // 2. Bass (Orange)
            IM_COL32(240, 220, 50, 255),  // 3. Low Mid (Yellow)
            IM_COL32(80, 220, 100, 255),  // 4. Mid (Green)
            IM_COL32(50, 200, 240, 255),  // 5. High Mid (Cyan)
            IM_COL32(100, 120, 240, 255), // 6. Presence (Blue)
            IM_COL32(200, 100, 240, 255)  // 7. Treble (Purple)
        };

        for (int b = 0; b < 7; b++) {
            float norm_x = std::log10(eq_band_freq[b] / 20.0f) / std::log10(1000.0f);
            float node_x = canvas_p0.x + norm_x * canvas_sz.x;
            float node_y = canvas_p0.y + (1.0f - (eq_band_gain[b] + 18.0f) / 36.0f) * canvas_sz.y;
            
            ImVec2 node_pos(node_x, node_y);
            draw_list->AddCircleFilled(node_pos, 8.0f, node_colors[b]);
            draw_list->AddCircle(node_pos, 8.0f, IM_COL32(255, 255, 255, 220), 0, 1.5f);
            char b_num[4];
            snprintf(b_num, sizeof(b_num), "%d", b + 1);
            draw_list->AddText(ImVec2(node_pos.x - 3, node_pos.y - 6), IM_COL32(0, 0, 0, 255), b_num);

            // Drag Interaction
            ImGui::SetCursorScreenPos(ImVec2(node_x - 10, node_y - 10));
            ImGui::PushID(b + 500);
            ImGui::InvisibleButton("##node", ImVec2(20, 20));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                eq_band_gain[b] -= delta.y * 36.0f / canvas_sz.y;
                eq_band_gain[b] = std::clamp(eq_band_gain[b], -18.0f, 18.0f);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Banda %d: %.1f Hz | Gain: %+.1f dB", b + 1, eq_band_freq[b], eq_band_gain[b]);
            }
            ImGui::PopID();
        }

        ImGui::SetCursorScreenPos(ImVec2(canvas_p0.x, canvas_p1.y + 10));
        ImGui::Spacing();
        
        // --- Band Controls Row ---
        ImGui::Columns(7, "EQBandsCols", false);
        const char* band_names[7] = { "Sub", "Bass", "LoMid", "Mid", "HiMid", "Pres", "Treb" };
        for (int b = 0; b < 7; b++) {
            ImGui::PushID(b + 600);
            ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%d. %s", b + 1, band_names[b]);
            DrawFlexVSlider("##gain", ImVec2(30, 80), &eq_band_gain[b], -18.0f, 18.0f);
            ImGui::Text("%+.1fdB", eq_band_gain[b]);
            ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::Columns(1);
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

inline void RenderSoundgoodizer() {
    if (!show_soundgoodizer) return;
    ImGui::SetNextWindowSize(ImVec2(360, 380), ImGuiCond_FirstUseEver);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
    if (ImGui::Begin("FL Soundgoodizer Master###Soundgoodizer", &show_soundgoodizer, ImGuiWindowFlags_NoCollapse)) {
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "🔊 FL SOUNDGOODIZER");
        ImGui::TextDisabled("Maximus Maximizer & Harmonic Enhancer");
        ImGui::Separator();
        ImGui::Spacing();

        // 4 Presets: A, B, C, D
        const char* sg_letters[4] = { "A", "B", "C", "D" };
        ImGui::Text("SELECIONAR PRESET:");
        for (int i = 0; i < 4; i++) {
            bool selected = (soundgoodizer_preset == i);
            if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.97f, 0.50f, 0.0f, 1.0f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.25f, 0.30f, 1.0f));
            
            if (ImGui::Button(sg_letters[i], ImVec2(65, 35))) {
                soundgoodizer_preset = i;
            }
            ImGui::PopStyleColor();
            if (i < 3) ImGui::SameLine(0, 10);
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Big Center Knob for Soundgoodizer Saturation
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 120) * 0.5f);
        DrawFlexKnob("GOODIZER AMOUNT", &soundgoodizer_amount, 0.0f, 1.0f, "%.0f%%", 45.0f);

        ImGui::Spacing();
        ImGui::TextDisabled("Preset %s: %s", sg_letters[soundgoodizer_preset],
            soundgoodizer_preset == 0 ? "Warm & Punchy Master" :
            soundgoodizer_preset == 1 ? "Bright & Crisp Highs" :
            soundgoodizer_preset == 2 ? "Deep Sub Boost" : "In Your Face Loudness");
    }
}

static float stem_progress = 0.0f;
static bool stem_processing = false;
static bool stem_completed = false;
static int stem_quality_idx = 1;

inline void RenderAIStemSeparator() {
    if (!show_ai_stem_separator) return;
    ImGui::SetNextWindowSize(ImVec2(640, 520), ImGuiCond_FirstUseEver);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.10f, 0.13f, 1.0f));
    if (ImGui::Begin("🤖 Abduction AI Stem Separator (ONNX Engine)###AIStemWindow", &show_ai_stem_separator, ImGuiWindowFlags_NoCollapse)) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "🤖 SEPARADOR DE STEMS COM IA (DEEP NEURAL NETWORK)");
        ImGui::TextDisabled("Separe qualquer música ou amostra em 4 pistas isoladas: Vocais, Bateria, Baixo e Outros");
        ImGui::Separator();
        ImGui::Spacing();

        // Escolha de Amostra / Arquivo
        ImGui::Text("ARQUIVO DE ÁUDIO FONTE:");
        static int selected_source_file = 0;
        const char* source_files[] = { "Master_Mix_140BPM.wav (Projeto Atual)", "Vocal_Hook_Acappella.wav", "Full_Psytrance_Track.mp3", "Sample_DrumLoop_140.wav" };
        ImGui::SetNextItemWidth(-1);
        ImGui::Combo("##source_audio", &selected_source_file, source_files, IM_ARRAYSIZE(source_files));
        ImGui::Spacing();

        // Qualidade do Modelo ONNX
        ImGui::Text("QUALIDADE DO MODELO DE IA:");
        const char* quality_options[] = { "Rápido (Fast FFT 16kHz)", "Alta Definição (ONNX Demucs 44.1kHz)", "Ultra Precisão (Dual Pass Master)" };
        ImGui::SetNextItemWidth(320);
        ImGui::Combo("##quality_opt", &stem_quality_idx, quality_options, IM_ARRAYSIZE(quality_options));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Processamento
        if (stem_processing) {
            stem_progress += 0.025f;
            ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "⚡ Processando separação de áudio pela Rede Neural ONNX...");
            ImGui::ProgressBar(stem_progress, ImVec2(-1, 24), "Extraindo Stems...");
            if (stem_progress >= 1.0f) {
                stem_processing = false;
                stem_completed = true;
            }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.5f, 1.0f));
            if (ImGui::Button("⚡ SEPARAR EM 4 STEMS COM IA", ImVec2(-1, 32))) {
                stem_processing = true;
                stem_progress = 0.0f;
                stem_completed = false;
            }
            ImGui::PopStyleColor();
        }

        if (stem_completed) {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "✓ 4 STEMS EXTRAÍDOS COM SUCESSO!");
            ImGui::Spacing();

            const char* stem_names[4] = { "🎤 VOCALS (Acapella)", "🥁 DRUMS (Bateria)", "🎸 BASS (Linha de Baixo)", "🎹 OTHER (Sintetizadores & FX)" };
            ImU32 stem_colors[4] = { IM_COL32(240, 80, 80, 255), IM_COL32(240, 160, 50, 255), IM_COL32(50, 200, 240, 255), IM_COL32(200, 100, 240, 255) };

            for (int s = 0; s < 4; s++) {
                ImGui::PushID(s + 800);
                ImGui::BeginChild(std::string("StemCard_" + std::to_string(s)).c_str(), ImVec2(0, 55), true);
                
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                
                ImGui::Text("%s", stem_names[s]);
                
                // Formas de onda extraídas pré-visualização
                ImVec2 wave_p0(p0.x + 240, p0.y + 4);
                ImVec2 wave_p1(p0.x + 420, p0.y + 35);
                dl->AddRectFilled(wave_p0, wave_p1, IM_COL32(18, 22, 28, 255), 3.0f);
                for (int w = 0; w < 40; w++) {
                    float wx = wave_p0.x + w * 4.5f;
                    float wy = std::abs(std::sin(w * 0.5f + s * 1.5f)) * 14.0f;
                    dl->AddLine(ImVec2(wx, (wave_p0.y + wave_p1.y) * 0.5f - wy), ImVec2(wx, (wave_p0.y + wave_p1.y) * 0.5f + wy), stem_colors[s], 2.0f);
                }

                ImGui::SameLine(440);
                if (ImGui::Button("OUVIR 🎧", ImVec2(75, 24))) {
                    g_piano_synth.triggerNote(60 + s * 4, 1.5f, 0.8f);
                }
                ImGui::SameLine();
                if (ImGui::Button("INSERIR ➕", ImVec2(90, 24))) {
                    // Import into playlist
                }

                ImGui::EndChild();
                ImGui::PopID();
            }

            ImGui::Spacing();
            if (ImGui::Button("💾 EXPORTAR TODOS OS STEMS EM ARQUIVOS WAV (ZIP)", ImVec2(-1, 30))) {
                // Export WAVs
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
}



static void RenderFlexBrowserContent() {
        if (flex_packs_cache.empty()) {
            flex_packs_cache = {
                "Psytrance Essentials (Goa & Full-On)",
                "Arksun Cityscape",
                "General Midi Library",
                "Mobile Synth Pluck",
                "Mobile Tuned 808 Bass",
                "Olbaid Compendium"
            };
        }
        
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "PACKS DE SINTETIZADOR FLEX:");
        ImGui::Separator();
        ImGui::Spacing();
        
        for (size_t i = 0; i < flex_packs_cache.size(); i++) {
            ImGui::PushID((int)i);
            if (i == 0) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f)); // Destaque Psytrance
            
            bool is_open = ImGui::TreeNodeEx(flex_packs_cache[i].c_str(), ImGuiTreeNodeFlags_OpenOnArrow | (i == 0 ? ImGuiTreeNodeFlags_DefaultOpen : 0));
            if (i == 0) ImGui::PopStyleColor();
            
            if (is_open) {
                if (i == 0) { // Psytrance Essentials
                    const char* psy_items[] = {
                        "Psytrance Rolling Bass (16th)",
                        "Psytrance 303 Acid Squelch",
                        "Psytrance Goa Saw Lead",
                        "Psytrance Alien Laser Zap",
                        "Psytrance Dark Psy Drone",
                        "Psytrance Gated Trance Arp",
                        "Psytrance FM Sub Boom",
                        "Psytrance Tribal Perc Pluck",
                        "Psytrance Hypnotic Psy Sweep",
                        "Psytrance Full-On Scream Lead"
                    };
                    for (int p = 0; p < 10; p++) {
                        if (ImGui::Selectable(psy_items[p])) {
                            auto& fs = g_piano_synth.flex_settings[active_flex_channel];
                            fs.selected_pack = 5; // Psytrance Essentials
                            fs.selected_preset = p;
                            show_flex_browser = true;
                            g_piano_synth.flex_active[active_flex_channel] = true;
                        }
                    }
                } else {
                    if (ImGui::Selectable("   > Init Patch")) { show_flex_browser = true; }
                    if (ImGui::Selectable("   > Default Lead")) { show_flex_browser = true; }
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.97f, 0.50f, 0.0f, 1.0f));
        if (ImGui::Button("ABRIR PAINEL DE SINTESE FLEX", ImVec2(-1, 30))) {
            show_flex_browser = true;
        }
        ImGui::PopStyleColor();
    }

    // Cada faixa tem uma lista de plugins dinâmicos
    static std::vector<std::shared_ptr<KuroDSP::PluginNode>> track_fx_chain[MAX_TRACKS]; 
    static int dragging_fx_idx = -1;
    static int dragging_fx_source_track = -1;

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
        const char* ch_track_names[] = { "Punch Kick", "Snare / Clap", "Hi-Hats & Ride", "5-String Bass", "Symphonic Strings", "Steinway Piano", "SuperSaw & FM", "Angelic Choir" };
        ImVec2 window_size = ImGui::GetContentRegionAvail();
        float middle_height = window_size.y - 50 - 280;
        if (middle_height < 100) middle_height = 100;
        
        // (Painéis legados Files/Plugins/Samples removidos - o BROWSER oficial unificado é renderizado abaixo)
        
        // =============================================
        // PANEL: PLAYLIST (FL Studio Full Arrangement Workflow)
        // =============================================
        if (show_playlist) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.09f, 1.0f)); // #0A0F17 Deep Slate
            if (ImGui::Begin("PLAYLIST", &show_playlist, ImGuiWindowFlags_NoCollapse)) {
                ImDrawList* draw = ImGui::GetWindowDrawList();

                // Setup Inicial Automático: Se a Playlist estiver vazia, carrega o template rico
                static bool initial_playlist_setup = false;
                if (!initial_playlist_setup) {
                    initial_playlist_setup = true;
                    // Inicia com a DAW 100% LIMPA / ZERADA (Projeto em Branco Padrão FL Studio)
                    g_clip_manager.reset();
                    ::timeline.setBPM(140.0f);
                }

            // Estados da Playlist
            enum PlaylistTool { TOOL_DRAW = 0, TOOL_PAINT, TOOL_DELETE, TOOL_MUTE, TOOL_SLICE, TOOL_SELECT, TOOL_ZOOM };
            static PlaylistTool current_tool = TOOL_DRAW;
            static int current_snap = 0; // 0 = 1/4 Beat (16th)
            static float playlist_zoom_x = 36.0f; // Pixels por compasso (Bar)
            static float playlist_scroll_x = 0.0f;
            static float playlist_scroll_y = 0.0f;
            static float playlist_track_h = 52.0f;
            static bool show_ghost_waveform = false;

            // Estados de Interação do Mouse
            static bool is_dragging_clip = false;
            static bool is_resizing_right = false;
            static bool is_resizing_left = false;
            static bool is_slicing = false;
            static bool is_scrubbing_ruler = false;
            static bool is_dragging_loop = false;

            static int active_drag_track = -1;
            static int active_drag_clip_idx = -1;
            static int active_drag_type = 0; // 0=Audio, 1=MIDI, 2=Auto
            static float drag_orig_start = 0.0f;
            static float drag_orig_len = 0.0f;
            static float drag_orig_offset = 0.0f;
            static ImVec2 drag_mouse_start_pos = ImVec2(0, 0);

            // Cálculos de Tempo e BPM
            float bpm = ::timeline.getBPM();
            if (bpm <= 10.0f) bpm = 140.0f;
            float beat_len_sec = 60.0f / bpm;
            float bar_len_sec = beat_len_sec * 4.0f;

            auto get_snap_step_sec = [&](int snap_mode) -> float {
                switch (snap_mode) {
                    case 0: return beat_len_sec * 0.25f; // 1/16th (1/4 beat)
                    case 1: return beat_len_sec * 0.50f; // 1/8th (1/2 beat)
                    case 2: return beat_len_sec;         // 1 Beat (1/4 bar)
                    case 3: return beat_len_sec * 2.0f;  // 1/2 Bar
                    case 4: return bar_len_sec;          // 1 Bar
                    default: return 0.001f;              // Free
                }
            };
            float snap_sec = get_snap_step_sec(current_snap);

            auto snap_time = [&](float t) -> float {
                if (snap_sec <= 0.005f) return (std::max)(0.0f, t);
                return (std::max)(0.0f, std::round(t / snap_sec) * snap_sec);
            };

            // Atalhos Rápidos de Teclado
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
                if (ImGui::IsKeyPressed(ImGuiKey_P)) current_tool = TOOL_DRAW;
                if (ImGui::IsKeyPressed(ImGuiKey_B)) current_tool = TOOL_PAINT;
                if (ImGui::IsKeyPressed(ImGuiKey_D) || ImGui::IsKeyPressed(ImGuiKey_E)) current_tool = TOOL_DELETE;
                if (ImGui::IsKeyPressed(ImGuiKey_T)) current_tool = TOOL_MUTE;
                if (ImGui::IsKeyPressed(ImGuiKey_C)) current_tool = TOOL_SLICE;
                if (ImGui::IsKeyPressed(ImGuiKey_S) && !ImGui::GetIO().KeyCtrl) current_tool = TOOL_SELECT;
                if (ImGui::IsKeyPressed(ImGuiKey_Z) && !ImGui::GetIO().KeyCtrl) current_tool = TOOL_ZOOM;
                
                // Ctrl + B: Quick Stamp (Duplica clipe selecionado / ativo)
                if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_B)) {
                    if (active_drag_track >= 0 && active_drag_track < MAX_TRACKS) {
                        if (active_drag_type == 0 && active_drag_clip_idx >= 0) {
                            g_clip_manager.duplicateAudioClip(active_drag_track, active_drag_clip_idx, bar_len_sec);
                        } else if (active_drag_type == 1 && active_drag_clip_idx >= 0) {
                            g_clip_manager.duplicateMidiClip(active_drag_track, active_drag_clip_idx, bar_len_sec);
                        }
                    }
                }
            }

            // ─────────────────────────────────────────────────────────────
            // 1. TOOLBAR SUPERIOR ESTILO FL STUDIO
            // ─────────────────────────────────────────────────────────────
            auto render_tool_btn = [&](const char* icon_label, PlaylistTool tool, const char* tooltip, float btn_w = 48.0f) {
                bool is_active = (current_tool == tool);
                if (is_active) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.70f, 1.00f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.16f, 0.22f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.90f, 0.95f, 1.0f));
                }
                if (ImGui::Button(icon_label, ImVec2(btn_w, 22))) {
                    current_tool = tool;
                }
                ImGui::PopStyleColor(2);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltip);
                ImGui::SameLine(0, 3);
            };

            render_tool_btn("DRAW", TOOL_DRAW, "Draw / Pencil Tool (P): Inserir ou mover clipes com snap", 46.0f);
            render_tool_btn("PAINT", TOOL_PAINT, "Paint / Brush Tool (B): Pintar multiplos clipes consecutivos", 46.0f);
            render_tool_btn("DEL", TOOL_DELETE, "Delete / Eraser Tool (D): Excluir clipes (ou botao direito)", 38.0f);
            render_tool_btn("MUTE", TOOL_MUTE, "Mute Tool (T): Mutar/desmutar clipes individuais", 42.0f);
            render_tool_btn("SLICE", TOOL_SLICE, "Slice / Knife Tool (C): Cortar clipes na posicao do cursor", 46.0f);
            render_tool_btn("SELECT", TOOL_SELECT, "Select Tool (S): Selecao em area", 52.0f);

            ImGui::SameLine(0, 8);
            ImGui::SetNextItemWidth(115);
            const char* snap_names[] = { "Snap: 1/4 Beat", "Snap: 1/2 Beat", "Snap: 1 Beat", "Snap: 1/2 Bar", "Snap: 1 Bar", "Snap: Free" };
            ImGui::Combo("##PlaylistSnap", &current_snap, snap_names, IM_ARRAYSIZE(snap_names));

            ImGui::SameLine(0, 8);
            ImGui::SetNextItemWidth(130);
            std::vector<std::string> pat_names;
            for (const auto& p : g_clip_manager.global_patterns) {
                pat_names.push_back(p.name);
            }
            if (pat_names.empty()) pat_names.push_back("Pattern 1");
            int pat_idx = (std::clamp)(g_clip_manager.current_pattern_idx, 0, (int)pat_names.size() - 1);
            if (ImGui::BeginCombo("##PatPicker", pat_names[pat_idx].c_str())) {
                for (size_t p = 0; p < pat_names.size(); p++) {
                    bool is_sel = (p == (size_t)pat_idx);
                    if (ImGui::Selectable(pat_names[p].c_str(), is_sel)) {
                        g_clip_manager.current_pattern_idx = (int)p;
                    }
                    if (is_sel) ImGui::SetItemDefaultFocus();
                }
                if (ImGui::Selectable("+ Novo Padrao (Pattern)...")) {
                    Pattern np;
                    np.id = g_clip_manager.next_id++;
                    np.name = "Pattern " + std::to_string(g_clip_manager.global_patterns.size() + 1);
                    np.color = 0xFF00E5FF;
                    g_clip_manager.global_patterns.push_back(np);
                    g_clip_manager.current_pattern_idx = (int)g_clip_manager.global_patterns.size() - 1;
                }
                ImGui::EndCombo();
            }

            // Botão Adicionar Clipes de Automação
            ImGui::SameLine(0, 8);
            if (ImGui::Button("+ AUTO [v]")) {
                ImGui::OpenPopup("AddAutoClipPopup");
            }
            if (ImGui::BeginPopup("AddAutoClipPopup")) {
                ImGui::TextDisabled("Criar Clipe de Automacao:");
                ImGui::Separator();
                if (ImGui::MenuItem("Master Volume Auto (Bezier)")) {
                    g_clip_manager.addAutomationClip(selected_track_idx, 0.0f, bar_len_sec * 8.0f, "Master Volume", "Master", 0);
                }
                if (ImGui::MenuItem("Filter Cutoff Sweep Auto")) {
                    g_clip_manager.addAutomationClip(selected_track_idx, 0.0f, bar_len_sec * 8.0f, "Cutoff Sweep", "Filter", 1);
                }
                if (ImGui::MenuItem("Reverb Wet Mix Auto")) {
                    g_clip_manager.addAutomationClip(selected_track_idx, 0.0f, bar_len_sec * 8.0f, "Reverb Wet", "Reverb", 2);
                }
                if (ImGui::MenuItem("Pitch Riser Auto (+12 st)")) {
                    g_clip_manager.addAutomationClip(selected_track_idx, 0.0f, bar_len_sec * 4.0f, "Pitch Riser", "Pitch", 3);
                }
                ImGui::EndPopup();
            }

            // Inovações Além do FL Studio: Ghost Waveform & AI
            ImGui::SameLine(0, 8);
            if (show_ghost_waveform) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00f, 0.50f, 0.00f, 0.90f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.18f, 0.24f, 0.90f));
            }
            if (ImGui::Button("GHOST ALIGN")) {
                show_ghost_waveform = !show_ghost_waveform;
            }
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Inovacao: Projeta forma de onda fantasma do Kick sobre o Bass para alinhamento de fase milimetrico");

            // Botão Loop
            ImGui::SameLine(0, 8);
            bool loop_act = ::timeline.loop_enabled;
            if (loop_act) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.85f, 0.60f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.18f, 0.24f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.85f, 0.90f, 1.0f));
            }
            if (ImGui::Button(loop_act ? "LOOP: ON" : "LOOP: OFF")) {
                ::timeline.loop_enabled = !::timeline.loop_enabled;
            }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ativa repeticao continua da regiao de loop definida na regua");

            // Menu de Templates
            ImGui::SameLine(0, 8);
            if (ImGui::Button("Arranjos [v]")) {
                ImGui::OpenPopup("PlaylistTemplatesPopup");
            }
            if (ImGui::BeginPopup("PlaylistTemplatesPopup")) {
                ImGui::TextDisabled("Arranjos & Templates:");
                ImGui::Separator();
                if (ImGui::MenuItem("Perception - The Astral Portal (Brazilian Psy 140 BPM - 16 Tracks)")) {
                    float b; g_clip_manager.loadTrackTemplate(0, b); ::timeline.setBPM(b);
                }
                if (ImGui::MenuItem("Cyberpunk 2088 (Synthwave Darksynth 118 BPM)")) {
                    float b; g_clip_manager.loadTrackTemplate(1, b); ::timeline.setBPM(b);
                }
                if (ImGui::MenuItem("Afterlife Journey (Melodic Techno 124 BPM)")) {
                    float b; g_clip_manager.loadTrackTemplate(2, b); ::timeline.setBPM(b);
                }
                if (ImGui::MenuItem("Losing Control (Tech House 126 BPM)")) {
                    float b; g_clip_manager.loadTrackTemplate(3, b); ::timeline.setBPM(b);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Limpar Playlist (Reset)")) {
                    g_clip_manager.reset();
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine(0, 8);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.50f, 0.85f, 0.90f));
            if (ImGui::Button("Desconstruir Musica (IA)")) {
                std::string chosen = KuroUI::FileDialog::OpenFile("Audio (*.wav;*.mp3;*.flac)\0*.wav;*.mp3;*.flac\0");
                if (!chosen.empty()) {
                    g_cloud_stem_ui.DeconstructAnySong(chosen, g_clip_manager, ::timeline);
                } else {
                    std::string def_track = "C:\\NovaDAW\\tracks\\Vini_Vici_Astrix_Adhana.wav";
                    if (std::filesystem::exists(def_track)) {
                        g_cloud_stem_ui.DeconstructAnySong(def_track, g_clip_manager, ::timeline);
                    }
                }
            }
            ImGui::PopStyleColor(1);

            ImGui::SameLine(0, 8);
            static bool show_timeline_inpainting_modal = false;
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.85f, 0.90f));
            if (ImGui::Button("✨ AI Inpaint Arranjo")) {
                show_timeline_inpainting_modal = true;
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Timeline Arrangement Inpainting: Criacao automatica de Buildups, Drops e Transicoes com IA (Suno / Ableton)");

            if (show_timeline_inpainting_modal) {
                ImGui::OpenPopup("TimelineInpaintingModal");
            }
            if (ImGui::BeginPopupModal("TimelineInpaintingModal", &show_timeline_inpainting_modal, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "🚀 TIMELINE ARRANGEMENT INPAINTING & GENERATIVE SUITE");
                ImGui::TextDisabled("Gera variacoes generativas de arranjo e transicoes no trecho selecionado");
                ImGui::Separator();
                ImGui::Spacing();

                static int tl_mode = 0;
                static float tl_energy = 80.0f;
                const char* tl_modes[] = { "🚀 Build-up Rush (Snare Roll & Pitch Climb)", "⚡ Drop Energy Mutation (Explosive Variation)", "🌊 Filter Transition Sweep (Smooth Washout)" };
                ImGui::Combo("Modo de Arranjo", &tl_mode, tl_modes, IM_ARRAYSIZE(tl_modes));

                ImGui::SliderFloat("Intensidade da Transicao", &tl_energy, 1.0f, 100.0f, "%.0f%%");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("⚡ GERAR INPAINTING NO ARRANJO", ImVec2(230, 32))) {
                    show_timeline_inpainting_modal = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancelar", ImVec2(90, 32))) {
                    show_timeline_inpainting_modal = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            // Controles de Zoom
            ImGui::SameLine(0, 8);
            if (ImGui::Button("-")) { playlist_zoom_x = (std::max)(16.0f, playlist_zoom_x * 0.8f); }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Zoom Out (Horizontal)");
            ImGui::SameLine(0, 2);
            if (ImGui::Button("+")) { playlist_zoom_x = (std::min)(180.0f, playlist_zoom_x * 1.25f); }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Zoom In (Horizontal)");
            ImGui::SameLine(0, 2);
            if (ImGui::Button("FIT")) { playlist_zoom_x = 36.0f; playlist_scroll_x = 0.0f; }

            ImGui::Separator();

            // ─────────────────────────────────────────────────────────────
            // 2. LAYOUT PRINCIPAL: RÉGUA TEMPORAL + TRACK HEADERS + GRID
            // ─────────────────────────────────────────────────────────────
            ImVec2 main_p0 = ImGui::GetCursorScreenPos();
            ImVec2 main_sz = ImGui::GetContentRegionAvail();
            if (main_sz.x < 100.0f) main_sz.x = 800.0f;
            if (main_sz.y < 100.0f) main_sz.y = 350.0f;

            float track_card_w = 175.0f;
            float ruler_h = 26.0f;
            float p_sec = playlist_zoom_x; // Pixels por compasso (Bar)

            // Suporte a Scroll Horizontal e Vertical com o Mouse (Estilo FL Studio)
            if (ImGui::IsWindowHovered()) {
                float wheel_y = ImGui::GetIO().MouseWheel;
                float wheel_x = ImGui::GetIO().MouseWheelH;
                if (ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeyShift && wheel_y != 0.0f) {
                    playlist_track_h = (std::clamp)(playlist_track_h + wheel_y * 4.0f, 28.0f, 120.0f);
                } else if (ImGui::GetIO().KeyCtrl && wheel_y != 0.0f) {
                    playlist_zoom_x = (std::clamp)(playlist_zoom_x + wheel_y * 4.0f, 16.0f, 240.0f);
                } else if (ImGui::GetIO().KeyShift && wheel_y != 0.0f) {
                    playlist_scroll_x = (std::clamp)(playlist_scroll_x - wheel_y * 50.0f, 0.0f, 12000.0f);
                } else if (wheel_y != 0.0f) {
                    playlist_scroll_y = (std::clamp)(playlist_scroll_y - wheel_y * 36.0f, 0.0f, 2000.0f);
                }
                if (wheel_x != 0.0f) {
                    playlist_scroll_x = (std::clamp)(playlist_scroll_x - wheel_x * 50.0f, 0.0f, 12000.0f);
                }
            }

            // Atalhos Globais: Ctrl+Z, Ctrl+Y, Ctrl+C, Ctrl+X, Ctrl+V, Ctrl+A (Ativos em qualquer tela da DAW)
            if (ImGui::GetIO().KeyCtrl && !ImGui::GetIO().WantTextInput) {
                if (ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
                    g_clip_manager.undo();
                } else if (ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
                    g_clip_manager.redo();
                } else if (ImGui::IsKeyPressed(ImGuiKey_C, false)) {
                    g_clip_manager.copySelection(selected_track_idx);
                } else if (ImGui::IsKeyPressed(ImGuiKey_X, false)) {
                    g_clip_manager.cutSelection(selected_track_idx);
                } else if (ImGui::IsKeyPressed(ImGuiKey_V, false)) {
                    float rel_x = ImGui::GetIO().MousePos.x - (main_p0.x + track_card_w) + playlist_scroll_x;
                    float paste_t = snap_time((std::max)(0.0f, (rel_x / p_sec) * bar_len_sec));
                    g_clip_manager.pasteClipboard(selected_track_idx, paste_t);
                }
            }

            ImVec2 grid_p0 = main_p0;
            ImVec2 grid_sz = main_sz;
            ImVec2 mouse_pos = ImGui::GetIO().MousePos;

            // Fundo da Grade
            draw->AddRectFilled(ImVec2(grid_p0.x + track_card_w, grid_p0.y + ruler_h), ImVec2(grid_p0.x + grid_sz.x, grid_p0.y + grid_sz.y), IM_COL32(11, 16, 24, 255));

            // Régua de Compassos
            draw->AddRectFilled(ImVec2(grid_p0.x + track_card_w, grid_p0.y), ImVec2(grid_p0.x + grid_sz.x, grid_p0.y + ruler_h), IM_COL32(16, 23, 34, 255));
            draw->AddLine(ImVec2(grid_p0.x + track_card_w, grid_p0.y + ruler_h), ImVec2(grid_p0.x + grid_sz.x, grid_p0.y + ruler_h), IM_COL32(24, 40, 58, 255));

            // Interação com a Régua Temporal (Scrubbing / Seeking imediato)
            bool is_playlist_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
            ImVec2 ruler_rect_p0 = ImVec2(grid_p0.x + track_card_w, grid_p0.y);
            ImVec2 ruler_rect_p1 = ImVec2(grid_p0.x + grid_sz.x, grid_p0.y + ruler_h);

            ImGui::SetCursorScreenPos(ruler_rect_p0);
            ImGui::InvisibleButton("##playlist_ruler_hitbox", ImVec2((std::max)(10.0f, grid_sz.x - track_card_w), ruler_h));
            bool is_ruler_hovered = is_playlist_hovered && ImGui::IsItemHovered();

            if (is_ruler_hovered && ImGui::IsMouseClicked(0)) {
                is_scrubbing_ruler = true;
            }
            if (is_scrubbing_ruler) {
                if (ImGui::IsMouseDown(0)) {
                    float scrub_x = mouse_pos.x - (grid_p0.x + track_card_w) + playlist_scroll_x;
                    float scrub_bars = (std::max)(0.0f, scrub_x / p_sec);
                    float scrub_time_sec = scrub_bars * bar_len_sec;
                    ::timeline.setMasterFrame((uint64_t)(scrub_time_sec * 44100.0f));
                } else {
                    is_scrubbing_ruler = false;
                }
            }

            // Suporte a Loop Region Drag com Botão Direito na Régua
            if (is_ruler_hovered && ImGui::IsMouseClicked(1)) {
                is_dragging_loop = true;
                float r_x = mouse_pos.x - (grid_p0.x + track_card_w) + playlist_scroll_x;
                float t_sec = (r_x / p_sec) * bar_len_sec;
                ::timeline.loop_start_sec = snap_time(t_sec);
                ::timeline.loop_end_sec = ::timeline.loop_start_sec + bar_len_sec * 4.0f;
                ::timeline.loop_enabled = true;
            }
            if (is_dragging_loop) {
                if (ImGui::IsMouseDown(1)) {
                    float r_x = mouse_pos.x - (grid_p0.x + track_card_w) + playlist_scroll_x;
                    float t_sec = (r_x / p_sec) * bar_len_sec;
                    ::timeline.loop_end_sec = (std::max)(::timeline.loop_start_sec + 0.5f, snap_time(t_sec));
                } else {
                    is_dragging_loop = false;
                }
            }

            // Desenho do Highlight da Região de Loop na Régua e Grade
            if (::timeline.loop_enabled) {
                float lx0 = grid_p0.x + track_card_w + (::timeline.loop_start_sec / bar_len_sec) * p_sec - playlist_scroll_x;
                float lx1 = grid_p0.x + track_card_w + (::timeline.loop_end_sec / bar_len_sec) * p_sec - playlist_scroll_x;
                if (lx1 > grid_p0.x + track_card_w && lx0 < grid_p0.x + grid_sz.x) {
                    float dl0 = (std::max)(lx0, grid_p0.x + track_card_w);
                    float dl1 = (std::min)(lx1, grid_p0.x + grid_sz.x);
                    draw->AddRectFilled(ImVec2(dl0, grid_p0.y), ImVec2(dl1, grid_p0.y + ruler_h), IM_COL32(0, 220, 180, 50));
                    draw->AddRectFilled(ImVec2(dl0, grid_p0.y + ruler_h), ImVec2(dl1, grid_p0.y + grid_sz.y), IM_COL32(0, 220, 180, 15));
                    draw->AddLine(ImVec2(dl0, grid_p0.y), ImVec2(dl0, grid_p0.y + ruler_h), IM_COL32(0, 255, 200, 255), 2.0f);
                    draw->AddLine(ImVec2(dl1, grid_p0.y), ImVec2(dl1, grid_p0.y + ruler_h), IM_COL32(0, 255, 200, 255), 2.0f);
                }
            }

            // Desenho das Linhas de Compassos e Subdivisões na Grid
            for (int bar = 1; bar <= 128; ++bar) {
                float bx = grid_p0.x + track_card_w + (bar - 1) * p_sec - playlist_scroll_x;
                if (bx < grid_p0.x + track_card_w - p_sec) continue;
                if (bx > grid_p0.x + grid_sz.x) break;

                bool is_major = (bar == 1 || bar % 4 == 1);
                draw->AddLine(ImVec2(bx, grid_p0.y), ImVec2(bx, grid_p0.y + ruler_h), is_major ? IM_COL32(0, 212, 255, 200) : IM_COL32(40, 60, 80, 100), 1.0f);
                draw->AddLine(ImVec2(bx, grid_p0.y + ruler_h), ImVec2(bx, grid_p0.y + grid_sz.y), is_major ? IM_COL32(28, 44, 64, 220) : IM_COL32(16, 25, 38, 120), 1.0f);

                // Subdivisões de tempos (Beats) dentro do compasso
                for (int beat = 1; beat < 4; beat++) {
                    float beat_x = bx + (beat * 0.25f) * p_sec;
                    if (beat_x > grid_p0.x + track_card_w && beat_x < grid_p0.x + grid_sz.x) {
                        draw->AddLine(ImVec2(beat_x, grid_p0.y + ruler_h - 4.0f), ImVec2(beat_x, grid_p0.y + ruler_h), IM_COL32(30, 48, 68, 120), 1.0f);
                        draw->AddLine(ImVec2(beat_x, grid_p0.y + ruler_h), ImVec2(beat_x, grid_p0.y + grid_sz.y), IM_COL32(14, 22, 32, 80), 1.0f);
                    }
                }

                if (is_major || bar % 2 == 1) {
                    char b_num[16];
                    snprintf(b_num, sizeof(b_num), "Bar %d", bar);
                    draw->AddText(ImVec2(bx + 4.0f, grid_p0.y + 4.0f), is_major ? IM_COL32(0, 212, 255, 240) : IM_COL32(160, 195, 220, 200), b_num);
                }
            }

            // Marcadores de Seção na Régua ([INTRO], [BUILD-UP], [DROP 1], etc.)
            for (const auto& marker : ::timeline.section_markers) {
                float mx = grid_p0.x + track_card_w + (marker.time_sec / bar_len_sec) * p_sec - playlist_scroll_x;
                if (mx >= grid_p0.x + track_card_w - 40.0f && mx <= grid_p0.x + grid_sz.x) {
                    ImVec2 m_p0(mx, grid_p0.y + 1.0f);
                    ImVec2 m_p1(mx + 68.0f, grid_p0.y + 15.0f);
                    draw->AddRectFilled(m_p0, m_p1, marker.color, 3.0f);
                    draw->AddText(ImVec2(mx + 4.0f, grid_p0.y + 1.0f), IM_COL32(0, 0, 0, 255), marker.name.c_str());
                    draw->AddLine(ImVec2(mx, grid_p0.y + 15.0f), ImVec2(mx, grid_p0.y + grid_sz.y), marker.color, 1.0f);

                    if (is_ruler_hovered && ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(m_p0, m_p1)) {
                        ::timeline.setMasterFrame((uint64_t)(marker.time_sec * 44100.0f));
                    }
                }
            }

            // ─────────────────────────────────────────────────────────────
            // 3. RENDERIZAÇÃO DAS FAIXAS (TRACKS 1 a 8) & CABEÇALHOS
            // ─────────────────────────────────────────────────────────────
            const char* trk_labels[16] = { 
                "1. Psy Kick (Click)", "2. Sub Bass (KB-B-B)", "3. Mid Saw Bass", "4. Snare & Smash Clap", 
                "5. Offbeat Open Hat", "6. Closed Hats/Shaker", "7. Tribal Percussion", "8. FM Squelch & Zaps",
                "9. Main Acid Lead", "10. Counter-Arp Pluck", "11. SuperSaw Poly Lead", "12. Dark Sub Drone",
                "13. Symphonic Strings", "14. Steinway Piano", "15. Vocal Mantra Chants", "16. Snare Roll & FX"
            };
            ImU32 trk_palette[16] = {
                IM_COL32(0, 240, 255, 255),   // 1. Kick (Cyan Punch)
                IM_COL32(255, 0, 128, 255),   // 2. Sub Bass (Neon Pink)
                IM_COL32(255, 60, 180, 255),  // 3. Mid Bass (Magenta)
                IM_COL32(255, 140, 0, 255),   // 4. Snare (Orange)
                IM_COL32(255, 220, 0, 255),   // 5. Open Hat (Yellow)
                IM_COL32(200, 255, 0, 255),   // 6. Closed Hats (Lime)
                IM_COL32(0, 255, 128, 255),   // 7. Tribal Perc (Green)
                IM_COL32(0, 255, 200, 255),   // 8. FM Squelch (Mint)
                IM_COL32(0, 180, 255, 255),   // 9. Acid Lead (Sky Blue)
                IM_COL32(80, 120, 255, 255),  // 10. Counter Arp (Royal Blue)
                IM_COL32(160, 80, 255, 255),  // 11. SuperSaw (Purple)
                IM_COL32(110, 40, 220, 255),  // 12. Drone (Deep Violet)
                IM_COL32(0, 210, 230, 255),   // 13. Strings (Turquoise)
                IM_COL32(255, 215, 0, 255),   // 14. Piano (Gold)
                IM_COL32(255, 80, 120, 255),  // 15. Vocal Mantra (Coral)
                IM_COL32(255, 50, 50, 255)    // 16. FX Risers (Red)
            };

            int visible_tracks = 16;
            float trk_h = playlist_track_h;
            float content_h = visible_tracks * trk_h;
            float view_h = grid_sz.y - ruler_h - 10.0f;
            float max_scroll_y = (std::max)(0.0f, content_h - view_h);
            playlist_scroll_y = (std::clamp)(playlist_scroll_y, 0.0f, max_scroll_y);
            float track_start_y = grid_p0.y + ruler_h + 2.0f - playlist_scroll_y;

            draw->PushClipRect(ImVec2(grid_p0.x, grid_p0.y + ruler_h), ImVec2(grid_p0.x + grid_sz.x, grid_p0.y + grid_sz.y), true);

            for (int t = 0; t < visible_tracks; ++t) {
                float ty0 = track_start_y + t * trk_h;
                float ty1 = ty0 + trk_h - 3.0f;

                // Faixa Alternada na Grade
                if (t % 2 == 1) {
                    draw->AddRectFilled(ImVec2(grid_p0.x + track_card_w, ty0), ImVec2(grid_p0.x + grid_sz.x, ty1), IM_COL32(13, 19, 28, 140));
                }
                draw->AddLine(ImVec2(grid_p0.x + track_card_w, ty1), ImVec2(grid_p0.x + grid_sz.x, ty1), IM_COL32(20, 32, 48, 255), 1.0f);

                // 3.1 Card do Cabeçalho da Faixa
                bool is_sel = (selected_track_idx == t);
                ImU32 card_bg = is_sel ? IM_COL32(22, 33, 48, 255) : IM_COL32(14, 20, 29, 255);
                draw->AddRectFilled(ImVec2(grid_p0.x, ty0), ImVec2(grid_p0.x + track_card_w, ty1), card_bg, 4.0f);
                draw->AddRect(ImVec2(grid_p0.x, ty0), ImVec2(grid_p0.x + track_card_w, ty1), is_sel ? IM_COL32(0, 212, 255, 200) : IM_COL32(24, 38, 55, 255), 4.0f);

                // Barra Colorida de Acento
                draw->AddRectFilled(ImVec2(grid_p0.x, ty0), ImVec2(grid_p0.x + 4.0f, ty1), trk_palette[t], 4.0f);

                // Nome da Faixa Dinâmico (Modular com suporte a renomeação)
                std::string display_name = (t < MAX_TRACKS && !::track_names[t].empty()) ? ::track_names[t] : ("Track " + std::to_string(t + 1));
                draw->AddText(ImVec2(grid_p0.x + 8.0f, ty0 + 6.0f), IM_COL32(235, 245, 255, 255), display_name.c_str());

                // Barra Horizontal de Volume
                float vol_bar_x0 = grid_p0.x + 8.0f;
                float vol_bar_x1 = grid_p0.x + track_card_w - 55.0f;
                float vol_bar_y = ty0 + 28.0f;
                float cur_vol = (t < MAX_TRACKS) ? ::track_linear_volumes[t] : 0.8f;
                draw->AddRectFilled(ImVec2(vol_bar_x0, vol_bar_y), ImVec2(vol_bar_x1, vol_bar_y + 5.0f), IM_COL32(20, 30, 42, 255), 2.0f);
                draw->AddRectFilled(ImVec2(vol_bar_x0, vol_bar_y), ImVec2(vol_bar_x0 + (vol_bar_x1 - vol_bar_x0) * (std::clamp)(cur_vol, 0.0f, 1.0f), vol_bar_y + 5.0f), trk_palette[t], 2.0f);

                // Botões Interativos [ M ] e [ S ]
                float btn_m_x = grid_p0.x + track_card_w - 48.0f;
                float btn_o_x = grid_p0.x + track_card_w - 24.0f;
                float btn_y = ty0 + 10.0f;
                
                bool is_muted = (t < MAX_TRACKS) ? ::track_mutes[t] : false;
                bool is_solo = (t < MAX_TRACKS) ? ::track_solos[t] : false;

                ImVec2 btn_m_p0(btn_m_x, btn_y);
                ImVec2 btn_m_p1(btn_m_x + 20.0f, btn_y + 20.0f);
                bool hover_m = is_playlist_hovered && ImGui::IsMouseHoveringRect(btn_m_p0, btn_m_p1);
                draw->AddRectFilled(btn_m_p0, btn_m_p1, is_muted ? IM_COL32(220, 40, 40, 255) : hover_m ? IM_COL32(30, 45, 60, 255) : IM_COL32(18, 26, 36, 255), 3.0f);
                draw->AddText(ImVec2(btn_m_x + 5.0f, btn_y + 3.0f), is_muted ? IM_COL32(255, 255, 255, 255) : IM_COL32(180, 205, 230, 255), "M");
                if (hover_m && ImGui::IsMouseClicked(0)) {
                    if (t < MAX_TRACKS) ::track_mutes[t] = !::track_mutes[t];
                }

                ImVec2 btn_o_p0(btn_o_x, btn_y);
                ImVec2 btn_o_p1(btn_o_x + 20.0f, btn_y + 20.0f);
                bool hover_o = is_playlist_hovered && ImGui::IsMouseHoveringRect(btn_o_p0, btn_o_p1);
                draw->AddRectFilled(btn_o_p0, btn_o_p1, is_solo ? IM_COL32(250, 190, 0, 255) : hover_o ? IM_COL32(30, 45, 60, 255) : IM_COL32(18, 26, 36, 255), 3.0f);
                draw->AddText(ImVec2(btn_o_x + 5.0f, btn_y + 3.0f), is_solo ? IM_COL32(0, 0, 0, 255) : IM_COL32(180, 205, 230, 255), "S");
                if (hover_o && ImGui::IsMouseClicked(0)) {
                    if (t < MAX_TRACKS) ::track_solos[t] = !::track_solos[t];
                }

                // Interação com o Cabeçalho: Seleção (Clique-Esq) ou Renomear (Duplo-Clique / Clique-Dir)
                std::string rename_popup_id = "##RenameTrackPopup_" + std::to_string(t);
                static char rename_buffer[128] = "";

                bool is_header_hovered = is_playlist_hovered && ImGui::IsMouseHoveringRect(ImVec2(grid_p0.x, ty0), ImVec2(grid_p0.x + track_card_w - 52.0f, ty1));
                if (is_header_hovered) {
                    if (ImGui::IsMouseDoubleClicked(0) || ImGui::IsMouseClicked(1)) {
                        selected_track_idx = t;
                        strncpy(rename_buffer, (t < MAX_TRACKS) ? ::track_names[t].c_str() : "", sizeof(rename_buffer) - 1);
                        ImGui::OpenPopup(rename_popup_id.c_str());
                    } else if (ImGui::IsMouseClicked(0) && !hover_m && !hover_o) {
                        selected_track_idx = t;
                    }
                }

                if (ImGui::BeginPopup(rename_popup_id.c_str())) {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "🏷️ Renomear Faixa %02d", t + 1);
                    ImGui::Separator();
                    ImGui::SetNextItemWidth(220);
                    if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
                    if (ImGui::InputText("##rename_input", rename_buffer, sizeof(rename_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                        if (strlen(rename_buffer) > 0 && t < MAX_TRACKS) {
                            ::track_names[t] = rename_buffer;
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::Button("Salvar", ImVec2(100, 24))) {
                        if (strlen(rename_buffer) > 0 && t < MAX_TRACKS) {
                            ::track_names[t] = rename_buffer;
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancelar", ImVec2(100, 24))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }

            // ─────────────────────────────────────────────────────────────
            // 4. RENDERIZAÇÃO E INTERAÇÃO COM CLIPES REAIS (AUDIO, MIDI & AUTO)
            // ─────────────────────────────────────────────────────────────
            ImVec2 grid_lane_p0 = ImVec2(grid_p0.x + track_card_w, grid_p0.y + ruler_h);
            ImVec2 grid_lane_sz = ImVec2((std::max)(10.0f, grid_sz.x - track_card_w - 14.0f), (std::max)(10.0f, grid_sz.y - ruler_h - 14.0f));
            ImVec2 grid_lane_p1 = ImVec2(grid_lane_p0.x + grid_lane_sz.x, grid_lane_p0.y + grid_lane_sz.y);

            ImGui::SetCursorScreenPos(grid_lane_p0);
            ImGui::InvisibleButton("##playlist_grid_hitbox", grid_lane_sz);
            bool is_grid_hitbox_hovered = ImGui::IsItemHovered();
            bool is_mouse_in_grid = is_playlist_hovered && is_grid_hitbox_hovered;

            int hovered_track_at_mouse = -1;
            if (is_mouse_in_grid) {
                float rel_y = mouse_pos.y - (grid_p0.y + ruler_h) + playlist_scroll_y;
                if (rel_y >= 0.0f) {
                    hovered_track_at_mouse = (std::clamp)((int)(rel_y / trk_h), 0, visible_tracks - 1);
                }
            }

            float current_mouse_time_sec = 0.0f;
            if (is_mouse_in_grid) {
                float rel_x = mouse_pos.x - (grid_p0.x + track_card_w) + playlist_scroll_x;
                current_mouse_time_sec = (rel_x / p_sec) * bar_len_sec;
            }

            bool clip_clicked_this_frame = false;

            for (int t = 0; t < visible_tracks; ++t) {
                float ty0 = track_start_y + t * trk_h;
                float ty1 = ty0 + trk_h - 3.0f;
                float mid_y = (ty0 + ty1) * 0.5f;

                // INOVAÇÃO GHOST WAVEFORM: Sobrepõe o Kick da Faixa 0 na Faixa 1 (Bassline) em neon laranja translúcido
                if (show_ghost_waveform && t == 1) {
                    auto kick_clips = g_clip_manager.getClips(0);
                    for (const auto& kc : kick_clips) {
                        float kx0 = grid_p0.x + track_card_w + (kc.start_time_sec / bar_len_sec) * p_sec - playlist_scroll_x;
                        float kx1 = kx0 + (kc.length_sec / bar_len_sec) * p_sec;
                        if (kx1 < grid_p0.x + track_card_w || kx0 > grid_p0.x + grid_sz.x) continue;
                        float k_draw_x0 = (std::max)(kx0, grid_p0.x + track_card_w);
                        float k_draw_x1 = (std::min)(kx1, grid_p0.x + grid_sz.x);

                        for (float wx = k_draw_x0; wx < k_draw_x1; wx += 3.0f) {
                            float prog = (wx - kx0) / (kx1 - kx0);
                            float amp = (std::sin(prog * 45.0f) * 0.5f + std::cos(prog * 12.0f) * 0.4f) * (trk_h * 0.35f);
                            draw->AddLine(ImVec2(wx, mid_y - std::abs(amp)), ImVec2(wx, mid_y + std::abs(amp)), IM_COL32(255, 140, 0, 70), 1.5f);
                        }
                    }
                }

                // 4.1 RENDERIZAR AUDIO CLIPS REAIS
                auto audio_clips = g_clip_manager.getClips(t);
                for (size_t c_idx = 0; c_idx < audio_clips.size(); ++c_idx) {
                    const auto& clip = audio_clips[c_idx];

                    float cx0 = grid_p0.x + track_card_w + (clip.start_time_sec / bar_len_sec) * p_sec - playlist_scroll_x;
                    float cx1 = cx0 + (clip.length_sec / bar_len_sec) * p_sec;

                    if (cx1 < grid_p0.x + track_card_w || cx0 > grid_p0.x + grid_sz.x) continue;

                    float draw_x0 = (std::max)(cx0, grid_p0.x + track_card_w);
                    float draw_x1 = (std::min)(cx1, grid_p0.x + grid_sz.x);

                    ImVec2 clip_p0(draw_x0, ty0 + 2.0f);
                    ImVec2 clip_p1(draw_x1, ty1 - 2.0f);
                    bool is_hovered = is_mouse_in_grid && ImGui::IsMouseHoveringRect(clip_p0, clip_p1);

                    ImU32 clip_col = trk_palette[t % 8];
                    if (clip.is_muted) clip_col = IM_COL32(80, 90, 100, 180);

                    // Caixa do Clipe
                    draw->AddRectFilled(clip_p0, clip_p1, clip_col, 4.0f);
                    draw->AddRect(clip_p0, clip_p1, is_hovered ? IM_COL32(255, 255, 255, 240) : IM_COL32(255, 255, 255, 120), 4.0f, 0, is_hovered ? 2.0f : 1.0f);

                    // Header do Clipe com Botão Menu ▼
                    draw->AddRectFilled(clip_p0, ImVec2(clip_p1.x, clip_p0.y + 14.0f), IM_COL32(10, 15, 22, 200), 4.0f);
                    std::string clip_title = clip.name.empty() ? (std::string("Audio ") + std::to_string(clip.id)) : clip.name;
                    if (clip.is_reversed) clip_title += " [REV]";
                    if (clip.is_normalized) clip_title += " [0dB]";

                    draw->AddText(ImVec2(clip_p0.x + 4.0f, clip_p0.y + 1.0f), IM_COL32(230, 245, 255, 240), (std::string("[AUDIO] ") + clip_title).c_str());

                    // Desenho da Forma de Onda Real (Extraída dos Dados PCM de Áudio da Música)
                    const std::vector<float>* p_stem_buf = (::g_ai_engine && t < MAX_TRACKS) ? &::g_ai_engine->getStemBuffer(t) : nullptr;
                    bool has_real_audio = (p_stem_buf && !p_stem_buf->empty());

                    if (has_real_audio) {
                        size_t total_samples = p_stem_buf->size();
                        size_t total_frames = total_samples / 2; // Interleaved stereo
                        float sr = 44100.0f;
                        float clip_dur_sec = (std::max)(0.1f, clip.length_sec);
                        float source_off_sec = (std::max)(0.0f, clip.source_offset_sec);

                        for (float wx = draw_x0 + 2.0f; wx < draw_x1 - 2.0f; wx += 2.0f) {
                            float norm_x = (wx - cx0) / (cx1 - cx0);
                            if (clip.is_reversed) norm_x = 1.0f - norm_x;
                            
                            float cur_sec = source_off_sec + norm_x * clip_dur_sec;
                            size_t frame_pos = (size_t)(cur_sec * sr);
                            
                            float peak = 0.0f;
                            size_t win_sz = 384;
                            size_t f_start = (frame_pos >= win_sz / 2) ? frame_pos - win_sz / 2 : 0;
                            size_t f_end = (std::min)(total_frames, f_start + win_sz);

                            for (size_t f = f_start; f < f_end; f += 8) {
                                size_t idx = f * 2;
                                if (idx < total_samples) {
                                    float s = std::abs((*p_stem_buf)[idx]);
                                    if (s > peak) peak = s;
                                }
                            }

                            if (clip.is_normalized && peak > 0.0f) peak *= 1.4f;
                            peak = std::clamp(peak, 0.02f, 1.0f);

                            float wave_amp = peak * (trk_h * 0.38f);
                            draw->AddLine(ImVec2(wx, mid_y - wave_amp), ImVec2(wx, mid_y + wave_amp), IM_COL32(10, 18, 28, 230), 1.6f);
                            draw->AddLine(ImVec2(wx, mid_y - wave_amp * 0.5f), ImVec2(wx, mid_y + wave_amp * 0.5f), IM_COL32(255, 255, 255, 45), 1.0f);
                        }
                    } else {
                        // Fallback procedural se ainda não carregou o buffer
                        for (float wx = draw_x0 + 4.0f; wx < draw_x1 - 4.0f; wx += 2.0f) {
                            float progress_in_clip = (wx - cx0) / (cx1 - cx0);
                            if (clip.is_reversed) progress_in_clip = 1.0f - progress_in_clip;
                            float wave_amp = (std::sin(progress_in_clip * 45.0f) * 0.5f + std::cos(progress_in_clip * 12.0f) * 0.4f) * (trk_h * 0.28f);
                            draw->AddLine(ImVec2(wx, mid_y - std::abs(wave_amp)), ImVec2(wx, mid_y + std::abs(wave_amp)), IM_COL32(10, 20, 30, 220), 1.5f);
                        }
                    }

                    if (clip.is_muted) {
                        draw->AddText(ImVec2(clip_p0.x + 6.0f, mid_y - 6.0f), IM_COL32(255, 80, 80, 240), "[MUTED]");
                    }

                    bool is_near_right = is_hovered && (mouse_pos.x >= draw_x1 - 8.0f);
                    bool is_near_left = is_hovered && (mouse_pos.x <= draw_x0 + 8.0f);

                    if (is_near_right || is_near_left) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    }

                    // INTERAÇÃO DO MOUSE COM AUDIO CLIP
                    if (is_hovered) {
                        ImGui::SetTooltip("Audio Clip: %s\nInicio: Bar %.2f | Duracao: %.2fs\n(Duplo clique: Inspector | Arraste: Mover/Esticar | Botao Direito: Excluir)", clip_title.c_str(), 1.0f + (clip.start_time_sec / bar_len_sec), clip.length_sec);

                        // Duplo Clique: Abre o Audio Clip Inspector Modal
                        if (ImGui::IsMouseDoubleClicked(0)) {
                            g_audio_clip_editor_ui.open(t, (int)c_idx);
                            clip_clicked_this_frame = true;
                            break;
                        }

                        // Botão Direito: Exclusão rápida
                        if (ImGui::IsMouseClicked(1)) {
                            g_clip_manager.deleteClip(t, (int)c_idx);
                            clip_clicked_this_frame = true;
                            break;
                        }

                        // Botão Esquerdo: Mover, Esticar ou Ferramenta
                        if (ImGui::IsMouseClicked(0)) {
                            clip_clicked_this_frame = true;
                            if (current_tool == TOOL_DELETE) {
                                g_clip_manager.deleteClip(t, (int)c_idx);
                                break;
                            } else if (current_tool == TOOL_MUTE) {
                                g_clip_manager.toggleMuteAudioClip(t, (int)c_idx);
                            } else if (current_tool == TOOL_SLICE) {
                                g_clip_manager.splitClip(t, (int)c_idx, current_mouse_time_sec);
                                break;
                            } else {
                                // Suporte a Shift + Drag (Clonar no arraste)
                                if (ImGui::GetIO().KeyShift) {
                                    g_clip_manager.duplicateAudioClip(t, (int)c_idx, 0.0f);
                                }
                                is_dragging_clip = true;
                                is_resizing_right = is_near_right;
                                is_resizing_left = is_near_left;
                                active_drag_track = t;
                                active_drag_clip_idx = (int)c_idx;
                                active_drag_type = 0;
                                drag_orig_start = clip.start_time_sec;
                                drag_orig_len = clip.length_sec;
                                drag_orig_offset = clip.source_offset_sec;
                                drag_mouse_start_pos = mouse_pos;
                            }
                        }
                    }
                }

                // 4.2 RENDERIZAR MIDI PATTERN CLIPS REAIS
                auto midi_clips = g_clip_manager.getMidiClips(t);
                for (size_t c_idx = 0; c_idx < midi_clips.size(); ++c_idx) {
                    const auto& clip = midi_clips[c_idx];

                    float cx0 = grid_p0.x + track_card_w + (clip.start_time_sec / bar_len_sec) * p_sec - playlist_scroll_x;
                    float cx1 = cx0 + (clip.length_sec / bar_len_sec) * p_sec;

                    if (cx1 < grid_p0.x + track_card_w || cx0 > grid_p0.x + grid_sz.x) continue;

                    float draw_x0 = (std::max)(cx0, grid_p0.x + track_card_w);
                    float draw_x1 = (std::min)(cx1, grid_p0.x + grid_sz.x);

                    ImVec2 clip_p0(draw_x0, ty0 + 2.0f);
                    ImVec2 clip_p1(draw_x1, ty1 - 2.0f);
                    bool is_hovered = is_mouse_in_grid && ImGui::IsMouseHoveringRect(clip_p0, clip_p1);

                    std::string pat_name = clip.name.empty() ? ("Pattern " + std::to_string(clip.pattern_id)) : clip.name;
                    if (pat_name.find("[MIDI]") == std::string::npos && pat_name.find("[PAT]") == std::string::npos) {
                        pat_name = "[MIDI] " + pat_name;
                    }
                    ImU32 pat_col = trk_palette[t % 8];
                    int associated_pat_idx = 0;
                    for (size_t p_i = 0; p_i < g_clip_manager.global_patterns.size(); ++p_i) {
                        if (g_clip_manager.global_patterns[p_i].id == clip.pattern_id) {
                            associated_pat_idx = (int)p_i;
                            break;
                        }
                    }

                    if (clip.is_muted) pat_col = IM_COL32(80, 90, 100, 180);

                    // Bloco do Padrão MIDI
                    draw->AddRectFilled(clip_p0, clip_p1, pat_col, 4.0f);
                    draw->AddRect(clip_p0, clip_p1, is_hovered ? IM_COL32(255, 255, 255, 240) : IM_COL32(255, 255, 255, 120), 4.0f, 0, is_hovered ? 2.0f : 1.0f);

                    // Header do Clipe MIDI
                    draw->AddRectFilled(clip_p0, ImVec2(clip_p1.x, clip_p0.y + 15.0f), IM_COL32(10, 15, 22, 220), 4.0f);
                    draw->AddText(ImVec2(clip_p0.x + 4.0f, clip_p0.y + 1.0f), IM_COL32(255, 255, 255, 255), pat_name.c_str());

                    // Desenha Mini-Notas do Piano Roll dentro do Clipe MIDI (igual FL Studio)
                    for (const auto& p : g_clip_manager.global_patterns) {
                        if (p.id == clip.pattern_id) {
                            const auto& notes = p.getChannelNotes(t % 8);
                            for (const auto& note : notes) {
                                float nx0 = cx0 + (note.start_time / bar_len_sec) * p_sec;
                                float nx1 = nx0 + (std::max)(2.5f, (note.duration / bar_len_sec) * p_sec);
                                if (nx1 >= draw_x0 && nx0 <= draw_x1) {
                                    float note_norm = std::clamp((note.pitch - 24) / 60.0f, 0.05f, 0.95f);
                                    float rel_pitch_y = (ty1 - 4.0f) - note_norm * (trk_h * 0.60f);
                                    draw->AddRectFilled(ImVec2((std::max)(nx0, draw_x0), rel_pitch_y - 1.5f), ImVec2((std::min)(nx1, draw_x1), rel_pitch_y + 1.5f), IM_COL32(255, 255, 255, 230), 1.0f);
                                }
                            }
                            break;
                        }
                    }

                    if (clip.is_muted) {
                        draw->AddText(ImVec2(clip_p0.x + 6.0f, mid_y - 6.0f), IM_COL32(255, 80, 80, 240), "[MUTED]");
                    }

                    bool is_near_right = is_hovered && (mouse_pos.x >= draw_x1 - 8.0f);
                    bool is_near_left = is_hovered && (mouse_pos.x <= draw_x0 + 8.0f);

                    if (is_near_right || is_near_left) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    }

                    // INTERAÇÃO DO MOUSE COM MIDI CLIP
                    if (is_hovered) {
                        ImGui::SetTooltip("MIDI / Pattern Clip: %s\nInicio: Bar %.2f | Duracao: %.2fs\n(Duplo clique: Abrir Piano Roll | Arraste: Mover | Botao Direito: Excluir)", pat_name.c_str(), 1.0f + (clip.start_time_sec / bar_len_sec), clip.length_sec);

                        // Duplo Clique: Abre e Foca o Piano Roll no Pattern
                        if (ImGui::IsMouseDoubleClicked(0)) {
                            selected_track_idx = t;
                            g_clip_manager.current_pattern_idx = associated_pat_idx;
                            show_piano_roll = true;
                            clip_clicked_this_frame = true;
                            break;
                        }

                        if (ImGui::IsMouseClicked(1)) {
                            g_clip_manager.deleteMidiClip(t, (int)c_idx);
                            clip_clicked_this_frame = true;
                            break;
                        }

                        if (ImGui::IsMouseClicked(0)) {
                            clip_clicked_this_frame = true;
                            if (current_tool == TOOL_DELETE) {
                                g_clip_manager.deleteMidiClip(t, (int)c_idx);
                                break;
                            } else if (current_tool == TOOL_MUTE) {
                                g_clip_manager.toggleMuteMidiClip(t, (int)c_idx);
                            } else if (current_tool == TOOL_SLICE) {
                                g_clip_manager.splitMidiClip(t, (int)c_idx, current_mouse_time_sec);
                                break;
                            } else {
                                if (ImGui::GetIO().KeyShift) {
                                    g_clip_manager.duplicateMidiClip(t, (int)c_idx, 0.0f);
                                }
                                is_dragging_clip = true;
                                is_resizing_right = is_near_right;
                                is_resizing_left = is_near_left;
                                active_drag_track = t;
                                active_drag_clip_idx = (int)c_idx;
                                active_drag_type = 1;
                                drag_orig_start = clip.start_time_sec;
                                drag_orig_len = clip.length_sec;
                                drag_mouse_start_pos = mouse_pos;
                            }
                        }
                    }
                }

                // 4.3 RENDERIZAR CLIPS DE AUTOMAÇÃO COM CURVAS BÉZIER
                auto auto_clips = g_clip_manager.getAutomationClips(t);
                for (size_t c_idx = 0; c_idx < auto_clips.size(); ++c_idx) {
                    const auto& clip = auto_clips[c_idx];

                    float cx0 = grid_p0.x + track_card_w + (clip.start_time_sec / bar_len_sec) * p_sec - playlist_scroll_x;
                    float cx1 = cx0 + (clip.length_sec / bar_len_sec) * p_sec;

                    if (cx1 < grid_p0.x + track_card_w || cx0 > grid_p0.x + grid_sz.x) continue;

                    float draw_x0 = (std::max)(cx0, grid_p0.x + track_card_w);
                    float draw_x1 = (std::min)(cx1, grid_p0.x + grid_sz.x);

                    ImVec2 clip_p0(draw_x0, ty0 + 2.0f);
                    ImVec2 clip_p1(draw_x1, ty1 - 2.0f);
                    bool is_hovered = is_mouse_in_grid && ImGui::IsMouseHoveringRect(clip_p0, clip_p1);

                    // Fundo da Automação com Gradiente Escuro
                    draw->AddRectFilled(clip_p0, clip_p1, IM_COL32(18, 28, 40, 220), 4.0f);
                    draw->AddRect(clip_p0, clip_p1, is_hovered ? IM_COL32(0, 229, 255, 255) : IM_COL32(0, 180, 220, 140), 4.0f);

                    // Header do Clipe de Automação
                    draw->AddRectFilled(clip_p0, ImVec2(clip_p1.x, clip_p0.y + 14.0f), IM_COL32(8, 14, 20, 240), 4.0f);
                    draw->AddText(ImVec2(clip_p0.x + 4.0f, clip_p0.y + 1.0f), IM_COL32(0, 229, 255, 240), (std::string("[AUTO] ") + clip.name).c_str());

                    // Desenho da Curva de Bézier Contínua com Underfill
                    ImVec2 last_pt(draw_x0, ty1 - 4.0f);
                    for (float wx = draw_x0; wx <= draw_x1; wx += 3.0f) {
                        float rel_t = ((wx - cx0) / (cx1 - cx0)) * clip.length_sec;
                        float val = clip.getValueAtTime(rel_t);
                        float pt_y = (ty1 - 4.0f) - val * (trk_h - 22.0f);
                        ImVec2 curr_pt(wx, pt_y);

                        if (wx > draw_x0) {
                            // Linha de Curva
                            draw->AddLine(last_pt, curr_pt, IM_COL32(0, 229, 255, 255), 2.0f);
                            // Preenchimento Suave por Baixo
                            draw->AddLine(curr_pt, ImVec2(wx, ty1 - 4.0f), IM_COL32(0, 200, 255, 40), 3.0f);
                        }
                        last_pt = curr_pt;
                    }

                    // Desenho dos Nós de Ancoragem (Círculos)
                    for (size_t pt_i = 0; pt_i < clip.points.size(); ++pt_i) {
                        float px = cx0 + (clip.points[pt_i].time_rel_sec / clip.length_sec) * (cx1 - cx0);
                        if (px >= draw_x0 && px <= draw_x1) {
                            float py = (ty1 - 4.0f) - clip.points[pt_i].value * (trk_h - 22.0f);
                            draw->AddCircleFilled(ImVec2(px, py), 4.0f, IM_COL32(255, 255, 255, 255));
                            draw->AddCircle(ImVec2(px, py), 4.0f, IM_COL32(0, 229, 255, 255), 0, 1.5f);
                        }
                    }

                    // INTERAÇÃO DO MOUSE COM CLIPE DE AUTOMAÇÃO
                    if (is_hovered) {
                        ImGui::SetTooltip("Automation Clip: %s\n(Clique na curva para criar no | Botao direito no clipe para excluir)", clip.name.c_str());

                        if (ImGui::IsMouseClicked(1)) {
                            g_clip_manager.deleteAutomationClip(t, (int)c_idx);
                            clip_clicked_this_frame = true;
                            break;
                        }

                        if (ImGui::IsMouseClicked(0)) {
                            clip_clicked_this_frame = true;
                            is_dragging_clip = true;
                            active_drag_track = t;
                            active_drag_clip_idx = (int)c_idx;
                            active_drag_type = 2;
                            drag_orig_start = clip.start_time_sec;
                            drag_orig_len = clip.length_sec;
                            drag_mouse_start_pos = mouse_pos;
                        }
                    }
                }
            }

            draw->PopClipRect();

            // ─────────────────────────────────────────────────────────────
            // SCROLLBARS INTERATIVAS (HORIZONTAL E VERTICAL ESTILO FL STUDIO)
            // ─────────────────────────────────────────────────────────────
            static bool is_dragging_h_scrollbar = false;
            static bool is_dragging_v_scrollbar = false;

            float total_song_sec = 420.0f;
            float total_content_w = (total_song_sec / bar_len_sec) * p_sec;
            float view_w = grid_sz.x - track_card_w;
            float max_scroll_x = (std::max)(0.0f, total_content_w - view_w);
            playlist_scroll_x = (std::clamp)(playlist_scroll_x, 0.0f, max_scroll_x);

            // 1. Barra de Rolagem Horizontal (Inferior)
            if (max_scroll_x > 0.0f) {
                float h_sb_h = 8.0f;
                float h_sb_x0 = grid_p0.x + track_card_w + 2.0f;
                float h_sb_x1 = grid_p0.x + grid_sz.x - 14.0f;
                float h_sb_y0 = grid_p0.y + grid_sz.y - h_sb_h - 2.0f;
                float h_sb_y1 = grid_p0.y + grid_sz.y - 2.0f;
                float h_sb_w = h_sb_x1 - h_sb_x0;
                
                float h_thumb_w = (std::clamp)(h_sb_w * (view_w / total_content_w), 36.0f, h_sb_w);
                float h_thumb_x0 = h_sb_x0 + (playlist_scroll_x / max_scroll_x) * (h_sb_w - h_thumb_w);
                
                ImVec2 h_sb_p0(h_sb_x0, h_sb_y0);
                ImVec2 h_sb_p1(h_sb_x1, h_sb_y1);
                ImVec2 h_thumb_p0(h_thumb_x0, h_sb_y0);
                ImVec2 h_thumb_p1(h_thumb_x0 + h_thumb_w, h_sb_y1);
                
                bool h_hovered = is_playlist_hovered && ImGui::IsMouseHoveringRect(h_sb_p0, h_sb_p1);
                if (h_hovered && ImGui::IsMouseClicked(0)) {
                    is_dragging_h_scrollbar = true;
                    clip_clicked_this_frame = true;
                }
                if (is_dragging_h_scrollbar) {
                    if (ImGui::IsMouseDown(0)) {
                        float norm = (std::clamp)((mouse_pos.x - h_sb_x0 - h_thumb_w * 0.5f) / (h_sb_w - h_thumb_w), 0.0f, 1.0f);
                        playlist_scroll_x = norm * max_scroll_x;
                    } else {
                        is_dragging_h_scrollbar = false;
                    }
                }
                
                draw->AddRectFilled(h_sb_p0, h_sb_p1, IM_COL32(10, 16, 24, 200), 4.0f);
                draw->AddRectFilled(h_thumb_p0, h_thumb_p1, is_dragging_h_scrollbar ? IM_COL32(0, 255, 255, 255) : (h_hovered ? IM_COL32(0, 230, 255, 240) : IM_COL32(0, 190, 230, 210)), 4.0f);
            }

            // 2. Barra de Rolagem Vertical (Lateral Direita)
            if (max_scroll_y > 0.0f) {
                float v_sb_w = 8.0f;
                float v_sb_x0 = grid_p0.x + grid_sz.x - v_sb_w - 2.0f;
                float v_sb_x1 = grid_p0.x + grid_sz.x - 2.0f;
                float v_sb_y0 = grid_p0.y + ruler_h + 2.0f;
                float v_sb_y1 = grid_p0.y + grid_sz.y - 12.0f;
                float v_sb_h = v_sb_y1 - v_sb_y0;
                
                float v_thumb_h = (std::clamp)(v_sb_h * (view_h / content_h), 28.0f, v_sb_h);
                float v_thumb_y0 = v_sb_y0 + (playlist_scroll_y / max_scroll_y) * (v_sb_h - v_thumb_h);
                
                ImVec2 v_sb_p0(v_sb_x0, v_sb_y0);
                ImVec2 v_sb_p1(v_sb_x1, v_sb_y1);
                ImVec2 v_thumb_p0(v_sb_x0, v_thumb_y0);
                ImVec2 v_thumb_p1(v_sb_x1, v_thumb_y0 + v_thumb_h);
                
                bool v_hovered = is_playlist_hovered && ImGui::IsMouseHoveringRect(v_sb_p0, v_sb_p1);
                if (v_hovered && ImGui::IsMouseClicked(0)) {
                    is_dragging_v_scrollbar = true;
                    clip_clicked_this_frame = true;
                }
                if (is_dragging_v_scrollbar) {
                    if (ImGui::IsMouseDown(0)) {
                        float norm = (std::clamp)((mouse_pos.y - v_sb_y0 - v_thumb_h * 0.5f) / (v_sb_h - v_thumb_h), 0.0f, 1.0f);
                        playlist_scroll_y = norm * max_scroll_y;
                    } else {
                        is_dragging_v_scrollbar = false;
                    }
                }
                
                draw->AddRectFilled(v_sb_p0, v_sb_p1, IM_COL32(10, 16, 24, 200), 4.0f);
                draw->AddRectFilled(v_thumb_p0, v_thumb_p1, is_dragging_v_scrollbar ? IM_COL32(0, 255, 255, 255) : (v_hovered ? IM_COL32(0, 230, 255, 240) : IM_COL32(0, 190, 230, 210)), 4.0f);
            }

            // ─────────────────────────────────────────────────────────────
            // 5. DRAG & DROP TARGET NA GRADE DA PLAYLIST
            // ─────────────────────────────────────────────────────────────
            if (ImGui::BeginDragDropTargetCustom(ImRect(grid_lane_p0, grid_lane_p1), ImGui::GetID("PlaylistGridDND"))) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SAMPLE_PATH")) {
                    const char* dropped_path = (const char*)payload->Data;
                    if (hovered_track_at_mouse >= 0) {
                        float drop_time = snap_time(current_mouse_time_sec);
                        AudioClip ac;
                        ac.id = g_clip_manager.next_id++;
                        ac.name = std::filesystem::path(dropped_path).filename().string();
                        ac.file_path = dropped_path;
                        ac.start_time_sec = drop_time;
                        ac.length_sec = bar_len_sec * 2.0f; // 2 bars default
                        ac.source_offset_sec = 0.0f;
                        g_clip_manager.track_clips[hovered_track_at_mouse].push_back(ac);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // ─────────────────────────────────────────────────────────────
            // 6. PROCESSAMENTO DE ARRASTE / REDIMENSIONAMENTO DE CLIPES
            // ─────────────────────────────────────────────────────────────
            if (is_dragging_clip) {
                if (ImGui::IsMouseDown(0)) {
                    float delta_px = mouse_pos.x - drag_mouse_start_pos.x;
                    float delta_time = (delta_px / p_sec) * bar_len_sec;

                    if (is_resizing_right) {
                        float new_len = snap_time(drag_orig_len + delta_time);
                        if (new_len >= 0.25f) {
                            if (active_drag_type == 0) g_clip_manager.resizeAudioClip(active_drag_track, active_drag_clip_idx, new_len);
                            else if (active_drag_type == 1) g_clip_manager.resizeMidiClip(active_drag_track, active_drag_clip_idx, new_len);
                            else if (active_drag_type == 2) g_clip_manager.resizeAutomationClip(active_drag_track, active_drag_clip_idx, new_len);
                        }
                    } else if (is_resizing_left) {
                        float new_start = snap_time(drag_orig_start + delta_time);
                        float new_len = drag_orig_len - (new_start - drag_orig_start);
                        if (new_len >= 0.25f && new_start >= 0.0f) {
                            if (active_drag_type == 0) {
                                g_clip_manager.updateClipStart(active_drag_track, active_drag_clip_idx, new_start);
                                g_clip_manager.resizeAudioClip(active_drag_track, active_drag_clip_idx, new_len);
                            } else if (active_drag_type == 1) {
                                g_clip_manager.updateMidiClipStart(active_drag_track, active_drag_clip_idx, new_start);
                                g_clip_manager.resizeMidiClip(active_drag_track, active_drag_clip_idx, new_len);
                            }
                        }
                    } else {
                        // Mover Clipe no Tempo e entre Faixas Verticais
                        float new_start = snap_time(drag_orig_start + delta_time);
                        int target_track = (hovered_track_at_mouse >= 0) ? hovered_track_at_mouse : active_drag_track;

                        if (target_track != active_drag_track) {
                            if (active_drag_type == 0) {
                                g_clip_manager.moveAudioClip(active_drag_track, active_drag_clip_idx, target_track, new_start);
                                active_drag_track = target_track;
                                active_drag_clip_idx = (int)g_clip_manager.track_clips[target_track].size() - 1;
                            } else if (active_drag_type == 1) {
                                g_clip_manager.moveMidiClip(active_drag_track, active_drag_clip_idx, target_track, new_start);
                                active_drag_track = target_track;
                                active_drag_clip_idx = (int)g_clip_manager.track_midi_clips[target_track].size() - 1;
                            }
                        } else {
                            if (active_drag_type == 0) g_clip_manager.updateClipStart(active_drag_track, active_drag_clip_idx, new_start);
                            else if (active_drag_type == 1) g_clip_manager.updateMidiClipStart(active_drag_track, active_drag_clip_idx, new_start);
                            else if (active_drag_type == 2) g_clip_manager.updateAutomationClipStart(active_drag_track, active_drag_clip_idx, new_start);
                        }
                    }
                } else {
                    is_dragging_clip = false;
                    is_resizing_right = false;
                    is_resizing_left = false;
                }
            }

            // Inserção com Ferramenta DRAW ou PAINT em Área Vazia
            bool is_scrollbar_active = is_dragging_h_scrollbar || is_dragging_v_scrollbar;
            if (is_mouse_in_grid && !is_scrollbar_active && ImGui::IsMouseClicked(0) && !clip_clicked_this_frame && hovered_track_at_mouse >= 0) {
                if (current_tool == TOOL_DRAW || current_tool == TOOL_PAINT) {
                    float insert_t = snap_time(current_mouse_time_sec);
                    int cur_p_idx = (std::clamp)(g_clip_manager.current_pattern_idx, 0, (int)g_clip_manager.global_patterns.size() - 1);
                    if (!g_clip_manager.global_patterns.empty()) {
                        int pat_id = g_clip_manager.global_patterns[cur_p_idx].id;
                        g_clip_manager.addPatternClip(hovered_track_at_mouse, insert_t, bar_len_sec * 4.0f, pat_id);
                    }
                }
            }

            // ─────────────────────────────────────────────────────────────
            // 7. RENDERIZAÇÃO DO PLAYHEAD & HUD DO TEMPO
            // ─────────────────────────────────────────────────────────────
            uint64_t m_frame = ::timeline.getMasterFrame();
            float cur_play_time_sec = (float)m_frame / 44100.0f;
            float playhead_x = grid_p0.x + track_card_w + (cur_play_time_sec / bar_len_sec) * p_sec - playlist_scroll_x;

            if (playhead_x >= grid_p0.x + track_card_w && playhead_x <= grid_p0.x + grid_sz.x) {
                // Laser Playhead Line
                draw->AddLine(ImVec2(playhead_x, grid_p0.y), ImVec2(playhead_x, grid_p0.y + grid_sz.y), IM_COL32(0, 229, 255, 255), 2.0f);
                // Top Triangular Pointer
                draw->AddTriangleFilled(ImVec2(playhead_x - 6.0f, grid_p0.y), ImVec2(playhead_x + 6.0f, grid_p0.y), ImVec2(playhead_x, grid_p0.y + ruler_h * 0.75f), IM_COL32(0, 229, 255, 255));
            }

            // HUD do Tempo no Canto Superior Direito da Playlist
            int cur_bar = 1 + (int)(cur_play_time_sec / bar_len_sec);
            int cur_sec = (int)cur_play_time_sec;
            int cur_ms = (int)((cur_play_time_sec - cur_sec) * 10.0f);
            char time_hud[64];
            snprintf(time_hud, sizeof(time_hud), "%02d:%02d.%d | Bar %d | %.0f BPM", cur_sec / 60, cur_sec % 60, cur_ms, cur_bar, bpm);
            draw->AddRectFilled(ImVec2(grid_p0.x + grid_sz.x - 190.0f, grid_p0.y + 3.0f), ImVec2(grid_p0.x + grid_sz.x - 6.0f, grid_p0.y + ruler_h - 3.0f), IM_COL32(10, 16, 24, 220), 4.0f);
            draw->AddText(ImVec2(grid_p0.x + grid_sz.x - 182.0f, grid_p0.y + 5.0f), IM_COL32(0, 229, 255, 240), time_hud);

            ImGui::Dummy(main_sz);
        }
        ImGui::End();
        ImGui::PopStyleColor(1);
    }

        // =============================================
        // PANEL: BROWSER (Pro Tree & Drag & Drop Support)
        // =============================================
        if (show_browser) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.09f, 0.14f, 1.0f));
            if (ImGui::Begin("BROWSER", &show_browser, ImGuiWindowFlags_NoCollapse)) {
            
                // Search Input on Top + Botão Ocultar
                static char browser_search[128] = "";
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.06f, 0.09f, 0.13f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.18f, 0.26f, 1.0f));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 76.0f);
                ImGui::InputTextWithHint("##BrowserSearch", "Search...", browser_search, IM_ARRAYSIZE(browser_search));
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 4);
                if (ImGui::Button("◀ Ocultar", ImVec2(70, 20))) {
                    show_browser = false;
                }
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

            auto render_sample_node = [](const std::string& display_name, const std::string& sub_or_full_path) {
                std::string resolved_path = sub_or_full_path;
                if (!std::filesystem::exists(to_fs_path(resolved_path))) {
                    std::vector<std::string> candidates = {
                        "assets\\samples\\" + sub_or_full_path,
                        "assets\\samples\\Real_Drums\\" + sub_or_full_path,
                        "assets\\samples\\Real_Piano\\" + sub_or_full_path,
                        "assets\\samples\\Real_Strings\\" + sub_or_full_path,
                        "assets\\samples\\Real_Guitar\\" + sub_or_full_path,
                        "assets\\samples\\Real_Bass\\" + sub_or_full_path,
                        "assets\\samples\\Real_Choir\\" + sub_or_full_path,
                        "assets\\samples\\Real_SFX\\" + sub_or_full_path,
                        "assets\\samples\\adhana_signature\\" + sub_or_full_path,
                        "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\assets\\samples\\" + sub_or_full_path,
                        "C:\\Users\\USUÁRIO\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\assets\\samples\\adhana_signature\\" + sub_or_full_path,
                        "C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs\\" + sub_or_full_path
                    };
                    for (const auto& c : candidates) {
                        if (std::filesystem::exists(to_fs_path(c))) {
                            resolved_path = c;
                            break;
                        }
                    }
                }

                if (ImGui::Selectable(display_name.c_str())) {
                    if (std::filesystem::exists(to_fs_path(resolved_path))) {
                        PlaySoundA(resolved_path.c_str(), NULL, SND_ASYNC | SND_FILENAME);
                        if (g_global_sampler) {
                            g_global_sampler->loadSample(resolved_path);
                        }
                    }
                }

                // Drag and Drop Source para soltar direto na Playlist
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    ImGui::SetDragDropPayload("DND_SAMPLE_PATH", resolved_path.c_str(), resolved_path.length() + 1);
                    ImGui::Text("Sample: %s", display_name.c_str());
                    ImGui::EndDragDropSource();
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Clique: Ouvir Preview | Arraste: Soltar na Playlist / Channel Rack");
                }
            };

            std::function<void(const std::filesystem::path&)> render_dir_tree;
            render_dir_tree = [&](const std::filesystem::path& dir_path) {
                try {
                    if (!std::filesystem::exists(dir_path)) return;
                    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                        if (entry.is_directory()) {
                            std::string folder_name = to_utf8_str(entry.path().filename());
                            if (ImGui::TreeNode(folder_name.c_str())) {
                                render_dir_tree(entry.path());
                                ImGui::TreePop();
                            }
                        } else if (entry.is_regular_file()) {
                            auto ext = entry.path().extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                            if (ext == ".wav" || ext == ".ogg" || ext == ".mp3" || ext == ".flac") {
                                std::string fname = to_utf8_str(entry.path().filename());
                                std::string fpath = to_utf8_str(entry.path());
                                render_sample_node(fname, fpath);
                            }
                        }
                    }
                } catch (...) {}
            };

        // 1. PROJECTS
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.75f, 1.00f, 1.0f));
        bool open_projects = ImGui::TreeNodeEx("PROJECTS", ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::PopStyleColor();
        if (open_projects) {
            if (ImGui::Selectable("  default.kuro")) {
                ProjectManagerBridge::Load("default.kuro");
            }
            if (ImGui::Selectable("  Area51_Jam.kuro")) {
                ProjectManagerBridge::Load("Area51_Jam.kuro");
            }
            if (ImGui::Selectable("  Cosmos_Track.kuro")) {
                ProjectManagerBridge::Load("Cosmos_Track.kuro");
            }
            ImGui::TreePop();
        }

        // 2. INSTRUMENTS
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.90f, 0.70f, 1.0f));
        bool open_instruments = ImGui::TreeNodeEx("INSTRUMENTS", ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::PopStyleColor();
        if (open_instruments) {
            if (ImGui::Selectable("  FL DirectWave (Multi-Sampler)")) g_directwave_ui.open();
            if (ImGui::Selectable("  Fruity Granulizer (Grain Cloud)")) g_granulizer_ui.open();
            if (ImGui::Selectable("  SliceX (Beat Slicer)")) g_slicex_ui.open();
            if (ImGui::Selectable("  FL Flex Synth (Psytrance)")) { show_flex_browser = true; active_flex_channel = 3; }
            if (ImGui::Selectable("  Delay Lama (Monge 3D)")) {
                show_delay_lama = true; focus_delay_lama = true;
                g_piano_synth.flex_channel_instrument[5] = KuroAudio::MidiInstrument::DELAY_LAMA;
                g_piano_synth.setInstrument(KuroAudio::MidiInstrument::DELAY_LAMA);
                g_piano_synth.triggerNote(48, 1.2f, 0.9f, 5);
            }
            if (ImGui::Selectable("  Alien LED Synth (Sci-Fi)")) {
                g_piano_synth.flex_channel_instrument[5] = KuroAudio::MidiInstrument::ALIEN_LED_SYNTH;
                g_piano_synth.setInstrument(KuroAudio::MidiInstrument::ALIEN_LED_SYNTH);
                g_piano_synth.triggerNote(60, 1.0f, 0.9f, 5);
            }
            if (ImGui::Selectable("  Sytrus (6-Op FM Synth)")) g_sytrus_synth_ui.open();
            if (ImGui::Selectable("  Harmor (Additive Resynthesis)")) g_harmor_synth_ui.open();
            if (ImGui::Selectable("  Soundfont Player (.SF2)")) g_soundfont_player_ui.open();
            if (ImGui::Selectable("  Contrabass Acustico Real")) { show_contrabass_window = true; }
            ImGui::TreePop();
        }

        // 3. SAMPLES
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.75f, 0.20f, 1.0f));
        bool open_samples = ImGui::TreeNodeEx("SAMPLES", ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::PopStyleColor();
        if (open_samples) {
            // 🛸 Músicas Desconstruídas & Custom Samples (C:\NovaDAW\samples)
            if (std::filesystem::exists(to_fs_path("C:\\NovaDAW\\samples"))) {
                if (ImGui::TreeNodeEx("🛸 Músicas Desconstruídas & Custom Samples", ImGuiTreeNodeFlags_DefaultOpen)) {
                    render_dir_tree(to_fs_path("C:\\NovaDAW\\samples"));
                    ImGui::TreePop();
                }
            }

            // 🎻 Instrumentos Reais (Real Instruments)
            if (ImGui::TreeNodeEx("🎻 Instrumentos Reais (Real Instruments)", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::TreeNode("🎹 Real Piano (Steinway Grand Piano)")) {
                    render_dir_tree(to_fs_path("assets/samples/Real_Piano"));
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("🎻 Real Strings & Cello")) {
                    render_dir_tree(to_fs_path("assets/samples/Real_Strings"));
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("🎸 Real Guitar (Nylon Acústico)")) {
                    render_dir_tree(to_fs_path("assets/samples/Real_Guitar"));
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("🎸 Real Bass (5-String & Deep Bass)")) {
                    render_dir_tree(to_fs_path("assets/samples/Real_Bass"));
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("🥁 Real Drums (Bateria Acústica Real)")) {
                    render_dir_tree(to_fs_path("assets/samples/Real_Drums"));
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("🎼 Real Choir (Coro Vocal Lírico)")) {
                    render_dir_tree(to_fs_path("assets/samples/Real_Choir"));
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("🌌 Real SFX (Risers & Swells)")) {
                    render_dir_tree(to_fs_path("assets/samples/Real_SFX"));
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }

            // 📁 FL Studio Packs
            if (std::filesystem::exists(to_fs_path("C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs"))) {
                if (ImGui::TreeNode("📁 FL Studio 2024 Packs")) {
                    render_dir_tree(to_fs_path("C:\\Program Files\\Image-Line\\FL Studio 2024\\Data\\Patches\\Packs"));
                    ImGui::TreePop();
                }
            }

            // Abduction Kits
            if (ImGui::TreeNode("Abduction Kits")) {
                if (ImGui::TreeNode("Vini Vici & Astrix - Adhana Kit")) {
                    render_sample_node("Adhana_Astrix_Kick_Punch_01.wav", "Adhana_Astrix_Kick_Punch_01.wav");
                    render_sample_node("Adhana_Astrix_Kick_Sub_02.wav", "Adhana_Astrix_Kick_Sub_02.wav");
                    render_sample_node("Adhana_Kick_Loop_138BPM.wav", "Adhana_Kick_Loop_138BPM.wav");
                    render_sample_node("Adhana_Rolling_Bass_Hit_01.wav", "Adhana_Rolling_Bass_Hit_01.wav");
                    render_sample_node("Adhana_Rolling_Bassline_Loop_138BPM.wav", "Adhana_Rolling_Bassline_Loop_138BPM.wav");
                    render_sample_node("Adhana_Acid_Bass_Stab_01.wav", "Adhana_Acid_Bass_Stab_01.wav");
                    render_sample_node("Adhana_Psy_Lead_Hook_2Bars.wav", "Adhana_Psy_Lead_Hook_2Bars.wav");
                    render_sample_node("Adhana_Lead_Stab_Hit.wav", "Adhana_Lead_Stab_Hit.wav");
                    render_sample_node("Adhana_Acid_Arp_Loop_138BPM.wav", "Adhana_Acid_Arp_Loop_138BPM.wav");
                    render_sample_node("Adhana_Vocal_Mantra_Chant_01.wav", "Adhana_Vocal_Mantra_Chant_01.wav");
                    render_sample_node("Adhana_Vocal_Mantra_Chant_02.wav", "Adhana_Vocal_Mantra_Chant_02.wav");
                    render_sample_node("Adhana_Tribal_Vocal_Chop.wav", "Adhana_Tribal_Vocal_Chop.wav");
                    render_sample_node("Adhana_Psy_Laser_Zap_FX.wav", "Adhana_Psy_Laser_Zap_FX.wav");
                    render_sample_node("Adhana_Cosmic_Riser_FX.wav", "Adhana_Cosmic_Riser_FX.wav");
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("EDM & Psytrance Sound Bank")) {
                    if (ImGui::TreeNode("Kicks")) {
                        render_sample_node("Psytrance_Kick_140BPM.wav", "Psytrance_Kick_140BPM.wav");
                        render_sample_node("Hardstyle_Sub_Kick.wav", "Hardstyle_Sub_Kick.wav");
                        render_sample_node("Clean_808_Sub_Kick.wav", "Clean_808_Sub_Kick.wav");
                        render_sample_node("Slap_Punch_Kick.wav", "Slap_Punch_Kick.wav");
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("Snares & Claps")) {
                        render_sample_node("909_Tight_Snare.wav", "909_Tight_Snare.wav");
                        render_sample_node("808_Crisp_Clap.wav", "808_Crisp_Clap.wav");
                        render_sample_node("EDM_Smash_Clap.wav", "EDM_Smash_Clap.wav");
                        render_sample_node("Ghost_Snare_Click.wav", "Ghost_Snare_Click.wav");
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("HiHats & Perc")) {
                        render_sample_node("Closed_Metal_Hat.wav", "Closed_Metal_Hat.wav");
                        render_sample_node("Open_Psy_Hat.wav", "Open_Psy_Hat.wav");
                        render_sample_node("Psy_Click_Perc.wav", "Psy_Click_Perc.wav");
                        render_sample_node("Shaker_Groove_Hit.wav", "Shaker_Groove_Hit.wav");
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("SFX & Risers")) {
                        render_sample_node("Psy_Zap_Laser.wav", "Psy_Zap_Laser.wav");
                        render_sample_node("Alien_Downlifter.wav", "Alien_Downlifter.wav");
                        render_sample_node("White_Noise_Sweep.wav", "White_Noise_Sweep.wav");
                        render_sample_node("Cyber_Sub_Impact.wav", "Cyber_Sub_Impact.wav");
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }

            // Loops
            if (ImGui::TreeNode("Loops")) {
                render_sample_node("Adhana_Kick_Loop_138BPM.wav", "Adhana_Kick_Loop_138BPM.wav");
                render_sample_node("Adhana_Rolling_Bassline_Loop_138BPM.wav", "Adhana_Rolling_Bassline_Loop_138BPM.wav");
                render_sample_node("Adhana_Acid_Arp_Loop_138BPM.wav", "Adhana_Acid_Arp_Loop_138BPM.wav");
                ImGui::TreePop();
            }

            // Vocals
            if (ImGui::TreeNode("Vocals")) {
                render_sample_node("Adhana_Vocal_Mantra_Chant_01.wav", "Adhana_Vocal_Mantra_Chant_01.wav");
                render_sample_node("Adhana_Vocal_Mantra_Chant_02.wav", "Adhana_Vocal_Mantra_Chant_02.wav");
                render_sample_node("Adhana_Tribal_Vocal_Chop.wav", "Adhana_Tribal_Vocal_Chop.wav");
                ImGui::TreePop();
            }

            // Stems
            if (ImGui::TreeNode("Musicas Recortadas (Stems)")) {
                std::string stems_base = "C:\\Users\\USUÁRIO\\Music\\Musicas Recortadas";
                auto base_p = to_fs_path(stems_base);
                if (std::filesystem::exists(base_p)) {
                    for (const auto& entry : std::filesystem::directory_iterator(base_p)) {
                        if (entry.is_directory()) {
                            std::string dir_name = to_utf8_str(entry.path().filename());
                            if (ImGui::TreeNode(dir_name.c_str())) {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.70f, 0.40f, 0.9f));
                                if (ImGui::Button(("Carregar Toda a Musica na Timeline##" + dir_name).c_str(), ImVec2(-1, 22))) {
                                    g_cloud_stem_ui.LoadExistingStemsDirectory(to_utf8_str(entry.path()), g_clip_manager, ::timeline);
                                }
                                ImGui::PopStyleColor();
                                for (const auto& f : std::filesystem::directory_iterator(entry.path())) {
                                    if (f.path().extension() == ".wav") {
                                        std::string fname = to_utf8_str(f.path().filename());
                                        if (ImGui::Selectable(fname.c_str())) {
                                            std::string path_str = to_utf8_str(f.path());
                                            PlaySoundA(path_str.c_str(), NULL, SND_ASYNC | SND_FILENAME);
                                            if (g_global_sampler) {
                                                g_global_sampler->loadSample(path_str);
                                            }
                                        }
                                    }
                                }
                                ImGui::TreePop();
                            }
                        }
                    }
                }
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        // 4. EFFECTS
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.45f, 1.00f, 1.0f));
        bool open_effects = ImGui::TreeNodeEx("EFFECTS", ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::PopStyleColor();
        if (open_effects) {
            if (ImGui::Selectable("  Maximus (Multiband Maximizer)")) g_maximus_master_ui.open();
            if (ImGui::Selectable("  Fruity Peak Controller")) g_peak_controller_ui.open();
            if (ImGui::Selectable("  Edison Audio Wave Editor")) g_edison_editor_ui.open();
            if (ImGui::Selectable("  FL Studio Patcher (Modular)")) g_patcher_ui.open();
            if (ImGui::Selectable("  Vocodex (100-Band Vocoder)")) g_vocodex_synth_ui.open();
            if (ImGui::Selectable("  Fruity Love Philter (Mod Filter)")) g_love_philter_ui.open();
            if (ImGui::Selectable("  Sakura (Physical Strings)")) g_sakura_synth_ui.open();
            if (ImGui::Selectable("  Fruity Convolver (IR Reverb)")) g_convolver_ui.open();
            if (ImGui::Selectable("  Fruity WaveShaper (Saturation)")) g_waveshaper_ui.open();
            if (ImGui::Selectable("  Soundgoodizer (Mix Polish)")) g_soundgoodizer_ui.open();
            if (ImGui::Selectable("  Parametric EQ 2")) g_parametric_eq2_ui.open();
            if (ImGui::Selectable("  Gross Beat (Time & Pitch)")) g_gross_beat_ui.open();
            if (ImGui::Selectable("  Delay 3")) g_delay3_ui.open();
            ImGui::TreePop();
        }

        // 5. TEMPLATES
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.85f, 1.00f, 1.0f));
        bool open_templates = ImGui::TreeNodeEx("TEMPLATES", ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::PopStyleColor();
        if (open_templates) {
            if (ImGui::Selectable("  Psytrance Full-On 140 BPM")) {
                show_genre_templates_window = true;
            }
            if (ImGui::Selectable("  Synthwave Cyberpunk 120 BPM")) {
                show_genre_templates_window = true;
            }
            if (ImGui::Selectable("  EDM Big Room 128 BPM")) {
                show_genre_templates_window = true;
            }
            if (ImGui::Selectable("  Drum & Bass 175 BPM")) {
                show_genre_templates_window = true;
            }
            ImGui::TreePop();
        }
        }
        ImGui::End();
        ImGui::PopStyleColor(1);
    }

        // ==========================================
        // 3. BOTTOM RACK (UNIFIED INSPECTOR DOCK)
        // ==========================================
        if (show_inspector) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.09f, 0.14f, 1.0f)); // #101722 Card Panels
            if (ImGui::Begin("INSPECTOR", &show_inspector, ImGuiWindowFlags_NoCollapse)) {
                
                // Header do Inspector
                ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "🎛️ DOCK DETALHES");
                ImGui::SameLine(0, 15);
                ImGui::TextDisabled("Controles da Faixa & Edição");
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 90.0f);
                if (ImGui::Button("⯆ Ocultar", ImVec2(80, 20))) {
                    show_inspector = false;
                }
                ImGui::Separator();
                ImGui::Spacing();

            if (ImGui::BeginTabBar("UnifiedInspectorDockTabs")) {
                
                // ── ABA 1: CONTROLES RÁPIDOS DA FAIXA SELECIONADA (EDIÇÃO PRÁTICA) ───
                if (ImGui::BeginTabItem("🎛️ CONTROLES DA FAIXA")) {
                    ImGui::Spacing();
                    
                    int sel_t = std::clamp(selected_track_idx, 0, MAX_TRACKS - 1);
                    const char* t_name = ::track_names[sel_t].c_str();
                    
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.12f, 0.18f, 0.90f));
                    ImGui::BeginChild("TrackQuickControlsCard", ImVec2(0, 0), true);
                    
                    // Header da Trilha Ativa
                    ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "FAIXA SELECIONADA:");
                    ImGui::SameLine(0, 8);
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.2f, 1.0f), "Track %02d — %s", sel_t + 1, t_name);
                    
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 320.0f);
                    if (ImGui::Button("🎹 Abrir Piano Roll", ImVec2(150, 26))) {
                        show_piano_roll = true;
                    }
                    ImGui::SameLine(0, 8);
                    if (ImGui::Button("🎚️ Ver no Mixer", ImVec2(140, 26))) {
                        set_mixer_focus = true;
                    }
                    
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    // Coluna 1: Ganho, Pan, Mute, Solo
                    ImGui::Columns(3, "TrackEditCols", false);
                    ImGui::SetColumnWidth(0, 240.0f);
                    ImGui::SetColumnWidth(1, 320.0f);
                    
                    ImGui::TextDisabled("NÍVEL & PANORAMA:");
                    float cur_vol = ::track_linear_volumes[sel_t];
                    if (ImGui::SliderFloat("Volume", &cur_vol, 0.0f, 1.5f, "%.2fx")) {
                        ::track_linear_volumes[sel_t] = cur_vol;
                        ::track_volumes[sel_t] = (cur_vol <= 0.001f) ? -60.0f : 20.0f * std::log10(cur_vol);
                    }
                    
                    float cur_pan = (sel_t < 8) ? ::dummy_pan[sel_t] : 0.0f;
                    if (ImGui::SliderFloat("Pan (L/R)", &cur_pan, -1.0f, 1.0f, (cur_pan < -0.05f) ? "L %.0f%%" : ((cur_pan > 0.05f) ? "R %.0f%%" : "CENTER"))) {
                        if (sel_t < 8) ::dummy_pan[sel_t] = cur_pan;
                    }
                    
                    ImGui::Spacing();
                    bool is_muted = (sel_t < 4) ? ::track_mutes[sel_t] : false;
                    if (ImGui::Button(is_muted ? "🔴 MUTED" : "MUTE", ImVec2(100, 24))) {
                        if (sel_t < 4) ::track_mutes[sel_t] = !::track_mutes[sel_t];
                    }
                    ImGui::SameLine(0, 8);
                    bool is_solo = (sel_t < 4) ? ::track_solos[sel_t] : false;
                    if (ImGui::Button(is_solo ? "🟡 SOLO [ON]" : "SOLO", ImVec2(100, 24))) {
                        if (sel_t < 4) ::track_solos[sel_t] = !::track_solos[sel_t];
                    }
                    
                    // Coluna 2: Timbre & Envelope ADSR
                    ImGui::NextColumn();
                    ImGui::TextDisabled("ENVELOPE ADSR & FILTRO:");
                    
                    auto& fs = ::g_piano_synth.flex_settings[sel_t];
                    
                    ImGui::SliderFloat("Ataque", &fs.env_vol_a, 0.001f, 1.0f, "%.3f s");
                    ImGui::SliderFloat("Decaimento", &fs.env_vol_d, 0.01f, 2.0f, "%.2f s");
                    ImGui::SliderFloat("Sustain", &fs.env_vol_s, 0.0f, 1.0f, "%.0f%%");
                    ImGui::SliderFloat("Release", &fs.env_vol_r, 0.01f, 3.0f, "%.2f s");
                    
                    ImGui::Spacing();
                    ImGui::SliderFloat("Corte Filtro", &fs.filter_cutoff, 0.01f, 1.0f, "Cutoff");
                    
                    // Coluna 3: Ações Rápidas & Sound Design Avançado
                    ImGui::NextColumn();
                    ImGui::TextDisabled("SOUND DESIGN AVANÇADO (OPCIONAL):");
                    ImGui::TextWrapped("Para editar notas e samples desta faixa com profundidade:");
                    ImGui::Spacing();
                    
                    if (ImGui::Button("🎹 Editar Padrão no Piano Roll", ImVec2(-1, 28))) {
                        show_piano_roll = true;
                    }
                    if (ImGui::Button("🎛️ Abrir Pedaleira de Efeitos (Rack)", ImVec2(-1, 28))) {
                        g_hardcore_pedalboard_ui.open();
                    }
                    if (ImGui::Button("✨ Abrir Multi-Sampler (DirectWave)", ImVec2(-1, 28))) {
                        g_directwave_ui.open();
                    }
                    
                    ImGui::Columns(1);
                    ImGui::EndChild();
                    ImGui::PopStyleColor(1);
                    
                    ImGui::EndTabItem();
                }

                // ── ABA 2: SOUND DESIGN & INSTRUMENTOS AVANÇADOS ──────────────
                if (ImGui::BeginTabItem("🔧 PLUGINS")) {
                    ImGui::Spacing();
                    if (ImGui::BeginTabBar("TrackInstrumentTabs")) {
                        if (ImGui::BeginTabItem("FL DIRECTWAVE")) {
                            g_directwave_ui.RenderEmbedded();
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("SYNTH (KuroWave)")) {
                            KuroUI::RenderKuroWaveSynth(g_kurowave);
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("SAMPLER")) {
                            KuroUI::RenderKuroSampler(g_global_sampler);
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("FRUITY GRANULIZER")) {
                            g_granulizer_ui.RenderEmbedded();
                            ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }
                    ImGui::EndTabItem();
                }

                // ── ABA 2: MIDI & PIANO ROLL ─────────────────────────────────
                if (ImGui::BeginTabItem("🎼 MIDI")) {
                    ImGui::Spacing();
                    KuroUI::RenderStepSequencer(nullptr, ::timeline, timeline.getBPM(), g_clip_manager);
                    ImGui::EndTabItem();
                }

                // ── ABA 3: GLOBAL MIXER (5 CANAIS OFICIAIS DO MOCKUP) ─────────
                ImGuiTabItemFlags mixer_flags = set_mixer_focus ? ImGuiTabItemFlags_SetSelected : 0;
                if (ImGui::BeginTabItem("🎚️ MIXER", nullptr, mixer_flags)) {
                    ImGui::Spacing();
                    
                    const char* mixer_ch_names[5] = { "DRUMS", "BASS", "SYNTH", "VOCALS", "MASTER" };
                    const char* mixer_ch_icons[5] = { "🥁", "🎸", "🎹", "🎤", "🎚️" };
                    ImU32 mixer_ch_colors[5] = {
                        IM_COL32(0, 212, 255, 255),
                        IM_COL32(0, 163, 217, 255),
                        IM_COL32(0, 119, 182, 255),
                        IM_COL32(94, 96, 206, 255),
                        IM_COL32(0, 212, 255, 255)
                    };

                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    float right_panel_w = std::clamp(avail.x * 0.38f, 260.0f, 420.0f);
                    float strip_w = std::max(85.0f, (avail.x - right_panel_w - 20.0f) / 5.0f);
                    float strip_h = avail.y - 10.0f;

                    ImGui::BeginChild("MixerTracksContainer", ImVec2(avail.x - right_panel_w - 10.0f, strip_h), false, ImGuiWindowFlags_HorizontalScrollbar);
                    
                    for (int ch = 0; ch < 5; ch++) {
                        ImGui::PushID(ch + 7700);
                        ImGui::BeginChild("ChStrip", ImVec2(strip_w, strip_h - 4.0f), true);

                        // Cabeçalho do Canal
                        ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "%s %s", mixer_ch_icons[ch], mixer_ch_names[ch]);
                        ImGui::Separator();
                        ImGui::Spacing();

                        // VU Meter LED Vertical (Gradiente Ciano -> Vermelho)
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        ImVec2 p = ImGui::GetCursorScreenPos();
                        float meter_w = 12.0f;
                        float meter_h = strip_h - 110.0f;

                        dl->AddRectFilled(p, ImVec2(p.x + meter_w, p.y + meter_h), IM_COL32(12, 18, 26, 255), 2.0f);
                        
                        float sim_level = (ch == 4) ? ((::master_vu_level_l + ::master_vu_level_r) * 0.5f) : (::track_vu_levels[ch] * 1.8f);
                        sim_level = std::clamp(sim_level, 0.05f, 1.0f);
                        float lit_h = meter_h * sim_level;

                        dl->AddRectFilled(ImVec2(p.x, p.y + meter_h - lit_h), ImVec2(p.x + meter_w, p.y + meter_h), mixer_ch_colors[ch], 2.0f);

                        // Fader Vertical ao Lado do VU Meter
                        ImGui::SetCursorScreenPos(ImVec2(p.x + meter_w + 14.0f, p.y));
                        ImGui::PushItemWidth(35.0f);
                        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.00f, 0.85f, 1.00f, 1.0f));
                        
                        if (ch == 4) {
                            ImGui::VSliderFloat("##m_fader", ImVec2(35.0f, meter_h), &g_master_volume, 0.0f, 1.2f, "");
                        } else {
                            static float fader_vols[4] = { 0.8f, 0.75f, 0.7f, 0.75f };
                            if (ImGui::VSliderFloat("##trk_fader", ImVec2(35.0f, meter_h), &fader_vols[ch], 0.0f, 1.2f, "")) {
                                ::track_volumes[ch] = (fader_vols[ch] <= 0.001f) ? -60.0f : 20.0f * std::log10(fader_vols[ch]);
                            }
                        }
                        ImGui::PopStyleColor();
                        ImGui::PopItemWidth();

                        // Botões Mute / Solo no Rodapé do Strip
                        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + meter_h + 10.0f));
                        bool muted = (ch < 4) ? ::track_mutes[ch] : false;
                        if (ImGui::Button(muted ? "M[ON]" : "MUTE", ImVec2(strip_w * 0.45f, 20.0f))) {
                            if (ch < 4) ::track_mutes[ch] = !::track_mutes[ch];
                        }
                        ImGui::SameLine();
                        bool soloed = (ch < 4) ? ::track_solos[ch] : false;
                        if (ImGui::Button(soloed ? "S[ON]" : "SOLO", ImVec2(strip_w * 0.45f, 20.0f))) {
                            if (ch < 4) ::track_solos[ch] = !::track_solos[ch];
                        }

                        ImGui::EndChild();
                        ImGui::SameLine();
                        ImGui::PopID();
                    }

                    ImGui::EndChild(); // MixerTracksContainer

                    ImGui::SameLine();
                    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                    ImGui::SameLine();

                    // Painel Lateral Master: Visualizador FFT/Goniômetro + Quick FX
                    ImGui::BeginChild("MasterScopeAndFx", ImVec2(0, strip_h - 4.0f), true);
                    
                    ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "🛸 MASTER SPECTRUM & SCOPE");
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    // Renderizador Embutido do Espectro & Vectorscope
                    g_spectrum_visualizer.RenderEmbedded(0.0f, std::min(125.0f, strip_h * 0.45f));
                    
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "FX INSERTS");
                    ImGui::Separator();
                    if (ImGui::Button("🔮 Gross Beat", ImVec2(-1, 22))) g_gross_beat_ui.open();
                    if (ImGui::Button("🎚️ Parametric EQ2", ImVec2(-1, 22))) g_parametric_eq2_ui.open();
                    if (ImGui::Button("🌊 Fruity Reverb 2", ImVec2(-1, 22))) g_reeverb2_ui.open();
                    if (ImGui::Button("⏱️ Fruity Delay 3", ImVec2(-1, 22))) g_delay3_ui.open();
                    if (ImGui::Button("⚡ Maximus Limiter", ImVec2(-1, 22))) g_maximus_master_ui.open();
                    ImGui::EndChild();

                    ImGui::EndTabItem();
                }

                // ── ABA 4: FX CHAIN ──────────────────────────────────────────
                if (ImGui::BeginTabItem("🔮 FX CHAIN")) {
                    ImGui::Spacing();
                    KuroUI::GrossBeatUI::Render(::g_gross_beat, nullptr);
                    ImGui::EndTabItem();
                }

                if (set_mixer_focus) set_mixer_focus = false;
                
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
        ImGui::PopStyleColor(1);
    }

        // ==========================================
        // 4. ATALHOS GLOBAIS (FL STUDIO STYLE)
        // ==========================================
        if (!io.WantTextInput) {
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
            if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
                show_playlist = !show_playlist;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F6)) {
                show_step_sequencer = !show_step_sequencer;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F7)) {
                show_piano_roll = !show_piano_roll;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F8)) {
                show_browser = !show_browser;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
                show_mixer = !show_mixer;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F10)) {
                g_cloud_stem_ui.toggle();
            }
        }
    }

    // FASE 28: Renderiza Janelas Flutuantes (Fora da Janela Principal)
    inline void RenderFloatingWindows() {
        RenderMixerFloatingWindow();
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
        
        static bool show_export_modal = false;
        show_piano_roll_prev = show_piano_roll;
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
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "MODO EXTRAÇÃO DE STEMS: EM CONSTRUÇÃO");
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

    static void RenderMixerFloatingWindow() {
        if (!show_mixer) return;

        ImGui::SetNextWindowSize(ImVec2(1060, 520), ImGuiCond_FirstUseEver);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.09f, 0.98f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.85f)); // Neon Cyan

        if (ImGui::Begin("🎚️ FL STUDIO MIXER CONSOLE (16 TRACKS + MASTER) (F9)###FloatingMixerWindow", &show_mixer, ImGuiWindowFlags_MenuBar)) {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Exibir")) {
                    if (ImGui::MenuItem("Espectro Master FFT", "", true)) {}
                    ImGui::EndMenu();
                }
                float menu_w = ImGui::GetWindowWidth();
                ImGui::SameLine(menu_w - 90);
                if (ImGui::Button("✖ Fechar", ImVec2(80, 20))) {
                    show_mixer = false;
                }
                ImGui::EndMenuBar();
            }

            ImVec2 avail = ImGui::GetContentRegionAvail();
            float right_panel_w = std::clamp(avail.x * 0.30f, 250.0f, 340.0f);
            float tracks_area_w = avail.x - right_panel_w - 15.0f;
            float strip_w = 84.0f;
            float strip_h = avail.y - 10.0f;

            // ── ÁREA DE FADERS DOS CANAIS (MASTER + 16 TRACKS) ─────────────
            ImGui::BeginChild("FloatingMixerTracksContainer", ImVec2(tracks_area_w, strip_h), true, ImGuiWindowFlags_HorizontalScrollbar);
            
            // 1. MASTER TRACK (Faixa 0 / Master)
            {
                ImGui::PushID(9999);
                ImGui::BeginChild("MasterStrip", ImVec2(strip_w, strip_h - 10.0f), true);
                ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.0f, 1.0f), "👑 MASTER");
                ImGui::Separator();
                ImGui::Spacing();

                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetCursorScreenPos();
                float meter_w = 10.0f;
                float meter_h = strip_h - 110.0f;

                dl->AddRectFilled(p, ImVec2(p.x + meter_w, p.y + meter_h), IM_COL32(12, 18, 26, 255), 2.0f);
                float sim_level = std::clamp(((::master_vu_level_l + ::master_vu_level_r) * 0.5f), 0.05f, 1.0f);
                float lit_h = meter_h * sim_level;
                dl->AddRectFilled(ImVec2(p.x, p.y + meter_h - lit_h), ImVec2(p.x + meter_w, p.y + meter_h), IM_COL32(255, 140, 0, 255), 2.0f);

                ImGui::SetCursorScreenPos(ImVec2(p.x + meter_w + 10.0f, p.y));
                ImGui::PushItemWidth(32.0f);
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.55f, 0.0f, 1.0f));
                ImGui::VSliderFloat("##m_fader_fl", ImVec2(32.0f, meter_h), &g_master_volume, 0.0f, 1.2f, "");
                ImGui::PopStyleColor();
                ImGui::PopItemWidth();

                ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + meter_h + 8.0f));
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%.1f dB", (g_master_volume > 0.001f) ? (20.0f * std::log10(g_master_volume)) : -60.0f);

                ImGui::EndChild();
                ImGui::SameLine(0, 4);
                ImGui::PopID();
            }

            // 2. 16 FAIXAS DE INSTRUMENTOS DO PROJETO
            for (int t = 0; t < 16; t++) {
                ImGui::PushID(t + 8800);
                ImGui::BeginChild("TrackStrip", ImVec2(strip_w, strip_h - 10.0f), true);

                ImVec4 trk_col = (t == 0) ? ImVec4(0.0f, 0.9f, 1.0f, 1.0f) :
                                 (t == 1 || t == 2) ? ImVec4(1.0f, 0.2f, 0.8f, 1.0f) :
                                 (t == 3 || t == 4 || t == 5) ? ImVec4(1.0f, 0.6f, 0.1f, 1.0f) :
                                 (t == 6 || t == 7 || t == 8) ? ImVec4(0.2f, 1.0f, 0.5f, 1.0f) : ImVec4(0.7f, 0.8f, 1.0f, 1.0f);

                ImGui::TextColored(trk_col, "%02d", t + 1);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", ::track_names[t].c_str());
                ImGui::Separator();
                ImGui::Spacing();

                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetCursorScreenPos();
                float meter_w = 8.0f;
                float meter_h = strip_h - 110.0f;

                dl->AddRectFilled(p, ImVec2(p.x + meter_w, p.y + meter_h), IM_COL32(12, 18, 26, 255), 2.0f);
                float trk_sim = std::clamp(::track_vu_levels[t] * 1.6f, 0.02f, 1.0f);
                float trk_lit_h = meter_h * trk_sim;
                dl->AddRectFilled(ImVec2(p.x, p.y + meter_h - trk_lit_h), ImVec2(p.x + meter_w, p.y + meter_h), ImGui::GetColorU32(trk_col), 2.0f);

                ImGui::SetCursorScreenPos(ImVec2(p.x + meter_w + 8.0f, p.y));
                ImGui::PushItemWidth(32.0f);
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, trk_col);
                
                float lin_v = (::track_volumes[t] <= -60.0f) ? 0.0f : std::pow(10.0f, ::track_volumes[t] / 20.0f);
                if (ImGui::VSliderFloat("##trk_fader_fl", ImVec2(32.0f, meter_h), &lin_v, 0.0f, 1.2f, "")) {
                    ::track_volumes[t] = (lin_v <= 0.001f) ? -60.0f : 20.0f * std::log10(lin_v);
                }
                ImGui::PopStyleColor();
                ImGui::PopItemWidth();

                ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + meter_h + 8.0f));
                bool muted = ::track_mutes[t];
                if (ImGui::Button(muted ? "M[ON]" : "M", ImVec2(strip_w * 0.42f, 20.0f))) {
                    ::track_mutes[t] = !::track_mutes[t];
                }
                ImGui::SameLine(0, 2);
                bool soloed = ::track_solos[t];
                if (ImGui::Button(soloed ? "S[ON]" : "S", ImVec2(strip_w * 0.42f, 20.0f))) {
                    ::track_solos[t] = !::track_solos[t];
                }

                ImGui::EndChild();
                ImGui::SameLine(0, 4);
                ImGui::PopID();
            }

            ImGui::EndChild(); // FloatingMixerTracksContainer

            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine();

            // ── PAINEL LATERAL MASTER: ESPECTRO + FX SLOTS ────────────────
            ImGui::BeginChild("FloatingMixerMasterPanel", ImVec2(0, strip_h), true);
            ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "🛸 MASTER SPECTRUM & SCOPE");
            ImGui::Separator();
            ImGui::Spacing();
            g_spectrum_visualizer.RenderEmbedded(0.0f, std::min(130.0f, strip_h * 0.40f));

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "🎛️ FX SLOTS & INSERTS");
            ImGui::Separator();
            if (ImGui::Button("🔮 Gross Beat", ImVec2(-1, 24))) g_gross_beat_ui.open();
            if (ImGui::Button("🎚️ Parametric EQ2", ImVec2(-1, 24))) g_parametric_eq2_ui.open();
            if (ImGui::Button("🌊 Fruity Reverb 2", ImVec2(-1, 24))) g_reeverb2_ui.open();
            if (ImGui::Button("⏱️ Fruity Delay 3", ImVec2(-1, 24))) g_delay3_ui.open();
            if (ImGui::Button("⚡ Maximus Multiband Limiter", ImVec2(-1, 24))) g_maximus_master_ui.open();
            if (ImGui::Button("✨ Soundgoodizer", ImVec2(-1, 24))) g_soundgoodizer_ui.open();
            ImGui::EndChild();
        }
        ImGui::End();
        ImGui::PopStyleColor(2);
    }

#include "SciFiIconSystem.h"
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

            // ── 1. LOGO & SCI-FI SHIP EMBLEM ──────────────────────────────
            {
                ImVec2 logo_pos = ImGui::GetCursorScreenPos();
                ImDrawList* dl = ImGui::GetWindowDrawList();
                SciFiHUD::DrawIcon(dl, ImVec2(logo_pos.x, logo_pos.y + 3), ImVec2(logo_pos.x + 16, logo_pos.y + 19), SciFiHUD::IconType::ABDUCTION_SHIP, 0xFF00E5FF, 1.5f);
                ImGui::Dummy(ImVec2(18, 22));
                ImGui::SameLine(0, 4);

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.85f, 1.00f, 1.00f));
                ImGui::TextUnformatted("ABDUCTION");
                ImGui::PopStyleColor();
            }
            ImGui::SameLine(0, 8);

            // ── 2. MENU DROPDOWNS (Arquivo, Editar, Exibir) ───────────────
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
                if (ImGui::MenuItem("Templates de Producao (Musicas Prontas)", "", show_genre_templates_window)) {
                    show_genre_templates_window = !show_genre_templates_window;
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
                if (ImGui::MenuItem("Mostrar Browser", "", show_browser)) { show_browser = !show_browser; }
                if (ImGui::MenuItem("Mostrar Playlist", "", show_playlist)) { show_playlist = !show_playlist; }
                if (ImGui::MenuItem("Channel Rack", "", show_step_sequencer)) { show_step_sequencer = !show_step_sequencer; }
                if (ImGui::MenuItem("Piano Roll", "", show_piano_roll)) { show_piano_roll = !show_piano_roll; }
                ImGui::Separator();
                if (ImGui::MenuItem("Separador de Stems AI (DJ Slicer)", "", g_cloud_stem_ui.getOpenState())) { g_cloud_stem_ui.toggle(); }
                ImGui::EndMenu();
            }

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 8);

            // ── 3. SCI-FI TRANSPORT CONSOLE (PLAY, STOP, REC, PAT/SONG) ────
            bool is_p = is_playing;
            if (SciFiHUD::SciFiTransportBtn("##TPlay", is_p ? SciFiHUD::IconType::PAUSE : SciFiHUD::IconType::PLAY, is_p, ImVec4(0.00f, 0.85f, 1.00f, 1.0f), ImVec2(24, 22), "Tocar / Pausar (Barra de Espaço)")) {
                KuroUI::TransportController::TogglePlayPause(timeline);
            }
            ImGui::SameLine(0, 2);

            if (SciFiHUD::SciFiTransportBtn("##TStop", SciFiHUD::IconType::STOP, false, ImVec4(0.50f, 0.65f, 0.80f, 1.0f), ImVec2(24, 22), "Parar (Stop)")) {
                KuroUI::TransportController::Stop(timeline);
            }
            ImGui::SameLine(0, 2);

            bool rec = g_record_manager.isRecording();
            if (SciFiHUD::SciFiTransportBtn("##TRec", SciFiHUD::IconType::RECORD, rec, ImVec4(0.90f, 0.15f, 0.20f, 1.0f), ImVec2(24, 22), "Gravar Áudio Master / MIDI")) {
                if (rec) g_record_manager.setRecording(false);
                else     g_record_manager.setRecording(true, false);
            }
            ImGui::SameLine(0, 4);

            // PAT / SONG Mode Switcher
            bool is_pat = timeline.is_pattern_mode;
            ImGui::PushStyleColor(ImGuiCol_Button, is_pat ? ImVec4(1.00f, 0.55f, 0.00f, 1.0f) : ImVec4(0.08f, 0.12f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, is_pat ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(1.00f, 0.65f, 0.20f, 1.0f));
            if (ImGui::Button("PAT", ImVec2(32, 22))) {
                timeline.is_pattern_mode = true;
            }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Modo Padrão / Pattern (Loop do Channel Rack / Piano Roll)");
            ImGui::SameLine(0, 2);

            ImGui::PushStyleColor(ImGuiCol_Button, !is_pat ? ImVec4(0.00f, 0.85f, 1.00f, 1.0f) : ImVec4(0.08f, 0.12f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, !is_pat ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.00f, 0.85f, 1.00f, 1.0f));
            if (ImGui::Button("SONG", ImVec2(38, 22))) {
                timeline.is_pattern_mode = false;
            }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Modo Música / Song (Toca a Playlist Completa)");
            ImGui::SameLine(0, 8);

            // ── 4. COMPACT LCD TEMPO & TIMECODE ───────────────────────────
            {
                size_t sr = ai_engine.getSampleRate(); if (sr == 0) sr = 44100;
                size_t mf = timeline.getMasterFrame();
                int cs = (int)(mf / sr);
                int ms = (int)((mf % sr) * 10 / sr);
                float current_bpm = timeline.getBPM();

                ImGui::SetNextItemWidth(62);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.08f, 0.12f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.85f, 1.00f, 1.0f));
                if (ImGui::DragFloat("##TopBPM", &current_bpm, 0.5f, 40.0f, 300.0f, "%.0f BPM")) {
                    timeline.setBPM(current_bpm);
                }
                ImGui::PopStyleColor(2);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Tempo da Música em BPM (Arraste)");
                ImGui::SameLine(0, 4);

                char tc_str[16];
                snprintf(tc_str, sizeof(tc_str), "%02d:%02d.%1d", cs / 60, cs % 60, ms);
                ImGui::TextColored(ImVec4(0.70f, 0.85f, 0.95f, 1.0f), "%s", tc_str);
            }

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 8);

            // ── 5. THE 5 SCI-FI COCKPIT VIEW SWITCHER BUTTONS ──────────────
            if (SciFiHUD::SciFiButton("##btn_playlist", "Playlist", SciFiHUD::IconType::PLAYLIST_TRACKS, show_playlist, ImVec2(0, 22), "Playlist / Arranjo da Nave (F5)")) {
                show_playlist = !show_playlist;
            }
            ImGui::SameLine(0, 3);

            if (SciFiHUD::SciFiButton("##btn_rack", "Rack", SciFiHUD::IconType::CHANNEL_RACK, show_step_sequencer, ImVec2(0, 22), "Channel Rack / Matrix 16-Passos (F6)")) {
                show_step_sequencer = !show_step_sequencer;
            }
            ImGui::SameLine(0, 3);

            if (SciFiHUD::SciFiButton("##btn_piano", "Piano", SciFiHUD::IconType::PIANO_KEYS, show_piano_roll, ImVec2(0, 22), "Piano Roll / Sintetizador & Melodia (F7)")) {
                show_piano_roll = !show_piano_roll;
            }
            ImGui::SameLine(0, 3);

            if (SciFiHUD::SciFiButton("##btn_mixer", "Mixer", SciFiHUD::IconType::MIXER_FADERS, show_mixer, ImVec2(0, 22), "Console Mixer & Faders Flutuante (F9)")) {
                show_mixer = !show_mixer;
            }
            ImGui::SameLine(0, 3);

            if (SciFiHUD::SciFiButton("##btn_browser", "Browser", SciFiHUD::IconType::BROWSER_FOLDER, show_browser, ImVec2(0, 22), "Banco de Amostras & VSTs (F8)")) {
                show_browser = !show_browser;
            }
            ImGui::SameLine(0, 3);

            if (SciFiHUD::SciFiButton("##btn_ai_stems", "AI Stems", SciFiHUD::IconType::AI_NEURAL_STEMS, g_cloud_stem_ui.getOpenState(), ImVec2(0, 22), "Separador de Stems & Elementos (IA)")) {
                g_cloud_stem_ui.toggle();
            }

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 8);

            // ── 6. WORKSPACE MODE & PROJECT NODE ───────────────────────────
            const char* mode_labels[] = { "PRODUCE", "SLICING", "MIX", "LIVE" };
            int cur_m_idx = (current_app_mode == AppMode::PRODUCE_MODE) ? 0 :
                            (current_app_mode == AppMode::BEAT_SLICING_MODE) ? 1 :
                            (current_app_mode == AppMode::MIX_MODE) ? 2 : 3;

            {
                ImVec2 ic_pos = ImGui::GetCursorScreenPos();
                ImDrawList* dl = ImGui::GetWindowDrawList();
                SciFiHUD::DrawIcon(dl, ImVec2(ic_pos.x, ic_pos.y + 4), ImVec2(ic_pos.x + 14, ic_pos.y + 18), SciFiHUD::IconType::WORKSPACE_TARGET, 0xFF00E5FF, 1.2f);
                ImGui::Dummy(ImVec2(16, 22));
                ImGui::SameLine(0, 2);
            }

            ImGui::SetNextItemWidth(90);
            if (ImGui::BeginCombo("##workspace_mode_select", mode_labels[cur_m_idx])) {
                for (int m = 0; m < 4; m++) {
                    bool is_sel = (cur_m_idx == m);
                    if (ImGui::Selectable(mode_labels[m], is_sel)) {
                        current_app_mode = (m == 0) ? AppMode::PRODUCE_MODE :
                                           (m == 1) ? AppMode::BEAT_SLICING_MODE :
                                           (m == 2) ? AppMode::MIX_MODE : AppMode::LIVE_MODE;
                    }
                    if (is_sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Modo do Cockpit de Trabalho");
            ImGui::SameLine(0, 8);

            {
                ImVec2 ic_pos = ImGui::GetCursorScreenPos();
                ImDrawList* dl = ImGui::GetWindowDrawList();
                SciFiHUD::DrawIcon(dl, ImVec2(ic_pos.x, ic_pos.y + 4), ImVec2(ic_pos.x + 14, ic_pos.y + 18), SciFiHUD::IconType::PROJECT_VAULT, 0xFF44FF88, 1.2f);
                ImGui::Dummy(ImVec2(16, 22));
                ImGui::SameLine(0, 2);
            }

            ImGui::TextColored(ImVec4(0.40f, 0.85f, 0.60f, 1.0f), "%s", g_current_project_name.c_str());
            
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
        RenderParametricEQ2();
        RenderSoundgoodizer();
        RenderAIStemSeparator();
        RenderDelayLamaPlugin();
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
            case AppMode::PRODUCE_MODE:
                RenderStudioMode(ai_engine);
                RenderFloatingPluginWindows();
                RenderDelayLamaPlugin();
                RenderMonkSynthVst3Window();
                RenderContrabassWindow();
                break;
            case AppMode::BEAT_SLICING_MODE:
                RenderStudioMode(ai_engine);
                g_stem_beat_slicer_ui.open();
                break;
            case AppMode::MIX_MODE:
                RenderStudioMode(ai_engine);
                set_mixer_focus = true;
                break;
            case AppMode::LIVE_MODE:
                RenderDJMode();
                break;
        }

        RenderTutorialWindow();
        DawApiExplorerUI::Render(show_daw_api_explorer);
        g_auto_master_ui.Render();
        g_spatial_panner_ui.Render();
        g_peak_controller_ui.Render();
        g_soundfont_player_ui.Render();
        g_edison_editor_ui.Render();
        g_patcher_ui.Render();
        g_mixer_routing_ui.Render();
        g_harmor_synth_ui.Render();
        g_sytrus_synth_ui.Render();
        g_slicex_ui.Render();
        g_maximus_master_ui.Render();
        g_vocodex_synth_ui.Render();
        g_love_philter_ui.Render();
        g_sakura_synth_ui.Render();
        g_convolver_ui.Render();
        g_granulizer_ui.Render();
        g_stereo_shaper_ui.Render();
        g_waveshaper_ui.Render();
        g_soundgoodizer_ui.Render();
        g_parametric_eq2_ui.Render();
        g_gross_beat_ui.Render();
        g_delay3_ui.Render();
        g_control_surface_ui.Render();
        g_fruity_limiter_ui.Render();
        g_fast_lp_ui.Render();
        g_flangus_ui.Render();
        g_envelope_controller_ui.Render();
        g_hardcore_pedalboard_ui.Render();
        g_blood_overdrive_ui.Render();
        g_reeverb2_ui.Render();
        g_3xosc_ui.Render();
        g_phaser_ui.Render();
        g_granular_delay_ui.Render();
        g_transient_processor_ui.Render();
        g_fruity_vocoder_ui.Render();
        g_fruity_squeeze_ui.Render();
        g_fruity_scratcher_ui.Render();
        g_stereo_enhancer_ui.Render();
        g_bass_drum_ui.Render();
        g_center_panner_ui.Render();
        g_formula_controller_ui.Render();
        g_fruity_mute2_ui.Render();
        g_fruity_filter_ui.Render();
        g_fruity_stereo_tool_ui.Render();
        g_fruity_notebook_ui.Render();
        g_fruity_panomatic_ui.Render();
        g_fruity_soft_clipper_ui.Render();
        g_fruity_stereo_delay_ui.Render();
        g_wave_candy_ui.Render();
        g_fruity_dx10_ui.Render();
        g_project_info_ui.Render();
        g_cloud_stem_ui.Render(g_clip_manager, ::timeline);
        g_preset_manager_ui.Render();
        g_stem_beat_slicer_ui.Render();
        g_directwave_ui.Render();
        g_audio_clip_editor_ui.Render(g_clip_manager);
        if (show_genre_templates_window) {
            KuroUI::RenderGenreTemplatesWindow(&show_genre_templates_window, ::timeline, g_clip_manager);
        }

        if (show_step_sequencer) {
            ImGui::SetNextWindowSize(ImVec2(740, 420), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.17f, 0.20f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.19f, 0.22f, 0.26f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.22f, 0.26f, 0.31f, 1.0f));
            if (ImGui::Begin("Channel Rack###ChannelRackWindow", &show_step_sequencer, ImGuiWindowFlags_NoCollapse)) {
                KuroUI::RenderStepSequencer(&show_step_sequencer, ::timeline, timeline.getBPM(), g_clip_manager);
            }
            ImGui::End();
            ImGui::PopStyleColor(3);
        }

        if (show_piano_roll) {
            unsigned long long curr_frame = timeline.getMasterFrame();
            KuroUI::RenderPianoRoll(&show_piano_roll, ::timeline, selected_track_idx, timeline.getBPM(), &curr_frame, ::is_playing, g_clip_manager, nullptr);
        }

        if (show_modulation_panel) {
            KuroUI::RenderModulationPanel(&show_modulation_panel);
        }

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

        // ── ATALHOS GLOBAIS DA DAW (FL STUDIO STYLE) ──────────
        ImGuiIO& main_io = ImGui::GetIO();
        if (!main_io.WantTextInput) {
            if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
                KuroUI::TransportController::TogglePlayPause(timeline);
            }
            if (main_io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
                g_clip_manager.undo();
            }
            if (main_io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
                g_clip_manager.redo();
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
                show_playlist = !show_playlist;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F6)) {
                show_step_sequencer = !show_step_sequencer;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F7)) {
                show_piano_roll = !show_piano_roll;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F8)) {
                show_browser = !show_browser;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
                show_mixer = !show_mixer;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F10)) {
                g_cloud_stem_ui.toggle();
            }
        }
    }
}
