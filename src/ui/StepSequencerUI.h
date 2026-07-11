#pragma once
#include "imgui.h"
#include "../core/TimelineManager.h"
#include "../audio/SynthEngine.h"
#include <vector>
#include <cmath>

namespace KuroUI {

    inline void RenderStepSequencer(bool* open, KuroDSP::TimelineManager& timeline_mgr, float bpm) {
        if (!*open) return;

        ImGui::SetNextWindowSize(ImVec2(700, 250), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Step Sequencer (FL Style)", open)) {
            ImGui::End();
            return;
        }

        // Drum instrument definitions
        const char* inst_names[] = { "KICK (Track 0)", "SNARE (Track 1)", "HIHAT (Track 2)", "CLAP (Track 3)" };
        int pitches[] = { 36, 38, 42, 39 };

        float beat_duration = 60.0f / bpm;
        float snap_step = beat_duration / 4.0f; // 1/16 note

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "FL Studio Style 16-Step Drum Sequencer");
        ImGui::Separator();

        std::lock_guard<std::mutex> lock(timeline_mgr.timeline_mutex);

        for (int inst = 0; inst < 4; inst++) {
            ImGui::PushID(inst);
            ImGui::Text("%-15s", inst_names[inst]);
            ImGui::SameLine();

            auto& notes = timeline_mgr.track_notes[inst];

            for (int step = 0; step < 16; step++) {
                ImGui::PushID(step);
                
                float step_time = step * snap_step;
                // Check if note exists at this step_time with the given pitch
                bool active = false;
                int note_idx = -1;
                for (size_t i = 0; i < notes.size(); i++) {
                    if (notes[i].pitch == pitches[inst] && std::abs(notes[i].start_time - step_time) < 0.01f) {
                        active = true;
                        note_idx = (int)i;
                        break;
                    }
                }

                // Cyberpunk aesthetics: neon green when active, dark grey when inactive
                ImVec4 btn_col = active ? ImVec4(0.0f, 0.9f, 0.4f, 1.0f) : ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
                if (step % 4 == 0) {
                    // Accent color for beat markers
                    if (!active) btn_col = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
                }
                
                ImGui::PushStyleColor(ImGuiCol_Button, btn_col);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.5f, 1.0f));
                
                if (ImGui::Button("##step", ImVec2(25, 25))) {
                    if (active) {
                        // Remove note
                        if (note_idx >= 0 && note_idx < (int)notes.size()) {
                            notes.erase(notes.begin() + note_idx);
                        }
                    } else {
                        // Add note
                        KuroDSP::MidiNote new_note;
                        new_note.pitch = pitches[inst];
                        new_note.start_time = step_time;
                        new_note.duration = snap_step * 0.9f; // short release
                        new_note.velocity = 0.9f;
                        notes.push_back(new_note);
                    }
                }
                
                ImGui::PopStyleColor(2);
                ImGui::SameLine();
                ImGui::PopID();
            }

            ImGui::NewLine();
            ImGui::PopID();
        }

        ImGui::Separator();
        if (ImGui::Button("Limpar Grid")) {
            for (int inst = 0; inst < 4; inst++) {
                timeline_mgr.clearNotes(inst);
            }
        }

        ImGui::End();
    }
}
