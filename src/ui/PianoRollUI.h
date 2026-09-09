#pragma once
#include "imgui.h"
#include "../core/TimelineManager.h"
#include "Commands.h"
#include <string>
#include <vector>
#include <algorithm>
#include <mutex>

#include "../audio/SynthEngine.h"
#include "PianoTextureManager.h"

extern KuroAudio::SynthEngine g_piano_synth;

#include "../core/ClipManager.h"
#include "ChannelInstrumentManager.h"

namespace KuroUI {
    extern CommandManager g_command_manager;
    extern bool g_open_psy_rolling_bass_window;

    enum class PianoRollTool { Draw, Paint, Erase, Mute, Slice, Select };

    inline bool IsPitchInScale(int pitch, int root_idx, int scale_type) {
        if (scale_type <= 0) return true; // Scale Highlighting Off
        int note_degree = ((pitch - root_idx) % 12 + 12) % 12;
        static const int scale_masks[] = {
            0b111111111111, // 0: Off
            0b101010110101, // 1: Major (0, 2, 4, 5, 7, 9, 11)
            0b010110101101, // 2: Natural Minor (0, 2, 3, 5, 7, 8, 10)
            0b100110101101, // 3: Harmonic Minor (0, 2, 3, 5, 7, 8, 11)
            0b101010101101, // 4: Melodic Minor (0, 2, 3, 5, 7, 9, 11)
            0b011010101101, // 5: Dorian (0, 2, 3, 5, 7, 9, 10)
            0b010110101110, // 6: Phrygian (0, 1, 3, 5, 7, 8, 10)
            0b101010100101, // 7: Lydian (0, 2, 4, 6, 7, 9, 11)
            0b011010110101, // 8: Mixolydian (0, 2, 4, 5, 7, 9, 10)
            0b010010101001, // 9: Pentatonic Minor (0, 3, 5, 7, 10)
            0b010010111001, // 10: Blues (0, 3, 5, 6, 7, 10)
        };
        if (scale_type >= 0 && scale_type < (int)(sizeof(scale_masks)/sizeof(scale_masks[0]))) {
            return (scale_masks[scale_type] & (1 << note_degree)) != 0;
        }
        return true;
    }

    inline void ApplyStrumming(std::vector<KuroDSP::MidiNote>& notes, float strum_step_sec = 0.025f, bool ascending = true, float vel_decay = 0.05f) {
        if (notes.empty()) return;
        std::vector<int> target_indices;
        for (size_t i = 0; i < notes.size(); i++) {
            if (notes[i].is_selected) target_indices.push_back((int)i);
        }
        if (target_indices.empty()) {
            for (size_t i = 0; i < notes.size(); i++) target_indices.push_back((int)i);
        }

        std::vector<std::vector<int>> chord_groups;
        std::vector<bool> processed(target_indices.size(), false);

        for (size_t i = 0; i < target_indices.size(); i++) {
            if (processed[i]) continue;
            std::vector<int> group;
            group.push_back(target_indices[i]);
            processed[i] = true;
            float base_time = notes[target_indices[i]].start_time;

            for (size_t j = i + 1; j < target_indices.size(); j++) {
                if (!processed[j] && std::abs(notes[target_indices[j]].start_time - base_time) < 0.06f) {
                    group.push_back(target_indices[j]);
                    processed[j] = true;
                }
            }
            chord_groups.push_back(group);
        }

        for (auto& group : chord_groups) {
            if (group.size() < 2) continue;
            std::sort(group.begin(), group.end(), [&](int a, int b) {
                return ascending ? (notes[a].pitch < notes[b].pitch) : (notes[a].pitch > notes[b].pitch);
            });
            float base_start = notes[group[0]].start_time;
            for (size_t k = 0; k < group.size(); k++) {
                int idx = group[k];
                notes[idx].start_time = base_start + k * strum_step_sec;
                notes[idx].velocity = std::clamp(notes[idx].velocity * (1.0f - k * vel_decay), 0.1f, 1.0f);
            }
        }
    }

    inline void ApplyChop(std::vector<KuroDSP::MidiNote>& notes, int sub_count = 4, bool vel_ramp = true) {
        if (notes.empty() || sub_count <= 1) return;
        std::vector<KuroDSP::MidiNote> new_notes;
        bool any_selected = false;
        for (auto& check : notes) if (check.is_selected) { any_selected = true; break; }

        for (auto& n : notes) {
            if (any_selected && !n.is_selected) {
                new_notes.push_back(n);
                continue;
            }
            float sub_dur = n.duration / (float)sub_count;
            for (int i = 0; i < sub_count; i++) {
                KuroDSP::MidiNote sub_n = n;
                sub_n.start_time = n.start_time + i * sub_dur;
                sub_n.duration = sub_dur * 0.92f;
                if (vel_ramp) {
                    float factor = 0.35f + 0.65f * ((float)(i + 1) / (float)sub_count);
                    sub_n.velocity = std::clamp(n.velocity * factor, 0.1f, 1.0f);
                }
                new_notes.push_back(sub_n);
            }
        }
        notes = new_notes;
    }

    // ── FL STUDIO RIFF MACHINE & ARPEGGIATOR MOTOR ──────────────────────────────
    inline void ApplyRiffMachine(std::vector<KuroDSP::MidiNote>& notes, int genre_style, int pattern_type, int octave_span, float gate_length, float bpm, int channel_idx) {
        notes.clear();
        float beat_sec = 60.0f / (bpm > 20.0f ? bpm : 140.0f);
        float step_sec = beat_sec * 0.25f; // 1/16th note

        // Escalas base para geração musical (ex: Root C3 = 48 ou A3 = 57)
        int root = (genre_style == 0) ? 36 : (genre_style == 1) ? 57 : (genre_style == 2) ? 60 : 48; // C2, A3, C4, C3
        
        // Padrões de Riff Machine Inspirados no FL Studio
        std::vector<int> pitch_seq;
        std::vector<float> vel_seq;
        std::vector<bool> gate_seq;

        if (genre_style == 0) { // 👽 Psytrance Rolling & Arp
            pitch_seq = { 0, 0, 0, 0,  12, 0, 3, 0,  7, 0, 10, 12,  15, 12, 10, 7 };
            vel_seq   = { 0.95f, 0.75f, 0.80f, 0.75f,  0.90f, 0.75f, 0.85f, 0.75f,  0.92f, 0.75f, 0.85f, 0.90f,  0.95f, 0.85f, 0.80f, 0.75f };
            gate_seq  = { false, true, true, true,   true, true, true, true,    true, true, true, true,     true, true, true, true }; // Kick offbeat
        } else if (genre_style == 1) { // 🚀 Synthwave 80s Darksynth
            pitch_seq = { 0, 0, 7, 7,  12, 12, 10, 10,  8, 8, 7, 7,  5, 5, 3, 3 };
            vel_seq   = { 0.9f, 0.8f, 0.85f, 0.8f,  0.95f, 0.8f, 0.9f, 0.8f,  0.85f, 0.8f, 0.85f, 0.8f,  0.8f, 0.75f, 0.8f, 0.75f };
            gate_seq  = { true, true, true, true,   true, true, true, true,   true, true, true, true,   true, true, true, true };
        } else if (genre_style == 2) { // 🎹 Melodic Techno / Progressive House
            pitch_seq = { 0, 12, 7, 15,  3, 10, 14, 7,  0, 12, 8, 15,  5, 12, 10, 7 };
            vel_seq   = { 0.95f, 0.70f, 0.85f, 0.65f,  0.90f, 0.70f, 0.85f, 0.65f,  0.95f, 0.70f, 0.85f, 0.65f,  0.90f, 0.70f, 0.85f, 0.65f };
            gate_seq  = { true, true, true, true,   true, true, true, true,   true, true, true, true,   true, true, true, true };
        } else { // 🔥 Trap Hi-Hat / Melodic Bell Motif
            pitch_seq = { 0, 0, 12, 0,  7, 0, 0, 10,  12, 12, 15, 12,  10, 7, 5, 3 };
            vel_seq   = { 0.9f, 0.6f, 0.85f, 0.6f,  0.9f, 0.6f, 0.6f, 0.85f,  0.95f, 0.8f, 0.9f, 0.8f,  0.85f, 0.8f, 0.75f, 0.7f };
            gate_seq  = { true, true, true, true,   true, true, true, true,   true, true, true, true,   true, true, true, true };
        }

        // Gera 16 passos rítmicos perfeitamente alinhados ao tempo
        for (int step = 0; step < 16; ++step) {
            if (!gate_seq[step]) continue;

            int oct_offset = (pattern_type == 1) ? ((step % (octave_span + 1)) * 12) : 0;
            int final_pitch = std::clamp(root + pitch_seq[step] + oct_offset, 20, 110);
            float s_time = step * step_sec;
            float dur = step_sec * std::clamp(gate_length, 0.1f, 1.2f);
            float vel = vel_seq[step];

            KuroDSP::MidiNote n(final_pitch, s_time, dur, vel, 1.0f, channel_idx);
            notes.push_back(n);
        }
    }

    // ── AI NOTE INPAINTING & GENERATIVE MUTATION MOTOR (SUNO STUDIO / FL HYBRID) ──
    inline void ApplyAINoteInpainting(std::vector<KuroDSP::MidiNote>& notes, int mode, float energy_pct, int root_idx, int scale_type, float bpm, int channel_idx) {
        float beat_sec = 60.0f / (bpm > 20.0f ? bpm : 140.0f);
        if (notes.empty()) {
            int base_pitch = (channel_idx == 1 || channel_idx == 2) ? 36 : 60;
            for (int b = 0; b < 16; b++) {
                float t = b * (beat_sec * 0.25f);
                notes.push_back(KuroDSP::MidiNote(base_pitch, t, beat_sec * 0.22f, 0.85f, 1.0f, channel_idx));
            }
        }

        bool any_sel = std::any_of(notes.begin(), notes.end(), [](const KuroDSP::MidiNote& x){ return x.is_selected; });
        float energy_norm = std::clamp(energy_pct * 0.01f, 0.05f, 1.0f);

        if (mode == 0) { // 1. Melodic Arp Mutation
            std::vector<int> scale_degrees;
            for (int p = 36; p <= 96; p++) {
                if (IsPitchInScale(p, root_idx, scale_type)) scale_degrees.push_back(p);
            }
            if (scale_degrees.empty()) scale_degrees = {60, 62, 64, 65, 67, 69, 71, 72};

            for (auto& n : notes) {
                if (any_sel && !n.is_selected) continue;
                int jump = (int)((rand() % 7 - 3) * energy_norm * 3.0f);
                int target_p = std::clamp(n.pitch + jump, 36, 96);
                int best_p = scale_degrees[0];
                int best_diff = 999;
                for (int sp : scale_degrees) {
                    int diff = std::abs(sp - target_p);
                    if (diff < best_diff) { best_diff = diff; best_p = sp; }
                }
                n.pitch = best_p;
                n.velocity = std::clamp(0.70f + 0.30f * ((float)(rand() % 100) / 100.0f) * energy_norm, 0.4f, 1.0f);
            }
        } else if (mode == 1) { // 2. Psy Bassline Groove Morph
            for (size_t i = 0; i < notes.size(); i++) {
                if (any_sel && !notes[i].is_selected) continue;
                int step = (int)(i % 4);
                if (step == 0) notes[i].velocity = 0.95f;
                else {
                    notes[i].velocity = 0.75f + 0.15f * energy_norm;
                    if (energy_norm > 0.5f && (i % 8 == 7)) notes[i].pitch += 12;
                }
                notes[i].duration = std::min(notes[i].duration, beat_sec * 0.20f);
            }
        } else if (mode == 2) { // 3. Chord Progression Voicing
            std::vector<KuroDSP::MidiNote> added_chords;
            for (auto& n : notes) {
                if (any_sel && !n.is_selected) continue;
                int third = n.pitch + 4;
                int fifth = n.pitch + 7;
                if (scale_type > 0) {
                    if (!IsPitchInScale(third, root_idx, scale_type)) third--;
                    if (!IsPitchInScale(fifth, root_idx, scale_type)) fifth--;
                }
                KuroDSP::MidiNote n3 = n; n3.pitch = std::clamp(third, 20, 110); n3.velocity *= 0.85f;
                KuroDSP::MidiNote n5 = n; n5.pitch = std::clamp(fifth, 20, 110); n5.velocity *= 0.80f;
                added_chords.push_back(n3);
                added_chords.push_back(n5);
            }
            for (const auto& ac : added_chords) notes.push_back(ac);
        } else if (mode == 3) { // 4. Humanize & Swing Groove
            for (auto& n : notes) {
                if (any_sel && !n.is_selected) continue;
                float micro_shift = ((float)(rand() % 100 - 50) / 1000.0f) * 0.035f * energy_norm;
                n.start_time = std::max(0.0f, n.start_time + micro_shift);
                n.velocity = std::clamp(n.velocity + ((float)(rand() % 40 - 20) / 100.0f) * energy_norm, 0.3f, 1.0f);
            }
        }
    }

    inline bool g_piano_roll_show_ghost_notes = true;

    inline void RenderPianoRoll(bool* open, KuroDSP::TimelineManager& timeline_mgr, int track_idx, float bpm, unsigned long long* current_sample_ptr, bool is_playing, ClipManager& clip_manager, std::vector<KuroDSP::MidiNote>* ghost_notes = nullptr) {
        if (!*open) return;
        static int active_ch_idx = 0;
        static int last_opened_track_idx = -1;
        if (track_idx >= 0 && track_idx < 16) {
            active_ch_idx = track_idx;
        }
        int ch_idx = std::clamp(active_ch_idx, 0, 15);

        static int current_snap_option = 0; // Default to 4/4 (1/4 note)
        static PianoRollTool current_tool = PianoRollTool::Draw;
        static int current_stamp_idx = 0;
        static int current_root_note = 0;  // 0=C
        static int current_scale_type = 0; // 0=Off
        static bool open_strum_modal = false;
        static bool open_chop_modal = false;
        static bool open_riff_modal = false;
        static bool open_ai_inpainting_modal = false;
        static int bottom_lane_mode = 0;   // 0=Velocity, 1=Pan, 2=Pitch Offset
        static bool auto_sync_playlist = true;

        // Estado do Piano Roll (Definidos antes para acesso pelo botão FIT)
        static float pan_x = 0.0f;
        static float pan_y = 1200.0f; // Centrado nas oitavas médias
        static float zoom_x = 100.0f; // Pixels por segundo
        static float zoom_y = 24.0f;  // Pixels por tecla (ideal para ver 2.5 oitavas)
        static bool request_auto_fit = false;
        static bool is_dragging_pr_h = false;
        static bool is_dragging_pr_v = false;

        if (track_idx != last_opened_track_idx) {
            last_opened_track_idx = track_idx;
            request_auto_fit = true;
        }

        ImGui::SetNextWindowSize(ImVec2(1050, 640), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(245, 50), ImGuiCond_FirstUseEver);
        extern bool g_need_focus_piano_roll;
        if (g_need_focus_piano_roll) {
            ImGui::SetNextWindowFocus();
            g_need_focus_piano_roll = false;
        }
        ImGuiWindowFlags pr_flags = ImGuiWindowFlags_MenuBar;
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.07f, 0.09f, 1.0f));
        if (!ImGui::Begin("Piano Roll###GlobalPianoRollWindow", open, pr_flags)) {
            ImGui::PopStyleColor();
            ImGui::End();
            return;
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 window_p0 = ImGui::GetWindowPos();
        ImVec2 window_sz = ImGui::GetWindowSize();
        ImVec2 window_p1 = ImVec2(window_p0.x + window_sz.x, window_p0.y + window_sz.y);

        // Moldura Externa de Neon Ciano Reluzente
        draw_list->AddRect(ImVec2(window_p0.x + 2, window_p0.y + 2), ImVec2(window_p1.x - 2, window_p1.y - 2), IM_COL32(0, 229, 255, 255), 8.0f, 0, 2.5f);

        // --- RACK DE CONTROLE E VISUALIZADOR DE ESTRUTURA NO TOPO ---
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.10f, 0.14f, 1.0f));
        ImGui::BeginChild("##TopControlRack", ImVec2(0, 52), true);
        {
            ImDrawList* rack_draw = ImGui::GetWindowDrawList();
            ImVec2 r_p0 = ImGui::GetCursorScreenPos();

            // Botões de Transporte (Play / Stop)
            ImGui::SetCursorScreenPos(ImVec2(r_p0.x + 8, r_p0.y + 8));
            if (ImGui::Button("▶", ImVec2(30, 30))) {
                KuroUI::TransportController::Play(timeline_mgr);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Play / Reproduzir (Barra de Espaço)");
            ImGui::SameLine();
            if (ImGui::Button("■", ImVec2(30, 30))) {
                KuroUI::TransportController::Stop(timeline_mgr);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Stop / Parar e Rebobinar");
            ImGui::SameLine();

            // 1. Target Channel Selector com Tag Colorida
            extern std::string track_names[20];
            extern int selected_track_idx;
            char ch_combo_label[64];
            int cur_c = std::clamp(ch_idx, 0, 15);
            snprintf(ch_combo_label, sizeof(ch_combo_label), "%02d. %s", cur_c + 1, ::track_names[cur_c].c_str());
            
            ImGui::SetNextItemWidth(145);
            if (ImGui::BeginCombo("##pr_channel_select", ch_combo_label)) {
                for (int t = 0; t < 16; t++) {
                    bool is_sel = (ch_idx == t);
                    char t_lbl[64];
                    snprintf(t_lbl, sizeof(t_lbl), "%02d. %s", t + 1, ::track_names[t].c_str());
                    if (ImGui::Selectable(t_lbl, is_sel)) {
                        ch_idx = t;
                        active_ch_idx = t;
                        selected_track_idx = t;
                        request_auto_fit = true;
                    }
                    if (is_sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine(0, 4);

            // 1.1 Pílula do Instrumento Ativo com Ícone e 1-Clique para Abrir Editor
            char inst_pill_lbl[96];
            snprintf(inst_pill_lbl, sizeof(inst_pill_lbl), "%s %s##pr_pill", GetChannelIcon(cur_c), GetChannelInstrumentName(cur_c));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.16f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.24f, 0.32f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.30f, 0.38f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.90f, 1.00f, 1.0f));
            if (ImGui::Button(inst_pill_lbl, ImVec2(0, 24))) {
                TriggerOpenInstrument(cur_c);
            }
            ImGui::PopStyleColor(4);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Abrir Editor Gráfico: %s (1 Clique)", GetChannelInstrumentName(cur_c));
            }
            ImGui::SameLine(0, 3);

            // 1.2 Botão de Menu de 4 Pontinhos (☷) para Trocar Instrumento Diretamente do Piano Roll
            std::string pr_popup_id = "##pr_inst_menu_" + std::to_string(cur_c);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.18f, 0.24f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.28f, 0.38f, 1.0f));
            if (ImGui::Button("☷##pr_m4_btn", ImVec2(24, 24))) {
                ImGui::OpenPopup(pr_popup_id.c_str());
            }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Substituir Instrumento / Carregar Sample / Opções da Faixa");
            }

            if (ImGui::BeginPopup(pr_popup_id.c_str())) {
                float beat_dur = 60.0f / (bpm > 20.0f ? bpm : 140.0f);
                float s_step = beat_dur * 0.25f;
                RenderChannelInstrumentMenu(cur_c, clip_manager, s_step, nullptr);
                ImGui::EndPopup();
            }
            ImGui::SameLine(0, 4);

            // 1.3 Botão Rápido para Abrir o Editor de Sample (ADSR, Pitch, Waveform)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.22f, 0.28f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.32f, 0.42f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.85f, 1.00f, 1.0f));
            if (ImGui::Button("🎛️ Sample##pr_sample_btn", ImVec2(0, 24))) {
                extern bool show_sampler_settings;
                extern int active_sampler_channel;
                selected_track_idx = cur_c;
                active_sampler_channel = cur_c;
                show_sampler_settings = true;
            }
            ImGui::PopStyleColor(3);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Abrir Editor de Sample desta Faixa (ADSR, Pitch, Waveform, Filtro)");
            }
            ImGui::SameLine(0, 6);

            // 2. Target Pattern Selector
            int cur_p = clip_manager.current_pattern_idx;
            char pat_combo_label[64];
            if (cur_p >= 0 && cur_p < (int)clip_manager.global_patterns.size()) {
                snprintf(pat_combo_label, sizeof(pat_combo_label), "%s", clip_manager.global_patterns[cur_p].name.c_str());
            } else {
                snprintf(pat_combo_label, sizeof(pat_combo_label), "Pattern 1");
            }

            ImGui::SetNextItemWidth(115);
            if (ImGui::BeginCombo("##pr_pattern_select", pat_combo_label)) {
                for (int p = 0; p < (int)clip_manager.global_patterns.size(); p++) {
                    bool is_sel = (cur_p == p);
                    if (ImGui::Selectable(clip_manager.global_patterns[p].name.c_str(), is_sel)) {
                        clip_manager.current_pattern_idx = p;
                        request_auto_fit = true;
                    }
                    if (is_sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine(0, 6);

            // Seletor de Snap
            ImGui::SetNextItemWidth(80);
            const char* snap_options[] = { "(1/16)", "(1/8)", "(1/4)", "(Free)" };
            ImGui::Combo("##snap_combo", &current_snap_option, snap_options, IM_ARRAYSIZE(snap_options)); ImGui::SameLine(0, 6);

            // Botão Mágico FIT NOTAS (Centraliza automaticamente a tela nas notas deste instrumento)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.35f, 0.45f, 1.0f));
            if (ImGui::Button("🔍 FIT NOTAS") || request_auto_fit) {
                request_auto_fit = false;
                if (!clip_manager.global_patterns.empty() && clip_manager.current_pattern_idx < (int)clip_manager.global_patterns.size()) {
                    auto& cur_notes = clip_manager.global_patterns[clip_manager.current_pattern_idx].getChannelNotes(ch_idx);
                    if (!cur_notes.empty()) {
                        int min_p = 127, max_p = 0;
                        float min_t = 1e9f;
                        for (const auto& n : cur_notes) {
                            min_p = (std::min)(min_p, n.pitch);
                            max_p = (std::max)(max_p, n.pitch);
                            min_t = (std::min)(min_t, n.start_time);
                        }
                        int center_pitch = (min_p + max_p) / 2;
                        int row = (120 - 1) - center_pitch;
                        pan_y = std::max(0.0f, row * zoom_y - 200.0f);
                        if (min_t < 1e8f) {
                            pan_x = std::max(0.0f, min_t * zoom_x - 30.0f);
                        }
                    } else {
                        pan_y = (120 - 1 - 48) * zoom_y - 200.0f; // C3 padrão
                    }
                }
            }
            ImGui::PopStyleColor(); ImGui::SameLine(0, 8);

            // Seletor de Templates de Tracks Prontas
            ImGui::SetNextItemWidth(140);
            const char* tpl_options[] = { "🎵 Templates...", "👽 Psytrance Rolling", "🚀 Cyberpunk Darksynth", "🎹 Melodic Techno", "🔥 Tech House Pump" };
            static int selected_tpl = 0;
            if (ImGui::Combo("##track_templates", &selected_tpl, tpl_options, IM_ARRAYSIZE(tpl_options))) {
                if (selected_tpl > 0) {
                    float new_bpm = bpm;
                    clip_manager.loadTrackTemplate(selected_tpl - 1, new_bpm);
                    selected_tpl = 0;
                    request_auto_fit = true;
                }
            }

            // HUD do Tempo e Seção da Música (Canto Direito do Top Control Rack)
            uint64_t m_frame = timeline_mgr.getMasterFrame();
            float cur_time_sec = (float)m_frame / 44100.0f;
            float bar_len_sec = (60.0f / (bpm > 20.0f ? bpm : 140.0f)) * 4.0f;
            int cur_bar = 1 + (int)(cur_time_sec / bar_len_sec);
            int cur_sec = (int)cur_time_sec;
            int cur_ms = (int)((cur_time_sec - cur_sec) * 10.0f);

            const char* sec_tag = "INTRO";
            ImU32 sec_col = IM_COL32(255, 215, 0, 255);
            if (cur_bar >= 9 && cur_bar < 17) { sec_tag = "BUILD-UP 1"; sec_col = IM_COL32(0, 200, 255, 255); }
            else if (cur_bar >= 17 && cur_bar < 33) { sec_tag = "DROP"; sec_col = IM_COL32(180, 0, 255, 255); }
            else if (cur_bar >= 33 && cur_bar < 49) { sec_tag = "BREAKDOWN"; sec_col = IM_COL32(255, 140, 0, 255); }
            else if (cur_bar >= 49) { sec_tag = "MAIN DROP 2"; sec_col = IM_COL32(255, 0, 128, 255); }

            char hud_buf[64];
            snprintf(hud_buf, sizeof(hud_buf), "%02d:%02d.%d | Bar %d [%s]", cur_sec / 60, cur_sec % 60, cur_ms, cur_bar, sec_tag);
            
            float hud_w = 210.0f;
            ImVec2 hud_p0(r_p0.x + ImGui::GetContentRegionAvail().x + 190.0f - hud_w, r_p0.y + 6.0f);
            ImVec2 hud_p1(hud_p0.x + hud_w, hud_p0.y + 36.0f);
            rack_draw->AddRectFilled(hud_p0, hud_p1, IM_COL32(12, 18, 28, 240), 4.0f);
            rack_draw->AddRect(hud_p0, hud_p1, sec_col, 4.0f, 0, 1.5f);
            rack_draw->AddText(ImVec2(hud_p0.x + 8.0f, hud_p0.y + 9.0f), sec_col, hud_buf);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        
        ImGui::Separator();

        // --- BARRA SECUNDÁRIA DE FERRAMENTAS E ESCALAS (FL STUDIO STYLE) ---
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.07f, 0.09f, 0.12f, 1.0f));
        ImGui::BeginChild("##SubToolBarFL", ImVec2(0, 32), false, ImGuiWindowFlags_NoScrollbar);
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
            
            // 1. Destaque de Escalas (Scale Highlighting)
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "🎼 Escala Guia:"); ImGui::SameLine();
            const char* root_names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
            ImGui::SetNextItemWidth(55);
            ImGui::Combo("##scale_root", &current_root_note, root_names, IM_ARRAYSIZE(root_names)); ImGui::SameLine();

            const char* scale_names[] = { "Desativada", "Maior (Major)", "Menor (Minor)", "Harmônica Menor", "Melódica Menor", "Dorian", "Phrygian", "Lydian", "Mixolydian", "Pentatônica", "Blues" };
            ImGui::SetNextItemWidth(140);
            ImGui::Combo("##scale_type", &current_scale_type, scale_names, IM_ARRAYSIZE(scale_names)); ImGui::SameLine();

            ImGui::TextDisabled("|"); ImGui::SameLine();

            // 2. Ferramentas FL Studio (Strum, Chop, Quantize)
            if (ImGui::Button("🎸 Strum (Alt+S)")) { open_strum_modal = true; } 
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Simular dedilhado de guitarra/violão em acordes (Alt+S)");
            ImGui::SameLine();

            if (ImGui::Button("✂️ Chop (Alt+U)")) { open_chop_modal = true; } 
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Subdividir notas em rolls rápidos de Trap (Alt+U)");
            ImGui::SameLine();

            if (ImGui::Button("🚀 Riff Machine (Alt+R)")) { open_riff_modal = true; } 
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Gerador Inteligente de Melodias, Riffs e Arpeggios Estilo FL Studio (Alt+R)");
            ImGui::SameLine();

            if (ImGui::Button("⚡ Quantize (Ctrl+Q)")) {
                auto& notes_q = clip_manager.global_patterns[clip_manager.current_pattern_idx].getChannelNotes(ch_idx);
                float b_dur = 60.0f / bpm;
                float div = (current_snap_option == 0) ? 1.0f : (current_snap_option == 1) ? 2.0f : (current_snap_option == 2) ? 4.0f : 8.0f;
                float s_step = b_dur / div;
                bool any_sel = std::any_of(notes_q.begin(), notes_q.end(), [](const KuroDSP::MidiNote& x){ return x.is_selected; });
                for (auto& n : notes_q) {
                    if (any_sel && !n.is_selected) continue;
                    n.start_time = std::round(n.start_time / s_step) * s_step;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Quantizar notas selecionadas para a grade atual (Ctrl+Q)");

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.85f, 0.95f));
            if (ImGui::Button("✨ AI Inpaint (Mutar)")) { open_ai_inpainting_modal = true; }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("AI Note Inpainting: Variação e mutação generativa de melodias, acordes e baixo (Suno / FL)");

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.65f, 0.55f, 0.95f));
            if (ImGui::Button("🛸 Rolling Bass")) {
                g_open_psy_rolling_bass_window = true;
                if (open) *open = false;
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Abrir Gerador Especializado de Rolling Bassline de Psytrance (Kuro Rolling Bass Engine)");

            ImGui::SameLine();
            if (auto_sync_playlist) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.65f, 0.35f, 0.95f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.28f, 0.32f, 0.95f));
            }
            if (ImGui::Button(auto_sync_playlist ? "📌 Auto-Sync Playlist [ON]" : "📌 Auto-Sync Playlist [OFF]")) {
                auto_sync_playlist = !auto_sync_playlist;
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Quando ativado, envia e atualiza automaticamente este Pattern na Trilha correspondente da Playlist!");

            ImGui::SameLine();
            if (ImGui::Button("🚀 Enviar p/ Playlist")) {
                auto& p_notes = clip_manager.global_patterns[clip_manager.current_pattern_idx].getChannelNotes(ch_idx);
                float max_t = 0.0f;
                for (const auto& n : p_notes) {
                    if (n.start_time + n.duration > max_t) max_t = n.start_time + n.duration;
                }
                float bar_len = (60.0f / bpm) * 4.0f;
                float clip_len = (std::max)(bar_len * 4.0f, std::ceil(max_t / bar_len) * bar_len);
                int pat_id = clip_manager.global_patterns[clip_manager.current_pattern_idx].id;
                clip_manager.addPatternClip(ch_idx, 0.0f, clip_len, pat_id);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Cria imediatamente um bloco deste Pattern na Playlist na faixa atual");

            ImGui::SameLine();
            if (g_piano_roll_show_ghost_notes) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.32f, 0.38f, 0.48f, 0.95f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.20f, 0.24f, 0.95f));
            }
            if (ImGui::Button(g_piano_roll_show_ghost_notes ? "👻 Ghost Notes [ON]" : "👻 Ghost Notes [OFF]")) {
                g_piano_roll_show_ghost_notes = !g_piano_roll_show_ghost_notes;
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Exibir Notas Fantasma (Ghost Notes) de outros canais ativos do mesmo Pattern");

            // --- MODAL DO AI NOTE INPAINTING (SUNO / FL) ---
            if (open_ai_inpainting_modal) {
                ImGui::OpenPopup("AINoteInpaintingModal");
                open_ai_inpainting_modal = false;
            }

            if (ImGui::BeginPopupModal("AINoteInpaintingModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "✨ PIANO ROLL AI NOTE INPAINTING & MUTATION");
                ImGui::TextDisabled("Mutação generativa de notas selecionadas ou padrão atual no tom da escala");
                ImGui::Separator();
                ImGui::Spacing();

                static int ai_mode = 0;
                static float ai_energy = 75.0f;
                static bool ai_has_gen = false;
                static std::vector<KuroDSP::MidiNote> backup_orig_notes;

                const char* ai_modes[] = { "🎹 Melodic Arp Mutation", "⚡ Psy Bassline Groove Morph", "🎼 Chord Progression Voicing", "🥁 Humanize & Swing Groove" };
                ImGui::Combo("Modo de Mutação", &ai_mode, ai_modes, IM_ARRAYSIZE(ai_modes));

                ImGui::SliderFloat("Nível de Energia (1% a 100%)", &ai_energy, 1.0f, 100.0f, "%.0f%%");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("⚡ MUTAR NOTAS COM IA", ImVec2(200, 32))) {
                    auto& pattern_notes = clip_manager.global_patterns[clip_manager.current_pattern_idx].getChannelNotes(ch_idx);
                    backup_orig_notes = pattern_notes;
                    ApplyAINoteInpainting(pattern_notes, ai_mode, ai_energy, current_root_note, current_scale_type, bpm, ch_idx);
                    ai_has_gen = true;
                }

                if (ai_has_gen) {
                    ImGui::SameLine();
                    if (ImGui::Button("↩ Desfazer (Original)", ImVec2(160, 32))) {
                        auto& pattern_notes = clip_manager.global_patterns[clip_manager.current_pattern_idx].getChannelNotes(ch_idx);
                        pattern_notes = backup_orig_notes;
                        ai_has_gen = false;
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Fechar", ImVec2(90, 32))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            // --- MODAL DA RIFF MACHINE ---
            if (open_riff_modal) {
                ImGui::OpenPopup("RiffMachineModal");
                open_riff_modal = false;
            }

            if (ImGui::BeginPopupModal("RiffMachineModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "🎹 FL STUDIO RIFF MACHINE & AUTO-ARPEGGIATOR");
                ImGui::TextDisabled("Gera automaticamente melodias, arpejos de psytrance, darksynth e hooks de trap");
                ImGui::Separator();
                ImGui::Spacing();

                static int rm_genre = 0;
                static int rm_pattern = 1;
                static int rm_octaves = 2;
                static float rm_gate = 0.85f;

                const char* rm_genres[] = { "👽 Psytrance Rolling Arp", "🚀 Synthwave 80s Darksynth", "🎹 Melodic Techno Motif", "🔥 Trap Melodic Pluck" };
                ImGui::Combo("Estilo de Riff", &rm_genre, rm_genres, IM_ARRAYSIZE(rm_genres));

                const char* rm_patterns[] = { "Linear / Reto", "Arp Up/Down Oitavado", "Salto de Terças", "Caos Rítmico" };
                ImGui::Combo("Movimento do Arpeggio", &rm_pattern, rm_patterns, IM_ARRAYSIZE(rm_patterns));

                ImGui::SliderInt("Extensão de Oitavas", &rm_octaves, 1, 3);
                ImGui::SliderFloat("Gate (Tamanho da Nota)", &rm_gate, 0.2f, 1.0f, "%.2f");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("⚡ GERAR RIFF NOVO", ImVec2(180, 32))) {
                    auto& pattern_notes = clip_manager.global_patterns[clip_manager.current_pattern_idx].getChannelNotes(ch_idx);
                    ApplyRiffMachine(pattern_notes, rm_genre, rm_pattern, rm_octaves, rm_gate, bpm, ch_idx);
                }
                ImGui::SameLine();
                if (ImGui::Button("Fechar", ImVec2(100, 32))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Separator();

        static int interacting_note_idx = -1; // Índice da nota sendo modificada
        static int interaction_mode = 0;      // 0=nenhum, 1=movendo, 2=redimensionando, 3=agulha, 4=painting, 5=erasing, 6=slicing, 7=selecting
        static float last_painted_time = -1.0f;
        static float slice_start_x = 0.0f;
        static ImVec2 select_start_pos;

        draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
        canvas_sz.y -= 135.0f; // Deixa 135px de espaço reservado para a faixa de 32 pílulas de velocity embaixo!
        canvas_sz.x -= 20.0f;  // Deixa espaço para a scrollbar vertical
        if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
        if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
        
        // Fundo do Canvas com contorno escuro sutil (moldura ciano unificada na janela externa)
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(12, 16, 23, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(25, 35, 48, 255), 4.0f, 0, 1.0f);
        
        // --- Controles de Navegação Estilo FL Studio (Mouse Wheel, Shift, Ctrl) ---
        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsWindowHovered()) {
            if (io.KeyCtrl && io.KeyShift && io.MouseWheel != 0.0f) {
                // Zoom Y (Altura das Teclas)
                zoom_y = (std::clamp)(zoom_y + io.MouseWheel * 2.0f, 12.0f, 60.0f);
            } else if (io.KeyCtrl && io.MouseWheel != 0.0f) {
                // Zoom X (Tempo)
                zoom_x = (std::clamp)(zoom_x + io.MouseWheel * 12.0f, 15.0f, 300.0f);
            } else if (io.KeyShift && io.MouseWheel != 0.0f) {
                // Scroll Horizontal no Tempo
                pan_x = (std::max)(0.0f, pan_x - io.MouseWheel * 60.0f);
            } else if (io.MouseWheel != 0.0f) {
                // Scroll Vertical nas Oitavas
                pan_y = (std::clamp)(pan_y - io.MouseWheel * 35.0f, 0.0f, 120.0f * zoom_y - canvas_sz.y);
            }
            // Pan com Botão do Meio
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                pan_x = (std::max)(0.0f, pan_x - io.MouseDelta.x);
                pan_y = (std::max)(0.0f, pan_y - io.MouseDelta.y);
            }

            // Atalhos de Edição no Piano Roll: Ctrl+Z, Ctrl+Y, Ctrl+C, Ctrl+X, Ctrl+V, Ctrl+A
            if (io.KeyCtrl) {
                if (ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
                    clip_manager.undo();
                } else if (ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
                    clip_manager.redo();
                } else if (ImGui::IsKeyPressed(ImGuiKey_C, false)) {
                    clip_manager.copySelection(ch_idx);
                } else if (ImGui::IsKeyPressed(ImGuiKey_X, false)) {
                    clip_manager.cutSelection(ch_idx);
                } else if (ImGui::IsKeyPressed(ImGuiKey_V, false)) {
                    float key_w = 90.0f;
                    float m_time = (io.MousePos.x - ((canvas_p0.x + key_w) - pan_x)) / zoom_x;
                    clip_manager.pasteClipboard(ch_idx, (std::max)(0.0f, m_time));
                } else if (ImGui::IsKeyPressed(ImGuiKey_A, false)) {
                    for (auto& n : clip_manager.getCurrentPattern().getChannelNotes(ch_idx)) {
                        n.is_selected = true;
                    }
                }
            }
        }

        float key_width = 90.0f;
        int num_keys = 120; // Expanded to 10 octaves
        int start_pitch = 0;
        
        float grid_start_x = canvas_p0.x + key_width;
        float beat_duration = 60.0f / bpm;
        float divisor = (current_snap_option == 0) ? 1.0f : (current_snap_option == 1) ? 2.0f : (current_snap_option == 2) ? 4.0f : 8.0f;
        float snap_step = beat_duration / divisor;

        // Prevenir pan_y além do número de chaves
        float total_height = num_keys * zoom_y;
        if (pan_y > total_height - canvas_sz.y) pan_y = total_height - canvas_sz.y;
        if (pan_y < 0.0f) pan_y = 0.0f;

        // --- Desenhar Grade de Fundo (Horizontal e Vertical) ---
        draw_list->PushClipRect(ImVec2(grid_start_x, canvas_p0.y), canvas_p1, true);
        
        // 1. Horizontal (Faixas das Teclas - Branco/Preto com Scale Highlighting Guia)
        for (int i = 0; i < num_keys; i++) {
            int pitch = start_pitch + (num_keys - 1 - i);
            float y = canvas_p0.y - pan_y + i * zoom_y;
            if (y + zoom_y < canvas_p0.y || y > canvas_p1.y) continue; // Culling
            
            int note_in_octave = pitch % 12;
            bool is_black = (note_in_octave == 1 || note_in_octave == 3 || note_in_octave == 6 || note_in_octave == 8 || note_in_octave == 10);
            bool in_scale = IsPitchInScale(pitch, current_root_note, current_scale_type);
            
            ImU32 bg_col;
            if (current_scale_type > 0 && in_scale) {
                bg_col = is_black ? IM_COL32(26, 42, 54, 255) : IM_COL32(38, 56, 72, 255); // Highlight Cyan Tint para notas da escala
            } else if (current_scale_type > 0 && !in_scale) {
                bg_col = is_black ? IM_COL32(12, 12, 15, 255) : IM_COL32(18, 18, 22, 255); // Tom escurecido fora da escala
            } else {
                bg_col = is_black ? IM_COL32(22, 22, 26, 255) : IM_COL32(32, 32, 38, 255);
            }
            draw_list->AddRectFilled(ImVec2(grid_start_x, y), ImVec2(canvas_p1.x, y + zoom_y), bg_col);
            draw_list->AddLine(ImVec2(grid_start_x, y + zoom_y), ImVec2(canvas_p1.x, y + zoom_y), IM_COL32(15, 15, 18, 255), 1.0f);
        }

        // 2. Vertical (Time Grid - Compassos e Batidas)
        float bar_duration = beat_duration * 4.0f;
        for (float t = 0.0f; (grid_start_x - pan_x) + t * zoom_x < canvas_p1.x + 1000.0f; t += snap_step) {
            float x = (grid_start_x - pan_x) + t * zoom_x;
            if (x < grid_start_x) continue;
            
            bool is_bar = std::fmod(t, bar_duration) < 0.01f;
            bool is_beat = std::fmod(t, beat_duration) < 0.01f;
            
            ImU32 col = is_bar ? IM_COL32(255,255,255,100) : (is_beat ? IM_COL32(255,255,255,50) : IM_COL32(255,255,255,15));
            float thickness = is_bar ? 2.0f : 1.0f;
            draw_list->AddLine(ImVec2(x, canvas_p0.y), ImVec2(x, canvas_p1.y), col, thickness);
        }
        draw_list->PopClipRect();

        // --- Área de Clique (Grid) ---
        ImGui::SetCursorScreenPos(ImVec2(grid_start_x, canvas_p0.y));
        ImGui::InvisibleButton("##pianoroll_grid", ImVec2(canvas_sz.x - key_width - 12.0f, canvas_sz.y - 12.0f));
        bool is_grid_hovered = ImGui::IsItemHovered();

        std::lock_guard<std::mutex> lock(timeline_mgr.timeline_mutex);
        auto& notes = clip_manager.global_patterns[clip_manager.current_pattern_idx].getChannelNotes(ch_idx);

        // --- Interação do Mouse com Notas (Estilo FL Studio) ---
        bool is_in_pr_sb = is_dragging_pr_h || is_dragging_pr_v;
        if (is_grid_hovered && !is_in_pr_sb) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            float time_sec = (mouse_pos.x - grid_start_x + pan_x) / zoom_x;
            float time_snapped = std::round(time_sec / snap_step) * snap_step;
            int row = (int)((mouse_pos.y - canvas_p0.y + pan_y) / zoom_y);
            int pitch = start_pitch + (num_keys - 1 - row);
            
            // Procurar se o mouse está sobre alguma nota
            int hovered_note_idx = -1;
            bool hovering_right_edge = false;
            
            for (int i = (int)notes.size() - 1; i >= 0; i--) {
                auto& n = notes[i];
                if (n.pitch == pitch && time_sec >= n.start_time && time_sec <= n.start_time + n.duration) {
                    hovered_note_idx = i;
                    // Verifica se está na borda direita (últimos 10 pixels ou 20% da nota)
                    float n_x1 = grid_start_x - pan_x + (n.start_time + n.duration) * zoom_x;
                    if (mouse_pos.x >= n_x1 - 8.0f && mouse_pos.x <= n_x1 + 4.0f) {
                        hovering_right_edge = true;
                    }
                    break;
                }
            }

            // Mouse Cursor Feedback
            if (hovering_right_edge) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

            // Left Click - Actions
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                float dy = ImGui::GetMousePos().y - canvas_p0.y;
                if (dy < 20.0f) { // Clicou na regua do tempo (topo)
                    float target_time = (mouse_pos.x - grid_start_x + pan_x) / zoom_x;
                    if (target_time < 0.0f) target_time = 0.0f;
                    *current_sample_ptr = (unsigned long long)(target_time * 44100.0f);
                    timeline_mgr.setMasterFrame(*current_sample_ptr);
                    interaction_mode = 3; // Movendo agulha
                } else {
                    if (current_tool == PianoRollTool::Draw || current_tool == PianoRollTool::Paint) {
                        if (hovered_note_idx != -1 && current_tool == PianoRollTool::Draw) {
                            interacting_note_idx = hovered_note_idx;
                            interaction_mode = hovering_right_edge ? 2 : 1; // 2=Resize, 1=Move
                            notes[hovered_note_idx].is_selected = true; // Select on click
                        } else {
                            // Adicionar nova nota (e acordes se selecionado)
                            KuroDSP::MidiNote note;
                            note.pitch = pitch;
                            note.start_time = time_snapped;
                            note.duration = snap_step;
                            note.velocity = 0.8f;
                            notes.push_back(note);
                            interacting_note_idx = (int)notes.size() - 1;
                            
                            // Adicionar notas extras para Acordes e Escalas Psytrance (Stamp)
                            if (current_stamp_idx == 1) { // Psytrance Minor (0, 3, 7, 12)
                                notes.push_back(KuroDSP::MidiNote(pitch + 3, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 12, time_snapped, snap_step));
                            } else if (current_stamp_idx == 2) { // Goa Phrygian Dominant (0, 1, 4, 7)
                                notes.push_back(KuroDSP::MidiNote(pitch + 1, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 4, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                            } else if (current_stamp_idx == 3) { // Harmonic Minor (0, 3, 7, 11)
                                notes.push_back(KuroDSP::MidiNote(pitch + 3, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 11, time_snapped, snap_step));
                            } else if (current_stamp_idx == 4) { // Pentatonic (0, 3, 5, 7, 10)
                                notes.push_back(KuroDSP::MidiNote(pitch + 3, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 5, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                            } else if (current_stamp_idx == 5) { // Major (0, 4, 7)
                                notes.push_back(KuroDSP::MidiNote(pitch + 4, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                            } else if (current_stamp_idx == 6) { // Minor (0, 3, 7)
                                notes.push_back(KuroDSP::MidiNote(pitch + 3, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                            } else if (current_stamp_idx == 7) { // 7th (0, 4, 7, 10)
                                notes.push_back(KuroDSP::MidiNote(pitch + 4, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 10, time_snapped, snap_step));
                            } else if (current_stamp_idx == 8) { // Maj7 (0, 4, 7, 11)
                                notes.push_back(KuroDSP::MidiNote(pitch + 4, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 11, time_snapped, snap_step));
                            } else if (current_stamp_idx == 9) { // Min7 (0, 3, 7, 10)
                                notes.push_back(KuroDSP::MidiNote(pitch + 3, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 7, time_snapped, snap_step));
                                notes.push_back(KuroDSP::MidiNote(pitch + 10, time_snapped, snap_step));
                            }
                            
                            interaction_mode = (current_tool == PianoRollTool::Paint) ? 4 : 2; 
                            last_painted_time = time_snapped;
                            g_piano_synth.triggerNote(pitch, 0.5f, 0.8f, ch_idx); // Preview no canal correto
                        }
                    } else if (current_tool == PianoRollTool::Erase) {
                        if (hovered_note_idx != -1) {
                            notes.erase(notes.begin() + hovered_note_idx);
                        }
                        interaction_mode = 5;
                    } else if (current_tool == PianoRollTool::Mute) {
                        if (hovered_note_idx != -1) {
                            notes[hovered_note_idx].is_muted = !notes[hovered_note_idx].is_muted;
                        }
                    } else if (current_tool == PianoRollTool::Slice) {
                        interaction_mode = 6;
                        slice_start_x = mouse_pos.x;
                    } else if (current_tool == PianoRollTool::Select) {
                        interaction_mode = 7;
                        select_start_pos = mouse_pos;
                        // Deselect all se clicar fora
                        if (hovered_note_idx == -1) {
                            for (auto& n : notes) n.is_selected = false;
                        }
                    }
                }
            }
            
            // Right Click - Delete global
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                for (auto it = notes.begin(); it != notes.end(); ) {
                    if (it->pitch == pitch && time_sec >= it->start_time && time_sec <= it->start_time + it->duration) {
                        it = notes.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }

        // Dragging Logic
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            float time_sec = (mouse_pos.x - grid_start_x + pan_x) / zoom_x;
            float time_snapped = std::round(time_sec / snap_step) * snap_step;
            int row = (int)((mouse_pos.y - canvas_p0.y + pan_y) / zoom_y);
            int pitch = start_pitch + (num_keys - 1 - row);
            
            if (interaction_mode == 3) {
                float target_time = (mouse_pos.x - grid_start_x + pan_x) / zoom_x;
                if (target_time < 0.0f) target_time = 0.0f;
                *current_sample_ptr = (unsigned long long)(target_time * 44100.0f);
                timeline_mgr.setMasterFrame(*current_sample_ptr);
            } 
            else if (interaction_mode == 1 || interaction_mode == 2) {
                if (interacting_note_idx != -1 && interacting_note_idx < notes.size()) {
                    auto& n = notes[interacting_note_idx];
                    if (interaction_mode == 1) { // Move
                        n.pitch = pitch;
                        n.start_time = time_snapped;
                        if (n.start_time < 0.0f) n.start_time = 0.0f;
                    } else if (interaction_mode == 2) { // Resize
                        float new_duration = time_snapped - n.start_time;
                        if (new_duration < snap_step) new_duration = snap_step;
                        n.duration = new_duration;
                    }
                }
            } else if (interaction_mode == 4) { // Painting
                if (time_snapped != last_painted_time) {
                    KuroDSP::MidiNote note;
                    note.pitch = pitch;
                    note.start_time = time_snapped;
                    note.duration = snap_step;
                    note.velocity = 0.8f;
                    notes.push_back(note);
                    last_painted_time = time_snapped;
                    g_piano_synth.triggerNote(pitch, 0.5f, 0.8f, ch_idx);
                }
            } else if (interaction_mode == 5) { // Erasing
                for (auto it = notes.begin(); it != notes.end(); ) {
                    if (it->pitch == pitch && time_sec >= it->start_time && time_sec <= it->start_time + it->duration) {
                        it = notes.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
        
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (interaction_mode == 6) { // Finaliza o Slice
                float slice_time = (ImGui::GetMousePos().x - grid_start_x + pan_x) / zoom_x;
                float slice_snapped = std::round(slice_time / snap_step) * snap_step;
                
                std::vector<KuroDSP::MidiNote> new_notes;
                for (auto& n : notes) {
                    if (slice_snapped > n.start_time && slice_snapped < n.start_time + n.duration) {
                        float old_dur = n.duration;
                        n.duration = slice_snapped - n.start_time; // Corta a nota original
                        
                        KuroDSP::MidiNote split_note = n; // Cria a metade direita
                        split_note.start_time = slice_snapped;
                        split_note.duration = old_dur - n.duration;
                        new_notes.push_back(split_note);
                    }
                }
                for (const auto& nn : new_notes) notes.push_back(nn);
            } else if (interaction_mode == 7) { // Finaliza a Seleção
                ImVec2 p0 = ImVec2(std::min(select_start_pos.x, ImGui::GetMousePos().x), std::min(select_start_pos.y, ImGui::GetMousePos().y));
                ImVec2 p1 = ImVec2(std::max(select_start_pos.x, ImGui::GetMousePos().x), std::max(select_start_pos.y, ImGui::GetMousePos().y));
                
                for (auto& n : notes) {
                    float n_x = grid_start_x - pan_x + n.start_time * zoom_x;
                    float n_y = canvas_p0.y - pan_y + ((num_keys - 1) - (n.pitch - start_pitch)) * zoom_y;
                    
                    if (n_x >= p0.x && n_x <= p1.x && n_y >= p0.y && n_y <= p1.y) {
                        n.is_selected = true;
                    }
                }
            }
            interacting_note_idx = -1;
            interaction_mode = 0;
        }

        // --- Desenhar Notas Ativas ---
        draw_list->PushClipRect(ImVec2(grid_start_x, canvas_p0.y), canvas_p1, true);
        
        if (ghost_notes) {
            for (const auto& gnote : *ghost_notes) {
                int row = (num_keys - 1) - (gnote.pitch - start_pitch);
                float y0 = canvas_p0.y - pan_y + row * zoom_y;
                float x0 = grid_start_x - pan_x + gnote.start_time * zoom_x;
                float x1 = x0 + gnote.duration * zoom_x;
                
                if (y0 + zoom_y > canvas_p0.y && y0 < canvas_p1.y && x1 > grid_start_x && x0 < canvas_p1.x) {
                    draw_list->AddRectFilled(ImVec2(x0, y0 + 1), ImVec2(x1 - 1, y0 + zoom_y - 1), IM_COL32(100, 100, 100, 100), 2.0f);
                }
            }
        }

        // Render FL Studio Ghost Notes from other channels of active pattern
        if (g_piano_roll_show_ghost_notes) {
            auto& current_pat = clip_manager.global_patterns[clip_manager.current_pattern_idx];
            for (int c = 0; c < 8; c++) {
                if (c == ch_idx) continue;
                const auto& ch_notes = current_pat.getChannelNotes(c);
                if (ch_notes.empty()) continue;

                // Cores discretas para identificação visual dos canais
                ImU32 ghost_fill = (c == 0) ? IM_COL32(155, 85, 75, 90)    // Kick (terracota suave)
                                 : (c == 1) ? IM_COL32(50, 125, 165, 90)   // Bass (azul ardósia)
                                 : (c == 2) ? IM_COL32(140, 140, 65, 90)   // Snare (amarelo suave)
                                            : IM_COL32(110, 120, 135, 80);  // Outros

                ImU32 ghost_border = (c == 0) ? IM_COL32(195, 110, 100, 160)
                                   : (c == 1) ? IM_COL32(70, 165, 205, 160)
                                   : (c == 2) ? IM_COL32(185, 185, 90, 160)
                                              : IM_COL32(135, 145, 160, 140);

                const char* ch_tag = (c == 0) ? "Kick" : (c == 1) ? "Bass" : (c == 2) ? "Snare" : "Ch";

                for (const auto& gnote : ch_notes) {
                    int row = (num_keys - 1) - (gnote.pitch - start_pitch);
                    float y0 = canvas_p0.y - pan_y + row * zoom_y;
                    float x0 = grid_start_x - pan_x + gnote.start_time * zoom_x;
                    float x1 = x0 + gnote.duration * zoom_x;
                    
                    if (y0 + zoom_y > canvas_p0.y && y0 < canvas_p1.y && x1 > grid_start_x && x0 < canvas_p1.x) {
                        draw_list->AddRectFilled(ImVec2(x0, y0 + 1), ImVec2(x1 - 1, y0 + zoom_y - 1), ghost_fill, 3.0f);
                        draw_list->AddRect(ImVec2(x0, y0 + 1), ImVec2(x1 - 1, y0 + zoom_y - 1), ghost_border, 3.0f, 0, 1.0f);

                        // Imprime tag do canal quando há espaço suficiente na nota
                        if (x1 - x0 > 24.0f && zoom_y >= 12.0f) {
                            draw_list->AddText(ImVec2(x0 + 3.0f, y0 + 1.0f), ghost_border, ch_tag);
                        }
                    }
                }
            }
        }

        // Cálculo Preciso do Playhead e Tempo de Loop do Pattern (Sincronizado)
        float pr_beat_len = 60.0f / (bpm > 20.0f ? bpm : 140.0f);
        float pr_max_note_t = 0.0f;
        for (const auto& n : notes) {
            if (n.start_time + n.duration > pr_max_note_t) pr_max_note_t = n.start_time + n.duration;
        }
        float pr_total_beats = std::ceil(pr_max_note_t / pr_beat_len);
        if (pr_total_beats < 4.0f) pr_total_beats = 4.0f; // Mínimo 1 compasso
        float pr_loop_beats = std::ceil(pr_total_beats / 4.0f) * 4.0f;
        float pr_loop_len = pr_loop_beats * pr_beat_len;

        float raw_playhead_t = (float)(*current_sample_ptr) / 44100.0f;
        float playhead_time = is_playing ? fmodf(raw_playhead_t, pr_loop_len) : 0.0f;
        float playhead_x = grid_start_x - pan_x + playhead_time * zoom_x;

        for (auto& note : notes) {
            int row = (num_keys - 1) - (note.pitch - start_pitch);
            float y0 = canvas_p0.y - pan_y + row * zoom_y;
            float x0 = grid_start_x - pan_x + note.start_time * zoom_x;
            float x1 = x0 + note.duration * zoom_x;
            float y1 = y0 + zoom_y;
            
            if (y1 > canvas_p0.y && y0 < canvas_p1.y && x1 > grid_start_x && x0 < canvas_p1.x) {
                // Paleta de Cores Reimaginada (Ciano Elétrico #00E5FF & Magenta Neon #FF007F)
                bool is_magenta = (note.pitch >= 57 && note.pitch <= 64 && note.start_time >= 1.2f);
                bool is_note_active = is_playing && (playhead_time >= note.start_time && playhead_time < note.start_time + (note.duration > 0.05f ? note.duration : 0.15f));
                ImU32 color = is_note_active ? IM_COL32(57, 255, 140, 255) : (is_magenta ? IM_COL32(255, 0, 127, 230) : IM_COL32(0, 229, 255, 230));
                if (note.is_muted) color = IM_COL32(80, 90, 100, 150);
                ImU32 border_col = (is_note_active || note.is_selected) ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 255, 255, 220);
                float border_thickness = (is_note_active || note.is_selected) ? 2.5f : 1.5f;
                
                // Bloco Neon com Textura PNG Sprite e cantos arredondados
                GLuint note_tex = is_magenta ? g_piano_sprites.neon_note_purple_tex : g_piano_sprites.neon_note_cyan_tex;
                if (note_tex && !is_note_active && !note.is_muted) {
                    draw_list->AddImage((ImTextureID)(intptr_t)note_tex, ImVec2(x0, y0 + 2.0f), ImVec2(x1 - 1.0f, y1 - 2.0f));
                } else {
                    draw_list->AddRectFilled(ImVec2(x0, y0 + 2.0f), ImVec2(x1 - 1.0f, y1 - 2.0f), color, 4.0f);
                }
                draw_list->AddRect(ImVec2(x0, y0 + 2.0f), ImVec2(x1 - 1.0f, y1 - 2.0f), border_col, 4.0f, 0, border_thickness);
                
                // Brilho de Relevo Glassmorphism 3D no topo da nota
                draw_list->AddLine(ImVec2(x0 + 4.0f, y0 + 3.5f), ImVec2(x1 - 4.0f, y0 + 3.5f), IM_COL32(255, 255, 255, 180), 1.2f);

                // Nome da Nota/Oitava impresso dentro do bloco
                if (x1 - x0 > 20.0f && zoom_y >= 12.0f) {
                    const char* note_names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
                    int note_in_octave = note.pitch % 12;
                    int octave = (note.pitch / 12) - 1;
                    char nbuf[16];
                    snprintf(nbuf, sizeof(nbuf), "%s%d", note_names[note_in_octave], octave);
                    draw_list->AddText(ImVec2(x0 + 6.0f, y0 + (zoom_y - 14.0f) * 0.5f), IM_COL32(255, 255, 255, 255), nbuf);
                }
            }
        }
        
        // Desenha ferramentas visuais ativas por cima das notas
        if (interaction_mode == 6) { // Slice Tool Line
            ImVec2 m_pos = ImGui::GetMousePos();
            draw_list->AddLine(ImVec2(m_pos.x, canvas_p0.y), ImVec2(m_pos.x, canvas_p1.y), IM_COL32(255, 50, 50, 200), 2.0f);
        } else if (interaction_mode == 7) { // Marquee Selection
            ImVec2 m_pos = ImGui::GetMousePos();
            ImVec2 p0 = ImVec2(std::min(select_start_pos.x, m_pos.x), std::min(select_start_pos.y, m_pos.y));
            ImVec2 p1 = ImVec2(std::max(select_start_pos.x, m_pos.x), std::max(select_start_pos.y, m_pos.y));
            draw_list->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, 30));
            draw_list->AddRect(p0, p1, IM_COL32(255, 255, 255, 150), 0.0f, 0, 1.0f);
        }

        // --- Régua do Tempo, Marcadores de Estrutura e Agulha (Playhead) ---
        float pr_ruler_h = 26.0f;
        draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p1.x, canvas_p0.y + pr_ruler_h), IM_COL32(22, 28, 38, 255));
        draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + pr_ruler_h), ImVec2(canvas_p1.x, canvas_p0.y + pr_ruler_h), IM_COL32(35, 50, 70, 255), 1.5f);
        
        // Desenhar os ticks da régua do Piano Roll e Marcadores Estruturais
        draw_list->PushClipRect(ImVec2(grid_start_x, canvas_p0.y), ImVec2(canvas_p1.x, canvas_p0.y + pr_ruler_h), true);
        
        float ruler_bar_duration = beat_duration * 4.0f;
        for (float t = 0.0f; (grid_start_x - pan_x) + t * zoom_x < canvas_p1.x + 1000.0f; t += beat_duration) {
            float tx = grid_start_x - pan_x + t * zoom_x;
            if (tx < grid_start_x) continue;
            
            bool is_bar = std::fmod(t, ruler_bar_duration) < 0.01f;
            if (is_bar) {
                // Major tick: Compasso (Bar)
                draw_list->AddLine(ImVec2(tx, canvas_p0.y + 6.0f), ImVec2(tx, canvas_p0.y + pr_ruler_h), IM_COL32(200, 220, 240, 255), 1.5f);
                char label[16];
                int bar_num = (int)(std::round(t / ruler_bar_duration)) + 1;
                snprintf(label, sizeof(label), "%d", bar_num);
                draw_list->AddText(ImVec2(tx + 4, canvas_p0.y + 4), IM_COL32(180, 210, 235, 255), label);
            } else {
                // Minor tick: Batida (Beat)
                draw_list->AddLine(ImVec2(tx, canvas_p0.y + 16.0f), ImVec2(tx, canvas_p0.y + pr_ruler_h), IM_COL32(80, 100, 125, 255), 1.0f);
            }
        }

        // Marcadores de Estrutura da Música Dinâmicos e Arrastáveis (FL Studio Time Markers)
        struct DynamicSongMarker { float bar; std::string name; ImU32 col; };
        static std::vector<DynamicSongMarker> s_dyn_markers = {
            { 1.0f, "INTRO", IM_COL32(255, 215, 0, 220) },
            { 17.0f, "BUILD-UP", IM_COL32(0, 200, 255, 220) },
            { 33.0f, "DROP 1", IM_COL32(180, 0, 255, 220) },
            { 65.0f, "BREAKDOWN", IM_COL32(255, 140, 0, 220) },
            { 97.0f, "MAIN DROP", IM_COL32(255, 0, 128, 220) },
            { 129.0f, "OUTRO", IM_COL32(0, 229, 255, 220) }
        };

        static int dragging_marker_idx = -1;
        static int edit_marker_idx = -1;
        static char marker_name_buf[64] = "";

        for (size_t m_idx = 0; m_idx < s_dyn_markers.size(); ++m_idx) {
            auto& st = s_dyn_markers[m_idx];
            float st_time = (st.bar - 1.0f) * ruler_bar_duration;
            float st_x = grid_start_x - pan_x + st_time * zoom_x;
            
            if (st_x >= grid_start_x - 120.0f && st_x < canvas_p1.x) {
                ImVec2 tag_p0(st_x + 10.0f, canvas_p0.y + 3.0f);
                ImVec2 tag_p1(st_x + 95.0f, canvas_p0.y + 22.0f);
                bool hovered_tag = ImGui::IsMouseHoveringRect(tag_p0, tag_p1);

                // Arrasto do marcador para ajustar ao tempo exato da música
                if (hovered_tag && ImGui::IsMouseClicked(0)) {
                    dragging_marker_idx = (int)m_idx;
                }
                if (dragging_marker_idx == (int)m_idx) {
                    if (ImGui::IsMouseDown(0)) {
                        float rel_mouse_x = ImGui::GetIO().MousePos.x - (grid_start_x - pan_x);
                        float new_time = (std::max)(0.0f, rel_mouse_x / zoom_x);
                        st.bar = (std::max)(1.0f, (std::round)(new_time / ruler_bar_duration) + 1.0f);
                    } else {
                        dragging_marker_idx = -1;
                    }
                }

                // Menu de edição com clique direito ou duplo-clique
                if (hovered_tag && (ImGui::IsMouseClicked(1) || ImGui::IsMouseDoubleClicked(0))) {
                    edit_marker_idx = (int)m_idx;
                    strncpy(marker_name_buf, st.name.c_str(), sizeof(marker_name_buf) - 1);
                    ImGui::OpenPopup("##EditMarkerPopup");
                }

                draw_list->AddRectFilled(tag_p0, tag_p1, st.col, 3.0f);
                draw_list->AddText(ImVec2(st_x + 14.0f, canvas_p0.y + 4.0f), IM_COL32(0, 0, 0, 255), st.name.c_str());
                // Linha vertical guia na grade
                draw_list->AddLine(ImVec2(st_x, canvas_p0.y + pr_ruler_h), ImVec2(st_x, canvas_p1.y), st.col, 1.0f);
            }
        }

        if (ImGui::BeginPopup("##EditMarkerPopup")) {
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "🏷️ Editar Marcador de Seção");
            ImGui::Separator();
            ImGui::SetNextItemWidth(180);
            if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##marker_name_inp", marker_name_buf, sizeof(marker_name_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (edit_marker_idx >= 0 && edit_marker_idx < (int)s_dyn_markers.size()) {
                    s_dyn_markers[edit_marker_idx].name = marker_name_buf;
                }
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Salvar", ImVec2(80, 22))) {
                if (edit_marker_idx >= 0 && edit_marker_idx < (int)s_dyn_markers.size()) {
                    s_dyn_markers[edit_marker_idx].name = marker_name_buf;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Excluir", ImVec2(80, 22))) {
                if (edit_marker_idx >= 0 && edit_marker_idx < (int)s_dyn_markers.size()) {
                    s_dyn_markers.erase(s_dyn_markers.begin() + edit_marker_idx);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        draw_list->PopClipRect();
        
        // Cabeçalho da Régua (canto superior esquerdo fixo acima do teclado)
        draw_list->AddRectFilled(canvas_p0, ImVec2(grid_start_x, canvas_p0.y + pr_ruler_h), IM_COL32(18, 24, 34, 255));
        draw_list->AddLine(ImVec2(grid_start_x, canvas_p0.y), ImVec2(grid_start_x, canvas_p0.y + pr_ruler_h), IM_COL32(35, 50, 70, 255), 2.0f);
        draw_list->AddText(ImVec2(canvas_p0.x + 8.0f, canvas_p0.y + 6.0f), IM_COL32(0, 229, 255, 255), "🎹 PIANO ROLL");

        // Laser Playhead ciano brilhante com triângulo marcador sincronizado com o loop
        draw_list->PushClipRect(ImVec2(grid_start_x, canvas_p0.y), canvas_p1, true);
        if (playhead_x >= grid_start_x && playhead_x < canvas_p1.x) {
            draw_list->AddLine(ImVec2(playhead_x, canvas_p0.y), ImVec2(playhead_x, canvas_p1.y), IM_COL32(0, 255, 255, 255), 2.0f);
            draw_list->AddTriangleFilled(ImVec2(playhead_x - 7, canvas_p0.y), ImVec2(playhead_x + 7, canvas_p0.y), ImVec2(playhead_x, canvas_p0.y + 12), IM_COL32(0, 255, 255, 255));
        }

        // Scrollbars Interativas do Piano Roll
        ImVec2 pr_mouse = ImGui::GetIO().MousePos;

        // 1. Horizontal Scrollbar
        float pr_total_w = 400.0f * zoom_x; // 400 segundos de timeline
        float pr_view_w = canvas_sz.x - key_width;
        float pr_max_scroll_x = (std::max)(0.0f, pr_total_w - pr_view_w);
        if (pr_max_scroll_x > 0.0f) {
            float h_x0 = grid_start_x + 2.0f;
            float h_x1 = canvas_p1.x - 12.0f;
            float h_y0 = canvas_p1.y - 8.0f;
            float h_y1 = canvas_p1.y - 1.0f;
            float h_w = h_x1 - h_x0;
            float h_thumb_w = (std::clamp)(h_w * (pr_view_w / pr_total_w), 30.0f, h_w);
            float h_thumb_x0 = h_x0 + (pan_x / pr_max_scroll_x) * (h_w - h_thumb_w);
            
            bool h_hovered = ImGui::IsMouseHoveringRect(ImVec2(h_x0, h_y0), ImVec2(h_x1, h_y1));
            if (h_hovered && ImGui::IsMouseClicked(0)) is_dragging_pr_h = true;
            if (is_dragging_pr_h) {
                if (ImGui::IsMouseDown(0)) {
                    float norm = (std::clamp)((pr_mouse.x - h_x0 - h_thumb_w * 0.5f) / (h_w - h_thumb_w), 0.0f, 1.0f);
                    pan_x = norm * pr_max_scroll_x;
                } else {
                    is_dragging_pr_h = false;
                }
            }
            draw_list->AddRectFilled(ImVec2(h_x0, h_y0), ImVec2(h_x1, h_y1), IM_COL32(10, 16, 24, 200), 3.0f);
            draw_list->AddRectFilled(ImVec2(h_thumb_x0, h_y0), ImVec2(h_thumb_x0 + h_thumb_w, h_y1), is_dragging_pr_h ? IM_COL32(0, 255, 255, 255) : (h_hovered ? IM_COL32(0, 229, 255, 230) : IM_COL32(0, 180, 220, 190)), 3.0f);
        }

        // 2. Vertical Scrollbar
        float pr_total_h = 120.0f * zoom_y;
        float pr_view_h = canvas_sz.y;
        float pr_max_scroll_y = (std::max)(0.0f, pr_total_h - pr_view_h);
        if (pr_max_scroll_y > 0.0f) {
            float v_x0 = canvas_p1.x - 8.0f;
            float v_x1 = canvas_p1.x - 1.0f;
            float v_y0 = canvas_p0.y + pr_ruler_h + 2.0f;
            float v_y1 = canvas_p1.y - 10.0f;
            float v_h = v_y1 - v_y0;
            float v_thumb_h = (std::clamp)(v_h * (pr_view_h / pr_total_h), 24.0f, v_h);
            float v_thumb_y0 = v_y0 + (pan_y / pr_max_scroll_y) * (v_h - v_thumb_h);
            
            bool v_hovered = ImGui::IsMouseHoveringRect(ImVec2(v_x0, v_y0), ImVec2(v_x1, v_y1));
            if (v_hovered && ImGui::IsMouseClicked(0)) is_dragging_pr_v = true;
            if (is_dragging_pr_v) {
                if (ImGui::IsMouseDown(0)) {
                    float norm = (std::clamp)((pr_mouse.y - v_y0 - v_thumb_h * 0.5f) / (v_h - v_thumb_h), 0.0f, 1.0f);
                    pan_y = norm * pr_max_scroll_y;
                } else {
                    is_dragging_pr_v = false;
                }
            }
            draw_list->AddRectFilled(ImVec2(v_x0, v_y0), ImVec2(v_x1, v_y1), IM_COL32(10, 16, 24, 200), 3.0f);
            draw_list->AddRectFilled(ImVec2(v_x0, v_thumb_y0), ImVec2(v_x1, v_thumb_y0 + v_thumb_h), is_dragging_pr_v ? IM_COL32(0, 255, 255, 255) : (v_hovered ? IM_COL32(0, 229, 255, 230) : IM_COL32(0, 180, 220, 190)), 3.0f);
        }

        draw_list->PopClipRect();
        
        draw_list->PopClipRect(); // Pop of main canvas clipping

        g_piano_sprites.Init();

        // --- Desenhar o Teclado (Fica Fixo na Esquerda, mas scrolla Verticalmente) ---
        draw_list->PushClipRect(canvas_p0, ImVec2(canvas_p0.x + key_width, canvas_p1.y), true);
        for (int i = 0; i < num_keys; i++) {
            int pitch = start_pitch + (num_keys - 1 - i);
            float y = canvas_p0.y - pan_y + i * zoom_y;
            
            if (y + zoom_y < canvas_p0.y || y > canvas_p1.y) continue; // Culling
            
            int note_in_octave = pitch % 12;
            bool is_black = (note_in_octave == 1 || note_in_octave == 3 || note_in_octave == 6 || note_in_octave == 8 || note_in_octave == 10);
            
            // Desenho com Texturas PNG em HD das Teclas 3D
            if (is_black) {
                // Tecla branca ao fundo
                if (g_piano_sprites.white_key_tex) {
                    draw_list->AddImage((ImTextureID)(intptr_t)g_piano_sprites.white_key_tex, ImVec2(canvas_p0.x + key_width * 0.62f, y), ImVec2(canvas_p0.x + key_width, y + zoom_y));
                }
                // Tecla preta com textura Obsidian 3D
                GLuint b_tex = g_piano_sprites.black_key_tex;
                draw_list->AddImage((ImTextureID)(intptr_t)b_tex, ImVec2(canvas_p0.x, y), ImVec2(canvas_p0.x + key_width * 0.62f, y + zoom_y - 1.0f));
            } else {
                // Tecla branca completa com textura Marfim 3D
                GLuint w_tex = g_piano_sprites.white_key_tex;
                draw_list->AddImage((ImTextureID)(intptr_t)w_tex, ImVec2(canvas_p0.x, y), ImVec2(canvas_p0.x + key_width, y + zoom_y));
            }

            // Animação de Tecla Pressionada / Iluminada ao Tocar
            bool key_is_active = false;
            if (is_playing) {
                for (const auto& n : notes) {
                    if (n.pitch == pitch && playhead_time >= n.start_time && playhead_time < n.start_time + (n.duration > 0.05f ? n.duration : 0.15f)) {
                        key_is_active = true;
                        break;
                    }
                }
            }
            if (key_is_active) {
                draw_list->AddRectFilled(ImVec2(canvas_p0.x + 2, y + 1), ImVec2(canvas_p0.x + key_width - 2, y + zoom_y - 1), IM_COL32(0, 229, 255, 170), 3.0f);
                draw_list->AddRect(ImVec2(canvas_p0.x + 2, y + 1), ImVec2(canvas_p0.x + key_width - 2, y + zoom_y - 1), IM_COL32(255, 255, 255, 240), 3.0f, 0, 1.5f);
            }
            
            // Nomes das notas e oitavas
            if (zoom_y > 12.0f) {
                const char* note_names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
                int octave = (pitch / 12) - 1;
                char buf[16];
                snprintf(buf, sizeof(buf), "%s%d", note_names[note_in_octave], octave);
                
                ImU32 text_color = is_black ? IM_COL32(180, 180, 180, 255) : IM_COL32(40, 40, 45, 255);
                
                if (is_black) {
                    draw_list->AddText(ImVec2(canvas_p0.x + 5, y + (zoom_y - 15.0f) * 0.5f), text_color, buf);
                } else {
                    if (note_in_octave == 0) {
                        draw_list->AddText(ImVec2(canvas_p0.x + key_width - 32, y + (zoom_y - 15.0f) * 0.5f), IM_COL32(230, 100, 20, 255), buf);
                    } else {
                        draw_list->AddText(ImVec2(canvas_p0.x + key_width - 25, y + (zoom_y - 15.0f) * 0.5f), text_color, buf);
                    }
                }
            }
            
            // Grade horizontal estendendo para a direita
            ImU32 line_col = is_black ? IM_COL32(255,255,255,10) : IM_COL32(255,255,255,20);
            if (note_in_octave == 0) line_col = IM_COL32(255,255,255,40); // Highlight C
            draw_list->AddLine(ImVec2(grid_start_x, y), ImVec2(canvas_p1.x, y), line_col);
            
            // Área clicável do teclado (registro básico no ImGui)
            ImGui::SetCursorScreenPos(ImVec2(canvas_p0.x, y));
            ImGui::PushID(pitch);
            ImGui::InvisibleButton("##key", ImVec2(key_width, zoom_y));
            ImGui::PopID();
        }
        draw_list->PopClipRect();

        // --- Lógica de Glissando (Slide) no Teclado do Piano ---
        static int last_slide_pitch = -1;
        static bool slide_started_on_keyboard = false;
        
        bool mouse_in_keyboard_x = (io.MousePos.x >= canvas_p0.x && io.MousePos.x < canvas_p0.x + key_width);
        bool mouse_in_keyboard_y = (io.MousePos.y >= canvas_p0.y && io.MousePos.y < canvas_p1.y);
        
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (mouse_in_keyboard_x && mouse_in_keyboard_y) {
                slide_started_on_keyboard = true;
            }
        }
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            slide_started_on_keyboard = false;
        }
        
        if (slide_started_on_keyboard && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            if (mouse_in_keyboard_x && mouse_in_keyboard_y) {
                float y_rel = io.MousePos.y - (canvas_p0.y - pan_y);
                int hovered_i = (int)(y_rel / zoom_y);
                if (hovered_i >= 0 && hovered_i < num_keys) {
                    int hovered_pitch = start_pitch + (num_keys - 1 - hovered_i);
                    if (hovered_pitch != last_slide_pitch) {
                        if (last_slide_pitch != -1) {
                            g_piano_synth.releaseNote(last_slide_pitch);
                        }
                        g_piano_synth.triggerNote(hovered_pitch, 1.5f, 0.8f, ch_idx);
                        last_slide_pitch = hovered_pitch;
                    }
                }
            } else {
                if (last_slide_pitch != -1) {
                    g_piano_synth.releaseNote(last_slide_pitch);
                    last_slide_pitch = -1;
                }
            }
        } else {
            if (last_slide_pitch != -1) {
                g_piano_synth.releaseNote(last_slide_pitch);
                last_slide_pitch = -1;
            }
        }

        // Linha Divisória Vertical de LED Ciano Neon entre o Teclado e o Grid (Imagem 2)
        draw_list->AddLine(ImVec2(grid_start_x, canvas_p0.y), ImVec2(grid_start_x, canvas_p1.y), IM_COL32(0, 229, 255, 255), 2.5f);

        // --- PAINEL INFERIOR MULTI-PROPRIEDADE (VELOCITY, PAN, PITCH OFFSET) ---
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.08f, 0.11f, 1.0f));
        ImGui::BeginChild("##BottomMultiPropLane", ImVec2(0, 125), true);
        {
            ImDrawList* v_draw = ImGui::GetWindowDrawList();
            ImVec2 v_p0 = ImGui::GetCursorScreenPos();
            float v_w = ImGui::GetContentRegionAvail().x;
            
            // Radios de alternância do rodapé
            ImGui::SetCursorScreenPos(ImVec2(v_p0.x + 10, v_p0.y + 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 2));
            if (ImGui::RadioButton("📊 Velocity", bottom_lane_mode == 0)) bottom_lane_mode = 0; ImGui::SameLine();
            if (ImGui::RadioButton("🎧 Pan", bottom_lane_mode == 1)) bottom_lane_mode = 1; ImGui::SameLine();
            if (ImGui::RadioButton("🎵 Pitch Offset", bottom_lane_mode == 2)) bottom_lane_mode = 2;
            ImGui::PopStyleVar();

            auto& ch_notes = clip_manager.global_patterns[clip_manager.current_pattern_idx].getChannelNotes(ch_idx);

            int display_count = std::max(32, (int)ch_notes.size());
            float pill_w = (v_w - 40.0f) / (float)display_count;
            if (pill_w < 12.0f) pill_w = 12.0f;

            for (int i = 0; i < display_count; i++) {
                float px = v_p0.x + 10.0f + i * pill_w;
                float py_top = v_p0.y + 30.0f;
                float py_bot = v_p0.y + 98.0f;
                float py_mid = (py_top + py_bot) * 0.5f;

                // Fundo da Pílula
                v_draw->AddRectFilled(ImVec2(px + 2, py_top), ImVec2(px + pill_w - 2, py_bot), IM_COL32(20, 26, 35, 255), 6.0f);
                v_draw->AddRect(ImVec2(px + 2, py_top), ImVec2(px + pill_w - 2, py_bot), IM_COL32(50, 65, 80, 255), 6.0f);

                bool is_step_active = false;
                if (i < (int)ch_notes.size()) {
                    auto& n = ch_notes[i];
                    is_step_active = is_playing && (playhead_time >= n.start_time && playhead_time < n.start_time + (n.duration > 0.05f ? n.duration : 0.15f));
                }
                if (is_step_active) {
                    v_draw->AddRectFilled(ImVec2(px + 2, py_top), ImVec2(px + pill_w - 2, py_bot), IM_COL32(255, 255, 255, 45), 6.0f);
                    v_draw->AddRect(ImVec2(px + 1, py_top - 1), ImVec2(px + pill_w - 1, py_bot + 1), IM_COL32(57, 255, 140, 255), 6.0f, 0, 2.0f);
                }

                if (i < (int)ch_notes.size()) {
                    auto& n = ch_notes[i];
                    ImVec2 mouse_pos = ImGui::GetMousePos();
                    bool hovered = (mouse_pos.x >= px + 2 && mouse_pos.x <= px + pill_w - 2 && mouse_pos.y >= py_top && mouse_pos.y <= py_bot);
                    
                    // Interatividade: Drag do mouse altera propriedade diretamente!
                    if (hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                        float norm_y = std::clamp(1.0f - (mouse_pos.y - py_top) / (py_bot - py_top), 0.0f, 1.0f);
                        if (bottom_lane_mode == 0) {
                            n.velocity = norm_y;
                        } else if (bottom_lane_mode == 1) {
                            n.pan = norm_y;
                        } else if (bottom_lane_mode == 2) {
                            n.pitch_offset = (norm_y - 0.5f) * 24.0f; // -12 a +12 semitons
                        }
                    }

                    if (bottom_lane_mode == 0) { // Velocity
                        float fill_h = (py_bot - py_top) * std::clamp(n.velocity, 0.0f, 1.0f);
                        ImU32 p_col = n.is_selected ? IM_COL32(255, 200, 0, 240) : (i % 2 == 0 ? IM_COL32(0, 229, 255, 220) : IM_COL32(255, 0, 127, 220));
                        v_draw->AddRectFilled(ImVec2(px + 3, py_bot - fill_h), ImVec2(px + pill_w - 3, py_bot - 2), p_col, 5.0f);
                    } else if (bottom_lane_mode == 1) { // Pan
                        v_draw->AddLine(ImVec2(px + 2, py_mid), ImVec2(px + pill_w - 2, py_mid), IM_COL32(100, 120, 140, 255), 1.0f);
                        float pan_y = py_mid - (n.pan - 0.5f) * (py_bot - py_top) * 0.9f;
                        ImU32 pan_col = (n.pan < 0.48f) ? IM_COL32(0, 255, 150, 220) : ((n.pan > 0.52f) ? IM_COL32(255, 180, 0, 220) : IM_COL32(200, 200, 200, 255));
                        v_draw->AddLine(ImVec2(px + pill_w * 0.5f, py_mid), ImVec2(px + pill_w * 0.5f, pan_y), pan_col, 3.0f);
                        v_draw->AddCircleFilled(ImVec2(px + pill_w * 0.5f, pan_y), 3.5f, pan_col);
                    } else if (bottom_lane_mode == 2) { // Pitch Offset
                        v_draw->AddLine(ImVec2(px + 2, py_mid), ImVec2(px + pill_w - 2, py_mid), IM_COL32(100, 120, 140, 255), 1.0f);
                        float poff_norm = std::clamp(n.pitch_offset / 24.0f, -0.5f, 0.5f);
                        float poff_y = py_mid - poff_norm * (py_bot - py_top) * 0.9f;
                        ImU32 poff_col = (n.pitch_offset < -0.1f) ? IM_COL32(0, 180, 255, 220) : ((n.pitch_offset > 0.1f) ? IM_COL32(255, 50, 120, 220) : IM_COL32(200, 200, 200, 255));
                        v_draw->AddLine(ImVec2(px + pill_w * 0.5f, py_mid), ImVec2(px + pill_w * 0.5f, poff_y), poff_col, 3.0f);
                        v_draw->AddCircleFilled(ImVec2(px + pill_w * 0.5f, poff_y), 3.5f, poff_col);
                    }

                    // Nome da Nota no rodapé
                    const char* note_names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
                    char step_lbl[8];
                    snprintf(step_lbl, sizeof(step_lbl), "%s%d", note_names[n.pitch % 12], (n.pitch / 12) - 1);
                    v_draw->AddText(ImVec2(px + 3, py_bot + 2.0f), IM_COL32(140, 160, 180, 255), step_lbl);
                } else {
                    // Pílula Vazia de preenchimento do grid
                    char step_lbl[8];
                    snprintf(step_lbl, sizeof(step_lbl), "%d", i + 1);
                    v_draw->AddText(ImVec2(px + 4, py_bot + 2.0f), IM_COL32(80, 90, 100, 255), step_lbl);
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        // --- Atalhos Globais de Teclado (FL Studio Shortcuts) ---
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
            if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_S, false)) open_strum_modal = true;
            if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_U, false)) open_chop_modal = true;
            if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_R, false)) open_riff_modal = true;
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false)) {
                for (auto& n : notes) n.is_selected = true;
            }
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q, false)) {
                float beat_duration = 60.0f / bpm;
                float divisor = (current_snap_option == 0) ? 1.0f : (current_snap_option == 1) ? 2.0f : (current_snap_option == 2) ? 4.0f : 8.0f;
                float snap_step = beat_duration / divisor;
                bool any_sel = std::any_of(notes.begin(), notes.end(), [](const KuroDSP::MidiNote& x){ return x.is_selected; });
                for (auto& n : notes) {
                    if (any_sel && !n.is_selected) continue;
                    n.start_time = std::round(n.start_time / snap_step) * snap_step;
                }
            }

            // Ctrl + B: Quick Duplicate (FL Studio style)
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_B, false)) {
                bool any_sel = std::any_of(notes.begin(), notes.end(), [](const KuroDSP::MidiNote& x){ return x.is_selected; });
                float min_t = 1e9f, max_t = 0.0f;
                for (const auto& n : notes) {
                    if (!any_sel || n.is_selected) {
                        if (n.start_time < min_t) min_t = n.start_time;
                        if (n.start_time + n.duration > max_t) max_t = n.start_time + n.duration;
                    }
                }
                if (max_t > 0.0f) {
                    float beat_dur = 60.0f / (bpm > 20.0f ? bpm : 140.0f);
                    float bar_dur = beat_dur * 4.0f;
                    float offset = std::ceil((max_t - min_t) / bar_dur) * bar_dur;
                    if (offset < bar_dur) offset = bar_dur;
                    
                    std::vector<KuroDSP::MidiNote> cloned;
                    for (auto& n : notes) {
                        if (!any_sel || n.is_selected) {
                            KuroDSP::MidiNote cn = n;
                            cn.start_time += offset;
                            cn.is_selected = true;
                            cloned.push_back(cn);
                            n.is_selected = false;
                        }
                    }
                    for (const auto& cn : cloned) notes.push_back(cn);
                }
            }

            // Delete / Backspace: Apagar Selecionadas
            if (ImGui::IsKeyPressed(ImGuiKey_Delete, false) || ImGui::IsKeyPressed(ImGuiKey_Backspace, false)) {
                notes.erase(std::remove_if(notes.begin(), notes.end(), [](const KuroDSP::MidiNote& x){ return x.is_selected; }), notes.end());
            }

            // Up / Down Arrow: Transposição de Semitom ou Oitava (com Shift)
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, false)) {
                int shift_semi = io.KeyShift ? 12 : 1;
                for (auto& n : notes) {
                    if (n.is_selected) {
                        n.pitch = std::clamp(n.pitch + shift_semi, 0, 127);
                        g_piano_synth.triggerNote(n.pitch, 0.25f, 0.85f, ch_idx);
                    }
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, false)) {
                int shift_semi = io.KeyShift ? 12 : 1;
                for (auto& n : notes) {
                    if (n.is_selected) {
                        n.pitch = std::clamp(n.pitch - shift_semi, 0, 127);
                        g_piano_synth.triggerNote(n.pitch, 0.25f, 0.85f, ch_idx);
                    }
                }
            }
        }

        // --- POPUP MODAL: STRUMMING GENERATOR (Alt + S) ---
        if (open_strum_modal) {
            ImGui::OpenPopup("🎸 Strumming Generator (FL Style)");
            open_strum_modal = false;
        }
        if (ImGui::BeginPopupModal("🎸 Strumming Generator (FL Style)", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            static float strum_ms = 25.0f;
            static bool strum_asc = true;
            static float strum_vel_decay = 0.05f;

            ImGui::Text("Simule um dedilhado natural de violão/guitarra em acordes.");
            ImGui::Separator();
            ImGui::SliderFloat("Atraso do Strum (ms)", &strum_ms, 5.0f, 100.0f, "%.1f ms");
            ImGui::Checkbox("Ordem Crescente de Pitch", &strum_asc);
            ImGui::SliderFloat("Atenuação de Velocity", &strum_vel_decay, 0.0f, 0.2f, "%.2f");

            ImGui::Spacing();
            if (ImGui::Button("Aplicar Strum", ImVec2(130, 0))) {
                ApplyStrumming(notes, strum_ms / 1000.0f, strum_asc, strum_vel_decay);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancelar", ImVec2(100, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // --- POPUP MODAL: CHOP / ROLL GENERATOR (Alt + U) ---
        if (open_chop_modal) {
            ImGui::OpenPopup("✂️ Chop / Roll Generator (FL Style)");
            open_chop_modal = false;
        }
        if (ImGui::BeginPopupModal("✂️ Chop / Roll Generator (FL Style)", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            static int chop_subdivisions = 4;
            static bool chop_vel_ramp = true;

            ImGui::Text("Subdivida notas longas em rolls rápidos para Hi-Hats de Trap ou Synths.");
            ImGui::Separator();
            ImGui::SliderInt("Subdivisões por Nota", &chop_subdivisions, 2, 16);
            ImGui::Checkbox("Crescendo de Velocity (Ramp)", &chop_vel_ramp);

            ImGui::Spacing();
            if (ImGui::Button("Aplicar Chop", ImVec2(130, 0))) {
                ApplyChop(notes, chop_subdivisions, chop_vel_ramp);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancelar", ImVec2(100, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // --- Scrollbar Horizontal ---
        ImGui::SetCursorScreenPos(ImVec2(canvas_p0.x + key_width, canvas_p1.y + 5.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        float max_pan_x = std::max(0.0f, 5000.0f - canvas_sz.x);
        ImGui::SliderFloat("##pan_x", &pan_x, 0.0f, max_pan_x, "");
        ImGui::PopStyleColor();

        // --- Scrollbar Vertical ---
        ImGui::SetCursorScreenPos(ImVec2(canvas_p1.x + 5.0f, canvas_p0.y));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        float max_pan_y = std::max(0.0f, total_height - canvas_sz.y);
        ImGui::VSliderFloat("##pan_y", ImVec2(15.0f, canvas_sz.y), &pan_y, max_pan_y, 0.0f, "");
        ImGui::PopStyleColor();

        // Auto-Sync Automático Inteligente para a Playlist: garante que as notas do Piano Roll estejam visíveis na Playlist
        if (auto_sync_playlist && !notes.empty() && clip_manager.current_pattern_idx >= 0 && clip_manager.current_pattern_idx < (int)clip_manager.global_patterns.size()) {
            float max_t = 0.0f;
            for (const auto& n : notes) {
                if (n.start_time + n.duration > max_t) max_t = n.start_time + n.duration;
            }
            float bar_len = (60.0f / bpm) * 4.0f;
            float needed_len = (std::max)(bar_len * 4.0f, std::ceil(max_t / bar_len) * bar_len);
            int pat_id = clip_manager.global_patterns[clip_manager.current_pattern_idx].id;

            auto existing = clip_manager.getMidiClips(ch_idx);
            bool found = false;
            for (const auto& mc : existing) {
                if (mc.pattern_id == pat_id) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                clip_manager.addPatternClip(ch_idx, 0.0f, needed_len, pat_id);
            }
        }

        ImGui::PopStyleColor(); // Pop WindowBg color
        ImGui::End();
    }
}
