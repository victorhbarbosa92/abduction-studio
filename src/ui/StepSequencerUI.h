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
#include "ChannelInstrumentManager.h"

extern float dummy_vol[MAX_TRACKS];
extern float dummy_pan[MAX_TRACKS];
extern int channel_tracks[MAX_TRACKS];
extern bool track_mutes[MAX_TRACKS];
extern bool track_solos[MAX_TRACKS];
extern float track_volumes[MAX_TRACKS];
extern float track_linear_volumes[MAX_TRACKS];
extern std::string track_names[MAX_TRACKS];
extern KuroAudio::SynthEngine g_piano_synth;
extern bool is_playing;

namespace KuroUI {
    extern int selected_track_idx;
    extern bool show_piano_roll;
    extern bool show_sampler_settings;
    extern bool show_flex_browser;
    extern bool show_contrabass_window;
    extern bool show_delay_lama;

    // Track activity meters (decaying audio/note pulse)
    inline float g_channel_activity[MAX_TRACKS] = { 0 };

    // Custom vector-drawn FL Studio style rotary knob (sleek dark metallic with indicator notch)
    inline bool FLKnob(const char* label, float* p_value, float v_min, float v_max, float radius = 7.0f, const char* tooltip_format = "%.2f") {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 center = ImVec2(pos.x + radius + 1.0f, pos.y + radius + 2.0f);
        
        ImGui::InvisibleButton(label, ImVec2(radius * 2.0f + 2.0f, radius * 2.0f + 4.0f));
        bool value_changed = false;
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();
        
        if (is_hovered && io.MouseWheel != 0.0f) {
            float step = (v_max - v_min) / 30.0f;
            *p_value += io.MouseWheel * step;
            if (*p_value < v_min) *p_value = v_min;
            if (*p_value > v_max) *p_value = v_max;
            value_changed = true;
        }
        
        if (is_active && io.MouseDelta.y != 0.0f) {
            float step = (v_max - v_min) / 100.0f;
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
        
        ImDrawList* draw = ImGui::GetWindowDrawList();
        
        // Knob base body (dark metallic gradient look)
        draw->AddCircleFilled(center, radius, IM_COL32(32, 38, 46, 255), 16);
        draw->AddCircle(center, radius, is_active ? IM_COL32(0, 230, 255, 255) : (is_hovered ? IM_COL32(100, 130, 160, 255) : IM_COL32(48, 56, 68, 255)), 16, 1.2f);
        
        // Center inner cap
        draw->AddCircleFilled(center, radius * 0.7f, IM_COL32(22, 26, 32, 255), 12);
        
        // Indicator line
        ImVec2 line_end = ImVec2(center.x + std::cos(angle - 1.570796f) * (radius - 1.5f), center.y + std::sin(angle - 1.570796f) * (radius - 1.5f));
        draw->AddLine(center, line_end, is_active ? IM_COL32(0, 255, 255, 255) : IM_COL32(220, 235, 245, 255), 1.8f);
        
        if (is_hovered) {
            ImGui::SetTooltip(tooltip_format, *p_value);
        }
        
        return value_changed;
    }

    inline bool Knob(const char* label, float* p_value, float v_min, float v_max, float radius = 8.5f) {
        return FLKnob(label, p_value, v_min, v_max, radius);
    }

    inline void RenderStepSequencer(bool* open, KuroDSP::TimelineManager& timeline_mgr, float bpm, ClipManager& clip_manager) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        // 16 Standard Pitches for the 16 Tracks
        static const int track_pitches[16] = {
            36, // Trk 0: 808 Kick (C2)
            30, // Trk 1: 808 Clap (F#1)
            42, // Trk 2: 808 HiHat (F#2)
            38, // Trk 3: 808 Snare (D2)
            46, // Trk 4: FLEX Bass (A#2)
            42, // Trk 5: Closed Hat (F#2)
            65, // Trk 6: Tribal Perc (F4)
            78, // Trk 7: FM Squelch (F#5)
            66, // Trk 8: Acid Lead (F#4)
            78, // Trk 9: Counter-Arp (F#5)
            66, // Trk 10: SuperSaw (F#4)
            42, // Trk 11: Sub Drone (F#2)
            57, // Trk 12: Symphonic Strings (A3)
            66, // Trk 13: Steinway Piano (F#4)
            66, // Trk 14: Vocal Mantra (F#4)
            38  // Trk 15: Snare Roll / Riser (D2)
        };

        static const char* default_names[16] = {
            "808 Kick", "808 Clap", "808 HiHat", "808 Snare", "FLEX Bass",
            "Closed Hat", "Tribal Perc", "FM Squelch", "Acid Lead", "Counter-Arp",
            "SuperSaw", "Sub Drone", "Symphonic Strings", "Steinway Piano", "Vocal Mantra", "Snare Roll"
        };

        // Decaimento suave dos medidores de atividade LED de cada canal
        for (int i = 0; i < MAX_TRACKS; i++) {
            if (g_channel_activity[i] > 0.0f) {
                g_channel_activity[i] = std::max(0.0f, g_channel_activity[i] - io.DeltaTime * 3.5f);
            }
        }

        float beat_duration = 60.0f / (bpm > 20.0f ? bpm : 140.0f);
        float snap_step = beat_duration * 0.25f; // 1/16 note

        // Playhead do sequenciador em tempo real
        uint64_t m_frame = timeline_mgr.getMasterFrame();
        float cur_play_time_sec = (float)m_frame / 44100.0f;
        int current_playing_step = ::is_playing ? ((int)(std::fmod(cur_play_time_sec, beat_duration * 4.0f) / snap_step) % 16) : -1;

        std::lock_guard<std::mutex> lock(timeline_mgr.timeline_mutex);

        // ── 1. CABEÇALHO DO FL STUDIO CHANNEL RACK ──────────────────────────
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.19f, 0.22f, 0.26f, 1.0f)); // #303741 FL Studio Header Gray
        ImGui::BeginChild("##ChannelRackHeader", ImVec2(0, 32), false);
        {
            ImVec2 h_p0 = ImGui::GetCursorScreenPos();
            ImDrawList* h_draw = ImGui::GetWindowDrawList();

            // Menu icon ▶
            ImGui::SetCursorPos(ImVec2(6, 6));
            if (ImGui::Button("▶##cr_menu", ImVec2(20, 20))) {
                ImGui::OpenPopup("##cr_main_options");
            }
            if (ImGui::BeginPopup("##cr_main_options")) {
                if (ImGui::MenuItem("Novo Padrão (Pattern)")) {
                    Pattern np;
                    np.id = (int)clip_manager.global_patterns.size() + 1;
                    np.name = "Pattern " + std::to_string(np.id);
                    np.color = 0xFF00E5FF;
                    clip_manager.global_patterns.push_back(np);
                    clip_manager.current_pattern_idx = (int)clip_manager.global_patterns.size() - 1;
                }
                if (ImGui::MenuItem("Limpar Todos os Passos")) {
                    int c_p = clip_manager.current_pattern_idx;
                    if (c_p >= 0 && c_p < (int)clip_manager.global_patterns.size()) {
                        for (int c = 0; c < MAX_TRACKS; c++) clip_manager.global_patterns[c_p].getChannelNotes(c).clear();
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Preencher Padrão 4-on-the-Floor (Kick)")) {
                    int c_p = clip_manager.current_pattern_idx;
                    if (c_p >= 0 && c_p < (int)clip_manager.global_patterns.size()) {
                        auto& notes = clip_manager.global_patterns[c_p].getChannelNotes(0);
                        notes.clear();
                        for (int s = 0; s < 16; s += 4) {
                            notes.push_back(KuroDSP::MidiNote(track_pitches[0], s * snap_step, snap_step * 0.85f, 0.95f, 1.0f, 0));
                        }
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine(0, 4);

            // Undo icon ↺
            if (ImGui::Button("↺##cr_undo", ImVec2(20, 20))) {
                clip_manager.undo();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Desfazer (Ctrl+Z)");

            ImGui::SameLine(0, 6);

            // Filter dropdown: [ All ▾ ]
            static int filter_idx = 0;
            const char* filter_modes[] = { "All", "Audio", "Unsorted", "Drums", "Synths" };
            ImGui::SetNextItemWidth(70.0f);
            ImGui::Combo("##cr_filter_group", &filter_idx, filter_modes, IM_ARRAYSIZE(filter_modes));

            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("⋮");
            ImGui::SameLine(0, 8);

            // Título: 🔊 Channel rack
            ImGui::TextColored(ImVec4(0.85f, 0.90f, 0.95f, 1.0f), "🔊 Channel rack");

            // Lado direito do cabeçalho
            float header_right_w = 180.0f;
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - header_right_w);

            // Swing Knob (Conectado ao motor de áudio)
            float cr_swing = timeline_mgr.getSwing();
            if (FLKnob("##cr_swing", &cr_swing, 0.0f, 1.0f, 6.5f, "Swing: %.0f%%")) {
                timeline_mgr.setSwing(cr_swing);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Groove Swing / Shuffle Global: %.0f%%", cr_swing * 100.0f);
            }
            ImGui::SameLine(0, 10);

            // Global Options [...]
            if (ImGui::Button("...##cr_opt", ImVec2(28, 20))) {}

            ImGui::SameLine(0, 4);

            // Graph Editor icon 📊
            static bool show_graph_editor = false;
            if (ImGui::Button("📊##cr_graph", ImVec2(22, 20))) {
                show_graph_editor = !show_graph_editor;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Editor de Gráficos (Velocity / Pitch / Pan)");

            ImGui::SameLine(0, 4);

            // Piano Roll View icon 🎹 (Amarelo / Dourado ativo como no FL Studio)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.75f, 0.10f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            if (ImGui::Button("🎹##cr_pr_toggle", ImVec2(22, 20))) {
                show_piano_roll = !show_piano_roll;
                if (show_piano_roll) {
                    extern bool g_need_focus_piano_roll;
                    g_need_focus_piano_roll = true;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Alternar Visualização do Piano Roll (F7)");
            ImGui::PopStyleColor(2);

            ImGui::SameLine(0, 6);

            // Botão Fechar [✖]
            if (ImGui::Button("✖##cr_close", ImVec2(20, 20))) {
                if (open) *open = false;
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        // ── 2. SELETOR DE PADRÃO (PATTERN) ATIVO ────────────────────────────
        int cur_pat_idx = clip_manager.current_pattern_idx;
        int total_pats = (int)clip_manager.global_patterns.size();
        if (cur_pat_idx < 0 || cur_pat_idx >= total_pats) {
            cur_pat_idx = 0;
            clip_manager.current_pattern_idx = 0;
        }
        auto& active_pat = clip_manager.global_patterns[cur_pat_idx];

        // ── 3. LISTA DE CANAIS E GRADE DE 16 PASSOS (FL STUDIO EXACT LOOK) ──
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.14f, 0.17f, 0.20f, 1.0f)); // #242b33 FL Studio Main Background
        ImGui::BeginChild("ChannelRackBody", ImVec2(0, -32), false, ImGuiWindowFlags_HorizontalScrollbar);
        {
            int num_channels = 8; // Canais principais configurados
            for (int inst = 0; inst < num_channels; inst++) {
                ImGui::PushID(inst + 7000);
                
                ImVec2 row_p0 = ImGui::GetCursorScreenPos();
                float row_h = 24.0f;
                float cur_x = row_p0.x + 4.0f;
                float cur_y = row_p0.y + 1.0f;

                // 3.1 LED de Mute / Atividade (Verde Neon com Anel Escuro)
                bool is_muted = ::track_mutes[inst];
                ImVec2 led_center = ImVec2(cur_x + 6.0f, cur_y + 10.0f);
                
                // Anel externo do LED
                draw->AddCircleFilled(led_center, 4.5f, IM_COL32(20, 26, 30, 255), 16);
                draw->AddCircle(led_center, 4.5f, IM_COL32(10, 14, 18, 255), 16, 1.0f);
                
                // Luz interna do LED (Verde FL Studio aceso ou apagado)
                ImU32 led_color = !is_muted ? IM_COL32(70, 220, 60, 255) : IM_COL32(30, 42, 32, 255);
                draw->AddCircleFilled(led_center, 3.2f, led_color, 12);
                if (!is_muted) {
                    // Ponto de brilho no LED
                    draw->AddCircleFilled(ImVec2(led_center.x - 1.0f, led_center.y - 1.0f), 1.0f, IM_COL32(200, 255, 200, 255), 8);
                }

                ImGui::SetCursorScreenPos(ImVec2(cur_x, cur_y));
                if (ImGui::InvisibleButton("##led_btn", ImVec2(14, 20))) {
                    ::track_mutes[inst] = !::track_mutes[inst];
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                    for (int t = 0; t < MAX_TRACKS; t++) ::track_solos[t] = false;
                    ::track_solos[inst] = true;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Mute (Esq) / Solo (Dir)");

                cur_x += 18.0f;

                // 3.2 Pan Knob (Rotativo)
                float dummy_p = (inst < 8) ? ::dummy_pan[inst] : 0.0f;
                ImGui::SetCursorScreenPos(ImVec2(cur_x, cur_y));
                if (FLKnob("##pan", &dummy_p, -1.0f, 1.0f, 6.5f, "Pan: %.2f")) {
                    if (inst < 8) ::dummy_pan[inst] = dummy_p;
                }
                cur_x += 18.0f;

                // 3.3 Volume Knob (Rotativo)
                float trk_v = ::track_linear_volumes[inst];
                ImGui::SetCursorScreenPos(ImVec2(cur_x, cur_y));
                if (FLKnob("##vol", &trk_v, 0.0f, 1.5f, 6.5f, "Vol: %.2f")) {
                    ::track_linear_volumes[inst] = trk_v;
                    ::track_volumes[inst] = (trk_v <= 0.001f) ? -60.0f : 20.0f * std::log10(trk_v);
                }
                cur_x += 20.0f;

                // 3.4 Target Mixer Track Box (ex: [ 1 ], [ 2 ], etc. com relevo e números)
                int mixer_trk = ::channel_tracks[inst] > 0 ? ::channel_tracks[inst] : (inst + 1);
                ImVec2 mbox_p0 = ImVec2(cur_x, cur_y + 1.0f);
                ImVec2 mbox_p1 = ImVec2(cur_x + 24.0f, cur_y + 21.0f);
                
                // Inset box bevel
                draw->AddRectFilled(mbox_p0, mbox_p1, IM_COL32(26, 30, 36, 255), 2.0f);
                draw->AddRect(mbox_p0, mbox_p1, IM_COL32(18, 22, 26, 255), 2.0f);
                
                // Texto do canal do mixer
                char m_str[8];
                snprintf(m_str, sizeof(m_str), "%d", mixer_trk);
                float m_txt_w = ImGui::CalcTextSize(m_str).x;
                draw->AddText(ImVec2(cur_x + (24.0f - m_txt_w) * 0.5f, cur_y + 3.0f), IM_COL32(170, 185, 200, 255), m_str);

                ImGui::SetCursorScreenPos(mbox_p0);
                if (ImGui::InvisibleButton("##mixer_trk_btn", ImVec2(24, 20))) {
                    ::channel_tracks[inst] = (mixer_trk % 16) + 1;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Canal do Mixer Alvo: %d (Clique para alterar)", mixer_trk);
                    if (io.MouseWheel != 0.0f) {
                        int nt = mixer_trk + (int)io.MouseWheel;
                        if (nt < 1) nt = 1;
                        if (nt > 16) nt = 16;
                        ::channel_tracks[inst] = nt;
                    }
                }
                cur_x += 28.0f;

                // 3.5.1 Ícone do Instrumento (Atalho de 1 Clique para Abrir o Editor Gráfico)
                ImVec2 icon_p0 = ImVec2(cur_x, cur_y + 1.0f);
                ImVec2 icon_p1 = ImVec2(cur_x + 22.0f, cur_y + 21.0f);
                bool is_icon_hovered = io.MousePos.x >= icon_p0.x && io.MousePos.x <= icon_p1.x && io.MousePos.y >= icon_p0.y && io.MousePos.y <= icon_p1.y;

                ImU32 icon_bg = is_icon_hovered ? IM_COL32(32, 46, 60, 255) : IM_COL32(20, 25, 32, 255);
                draw->AddRectFilled(icon_p0, icon_p1, icon_bg, 3.0f);
                draw->AddRect(icon_p0, icon_p1, is_icon_hovered ? IM_COL32(0, 229, 255, 220) : IM_COL32(35, 45, 55, 255), 3.0f);

                const char* inst_icon = GetChannelIcon(inst);
                ImVec2 icon_sz = ImGui::CalcTextSize(inst_icon);
                draw->AddText(ImVec2(icon_p0.x + (22.0f - icon_sz.x) * 0.5f, icon_p0.y + 2.0f), IM_COL32(255, 255, 255, 255), inst_icon);

                ImGui::SetCursorScreenPos(icon_p0);
                if (ImGui::InvisibleButton("##inst_icon_btn", ImVec2(22, 20))) {
                    selected_track_idx = inst;
                    TriggerOpenInstrument(inst);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Abrir Editor: %s (1 Clique)", GetChannelInstrumentName(inst));
                }
                cur_x += 24.0f;

                // 3.5.2 Botão Tátil do Canal (FL Studio Name Button com chanfro e cor)
                bool is_selected = (selected_track_idx == inst);
                const char* ch_name = (!::track_names[inst].empty()) ? ::track_names[inst].c_str() : default_names[inst % 16];
                
                float btn_w = 86.0f;
                float btn_h = 20.0f;
                ImVec2 btn_p0 = ImVec2(cur_x, cur_y + 1.0f);
                ImVec2 btn_p1 = ImVec2(cur_x + btn_w, cur_y + 1.0f + btn_h);

                bool is_btn_hovered = io.MousePos.x >= btn_p0.x && io.MousePos.x <= btn_p1.x && io.MousePos.y >= btn_p0.y && io.MousePos.y <= btn_p1.y;

                // Gradiente e chanfro 3D do botão FL Studio
                ImU32 btn_top_col = is_selected ? IM_COL32(52, 64, 76, 255) : (is_btn_hovered ? IM_COL32(58, 66, 76, 255) : IM_COL32(48, 54, 62, 255));
                ImU32 btn_bot_col = is_selected ? IM_COL32(36, 44, 54, 255) : (is_btn_hovered ? IM_COL32(42, 48, 56, 255) : IM_COL32(32, 36, 42, 255));
                draw->AddRectFilledMultiColor(btn_p0, btn_p1, btn_top_col, btn_top_col, btn_bot_col, btn_bot_col);
                
                // Chanfro superior iluminado
                draw->AddLine(btn_p0, ImVec2(btn_p1.x, btn_p0.y), IM_COL32(80, 92, 106, 200));
                // Chanfro inferior sombreado
                draw->AddLine(ImVec2(btn_p0.x, btn_p1.y), btn_p1, IM_COL32(18, 22, 26, 255));
                draw->AddRect(btn_p0, btn_p1, is_selected ? IM_COL32(0, 220, 255, 220) : IM_COL32(22, 26, 30, 255), 2.0f);

                // Tarja inferior de cor personalizada se definida
                if (g_channel_slots[inst].custom_color != 0) {
                    draw->AddLine(ImVec2(btn_p0.x + 2.0f, btn_p1.y - 1.5f), ImVec2(btn_p1.x - 2.0f, btn_p1.y - 1.5f), g_channel_slots[inst].custom_color, 2.0f);
                }

                // Texto do Instrumento
                draw->AddText(ImVec2(btn_p0.x + 6.0f, btn_p0.y + 3.0f), is_selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(210, 220, 230, 255), ch_name);

                ImGui::SetCursorScreenPos(btn_p0);
                if (ImGui::InvisibleButton("##ch_btn", ImVec2(btn_w, btn_h))) {
                    selected_track_idx = inst;
                    g_channel_activity[inst] = 1.0f; // Acende o medidor de atividade
                    g_piano_synth.triggerNote(track_pitches[inst], 0.25f, 0.9f, inst);
                }

                // Menu de Contexto FL Studio no Botão do Canal (Clique Direito)
                std::string popup_name = "##cr_popup_" + std::to_string(inst);
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                    selected_track_idx = inst;
                    ImGui::OpenPopup(popup_name.c_str());
                }
                if (g_force_open_popup_ch == inst) {
                    selected_track_idx = inst;
                    ImGui::OpenPopup(popup_name.c_str());
                    g_force_open_popup_ch = -1;
                }

                cur_x += btn_w + 2.0f;

                // 3.5.3 Botão de Menu de 4 Pontinhos (☷) para Troca Direta de Instrumento / Sample
                ImVec2 m4_p0 = ImVec2(cur_x, cur_y + 1.0f);
                ImVec2 m4_p1 = ImVec2(cur_x + 19.0f, cur_y + 21.0f);
                bool is_m4_hovered = io.MousePos.x >= m4_p0.x && io.MousePos.x <= m4_p1.x && io.MousePos.y >= m4_p0.y && io.MousePos.y <= m4_p1.y;

                ImU32 m4_bg = is_m4_hovered ? IM_COL32(45, 58, 72, 255) : IM_COL32(26, 32, 40, 255);
                draw->AddRectFilled(m4_p0, m4_p1, m4_bg, 3.0f);
                draw->AddRect(m4_p0, m4_p1, is_m4_hovered ? IM_COL32(0, 229, 255, 200) : IM_COL32(36, 44, 52, 255), 3.0f);

                // Desenho geométrico dos 4 pontinhos vetoriais (2x2)
                float m4_cx = m4_p0.x + 9.5f;
                float m4_cy = m4_p0.y + 10.0f;
                ImU32 dot_col = is_m4_hovered ? IM_COL32(0, 240, 255, 255) : IM_COL32(180, 195, 210, 220);
                draw->AddCircleFilled(ImVec2(m4_cx - 3.2f, m4_cy - 3.2f), 1.5f, dot_col);
                draw->AddCircleFilled(ImVec2(m4_cx + 3.2f, m4_cy - 3.2f), 1.5f, dot_col);
                draw->AddCircleFilled(ImVec2(m4_cx - 3.2f, m4_cy + 3.2f), 1.5f, dot_col);
                draw->AddCircleFilled(ImVec2(m4_cx + 3.2f, m4_cy + 3.2f), 1.5f, dot_col);

                ImGui::SetCursorScreenPos(m4_p0);
                if (ImGui::InvisibleButton("##m4_menu_btn", ImVec2(19, 20))) {
                    selected_track_idx = inst;
                    ImGui::OpenPopup(popup_name.c_str());
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Menu de Instrumentos, Sintetizadores & Opções da Faixa");
                }

                if (ImGui::BeginPopup(popup_name.c_str())) {
                    RenderChannelInstrumentMenu(inst, clip_manager, snap_step, track_pitches);
                    ImGui::EndPopup();
                }

                cur_x += 21.0f;

                // 3.6 Barra de Atividade / Áudio (Pill Vertical que brilha verde quando tocado)
                float act_val = g_channel_activity[inst];
                ImVec2 act_p0 = ImVec2(cur_x, cur_y + 2.0f);
                ImVec2 act_p1 = ImVec2(cur_x + 4.0f, cur_y + 20.0f);
                
                draw->AddRectFilled(act_p0, act_p1, IM_COL32(24, 28, 34, 255), 2.0f);
                if (act_val > 0.01f) {
                    ImU32 act_col = IM_COL32(80, 255, 60, (int)(act_val * 255));
                    draw->AddRectFilled(act_p0, act_p1, act_col, 2.0f);
                }
                draw->AddRect(act_p0, act_p1, IM_COL32(16, 20, 24, 255), 2.0f);

                cur_x += 10.0f;

                // 3.7 Os 16 Botões Táteis de Passo (FL Studio Step Pads em Blocos de 4)
                auto& ch_notes = active_pat.getChannelNotes(inst);
                float pad_w = 15.0f;
                float pad_h = 20.0f;

                for (int step = 0; step < 16; step++) {
                    ImGui::PushID(step);
                    float step_time = step * snap_step;
                    
                    bool active = false;
                    int note_idx = -1;
                    for (size_t i = 0; i < ch_notes.size(); i++) {
                        if (std::abs(ch_notes[i].start_time - step_time) < 0.02f) {
                            active = true;
                            note_idx = (int)i;
                            break;
                        }
                    }

                    // Se for o passo atualmente tocado pelo playhead, acende a atividade
                    if (active && current_playing_step == step) {
                        g_channel_activity[inst] = 1.0f;
                    }

                    ImVec2 pad_p0 = ImVec2(cur_x, cur_y + 1.0f);
                    ImVec2 pad_p1 = ImVec2(cur_x + pad_w, cur_y + 1.0f + pad_h);
                    
                    bool is_pad_hovered = io.MousePos.x >= pad_p0.x && io.MousePos.x <= pad_p1.x && io.MousePos.y >= pad_p0.y && io.MousePos.y <= pad_p1.y;

                    // Paleta Exata do FL Studio (4 passos Cinza Grafite, 4 passos Castanho/Avermelhado)
                    int beat_idx = (step / 4) % 2;
                    ImU32 base_top_col, base_bot_col;

                    if (active) {
                        // Ativo: Ouro / Laranja Radiante do FL Studio
                        base_top_col = is_pad_hovered ? IM_COL32(255, 185, 60, 255) : IM_COL32(255, 160, 36, 255);
                        base_bot_col = is_pad_hovered ? IM_COL32(230, 130, 20, 255) : IM_COL32(210, 110, 15, 255);
                    } else {
                        if (beat_idx == 0) {
                            // Beat 1 e 3: Cinza Grafite (#40454e / #2e333a)
                            base_top_col = is_pad_hovered ? IM_COL32(72, 80, 92, 255) : IM_COL32(58, 64, 74, 255);
                            base_bot_col = is_pad_hovered ? IM_COL32(48, 54, 62, 255) : IM_COL32(38, 44, 50, 255);
                        } else {
                            // Beat 2 e 4: Castanho Avermelhado (#5a4542 / #463532)
                            base_top_col = is_pad_hovered ? IM_COL32(100, 75, 72, 255) : IM_COL32(84, 62, 60, 255);
                            base_bot_col = is_pad_hovered ? IM_COL32(72, 52, 50, 255) : IM_COL32(58, 42, 40, 255);
                        }
                    }

                    // Renderização do corpo do Pad em estilo Pílula com Relevo
                    draw->AddRectFilledMultiColor(pad_p0, pad_p1, base_top_col, base_top_col, base_bot_col, base_bot_col);
                    
                    // Chanfro superior
                    draw->AddLine(ImVec2(pad_p0.x + 1, pad_p0.y), ImVec2(pad_p1.x - 1, pad_p0.y), active ? IM_COL32(255, 220, 150, 255) : IM_COL32(110, 120, 135, 140));
                    // Chanfro inferior
                    draw->AddLine(ImVec2(pad_p0.x + 1, pad_p1.y), ImVec2(pad_p1.x - 1, pad_p1.y), IM_COL32(14, 18, 22, 255));
                    // Contorno do Pad
                    draw->AddRect(pad_p0, pad_p1, IM_COL32(18, 22, 26, 255), 2.5f);

                    // Rebaixo Central (Friso horizontal característico do FL Studio)
                    float mid_y = pad_p0.y + pad_h * 0.5f;
                    draw->AddLine(ImVec2(pad_p0.x + 2, mid_y), ImVec2(pad_p1.x - 2, mid_y), active ? IM_COL32(255, 235, 180, 220) : IM_COL32(22, 26, 30, 200));

                    // Destaque do Playhead varrendo o sequenciador em tempo real
                    if (current_playing_step == step) {
                        draw->AddRect(pad_p0, pad_p1, IM_COL32(0, 255, 255, 255), 2.5f, 0, 1.5f);
                    }

                    // Interação do Mouse
                    ImGui::SetCursorScreenPos(pad_p0);
                    if (ImGui::InvisibleButton("##step_btn", ImVec2(pad_w, pad_h))) {
                        clip_manager.pushUndo();
                        if (active) {
                            if (note_idx >= 0 && note_idx < (int)ch_notes.size()) {
                                ch_notes.erase(ch_notes.begin() + note_idx);
                            }
                        } else {
                            KuroDSP::MidiNote nn(track_pitches[inst], step_time, snap_step * 0.88f, 0.90f, 1.0f, inst);
                            ch_notes.push_back(nn);
                            g_channel_activity[inst] = 1.0f;
                            g_piano_synth.triggerNote(track_pitches[inst], 0.25f, 0.9f, inst);
                        }
                    }

                    // Clique-Direito ou Arraste com Direito = Apaga imediatamente
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                        if (active && note_idx >= 0 && note_idx < (int)ch_notes.size()) {
                            clip_manager.pushUndo();
                            ch_notes.erase(ch_notes.begin() + note_idx);
                        }
                    }

                    // Arraste com Esquerdo = Pinta e ativa continuamente
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
                        if (!active) {
                            clip_manager.pushUndo();
                            KuroDSP::MidiNote nn(track_pitches[inst], step_time, snap_step * 0.88f, 0.90f, 1.0f, inst);
                            ch_notes.push_back(nn);
                            g_channel_activity[inst] = 1.0f;
                            g_piano_synth.triggerNote(track_pitches[inst], 0.25f, 0.9f, inst);
                        }
                    }

                    // Espaçamento: 2px entre passos, e +4px extra de intervalo a cada 4 passos (batida)
                    cur_x += pad_w + ((step % 4 == 3) ? 6.0f : 2.0f);
                    ImGui::PopID();
                }

                // Declarar a área da linha para o ImGui expandir as bordas e scrollbars corretamente
                ImGui::SetCursorScreenPos(ImVec2(row_p0.x, row_p0.y));
                ImGui::Dummy(ImVec2(cur_x - row_p0.x + 10.0f, row_h + 3.0f));
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        // ── 4. BARRA INFERIOR (FL STUDIO ADD CHANNEL & LENGTH CONTROLS) ─────
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.21f, 0.25f, 1.0f));
        ImGui::BeginChild("##ChannelRackFooter", ImVec2(0, 26), false);
        {
            // Botão [+] centralizado sob os botões de instrumento
            ImGui::SetCursorPos(ImVec2(80.0f, 3.0f));
            if (ImGui::Button("+##add_channel_btn", ImVec2(85, 20))) {
                ImGui::OpenPopup("##cr_add_plugin_popup");
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Adicionar Novo Instrumento / Sampler / VST (+)");

            if (ImGui::BeginPopup("##cr_add_plugin_popup")) {
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "➕ ADICIONAR INSTRUMENTO / PLUGIN");
                ImGui::Separator();
                if (ImGui::MenuItem("FL DirectWave (Multi-Sampler)")) { show_sampler_settings = true; }
                if (ImGui::MenuItem("FL Flex Synth (Psytrance / EDM)")) { show_flex_browser = true; }
                if (ImGui::MenuItem("Contrabaixo Acústico Real")) { show_contrabass_window = true; }
                if (ImGui::MenuItem("Delay Lama (Monge 3D)")) { show_delay_lama = true; }
                if (ImGui::MenuItem("3x Osc (Triple Oscillator)")) {}
                if (ImGui::MenuItem("Sytrus FM Synth")) {}
                if (ImGui::MenuItem("Harmor Additive Synth")) {}
                ImGui::EndPopup();
            }

            // Slider / Dragger de Extensão de Passos no Canto Inferior Direito
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 140.0f);
            ImGui::TextDisabled("Extensão:");
            ImGui::SameLine(0, 6);
            static int step_length_mode = 0;
            const char* length_modes[] = { "16 Passos", "32 Passos", "64 Passos" };
            ImGui::SetNextItemWidth(90.0f);
            ImGui::Combo("##step_length", &step_length_mode, length_modes, IM_ARRAYSIZE(length_modes));
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        // Modal de renomeação de faixa (se ativo)
        RenderTrackRenameModal();
    }
}
