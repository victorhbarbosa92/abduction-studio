#pragma once
#include "imgui.h"
#include "../core/TimelineManager.h"
#include "../audio/SynthEngine.h"
#include <vector>
#include <cmath>
#include <cstdio>

namespace KuroUI {

    inline void RenderStepSequencer(bool* open, KuroDSP::TimelineManager& timeline_mgr, float bpm) {
        if (!*open) return;

        ImGui::SetNextWindowSize(ImVec2(780, 420), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Step Sequencer (FL Style)", open)) {
            ImGui::End();
            return;
        }

        // Drum instrument definitions
        const char* inst_names[] = { "KICK (Track 0)", "SNARE (Track 1)", "HIHAT (Track 2)", "CLAP (Track 3)" };
        int pitches[] = { 36, 38, 42, 39 };

        float beat_duration = 60.0f / bpm;
        float snap_step = beat_duration / 4.0f; // 1/16 note

        static int selected_inst = 0;
        static float step_probs[4][16] = {
            {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}
        };

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Polyrhythmic 16-Step Drum Sequencer with Probability");
        ImGui::Separator();

        std::lock_guard<std::mutex> lock(timeline_mgr.timeline_mutex);

        for (int inst = 0; inst < 4; inst++) {
            ImGui::PushID(inst);
            
            // Select instrument on click
            bool is_selected = (selected_inst == inst);
            if (ImGui::Selectable(inst_names[inst], is_selected, 0, ImVec2(120, 25))) {
                selected_inst = inst;
            }
            
            ImGui::SameLine();
            
            // Poly step limit control
            ImGui::Text("Steps:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(45);
            int steps_lim = timeline_mgr.track_steps_limit[inst];
            if (ImGui::DragInt("##steps_lim", &steps_lim, 0.1f, 1, 16)) {
                if (steps_lim < 1) steps_lim = 1;
                if (steps_lim > 16) steps_lim = 16;
                timeline_mgr.track_steps_limit[inst] = steps_lim;
            }
            
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();

            auto& notes = timeline_mgr.track_notes[inst];
            int limit = timeline_mgr.track_steps_limit[inst];

            for (int step = 0; step < 16; step++) {
                ImGui::PushID(step);
                
                // Show disabled slots beyond step limit
                bool out_of_bounds = (step >= limit);
                if (out_of_bounds) {
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
                }

                float step_time = step * snap_step;
                bool active = false;
                int note_idx = -1;
                for (size_t i = 0; i < notes.size(); i++) {
                    if (notes[i].pitch == pitches[inst] && std::abs(notes[i].start_time - step_time) < 0.01f) {
                        active = true;
                        note_idx = (int)i;
                        break;
                    }
                }

                // Color coding
                ImVec4 btn_col = active ? ImVec4(0.0f, 0.9f, 0.4f, 1.0f) : ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
                if (step % 4 == 0) {
                    if (!active) btn_col = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
                }
                
                ImGui::PushStyleColor(ImGuiCol_Button, btn_col);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.5f, 1.0f));
                
                if (ImGui::Button("##step", ImVec2(25, 25))) {
                    if (!out_of_bounds) {
                        if (active) {
                            if (note_idx >= 0 && note_idx < (int)notes.size()) {
                                notes.erase(notes.begin() + note_idx);
                            }
                        } else {
                            KuroDSP::MidiNote new_note;
                            new_note.pitch = pitches[inst];
                            new_note.start_time = step_time;
                            new_note.duration = snap_step * 0.9f;
                            new_note.velocity = 0.9f;
                            new_note.probability = step_probs[inst][step];
                            notes.push_back(new_note);
                        }
                    }
                }
                
                ImGui::PopStyleColor(2);
                if (out_of_bounds) {
                    ImGui::PopStyleVar();
                }
                ImGui::SameLine();
                ImGui::PopID();
            }

            ImGui::NewLine();
            ImGui::PopID();
        }

        ImGui::Separator();
        
        // Probability (Chance) Editor Section
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Chance / Trigger Probability Editor for: %s", inst_names[selected_inst]);
        ImGui::Spacing();
        
        auto& sel_notes = timeline_mgr.track_notes[selected_inst];
        int sel_limit = timeline_mgr.track_steps_limit[selected_inst];
        
        ImGui::BeginChild("ChanceEditor", ImVec2(0, 90), true);
        for (int step = 0; step < 16; step++) {
            ImGui::PushID(step + 100);
            
            bool out_of_bounds = (step >= sel_limit);
            if (out_of_bounds) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
            }
            
            float step_time = step * snap_step;
            int note_idx = -1;
            for (size_t i = 0; i < sel_notes.size(); i++) {
                if (sel_notes[i].pitch == pitches[selected_inst] && std::abs(sel_notes[i].start_time - step_time) < 0.01f) {
                    note_idx = (int)i;
                    break;
                }
            }
            
            float current_prob = step_probs[selected_inst][step];
            if (note_idx >= 0) {
                current_prob = sel_notes[note_idx].probability;
            }
            
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.2f, 0.7f, 1.0f, 1.0f));
            
            char label[32];
            sprintf(label, "##prob%d", step);
            if (ImGui::VSliderFloat(label, ImVec2(25, 70), &current_prob, 0.0f, 1.0f, "")) {
                if (!out_of_bounds) {
                    step_probs[selected_inst][step] = current_prob;
                    if (note_idx >= 0) {
                        sel_notes[note_idx].probability = current_prob;
                    }
                }
            }
            
            ImGui::PopStyleColor(2);
            if (out_of_bounds) {
                ImGui::PopStyleVar();
            }
            
            // Tooltip showing chance percentage
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Step %d Chance: %d%%", step + 1, (int)(current_prob * 100.0f));
            }
            
            ImGui::SameLine(0, 10);
            ImGui::PopID();
        }
        ImGui::EndChild();
        
        ImGui::Separator();
        if (ImGui::Button("Limpar Grid")) {
            for (int inst = 0; inst < 4; inst++) {
                timeline_mgr.clearNotes(inst);
            }
        }

        ImGui::End();
    }
}
