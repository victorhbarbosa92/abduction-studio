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
#include "../plugin_manager/Vst3Wrapper.h"
#include "../plugin_manager/ClapWrapper.h"
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
        if (!rack_node) {
            rack_node = std::dynamic_pointer_cast<KuroDSP::RackNode>(master_graph.getNode("TrackPedalboard_" + std::to_string(selected_track_idx)));
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.30f, 0.10f, 1.0f));
        if (ImGui::Button("+ Adicionar Efeito Kuro", ImVec2(200, 30))) {
            ImGui::OpenPopup("AddEffectPopup");
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.20f, 0.40f, 1.0f));
        if (ImGui::Button("+ Carregar VST3/CLAP", ImVec2(220, 30))) {
            std::string dll_path = KuroUI::FileDialog::OpenFile("Plugins VST3 / CLAP (*.vst3;*.clap;*.dll)\0*.vst3;*.clap;*.dll\0All Files\0*.*\0");
            if (!dll_path.empty() && rack_node) {
                if (dll_path.find(".vst3") != std::string::npos) {
                    auto wrapper = std::make_shared<KuroDSP::Vst3Wrapper>("vst3_" + std::to_string(selected_track_idx), std::filesystem::path(dll_path).stem().string());
                    if (wrapper->load(dll_path)) {
                        rack_node->addPlugin(wrapper);
                        wrapper->openGui();
                    }
                } else {
                    auto wrapper = std::make_shared<KuroDSP::ClapWrapper>("clap_" + std::to_string(selected_track_idx), std::filesystem::path(dll_path).stem().string());
                    if (wrapper->load(dll_path)) {
                        rack_node->addPlugin(wrapper);
                        wrapper->openGui();
                    }
                }
            }
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        if (ImGui::BeginPopup("AddEffectPopup")) {
            if (ImGui::Selectable("Compressor Gótico")) { track_pedalboards[selected_track_idx].preset_dark = true; }
            if (ImGui::Selectable("Delay Espacial")) { track_pedalboards[selected_track_idx].preset_space = true; }
            if (ImGui::Selectable("Reverb Cavernoso")) { track_pedalboards[selected_track_idx].preset_cavern = true; }
            ImGui::EndPopup();
        }

        ImGui::Spacing(); ImGui::Spacing();

        ImGui::BeginChild("PluginChain", ImVec2(0, 140), true);
        if (rack_node && rack_node->getPluginCount() > 0) {
            auto plugs = rack_node->getPlugins();
            for (size_t i = 0; i < plugs.size(); ++i) {
                auto& p = plugs[i];
                if (!p) continue;
                ImGui::PushID((int)i);
                ImGui::BulletText("%s", p->getName().c_str());
                ImGui::SameLine();
                bool bypass = p->getBypass();
                if (ImGui::Checkbox("Bypass", &bypass)) {
                    p->setBypass(bypass);
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_WINDOW_RESTORE " GUI")) {
                    auto v3 = std::dynamic_pointer_cast<KuroDSP::Vst3Wrapper>(p);
                    if (v3) v3->openGui();
                    auto cp = std::dynamic_pointer_cast<KuroDSP::ClapWrapper>(p);
                    if (cp) cp->openGui();
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_TRASH_CAN " Remover")) {
                    rack_node->removePlugin(i);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Cadeia de Plugins (Vazio)");
            ImGui::TextWrapped("Carregue VST3, CLAP ou sintetizadores para processar esta faixa.");
        }
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
