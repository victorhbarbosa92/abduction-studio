#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <mutex>
#include <cmath>
#include <map>
#include <fstream>
#include <filesystem>


// GLFW e ImGui
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#define NOMINMAX
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "RtAudio.h"

HWND main_hwnd = nullptr;



#include "audio/LockFreeAudioQueue.h"
#include "ai/StemSeparationEngine.h"
#include "plugin_manager/NativePlugins.h"
#include "plugin_manager/ThematicSynths.h"
#include "plugin_manager/KuroSamplerNode.h"
#include "plugin_manager/KuroClipPlayerNode.h"
#include "core/ClipManager.h"
#include "audio/SynthEngine.h"
#include "audio/LFOModulator.h"
#include "audio/KuroWave.h"
#include "audio/RecordManager.h"
#include "core/TimelineManager.h"
#include "plugin_manager/DAG.h"
#include "core/DJEngine.h"
#include "core/StemExtractorEngine.h"
#include "audio/GrossBeatNode.h"
#include "core/DawApiBridge.h"
#include "audio/MasterLimiter.h"
#include "ui/SpectrumVisualizerUI.h"
#include "ui/ChannelInstrumentManager.h"

// Globais para o StudioUI
bool is_playing = false;
unsigned long long global_sample_count = 0;
ClipManager g_clip_manager;
KuroAudio::KuroWave g_kurowave;
KuroAudio::SynthEngine g_piano_synth;
KuroAudio::MasterLimiter g_master_limiter;
KuroUI::SpectrumVisualizerUI g_spectrum_visualizer;
KuroDSP::ExpressiveLeadSynth g_lead_synth("lead");
KuroDSP::MonkSynth g_monk_synth("monk");
KuroDSP::AlienVoiceSynth g_alien_synth("alien");
KuroDSP::AnalogMonsterSynth g_analog_synth("analog");
KuroDSP::SynthwaveSynth g_synthwave_synth("synthwave");
KuroDSP::AbductionFMSynth g_fm_synth("fm");
KuroDSP::AcousticContrabassSynth g_contrabass_synth("contrabass");
KuroDSP::PsytranceRollingBassSynth g_psy_bass_synth("psy_rolling_bass");
std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;

void clear_all_synths() {
    g_piano_synth.clearNotes();
    g_kurowave.clearNotes();
    if (g_global_sampler) {
        g_global_sampler->stop();
    }
    g_lead_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
    g_monk_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
    g_alien_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
    g_analog_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
    g_synthwave_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
    g_fm_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
    g_psy_bass_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
}
KuroAudio::RecordManager g_record_manager;
KuroDSP::AudioGraph master_graph;
#include "audio/AudioEvent.h"
KuroDSP::TimelineManager timeline;

KuroDSP::GrossBeatNode g_gross_beat;
std::unique_ptr<StemSeparationEngine> g_ai_engine;
std::unique_ptr<KuroAudio::DJEngine> g_dj_engine;
std::unique_ptr<KuroAudio::StemExtractorEngine> g_stem_engine;

// ==========================================
// CALLBACKS GLFW
// ==========================================
// Removido drop_callback duplicado daqui.

// ==========================================
// ESTRUTURAS GLOBAIS
// ==========================================
float track_synth_buffer_l[MAX_TRACKS][2048] = {{0.0f}};
float track_synth_buffer_r[MAX_TRACKS][2048] = {{0.0f}};

class TrackSynthNode : public KuroDSP::PluginNode {
private:
    float* src_l;
    float* src_r;
public:
    TrackSynthNode(const std::string& id, float* sl, float* sr) 
        : PluginNode(id, "TrackSynthNode"), src_l(sl), src_r(sr) {}
        
    void process(float* left, float* right, unsigned int frames) override {
        for (unsigned int i = 0; i < frames; i++) {
            left[i] += src_l[i];
            right[i] += src_r[i];
        }
    }
    
    void setParameter(int param_index, float target_value, unsigned int frames_to_lerp = 0) override {}
};

std::string track_names[MAX_TRACKS] = {
    "Track 1", "Track 2", "Track 3", "Track 4",
    "Track 5", "Track 6", "Track 7", "Track 8",
    "Track 9", "Track 10", "Track 11", "Track 12",
    "Track 13", "Track 14", "Track 15", "Track 16",
    "Track 17", "Track 18", "Track 19", "Track 20"
};


float global_time_sec = 0.0f;
float param_filter_cutoff = 20000.0f;

LockFreeAudioQueue<AudioEvent> ui_to_audio_queue(1024);

extern "C" {
    float getGlobalTimeSec() { return global_time_sec; }
}

float track_volumes[MAX_TRACKS];
float track_pans[MAX_TRACKS] = { 0.0f };
float track_sends_A[MAX_TRACKS];
float track_sends_B[MAX_TRACKS];
bool track_mutes[MAX_TRACKS] = { false };
bool track_solos[MAX_TRACKS] = { false };
bool track_fx_bypass[MAX_TRACKS] = { false };
bool track_abyss_pitch_enabled[MAX_TRACKS] = { false };
float track_pitch_semitones[MAX_TRACKS] = { 0.0f };
float track_offsets[MAX_TRACKS] = { 0.0f };

// Linear gains para o DAG
float track_linear_volumes[MAX_TRACKS] = { 1.0f };
float track_linear_sends_A[MAX_TRACKS] = { 0.0f };
float track_linear_sends_B[MAX_TRACKS] = { 0.0f };
float g_master_volume = 0.8f;

float dummy_vol[MAX_TRACKS] = { 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f };
float dummy_pan[MAX_TRACKS] = { 0.0f };
int channel_tracks[MAX_TRACKS] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

float track_vu_levels[MAX_TRACKS] = { 0.0f };
float master_vu_level_l = 0.0f;
float master_vu_level_r = 0.0f;

namespace KuroUI {
    // Apenas instanciando as globais declaradas nos headers
    // CommandManager já está definido em ui/CommandManager.h
}

#include "ui/StudioUI.h"
#include "core/ProjectManager.h"

namespace ProjectManagerBridge {
    void Save(const std::string& path) {
        ProjectManager::SaveProject(path);
    }
    void Load(const std::string& path) {
        ProjectManager::LoadProject(path);
    }
}

namespace KuroUI {
    CommandManager g_command_manager;
}

// ==========================================
// ESTADO DSP GLOBAL (Zero Allocation in Audio Thread)
// ==========================================
KuroDSP::KuroLimiter track_limiters[4];
KuroDSP::KuroDelay track_delays[4];
KuroDSP::KuroChorus track_choruses[4];
KuroDSP::KuroReverb track_reverbs[4];


// Definição da matriz global de efeitos (DSP)
KuroDSP::Pedalboard track_pedalboards[MAX_TRACKS];
KuroDSP::LFOModulator g_lfo_modulators[4];

// Variável para forçar foco do teclado (FASE 23)
bool force_mixer_focus = false;

bool play_test_tone = false;
float global_phase = 0.0f;
// track_names foi movido para o StudioUI globals

// ==========================================
// ESTRUTURAS GLOBAIS
// AudioEvent já definido em LockFreeAudioQueue / AudioEvent.h

// ==========================================
// AUDIO CALLBACK (Lock-Free / Zero Allocation)
// ==========================================
float global_synth_l[4096];
float global_synth_r[4096];

int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames,
                  double streamTime, RtAudioStreamStatus status, void *userData)
{
    float *out = (float *)outputBuffer;
    LockFreeAudioQueue<AudioEvent>* queue = (LockFreeAudioQueue<AudioEvent>*)userData;
    
    // Ler eventos UI (sem locks)
    AudioEvent evt;
    while(queue->pop(evt)) {
        // Tratar eventos de UI aqui futuramente
    }
    
    // Limpar o buffer de saída
    for (unsigned int i = 0; i < nFrames * 2; i++) {
        out[i] = 0.0f;
    }

    bool currently_playing = is_playing;
    static bool was_playing = false;
    
    // Se a reprodução foi parada neste exato instante, limpa as notas presas da timeline
    if (was_playing && !currently_playing) {
        g_piano_synth.clearNotes();
        g_kurowave.clearNotes();
        
        // CRÍTICO: Resetar o flag is_playing das notas dos canais
        {
            std::lock_guard<std::mutex> lock(g_clip_manager.clip_mutex);
            for (auto& pat : g_clip_manager.global_patterns) {
                for (int c = 0; c < MAX_TRACKS; c++) {
                    for (auto& note : pat.getChannelNotes(c)) {
                        note.is_playing = false;
                    }
                }
            }
        }
    }
    was_playing = currently_playing;

    // --- Suavização de Volumes e Sends (Evita zipper noise, respeita Mute/Solo global) ---
    bool any_solo = false;
    for (int i = 0; i < MAX_TRACKS; i++) {
        if (track_solos[i]) { any_solo = true; break; }
    }

    for (int i = 0; i < MAX_TRACKS; i++) {
        float target_vol = 0.0f;
        bool is_muted = track_mutes[i] || (any_solo && !track_solos[i]);
        if (!is_muted) {
            target_vol = (track_volumes[i] <= -59.9f) ? 0.0f : std::pow(10.0f, track_volumes[i] / 20.0f);
            track_linear_volumes[i] = track_linear_volumes[i] * 0.90f + target_vol * 0.10f;
        } else {
            track_linear_volumes[i] = 0.0f; // Mute instantâneo absoluto
        }
        
        float target_send_A = is_muted ? 0.0f : ((track_sends_A[i] <= -59.9f) ? 0.0f : std::pow(10.0f, track_sends_A[i] / 20.0f));
        float target_send_B = is_muted ? 0.0f : ((track_sends_B[i] <= -59.9f) ? 0.0f : std::pow(10.0f, track_sends_B[i] / 20.0f));
        
        track_linear_sends_A[i] = track_linear_sends_A[i] * 0.90f + target_send_A * 0.10f;
        track_linear_sends_B[i] = track_linear_sends_B[i] * 0.90f + target_send_B * 0.10f;
    }

    // --- Resetar parâmetros ativos para os valores base ---
    for (int i = 0; i < 8; i++) {
        track_pedalboards[i].active_dark_drive = track_pedalboards[i].param_dark_drive;
        track_pedalboards[i].active_alien_freq = track_pedalboards[i].param_alien_freq;
        track_pedalboards[i].active_ritual_rate = track_pedalboards[i].param_ritual_rate;
        track_pedalboards[i].active_ritual_depth = track_pedalboards[i].param_ritual_depth;
    }

    // --- Processar Modulações por LFO (Control-Rate) ---
    float bpm = timeline.getBPM();
    for (int l = 0; l < 4; l++) {
        auto& mod = g_lfo_modulators[l];
        if (!mod.active || mod.target_track < 0) continue;
        
        mod.engine.setShape((KuroDSP::LFOShape)mod.shape);
        mod.engine.setFrequency(mod.rate);
        
        float beat_fraction = 0.25f;
        if (mod.sync_rate_idx == 0) beat_fraction = 1.0f;       // 1/1
        else if (mod.sync_rate_idx == 1) beat_fraction = 0.5f;  // 1/2
        else if (mod.sync_rate_idx == 2) beat_fraction = 0.25f; // 1/4 (default)
        else if (mod.sync_rate_idx == 3) beat_fraction = 0.125f;// 1/8
        
        mod.engine.setSync(mod.sync, beat_fraction);
        
        float lfo_val = mod.engine.process(bpm); // Output: -1.0 a +1.0
        
        auto& board = track_pedalboards[mod.target_track];
        if (mod.target_param == 0) {
            // Dark Drive (range: 1.0 a 10.0)
            float mod_range = 9.0f * mod.depth;
            float new_val = board.param_dark_drive + lfo_val * mod_range * 0.5f;
            board.active_dark_drive = std::clamp(new_val, 1.0f, 10.0f);
        } else if (mod.target_param == 1) {
            // RingMod Alien Freq (range: 20.0 a 1000.0)
            float mod_range = 980.0f * mod.depth;
            float new_val = board.param_alien_freq + lfo_val * mod_range * 0.5f;
            board.active_alien_freq = std::clamp(new_val, 20.0f, 1000.0f);
        } else if (mod.target_param == 2) {
            // Tremolo Depth (range: 0.0 a 1.0)
            float mod_range = 1.0f * mod.depth;
            float new_val = board.param_ritual_depth + lfo_val * mod_range * 0.5f;
            board.active_ritual_depth = std::clamp(new_val, 0.0f, 1.0f);
        } else if (mod.target_param == 3) {
            // Tremolo Rate (range: 0.1 a 5.0)
            float mod_range = 4.9f * mod.depth;
            float new_val = board.param_ritual_rate + lfo_val * mod_range * 0.5f;
        }
    }

    // 1. Processar a Timeline (Gera Eventos MIDI e Automação)
    unsigned int out_offset_frames = 0;
    auto [midi_events, auto_events] = timeline.processBlock(nFrames, out_offset_frames);
    
    // Roteia Eventos MIDI (Piano Roll e Step Sequencer) para os Synths correspondentes
    for (const auto& ev : midi_events) {
        int track_idx = std::get<0>(ev);
        int pitch = std::get<1>(ev);
        float duration = std::get<2>(ev);
        float velocity = std::get<3>(ev);
        
        // 1. Aciona a reprodução de samples / baterias / instrumentos reais (FLEX) no sampler multicanal universal
        if (track_idx >= 0 && track_idx < MAX_TRACKS) {
            g_piano_synth.triggerNote(pitch, duration, velocity, track_idx);
            
            // 2. Roteamento de sintetizadores dedicados direcionado por Canal
            if (track_idx == 1 && KuroUI::g_channel_slots[1].type == KuroUI::ChannelInstrumentType::KURO_RHYTHM_BASS) { // Canal 2: Bass Track -> Rolling Bass Engine (se ativo)
                KuroDSP::MpeMidiEvent ev_mpe{pitch, pitch, true, velocity, 0.0f, 0.5f, 0.5f, duration};
                g_psy_bass_synth.pushMidiEvent(ev_mpe);
            }
            else if (track_idx == 3 && (KuroUI::show_contrabass_window || KuroUI::show_delay_lama)) { // Contrabaixo acústico ou Delay Lama (se janelas ativas)
                KuroDSP::MpeMidiEvent ev_mpe{pitch, pitch, true, velocity, 0.0f, 0.5f, 0.5f, duration};
                if (KuroUI::show_contrabass_window) {
                    g_contrabass_synth.pushMidiEvent(ev_mpe);
                } else if (KuroUI::show_delay_lama) {
                    g_monk_synth.pushMidiEvent(ev_mpe);
                }
            }
        }
    }
    
    // Roteia Automação (Automation Lanes) para o master_graph (DAG)
    for (const auto& ae : auto_events) {
        auto node = master_graph.getNode(ae.target_node_id);
        if (node) {
            node->setParameter(ae.param_index, ae.value, 0); // Sem LERP aqui por enquanto, pois o param_value já é LERP na lane
        }
    }

    // 2. Sintetizador (Processa para L e R)
    std::fill_n(global_synth_l, nFrames, 0.0f);
    std::fill_n(global_synth_r, nFrames, 0.0f);
    
    // Zera os buffers de injeção dos canais do mixer
    for (int t = 0; t < MAX_TRACKS; t++) {
        std::fill_n(track_synth_buffer_l[t], nFrames, 0.0f);
        std::fill_n(track_synth_buffer_r[t], nFrames, 0.0f);
    }
    
    // Processa o Sampler Multicanal nos buffers de injeção
    float* multitrack_l[MAX_TRACKS];
    float* multitrack_r[MAX_TRACKS];
    for (int t = 0; t < MAX_TRACKS; t++) {
        multitrack_l[t] = track_synth_buffer_l[t];
        multitrack_r[t] = track_synth_buffer_r[t];
    }
    g_piano_synth.processMultitrack(multitrack_l, multitrack_r, nFrames, global_time_sec);
    
    // Processa os sintetizadores nos respectivos buffers de canal (se o FLEX/Sample estiver inativo para aquele canal)
    if (KuroUI::g_channel_slots[1].type == KuroUI::ChannelInstrumentType::KURO_RHYTHM_BASS) {
        g_psy_bass_synth.process(track_synth_buffer_l[1], track_synth_buffer_r[1], nFrames);
    }

    if (!g_piano_synth.flex_active[3]) {
        if (KuroUI::show_contrabass_window) {
            g_contrabass_synth.process(track_synth_buffer_l[3], track_synth_buffer_r[3], nFrames);
        } else if (KuroUI::show_delay_lama) {
            g_monk_synth.process(track_synth_buffer_l[3], track_synth_buffer_r[3], nFrames);
        } else {
            g_analog_synth.process(track_synth_buffer_l[3], track_synth_buffer_r[3], nFrames);
        }
    }
    if (!g_piano_synth.flex_active[4]) {
        g_synthwave_synth.process(track_synth_buffer_l[4], track_synth_buffer_r[4], nFrames);
    }
    if (!g_piano_synth.flex_active[5]) {
        g_lead_synth.process(track_synth_buffer_l[5], track_synth_buffer_r[5], nFrames);
    }
    if (!g_piano_synth.flex_active[6]) {
        g_fm_synth.process(track_synth_buffer_l[6], track_synth_buffer_r[6], nFrames);
    }
    
    // Aplica o Gross Beat nos canais de synth correspondentes (Tracks 3, 4 e 5)
    extern KuroDSP::GrossBeatNode g_gross_beat;
    if (g_gross_beat.enabled) {
        g_gross_beat.processBlock(track_synth_buffer_l[3], track_synth_buffer_r[3], nFrames);
        g_gross_beat.processBlock(track_synth_buffer_l[4], track_synth_buffer_r[4], nFrames);
        g_gross_beat.processBlock(track_synth_buffer_l[5], track_synth_buffer_r[5], nFrames);
    }
    
    // Processa o Grafo Principal (que vai acumular as injeções e somar no Master)
    master_graph.process(global_synth_l, global_synth_r, nFrames);

    // Mixa o Motor DJ Nativo (Deck A, Deck B e Sampler Slots)
    if (g_dj_engine) {
        g_dj_engine->process(global_synth_l, global_synth_r, nFrames);
    }
    
    // 3. Efeitos Nativos do Master Bus e Mixagem Final
    float* out_buffer = (float*)outputBuffer;
    
    for (unsigned int i = 0; i < nFrames; i++) {
        float left = global_synth_l[i];
        float right = global_synth_r[i];

        if (track_limiters[0].enabled) track_limiters[0].process(&left, &right, 1);
        if (track_delays[0].enabled) track_delays[0].process(&left, &right, 1);
        if (track_choruses[0].enabled) track_choruses[0].process(&left, &right, 1);
        if (track_reverbs[0].enabled) track_reverbs[0].process(&left, &right, 1);

        // Aplica o volume master final
        left *= g_master_volume;
        right *= g_master_volume;

        // Processa pelo Master Limiter de pico
        g_master_limiter.processBlock(&left, &right, 1);

        // Alimenta o Spectrum Visualizer em tempo real
        g_spectrum_visualizer.pushAudioSamples(left, right);

        out_buffer[i * 2] = left;     // L
        out_buffer[i * 2 + 1] = right; // R
    }

    // Calcula níveis de VU reais para o Master e a trilha 5 (Synth)
    float peak_l = 0.0f;
    float peak_r = 0.0f;
    float peak_s = 0.0f;
    for (unsigned int i = 0; i < nFrames; i++) {
        float al = std::abs(out_buffer[i * 2]);
        float ar = std::abs(out_buffer[i * 2 + 1]);
        if (al > peak_l) peak_l = al;
        if (ar > peak_r) peak_r = ar;

        float val = (std::abs(global_synth_l[i]) + std::abs(global_synth_r[i])) * 0.5f;
        if (val > peak_s) peak_s = val;
    }
    master_vu_level_l = master_vu_level_l * 0.8f + peak_l * 0.2f;
    master_vu_level_r = master_vu_level_r * 0.8f + peak_r * 0.2f;
    track_vu_levels[5] = track_vu_levels[5] * 0.8f + peak_s * 0.2f;

    // Gravação Master (Bounce)
    g_record_manager.processOutput(global_synth_l, global_synth_r, nFrames);

    global_sample_count += nFrames;
    return 0;
}

static void glfw_error_callback(int error, const char* description) {
    std::ofstream err("crash_log.txt", std::ios::app);
    err << "[GLFW ERROR] " << error << ": " << description << "\n";
    err.flush();
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Callback real para Drag and Drop de arquivos do Windows
void drop_callback(GLFWwindow* window, int count, const char** paths)
{
    if (count > 0) {
        std::string file_path = paths[0];
        
        // Se for wav e existir o sampler, tenta carregar nele
        if (g_global_sampler && 
           (file_path.substr(file_path.find_last_of(".") + 1) == "wav" || 
            file_path.substr(file_path.find_last_of(".") + 1) == "WAV")) {
            g_global_sampler->loadSample(file_path);
        }
        
        // Também envia pra Stem Separation (comportamento original)
        if (g_ai_engine && !g_ai_engine->isRunning()) {
            g_ai_engine->startProcessing(paths[0]);
        }
    }
}

// Configura o Tema ImGui
void setupGothicTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.06f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.60f, 0.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.00f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.16f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 1.00f, 0.40f, 0.30f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.20f, 1.00f); 
    
    style.WindowRounding = 0.0f; 
    style.FrameRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
}

void setupAbductionTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Paleta Deep Blue / Slate / Neon Cyan (Mockup Aprovado)
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.06f, 0.09f, 1.00f);      // #0A0F17 Deep Space Blue
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.09f, 0.13f, 1.00f);       // #101722 Card Panel
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.09f, 0.13f, 0.98f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.10f, 0.16f, 0.23f, 0.80f);        // #18283A Crisp Border
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.11f, 0.16f, 1.00f);       // #121B27
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.16f, 0.23f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.85f, 1.00f, 0.35f);  // Cyan Focus
    
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.08f, 0.11f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.12f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.04f, 0.06f, 0.09f, 1.00f);
    
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.09f, 0.13f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.04f, 0.06f, 0.09f, 0.60f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.12f, 0.18f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.16f, 0.24f, 0.34f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 0.85f, 1.00f, 0.80f);
    
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.85f, 1.00f, 1.00f);      // Cyan
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.85f, 1.00f, 1.00f);     // Cyan Grabber
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.95f, 1.00f, 1.00f);
    
    style.Colors[ImGuiCol_Button] = ImVec4(0.08f, 0.12f, 0.18f, 1.00f);        // Dark Slate Button
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.14f, 0.21f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.85f, 1.00f, 0.90f);   // Neon Cyan Click
    
    style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.16f, 0.23f, 0.80f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.14f, 0.22f, 0.32f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.85f, 1.00f, 0.40f);
    
    style.Colors[ImGuiCol_Separator] = ImVec4(0.10f, 0.16f, 0.23f, 0.70f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.00f, 0.85f, 1.00f, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.85f, 1.00f, 1.00f);
    
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.10f, 0.16f, 0.23f, 0.50f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.85f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.85f, 1.00f, 1.00f);
    
    style.Colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.10f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.12f, 0.18f, 0.26f, 1.00f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.85f, 1.00f, 0.90f);      // Cyan Active Tab
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.08f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.13f, 0.19f, 1.00f);
    
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.85f, 1.00f, 1.00f);  // Cyan Meters
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.30f, 0.95f, 1.00f, 1.00f);
    
    style.Colors[ImGuiCol_Text] = ImVec4(0.92f, 0.95f, 0.98f, 1.00f);          // Clean White/Silver Text
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.48f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.85f, 1.00f, 0.35f);
    
    // Geometria & Arredondamentos Modernos
    style.WindowRounding = 6.0f; 
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(6.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
}

void setupFLStudioTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Exact colors from Image 1
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f); 
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.10f, 0.11f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f); 
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.16f, 0.18f, 1.00f); 
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.28f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.16f, 0.18f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.20f, 0.22f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.18f, 0.19f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.27f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.32f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.70f, 0.72f, 0.75f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.46f, 0.85f, 0.44f, 1.00f); // Bright FL Green
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.46f, 0.85f, 0.44f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.95f, 0.54f, 1.00f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.16f, 0.18f, 1.00f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.46f, 0.85f, 0.44f, 0.50f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.25f, 0.26f, 0.28f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.36f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.42f, 0.45f, 1.00f);

    // Padding & Borders
    style.WindowPadding = ImVec2(4.0f, 4.0f);
    style.FramePadding = ImVec2(2.0f, 2.0f);
    style.ItemSpacing = ImVec2(4.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(2.0f, 2.0f);
    
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;

    // Rounding
    style.WindowRounding = 0.0f; 
    style.ChildRounding = 0.0f;
    style.FrameRounding = 1.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 1.0f;
    style.TabRounding = 1.0f;
}

void setupAbletonTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.85f, 0.55f, 0.20f, 1.00f); // Ableton orange
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.85f, 0.55f, 0.20f, 1.00f);
    style.WindowRounding = 0.0f; 
    style.FrameRounding = 0.0f;
}

int main(int argc, char* argv[]) {

    KuroUtils::Log("Abduction Studio V2 Iniciado.");
    
    // Inicializa a integração de APIs do FL Studio e Ableton Live (porta UDP 9000)
    DawApiBridge::startBridge();

    // Garante que todos os sintetizadores iniciem 100% silenciados e sem nenhuma nota
    clear_all_synths();
    g_clip_manager.reset();
    timeline.setMasterFrame(0);
    timeline.setPlaying(false);
    is_playing = false;

    // Sincroniza volumes das faixas com os faders da interface gráfica no início
    float fader_vals_init[9] = {0.80f, 0.75f, 0.75f, 0.70f, 0.75f, 0.70f, 0.72f, 0.68f, 0.70f};
    g_master_volume = fader_vals_init[0];
    for (int i = 0; i < MAX_TRACKS; i++) {
        float fval = (i < 8) ? fader_vals_init[i + 1] : 0.75f;
        track_volumes[i] = (fval <= 0.001f) ? -60.0f : 20.0f * std::log10(fval);
        track_linear_volumes[i] = fval;
    }

    // Instancia Engine de IA após o carregamento das DLLs do SO
    g_ai_engine = std::make_unique<StemSeparationEngine>();
    g_dj_engine = std::make_unique<KuroAudio::DJEngine>();
    KuroUI::g_dj_studio_ui.init(g_dj_engine.get());
    g_stem_engine = std::make_unique<KuroAudio::StemExtractorEngine>();
    
    // Link ClipManager to Timeline
    timeline.setClipManager(&g_clip_manager);
    
    // Inicializar Grafo Global
    auto master_bus = std::make_shared<KuroDSP::RackNode>("Master", "Master Bus");
    master_graph.addNode("Master", master_bus);

    auto return_a = std::make_shared<KuroDSP::ReverbNode>("Return_A", "Return A (Reverb)");
    auto return_b = std::make_shared<KuroDSP::DelayNode>("Return_B", "Return B (Delay)");
    master_graph.addNode("Return_A", return_a);
    master_graph.addNode("Return_B", return_b);
    master_graph.connect("Return_A", "Master");
    master_graph.connect("Return_B", "Master");

    master_graph.addNode("pedalboard", std::make_shared<KuroDSP::RackNode>("pedalboard", "My Pedalboard"));
    master_graph.connect("pedalboard", "Master");
    
    // Sampler Global
    g_global_sampler = std::make_shared<KuroDSP::KuroSamplerNode>("main_sampler", "Psy Sampler");
    master_graph.addNode("main_sampler", g_global_sampler);
    master_graph.connect("main_sampler", "Master");

    // Instanciar os Clip Players (1 por faixa) mapeando os buffers extraídos pela IA
    for (int i = 0; i < MAX_TRACKS; i++) {
        auto clip_player = std::make_shared<KuroDSP::KuroClipPlayerNode>(
            "ClipPlayer_" + std::to_string(i), 
            "Clip Player " + std::to_string(i), 
            &timeline, 
            &g_clip_manager, 
            i, 
            &g_ai_engine->getStemBuffer(i), 
            44100.0f
        );
        master_graph.addNode("ClipPlayer_" + std::to_string(i), clip_player);

        // Adiciona o RackNode para a pedaleira do canal i
        auto track_pedal = std::make_shared<KuroDSP::RackNode>(
            "TrackPedalboard_" + std::to_string(i),
            "Track Pedalboard " + std::to_string(i),
            &track_pedalboards[i]
        );
        master_graph.addNode("TrackPedalboard_" + std::to_string(i), track_pedal);

        // Adiciona o TrackSynthNode para injetar som dos synths/bateria no canal i
        auto synth_inject = std::make_shared<TrackSynthNode>(
            "SynthInject_" + std::to_string(i),
            track_synth_buffer_l[i],
            track_synth_buffer_r[i]
        );
        master_graph.addNode("SynthInject_" + std::to_string(i), synth_inject);
        
        auto track_out = std::make_shared<KuroDSP::BusNode>("TrackOut_" + std::to_string(i), "TrackOut " + std::to_string(i), &track_volumes[i]);
        master_graph.addNode("TrackOut_" + std::to_string(i), track_out);
        
        // Conexões do fluxo:
        // Clipes e Injeção -> Pedalboard (Efeitos) -> Saída do Canal
        master_graph.connect("ClipPlayer_" + std::to_string(i), "TrackPedalboard_" + std::to_string(i));
        master_graph.connect("SynthInject_" + std::to_string(i), "TrackPedalboard_" + std::to_string(i));
        master_graph.connect("TrackPedalboard_" + std::to_string(i), "TrackOut_" + std::to_string(i));
        
        master_graph.connect("TrackOut_" + std::to_string(i), "Master", &track_linear_volumes[i]);
        master_graph.connect("TrackOut_" + std::to_string(i), "Return_A", &track_linear_sends_A[i]);
        master_graph.connect("TrackOut_" + std::to_string(i), "Return_B", &track_linear_sends_B[i]);
    }

    // 1. Setup Audio Engine (RtAudio WASAPI com Auto-detecção de Taxa e Dispositivo)
    RtAudio adc(RtAudio::WINDOWS_WASAPI);
    LockFreeAudioQueue<AudioEvent> ui_to_audio_queue(1024);
    bool audio_stream_started = false;
    unsigned int actual_sample_rate = 48000;

    unsigned int numDevices = adc.getDeviceCount();
    KuroUtils::Log("[Audio Engine] Dispositivos WASAPI encontrados: " + std::to_string(numDevices));

    if (numDevices >= 1) {
        unsigned int defaultOut = adc.getDefaultOutputDevice();
        std::vector<unsigned int> candidate_devices;
        candidate_devices.push_back(defaultOut);
        for (unsigned int d = 0; d < numDevices; d++) {
            if (d != defaultOut) candidate_devices.push_back(d);
        }

        for (unsigned int devId : candidate_devices) {
            RtAudio::DeviceInfo info;
            try { info = adc.getDeviceInfo(devId); } catch (...) { continue; }
            if (info.outputChannels < 2) continue;

            KuroUtils::Log("[Audio Engine] Testando dispositivo " + std::to_string(devId) + ": " + info.name);

            std::vector<unsigned int> candidate_rates;
            if (info.preferredSampleRate > 0) {
                candidate_rates.push_back(info.preferredSampleRate);
            }
            if (std::find(candidate_rates.begin(), candidate_rates.end(), 48000) == candidate_rates.end()) {
                candidate_rates.push_back(48000);
            }
            if (std::find(candidate_rates.begin(), candidate_rates.end(), 44100) == candidate_rates.end()) {
                candidate_rates.push_back(44100);
            }
            if (std::find(candidate_rates.begin(), candidate_rates.end(), 96000) == candidate_rates.end()) {
                candidate_rates.push_back(96000);
            }

            for (unsigned int sr : candidate_rates) {
                RtAudio::StreamParameters parameters;
                parameters.deviceId = devId;
                parameters.nChannels = 2; // Stereo
                parameters.firstChannel = 0;
                unsigned int bufferFrames = 256;

                try {
                    adc.openStream(&parameters, NULL, RTAUDIO_FLOAT32, sr, &bufferFrames, &audioCallback, &ui_to_audio_queue);
                    adc.startStream();
                    audio_stream_started = true;
                    actual_sample_rate = sr;
                    if (g_dj_engine) {
                        g_dj_engine->host_sample_rate = (double)sr;
                    }
                    KuroUtils::Log("[Audio Engine] SUCESSO: WASAPI iniciado em " + info.name + " (" + std::to_string(sr) + " Hz, " + std::to_string(bufferFrames) + " frames)");
                    std::cout << "[SYSTEM] WASAPI Audio Engine iniciado (" << sr << "Hz, " << bufferFrames << " buffers).\n";
                    break;
                } catch (const std::exception& ex) {
                    KuroUtils::Log("[Audio Engine] Falha ao abrir dispositivo " + std::to_string(devId) + " com " + std::to_string(sr) + " Hz: " + ex.what());
                } catch (...) {
                    KuroUtils::Log("[Audio Engine] Falha desconhecida ao abrir dispositivo " + std::to_string(devId) + " com " + std::to_string(sr) + " Hz");
                }
            }

            if (audio_stream_started) break;
        }
    }

    if (!audio_stream_started) {
        KuroUtils::Log("[Audio Engine] ALERTA: Nenhum dispositivo WASAPI pôde ser iniciado! Áudio estará mudo.");
    }

    // 2. Setup ImGui / GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::ofstream err("crash_log.txt", std::ios::app);
        err << "[FATAL] glfwInit failed!\n";
        err.flush();
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // Habilitar borda e barra do Windows
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Abduction Studio V4.0 - Flagship Edition", NULL, NULL);
    if (window == NULL) {
        std::ofstream err("crash_log.txt", std::ios::app);
        err << "[FATAL] glfwCreateWindow failed!\n";
        err.flush();
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwShowWindow(window);
    glfwSwapInterval(1); // Enable vsync
    
    // FASE 21: Drag & Drop Callback
    glfwSetDropCallback(window, drop_callback);
    main_hwnd = glfwGetWin32Window(window);
    if (main_hwnd) {
        ShowWindow(main_hwnd, SW_SHOWDEFAULT);
        BringWindowToTop(main_hwnd);
        SetForegroundWindow(main_hwnd);
    }
    glfwSwapInterval(1); // Enable vsync (60 FPS)

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Desativado para manter renderização unificada na janela principal
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Habilita sistema de Docking (Ideia 3)
    io.ConfigDebugHighlightIdConflicts = false;         // Desativa popups de debug em build de produção
    
    // Load Fonts
    auto findResourcePath = [](const std::string& path) -> std::string {
        if (std::filesystem::exists(path)) return path;
        std::string p2 = "../" + path;
        if (std::filesystem::exists(p2)) return p2;
        std::string p3 = "../../" + path;
        if (std::filesystem::exists(p3)) return p3;
        return path;
    };

    ImFontConfig font_cfg;
    font_cfg.OversampleH = 2;
    font_cfg.OversampleV = 2;
    io.Fonts->AddFontFromFileTTF(findResourcePath("src/ui/Roboto-Regular.ttf").c_str(), 15.0f, &font_cfg);
    
    // Merge FontAwesome
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = 14.0f; // Fix icon size
    static const ImWchar icons_ranges[] = { 0xe000, 0xf8ff, 0 }; // Basic FontAwesome range
    io.Fonts->AddFontFromFileTTF(findResourcePath("src/ui/fa-solid-900.ttf").c_str(), 14.0f, &icons_config, icons_ranges);
    
    setupFLStudioTheme(); // Tema FL Studio (Fase 31)

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Conectar DSP do Rolling Bass à UI
    KuroUI::g_psy_rolling_bass_ui.setSynthDsp(&g_psy_bass_synth);

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--open-dj" || arg == "--dj") {
            KuroUI::show_dj_modal = true;
        } else if (arg == "--test-phase3-dj" || arg == "--test-deck-play") {
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1894735727.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_title = "Cliffjumper";
                    g_dj_engine->deckA.track_artist = "Aura Vortex & Klipsun";
                    g_dj_engine->deckA.bpm = 138.0;
                    g_dj_engine->deckA.key_signature = "9B / G";
                    g_dj_engine->deckA.togglePlay();
                }

                std::string pB = "scratch/test_astrix.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/tracks/track_demo_djw.wav";
                if (g_dj_engine->deckB.loadTrack(pB)) {
                    g_dj_engine->deckB.track_title = "Deep Jungle Walk";
                    g_dj_engine->deckB.track_artist = "Astrix";
                    g_dj_engine->deckB.bpm = 138.0;
                    g_dj_engine->deckB.key_signature = "8A / Am";
                    g_dj_engine->toggleSync(1, 0);
                    g_dj_engine->deckB.togglePlay(&g_dj_engine->deckA);
                }
            }
        } else if (arg == "--test-beat-sync") {
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1894735727.wav";
                g_dj_engine->deckA.loadTrack(pA);
                g_dj_engine->deckA.track_title = "Cliffjumper";
                g_dj_engine->deckA.track_artist = "Aura Vortex & Klipsun";
                g_dj_engine->deckA.bpm = 138.0;
                g_dj_engine->deckA.key_signature = "9B / G";
                g_dj_engine->deckA.togglePlay();

                std::string pB = "scratch/test_astrix.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/tracks/track_demo_djw.wav";
                g_dj_engine->deckB.loadTrack(pB);
                g_dj_engine->deckB.track_title = "Deep Jungle Walk";
                g_dj_engine->deckB.track_artist = "Astrix";
                g_dj_engine->deckB.bpm = 138.0;
                g_dj_engine->deckB.key_signature = "8A / Am";

                // Ativa Pioneer Beat Sync: Deck B trancado ao Deck A
                g_dj_engine->toggleSync(1, 0);
                g_dj_engine->deckB.togglePlay(&g_dj_engine->deckA);
            }
        } else if (arg == "--test-usb-export") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_export_modal = true;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1659092811.wav";
                g_dj_engine->deckA.loadTrack(pA);
                g_dj_engine->deckA.track_title = "Cliffhanger";
                g_dj_engine->deckA.track_artist = "Revoluo / Discip";
                g_dj_engine->deckA.hot_cues[0].active = true;
                g_dj_engine->deckA.hot_cues[0].time_sec = 15.0;

                std::string pB = "scratch/test_astrix.wav";
                g_dj_engine->deckB.loadTrack(pB);
                g_dj_engine->deckB.track_title = "Deep Jungle Walk";
                g_dj_engine->deckB.track_artist = "Astrix";
                g_dj_engine->deckB.hot_cues[0].active = true;
                g_dj_engine->deckB.hot_cues[0].time_sec = 10.0;
            }
        } else if (arg == "--test-dj-crates" || arg == "--test-crates") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_playlist_drawer = true;
            KuroAudio::DJLibraryManager::getInstance().scanLocalTracks("scratch/tracks");
            KuroAudio::DJLibraryManager::getInstance().setActiveFolderId("prog_psy");
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1894735727.wav";
                g_dj_engine->deckA.loadTrack(pA);
                g_dj_engine->deckA.track_title = "Cliffjumper";
                g_dj_engine->deckA.track_artist = "Aura Vortex & Klipsun";
                g_dj_engine->deckA.bpm = 138.0;
                g_dj_engine->deckA.key_signature = "9B / G";

                std::string pB = "scratch/tracks/track_demo_djw.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/test_astrix.wav";
                g_dj_engine->deckB.loadTrack(pB);
                g_dj_engine->deckB.track_title = "Deep Jungle Walk";
                g_dj_engine->deckB.track_artist = "Astrix";
                g_dj_engine->deckB.bpm = 138.0;
                g_dj_engine->deckB.key_signature = "8A / Am";
            }
        } else if (arg == "--test-search-modal") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.focus_search_modal = true;
            if (std::filesystem::exists("scratch/tracks/search_results.tsv")) {
                KuroUI::g_dj_studio_ui.spotify_service.parseSearchResultsTsv("scratch/tracks/search_results.tsv");
            }
            if (g_dj_engine) {
                std::string pA = "scratch/tracks/track_1894735727.wav";
                if (std::filesystem::exists(pA)) {
                    g_dj_engine->deckA.loadTrack(pA);
                    g_dj_engine->deckA.track_title = "Cliffjumper";
                    g_dj_engine->deckA.track_artist = "Aura Vortex & Klipsun";
                    g_dj_engine->deckA.bpm = 138.0;
                    g_dj_engine->deckA.key_signature = "9B / G";
                }
                std::string pB = "scratch/tracks/track_demo_djw.wav";
                if (std::filesystem::exists(pB)) {
                    g_dj_engine->deckB.loadTrack(pB);
                    g_dj_engine->deckB.track_title = "Deep Jungle Walk";
                    g_dj_engine->deckB.track_artist = "Astrix";
                    g_dj_engine->deckB.bpm = 138.0;
                    g_dj_engine->deckB.key_signature = "8A / Am";
                }
            }
        } else if (arg == "--run-export-test") {
            std::filesystem::create_directories("scratch/tracks/export");
            std::string status;
            std::vector<KuroAudio::SpotifyDJTrack> tracks;
            KuroAudio::SpotifyDJTrack t1;
            t1.title = "Cliffhanger";
            t1.artist = "Revoluo";
            t1.bpm = 128.0;
            t1.key = "8A";
            t1.duration_sec = 240.0;
            t1.local_wav_path = "scratch/test_cliff.wav";
            tracks.push_back(t1);

            KuroAudio::DJDeck test_decks[4];
            test_decks[0].track_path = t1.local_wav_path;
            test_decks[0].hot_cues[0].active = true;
            test_decks[0].hot_cues[0].time_sec = 16.0;

            bool ok1 = KuroAudio::DJExportService::exportRekordboxPackage("scratch/tracks/export", tracks, test_decks, status);
            bool ok2 = KuroAudio::DJExportService::exportUniversalPackage("scratch/tracks/export", tracks, test_decks, status);
            std::cout << "[EXPORT_TEST] Rekordbox XML: " << (ok1 ? "OK" : "FAIL") << " | Universal M3U8: " << (ok2 ? "OK" : "FAIL") << "\n";
            exit((ok1 && ok2) ? 0 : 1);
        } else if (arg == "--open-psy-bass") {
            KuroUI::g_psy_rolling_bass_ui.is_open = true;
            KuroUI::g_psy_rolling_bass_ui.need_focus = true;
        } else if (arg == "--psy-bass-virus-a1") {
            KuroUI::g_psy_rolling_bass_ui.is_open = true;
            KuroUI::g_psy_rolling_bass_ui.need_focus = true;
            KuroUI::g_psy_rolling_bass_ui.current_synth_model = 1; // Virus TI Hyper-Saw
            KuroUI::g_psy_rolling_bass_ui.current_root_idx = 9;   // A1
            KuroUI::g_psy_rolling_bass_ui.current_scale_idx = 1;  // Natural Minor
            if (KuroUI::g_psy_rolling_bass_ui.synth_dsp) {
                KuroUI::g_psy_rolling_bass_ui.synth_dsp->setSynthModel(1);
            }
        } else if (arg == "--psy-bass-audition") {
            KuroUI::g_psy_rolling_bass_ui.is_open = true;
            KuroUI::g_psy_rolling_bass_ui.need_focus = true;
            KuroUI::g_psy_rolling_bass_ui.current_synth_model = 1; // Virus TI Hyper-Saw
            KuroUI::g_psy_rolling_bass_ui.current_root_idx = 9;   // A1
            KuroUI::g_psy_rolling_bass_ui.current_scale_idx = 1;  // Natural Minor
            KuroUI::g_psy_rolling_bass_ui.is_auditioning = true;
            if (KuroUI::g_psy_rolling_bass_ui.synth_dsp) {
                KuroUI::g_psy_rolling_bass_ui.synth_dsp->setSynthModel(1);
            }
        } else if (arg == "--psy-bass-generate-pr") {
            KuroUI::g_psy_rolling_bass_ui.current_synth_model = 1;
            KuroUI::g_psy_rolling_bass_ui.current_root_idx = 9;
            KuroUI::g_psy_rolling_bass_ui.current_scale_idx = 1;
            auto gen_notes = KuroUI::g_psy_rolling_bass_ui.generateMidiNotes(140.0f, 4);
            if (!g_clip_manager.global_patterns.empty()) {
                g_clip_manager.global_patterns[0].getChannelNotes(1) = gen_notes;
            }
            KuroUI::show_piano_roll = true;
            KuroUI::g_need_focus_piano_roll = true;
        } else if (arg == "--psy-rhythm-full") {
            KuroUI::g_psy_rolling_bass_ui.is_open = true;
            KuroUI::g_psy_rolling_bass_ui.need_focus = true;
            KuroUI::g_psy_rolling_bass_ui.current_layer_tab = KuroUI::KuroPsytranceRollingBassUI::TAB_FULL_BASE;
            KuroUI::g_psy_rolling_bass_ui.current_root_idx = 9;
        } else if (arg == "--psy-rhythm-kick") {
            KuroUI::g_psy_rolling_bass_ui.is_open = true;
            KuroUI::g_psy_rolling_bass_ui.need_focus = true;
            KuroUI::g_psy_rolling_bass_ui.current_layer_tab = KuroUI::KuroPsytranceRollingBassUI::TAB_KICK;
            KuroUI::g_psy_rolling_bass_ui.current_root_idx = 9;
        } else if (arg == "--psy-rhythm-snare") {
            KuroUI::g_psy_rolling_bass_ui.is_open = true;
            KuroUI::g_psy_rolling_bass_ui.need_focus = true;
            KuroUI::g_psy_rolling_bass_ui.current_layer_tab = KuroUI::KuroPsytranceRollingBassUI::TAB_SNARE;
            KuroUI::g_psy_rolling_bass_ui.current_root_idx = 9;
        } else if (arg == "--psy-rhythm-audition") {
            KuroUI::g_psy_rolling_bass_ui.is_open = true;
            KuroUI::g_psy_rolling_bass_ui.need_focus = true;
            KuroUI::g_psy_rolling_bass_ui.current_layer_tab = KuroUI::KuroPsytranceRollingBassUI::TAB_FULL_BASE;
            KuroUI::g_psy_rolling_bass_ui.is_auditioning = true;
            KuroUI::g_psy_rolling_bass_ui.current_root_idx = 9;
        } else if (arg == "--psy-rhythm-generate-all") {
            KuroUI::g_psy_rolling_bass_ui.current_root_idx = 9;
            auto kick_notes = KuroUI::g_psy_rolling_bass_ui.generateKickNotes(140.0f, 4);
            auto bass_notes = KuroUI::g_psy_rolling_bass_ui.generateBassNotes(140.0f, 4);
            auto snare_notes = KuroUI::g_psy_rolling_bass_ui.generateSnareNotes(140.0f, 4);
            if (!g_clip_manager.global_patterns.empty()) {
                g_clip_manager.global_patterns[0].getChannelNotes(0) = kick_notes;
                g_clip_manager.global_patterns[0].getChannelNotes(1) = bass_notes;
                g_clip_manager.global_patterns[0].getChannelNotes(2) = snare_notes;
            }
            KuroUI::show_piano_roll = true;
            KuroUI::g_need_focus_piano_roll = true;
        } else if (arg == "--psy-rhythm-channel-rack") {
            KuroUI::g_psy_rolling_bass_ui.current_root_idx = 9;
            auto kick_notes = KuroUI::g_psy_rolling_bass_ui.generateKickNotes(140.0f, 4);
            auto bass_notes = KuroUI::g_psy_rolling_bass_ui.generateBassNotes(140.0f, 4);
            auto snare_notes = KuroUI::g_psy_rolling_bass_ui.generateSnareNotes(140.0f, 4);
            if (!g_clip_manager.global_patterns.empty()) {
                g_clip_manager.global_patterns[0].getChannelNotes(0) = kick_notes;
                g_clip_manager.global_patterns[0].getChannelNotes(1) = bass_notes;
                g_clip_manager.global_patterns[0].getChannelNotes(2) = snare_notes;
            }
            KuroUI::show_step_sequencer = true;
            KuroUI::g_need_focus_step_sequencer = true;
        } else if (arg == "--test-cr-menu") {
            KuroUI::show_step_sequencer = true;
            KuroUI::g_need_focus_step_sequencer = true;
            KuroUI::g_force_open_popup_ch = 0; // Abre popup da Track 1 (Kick Drum)
        } else if (arg == "--test-cr-1click-open") {
            KuroUI::show_step_sequencer = true;
            KuroUI::g_need_focus_step_sequencer = true;
            KuroUI::TriggerOpenInstrument(1); // Abre interface do instrumento da Track 2 (Rolling Bass)
        } else if (arg == "--test-pr-playing") {
            KuroUI::g_psy_rolling_bass_ui.current_root_idx = 9;
            auto kick_notes = KuroUI::g_psy_rolling_bass_ui.generateKickNotes(140.0f, 4);
            auto bass_notes = KuroUI::g_psy_rolling_bass_ui.generateBassNotes(140.0f, 4);
            auto snare_notes = KuroUI::g_psy_rolling_bass_ui.generateSnareNotes(140.0f, 4);
            if (!g_clip_manager.global_patterns.empty()) {
                g_clip_manager.global_patterns[0].getChannelNotes(0) = kick_notes;
                g_clip_manager.global_patterns[0].getChannelNotes(1) = bass_notes;
                g_clip_manager.global_patterns[0].getChannelNotes(2) = snare_notes;
            }
            KuroUI::show_piano_roll = true;
            KuroUI::g_need_focus_piano_roll = true;
            is_playing = true;
            timeline.setPlaying(true);
        } else if (arg == "--test-pr-sampler") {
            KuroUI::show_piano_roll = true;
            KuroUI::g_need_focus_piano_roll = true;
            KuroUI::active_sampler_channel = 0;
            KuroUI::show_sampler_settings = true;
        } else if (arg == "--test-snap-hint") {
            KuroUI::g_forced_hint_text = "Resolucao do Snap / Grade de Alinhamento (Linhas de Grade no Sequenciador e Playlist)";
        } else if (arg == "--test-piano-playlist-sync") {
            KuroUI::show_piano_roll = true;
            KuroUI::g_need_focus_piano_roll = true;
            KuroUI::show_playlist = true;
            if (!g_clip_manager.global_patterns.empty()) {
                auto& p_notes = g_clip_manager.global_patterns[0].getChannelNotes(0);
                p_notes.clear();
                p_notes.push_back(KuroDSP::MidiNote(36, 0.0f, 0.25f, 0.85f)); // C2
                p_notes.push_back(KuroDSP::MidiNote(33, 1.0f, 0.25f, 0.85f)); // A1
                p_notes.push_back(KuroDSP::MidiNote(34, 1.5f, 0.25f, 0.85f)); // A#1
            }
        } else if (arg == "--test-engine-refinements") {
            // 1. Swing Groove
            timeline.setSwing(0.35f);
            
            // 2. Pattern notes for Kick (Ch 0) and Bass (Ch 1)
            if (!g_clip_manager.global_patterns.empty()) {
                auto& kick_notes = g_clip_manager.global_patterns[0].getChannelNotes(0);
                kick_notes.clear();
                kick_notes.push_back(KuroDSP::MidiNote(36, 0.0f, 0.25f, 0.90f)); // C2 Kick
                kick_notes.push_back(KuroDSP::MidiNote(36, 0.5f, 0.25f, 0.90f));
                kick_notes.push_back(KuroDSP::MidiNote(36, 1.0f, 0.25f, 0.90f));
                kick_notes.push_back(KuroDSP::MidiNote(36, 1.5f, 0.25f, 0.90f));

                auto& bass_notes = g_clip_manager.global_patterns[0].getChannelNotes(1);
                bass_notes.clear();
                bass_notes.push_back(KuroDSP::MidiNote(33, 0.125f, 0.12f, 0.85f)); // A1 16th
                bass_notes.push_back(KuroDSP::MidiNote(33, 0.250f, 0.12f, 0.85f));
                bass_notes.push_back(KuroDSP::MidiNote(33, 0.375f, 0.12f, 0.85f));
                bass_notes.push_back(KuroDSP::MidiNote(33, 0.625f, 0.12f, 0.85f));
                bass_notes.push_back(KuroDSP::MidiNote(33, 0.750f, 0.12f, 0.85f));
                bass_notes.push_back(KuroDSP::MidiNote(33, 0.875f, 0.12f, 0.85f));
            }

            // 3. Test .kuro persistence roundtrip
            ProjectManager::SaveProject("test_refinements.kuro");
            ProjectManager::LoadProject("test_refinements.kuro");

            // 4. Focus Piano Roll on Channel 1 (Bass) with Ghost Notes on
            KuroUI::selected_track_idx = 1;
            KuroUI::show_piano_roll = true;
            KuroUI::g_need_focus_piano_roll = true;
            KuroUI::show_step_sequencer = true;
            KuroUI::g_piano_roll_show_ghost_notes = true;
            std::cout << "[TEST_REFINEMENTS] Persistence, Ghost Notes & Swing verified successfully!\n";
        } else if (arg == "--test-song-arranger") {
            KuroArranger::PsyArrangerConfig cfg;
            cfg.subgenre = 1; // Full-On Psy
            cfg.bpm = 142.0f;
            cfg.root_note = 30; // F#1
            cfg.enable_filter_sweep = true;
            cfg.enable_reverb_washout = true;
            cfg.enable_pitch_riser = true;
            cfg.use_pryzma_samples = true;

            KuroArranger::PsySongArranger::GenerateArrangement(cfg, g_clip_manager, ::timeline);

            // Test .kuro persistence roundtrip: save, clear, and reload
            ProjectManager::SaveProject("test_arranger_project.kuro");
            g_clip_manager.reset();
            timeline.clearSectionMarkers();
            ProjectManager::LoadProject("test_arranger_project.kuro");

            KuroUI::show_playlist = true;
            KuroUI::show_piano_roll = false;
            KuroUI::show_step_sequencer = false;

            std::cout << "[TEST_SONG_ARRANGER] Full Psytrance Song Arrangement Generated & Reloaded successfully!\n";
            int total_auto = 0;
            for (int t = 0; t < MAX_TRACKS; t++) total_auto += (int)g_clip_manager.track_auto_clips[t].size();
            std::cout << "[TEST_SONG_ARRANGER] Automation clips: " << total_auto << "\n";
            std::cout << "[TEST_SONG_ARRANGER] Timeline markers: " << timeline.section_markers.size() << "\n";
            std::cout << "[TEST_SONG_ARRANGER] Track 0 (Kick) clips: " << g_clip_manager.track_midi_clips[0].size() << "\n";
        } else if (arg == "--test-song-arranger-modal") {
            KuroUI::show_playlist = true;
            KuroUI::show_psy_arranger_modal = true;
            KuroUI::show_piano_roll = false;
            KuroUI::show_step_sequencer = false;
            std::cout << "[TEST_SONG_ARRANGER] Opening Psytrance Song Arranger Modal Popup!\n";
        } else if (arg == "--test-playlist-bezier") {
            KuroArranger::PsyArrangerConfig cfg;
            cfg.subgenre = 1;
            cfg.bpm = 142.0f;
            cfg.root_note = 30;
            cfg.enable_filter_sweep = true;
            cfg.enable_reverb_washout = true;
            cfg.enable_pitch_riser = true;
            cfg.use_pryzma_samples = true;

            KuroArranger::PsySongArranger::GenerateArrangement(cfg, g_clip_manager, ::timeline);
            KuroUI::show_playlist = true;
            KuroUI::show_piano_roll = false;
            KuroUI::show_step_sequencer = false;
            KuroUI::g_playlist_scroll_y = 520.0f; // Scroll down to focus directly on Bézier automation tracks
            KuroUI::g_playlist_scroll_x = 200.0f;
            std::cout << "[TEST_BEZIER] Scrolled directly to automation tracks 16-20 with Bézier curves!\n";
        }
    }

    // Setup GLFW Drop Callback
    glfwSetDropCallback(window, drop_callback);

    // 3. Main Loop Gráfico com Frame Pacing Inteligente (60 FPS / Modo Baixo Consumo de CPU)
    auto frame_start_time = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window)) {
        // Se a janela estiver minimizada, descansa a CPU (10 FPS)
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            glfwWaitEventsTimeout(0.1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        frame_start_time = std::chrono::high_resolution_clock::now();
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Renderizar a Barra Superior Dupla FL Studio no topo da tela (0..62px)
        if (g_ai_engine) {
            KuroUI::RenderFLStyleTopbar(*g_ai_engine);
        }

        // 2. Configuração da Janela Principal para hospedar o DockSpace (abaixo da barra superior de 68px)
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        float fl_topbar_h = 68.0f;
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + fl_topbar_h));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - fl_topbar_h));
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | 
                                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("Kuro Main", nullptr, window_flags);
        ImGui::PopStyleVar(2);
        
        // Criar o DockSpace
        ImGuiID dockspace_id = ImGui::GetID("KuroDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        static bool first_time = true;
        if (first_time) {
            first_time = false;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(viewport->Size.x, viewport->Size.y - fl_topbar_h));

            ImGuiID dock_main_id = dockspace_id;

            // ─────────────────────────────────────────────────────────────────
            // ABDUCTION STUDIO FLUID 3-ZONE LAYOUT (FL STUDIO & ABLETON INSPIRED):
            //  [ BROWSER (Left 18%) ]  [ PLAYLIST (Right Top 72%) ]
            //                          [ DETAIL DOCK (Right Bottom 28%) ]
            // ─────────────────────────────────────────────────────────────────

            // 1. Split Left para BROWSER (18% da largura)
            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.18f, NULL, &dock_main_id);

            // 2. O restante do espaço central é a Área de Trabalho Limpa (DESKTOP) do Abduction Studio
            ImGuiID dock_id_desktop = dock_main_id;

            // ── Ancorar painéis ──────────────────
            ImGui::DockBuilderDockWindow("BROWSER", dock_id_left);
            ImGui::DockBuilderDockWindow("Browser", dock_id_left);
            ImGui::DockBuilderDockWindow("DESKTOP", dock_id_desktop);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        // ----------------------------------------------------
        // INTEGRAÇÃO REAL DA UI MODULAR (Phase 19 / Phase 24)
        // ----------------------------------------------------
        if (g_ai_engine) {
            KuroUI::RenderMainApp(*g_ai_engine);
        }

        // Render Spectrum Visualizer & Vectorscope UI
        g_spectrum_visualizer.renderUI();
        
        ImGui::End();
        
        KuroUI::RenderFloatingWindows();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.04f, 0.04f, 0.06f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Captura Interna Hardware de Framebuffer para Testes Autônomos e Debug
        std::filesystem::path trigger_p = std::filesystem::temp_directory_path() / "abduction_screenshot_req.txt";
        std::filesystem::path default_out = std::filesystem::temp_directory_path() / "abduction_screenshot.bmp";
        if (std::filesystem::exists(trigger_p) || (ImGui::IsKeyPressed(ImGuiKey_F12, false))) {
            std::error_code ec;
            std::filesystem::remove(trigger_p, ec);
            
            int w = display_w;
            int h = display_h;
            std::vector<unsigned char> pixels(w * h * 4);
            glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            
            #pragma pack(push, 1)
            struct BMPHeader {
                uint16_t bfType = 0x4D42;
                uint32_t bfSize = 0;
                uint16_t bfReserved1 = 0;
                uint16_t bfReserved2 = 0;
                uint32_t bfOffBits = 54;
                uint32_t biSize = 40;
                int32_t  biWidth = 0;
                int32_t  biHeight = 0;
                uint16_t biPlanes = 1;
                uint16_t biBitCount = 24;
                uint32_t biCompression = 0;
                uint32_t biSizeImage = 0;
                int32_t  biXPelsPerMeter = 2835;
                int32_t  biYPelsPerMeter = 2835;
                uint32_t biClrUsed = 0;
                uint32_t biClrImportant = 0;
            } bmp;
            #pragma pack(pop)
            
            int row_stride = ((w * 3 + 3) / 4) * 4;
            bmp.biWidth = w;
            bmp.biHeight = h;
            bmp.biSizeImage = row_stride * h;
            bmp.bfSize = 54 + bmp.biSizeImage;
            
            std::vector<unsigned char> row(row_stride, 0);
            std::ofstream f(default_out, std::ios::binary);
            if (f.is_open()) {
                f.write((const char*)&bmp, 54);
                for (int y = 0; y < h; y++) {
                    for (int x = 0; x < w; x++) {
                        int src_idx = (y * w + x) * 4;
                        row[x * 3 + 0] = pixels[src_idx + 2]; // B
                        row[x * 3 + 1] = pixels[src_idx + 1]; // G
                        row[x * 3 + 2] = pixels[src_idx + 0]; // R
                    }
                    f.write((const char*)row.data(), row_stride);
                }
                f.close();
            }
        }

        // FASE 27: Renderização de Viewports Independentes (Multi-monitor)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);

        // Frame Pacing: 60 FPS quando focado (~16ms), 30 FPS em segundo plano (~33ms)
        bool is_focused = (glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0);
        int target_frame_ms = is_focused ? 16 : 33;
        auto frame_end_time = std::chrono::high_resolution_clock::now();
        auto elapsed_frame_ms = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end_time - frame_start_time).count();
        if (elapsed_frame_ms < target_frame_ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(target_frame_ms - elapsed_frame_ms));
        }
    }

    if (adc.isStreamOpen()) {
        adc.stopStream();
        adc.closeStream();
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
