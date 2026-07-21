#pragma once
#include "imgui.h"
#include <string>
#include "../audio/AudioEvent.h"
#include "../ai/StemSeparationEngine.h"
#include "../audio/LockFreeAudioQueue.h"
#include "FileDialog.h"
#include "../plugin_manager/ClapWrapper.h"
#include "../plugin_manager/KuroSamplerNode.h"
#include "../core/TimelineManager.h"

#include "../core/KuroConfig.h"

extern float track_volumes[MAX_TRACKS];
extern float track_sends_A[MAX_TRACKS];
extern float track_sends_B[MAX_TRACKS];
extern bool global_fx_bypass;
extern bool track_mutes[MAX_TRACKS];
extern bool track_solos[MAX_TRACKS];
extern bool track_fx_bypass[MAX_TRACKS];
extern bool track_abyss_pitch_enabled[MAX_TRACKS];
extern float track_pitch_semitones[MAX_TRACKS];
extern std::string track_names[MAX_TRACKS];
extern LockFreeAudioQueue<AudioEvent> ui_to_audio_queue;

#include "../plugin_manager/NativePlugins.h"
#include "../plugin_manager/DAG.h"
#include "../plugin_manager/KuroPedalboard.h"
extern KuroDSP::Pedalboard track_pedalboards[MAX_TRACKS];
extern KuroDSP::AudioGraph master_graph;
extern KuroDSP::TimelineManager timeline;

namespace KuroUI {

    // Helper para desenhar o Rack DSP da faixa selecionada
    inline void RenderKuroDSPRack(StemSeparationEngine& ai_engine, int selected_track_idx) {
        ImGui::Spacing();
        std::string node_id = "Track" + std::to_string(selected_track_idx);
        
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.08f, 1.0f), "RACK DE EFEITOS: %s", track_names[selected_track_idx].c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() - 200.0f);
        ImGui::Checkbox(std::string("BYPASS FX (" + track_names[selected_track_idx] + ")").c_str(), &track_fx_bypass[selected_track_idx]);
        
        ImGui::Separator();
        ImGui::Spacing();
        
        auto rack_node = std::dynamic_pointer_cast<KuroDSP::RackNode>(master_graph.getNode(node_id));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.30f, 0.10f, 1.0f));
        if (ImGui::Button("+ Adicionar Efeito Kuro", ImVec2(200, 30))) {
            ImGui::OpenPopup("AddEffectPopup");
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.20f, 0.40f, 1.0f));
        if (ImGui::Button("+ Carregar VST/CLAP (.dll)", ImVec2(220, 30))) {
            std::string dll_path = KuroUI::FileDialog::OpenFile("Plugins DLL (*.dll)\0*.dll\0All Files\0*.*\0");
            if (!dll_path.empty() && rack_node) {
                auto wrapper = std::make_unique<KuroDSP::ClapWrapper>("temp_vst_" + std::to_string(selected_track_idx), "VST Externo");
                wrapper->load(dll_path);
            }
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        if (ImGui::BeginPopup("AddEffectPopup")) {
            if (ImGui::Selectable("Compressor Gótico")) { }
            if (ImGui::Selectable("Delay Espacial")) { }
            if (ImGui::Selectable("Reverb Algorítmico")) { }
            ImGui::EndPopup();
        }

        ImGui::Spacing(); ImGui::Spacing();

        ImGui::BeginChild("PluginChain", ImVec2(0, 120), true);
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Cadeia de Plugins (Vazio)");
        ImGui::TextWrapped("Adicione plugins acima para moldar o som da faixa selecionada.");
        ImGui::EndChild();
        
        ImGui::Spacing();
        
        // --- KURO SYNTH ASSIST ---
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.15f, 0.05f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 1.0f, 0.08f, 0.3f));
        ImGui::BeginChild("SynthAssist", ImVec2(0, 120), true);
        ImGui::TextColored(ImVec4(0.22f, 1.0f, 0.2f, 1.0f), "👽 ALIEN INTELLIGENCE (Sugestões)");
        float bpm = ai_engine.getBPM();
        ImGui::TextWrapped("Análise Neural: BPM (%.0f) | Tom (%s)", bpm, ai_engine.getKey().c_str());
        ImGui::Spacing();
        
        if (bpm > 130) {
            ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.8f, 1.0f), "> DICA: Uma faixa Psytrance rápida. Aplique um Delay em sincronia com 1/8 no Bass para criar groove alienígena.");
        } else if (bpm > 0) {
            ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.8f, 1.0f), "> DICA: Cadência lenta. Use o Reverb em 100%% Wet para criar soundscapes imersivos tipo 'Abduction'.");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[ Aguardando áudio base para abduzir dados... ]");
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
    }
}
