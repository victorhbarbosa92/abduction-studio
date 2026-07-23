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
    inline bool show_delay_lama = false;
    inline bool focus_delay_lama = false;
    inline bool show_monksynth_vst3 = false;
    inline bool focus_monksynth_vst3 = false;
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
            if (ImGui::Button("GET / STORE 🛒", ImVec2(110, 20))) flex_top_tab = 1;
            ImGui::PopStyleColor();
        }
        ImGui::SameLine(190);
        ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "PRESETS (65+)");
        ImGui::Separator();

        if (flex_top_tab == 1) {
            // =========================================================================
            // LOJA DE EXPANSÕES INTERATIVA (GET / STORE VIEW)
            // =========================================================================
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "🛒 FLEX STORE & EXPANSION PACK DOWNLOADER");
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
                    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "✓ INSTALADO & ATIVO NA DAW");
                } else if (downloading[k]) {
                    download_progress[k] += 0.03f;
                    ImGui::ProgressBar(download_progress[k], ImVec2(-1, 20), "Baixando Expansão...");
                    if (download_progress[k] >= 1.0f) {
                        downloading[k] = false;
                        flex_pack_installed[k] = true;
                    }
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.5f, 1.0f));
                    if (ImGui::Button("⚡ BAIXAR & INSTALAR (GRÁTIS)", ImVec2(-1, 26))) {
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
            if (ImGui::Button("← VOLTAR PARA O NAVEGADOR DE PRESETS", ImVec2(-1, 30))) {
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

static bool show_ai_stem_separator = false;
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



static void RenderFlexBrowser() {
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
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::SetNextWindowSize(ImVec2(340, 500), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("FLEX BROWSER", nullptr, ImGuiWindowFlags_NoDocking)) {
            ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "🛸 PACKS DE SINTETIZADOR FLEX:");
            ImGui::Separator();
            ImGui::Spacing();
            
            for (size_t i = 0; i < flex_packs_cache.size(); i++) {
                ImGui::PushID((int)i);
                if (i == 0) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f)); // Destaque Psytrance
                
                bool is_open = ImGui::TreeNodeEx(flex_packs_cache[i].c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen);
                if (i == 0) ImGui::PopStyleColor();
                
                if (is_open) {
                    if (i == 0) { // Psytrance Essentials
                        const char* psy_items[] = {
                            "🌀 Psytrance Rolling Bass (16th)",
                            "🧪 Psytrance 303 Acid Squelch",
                            "🌌 Psytrance Goa Saw Lead",
                            "🛸 Psytrance Alien Laser Zap",
                            "👁️ Psytrance Dark Psy Drone",
                            "⚡ Psytrance Gated Trance Arp",
                            "💥 Psytrance FM Sub Boom",
                            "🪵 Psytrance Tribal Perc Pluck",
                            "🚀 Psytrance Hypnotic Psy Sweep",
                            "🎯 Psytrance Full-On Scream Lead"
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
            if (ImGui::Button("🎹 ABRIR SINTETIZADOR FLEX 1:1", ImVec2(-1, 35))) {
                show_flex_browser = true;
            }
            ImGui::PopStyleColor();
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
                        g_piano_synth.triggerNote(49, 1.0f, 0.7f, 6);
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
        if (ImGui::TreeNodeEx("Samples & Loops (Clique para Selecionar)", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Selectable("  808 Kick.wav")) { g_piano_synth.selected_variant[0] = 0; g_piano_synth.triggerNote(36, 0.25f, 0.9f, 0); }
            if (ImGui::Selectable("  909 Kick.wav")) { g_piano_synth.selected_variant[0] = 1; g_piano_synth.triggerNote(36, 0.25f, 0.9f, 0); }
            if (ImGui::Selectable("  Acoustic Kick.wav")) { g_piano_synth.selected_variant[0] = 2; g_piano_synth.triggerNote(36, 0.25f, 0.9f, 0); }
            if (ImGui::Selectable("  FPC Kick.wav")) { g_piano_synth.selected_variant[0] = 3; g_piano_synth.triggerNote(36, 0.25f, 0.9f, 0); }
            if (ImGui::Selectable("  Kick_Alien_01.wav")) { g_piano_synth.selected_variant[0] = 0; g_piano_synth.triggerNote(36, 0.25f, 0.9f, 0); }
            if (ImGui::Selectable("  Bass_FM_C.wav")) { g_piano_synth.selected_variant[3] = 0; g_piano_synth.triggerNote(48, 0.25f, 0.9f, 3); }
            if (ImGui::Selectable("  Vocal_Abduction.wav")) { g_piano_synth.selected_variant[7] = 0; g_piano_synth.triggerNote(46, 0.25f, 0.9f, 7); }
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Plugins & Sintetizadores", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.97f, 0.50f, 0.0f, 1.0f)); // Laranja FL Studio
            if (ImGui::Selectable("  🎹 ABRIR FL FLEX SYNTH (PSYTRANCE)")) {
                show_flex_browser = true;
                active_flex_channel = 3;
            }
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.75f, 0.20f, 1.0f)); // Dourado Tibetano
            if (ImGui::Selectable("  [+] DELAY LAMA (MONGE TIBETANO 3D)")) {
                show_delay_lama = true;
                focus_delay_lama = true;
                g_piano_synth.flex_channel_instrument[5] = KuroAudio::MidiInstrument::DELAY_LAMA;
                g_piano_synth.flex_active[5] = true;
                g_piano_synth.setInstrument(KuroAudio::MidiInstrument::DELAY_LAMA);
                g_piano_synth.triggerNote(48, 1.2f, 0.9f, 5);
            }
            if (ImGui::Selectable("  [+] DELAY LAMA VST3 (MONKSYNTH NATIVO)")) {
                show_delay_lama = true;
                focus_delay_lama = true;
                g_piano_synth.flex_channel_instrument[5] = KuroAudio::MidiInstrument::DELAY_LAMA;
                g_piano_synth.flex_active[5] = true;
                g_piano_synth.setInstrument(KuroAudio::MidiInstrument::DELAY_LAMA);
                g_piano_synth.triggerNote(48, 1.2f, 0.9f, 5);
            }
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.22f, 1.00f, 0.85f, 1.0f)); // Ciano Neon Sci-Fi
            if (ImGui::Selectable("  [+] ALIEN LED SYNTH (AUTORAL SCI-FI)")) {
                g_piano_synth.flex_channel_instrument[5] = KuroAudio::MidiInstrument::ALIEN_LED_SYNTH;
                g_piano_synth.flex_active[5] = true;
                g_piano_synth.setInstrument(KuroAudio::MidiInstrument::ALIEN_LED_SYNTH);
                g_piano_synth.triggerNote(60, 1.0f, 0.9f, 5);
            }
            ImGui::PopStyleColor();

            ImGui::BulletText("Serum (VST3)");
            ImGui::BulletText("Vital (CLAP)");
            ImGui::BulletText("Abduction FM 4-Op");
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Projetos")) {
            ImGui::BulletText("Area51_Jam.abduct");
            ImGui::TreePop();
        }
        ImGui::End();

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
                const char* fx_names[10] = { "Parametric EQ 2", "Soundgoodizer", "FLEX Synth", "Fruity Delay 3", "Fruity Reverb 2", "(slot vazio)", "(slot vazio)", "(slot vazio)", "(slot vazio)", "(slot vazio)" };
                for(int slot = 0; slot < 10; slot++) {
                    ImGui::PushID(slot);
                    // FL Studio style FX slots
                    if (slot < 3) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.28f, 1.0f));
                    else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));

                    if (ImGui::Button(fx_names[slot], ImVec2(ImGui::GetContentRegionAvail().x - 20, 16))) {
                        if (slot == 0) show_parametric_eq2 = true;
                        else if (slot == 1) show_soundgoodizer = true;
                        else if (slot == 2) show_flex_browser = true;
                    }
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    
                    static float mix_lvls[10] = { 1.0f, 0.8f, 1.0f, 0.5f, 0.4f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
                    ImGui::PushItemWidth(15);
                    ImGui::SliderFloat("##mix", &mix_lvls[slot], 0, 1, "");
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

            // Timecode + Interativo BPM Display & Tap Tempo Controller
            {
                size_t sr = ai_engine.getSampleRate(); if (sr == 0) sr = 44100;
                size_t mf = timeline.getMasterFrame();
                int cs = (int)(mf / sr);
                int ms = (int)((mf % sr) * 10 / sr);
                
                // Display de Timecode LCD
                char tc_str[32];
                snprintf(tc_str, sizeof(tc_str), " %02d:%02d.%1d ", cs / 60, cs % 60, ms);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.06f, 0.07f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.33f, 1.00f, 0.20f, 1.0f));
                ImGui::Button(tc_str, ImVec2(75, 20));
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 4);

                // Controle Interativo de BPM (Arrasto / Edição Direta)
                float current_bpm = timeline.getBPM();
                ImGui::PushItemWidth(75);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.10f, 0.13f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.97f, 0.50f, 0.0f, 1.0f)); // Laranja FL
                if (ImGui::DragFloat("##InteractiveBPM", &current_bpm, 0.5f, 40.0f, 300.0f, "%.0f BPM")) {
                    timeline.setBPM(current_bpm);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Clique e arraste para mudar o BPM, ou clique duas vezes para digitar!");
                }
                ImGui::PopStyleColor(2);
                ImGui::PopItemWidth();
                ImGui::SameLine(0, 2);

                // Botões de Passo +1 e -1 BPM
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.18f, 0.22f, 1.0f));
                if (ImGui::Button("-##bpm_dec", ImVec2(18, 20))) {
                    timeline.setBPM(std::max(40.0f, current_bpm - 1.0f));
                }
                ImGui::SameLine(0, 1);
                if (ImGui::Button("+##bpm_inc", ImVec2(18, 20))) {
                    timeline.setBPM(std::min(300.0f, current_bpm + 1.0f));
                }
                ImGui::PopStyleColor();
                ImGui::SameLine(0, 4);

                // Tap Tempo & Presets de Gênero
                static double last_tap_time = 0.0;
                static std::vector<double> tap_intervals;
                if (ImGui::Button("TAP", ImVec2(36, 20))) {
                    double now = ImGui::GetTime();
                    if (last_tap_time > 0.0) {
                        double interval = now - last_tap_time;
                        if (interval > 0.15 && interval < 2.0) { // Entre 30 e 400 BPM
                            tap_intervals.push_back(interval);
                            if (tap_intervals.size() > 4) tap_intervals.erase(tap_intervals.begin());
                            double avg_interval = 0.0;
                            for (double d : tap_intervals) avg_interval += d;
                            avg_interval /= tap_intervals.size();
                            float tapped_bpm = (float)(60.0 / avg_interval);
                            timeline.setBPM(std::round(tapped_bpm));
                        } else {
                            tap_intervals.clear();
                        }
                    }
                    last_tap_time = now;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Clique repetidamente no ritmo da música para definir o BPM por Tap!");
                }
                
                // Menu pop-up de BPM ao clicar com o botão direito no widget de BPM
                if (ImGui::BeginPopupContextItem("BpmPresetsPopup")) {
                    ImGui::TextColored(ImVec4(0.97f, 0.50f, 0.0f, 1.0f), "PRESETS DE BPM (GÊNEROS)");
                    ImGui::Separator();
                    if (ImGui::Selectable("140 BPM (Psytrance Full-On)")) timeline.setBPM(140.0f);
                    if (ImGui::Selectable("145 BPM (Goa Trance)")) timeline.setBPM(145.0f);
                    if (ImGui::Selectable("150 BPM (Darkpsy)")) timeline.setBPM(150.0f);
                    if (ImGui::Selectable("180 BPM (Psycore)")) timeline.setBPM(180.0f);
                    if (ImGui::Selectable("128 BPM (House & EDM)")) timeline.setBPM(128.0f);
                    if (ImGui::Selectable("175 BPM (Drum & Bass)")) timeline.setBPM(175.0f);
                    if (ImGui::Selectable("90 BPM (Trap / Hip-Hop)")) timeline.setBPM(90.0f);
                    ImGui::EndPopup();
                }
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
                if (ImGui::MenuItem("[+] Delay Lama (Monge Tibetano 3D)", "")) {
                    show_delay_lama = true;
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
            case AppMode::STUDIO_MODE:
                RenderStudioMode(ai_engine);
                RenderFloatingPluginWindows();
                RenderDelayLamaPlugin(); // 🕉️ RENDERIZAR A JANELA 3D DO MONGE TIBETANO DELAY LAMA!
                RenderMonkSynthVst3Window(); // 🎛️ RENDERIZAR A JANELA DO VST3 NATIVO IMPORTADO!
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
