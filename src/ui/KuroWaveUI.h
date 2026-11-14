#pragma once
#include <imgui.h>
#include "../audio/KuroWave.h"
#include <string>

namespace KuroUI {

    inline void RenderKuroWaveSynth(KuroAudio::KuroWave& synth) {
        ImGui::BeginChild("KuroWaveSynth", ImVec2(0, 200), true);
        
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "KURO-WAVE (PSYTRANCE SYNTH)");
        ImGui::SameLine(ImGui::GetWindowWidth() - 100);
        
        // Botão de Power (Bypass)
        if (synth.enabled) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        }
        if (ImGui::Button(synth.enabled ? "ON" : "OFF", ImVec2(50, 20))) {
            synth.enabled = !synth.enabled;
        }
        ImGui::PopStyleColor();

        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::Columns(3, "synth_cols");
        
        // Coluna 1: Oscilador / Wavetable
        ImGui::Text("GERAÇÃO DE SOM (Timbre)");
        ImGui::SliderFloat("Tipo de Voz", &synth.param_wt_position, 0.0f, 3.0f, "%.2f");
        ImGui::TextDisabled("0=Suave, 1=Triângulo, 2=Serrote, 3=Quadrado");
        ImGui::Spacing();
        ImGui::SliderInt("Coro (Vozes)", &synth.param_unison_voices, 1, 16);
        ImGui::SliderFloat("Espessura (Detune)", &synth.param_unison_detune, 0.0f, 0.5f, "%.2f");
        
        ImGui::NextColumn();
        
        // Coluna 2: Filtro
        ImGui::Text("FILTRO DE FREQUÊNCIA");
        ImGui::SliderFloat("Brilho (Cutoff)", &synth.param_cutoff, 20.0f, 20000.0f, "%.0f Hz", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("Pico Metálico (Res)", &synth.param_resonance, 0.1f, 10.0f, "%.2f");
        
        ImGui::NextColumn();
        
        // Coluna 3: LFO Modulation
        ImGui::Text("MOVIMENTO AUTOMÁTICO");
        float lfo_freq = 0.5f; 
        ImGui::SliderFloat("Intensidade", &synth.param_lfo_wt_mod, 0.0f, 1.0f, "%.2f");
        
        ImGui::Columns(1);
        ImGui::EndChild();
    }
}
