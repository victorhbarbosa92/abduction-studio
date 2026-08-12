#pragma once
#include "imgui.h"
#include "../core/TimelineManager.h"
#include "../audio/SynthEngine.h"
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <string>

#include "../core/ClipManager.h"

extern float dummy_vol[8];
extern float dummy_pan[8];
extern int channel_tracks[8];
extern bool track_mutes[20];
extern bool track_solos[20];

namespace KuroUI {

    // Custom vector-drawn FL Studio style circular knob
    inline bool Knob(const char* label, float* p_value, float v_min, float v_max, float radius = 9.0f) {
        ImGuiIO& io = ImGui::GetIO();
        
        std::string display_name = label;
        size_t hash_pos = display_name.find("##");
        if (hash_pos != std::string::npos) {
            display_name = display_name.substr(0, hash_pos);
        }
        
        float label_h = display_name.empty() ? 0.0f : 12.0f;
        float item_w = std::max(radius * 2.0f, display_name.empty() ? (radius * 2.0f) : ImGui::CalcTextSize(display_name.c_str()).x);
        
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 center = ImVec2(pos.x + item_w * 0.5f, pos.y + radius);
        
        ImGui::InvisibleButton(label, ImVec2(item_w, radius * 2.0f + label_h));
        bool value_changed = false;
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        
        if (is_hovered && io.MouseWheel != 0.0f) {
            float step = (v_max - v_min) / 40.0f;
            *p_value += io.MouseWheel * step;
            if (*p_value < v_min) *p_value = v_min;
            if (*p_value > v_max) *p_value = v_max;
            value_changed = true;
        }
        
        if (is_active && io.MouseDelta.y != 0.0f) {
            float step = (v_max - v_min) / 120.0f;
            *p_value -= io.MouseDelta.y * step;
            if (*p_value < v_min) *p_value = v_min;
            if (*p_value > v_max) *p_value = v_max;
            value_changed = true;
        }
        
        float angle_min = -135.0f * (3.14159265f / 180.0f);
        float angle_max = 135.0f * (3.14159265f / 180.0f);
        float norm_v = (*p_value - v_min) / (v_max - v_min);
        norm_v = std::clamp(norm_v, 0.0f, 1.0f);
        float angle = angle_min + norm_v * (angle_max - angle_min);
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        
        // Knob base circle
        draw_list->AddCircleFilled(center, radius, IM_COL32(35, 38, 42, 255), 16);
        
        // Knob ring indicator
        ImU32 ring_col = is_active ? IM_COL32(90, 200, 80, 255) : (is_hovered ? IM_COL32(100, 180, 240, 255) : IM_COL32(55, 60, 68, 255));
        draw_list->AddCircle(center, radius, ring_col, 16, 1.5f);
        
        // Arc line indicator
        draw_list->AddLine(center, ImVec2(center.x + std::cos(angle - 1.570796f) * (radius - 2.0f), center.y + std::sin(angle - 1.570796f) * (radius - 2.0f)), IM_COL32(230, 235, 240, 255), 2.0f);
        
        // Draw Text Label centered below knob
        if (!display_name.empty()) {
            float text_w = ImGui::CalcTextSize(display_name.c_str()).x;
            ImU32 text_col = is_hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(170, 175, 185, 255);
            draw_list->AddText(ImVec2(center.x - text_w * 0.5f, pos.y + radius * 2.0f + 1.0f), text_col, display_name.c_str());
        }
        
        if (is_hovered) {
            ImGui::SetTooltip("%s: %.2f", display_name.c_str(), *p_value);
        }
        
        return value_changed;
    }

    inline void RenderStepSequencer(bool* open, KuroDSP::TimelineManager& timeline_mgr, float bpm, ClipManager& clip_manager) {
        if (!ImGui::BeginChild("Channel Rack (FL Style)")) {
            ImGui::EndChild();
            return;
        }

        // Expanded 8 instruments mapping standard GM pitches
        const char* inst_names[] = { 
            "808 Kick", "808 Snare", "808 HiHat", "Bassline", 
            "Serum Chords", "Lead Synth", "909 Clap", "808 Open Hat" 
        };
        int pitches[] = { 36, 38, 42, 48, 60, 72, 39, 46 };

        float beat_duration = 60.0f / bpm;
        float snap_step = beat_duration / 4.0f; // 1/16 note

        static int selected_inst = 0;
        static float step_probs[8][16] = {
            {1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f},
            {1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f},
            {1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f},
            {1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f},
            {1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f},
            {1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f},
            {1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f},
            {1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f}
        };

        ImGui::Spacing();
        std::lock_guard<std::mutex> lock(timeline_mgr.timeline_mutex);

        // Header for Channel Rack / Step Sequencer
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::Text("   Pan  Vol  Trk    Channel Name");
        ImGui::PopStyleColor();
        ImGui::Separator();

        for (int inst = 0; inst < 8; inst++) {
            ImGui::PushID(inst);
            
            // --- 1. Mute/Solo LED Button ---
            ImVec2 led_pos = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            
            bool is_muted = ::track_mutes[::channel_tracks[inst]];
            ImU32 led_color = !is_muted ? IM_COL32(130, 230, 40, 255) : IM_COL32(60, 65, 70, 255);
            
            // Render LED circle
            draw_list->AddCircleFilled(ImVec2(led_pos.x + 6, led_pos.y + 11), 4.5f, led_color);
            // Draw a subtle border
            draw_list->AddCircle(ImVec2(led_pos.x + 6, led_pos.y + 11), 4.5f, IM_COL32(30, 30, 35, 255), 12, 1.0f);
            
            // Invisible button over LED to toggle mute
            ImGui::SetCursorScreenPos(led_pos);
            if (ImGui::InvisibleButton("##led_mute", ImVec2(14, 22))) {
                int trk = ::channel_tracks[inst];
                ::track_mutes[trk] = !::track_mutes[trk];
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                int trk = ::channel_tracks[inst];
                for (int t = 0; t < 8; t++) ::track_solos[t] = false;
                ::track_solos[trk] = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Mute (L-Click) / Solo (R-Click)");
            
            ImGui::SameLine(20);
            
            // --- 2. Circular Pan & Vol Knobs ---
            Knob("##pan", &::dummy_pan[inst], -1.0f, 1.0f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pan: %.2f", ::dummy_pan[inst]);
            ImGui::SameLine(44);
            Knob("##vol", &::dummy_vol[inst], 0.0f, 1.0f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Volume: %d%%", (int)(::dummy_vol[inst] * 100.0f));
            
            ImGui::SameLine(68);
            
            // --- 3. Target Mixer Track Selector ---
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(30, 32, 35, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(40, 45, 50, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(45, 50, 55, 255));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
            char trk_label[16];
            if (::channel_tracks[inst] >= 0) sprintf(trk_label, "%d", ::channel_tracks[inst] + 1);
            else sprintf(trk_label, "--");
            
            ImGui::Button(trk_label, ImVec2(24, 20));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
                float dy = ImGui::GetIO().MouseDelta.y;
                static float accum = 0.0f;
                accum -= dy;
                if (std::abs(accum) > 10.0f) {
                    ::channel_tracks[inst] += (accum > 0.0f) ? 1 : -1;
                    ::channel_tracks[inst] = std::max(0, std::min(::channel_tracks[inst], 7));
                    accum = 0.0f;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Target Mixer Track (Drag Up/Down)");
            ImGui::PopStyleColor(4);
            
            ImGui::SameLine(98);
            
            // --- 4. Channel Button (Instrument Selection) ---
            bool is_selected = (selected_inst == inst);
            ImGui::PushStyleColor(ImGuiCol_Button, is_selected ? ImVec4(0.32f, 0.35f, 0.38f, 1.0f) : ImVec4(0.20f, 0.22f, 0.24f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.38f, 0.42f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.30f, 0.33f, 1.0f));
            
            // Show variant name if sample loaded
            const char* btn_label = inst_names[inst];
            int cur_var = g_piano_synth.selected_variant[inst];
            if (g_piano_synth.drum_variants[inst][cur_var].loaded) {
                // Display variant name dynamically
                if (inst == 0) btn_label = g_piano_synth.kick_variant_names[cur_var];
                else if (inst == 1) btn_label = g_piano_synth.snare_variant_names[cur_var];
                else if (inst == 2) btn_label = g_piano_synth.hat_variant_names[cur_var];
            }
            
            if (ImGui::Button(btn_label, ImVec2(72, 22))) {
                selected_inst = inst;
                g_piano_synth.triggerNote(pitches[inst], 0.25f, 0.9f);
                
                extern bool show_sampler_settings;
                extern int active_sampler_channel;
                extern bool show_flex_browser;
                extern int active_flex_channel;
                
                if (inst == 3 || inst == 4 || inst == 5) {
                    show_flex_browser = true;
                    active_flex_channel = inst;
                    show_sampler_settings = false;
                } else {
                    show_sampler_settings = true;
                    active_sampler_channel = inst;
                    show_flex_browser = false;
                }
            }
            // Right-click to open variant selector popup
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                ImGui::OpenPopup(("##var_popup_" + std::to_string(inst)).c_str());
            }
            if (ImGui::BeginPopup(("##var_popup_" + std::to_string(inst)).c_str())) {
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.f), "Variante: %s", inst_names[inst]);
                ImGui::Separator();
                for (int v = 0; v < KuroAudio::SynthEngine::MAX_DRUM_VARIANTS; v++) {
                    auto& var_samp = g_piano_synth.drum_variants[inst][v];
                    bool loaded = var_samp.loaded;
                    const char* var_name = "?";
                    if (inst == 0) var_name = g_piano_synth.kick_variant_names[v];
                    else if (inst == 1) var_name = g_piano_synth.snare_variant_names[v];
                    else if (inst == 2) var_name = g_piano_synth.hat_variant_names[v];
                    else if (inst == 6) var_name = g_piano_synth.crash_variant_names[v];
                    else { static char vbuf[16]; snprintf(vbuf, sizeof(vbuf), "Var %d", v+1); var_name = vbuf; }
                    
                    bool is_cur = (g_piano_synth.selected_variant[inst] == v);
                    if (!loaded) ImGui::BeginDisabled();
                    if (ImGui::Selectable(var_name, is_cur)) {
                        g_piano_synth.selected_variant[inst] = v;
                    }
                    if (!loaded) ImGui::EndDisabled();
                }
                ImGui::EndPopup();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("L-Click: Select | R-Click: Trocar Sample");
            ImGui::PopStyleColor(3);

            ImGui::SameLine(172);
            ImGui::SetNextItemWidth(25);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(30, 32, 35, 255));
            ImGui::DragInt("##limit", &timeline_mgr.track_steps_limit[inst], 0.1f, 1, 16, "%d", ImGuiSliderFlags_NoInput);
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Passos do Canal (Polirritmia): %d", timeline_mgr.track_steps_limit[inst]);
            
            ImGui::SameLine(202);
            
            // --- 5. Selection Vertical Bar Indicator ---
            ImVec2 bar_pos = ImGui::GetCursorScreenPos();
            ImU32 bar_color = is_selected ? IM_COL32(90, 200, 80, 255) : IM_COL32(40, 43, 46, 255);
            draw_list->AddRectFilled(ImVec2(bar_pos.x, bar_pos.y + 2), ImVec2(bar_pos.x + 3, bar_pos.y + 20), bar_color, 1.0f);
            
            ImGui::SameLine(212);
            
            // --- 6. 16 steps pads (Groups of 4 alternating colors) ---
            auto& current_pattern = clip_manager.getCurrentPattern();
            auto& notes = current_pattern.getChannelNotes(inst);
            int limit = timeline_mgr.track_steps_limit[inst]; // protect bounds

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 4));

            for (int step = 0; step < 16; step++) {
                ImGui::PushID(step);
                
                bool out_of_bounds = (step >= limit);
                if (out_of_bounds) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.15f);

                float step_time = step * snap_step;
                bool active = false;
                int note_idx = -1;
                for (size_t i = 0; i < notes.size(); i++) {
                    if (std::abs(notes[i].start_time - step_time) < 0.01f) {
                        active = true;
                        note_idx = (int)i;
                        break;
                    }
                }

                // FL Studio standard step pads coloring (alternating gray/brown groups of 4)
                ImVec4 btn_col;
                if (active) {
                    btn_col = ImVec4(0.43f, 0.90f, 0.38f, 1.0f); // Neon Green active
                } else {
                    if ((step / 4) % 2 == 0) {
                        btn_col = ImVec4(0.35f, 0.37f, 0.40f, 1.0f); // Light gray-slate
                    } else {
                        btn_col = ImVec4(0.24f, 0.22f, 0.21f, 1.0f); // FL Brick-brown
                    }
                }
                
                ImGui::PushStyleColor(ImGuiCol_Button, btn_col);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.95f, 0.50f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.85f, 0.30f, 1.0f));
                
                // Drawing rectangular step buttons
                if (ImGui::Button("##step", ImVec2(18, 22))) {
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
                            g_piano_synth.triggerNote(pitches[inst], 0.25f, 0.9f, inst);
                        }
                    }
                }
                
                ImGui::PopStyleColor(3);
                if (out_of_bounds) ImGui::PopStyleVar();
                
                ImGui::SameLine();
                ImGui::PopID();
            }

            ImGui::PopStyleVar(); // ItemSpacing
            ImGui::NewLine();
            ImGui::PopID();
        }

        ImGui::Separator();
        
        // Probability Chance editor at the bottom
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Chance / Trigger Probability Editor: %s", inst_names[selected_inst]);
        ImGui::Spacing();
        
        auto& current_pattern = clip_manager.getCurrentPattern();
        auto& sel_notes = current_pattern.getChannelNotes(selected_inst);
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
                if (std::abs(sel_notes[i].start_time - step_time) < 0.01f) {
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
            
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Step %d Chance: %d%%", step + 1, (int)(current_prob * 100.0f));
            }
            
            ImGui::SameLine(0, 10);
            ImGui::PopID();
        }
        ImGui::EndChild();
        
        ImGui::Separator();
        if (ImGui::Button("Limpar Grid", ImVec2(120, 25))) {
            for (int inst = 0; inst < 8; inst++) {
                timeline_mgr.clearNotes(pitches[inst]);
            }
        }
        ImGui::EndChild();
    }
}
