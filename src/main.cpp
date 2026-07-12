#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <mutex>
#include <cmath>
#include <map>
#include <fstream>

// GLFW e ImGui
#include "imgui.h"
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

// Globais para o StudioUI
bool is_playing = false;
unsigned long long global_sample_count = 0;
ClipManager g_clip_manager;
KuroAudio::KuroWave g_kurowave;
KuroAudio::SynthEngine g_piano_synth;
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
std::string track_names[MAX_TRACKS] = {"KICK/BASS", "LEADS", "VOX", "FX", "DRUMS", "SYNTH", "PADS", "EXTRA", "TRK 9", "TRK 10", "TRK 11", "TRK 12", "TRK 13", "TRK 14", "TRK 15", "TRK 16", "TRK 17", "TRK 18", "TRK 19", "TRK 20"};

float global_time_sec = 0.0f;
float param_filter_cutoff = 20000.0f;

LockFreeAudioQueue<AudioEvent> ui_to_audio_queue(1024);

extern "C" {
    float getGlobalTimeSec() { return global_time_sec; }
}

float track_volumes[MAX_TRACKS];
bool track_mutes[MAX_TRACKS] = { false };
bool track_solos[MAX_TRACKS] = { false };
bool track_fx_bypass[MAX_TRACKS] = { false };
bool track_abyss_pitch_enabled[MAX_TRACKS] = { false };
float track_pitch_semitones[MAX_TRACKS] = { 0.0f };

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
    }
    was_playing = currently_playing;

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
    
    // Roteia Eventos MIDI (Piano Roll e Step Sequencer) para os Synths
    for (const auto& ev : midi_events) {
        int track_idx = std::get<0>(ev);
        int pitch = std::get<1>(ev);
        float duration = std::get<2>(ev);
        float velocity = std::get<3>(ev);
        
        g_piano_synth.triggerNote(pitch, duration, velocity);
        // Opcional: Se existir track 5 com KuroWave, rotear tbm
        if (track_idx == 5) {
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
    g_kurowave.process(global_synth_l, global_synth_r, nFrames, global_time_sec);

    // Aplica Kuro Gross Beat APENAS nos sintetizadores (antes do Master Bus)
    extern KuroDSP::GrossBeatNode g_gross_beat;
    g_gross_beat.processBlock(global_synth_l, global_synth_r, nFrames);

    // Mute / Solo logic para o Synth (Track 5)
    bool any_solo = false;
    for (int i=0; i<8; i++) {
        if (track_solos[i]) { any_solo = true; break; }
    }
    
    if ((any_solo && !track_solos[5]) || track_mutes[5]) {
        std::fill_n(global_synth_l, nFrames, 0.0f);
        std::fill_n(global_synth_r, nFrames, 0.0f);
    }
    
    // Processar efeitos da Track 5 (Synth) - NÃO AFETA MAIS O PIANO DE PREVIEW!
    track_pedalboards[5].process(global_synth_l, global_synth_r, nFrames);

    // 2. Grafo Principal (onde os VSTs/CLAPs e Clips vivem)
    master_graph.process(global_synth_l, global_synth_r, nFrames);

    // Processa o Preview do Piano de forma isolada e imaculada!
    float piano_l[2048] = {0};
    float piano_r[2048] = {0};
    g_piano_synth.process(piano_l, piano_r, nFrames, global_time_sec);

    // 3. Efeitos Nativos do Master Bus e Mixagem Final
    float* out_buffer = (float*)outputBuffer;
    
    for (unsigned int i = 0; i < nFrames; i++) {
        float left = global_synth_l[i];
        float right = global_synth_r[i];

        if (track_limiters[0].enabled) track_limiters[0].process(&left, &right, 1);
        if (track_delays[0].enabled) track_delays[0].process(&left, &right, 1);
        if (track_choruses[0].enabled) track_choruses[0].process(&left, &right, 1);
        if (track_reverbs[0].enabled) track_reverbs[0].process(&left, &right, 1);

        // Adiciona o piano limpo diretamente no Master Output
        left += piano_l[i];
        right += piano_r[i];

        out_buffer[i * 2] = left;     // L
        out_buffer[i * 2 + 1] = right; // R
    }
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
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.18f, 0.20f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.28f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.29f, 0.31f, 0.33f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.43f, 0.45f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.55f, 0.60f, 0.63f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.24f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.14f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.28f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.30f, 0.80f, 0.30f, 1.00f);
    style.WindowRounding = 4.0f; 
    style.FrameRounding = 2.0f;
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

    // Instancia Engine de IA após o carregamento das DLLs do SO
    g_ai_engine = std::make_unique<StemSeparationEngine>();
    g_dj_engine = std::make_unique<KuroAudio::DJEngine>();
    g_stem_engine = std::make_unique<KuroAudio::StemExtractorEngine>();
    
    // Inicializar Grafo Global
    auto master_bus = std::make_shared<KuroDSP::RackNode>("Master", "Master Bus");
    master_graph.addNode("Master", master_bus);

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
        master_graph.connect("ClipPlayer_" + std::to_string(i), "Master"); // Roteia para o Master
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
    setupAbductionTheme(); // Tema Alien Spaceship (Fase 23)

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

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Kuro Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
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
