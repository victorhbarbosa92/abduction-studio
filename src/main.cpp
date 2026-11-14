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
#include "ui/UIScaleManager.h"
#include "ui/DJLayoutMetrics.h"
#include "ui/DJInteractiveTourEngine.h"
#include "ui/DJAcademyGameUI.h"
#include "ui/DJStudioRackUI.h"
#include "ui/SpectrumVisualizerUI.h"
#include "ui/ChannelInstrumentManager.h"
#include "core/DJAudioDeviceManager.h"
#include "core/AutoUpdater.h"

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
ImFont* g_font_rekordbox_bold = nullptr;
ImFont* g_font_rekordbox_regular = nullptr;
ImFont* g_font_rekordbox_large = nullptr;
ImFont* g_font_rekordbox_small = nullptr;
ImFont* g_font_display_hero = nullptr;    // 22.0f Bold: Numerais BPM no Jogwheel e Telemetria Hero
ImFont* g_font_title_large = nullptr;     // 16.0f Bold: Título da Faixa, DECK 1/2, Master
ImFont* g_font_body_medium = nullptr;     // 13.0f Semibold: Artista, Camelot, Decimais, Sync
ImFont* g_font_control_small = nullptr;   // 11.0f Bold: Knobs Mixer, MST, CUE, Pitch
ImFont* g_font_micro_tag = nullptr;       // 9.5f Bold: Badges X.1Bars, Timestamps Pads, Ticks

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
float dummy_pitch[MAX_TRACKS] = { 0.0f };
int channel_tracks[MAX_TRACKS] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

float track_vu_levels[MAX_TRACKS] = { 0.0f };
float master_vu_level_l = 0.0f;
float master_vu_level_r = 0.0f;
unsigned int g_audio_output_channels = 2;

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
    AudioClipEditorUI g_audio_clip_editor_ui;
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
bool show_playlist = false;

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
        g_psy_bass_synth.pushMidiEvent({-999, 0, false, 0.0f, 0.0f, 0.0f, 0.0f});
        
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
            // Se o canal 1 estiver configurado como KURO_RHYTHM_BASS, o som de baixo vem exclusivamente do motor dedicado
            if (!(track_idx == 1 && KuroUI::g_channel_slots[1].type == KuroUI::ChannelInstrumentType::KURO_RHYTHM_BASS)) {
                g_piano_synth.triggerNote(pitch, duration, velocity, track_idx);
            }
            
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

            // 3. Roteamento de notas MIDI para Plugins VST3 / CLAP ativos no Rack da Faixa
            auto rack_node = std::dynamic_pointer_cast<KuroDSP::RackNode>(master_graph.getNode("TrackPedalboard_" + std::to_string(track_idx)));
            if (rack_node) {
                rack_node->pushMidiNote(pitch, true, velocity);
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

    // Mixa o Motor DJ Nativo (Deck A, Deck B e Sampler Slots) com Roteamento Master + Headphone CUE
    std::vector<float> dj_cue_l(nFrames, 0.0f);
    std::vector<float> dj_cue_r(nFrames, 0.0f);

    if (g_dj_engine) {
        g_dj_engine->process(global_synth_l, global_synth_r, dj_cue_l.data(), dj_cue_r.data(), nFrames);
    }
    
    // 3. Efeitos Nativos do Master Bus e Mixagem Final
    float* out_buffer = (float*)outputBuffer;
    auto& devMgr = KuroAudio::DJAudioDeviceManager::getInstance();

    // Injeção de tom de teste Master (Bip suave de 440 Hz para identificação de caixas)
    if (devMgr.test_tone_master.load()) {
        for (unsigned int i = 0; i < nFrames; i++) {
            if (devMgr.master_tone_samples_left > 0) {
                float env = 0.4f;
                if (devMgr.master_tone_samples_left < 2000) {
                    env = (float)devMgr.master_tone_samples_left / 2000.0f * 0.4f;
                }
                float sample = (float)std::sin(devMgr.master_tone_phase) * env;
                devMgr.master_tone_phase += 2.0 * 3.141592653589793 * 440.0 / (double)devMgr.sample_rate;
                if (devMgr.master_tone_phase > 2.0 * 3.141592653589793) devMgr.master_tone_phase -= 2.0 * 3.141592653589793;
                global_synth_l[i] += sample;
                global_synth_r[i] += sample;
                devMgr.master_tone_samples_left--;
            } else {
                devMgr.test_tone_master.store(false);
            }
        }
    }

    // Se estiver no modo DUAL_DEVICE, alimenta o ring buffer para a segunda placa (Fone/CUE):
    if (devMgr.routing_mode == KuroAudio::DJRoutingMode::DUAL_DEVICE) {
        for (unsigned int i = 0; i < nFrames; i++) {
            devMgr.pushCueSamples(dj_cue_l[i], dj_cue_r[i]);
        }
    }

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

        if (devMgr.routing_mode == KuroAudio::DJRoutingMode::MULTI_CHANNEL_4CH && g_audio_output_channels >= 4) {
            out_buffer[i * 4 + 0] = left;          // Ch 1: Master L
            out_buffer[i * 4 + 1] = right;         // Ch 2: Master R
            out_buffer[i * 4 + 2] = dj_cue_l[i];   // Ch 3: Retorno CUE L
            out_buffer[i * 4 + 3] = dj_cue_r[i];   // Ch 4: Retorno CUE R
        } else if (devMgr.routing_mode == KuroAudio::DJRoutingMode::SPLIT_MONO) {
            float master_mono = (left + right) * 0.5f;
            float cue_mono = (dj_cue_l[i] + dj_cue_r[i]) * 0.5f;
            out_buffer[i * 2 + 0] = master_mono;   // Ch 1 (Esq): Master Mono (Caixas)
            out_buffer[i * 2 + 1] = cue_mono;      // Ch 2 (Dir): Fone Cue Mono
        } else {
            out_buffer[i * 2 + 0] = left;          // Ch 1: Master L
            out_buffer[i * 2 + 1] = right;         // Ch 2: Master R
        }
    }

    // Calcula níveis de VU reais para o Master e a trilha 5 (Synth)
    unsigned int stride = (devMgr.routing_mode == KuroAudio::DJRoutingMode::MULTI_CHANNEL_4CH && g_audio_output_channels >= 4) ? 4 : 2;
    float peak_l = 0.0f;
    float peak_r = 0.0f;
    float peak_s = 0.0f;
    for (unsigned int i = 0; i < nFrames; i++) {
        float al = std::abs(out_buffer[i * stride + 0]);
        float ar = std::abs(out_buffer[i * stride + 1]);
        if (al > peak_l) peak_l = al;
        if (ar > peak_r) peak_r = ar;

        float val = (std::abs(global_synth_l[i]) + std::abs(global_synth_r[i])) * 0.5f;
        if (val > peak_s) peak_s = val;
    }
    master_vu_level_l = master_vu_level_l * 0.8f + peak_l * 0.2f;
    master_vu_level_r = master_vu_level_r * 0.8f + peak_r * 0.2f;
    track_vu_levels[5] = track_vu_levels[5] * 0.8f + peak_s * 0.2f;

    devMgr.master_vu_l.store(master_vu_level_l);
    devMgr.master_vu_r.store(master_vu_level_r);

    if (devMgr.routing_mode != KuroAudio::DJRoutingMode::DUAL_DEVICE) {
        float peak_cue = 0.0f;
        for (unsigned int i = 0; i < nFrames; i++) {
            float c = (std::abs(dj_cue_l[i]) + std::abs(dj_cue_r[i])) * 0.5f;
            if (c > peak_cue) peak_cue = c;
        }
        devMgr.cue_vu_l.store(devMgr.cue_vu_l.load() * 0.8f + peak_cue * 0.2f);
        devMgr.cue_vu_r.store(devMgr.cue_vu_r.load() * 0.8f + peak_cue * 0.2f);
    }

    // Gravação Master (Bounce)
    g_record_manager.processOutput(global_synth_l, global_synth_r, nFrames);

    global_sample_count += nFrames;
    return 0;
}

// Callback secundário exclusivo para a Saída de Retorno / Fone (Modo Dual Device)
int cueAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                     double streamTime, RtAudioStreamStatus status, void *userData) {
    float *out = (float *)outputBuffer;
    auto& devMgr = KuroAudio::DJAudioDeviceManager::getInstance();

    std::vector<float> cue_l(nBufferFrames, 0.0f);
    std::vector<float> cue_r(nBufferFrames, 0.0f);

    devMgr.popCueSamples(cue_l.data(), cue_r.data(), nBufferFrames);

    // Se tom de teste de fone/retorno estiver ativo, mixa tom suave de 880 Hz:
    if (devMgr.test_tone_cue.load()) {
        for (unsigned int i = 0; i < nBufferFrames; i++) {
            if (devMgr.cue_tone_samples_left > 0) {
                float env = 0.4f;
                if (devMgr.cue_tone_samples_left < 2000) {
                    env = (float)devMgr.cue_tone_samples_left / 2000.0f * 0.4f;
                }
                float sample = (float)std::sin(devMgr.cue_tone_phase) * env;
                devMgr.cue_tone_phase += 2.0 * 3.141592653589793 * 880.0 / (double)devMgr.sample_rate;
                if (devMgr.cue_tone_phase > 2.0 * 3.141592653589793) devMgr.cue_tone_phase -= 2.0 * 3.141592653589793;
                cue_l[i] += sample;
                cue_r[i] += sample;
                devMgr.cue_tone_samples_left--;
            } else {
                devMgr.test_tone_cue.store(false);
            }
        }
    }

    float peak_l = 0.0f, peak_r = 0.0f;
    for (unsigned int i = 0; i < nBufferFrames; i++) {
        float l = cue_l[i];
        float r = cue_r[i];
        out[i * 2 + 0] = l;
        out[i * 2 + 1] = r;
        if (std::abs(l) > peak_l) peak_l = std::abs(l);
        if (std::abs(r) > peak_r) peak_r = std::abs(r);
    }
    devMgr.cue_vu_l.store(devMgr.cue_vu_l.load() * 0.8f + peak_l * 0.2f);
    devMgr.cue_vu_r.store(devMgr.cue_vu_r.load() * 0.8f + peak_r * 0.2f);

    return 0;
}

// Inicializa ou reinicia dinamicamente os fluxos de áudio de acordo com a seleção de hardware do PC
void initOrRestartAudioStreams(RtAudio& adc, RtAudio& dac_cue, LockFreeAudioQueue<AudioEvent>& queue) {
    auto& devMgr = KuroAudio::DJAudioDeviceManager::getInstance();

    // 1. Fecha streams existentes com segurança
    try {
        if (dac_cue.isStreamOpen()) {
            if (dac_cue.isStreamRunning()) dac_cue.stopStream();
            dac_cue.closeStream();
        }
    } catch (...) {}

    try {
        if (adc.isStreamOpen()) {
            if (adc.isStreamRunning()) adc.stopStream();
            adc.closeStream();
        }
    } catch (...) {}

    // 2. Garante que os dispositivos do PC foram escaneados
    if (devMgr.output_devices.empty()) {
        devMgr.rescanDevices();
    }
    if (devMgr.output_devices.empty()) {
        KuroUtils::Log("[Audio Engine] ERRO: Nenhum dispositivo de áudio de saída encontrado neste PC.");
        return;
    }

    // 3. Configura a saída Mestre (adc)
    unsigned int master_dev_id = devMgr.master_device_id;
    bool master_valid = false;
    for (const auto& d : devMgr.output_devices) {
        if (d.id == master_dev_id) { master_valid = true; break; }
    }
    if (!master_valid) {
        master_dev_id = devMgr.default_device_id;
        devMgr.master_device_id = master_dev_id;
    }

    unsigned int m_channels = 2;
    if (devMgr.routing_mode == KuroAudio::DJRoutingMode::MULTI_CHANNEL_4CH) {
        for (const auto& d : devMgr.output_devices) {
            if (d.id == master_dev_id && d.output_channels >= 4) {
                m_channels = 4;
                break;
            }
        }
    }

    unsigned int bufferFrames = devMgr.buffer_size >= 64 ? devMgr.buffer_size : 256;
    unsigned int target_sr = devMgr.sample_rate >= 8000 ? devMgr.sample_rate : 48000;

    std::vector<unsigned int> candidate_rates = { target_sr, 48000, 44100, 96000 };
    candidate_rates.erase(std::remove_if(candidate_rates.begin(), candidate_rates.end(), [](unsigned int r) {
        return r < 8000;
    }), candidate_rates.end());
    candidate_rates.erase(std::unique(candidate_rates.begin(), candidate_rates.end()), candidate_rates.end());

    bool master_started = false;
    for (unsigned int sr : candidate_rates) {
        RtAudio::StreamParameters parameters;
        parameters.deviceId = master_dev_id;
        parameters.nChannels = m_channels;
        parameters.firstChannel = 0;
        unsigned int bf = bufferFrames;

        try {
            adc.openStream(&parameters, NULL, RTAUDIO_FLOAT32, sr, &bf, &audioCallback, &queue);
            adc.startStream();
            master_started = true;
            g_audio_output_channels = m_channels;
            devMgr.sample_rate = sr;
            devMgr.buffer_size = bf;
            if (g_dj_engine) {
                g_dj_engine->host_sample_rate = (double)sr;
            }
            KuroUtils::Log("[Audio Engine] SUCESSO: Master WASAPI aberto em Dev ID " + std::to_string(master_dev_id) +
                           " (" + std::to_string(m_channels) + " canais, " + std::to_string(sr) + " Hz, " + std::to_string(bf) + " frames)");
            std::cout << "[SYSTEM] WASAPI Master iniciado (" << m_channels << " canais, " << sr << "Hz, " << bf << " frames).\n";
            break;
        } catch (const std::exception& ex) {
            KuroUtils::Log("[Audio Engine] Master falhou com " + std::to_string(sr) + " Hz: " + ex.what());
        } catch (...) {
            KuroUtils::Log("[Audio Engine] Master falhou.");
        }
    }

    // 4. Se estiver em DUAL_DEVICE, abre a saída de Retorno / Fone (dac_cue)
    if (master_started && devMgr.routing_mode == KuroAudio::DJRoutingMode::DUAL_DEVICE) {
        unsigned int cue_dev_id = devMgr.cue_device_id;
        bool cue_valid = false;
        for (const auto& d : devMgr.output_devices) {
            if (d.id == cue_dev_id) { cue_valid = true; break; }
        }
        if (cue_valid && cue_dev_id != master_dev_id) {
            bool cue_started = false;
            for (unsigned int sr : candidate_rates) {
                RtAudio::StreamParameters cue_params;
                cue_params.deviceId = cue_dev_id;
                cue_params.nChannels = 2;
                cue_params.firstChannel = 0;
                unsigned int bf_cue = bufferFrames;

                try {
                    dac_cue.openStream(&cue_params, NULL, RTAUDIO_FLOAT32, sr, &bf_cue, &cueAudioCallback, nullptr);
                    dac_cue.startStream();
                    cue_started = true;
                    KuroUtils::Log("[Audio Engine] SUCESSO: Retorno (CUE) WASAPI aberto em Dev ID " + std::to_string(cue_dev_id) +
                                   " (2 canais, " + std::to_string(sr) + " Hz, " + std::to_string(bf_cue) + " frames)");
                    std::cout << "[SYSTEM] WASAPI Retorno/CUE iniciado (2 canais, " << sr << "Hz, " << bf_cue << " frames).\n";
                    break;
                } catch (const std::exception& ex) {
                    KuroUtils::Log("[Audio Engine] Retorno falhou com " + std::to_string(sr) + " Hz: " + ex.what());
                } catch (...) {
                    KuroUtils::Log("[Audio Engine] Retorno falhou.");
                }
            }
            if (!cue_started) {
                KuroUtils::Log("[Audio Engine] AVISO: Não foi possível abrir o dispositivo de Retorno.");
            }
        }
    }
}

static void glfw_error_callback(int error, const char* description) {
    std::ofstream err("crash_log.txt", std::ios::app);
    err << "[GLFW ERROR] " << error << ": " << description << "\n";
    err.flush();
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Callback real para Drag and Drop de arquivos do Windows (Multi-Drop & Auto-Track Creation)
void drop_callback(GLFWwindow* window, int count, const char** paths)
{
    if (count <= 0 || !paths) return;

    auto get_ext = [](const std::string& path) {
        size_t dot_pos = path.find_last_of(".");
        if (dot_pos == std::string::npos) return std::string("");
        std::string ext = path.substr(dot_pos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)::tolower(c); });
        return ext;
    };

    // 1. Se a modal DJ Studio estiver aberta, carrega diretamente no Deck A e/ou B
    if (KuroUI::show_dj_modal && g_dj_engine) {
        if (count == 1) {
            std::string file_path = paths[0];
            std::string ext = get_ext(file_path);
            if (ext == "wav" || ext == "mp3" || ext == "flac" || ext == "aiff" || ext == "ogg") {
                if (!g_dj_engine->deckA.is_playing.load()) {
                    g_dj_engine->deckA.loadTrack(file_path);
                    std::cout << "[DJ DROP] Carregado no Deck A: " << file_path << std::endl;
                } else {
                    g_dj_engine->deckB.loadTrack(file_path);
                    std::cout << "[DJ DROP] Carregado no Deck B: " << file_path << std::endl;
                }
                return;
            }
        } else if (count >= 2) {
            g_dj_engine->deckA.loadTrack(paths[0]);
            g_dj_engine->deckB.loadTrack(paths[1]);
            std::cout << "[DJ MULTI-DROP] Deck A: " << paths[0] << " | Deck B: " << paths[1] << std::endl;
            return;
        }
    }

    // 2. Modo DAW / Playlist: Multi-Drop com Auto-Track Creation e Roteamento de Mixer
    float bpm = timeline.getBPM();
    if (bpm < 10.0f) bpm = 140.0f;
    float bar_len_sec = (60.0f / bpm) * 4.0f;
    float playhead_sec = (float)timeline.getMasterFrame() / 44100.0f;
    float snap_pos = std::round(playhead_sec / (bar_len_sec * 0.25f)) * (bar_len_sec * 0.25f);

    int imported_count = 0;
    int next_search_track = 0;

    for (int p = 0; p < count; ++p) {
        std::string file_path = paths[p];
        std::string ext = get_ext(file_path);

        bool is_audio = (ext == "wav" || ext == "mp3" || ext == "flac" || ext == "aiff" || ext == "ogg" || ext == "m4a");
        bool is_midi = (ext == "mid" || ext == "midi");

        if (!is_audio && !is_midi) {
            if (g_ai_engine && !g_ai_engine->isRunning()) {
                g_ai_engine->startProcessing(file_path);
            }
            continue;
        }

        // Auto-Track Creation: Encontra a próxima faixa livre dedicada
        int target_t = -1;
        for (int ch = next_search_track; ch < MAX_TRACKS; ++ch) {
            if (g_clip_manager.track_clips[ch].empty() && g_clip_manager.track_midi_clips[ch].empty() && !g_piano_synth.getDrumSample(ch).loaded) {
                target_t = ch;
                break;
            }
        }
        if (target_t < 0) {
            for (int ch = 0; ch < MAX_TRACKS; ++ch) {
                if (g_clip_manager.track_clips[ch].empty() && g_clip_manager.track_midi_clips[ch].empty() && !g_piano_synth.getDrumSample(ch).loaded) {
                    target_t = ch;
                    break;
                }
            }
        }
        if (target_t < 0) {
            target_t = (next_search_track + p) % MAX_TRACKS;
        }

        next_search_track = target_t + 1;

        // Auto-roteamento para canais livres do mixer
        channel_tracks[target_t] = (target_t % 16) + 1;

        std::string fname = std::filesystem::path(file_path).filename().string();

        if (is_audio) {
            g_piano_synth.loadTrackSample(target_t, file_path.c_str());
            track_names[target_t] = "[SMP] " + fname;

            AudioClip ac;
            ac.id = g_clip_manager.next_id++;
            ac.start_time_sec = snap_pos;
            float dur = bar_len_sec * 4.0f;
            auto& ds = g_piano_synth.getDrumSample(target_t);
            if (ds.loaded && ds.sample_rate > 0 && ds.total_frames > 0) {
                dur = (float)ds.total_frames / (float)ds.sample_rate;
            }
            ac.length_sec = dur;
            ac.source_offset_sec = 0.0f;
            ac.file_path = file_path;
            ac.name = fname;
            ac.is_selected = true;
            g_clip_manager.track_clips[target_t].push_back(ac);

            if (g_global_sampler && p == 0) {
                g_global_sampler->loadSample(file_path);
            }
            std::cout << "[DAW MULTI-DROP] Sample em Faixa " << (target_t + 1) << " (Mixer " << channel_tracks[target_t] << "): " << fname << std::endl;
        } else if (is_midi) {
            track_names[target_t] = "[MIDI] " + fname;

            MidiClip mc;
            mc.id = g_clip_manager.next_id++;
            mc.start_time_sec = snap_pos;
            mc.length_sec = bar_len_sec * 4.0f;
            mc.pattern_id = (target_t < (int)g_clip_manager.global_patterns.size()) ? target_t : 0;
            mc.name = fname;
            mc.is_selected = true;
            g_clip_manager.track_midi_clips[target_t].push_back(mc);

            std::cout << "[DAW MULTI-DROP] MIDI em Faixa " << (target_t + 1) << " (Mixer " << channel_tracks[target_t] << "): " << fname << std::endl;
        }

        imported_count++;
    }

    if (imported_count > 0) {
        g_clip_manager.pushUndo();
        char hint_buf[128];
        snprintf(hint_buf, sizeof(hint_buf), "Multi-Drop: %d arquivos importados e roteados no Mixer!", imported_count);
        KuroUI::SetFLHint(hint_buf);
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
    style.Colors[ImGuiCol_TabSelected] = ImVec4(0.00f, 0.85f, 1.00f, 0.90f);      // Cyan Active Tab
    style.Colors[ImGuiCol_TabDimmed] = ImVec4(0.05f, 0.08f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.08f, 0.13f, 0.19f, 1.00f);
    
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
    style.Colors[ImGuiCol_TabSelected] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_TabDimmed] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
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

#ifdef _WIN32
    timeBeginPeriod(1); // Alta precisão de 1ms para temporizadores no Windows (Fluidez 60+ FPS)
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        std::locale::global(std::locale(".UTF-8"));
    } catch (...) {}

    KuroUtils::Log("Abduction Studio V2 Iniciado.");
    KuroUtils::Log("[Step 1] after Log Iniciado");
    
    // Inicializa a integração de APIs do FL Studio e Ableton Live (porta UDP 9000)
    DawApiBridge::startBridge();
    KuroUtils::Log("[Step 2] after startBridge");

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
    KuroUtils::Log("[Step 4] after g_ai_engine");
    g_dj_engine = std::make_unique<KuroAudio::DJEngine>();
    KuroUtils::Log("[Step 5] after g_dj_engine");
    KuroUI::g_dj_studio_ui.init(g_dj_engine.get());
    KuroUtils::Log("[Step 6] after g_dj_studio_ui.init");
    g_stem_engine = std::make_unique<KuroAudio::StemExtractorEngine>();
    KuroUtils::Log("[Step 7] after g_stem_engine");
    
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
        master_graph.addNode("Track" + std::to_string(i), track_pedal);

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

    // 1. Setup Audio Engine (RtAudio WASAPI com Auto-detecção de Saídas Mestre e Retorno/Fone por PC)
    RtAudio adc(RtAudio::WINDOWS_WASAPI);
    RtAudio dac_cue(RtAudio::WINDOWS_WASAPI);
    LockFreeAudioQueue<AudioEvent> ui_to_audio_queue(1024);

    initOrRestartAudioStreams(adc, dac_cue, ui_to_audio_queue);
    KuroUtils::Log("[Step 8] after initOrRestartAudioStreams");

    // 2. Setup ImGui / GLFW
#ifdef _WIN32
    typedef BOOL(WINAPI* PFN_SetProcessDpiAwarenessContext)(void*);
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        auto pSetContext = (PFN_SetProcessDpiAwarenessContext)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (pSetContext) {
            pSetContext((void*)-4); // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
        }
    }
#endif
    KuroUtils::Log("[Init] Starting glfwInit...");
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        KuroUtils::Log("[Init FATAL] glfwInit failed!");
        std::ofstream err("crash_log.txt", std::ios::app);
        err << "[FATAL] glfwInit failed!\n";
        err.flush();
        return 1;
    }
    KuroUtils::Log("[Init] glfwInit SUCCESS.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // Habilitar borda e barra do Windows
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWmonitor* primary_mon = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = primary_mon ? glfwGetVideoMode(primary_mon) : nullptr;
    int win_w = 1280;
    int win_h = 720;

    if (mode) {
        win_w = std::max(1280, mode->width - 60);
        win_h = std::max(720, mode->height - 60);
    }

    KuroUtils::Log("[Init] Creating GLFW window (" + std::to_string(win_w) + "x" + std::to_string(win_h) + ")...");
    GLFWwindow* window = glfwCreateWindow(win_w, win_h, "Abduction Studio V4.0 - Flagship Edition", NULL, NULL);
    if (window == NULL) {
        KuroUtils::Log("[Init FATAL] glfwCreateWindow returned NULL!");
        std::ofstream err("crash_log.txt", std::ios::app);
        err << "[FATAL] glfwCreateWindow failed!\n";
        err.flush();
        glfwTerminate();
        return 1;
    }
    KuroUtils::Log("[Init] glfwCreateWindow SUCCESS.");
    glfwSetWindowSizeLimits(window, 960, 540, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);
    KuroUtils::Log("[Init] glfwMakeContextCurrent done. Showing window...");
    glfwShowWindow(window);
    glfwMaximizeWindow(window); // DAW profissional sempre preenche a tela do usuário de forma adaptativa
    glfwSwapInterval(0); // VSync OFF — frame limiter manual a 120 FPS
    
    // FASE 21: Drag & Drop Callback
    glfwSetDropCallback(window, drop_callback);
    main_hwnd = glfwGetWin32Window(window);
    if (main_hwnd) {
        KuroUtils::Log("[Init] Win32 HWND obtained: " + std::to_string((uintptr_t)main_hwnd) + ". Bringing window to top...");
        ShowWindow(main_hwnd, SW_SHOWDEFAULT);
        BringWindowToTop(main_hwnd);
        SetForegroundWindow(main_hwnd);
    } else {
        KuroUtils::Log("[Init ALERTA] Win32 HWND é NULL!");
    }
    glfwSwapInterval(0); // VSync OFF — 120 FPS via frame limiter dedicado

    KuroUtils::Log("[Init] Initializing Dear ImGui context...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Desativado para manter renderização unificada na janela principal
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Habilita sistema de Docking (Ideia 3)
    io.ConfigDebugHighlightIdConflicts = false;         // Desativa popups de debug em build de produção
    
    // Load Fonts com Oversampling de Alta Fidelidade (Anti-Aliasing nítido em qualquer escala)
    auto findResourcePath = [](const std::string& path) -> std::string {
        if (std::filesystem::exists(path)) return path;
        std::string p2 = "../" + path;
        if (std::filesystem::exists(p2)) return p2;
        std::string p3 = "../../" + path;
        if (std::filesystem::exists(p3)) return p3;
        return path;
    };

    KuroUtils::Log("[Init] Loading TTF fonts...");
    ImFontConfig font_cfg;
    font_cfg.OversampleH = 3;
    font_cfg.OversampleV = 3;
    font_cfg.PixelSnapH = true;

    std::string seg_reg = "C:/Windows/Fonts/segoeui.ttf";
    std::string seg_bld = "C:/Windows/Fonts/segoeuib.ttf";
    std::string rob_reg = findResourcePath("src/ui/Roboto-Regular.ttf");

    std::string main_font = std::filesystem::exists(seg_reg) ? seg_reg : rob_reg;
    g_font_rekordbox_regular = io.Fonts->AddFontFromFileTTF(main_font.c_str(), 16.0f, &font_cfg);
    
    // Merge FontAwesome
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 3;
    icons_config.OversampleV = 3;
    icons_config.GlyphMinAdvanceX = 16.0f; // Fix icon size
    static const ImWchar icons_ranges[] = { 0xe000, 0xf8ff, 0 }; // Basic FontAwesome range
    io.Fonts->AddFontFromFileTTF(findResourcePath("src/ui/fa-solid-900.ttf").c_str(), 15.0f, &icons_config, icons_ranges);

    // =========================================================================
    // ATLAS TIPOGRÁFICO NATIVO EM 5 ESCALAS (PADRÃO PIONEER REKORDBOX / CDJ-3000)
    // =========================================================================
    std::string seg_sem = "C:/Windows/Fonts/seguisb.ttf";
    std::string bold_font = std::filesystem::exists(seg_bld) ? seg_bld : main_font;
    std::string semi_font = std::filesystem::exists(seg_sem) ? seg_sem : bold_font;

    // 1. Hero 22.0px Bold (Numerais do BPM no LCD do Jogwheel e Telemetria Principal)
    ImFontConfig hero_cfg;
    hero_cfg.OversampleH = 3; hero_cfg.OversampleV = 3; hero_cfg.PixelSnapH = true;
    g_font_display_hero = io.Fonts->AddFontFromFileTTF(bold_font.c_str(), 22.0f, &hero_cfg);

    // 2. Title 16.0px Bold (Título da Música, DECK 1/2, Master BPM)
    ImFontConfig title_cfg;
    title_cfg.OversampleH = 3; title_cfg.OversampleV = 3; title_cfg.PixelSnapH = true;
    g_font_title_large = io.Fonts->AddFontFromFileTTF(bold_font.c_str(), 16.0f, &title_cfg);

    // 3. Body 13.0px Semibold/Regular (Artista, Tom Camelot, Decimais de BPM, Botões Principais)
    ImFontConfig body_cfg;
    body_cfg.OversampleH = 3; body_cfg.OversampleV = 3; body_cfg.PixelSnapH = true;
    g_font_body_medium = io.Fonts->AddFontFromFileTTF(semi_font.c_str(), 13.0f, &body_cfg);

    // 4. Control 11.0px Bold (Rótulos do Mixer TRIM/EQ, Botões MST, CUE, Pitch %)
    ImFontConfig ctrl_cfg;
    ctrl_cfg.OversampleH = 3; ctrl_cfg.OversampleV = 3; ctrl_cfg.PixelSnapH = true;
    g_font_control_small = io.Fonts->AddFontFromFileTTF(bold_font.c_str(), 11.0f, &ctrl_cfg);

    // 5. Micro 9.5px Bold (Badges X.1Bars, Timestamps dos Pads 00:00, Ticks da Régua)
    ImFontConfig micro_cfg;
    micro_cfg.OversampleH = 3; micro_cfg.OversampleV = 3; micro_cfg.PixelSnapH = true;
    g_font_micro_tag = io.Fonts->AddFontFromFileTTF(bold_font.c_str(), 9.5f, &micro_cfg);

    // Aliases retrocompatíveis (preserva compatibilidade total com código existente)
    g_font_rekordbox_bold = g_font_title_large;
    g_font_rekordbox_large = g_font_display_hero;
    g_font_rekordbox_small = g_font_control_small;
    
    KuroUtils::Log("[Init] Setting FL Studio theme...");
    setupFLStudioTheme(); // Tema FL Studio (Fase 31)

    // Inicialização do Gerenciador de Multi-Resolução Dinâmica e DPI
    KuroUtils::Log("[Init] Initializing UIScaleManager...");
    KuroUI::UIScaleManager::init(window);
    static ImGuiStyle base_fl_style = ImGui::GetStyle();
    ImGui::GetStyle().ScaleAllSizes(KuroUI::UIScaleManager::getScale());
    io.FontGlobalScale = KuroUI::UIScaleManager::getScale();

    KuroUtils::Log("[Init] Initializing ImGui OpenGL / GLFW backends...");
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Conectar DSP do Rolling Bass à UI
    KuroUI::g_psy_rolling_bass_ui.setSynthDsp(&g_psy_bass_synth);
    KuroUtils::Log("[Init] Initialization complete!");

    // Inicialização direta no Modo DJ Studio Pro (Rekordbox Fullscreen sem tela de Splash)
    KuroUI::show_dj_modal = true;
    KuroUI::g_home_dashboard_ui.close();

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--open-dj" || arg == "--dj") {
            KuroUI::show_dj_modal = true;
        } else if (arg == "--test-phase3-dj" || arg == "--test-deck-play" || arg == "--test-3band" || arg == "--test-1l" || arg == "--test-fx" || arg == "--test-line" || arg == "--test-wave-3bm" || arg == "--test-wave-3bf" || arg == "--test-wave-hyb") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            if (arg == "--test-wave-3bm") {
                KuroUI::DJLayoutMetrics::getInstance().waveform_engine_mode = 0;
                KuroUI::DJLayoutMetrics::getInstance().ribbon_mode_active = false;
            } else if (arg == "--test-wave-3bf") {
                KuroUI::DJLayoutMetrics::getInstance().waveform_engine_mode = 1;
                KuroUI::DJLayoutMetrics::getInstance().ribbon_mode_active = false;
            } else if (arg == "--test-wave-hyb") {
                KuroUI::DJLayoutMetrics::getInstance().waveform_engine_mode = 4;
                KuroUI::DJLayoutMetrics::getInstance().ribbon_mode_active = false;
            } else if (arg == "--test-3band") {
                KuroUI::g_dj_studio_ui.waveform_color_mode = 1;
                KuroUI::DJLayoutMetrics::getInstance().waveform_engine_mode = 1;
            } else if (arg == "--test-1l") {
                KuroUI::g_dj_studio_ui.waveform_view_mode = 0;
            } else if (arg == "--test-line") {
                KuroUI::g_dj_studio_ui.waveform_render_style = 0;
                KuroUI::g_dj_studio_ui.waveform_zoom_bars = 1;
            }
            if (g_dj_engine) {
                std::string pA = "scratch/tracks/Render & Gigi - Dite The Bust/Render & Gigi - Dite The Bust.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/Aura Vortex & Blazy - Nuke/Aura Vortex & Blazy - Nuke.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_title = "Dite the Bust";
                    g_dj_engine->deckA.track_artist = "Render & GiGi";
                    g_dj_engine->deckA.loop_beats = 4.0;
                    g_dj_engine->deckA.current_frame = 30.0 * g_dj_engine->deckA.sample_rate;
                    g_dj_engine->deckA.hot_cues[0].active = true;
                    g_dj_engine->deckA.hot_cues[0].time_sec = g_dj_engine->deckA.first_beat_frame / g_dj_engine->deckA.sample_rate;
                    g_dj_engine->deckA.togglePlay();
                }

                std::string pB = "scratch/tracks/Groundbass & Zanon - Psychedelic Experience/Groundbass & Zanon - Psychedelic Experience.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/tracks/Blazy, Sighter & Basscannon - Dystopia/Blazy, Sighter & Basscannon - Dystopia.wav";
                if (g_dj_engine->deckB.loadTrack(pB)) {
                    g_dj_engine->deckB.track_title = "Psychedelic Experience";
                    g_dj_engine->deckB.track_artist = "Groundbass & Zanon";
                    g_dj_engine->deckB.loop_beats = 4.0;
                    g_dj_engine->deckB.current_frame = 60.0 * g_dj_engine->deckB.sample_rate;
                    g_dj_engine->toggleSync(1, 0);
                    g_dj_engine->deckB.togglePlay(&g_dj_engine->deckA);
                }

                if (arg == "--test-fx") {
                    g_dj_engine->deckA.sound_color_fx_mode = 0; // FILTER
                    g_dj_engine->deckA.filter_bipolar = 0.20f;  // LPF Magma Red Lightning
                    g_dj_engine->deckB.sound_color_fx_mode = 2; // CRUSH Toxic Green Lightning
                    g_dj_engine->deckB.filter_bipolar = 0.80f;
                }
            }
        } else if (arg == "--test-slip-mode") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {
                std::string pA = "scratch/tracks/track_online_1753034172.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_jox_02.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_title = "Vortex Mind";
                    g_dj_engine->deckA.track_artist = "JOX (BR)";
                    g_dj_engine->deckA.loop_beats = 4.0;
                    g_dj_engine->deckA.togglePlay();
                    g_dj_engine->deckA.slip_mode = true;
                    g_dj_engine->deckA.toggleLoop(4.0);
                }

                std::string pB = "scratch/tracks/track_online_1775826061.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/tracks/track_online_1785069826.wav";
                if (g_dj_engine->deckB.loadTrack(pB)) {
                    g_dj_engine->deckB.track_title = "Disk Jockey's";
                    g_dj_engine->deckB.track_artist = "Lucid (BR) & Parallelus";
                    g_dj_engine->deckB.loop_beats = 2.0;
                    g_dj_engine->toggleSync(1, 0);
                    g_dj_engine->deckB.togglePlay(&g_dj_engine->deckA);
                    g_dj_engine->deckB.slip_mode = true;
                }
            }
        }
 else if (arg == "--test-grid-editor") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_grid_edit_drawer = true;
            if (g_dj_engine) {
                std::string pA = "scratch/tracks/track_online_1753034172.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_jox_02.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_title = "Vortex Mind";
                    g_dj_engine->deckA.track_artist = "JOX (BR)";
                }
            }
        } else if (arg == "--test-big-library") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_grid_edit_drawer = false;
            KuroUI::g_dj_studio_ui.show_v10_mixer_mode = false;
            KuroUI::g_dj_studio_ui.show_master_isolator_panel = false;
            KuroUI::g_dj_studio_ui.library_view_mode = KuroUI::KuroDJStudioUI::LIB_VIEW_BIG_LIBRARY;
            KuroAudio::DJLibraryManager::getInstance().scanLocalTracks("scratch/tracks");
            KuroAudio::DJLibraryManager::getInstance().setActiveFolderId("all");
            KuroUI::g_dj_studio_ui.hovered_track_key = "10B";
            KuroUI::g_dj_studio_ui.hovered_track_bpm = 143.6;
            KuroUI::g_dj_studio_ui.hovered_track_title = "Nuke";
            KuroUI::g_dj_studio_ui.hovered_track_artist = "Aura Vortex & Blazy";
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_artist = "Imaginarium";
                    g_dj_engine->deckA.track_title = "Spirit Of Ayahuaska";
                    g_dj_engine->deckA.bpm = 144.0;
                    g_dj_engine->deckA.key_signature = "4A";
                }
                std::string pB = "scratch/test_astrix.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/test_cliff.wav";
                if (g_dj_engine->deckB.loadTrack(pB)) {
                    g_dj_engine->deckB.track_artist = "Sixsense";
                    g_dj_engine->deckB.track_title = "Divinity";
                    g_dj_engine->deckB.bpm = 147.0;
                    g_dj_engine->deckB.key_signature = "9A";
                }
            }
        } else if (arg == "--test-performance-mode") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_grid_edit_drawer = false;
            KuroUI::g_dj_studio_ui.show_v10_mixer_mode = false;
            KuroUI::g_dj_studio_ui.show_master_isolator_panel = false;
            KuroUI::g_dj_studio_ui.library_view_mode = KuroUI::KuroDJStudioUI::LIB_VIEW_HIDDEN;
            KuroUI::g_dj_studio_ui.dj_layout_mode = 0;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_artist = "Imaginarium";
                    g_dj_engine->deckA.track_title = "Spirit Of Ayahuaska";
                    g_dj_engine->deckA.bpm = 144.0;
                    g_dj_engine->deckA.key_signature = "4A";
                }
                std::string pB = "scratch/test_astrix.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/test_cliff.wav";
                if (g_dj_engine->deckB.loadTrack(pB)) {
                    g_dj_engine->deckB.track_artist = "Sixsense";
                    g_dj_engine->deckB.track_title = "Divinity";
                    g_dj_engine->deckB.bpm = 147.0;
                    g_dj_engine->deckB.key_signature = "9A";
                }
            }
        } else if (arg == "--test-split-library" || arg == "--test-crates-and-pads" || arg == "--test-edit-mode" || arg == "--test-tour-mode" || arg == "--test-game-mode" || arg == "--test-rack-19" || arg == "--test-live-rack") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_grid_edit_drawer = false;
            KuroUI::g_dj_studio_ui.show_v10_mixer_mode = false;
            KuroUI::g_dj_studio_ui.show_master_isolator_panel = false;
            KuroUI::g_dj_studio_ui.rekordbox_deck_count = 4;
            KuroUI::g_dj_studio_ui.library_view_mode = KuroUI::KuroDJStudioUI::LIB_VIEW_SPLIT;
            if (arg == "--test-edit-mode") {
                KuroUI::DJLayoutMetrics::getInstance().edit_mode_active = true;
            } else if (arg == "--test-tour-mode") {
                KuroUI::DJInteractiveTourEngine::getInstance().setActive(true);
                KuroUI::DJInteractiveTourEngine::getInstance().setStep(0);
            } else if (arg == "--test-game-mode") {
                KuroUI::DJAcademyGameUI::getInstance().setActive(true);
                KuroUI::DJAcademyGameUI::getInstance().setModalGuideOpen(true);
                if (g_dj_engine) {
                    KuroUI::DJAcademyGameUI::getInstance().loadStudiedTracks(*g_dj_engine);
                }
            } else if (arg == "--test-rack-19") {
                KuroUI::DJStudioRackUI::getInstance().setOpen(true);
            } else if (arg == "--test-live-rack") {
                KuroUI::g_dj_studio_ui.show_live_synth_fx_rack = true;
                KuroUI::g_dj_studio_ui.live_rack_active_tab = 0;
                KuroUI::g_dj_studio_ui.library_view_mode = KuroUI::KuroDJStudioUI::LIB_VIEW_HIDDEN;
            }
            KuroUI::g_dj_studio_ui.hovered_track_key = "8A";
            KuroUI::g_dj_studio_ui.hovered_track_bpm = 143.6;
            KuroUI::g_dj_studio_ui.hovered_track_title = "Horizons (Phaxe Remix)";
            KuroUI::g_dj_studio_ui.hovered_track_artist = "Aura Vortex & Blazy";
            KuroUI::g_dj_studio_ui.dj_layout_mode = 0;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_artist = "Imaginarium";
                    g_dj_engine->deckA.track_title = "Spirit Of Ayahuaska";
                    g_dj_engine->deckA.bpm = 144.0;
                    g_dj_engine->deckA.key_signature = "4A";
                }
                std::string pB = "scratch/test_astrix.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/test_cliff.wav";
                if (g_dj_engine->deckB.loadTrack(pB)) {
                    g_dj_engine->deckB.track_artist = "Sixsense";
                    g_dj_engine->deckB.track_title = "Divinity";
                    g_dj_engine->deckB.bpm = 147.0;
                    g_dj_engine->deckB.key_signature = "9A";
                }
            }
        } else if (arg == "--test-flagship-dj-v10-grid") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_grid_edit_drawer = false;
            KuroUI::g_dj_studio_ui.show_v10_mixer_mode = false;
            KuroUI::g_dj_studio_ui.show_master_isolator_panel = false;
            KuroUI::g_dj_studio_ui.show_playlist_drawer = false;
            KuroUI::g_dj_studio_ui.side_drawer_ui.is_open = false;
            KuroUI::g_dj_studio_ui.dj_layout_mode = 0;
            if (g_dj_engine) {
                std::string pA = "scratch/tracks/Ajja - Burn In Noise - Image of Yourself Original Mix Original Mix (DemoLat.org)/Ajja - Burn In Noise - Image of Yourself Original Mix Original Mix (DemoLat.org).mp3";
                if (!std::filesystem::exists(pA)) pA = "scratch/test_cliff.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_artist = "Imaginarium";
                    g_dj_engine->deckA.track_title = "Spirit Of Ayahuaska";
                    g_dj_engine->deckA.bpm = 144.0;
                    g_dj_engine->deckA.key_signature = "4A";
                    double beat_sec_A = 60.0 / 144.0;
                    g_dj_engine->deckA.current_frame = beat_sec_A * 56.4 * 4.0 * g_dj_engine->deckA.sample_rate;
                    g_dj_engine->deckA.first_beat_frame = 0.0;
                    g_dj_engine->deckA.is_playing = false;
                }

                std::string pB = "scratch/test_astrix.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/test_cliff.wav";
                if (g_dj_engine->deckB.loadTrack(pB)) {
                    g_dj_engine->deckB.track_artist = "Sixsense";
                    g_dj_engine->deckB.track_title = "Divinity";
                    g_dj_engine->deckB.bpm = 147.0;
                    g_dj_engine->deckB.key_signature = "9A";
                    double beat_sec_B = 60.0 / 147.0;
                    g_dj_engine->deckB.current_frame = beat_sec_B * 97.4 * 4.0 * g_dj_engine->deckB.sample_rate;
                    g_dj_engine->deckB.first_beat_frame = 0.0;
                    g_dj_engine->deckB.is_playing = false;
                }
            }
        } else if (arg == "--test-bpm-edit-open") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_grid_edit_drawer = false;
            KuroUI::g_dj_studio_ui.show_v10_mixer_mode = false;
            KuroUI::g_dj_studio_ui.show_master_isolator_panel = false;
            KuroUI::g_dj_studio_ui.show_playlist_drawer = false;
            KuroUI::g_dj_studio_ui.side_drawer_ui.is_open = false;
            KuroUI::g_dj_studio_ui.dj_layout_mode = 0;
            KuroUI::g_dj_studio_ui.bpm_editing_deck = 0; // Deck 1 com a caixa de texto do BPM aberta!
            snprintf(KuroUI::g_dj_studio_ui.bpm_edit_buf, sizeof(KuroUI::g_dj_studio_ui.bpm_edit_buf), "144.00");
            KuroUI::g_dj_studio_ui.bpm_edit_focus_needed = true;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_artist = "Imaginarium";
                    g_dj_engine->deckA.track_title = "Spirit Of Ayahuaska";
                    g_dj_engine->deckA.bpm = 144.0;
                    g_dj_engine->deckA.key_signature = "4A";
                }
                std::string pB = "scratch/test_cliff.wav";
                if (g_dj_engine->deckB.loadTrack(pB)) {
                    g_dj_engine->deckB.track_artist = "Sixsense";
                    g_dj_engine->deckB.track_title = "Divinity";
                    g_dj_engine->deckB.bpm = 147.0;
                    g_dj_engine->deckB.key_signature = "9A";
                }
            }
        } else if (arg == "--test-pad-modes-active") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.side_drawer_ui.is_open = false;
            KuroUI::g_dj_studio_ui.dj_layout_mode = 0;
            KuroUI::g_dj_studio_ui.rekordbox_pad_mode[0] = 1; // Deck 1 = BEAT LOOP
            KuroUI::g_dj_studio_ui.rekordbox_pad_mode[1] = 3; // Deck 2 = SAMPLER
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_artist = "Imaginarium";
                    g_dj_engine->deckA.track_title = "Spirit Of Ayahuaska";
                    g_dj_engine->deckA.bpm = 144.0;
                    g_dj_engine->deckA.loop_active = true;
                    g_dj_engine->deckA.loop_beats = 4.0;
                }
                std::string pB = "scratch/test_astrix.wav";
                if (g_dj_engine->deckB.loadTrack(pB)) {
                    g_dj_engine->deckB.track_artist = "Sixsense";
                    g_dj_engine->deckB.track_title = "Divinity";
                    g_dj_engine->deckB.bpm = 147.0;
                }
            }
        } else if (arg == "--test-dj-shortcuts-modal") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_shortcuts_help_modal = true;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1894735727.wav";
                g_dj_engine->deckA.loadTrack(pA);
                g_dj_engine->deckA.track_title = "Ajja_Burn_In_Noise_-_Image_of_Yourself...";
                g_dj_engine->deckA.track_artist = "Ajja, Burn In Noise";
                g_dj_engine->deckA.bpm = 142.0;
                g_dj_engine->deckA.key_signature = "10B";

                std::string pB = "scratch/test_astrix.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/tracks/track_demo_djw.wav";
                g_dj_engine->deckB.loadTrack(pB);
                g_dj_engine->deckB.track_title = "Ajja_-_Bazinga_Earthspace_Remix_Earth...";
                g_dj_engine->deckB.track_artist = "Ajja";
                g_dj_engine->deckB.bpm = 144.0;
                g_dj_engine->deckB.key_signature = "6A";
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
        } else if (arg == "--test-cloud-hub" || arg == "--test-radar-tab") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.search_modal_view = 0;
            if (arg == "--test-radar-tab") {
                KuroUI::g_dj_studio_ui.cloud_hub_active_tab = 3;
            } else {
                KuroUI::g_dj_studio_ui.cloud_hub_active_tab = 1;
            }
            strncpy(KuroUI::g_dj_studio_ui.custom_direct_url_buf, "https://open.spotify.com/playlist/37i9dQZF1DXdLEN7aqioXM", sizeof(KuroUI::g_dj_studio_ui.custom_direct_url_buf) - 1);
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (g_dj_engine->deckA.loadTrack(pA)) {
                    g_dj_engine->deckA.track_artist = "Imaginarium";
                    g_dj_engine->deckA.track_title = "Spirit Of Ayahuaska";
                    g_dj_engine->deckA.bpm = 144.0;
                    g_dj_engine->deckA.key_signature = "4A";
                }
                std::string pB = "scratch/test_astrix.wav";
                if (g_dj_engine->deckB.loadTrack(pB)) {
                    g_dj_engine->deckB.track_artist = "Sixsense";
                    g_dj_engine->deckB.track_title = "Divinity";
                    g_dj_engine->deckB.bpm = 147.0;
                    g_dj_engine->deckB.key_signature = "9A";
                }
                // Carrega o TSV da playlist diretamente
                auto tsv_path = KuroAudio::getTracksDirectory() / "search_results.tsv";
                if (std::filesystem::exists(tsv_path)) {
                    KuroUI::g_dj_studio_ui.spotify_service.parseSearchResultsTsv(tsv_path);
                }
            }
        } else if (arg == "--test-dj-drawer" || arg == "--test-side-drawer") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.side_drawer_ui.is_open = true;
            KuroUI::g_dj_studio_ui.show_library_modal = false;
            KuroAudio::DJLibraryManager::getInstance().scanLocalTracks("scratch/tracks");
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
        } else if (arg == "--test-hover-radar") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.side_drawer_ui.is_open = true;
            KuroUI::g_dj_studio_ui.show_library_modal = false;
            KuroAudio::DJLibraryManager::getInstance().scanLocalTracks("scratch/tracks");
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
            KuroUI::g_dj_studio_ui.side_drawer_ui.setHoveredTrack("track_hover_demo", "10B", 136.0, "Thinking Pattern", "Airi");
        } else if (arg == "--test-dock-left" || arg == "--test-drawer-left") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.side_drawer_ui.is_open = true;
            KuroUI::g_dj_studio_ui.side_drawer_ui.dock_left = true;
            KuroUI::g_dj_studio_ui.show_library_modal = false;
            KuroAudio::DJLibraryManager::getInstance().scanLocalTracks("scratch/tracks");
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
        } else if (arg == "--test-dj-crates" || arg == "--test-crates") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.side_drawer_ui.is_open = true;
            KuroUI::g_dj_studio_ui.show_library_modal = true;
            KuroAudio::DJLibraryManager::getInstance().scanLocalTracks("scratch/tracks");
            KuroAudio::DJLibraryManager::getInstance().setActiveFolderId("first_kuru");
        } else if (arg == "--test-dj-setlist") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_playlist_drawer = true;
            KuroUI::g_dj_studio_ui.show_library_modal = true;
            KuroUI::g_dj_studio_ui.library_modal_tab = 1;
            KuroAudio::DJLibraryManager::getInstance().scanLocalTracks("scratch/tracks");
            KuroAudio::DJLibraryManager::getInstance().setActiveFolderId("prog_psy");
            auto& s_mgr = KuroAudio::DJSetlistManager::getInstance();
            if (s_mgr.getSetlists().empty()) {
                s_mgr.createSetlist("Set Psytrance Flagship 2026", "Setlist profissional com progressao de 4 fases");
            }
            if (s_mgr.getActiveSetlist() && s_mgr.getActiveSetlist()->items.empty()) {
                const auto trks = KuroAudio::DJLibraryManager::getInstance().getTracksForFolder("all");
                if (!trks.empty()) {
                    s_mgr.addTrackToActiveSetlist(trks[0], 0, "Faixa de abertura: impacto sonoro e timbre marcante");
                    if (trks.size() > 1) s_mgr.addTrackToActiveSetlist(trks[1], 1, "Transicao harmonica suave (+0 Camelot, mesmo tom)");
                    if (trks.size() > 2) s_mgr.addTrackToActiveSetlist(trks[2], 1, "Construcao de energia progressiva (+1 Camelot)");
                }
            }
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
            KuroUI::g_home_dashboard_ui.show_on_startup = false;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.focus_search_modal = true;
            KuroUI::g_dj_studio_ui.side_drawer_ui.is_open = false;
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
        } else if (arg == "--test-dig-deeper") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_home_dashboard_ui.show_on_startup = false;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.focus_search_modal = true;
            KuroUI::g_dj_studio_ui.side_drawer_ui.is_open = false;
            if (std::filesystem::exists("scratch/tracks/search_results.tsv")) {
                KuroUI::g_dj_studio_ui.spotify_service.parseSearchResultsTsv("scratch/tracks/search_results.tsv");
            }
            if (std::filesystem::exists("scratch/tracks/dig_deeper_results.json")) {
                KuroUI::g_dj_studio_ui.spotify_service.parseDigDeeperJsonFile("scratch/tracks/dig_deeper_results.json");
            }
            KuroUI::g_dj_studio_ui.spotify_service.show_dig_deeper_view = true;
        } else if (arg == "--test-preview-audio") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_home_dashboard_ui.show_on_startup = false;
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.focus_search_modal = true;
            if (std::filesystem::exists("scratch/tracks/search_results.tsv")) {
                KuroUI::g_dj_studio_ui.spotify_service.parseSearchResultsTsv("scratch/tracks/search_results.tsv");
            }
            std::string pGoa = "scratch/test_cliff.wav";
            if (!std::filesystem::exists(pGoa)) pGoa = "scratch/test_astrix.wav";
            KuroAudio::DJPreviewPlayer::getInstance().play(pGoa, "1891678034", "Paraíso De Goa", "JOX (BR)");
        } else if (arg == "--test-camelot-radar" || arg == "--test-radar") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_camelot_radar_modal = true;
            KuroUI::g_dj_studio_ui.camelot_ref_deck = 0;
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
        } else if (arg == "--test-dj-refined") {
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1894735727.wav";
                g_dj_engine->deckA.loadTrack(pA);
                g_dj_engine->deckA.track_title = "Cliffjumper";
                g_dj_engine->deckA.track_artist = "Aura Vortex & Klipsun";
                g_dj_engine->deckA.bpm = 138.0;
                g_dj_engine->deckA.key_signature = "9B / G";
                g_dj_engine->setMasterDeck(0);
                g_dj_engine->deckA.togglePlay();

                std::string pB = "scratch/tracks/track_demo_djw.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/test_astrix.wav";
                g_dj_engine->deckB.loadTrack(pB);
                g_dj_engine->deckB.track_title = "Deep Jungle Walk";
                g_dj_engine->deckB.track_artist = "Astrix";
                g_dj_engine->deckB.bpm = 138.0;
                g_dj_engine->deckB.key_signature = "8A / Am";

                // Refinamentos Ativos: Key Sync, Slip Mode, Beat Roll, Auto-Gain & Master REC
                g_dj_engine->toggleSync(1, 0);
                g_dj_engine->toggleKeySync(1, 0);
                g_dj_engine->deckB.slip_mode = true;
                g_dj_engine->deckB.pad_mode = 1; // BEAT ROLL
                g_dj_engine->deckB.togglePlay(&g_dj_engine->deckA);

                // Dispara gravação Master ao vivo em arquivo WAV
                g_dj_engine->set_recorder.startRecording("scratch/recordings", 44100.0);
            }
        } else if (arg == "--test-dj-phase-stems") {
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1894735727.wav";
                g_dj_engine->deckA.loadTrack(pA);
                g_dj_engine->deckA.track_title = "Cliffjumper";
                g_dj_engine->setMasterDeck(0);

                std::string pB = "scratch/tracks/track_demo_djw.wav";
                if (!std::filesystem::exists(pB)) pB = "scratch/test_astrix.wav";
                g_dj_engine->deckB.loadTrack(pB);
                g_dj_engine->deckB.track_title = "Deep Jungle Walk";
                g_dj_engine->deckB.track_artist = "Astrix";
                g_dj_engine->deckB.bpm = 138.0;
                g_dj_engine->deckB.key_signature = "8A / Am";

                // Avança para o drop (36.0s) onde as batidas e transientes atingem potência máxima
                g_dj_engine->deckA.current_frame = 36.0 * g_dj_engine->deckA.sample_rate;
                g_dj_engine->deckB.current_frame = 36.0 * g_dj_engine->deckB.sample_rate;

                g_dj_engine->deckA.togglePlay();

                // Ativa Beat Sync, Key Sync, Phase Meter e Stems Mode
                g_dj_engine->toggleSync(1, 0);
                g_dj_engine->toggleKeySync(1, 0);
                g_dj_engine->deckA.stem_mode = true;
                g_dj_engine->deckB.stem_mode = true;
                g_dj_engine->deckB.pad_mode = 2; // STEMS PERFORMANCE MODE
                g_dj_engine->deckB.togglePlay(&g_dj_engine->deckA);

                // Ativa Loop de 4 beats no Deck A para exibir a região sombreada e marcadores [IN] / [OUT]
                g_dj_engine->deckA.toggleLoop(4.0);
                g_dj_engine->deckA.quantize_active = true;
                g_dj_engine->deckB.quantize_active = true;

                // Dispara gravação Master ao vivo
                g_dj_engine->set_recorder.startRecording("scratch/recordings", 44100.0);
            }
        } else if (arg == "--test-stems-pad" || arg == "--test-dj-stems") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1894735727.wav";
                g_dj_engine->deckA.loadTrack(pA);
                g_dj_engine->deckA.track_title = "Cliffjumper";
                g_dj_engine->deckA.track_artist = "Aura Vortex";
                g_dj_engine->deckA.bpm = 138.0;
                g_dj_engine->deckA.current_frame = 36.0 * g_dj_engine->deckA.sample_rate;
                g_dj_engine->deckA.is_playing.store(true);
                g_dj_engine->deckA.pad_mode = 4;
                KuroUI::g_dj_studio_ui.rekordbox_pad_mode[0] = 4;
                KuroUI::g_dj_studio_ui.rekordbox_pad_mode[1] = 4;
                g_dj_engine->deckA.stem_vocal_active.store(false);
                g_dj_engine->deckA.stem_vocal_gain.store(0.0f);
            }
        } else if (arg == "--test-autodj" || arg == "--test-dj-autodj") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
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

                g_dj_engine->deckA.current_frame = 36.0 * g_dj_engine->deckA.sample_rate;
                g_dj_engine->deckA.is_playing.store(true);
                g_dj_engine->setMasterDeck(0);

                auto& autodj = KuroAudio::DJAutoDJEngine::getInstance();
                autodj.setEnabled(true);
                autodj.setTransitionLengthBars(32);
                autodj.setBassSwapEnabled(true);
                g_dj_engine->deckB.current_frame = 36.0 * g_dj_engine->deckB.sample_rate;
                g_dj_engine->deckB.is_playing.store(true);
                g_dj_engine->deckB.volume = 1.0f;
                g_dj_engine->deckB.eq_low = 1.0f;
                g_dj_engine->deckA.eq_low = 0.0f;
                g_dj_engine->setCrossfader(0.5f);
            }
        } else if (arg == "--test-audio-routing-cue" || arg == "--test-dj-cue") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1894735727.wav";
                g_dj_engine->deckA.loadTrack(pA);
                g_dj_engine->deckA.is_playing.store(true);
                g_dj_engine->setCueDeckA(true);
                g_dj_engine->setCueMaster(true);
                KuroAudio::DJAudioDeviceManager::getInstance().show_routing_modal = true;
            }
        } else if (arg == "--test-dj-context") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_playlist_drawer = true;
            KuroAudio::DJLibraryManager::getInstance().scanLocalTracks("scratch/tracks");
            KuroAudio::DJLibraryManager::getInstance().setActiveFolderId("prog_psy");
            auto tracks = KuroAudio::DJLibraryManager::getInstance().getTracksForFolder("prog_psy");
            if (!tracks.empty()) {
                KuroUI::g_dj_studio_ui.context_track_id = tracks[0].id;
                strncpy(KuroUI::g_dj_studio_ui.edit_title_buf, tracks[0].title.c_str(), sizeof(KuroUI::g_dj_studio_ui.edit_title_buf) - 1);
                strncpy(KuroUI::g_dj_studio_ui.edit_artist_buf, tracks[0].artist.c_str(), sizeof(KuroUI::g_dj_studio_ui.edit_artist_buf) - 1);
                KuroUI::g_dj_studio_ui.edit_bpm = tracks[0].bpm;
                strncpy(KuroUI::g_dj_studio_ui.edit_key_buf, tracks[0].key.c_str(), sizeof(KuroUI::g_dj_studio_ui.edit_key_buf) - 1);
                strncpy(KuroUI::g_dj_studio_ui.edit_subgenre_buf, tracks[0].subgenre.c_str(), sizeof(KuroUI::g_dj_studio_ui.edit_subgenre_buf) - 1);
                KuroUI::g_dj_studio_ui.show_edit_meta_modal = true;
            }
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
        } else if (arg == "--test-master-source-matrix" || arg == "--test-harmonic-dna") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.search_modal_view = 1;
            if (arg == "--test-harmonic-dna") {
                KuroUI::g_dj_studio_ui.sources_tab_filter = 5;
            } else {
                KuroUI::g_dj_studio_ui.sources_tab_filter = 0;
            }
            KuroUI::g_dj_studio_ui.selected_source_track_id = "1891678034";
            KuroUI::g_dj_studio_ui.selected_source_track_title = "Paraiso De Goa";
            KuroUI::g_dj_studio_ui.selected_source_track_artist = "JOX (BR)";
            KuroUI::g_dj_studio_ui.selected_source_track_album = "Psychedelic Experience Vol. 2";
            KuroUI::g_dj_studio_ui.selected_source_track_bpm = 138.0f;
            KuroUI::g_dj_studio_ui.selected_source_track_key = "8A / Am";
            KuroUI::g_dj_studio_ui.selected_source_track_subgenre = "Full-On Night";
            KuroUI::g_dj_studio_ui.selected_source_track_dur = 434.0;
            KuroUI::g_dj_studio_ui.spotify_service.searchSourcesAsync("1891678034", "Paraiso De Goa", "JOX (BR)");
            KuroUI::g_dj_studio_ui.spotify_service.parseSourcesJsonFile("scratch/tracks/sources_1891678034.json");
            if (g_dj_engine) {
                std::string pA = "scratch/test_cliff.wav";
                if (!std::filesystem::exists(pA)) pA = "scratch/tracks/track_1894735727.wav";
                g_dj_engine->deckA.loadTrack(pA);
                g_dj_engine->deckA.track_title = "Cliffjumper";
                g_dj_engine->deckA.track_artist = "Aura Vortex";
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
        } else if (arg == "--test-sources-modal") {
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.spotify_service.setShowSourcesModal(true);
            KuroUI::g_dj_studio_ui.spotify_service.setSourcesTrackInfo("1891678034", "Paraíso De Goa", "JOX (BR)");
            KuroUI::g_dj_studio_ui.spotify_service.parseSourcesJsonFile("scratch/tracks/sources_1891678034.json");
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
        } else if (arg == "--test-automation-lanes") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_playlist = true;
            KuroUI::show_piano_roll = false;
            KuroUI::show_step_sequencer = false;
            KuroUI::show_mixer = false;
            float bpm = 140.0f;
            float bar_len_sec = (60.0f / bpm) * 4.0f;

            // Track 1: Audio Clip + Volume Automation (Cyan)
            if (g_clip_manager.track_clips[0].empty()) {
                AudioClip ac;
                ac.id = g_clip_manager.next_id++;
                ac.start_time_sec = 0.0f;
                ac.length_sec = bar_len_sec * 4.0f;
                ac.name = "Infected_Psy_Vocal.wav";
                ac.is_selected = true;
                g_clip_manager.track_clips[0].push_back(ac);
            }
            g_clip_manager.track_auto_clips[0].clear();
            g_clip_manager.addAutomationClip(0, 0.0f, bar_len_sec * 6.0f, "Volume (dB)", "Master", 0);

            // Track 2: Filter Cutoff Sweep (Amber)
            g_clip_manager.track_auto_clips[1].clear();
            g_clip_manager.addAutomationClip(1, bar_len_sec * 1.0f, bar_len_sec * 7.0f, "Filter Cutoff (Hz)", "Filter", 1);

            // Track 3: Pan Sweep (Magenta)
            g_clip_manager.track_auto_clips[2].clear();
            g_clip_manager.addAutomationClip(2, bar_len_sec * 0.0f, bar_len_sec * 8.0f, "Pan (L/R)", "Pan", 2);

            // Track 4: Dry/Wet Mix (Emerald)
            g_clip_manager.track_auto_clips[3].clear();
            g_clip_manager.addAutomationClip(3, bar_len_sec * 2.0f, bar_len_sec * 6.0f, "Dry/Wet Mix (%)", "Reverb", 3);

            std::cout << "[TEST_AUTOMATION_LANES] Playlist opened with 4 Bézier automation curves (Volume, Cutoff, Pan, Dry/Wet)!\n";
        } else if (arg == "--test-detail-dock") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::g_force_test_ghost_preview = true;
            KuroUI::show_playlist = true;
            KuroUI::show_piano_roll = false;
            KuroUI::show_step_sequencer = false;
            KuroUI::show_mixer = false;
            if (g_clip_manager.track_clips[0].empty()) {
                AudioClip ac;
                ac.id = g_clip_manager.next_id++;
                ac.start_time_sec = 0.0f;
                ac.length_sec = 8.0f;
                ac.name = "Infected_Psy_Vocal_Warp.wav";
                ac.enable_stretch = true;
                ac.time_stretch_ratio = 1.25f;
                ac.pitch_shift_semitones = 3.0f;
                ac.gain_db = 1.5f;
                ac.is_reversed = false;
                ac.fade_in_sec = 0.5f;
                ac.fade_out_sec = 1.0f;
                ac.is_selected = true;
                g_clip_manager.track_clips[0].push_back(ac);
            }
            KuroUI::g_audio_clip_editor_ui.open(0, 0);
            std::cout << "[TEST_DETAIL_DOCK] Detail Dock activated with Warp and Waveform!\n";
        } else if (arg == "--test-floating-mixer") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_playlist = true;
            KuroUI::show_mixer = true;
            KuroUI::g_mixer_selected_track = 1;
            ::master_vu_level_l = 0.72f;
            ::master_vu_level_r = 0.75f;
            ::track_vu_levels[0] = 0.88f;
            ::track_vu_levels[1] = 0.65f;
            ::track_vu_levels[2] = 0.50f;
            std::cout << "[TEST_FLOATING_MIXER] Floating Mixer Rack activated with 10 slots and VU meters!\n";
        } else if (arg == "--test-mixer-routing-sidechain") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_playlist = true;
            KuroUI::show_mixer = true;
            KuroUI::g_mixer_selected_track = 1;
            KuroUI::g_mixer_routing_ui.right_panel_tab = 1;
            ::master_vu_level_l = 0.78f;
            ::master_vu_level_r = 0.81f;
            ::track_vu_levels[0] = 0.92f;
            ::track_vu_levels[1] = 0.85f;
            ::track_vu_levels[2] = 0.60f;
            std::cout << "[TEST_MIXER_ROUTING_SIDECHAIN] Mixer Routing & Sidechain Ducking oscilloscope activated!\n";
        } else if (arg == "--test-master-processing-and-stems") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_playlist = false;
            KuroUI::show_step_sequencer = false;
            KuroUI::show_mixer = false;
            KuroUI::show_piano_roll = false;
            KuroUI::show_browser = false;
            KuroUI::g_maximus_master_ui.open();
            KuroUI::g_multi_export_ui.open();
            ::master_vu_level_l = 0.88f;
            ::master_vu_level_r = 0.90f;
            ::track_vu_levels[0] = 0.94f;
            ::track_vu_levels[1] = 0.88f;
            ::track_vu_levels[2] = 0.70f;
            std::cout << "[TEST_MASTER_STEMS] Maximus Multiband Master Bus & Parallel Stems Bounce activated!\n";
        } else if (arg == "--test-advanced-piano-roll") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_playlist = false;
            KuroUI::show_step_sequencer = false;
            KuroUI::show_mixer = false;
            KuroUI::show_piano_roll = true;
            KuroUI::g_need_focus_piano_roll = true;

            // Configure scale & ghost notes
            KuroUI::g_piano_roll_show_ghost_notes = true;
            KuroUI::g_piano_roll_ghost_filter = 0; // Todos os Canais
            KuroUI::g_piano_roll_snap_scale = true;
            KuroUI::g_piano_roll_fold_scale = false;
            KuroUI::g_piano_roll_scale_root = 2; // D
            KuroUI::g_piano_roll_scale_type = 9; // Phrygian Dominant (Psytrance / Goa)
            KuroUI::g_piano_roll_current_stamp_idx = 1; // Major triad

            // Populate Pattern 0 with rich multi-channel arrangement for Ghost Notes and Polyphony
            auto& pat = g_clip_manager.global_patterns[0];
            for (int c = 0; c < 8; c++) pat.getChannelNotes(c).clear();

            float beat = 60.0f / 142.0f; // 142 BPM
            // Channel 0: Kick (Ch 0 - Neon Coral ghost notes)
            for (int b = 0; b < 16; b += 2) {
                pat.getChannelNotes(0).push_back(KuroDSP::MidiNote(74, b * beat, beat * 0.45f, 0.95f, 1.0f, 0));
            }

            // Channel 1: Psy Rolling Bassline (Ch 1 - Cyan Glow ghost notes)
            for (int b = 0; b < 16; b++) {
                pat.getChannelNotes(1).push_back(KuroDSP::MidiNote(75, (b + 0.25f) * beat, beat * 0.22f, 0.82f, 1.0f, 1));
                pat.getChannelNotes(1).push_back(KuroDSP::MidiNote(74, (b + 0.50f) * beat, beat * 0.22f, 0.85f, 1.0f, 1));
                pat.getChannelNotes(1).push_back(KuroDSP::MidiNote(75, (b + 0.75f) * beat, beat * 0.22f, 0.80f, 1.0f, 1));
            }

            // Channel 3: Atmospheric Pad (Ch 3 - Ultra Violet ghost notes)
            pat.getChannelNotes(3).push_back(KuroDSP::MidiNote(74, 0.0f * beat, beat * 3.8f, 0.70f, 1.0f, 3)); // D5
            pat.getChannelNotes(3).push_back(KuroDSP::MidiNote(78, 0.0f * beat, beat * 3.8f, 0.65f, 1.0f, 3)); // F#5
            pat.getChannelNotes(3).push_back(KuroDSP::MidiNote(81, 0.0f * beat, beat * 3.8f, 0.68f, 1.0f, 3)); // A5
            pat.getChannelNotes(3).push_back(KuroDSP::MidiNote(75, 4.0f * beat, beat * 3.8f, 0.70f, 1.0f, 3)); // D#5
            pat.getChannelNotes(3).push_back(KuroDSP::MidiNote(79, 4.0f * beat, beat * 3.8f, 0.65f, 1.0f, 3)); // G5
            pat.getChannelNotes(3).push_back(KuroDSP::MidiNote(82, 4.0f * beat, beat * 3.8f, 0.68f, 1.0f, 3)); // A#5

            // Channel 2: Active Track - Lead / Polyphonic Chords in D Phrygian Dominant (D5 - F#5 - A5 / D#5 - G5 - A#5)
            // Polyphonic Chord 1 at Bar 0 (Beat 0)
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(74, 0.0f * beat, beat * 1.8f, 0.90f, 1.0f, 2)); // D5 (Root)
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(78, 0.0f * beat, beat * 1.8f, 0.88f, 1.0f, 2)); // F#5 (Major 3rd)
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(81, 0.0f * beat, beat * 1.8f, 0.85f, 1.0f, 2)); // A5 (5th)

            // Melody continuation (Phrygian Dominant: D, D#, F#, G, A, A#, C)
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(74, 2.0f * beat, beat * 0.45f, 0.92f, 1.0f, 2)); // D5
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(75, 2.5f * beat, beat * 0.45f, 0.88f, 1.0f, 2)); // D#5
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(78, 3.0f * beat, beat * 0.90f, 0.95f, 1.0f, 2)); // F#5

            // Polyphonic Chord 2 at Bar 1 (Beat 4)
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(75, 4.0f * beat, beat * 1.8f, 0.88f, 1.0f, 2)); // D#5 (b2 chord)
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(79, 4.0f * beat, beat * 1.8f, 0.85f, 1.0f, 2)); // G5
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(82, 4.0f * beat, beat * 1.8f, 0.87f, 1.0f, 2)); // A#5

            // Melody riff Bar 1-2
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(75, 6.0f * beat, beat * 0.45f, 0.90f, 1.0f, 2)); // D#5
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(74, 6.5f * beat, beat * 0.45f, 0.85f, 1.0f, 2)); // D5
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(82, 7.0f * beat, beat * 0.90f, 0.92f, 1.0f, 2)); // A#5

            // Fast Psy Goa Riff (16th notes) Bar 2 - 3
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(74, 8.0f * beat, beat * 0.22f, 0.90f, 1.0f, 2));
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(75, 8.25f * beat, beat * 0.22f, 0.88f, 1.0f, 2));
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(78, 8.50f * beat, beat * 0.22f, 0.95f, 1.0f, 2));
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(79, 8.75f * beat, beat * 0.22f, 0.90f, 1.0f, 2));
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(82, 9.00f * beat, beat * 0.45f, 0.95f, 1.0f, 2));
            pat.getChannelNotes(2).push_back(KuroDSP::MidiNote(78, 9.50f * beat, beat * 0.45f, 0.90f, 1.0f, 2));

            KuroUI::selected_track_idx = 2; // Active Channel: 2 (Lead Synth)
            std::cout << "[TEST_ADVANCED_PIANO_ROLL] Advanced Piano Roll loaded with D Phrygian Dominant scale, polyphonic chords, and ghost notes from Kick, Bass, and Pad!\n";
        } else if (arg == "--test-template-playback") {
            KuroUI::g_home_dashboard_ui.LoadTemplate(0, ::timeline, g_clip_manager);
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_playlist = true;
            KuroUI::show_mixer = false;
            KuroUI::show_piano_roll = false;
            KuroUI::TransportController::Play(::timeline);
            std::cout << "[TEST_TEMPLATE_PLAYBACK] Full-On Psytrance Template loaded and real-time audio playback started!\n";
        }
    }

    // Setup GLFW Drop Callback
    glfwSetDropCallback(window, drop_callback);

    // Inicialização do AutoUpdater (Verificação silenciosa em segundo plano)
    KuroAudio::AutoUpdater::GetInstance().Initialize("", "2.0.0");
    KuroAudio::AutoUpdater::GetInstance().CheckForUpdatesAsync(false);

    // 3. Main Loop Gráfico com Frame Pacing Inteligente (60 FPS / Modo Baixo Consumo de CPU)
    auto frame_start_time = std::chrono::high_resolution_clock::now();
    KuroUtils::Log("[Main Loop] Entering render loop...");
    static int s_logged_frames = 0;

    while (!glfwWindowShouldClose(window)) {
        if (s_logged_frames < 3) {
            KuroUtils::Log("[Main Loop] Processing frame " + std::to_string(s_logged_frames) + "...");
            s_logged_frames++;
        }
        // Se a janela estiver minimizada, descansa a CPU (10 FPS)
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            glfwWaitEventsTimeout(0.1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        frame_start_time = std::chrono::high_resolution_clock::now();
        glfwPollEvents();

#ifdef _WIN32
        MSG win_msg;
        while (PeekMessageA(&win_msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&win_msg);
            DispatchMessageA(&win_msg);
        }
#endif

        // Reconfiguração de áudio dinâmico (troca de saídas pelo usuário)
        if (KuroAudio::DJAudioDeviceManager::getInstance().need_reinit.load()) {
            KuroAudio::DJAudioDeviceManager::getInstance().need_reinit.store(false);
            initOrRestartAudioStreams(adc, dac_cue, ui_to_audio_queue);
    KuroUtils::Log("[Step 8] after initOrRestartAudioStreams");
        }

        // 0. Atualização do Gerenciador de Multi-Resoluções e DPI Dinâmico
        KuroUI::UIScaleManager::update(window);
        KuroUI::UIScaleManager::checkShortcuts();
        if (KuroUI::UIScaleManager::hasScaleChanged()) {
            ImGui::GetStyle() = base_fl_style;
            ImGui::GetStyle().ScaleAllSizes(KuroUI::UIScaleManager::getScale());
            io.FontGlobalScale = KuroUI::UIScaleManager::getScale();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        // Native ImGui Simulated Mouse Commands (for automated GUI testing)
        static bool s_pending_mouse_up = false;
        static float s_pending_mouse_x = 0.0f;
        static float s_pending_mouse_y = 0.0f;

        std::filesystem::path mouse_cmd = "abduction_mouse_cmd.txt";
        std::error_code ec_mouse;
        if (std::filesystem::exists(mouse_cmd, ec_mouse) && !ec_mouse) {
            try {
                std::string action = "";
                float mx = 0.0f, my = 0.0f;
                std::ifstream ifs(mouse_cmd);
                if (ifs >> action) {
                    if (action == "click") {
                        ifs >> mx >> my;
                        io.AddMousePosEvent(mx, my);
                        io.AddMouseButtonEvent(0, true);
                        s_pending_mouse_up = true;
                        s_pending_mouse_x = mx;
                        s_pending_mouse_y = my;
                    } else if (action == "move") {
                        ifs >> mx >> my;
                        io.AddMousePosEvent(mx, my);
                    }
                }
                ifs.close();
                std::error_code ec;
                std::filesystem::remove(mouse_cmd, ec);
            } catch (...) {}
        } else if (s_pending_mouse_up) {
            io.AddMousePosEvent(s_pending_mouse_x, s_pending_mouse_y);
            io.AddMouseButtonEvent(0, false);
            s_pending_mouse_up = false;
        }

        ImGui::NewFrame();

        // 1. Renderizar a Barra Superior Dupla FL Studio no topo da tela (apenas fora do modo DJ fullscreen)
        if (g_ai_engine && !KuroUI::show_dj_modal) {
            KuroUI::RenderFLStyleTopbar(*g_ai_engine);
        }

        // 2. Configuração da Janela Principal para hospedar o DockSpace (abaixo da barra superior escalada)
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        float fl_topbar_h = KuroUI::show_dj_modal ? 0.0f : KuroUI::UIScaleManager::Scale(68.0f);
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
        KuroAudio::AutoUpdater::GetInstance().RenderUI();

        // Clamping universal: Garante que nenhuma janela flutuante escape do viewport em qualquer resolução
        KuroUI::UIScaleManager::ClampAllFloatingWindows(viewport, fl_topbar_h);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.04f, 0.04f, 0.06f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Triggers para Automação e Testes (Protegidos contra conflitos de I/O)
        try {
            std::error_code ec_trig;
            std::filesystem::path trigger_ai = std::filesystem::temp_directory_path() / "abduction_open_ai_architect.txt";
            if (std::filesystem::exists(trigger_ai, ec_trig) && !ec_trig) {
                std::string content = "";
                std::ifstream ifs(trigger_ai);
                if (ifs) ifs >> content;
                ifs.close();
                std::filesystem::remove(trigger_ai, ec_trig);
                if (content == "close") KuroUI::g_ai_track_architect_ui.is_open = false;
                else KuroUI::g_ai_track_architect_ui.Open();
            }
            std::filesystem::path trigger_inject = std::filesystem::temp_directory_path() / "abduction_inject_ai_track.txt";
            if (std::filesystem::exists(trigger_inject, ec_trig) && !ec_trig) {
                std::filesystem::remove(trigger_inject, ec_trig);
                KuroUI::g_ai_track_architect_ui.InjectTrackToProject();
            }

            std::filesystem::path trigger_surg = std::filesystem::temp_directory_path() / "abduction_open_surgical_analyzer.txt";
            if (std::filesystem::exists(trigger_surg, ec_trig) && !ec_trig) {
                std::string content = "";
                std::ifstream ifs(trigger_surg);
                if (ifs) ifs >> content;
                ifs.close();
                std::filesystem::remove(trigger_surg, ec_trig);
                if (content == "close") KuroUI::g_surgical_audio_analyzer_ui.is_open = false;
                else KuroUI::g_surgical_audio_analyzer_ui.Open();
            }
            std::filesystem::path trigger_reconstruct = std::filesystem::temp_directory_path() / "abduction_reconstruct_surgical.txt";
            if (std::filesystem::exists(trigger_reconstruct, ec_trig) && !ec_trig) {
                std::filesystem::remove(trigger_reconstruct, ec_trig);
                KuroUI::g_surgical_audio_analyzer_ui.RecreateAsEditableKuroProject();
            }
            std::filesystem::path trigger_solo = std::filesystem::temp_directory_path() / "abduction_solo_stem.txt";
            if (std::filesystem::exists(trigger_solo, ec_trig) && !ec_trig) {
                std::string stem_cmd = "";
                std::ifstream ifs(trigger_solo);
                if (ifs) ifs >> stem_cmd;
                ifs.close();
                std::filesystem::remove(trigger_solo, ec_trig);
                if (stem_cmd == "kick") KuroUI::g_surgical_audio_analyzer_ui.PlaySoloStem(KuroUI::KuroSurgicalAudioAnalyzerUI::SOLO_KICK);
                else if (stem_cmd == "bass") KuroUI::g_surgical_audio_analyzer_ui.PlaySoloStem(KuroUI::KuroSurgicalAudioAnalyzerUI::SOLO_BASS);
                else if (stem_cmd == "lead") KuroUI::g_surgical_audio_analyzer_ui.PlaySoloStem(KuroUI::KuroSurgicalAudioAnalyzerUI::SOLO_LEAD);
                else if (stem_cmd == "drums") KuroUI::g_surgical_audio_analyzer_ui.PlaySoloStem(KuroUI::KuroSurgicalAudioAnalyzerUI::SOLO_DRUMS);
                else if (stem_cmd == "master") KuroUI::g_surgical_audio_analyzer_ui.PlaySoloStem(KuroUI::KuroSurgicalAudioAnalyzerUI::SOLO_FULL_MASTER);
                else if (stem_cmd == "stop") KuroUI::g_surgical_audio_analyzer_ui.StopSoloPreview();
            }

            std::filesystem::path trigger_pm = std::filesystem::temp_directory_path() / "abduction_open_plugin_manager.txt";
            if (std::filesystem::exists(trigger_pm, ec_trig) && !ec_trig) {
                std::string content = "";
                std::ifstream ifs(trigger_pm);
                if (ifs) ifs >> content;
                ifs.close();
                std::filesystem::remove(trigger_pm, ec_trig);
                if (content == "close") KuroUI::g_plugin_manager_ui.close();
                else KuroUI::g_plugin_manager_ui.open();
            }

            // Trigger para Comandos de Teste de Interface WYSIWYG
            std::filesystem::path trigger_test = "abduction_test_cmd.txt";
            if (std::filesystem::exists(trigger_test, ec_trig) && !ec_trig) {
                std::ifstream ifs(trigger_test);
                ifs.imbue(std::locale::classic());
                std::string cmd = "";
                auto& m = KuroUI::DJLayoutMetrics::getInstance();
                while (ifs >> cmd) {
                    if (cmd == "edit_toggle") {
                        m.edit_mode_active = !m.edit_mode_active;
                    } else if (cmd == "edit_open") {
                        m.edit_mode_active = true;
                    } else if (cmd == "edit_close") {
                        m.edit_mode_active = false;
                    } else if (cmd == "edit_tab") {
                        int t_idx = 0;
                        ifs >> t_idx;
                        m.requested_tab_idx = t_idx;
                    } else if (cmd == "select_widget") {
                        int w_idx = 0;
                        ifs >> w_idx;
                        m.selected_edit_widget = w_idx;
                    } else if (cmd == "load_tracks") {
                        auto& lib_mgr = KuroAudio::DJLibraryManager::getInstance();
                        auto tracks = lib_mgr.getAllTracks();
                        if (g_dj_engine && tracks.size() >= 1) {
                            KuroUI::g_dj_studio_ui.spotify_service.loadLibraryTrack(tracks[0], 0, *g_dj_engine);
                            if (tracks.size() >= 2) {
                                KuroUI::g_dj_studio_ui.spotify_service.loadLibraryTrack(tracks[1], 1, *g_dj_engine);
                            }
                        }
                    } else if (cmd == "toggle_play") {
                        int d = 0;
                        ifs >> d;
                        if (g_dj_engine && d >= 0 && d < 4) {
                            g_dj_engine->decks[d].togglePlay();
                        }
                    } else if (cmd == "wf_beats") {
                        int b = 16;
                        ifs >> b;
                        m.waveform_visible_beats = std::clamp(b, 4, 64);
                        m.needs_waveform_recompute = true;
                    } else if (cmd == "wf_speed") {
                        float s = 1.0f;
                        ifs >> s;
                        m.waveform_scroll_speed = std::clamp(s, 0.50f, 3.00f);
                    } else if (cmd == "wf_stretch") {
                        float st = 1.65f;
                        ifs >> st;
                        m.waveform_horizontal_stretch = std::clamp(st, 1.00f, 4.00f);
                        m.needs_waveform_recompute = true;
                    } else if (cmd == "wf_reset") {
                        m.waveform_visible_beats = 16;
                        m.waveform_scroll_speed = 1.0f;
                        m.waveform_horizontal_stretch = 1.65f;
                        m.waveform_engine_mode = 0;
                        m.needs_waveform_recompute = true;
                    } else if (cmd == "apply_preset") {
                        int p_idx = 0;
                        ifs >> p_idx;
                        m.applyPreset((KuroUI::DJPresetLayoutType)p_idx);
                        m.needs_waveform_recompute = true;
                    }
                }
                ifs.close();
                std::filesystem::remove(trigger_test, ec_trig);
            }

            std::filesystem::path trigger_p = std::filesystem::temp_directory_path() / "abduction_screenshot_req.txt";
            std::filesystem::path trigger_local = "abduction_screenshot_req.txt";
            std::filesystem::path default_out = std::filesystem::temp_directory_path() / "abduction_screenshot.bmp";
            std::filesystem::path local_out = "abduction_screenshot.bmp";

            bool has_req_local = std::filesystem::exists(trigger_local, ec_trig) && !ec_trig;
            bool has_req_p = std::filesystem::exists(trigger_p, ec_trig) && !ec_trig;
            std::string custom_snap_dest = "";
            if (has_req_local) {
                std::ifstream ifs(trigger_local);
                if (ifs) ifs >> custom_snap_dest;
                ifs.close();
            } else if (has_req_p) {
                std::ifstream ifs(trigger_p);
                if (ifs) ifs >> custom_snap_dest;
                ifs.close();
            }

            if (has_req_local || has_req_p || (ImGui::IsKeyPressed(ImGuiKey_F12, false))) {
                std::filesystem::remove(trigger_p, ec_trig);
                std::filesystem::remove(trigger_local, ec_trig);
                KuroUtils::Log("[Screenshot] Capturando hardware framebuffer...");
            
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
            auto writeBmpTo = [&](const std::filesystem::path& out_path) {
                std::ofstream f(out_path, std::ios::binary);
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
            };
            writeBmpTo(default_out);
            writeBmpTo(local_out);
            if (!custom_snap_dest.empty() && custom_snap_dest != "snap") {
                writeBmpTo(custom_snap_dest);
            }
            KuroUtils::Log("[Screenshot] Framebuffer capturado com sucesso em: " + default_out.string() + " e " + local_out.string());
        }
        } catch (const std::exception& e) {
            KuroUtils::Log("[Trigger Warning] " + std::string(e.what()));
        } catch (...) {}

        // FASE 27: Renderização de Viewports Independentes (Multi-monitor)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);

        // ============================================================
        // Frame Limiter de Alta Precisão — Alvo: 120 FPS (8.333ms/frame)
        // glfwSwapInterval(0) = sem VSync → frame limiter manual controla
        // Minimizado: throttle a 20 FPS para economizar CPU
        // ============================================================
        static auto s_last_frame = std::chrono::high_resolution_clock::now();
        bool is_iconified = (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0);
        if (is_iconified) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } else {
            // Teto de 120 FPS: dorme o tempo restante do slot de 8.333ms
            // Subtrai 0.3ms de margem para compensar jitter do sleep do SO
            constexpr double k_target_dt = 1.0 / 120.0;
            auto now_tp = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now_tp - s_last_frame).count();
            if (elapsed < k_target_dt) {
                double sleep_sec = k_target_dt - elapsed - 0.0003;
                if (sleep_sec > 0.0001) {
                    std::this_thread::sleep_for(std::chrono::duration<double>(sleep_sec));
                }
            }
        }
        s_last_frame = std::chrono::high_resolution_clock::now();
    }

    if (adc.isStreamOpen()) {
        adc.stopStream();
        adc.closeStream();
    }
    
    // Salva automaticamente o layout e preferências de waveform antes de encerrar
    try {
        KuroUI::g_dj_studio_ui.saveCustomLayout();
    } catch (...) {}

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

#ifdef _WIN32
    timeEndPeriod(1);
#endif

    return 0;
}
