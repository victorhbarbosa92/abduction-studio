#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <mutex>
#include <cmath>

// GLFW e ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// RtAudio
#include "RtAudio.h"

#include "audio/LockFreeAudioQueue.h"
#include "ai/StemSeparationEngine.h"
#include "plugin_manager/NativePlugins.h"

// ==========================================
// ESTADO DSP GLOBAL (Zero Allocation in Audio Thread)
// ==========================================
KuroDSP::CompGotica track_comps[4];
KuroDSP::AtrasoEspacial track_delays[4];
KuroDSP::GothicEQ track_eqs[4];
KuroDSP::Atrasoleria track_reverbs[4];
bool play_test_tone = false;
float global_phase = 0.0f;
int active_eclipse_track = -1; // -1 = Fechado

// ==========================================
// ESTRUTURAS GLOBAIS
// ==========================================
struct AudioEvent {
    int channel_id;
    float param_value;
};

// ==========================================
// CALLBACK DO RTAUDIO (Thread de alta prioridade - WASAPI)
// ==========================================
int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames,
                  double streamTime, RtAudioStreamStatus status, void *userData)
{
    float *out = (float *)outputBuffer;
    LockFreeAudioQueue<AudioEvent>* queue = (LockFreeAudioQueue<AudioEvent>*)userData;
    
    // Ler eventos UI (sem locks)
    AudioEvent evt;
    while(queue->pop(evt)) {
        // Ex: Atualizar volume
    }
    
    // Ruído e Bipe de Teste
    for (unsigned int i = 0; i < nFrames; i++) {
        float sample = 0.0f;
        
        // Gerador de Teste Simples (Bipe de 440Hz se ativado pela UI)
        if (play_test_tone) {
            sample = std::sin(global_phase * 2.0f * 3.1415926535f) * 0.1f;
            global_phase += 440.0f / 44100.0f;
            if (global_phase > 1.0f) global_phase -= 1.0f;
        } else {
            // Ruido ciber-gotico continuo
            sample = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 0.002f;
        }

        // Processa todos os DSPs (Por enquanto rodamos no Track 0 como master de teste)
        if (track_comps[0].enabled) track_comps[0].process(&sample, 1);
        if (track_eqs[0].enabled) track_eqs[0].process(&sample, 1);
        if (track_reverbs[0].enabled) track_reverbs[0].process(&sample, 1);

        float left = sample;
        float right = sample;
        
        if (track_delays[0].enabled) track_delays[0].process(&left, &right, 1);

        out[i * 2] = left;     // L
        out[i * 2 + 1] = right; // R
    }

    return 0;
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Callback real para Drag and Drop de arquivos do Windows
void drop_callback(GLFWwindow* window, int count, const char** paths)
{
    if (count > 0) {
        StemSeparationEngine* engine = static_cast<StemSeparationEngine*>(glfwGetWindowUserPointer(window));
        if (engine && !engine->isRunning()) {
            // Pegamos o primeiro arquivo arrastado e enviamos pro motor
            engine->startProcessing(paths[0]);
        }
    }
}

// Configura o Tema ImGui
void setupGothicTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.06f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.60f, 0.00f, 1.00f, 1.00f); // Spectral Purple
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.00f, 0.20f, 1.00f);  // Blood Red
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.16f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 1.00f, 0.40f, 0.30f); // Kryptonite green
    style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.20f, 1.00f); 
    
    style.WindowRounding = 0.0f; 
    style.FrameRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
}

int main(int, char**) {
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
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1024, 600, "KURO SEPARATOR PRO (Gothic-Space Edition)", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync (60 FPS)

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    setupGothicTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    StemSeparationEngine ai_engine;

    // Conectar ponteiros para o Callback de Drag & Drop do GLFW
    glfwSetWindowUserPointer(window, &ai_engine);
    glfwSetDropCallback(window, drop_callback);

    // 3. Main Loop Gráfico (60 FPS)
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Kuro Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.4f, 1.0f), "KURO SEPARATOR PRO (Build v1.1) - Motor Nativo Ativo");
        ImGui::Separator();
        
        ImGui::Spacing(); ImGui::Spacing();
        
        // Área DROPZONE ou TIMELINE
        if (!ai_engine.hasFinished()) {
            ImGui::BeginChild("Dropzone", ImVec2(0, 150), true);
            if (!ai_engine.isRunning()) {
                ImGui::SetCursorPosY(50);
                ImGui::Text("                      [ ARRASTAR E SOLTAR ARQUIVOS DE ÁUDIO AQUI ]");
                if (ImGui::Button("Simular Dropzone / Isolamento via IA", ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
                    ai_engine.startProcessing("track.wav");
                }
            } else {
                ImGui::SetCursorPosY(40);
                ImGui::Text("IA Isolando o Kick e o Bass (Demucs via ONNX AVX2)...");
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.2f, 1.0f), "%s", ai_engine.getStatus().c_str());
                ImGui::ProgressBar(ai_engine.getProgress(), ImVec2(-1.0f, 0.0f), "");
            }
            ImGui::EndChild();
        } else {
            // TIMELINE PLAYLIST (Ableton Style)
            ImGui::BeginChild("Timeline", ImVec2(0, 260), true);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "T I M E L I N E   P L A Y L I S T");
            ImGui::SameLine(ImGui::GetWindowWidth() - 150);
            if (ImGui::Button("Limpar Projeto")) {
                ai_engine.reset();
            }
            ImGui::Separator();
            
            const char* track_names[4] = {"KICK/BASS", "LEADS", "VOX", "FX"};
            ImVec4 track_colors[4] = {
                ImVec4(1.0f, 0.0f, 0.2f, 1.0f), // Vermelho Neon
                ImVec4(0.0f, 1.0f, 0.4f, 1.0f), // Verde Kryptonita
                ImVec4(0.9f, 0.8f, 0.1f, 1.0f), // Amarelo/Dourado
                ImVec4(0.6f, 0.0f, 1.0f, 1.0f)  // Roxo Espectral
            };

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            
            for (int i = 0; i < 4; i++) {
                ImGui::PushID(i);
                ImVec2 p_min = ImGui::GetCursorScreenPos();
                ImVec2 p_max = ImVec2(p_min.x + ImGui::GetContentRegionAvail().x - 60, p_min.y + 40);
                
                // Fundo da trilha
                draw_list->AddRectFilled(p_min, p_max, IM_COL32(20, 20, 25, 255));
                draw_list->AddRect(p_min, p_max, IM_COL32(50, 50, 60, 255));
                
                // Nome da trilha
                draw_list->AddText(ImVec2(p_min.x + 5, p_min.y + 12), ImColor(track_colors[i]), track_names[i]);
                
                // Simulando Waveform (Ondas luminosas)
                ImU32 col = ImColor(track_colors[i]);
                float mid_y = p_min.y + 20;
                float start_x = p_min.x + 90;
                float end_x = p_max.x - 10;
                
                // Linha central
                draw_list->AddLine(ImVec2(start_x, mid_y), ImVec2(end_x, mid_y), col, 1.0f);
                
                // Picos vetoriais pseudo-aleatórios consistentes
                float curr_x = start_x;
                srand(i * 12345); // Semente fixa para não piscar a cada frame
                while (curr_x < end_x) {
                    float step = 5.0f + (rand() % 8);
                    float amp = (rand() % 16);
                    if (curr_x + step > end_x) step = end_x - curr_x;
                    draw_list->AddLine(ImVec2(curr_x, mid_y), ImVec2(curr_x + step/2.0f, mid_y - amp), col, 1.5f);
                    draw_list->AddLine(ImVec2(curr_x + step/2.0f, mid_y - amp), ImVec2(curr_x + step, mid_y), col, 1.5f);
                    
                    // Waveform espelhado para baixo
                    draw_list->AddLine(ImVec2(curr_x, mid_y), ImVec2(curr_x + step/2.0f, mid_y + amp), col, 1.5f);
                    draw_list->AddLine(ImVec2(curr_x + step/2.0f, mid_y + amp), ImVec2(curr_x + step, mid_y), col, 1.5f);
                    
                    curr_x += step;
                }
                
                // Avançar o cursor do ImGui invisivelmente
                ImGui::Dummy(ImVec2(p_max.x - p_min.x, 40));
                
                // Slot de plugin gótico
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.15f, 1.0f));
                if (ImGui::Button("[+ VST]", ImVec2(50, 40))) {
                    active_eclipse_track = i;
                }
                ImGui::PopStyleColor();
                
                ImGui::PopID();
                ImGui::Spacing();
            }
            
            ImGui::EndChild();
        }
        
        // Mixer Compacto
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.0f, 1.0f, 1.0f), "M I X E R   G Ó T I C O");
        ImGui::BeginChild("Mixer", ImVec2(0, 0), true);
        ImGui::Columns(4, "mixer_cols");
        for (int i = 0; i < 4; i++) {
            ImGui::Text(i == 0 ? "KICK/BASS" : (i == 1 ? "LEADS" : (i == 2 ? "VOX" : "FX")));
            static float vol[4] = { -3.5f, -6.0f, -1.0f, -12.0f };
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, (i==0) ? ImVec4(1,0,0,1) : (i==1) ? ImVec4(0,1,0,1) : ImVec4(0.6f,0,1,1));
            ImGui::VSliderFloat(std::string("##v" + std::to_string(i)).c_str(), ImVec2(60, 200), &vol[i], -60.0f, 6.0f, "%.1f dB");
            ImGui::PopStyleColor(2);
            
            if (ImGui::IsItemActive()) {
                ui_to_audio_queue.push({i, vol[i]});
            }
            ImGui::NextColumn();
        }
        ImGui::EndChild();

        ImGui::End();

        // 4. ECLIPSE DINÂMICO (Janela Flutuante de Racks)
        if (active_eclipse_track != -1) {
            ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Eclipse Dinâmico - Rack de Efeitos", nullptr, ImGuiWindowFlags_NoCollapse)) {
                ImGui::TextColored(ImVec4(0.6f, 0.0f, 1.0f, 1.0f), "RACK DE SINAL: Trilha %d", active_eclipse_track + 1);
                ImGui::SameLine(ImGui::GetWindowWidth() - 100);
                if (ImGui::Button("FECHAR (X)")) {
                    active_eclipse_track = -1;
                }
                ImGui::Separator();
                
                ImGui::Checkbox("Som de Teste (Bipe 440Hz)", &play_test_tone);
                ImGui::Spacing();
                
                int t = active_eclipse_track;
                
                // CompGotica Widget
                ImGui::BeginChild("CompGotica", ImVec2(160, 200), true);
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.2f, 1.0f), "COMP GÓTICA");
                ImGui::Checkbox("On##comp", &track_comps[t].enabled);
                float thresh = track_comps[t].getThreshold();
                if (ImGui::SliderFloat("Thresh", &thresh, -40.0f, 0.0f, "%.1f dB")) {
                    track_comps[t].setThreshold(thresh);
                }
                ImGui::EndChild();
                ImGui::SameLine();
                
                // AtrasoEspacial Widget
                ImGui::BeginChild("Delay", ImVec2(160, 200), true);
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.4f, 1.0f), "ATRASO ESPACIAL");
                ImGui::Checkbox("On##delay", &track_delays[t].enabled);
                float fb = track_delays[t].getFeedback();
                if (ImGui::SliderFloat("Feedback", &fb, 0.0f, 0.99f, "%.2f")) {
                    track_delays[t].setFeedback(fb);
                }
                ImGui::EndChild();
                ImGui::SameLine();
                
                // Atrasoleria (Reverb Mock)
                ImGui::BeginChild("Reverb", ImVec2(160, 200), true);
                ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.1f, 1.0f), "ATRASOLERIA");
                ImGui::Checkbox("On##rev", &track_reverbs[t].enabled);
                ImGui::TextDisabled("Reverb Fixo Algorítmico");
                ImGui::EndChild();
            }
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.04f, 0.04f, 0.06f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
