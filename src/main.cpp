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

// Globais para o StudioUI
bool is_playing = false;
unsigned long long global_sample_count = 0;
ClipManager g_clip_manager;
KuroAudio::KuroWave g_kurowave;
KuroAudio::SynthEngine g_piano_synth;
KuroDSP::ExpressiveLeadSynth g_lead_synth("lead");
KuroDSP::MonkSynth g_monk_synth("monk");
KuroDSP::AlienVoiceSynth g_alien_synth("alien");
KuroDSP::AnalogMonsterSynth g_analog_synth("analog");
KuroDSP::SynthwaveSynth g_synthwave_synth("synthwave");
void clear_all_synths() {
    g_piano_synth.clearNotes();
    g_kurowave.clearNotes();
    g_lead_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
    g_monk_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
    g_alien_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
    g_analog_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
    g_synthwave_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
}
KuroAudio::RecordManager g_record_manager;
KuroDSP::AudioGraph master_graph;
#include "audio/AudioEvent.h"
KuroDSP::TimelineManager timeline;

KuroDSP::GrossBeatNode g_gross_beat;
std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;
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
float track_synth_buffer_l[8][2048] = {{0.0f}};
float track_synth_buffer_r[8][2048] = {{0.0f}};

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

std::string track_names[MAX_TRACKS] = {"KICK/BASS", "LEADS", "VOX", "FX", "DRUMS", "SYNTH", "PADS", "EXTRA", "TRK 9", "TRK 10", "TRK 11", "TRK 12", "TRK 13", "TRK 14", "TRK 15", "TRK 16", "TRK 17", "TRK 18", "TRK 19", "TRK 20"};


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

// Linear gains para o DAG
float track_linear_volumes[MAX_TRACKS] = { 1.0f };
float track_linear_sends_A[MAX_TRACKS] = { 0.0f };
float track_linear_sends_B[MAX_TRACKS] = { 0.0f };
float g_master_volume = 0.8f;

float dummy_vol[8] = { 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f };
float dummy_pan[8] = { 0.0f };
int channel_tracks[8] = { 0, 1, 2, 3, 2, 3, 6, 7 };

float track_vu_levels[8] = { 0.0f };
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
        
        // CRÍTICO: Resetar o flag is_playing das notas do padrão ativo
        // Sem isso, ao reiniciar o play as notas que já tocaram não disparam de novo
        {
            std::lock_guard<std::mutex> lock(g_clip_manager.clip_mutex);
            for (auto& pat : g_clip_manager.global_patterns) {
                for (auto& note : pat.notes) {
                    note.is_playing = false;
                }
            }
        }
    }
    was_playing = currently_playing;

    // --- Suavização de Volumes e Sends (Evita zipper noise, respeita Mute/Solo global) ---
    bool any_solo = false;
    for (int i = 0; i < 8; i++) {
        if (track_solos[i]) { any_solo = true; break; }
    }

    for (int i = 0; i < MAX_TRACKS; i++) {
        float target_vol = 0.0f;
        bool is_muted = (i < 8) ? (track_mutes[i] || (any_solo && !track_solos[i])) : false;
        if (!is_muted) {
            target_vol = (track_volumes[i] <= -59.9f) ? 0.0f : std::pow(10.0f, track_volumes[i] / 20.0f);
        }
        
        float target_send_A = (track_sends_A[i] <= -59.9f) ? 0.0f : std::pow(10.0f, track_sends_A[i] / 20.0f);
        float target_send_B = (track_sends_B[i] <= -59.9f) ? 0.0f : std::pow(10.0f, track_sends_B[i] / 20.0f);
        
        track_linear_volumes[i] = track_linear_volumes[i] * 0.99f + target_vol * 0.01f;
        track_linear_sends_A[i] = track_linear_sends_A[i] * 0.99f + target_send_A * 0.01f;
        track_linear_sends_B[i] = track_linear_sends_B[i] * 0.99f + target_send_B * 0.01f;
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
            board.active_ritual_rate = std::clamp(new_val, 0.1f, 5.0f);
        }
    }

    // Calcular tempo global em segundos
    float global_time_sec = (float)global_sample_count / 44100.0f;

    // 1. Processar a Timeline (Gera Eventos MIDI e Automação)
    unsigned int out_offset_frames = 0;
    auto [midi_events, auto_events] = timeline.processBlock(nFrames, out_offset_frames);
    
    // Roteia Eventos MIDI (Piano Roll e Step Sequencer) para os Synths correspondentes
    for (const auto& ev : midi_events) {
        int track_idx = std::get<0>(ev);
        int pitch = std::get<1>(ev);
        float duration = std::get<2>(ev);
        float velocity = std::get<3>(ev);
        
        // 1. Sempre aciona a reprodução de samples / baterias no sampler geral
        g_piano_synth.triggerNote(pitch, duration, velocity, track_idx);
        
        // 2. Roteamento de sintetizadores direcionado por PITCH (evita conflitos de tracks no mixer se o FLEX estiver inativo)
        if (pitch == 48 && !g_piano_synth.flex_active[3]) { // Bassline -> MonkSynth & AnalogMonster
            KuroDSP::MpeMidiEvent ev_mpe{pitch, pitch, true, velocity, 0.0f, 0.5f, 0.5f, duration};
            g_monk_synth.pushMidiEvent(ev_mpe);
            g_analog_synth.pushMidiEvent(ev_mpe);
        }
        else if (pitch == 60 && !g_piano_synth.flex_active[4]) { // Serum Chords -> SynthwaveSynth
            KuroDSP::MpeMidiEvent ev_mpe{pitch, pitch, true, velocity, 0.0f, 0.5f, 0.5f, duration};
            g_synthwave_synth.pushMidiEvent(ev_mpe);
        }
        else if (pitch == 72 && !g_piano_synth.flex_active[5]) { // Lead Synth -> LeadSynth & AlienSynth
            KuroDSP::MpeMidiEvent ev_mpe{pitch, pitch, true, velocity, 0.0f, 0.5f, 0.5f, duration};
            g_lead_synth.pushMidiEvent(ev_mpe);
            g_alien_synth.pushMidiEvent(ev_mpe);
        }
        else if (track_idx == 5 && !g_piano_synth.flex_active[5]) { // Notas da track 5 vão para o KuroWave
            g_kurowave.triggerNote(pitch, duration, velocity);
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
    for (int t = 0; t < 8; t++) {
        std::fill_n(track_synth_buffer_l[t], nFrames, 0.0f);
        std::fill_n(track_synth_buffer_r[t], nFrames, 0.0f);
    }
    
    // Processa o Sampler Multicanal nos buffers de injeção
    float* multitrack_l[8];
    float* multitrack_r[8];
    for (int t = 0; t < 8; t++) {
        multitrack_l[t] = track_synth_buffer_l[t];
        multitrack_r[t] = track_synth_buffer_r[t];
    }
    g_piano_synth.processMultitrack(multitrack_l, multitrack_r, nFrames, global_time_sec);
    
    // Processa os sintetizadores nos respectivos buffers de canal (se o FLEX estiver inativo para aquele canal)
    if (!g_piano_synth.flex_active[3]) {
        g_monk_synth.process(track_synth_buffer_l[3], track_synth_buffer_r[3], nFrames);
        g_analog_synth.process(track_synth_buffer_l[3], track_synth_buffer_r[3], nFrames);
    }
    if (!g_piano_synth.flex_active[4]) {
        g_synthwave_synth.process(track_synth_buffer_l[4], track_synth_buffer_r[4], nFrames);
    }
    if (!g_piano_synth.flex_active[5]) {
        g_lead_synth.process(track_synth_buffer_l[5], track_synth_buffer_r[5], nFrames);
        g_alien_synth.process(track_synth_buffer_l[5], track_synth_buffer_r[5], nFrames);
        g_kurowave.process(track_synth_buffer_l[5], track_synth_buffer_r[5], nFrames, global_time_sec);
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
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.06f, 1.00f); // Deep Space Black
    style.Colors[ImGuiCol_Border] = ImVec4(0.22f, 1.00f, 0.08f, 0.30f); // Faint Neon Green Border
    style.Colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f); // Dark Grey
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 1.00f, 0.08f, 0.60f); // Neon Green Glow
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.22f, 1.00f, 0.08f, 1.00f); // Solid Neon Green
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.40f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.50f, 0.10f, 0.60f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 1.00f, 0.08f, 0.80f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.95f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.22f, 1.00f, 0.08f, 1.00f); // Neon Green VU
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.40f, 1.00f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.22f, 1.00f, 0.08f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.22f, 1.00f, 0.08f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 1.00f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.22f, 1.00f, 0.08f, 0.40f);
    
    style.WindowRounding = 5.0f; 
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 4.0f;
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

    // FASE 22: Instanciar os 8 Clip Players (1 por faixa) mapeando os buffers extraídos pela IA
    for (int i = 0; i < 8; i++) {
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

    // 1. Setup Audio Engine (RtAudio)
    RtAudio adc(RtAudio::WINDOWS_WASAPI);
    if (adc.getDeviceCount() < 1) {
        std::cerr << "Nenhum dispositivo de áudio encontrado!\n";
    }
    
    LockFreeAudioQueue<AudioEvent> ui_to_audio_queue(1024);
    
    if (adc.getDeviceCount() >= 1) {
        RtAudio::StreamParameters parameters;
        parameters.deviceId = adc.getDefaultOutputDevice();
        parameters.nChannels = 2; // Stereo
        parameters.firstChannel = 0;
        unsigned int sampleRate = 44100;
        unsigned int bufferFrames = 256; 
        
        try {
            adc.openStream(&parameters, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &audioCallback, &ui_to_audio_queue);
            adc.startStream();
            std::cout << "[SYSTEM] WASAPI Audio Engine inciado (" << sampleRate << "Hz, " << bufferFrames << " buffers).\n";
        } catch (...) {
            std::cerr << "[SYSTEM] Falha ao iniciar WASAPI. Rodando apenas Interface Gráfica.\n";
        }
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
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Janela sem borda para controles customizados

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Abduction Studio V2 (Modo Automático)", NULL, NULL);
    if (window == NULL) {
        std::ofstream err("crash_log.txt", std::ios::app);
        err << "[FATAL] glfwCreateWindow failed!\n";
        err.flush();
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // FASE 21: Drag & Drop Callback
    glfwSetDropCallback(window, drop_callback);
    main_hwnd = glfwGetWin32Window(window);
    glfwSwapInterval(1); // Enable vsync (60 FPS)

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // FASE 27: Habilita janelas flutuantes multi-monitor
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Habilita sistema de Docking (Ideia 3)
    
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

    // Setup GLFW Drop Callback
    glfwSetDropCallback(window, drop_callback);

    // 3. Main Loop Gráfico (60 FPS)
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Configuração da Janela Principal para hospedar o DockSpace
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | 
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
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            ImGuiID dock_main_id = dockspace_id;

            // ─────────────────────────────────────────────────────────────────
            // FL Studio Layout:
            //  [Browser(L 18%)]  [Piano Roll / Proj. Settings (Center)]  [Mixer (R 28%)]
            //  ───────────────────────────────────────────────────────────────
            //  [Playlist / Channel Rack / Audio Editor (Bottom 32%)]
            // ─────────────────────────────────────────────────────────────────

            // 1. Split Bottom first (so it spans full width)
            ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.32f, NULL, &dock_main_id);

            // 2. Split Left for Browser panel
            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.18f, NULL, &dock_main_id);

            // 3. Split Right for Mixer Panel
            ImGuiID dock_id_mixer = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.30f, NULL, &dock_main_id);

            // Remaining center = Piano Roll area
            ImGuiID dock_id_center = dock_main_id;

            // ── Dock the actual windows ──────────────────────────────────────

            // Left: Browser (Files tab)
            ImGui::DockBuilderDockWindow("Files", dock_id_left);
            ImGui::DockBuilderDockWindow("Plugins", dock_id_left);
            ImGui::DockBuilderDockWindow("Samples", dock_id_left);
            ImGui::DockBuilderDockWindow("Browser", dock_id_left);

            // Center: Piano Roll + Project Settings (tabbed)
            ImGui::DockBuilderDockWindow("Piano Roll", dock_id_center);
            ImGui::DockBuilderDockWindow("Project Settings", dock_id_center);

            // Right: Mixer Panel only
            ImGui::DockBuilderDockWindow("Mixer Panel", dock_id_mixer);
            ImGui::DockBuilderDockWindow("Master", dock_id_mixer);

            // Bottom: Playlist, Channel Rack, Audio Editor (tabbed)
            ImGui::DockBuilderDockWindow("[Playlist]", dock_id_bottom);
            ImGui::DockBuilderDockWindow("[Channel Rack]", dock_id_bottom);
            ImGui::DockBuilderDockWindow("[Automation Clip]", dock_id_bottom);
            ImGui::DockBuilderDockWindow("[Audio Editor]", dock_id_bottom);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        // ----------------------------------------------------
        // INTEGRAÇÃO REAL DA UI MODULAR (Phase 19 / Phase 24)
        // ----------------------------------------------------
        if (g_ai_engine) {
            KuroUI::RenderMainApp(*g_ai_engine);
        }
        
        ImGui::End();
        
        KuroUI::RenderFloatingWindows();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.04f, 0.04f, 0.06f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // FASE 27: Renderização de Viewports Independentes (Multi-monitor)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
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
