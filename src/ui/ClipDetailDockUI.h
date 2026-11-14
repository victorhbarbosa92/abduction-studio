#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "imgui.h"
#include "imgui_internal.h"
#include "../core/ClipManager.h"
#include "../core/TimelineManager.h"
#include "AudioClipEditorUI.h"
#include "ChannelInstrumentManager.h"
#include "../plugin_manager/ThematicSynths.h"

extern KuroDSP::PsytranceRollingBassSynth g_psy_bass_synth;
extern KuroAudio::SynthEngine g_piano_synth;
extern bool show_piano_roll;
extern int selected_track_idx;

namespace KuroUI {
    extern bool show_playlist;

    enum class DetailDockMode {
        AUDIO_CLIP = 0,
        MINI_PIANO_ROLL = 1
    };

    class ClipDetailDockUI {
    public:
        bool is_open = false;
        DetailDockMode current_mode = DetailDockMode::MINI_PIANO_ROLL;

        // Active Context
        int active_track_idx = 0;
        int active_clip_idx = -1;
        int active_pattern_id = -1;

        // Mini Piano Roll View Parameters
        int base_pitch = 30;       // Pitch inferior visível (0 a 127 - total_visible_keys)
        int total_visible_keys = 14; // 14 semitons visíveis (permite notas altas e nítidas de 10-12px)
        int snap_mode = 0;         // 0 = 1/16th, 1 = 1/8th, 2 = 1/4th, 3 = 1/32nd
        int active_tool = 0;       // 0 = Draw/Move, 1 = Erase
        bool audition_enabled = true;

        // Interaction State for Note Dragging / Resizing
        int dragging_note_idx = -1;
        bool is_resizing_note = false;
        float orig_note_start = 0.0f;
        float orig_note_dur = 0.0f;
        int orig_note_pitch = 60;
        ImVec2 drag_start_mouse = ImVec2(0, 0);

        // Velocity Dragging
        int active_vel_note_idx = -1;

        ClipDetailDockUI() {}

        void openMidi(int track_idx, int clip_idx, int pattern_id) {
            active_track_idx = std::clamp(track_idx, 0, MAX_TRACKS - 1);
            active_clip_idx = clip_idx;
            active_pattern_id = pattern_id;
            current_mode = DetailDockMode::MINI_PIANO_ROLL;
            is_open = true;
            dragging_note_idx = -1;
            active_vel_note_idx = -1;
            autoFitOctaveToNotes();
        }

        void openMidiChannel(int track_idx, int pattern_id) {
            active_track_idx = std::clamp(track_idx, 0, MAX_TRACKS - 1);
            active_clip_idx = -1;
            active_pattern_id = pattern_id;
            current_mode = DetailDockMode::MINI_PIANO_ROLL;
            is_open = true;
            dragging_note_idx = -1;
            active_vel_note_idx = -1;
            autoFitOctaveToNotes();
        }

        void openAudio(int track_idx, int clip_idx) {
            active_track_idx = std::clamp(track_idx, 0, MAX_TRACKS - 1);
            active_clip_idx = clip_idx;
            current_mode = DetailDockMode::AUDIO_CLIP;
            is_open = true;
            dragging_note_idx = -1;
            active_vel_note_idx = -1;
            g_audio_clip_editor_ui.open(track_idx, clip_idx);
        }

        void close() {
            is_open = false;
            dragging_note_idx = -1;
            active_vel_note_idx = -1;
            g_audio_clip_editor_ui.close();
        }

        void autoFitOctaveToNotes() {
            Pattern* p = getActivePattern(::g_clip_manager);
            if (!p) {
                base_pitch = (active_track_idx <= 1) ? 30 : ((active_track_idx <= 3) ? 38 : 55);
                return;
            }
            auto& notes = p->getChannelNotes(active_track_idx);
            if (notes.empty()) {
                base_pitch = (active_track_idx <= 1) ? 30 : ((active_track_idx <= 3) ? 38 : 55);
                return;
            }
            int min_p = 127;
            int max_p = 0;
            for (const auto& n : notes) {
                if (n.pitch < min_p) min_p = n.pitch;
                if (n.pitch > max_p) max_p = n.pitch;
            }
            // Centraliza o range visível perfeitamente na média das notas da trilha
            int center_p = (min_p + max_p) / 2;
            base_pitch = std::clamp(center_p - (total_visible_keys / 2), 0, 127 - total_visible_keys);
        }

        Pattern* getActivePattern(ClipManager& clip_mgr) {
            for (auto& pat : clip_mgr.global_patterns) {
                if (pat.id == active_pattern_id) return &pat;
            }
            if (active_pattern_id >= 0 && active_pattern_id < (int)clip_mgr.global_patterns.size()) {
                return &clip_mgr.global_patterns[active_pattern_id];
            }
            if (!clip_mgr.global_patterns.empty()) {
                int cur = std::clamp(clip_mgr.current_pattern_idx, 0, (int)clip_mgr.global_patterns.size() - 1);
                return &clip_mgr.global_patterns[cur];
            }
            return nullptr;
        }

        void auditNote(int pitch, float dur = 0.20f, float vel = 0.9f) {
            if (!audition_enabled) return;
            if (active_track_idx == 1 && g_channel_slots[1].type == ChannelInstrumentType::KURO_RHYTHM_BASS) {
                g_psy_bass_synth.pushMidiEvent({pitch, pitch, true, vel, 0.0f, 0.5f, 0.5f, dur});
            } else {
                g_piano_synth.triggerNote(pitch, dur, vel, active_track_idx);
            }
        }

        void Render(ClipManager& clip_mgr, KuroDSP::TimelineManager& timeline, ImVec2 pos, ImVec2 size) {
            if (!is_open) return;
            RenderInline(clip_mgr, timeline, pos, size);
        }

        void RenderInline(ClipManager& clip_mgr, KuroDSP::TimelineManager& timeline, ImVec2 pos, ImVec2 size) {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImGuiIO& io = ImGui::GetIO();

            static const char* note_names[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

            ImVec2 dock_p0 = pos;
            ImVec2 dock_p1 = ImVec2(pos.x + size.x, pos.y + size.y);

            // Fundo escuro Cyberpunk com borda Ciano Neon
            draw->AddRectFilled(dock_p0, dock_p1, IM_COL32(9, 13, 20, 255), 6.0f);
            draw->AddRect(dock_p0, dock_p1, IM_COL32(0, 220, 255, 180), 6.0f, 0, 1.5f);

            // ─────────────────────────────────────────────────────────────────
            // 1. TOOLBAR SUPERIOR DO DOCK (ESTILO ABLETON LIVE CLIP VIEW)
            // ─────────────────────────────────────────────────────────────────
            float header_h = 28.0f;
            ImVec2 hdr_p0 = dock_p0;
            ImVec2 hdr_p1 = ImVec2(dock_p1.x, dock_p0.y + header_h);
            draw->AddRectFilled(hdr_p0, hdr_p1, IM_COL32(14, 20, 30, 255), 6.0f, ImDrawFlags_RoundCornersTop);
            draw->AddLine(ImVec2(hdr_p0.x, hdr_p1.y), hdr_p1, IM_COL32(28, 44, 62, 255), 1.0f);

            ImGui::SetCursorScreenPos(ImVec2(hdr_p0.x + 8.0f, hdr_p0.y + 3.0f));

            // Botão Tab: MINI PIANO ROLL
            bool is_midi_mode = (current_mode == DetailDockMode::MINI_PIANO_ROLL);
            if (is_midi_mode) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.75f, 0.90f, 0.95f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.17f, 0.24f, 0.85f));
            if (ImGui::Button("🎹 MINI PIANO ROLL", ImVec2(145, 22))) {
                current_mode = DetailDockMode::MINI_PIANO_ROLL;
            }
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 4.0f);

            // Botão Tab: AUDIO CLIP
            bool is_audio_mode = (current_mode == DetailDockMode::AUDIO_CLIP);
            if (is_audio_mode) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.55f, 0.10f, 0.95f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.17f, 0.24f, 0.85f));
            if (ImGui::Button("🌊 AUDIO / WARP", ImVec2(130, 22))) {
                current_mode = DetailDockMode::AUDIO_CLIP;
                if (active_clip_idx >= 0) g_audio_clip_editor_ui.open(active_track_idx, active_clip_idx);
            }
            ImGui::PopStyleColor();

            // Identificação da Trilha e Clipe
            ImGui::SameLine(0, 14.0f);
            std::string trk_name = (!::track_names[active_track_idx].empty()) ? ::track_names[active_track_idx] : ("Track " + std::to_string(active_track_idx + 1));
            ImGui::TextColored(ImVec4(0.40f, 0.85f, 1.0f, 1.0f), "Track %02d: %s", active_track_idx + 1, trk_name.c_str());

            // Seletor de Snap
            ImGui::SameLine(0, 14.0f);
            ImGui::TextDisabled("Snap:");
            ImGui::SameLine(0, 4.0f);
            const char* snap_names[4] = { "1/16", "1/8", "1/4", "1/32" };
            ImGui::SetNextItemWidth(65);
            ImGui::Combo("##MiniSnapCombo", &snap_mode, snap_names, 4);

            // Controles de Altura / Oitava
            ImGui::SameLine(0, 10.0f);
            if (ImGui::Button("▼ Oct-", ImVec2(52, 22))) {
                base_pitch = std::clamp(base_pitch - 12, 0, 127 - total_visible_keys);
            }
            ImGui::SameLine(0, 3.0f);
            char oct_buf[24];
            int p_high = base_pitch + total_visible_keys - 1;
            snprintf(oct_buf, sizeof(oct_buf), "%s%d..%s%d", note_names[base_pitch % 12], base_pitch / 12, note_names[p_high % 12], p_high / 12);
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "%s", oct_buf);
            ImGui::SameLine(0, 3.0f);
            if (ImGui::Button("▲ Oct+", ImVec2(52, 22))) {
                base_pitch = std::clamp(base_pitch + 12, 0, 127 - total_visible_keys);
            }

            // Ações Rápidas: Quantize, Transpose, Clear
            ImGui::SameLine(0, 10.0f);
            if (ImGui::Button("⚡ Quantize", ImVec2(80, 22))) {
                Pattern* p = getActivePattern(clip_mgr);
                if (p) {
                    clip_mgr.pushUndo();
                    float beat_sec = 60.0f / (timeline.getBPM() > 20.0f ? timeline.getBPM() : 140.0f);
                    float step_sec = beat_sec * 0.25f;
                    if (snap_mode == 1) step_sec = beat_sec * 0.5f;
                    if (snap_mode == 2) step_sec = beat_sec * 1.0f;
                    if (snap_mode == 3) step_sec = beat_sec * 0.125f;

                    for (auto& n : p->getChannelNotes(active_track_idx)) {
                        n.start_time = std::round(n.start_time / step_sec) * step_sec;
                    }
                }
            }

            ImGui::SameLine(0, 4.0f);
            if (ImGui::Button("+1st", ImVec2(36, 22))) {
                Pattern* p = getActivePattern(clip_mgr);
                if (p) {
                    clip_mgr.pushUndo();
                    for (auto& n : p->getChannelNotes(active_track_idx)) if (n.pitch < 127) n.pitch++;
                }
            }
            ImGui::SameLine(0, 2.0f);
            if (ImGui::Button("-1st", ImVec2(36, 22))) {
                Pattern* p = getActivePattern(clip_mgr);
                if (p) {
                    clip_mgr.pushUndo();
                    for (auto& n : p->getChannelNotes(active_track_idx)) if (n.pitch > 0) n.pitch--;
                }
            }

            ImGui::SameLine(0, 4.0f);
            if (ImGui::Button("🧹 Clear", ImVec2(55, 22))) {
                Pattern* p = getActivePattern(clip_mgr);
                if (p) {
                    clip_mgr.pushUndo();
                    p->getChannelNotes(active_track_idx).clear();
                }
            }

            // Botão Expandir para Janela Flutuante do Piano Roll
            ImGui::SameLine(0, 10.0f);
            if (ImGui::Button("↗ EXPAND", ImVec2(78, 22))) {
                selected_track_idx = active_track_idx;
                show_piano_roll = true;
            }

            // Botão Fechar Dock no canto direito
            float close_btn_w = 26.0f;
            ImGui::SameLine(size.x - close_btn_w - 8.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.10f, 0.15f, 0.85f));
            if (ImGui::Button("✕", ImVec2(close_btn_w, 22))) {
                close();
            }
            ImGui::PopStyleColor();

            // ─────────────────────────────────────────────────────────────────
            // 2. CORPO DO DOCK: AUDIO CLIP OU MINI PIANO ROLL
            // ─────────────────────────────────────────────────────────────────
            ImVec2 body_p0 = ImVec2(dock_p0.x + 6.0f, dock_p0.y + header_h + 4.0f);
            ImVec2 body_sz = ImVec2(size.x - 12.0f, size.y - header_h - 8.0f);

            if (current_mode == DetailDockMode::AUDIO_CLIP) {
                if (active_clip_idx >= 0 && active_clip_idx < (int)clip_mgr.track_clips[active_track_idx].size()) {
                    g_audio_clip_editor_ui.RenderDetailDock(clip_mgr, body_p0, body_sz);
                } else {
                    draw->AddRectFilled(body_p0, ImVec2(body_p0.x + body_sz.x, body_p0.y + body_sz.y), IM_COL32(11, 16, 24, 255), 4.0f);
                    draw->AddText(ImVec2(body_p0.x + 24.0f, body_p0.y + 35.0f), IM_COL32(0, 229, 255, 255), "🌊 AUDIO CLIP EDITOR");
                    draw->AddText(ImVec2(body_p0.x + 24.0f, body_p0.y + 60.0f), IM_COL32(180, 205, 230, 220), 
                        "Nenhum clipe de áudio selecionado nesta faixa.\nArraste um sample .wav do Sample Browser para a Playlist para usar Warp, Pitch e Envelopes.");
                }
                return;
            }

            // ─────────────────────────────────────────────────────────────────
            // 3. RENDERIZAÇÃO DO MINI PIANO ROLL INLINE (ABLETON LIVE / FL STUDIO STYLE)
            // ─────────────────────────────────────────────────────────────────
            Pattern* pat = getActivePattern(clip_mgr);
            if (!pat) {
                draw->AddText(ImVec2(body_p0.x + 20.0f, body_p0.y + 20.0f), IM_COL32(180, 200, 220, 255), "Nenhum Padrão MIDI Ativo. Crie ou selecione um clipe na Playlist.");
                return;
            }

            float bpm = timeline.getBPM();
            if (bpm <= 10.0f) bpm = 140.0f;
            float beat_sec = 60.0f / bpm;
            float bar_len_sec = beat_sec * 4.0f;
            float step_sec = beat_sec * 0.25f; // 1/16th

            if (snap_mode == 1) step_sec = beat_sec * 0.5f;
            else if (snap_mode == 2) step_sec = beat_sec * 1.0f;
            else if (snap_mode == 3) step_sec = beat_sec * 0.125f;

            auto& notes = pat->getChannelNotes(active_track_idx);

            // Comprimento do padrão visualizado (pelo menos 1 compasso)
            float max_n_end = bar_len_sec;
            for (const auto& n : notes) {
                if (n.start_time + n.duration > max_n_end) max_n_end = n.start_time + n.duration;
            }
            int num_bars = (int)std::ceil(max_n_end / bar_len_sec);
            if (num_bars < 1) num_bars = 1;
            if (num_bars > 8) num_bars = 8;
            float view_total_time = num_bars * bar_len_sec;
            int total_steps = num_bars * 16;

            float keybed_w = 48.0f;
            float ruler_h = 16.0f;
            float vel_h = 24.0f;
            float grid_y0 = body_p0.y + ruler_h;
            float grid_h = body_sz.y - ruler_h - vel_h;
            float grid_x0 = body_p0.x + keybed_w;
            float grid_w = body_sz.x - keybed_w - 4.0f;

            float row_h = grid_h / (float)total_visible_keys;

            // ── RÉGUA DE COMPASSO SUPERIOR (1.1, 1.2, 1.3, 1.4, 2.1...) ──────
            draw->AddRectFilled(ImVec2(grid_x0, body_p0.y), ImVec2(grid_x0 + grid_w, body_p0.y + ruler_h), IM_COL32(11, 16, 24, 255));
            draw->AddLine(ImVec2(grid_x0, body_p0.y + ruler_h), ImVec2(grid_x0 + grid_w, body_p0.y + ruler_h), IM_COL32(28, 42, 60, 255));

            for (int s = 0; s <= total_steps; s++) {
                float sx = grid_x0 + ((float)s / (float)total_steps) * grid_w;
                if (s % 16 == 0) {
                    int bar_num = (s / 16) + 1;
                    char bar_str[16];
                    snprintf(bar_str, sizeof(bar_str), "%d", bar_num);
                    draw->AddText(ImVec2(sx + 3.0f, body_p0.y + 1.0f), IM_COL32(0, 229, 255, 230), bar_str);
                    draw->AddLine(ImVec2(sx, body_p0.y), ImVec2(sx, body_p0.y + ruler_h), IM_COL32(0, 229, 255, 120), 1.2f);
                } else if (s % 4 == 0) {
                    draw->AddLine(ImVec2(sx, body_p0.y + ruler_h - 4.0f), ImVec2(sx, body_p0.y + ruler_h), IM_COL32(140, 160, 180, 120));
                }
            }

            // ── TECLADO DO PIANO (KEYBED À ESQUERDA) ──────────────────────────
            ImVec2 kb_p0 = ImVec2(body_p0.x, grid_y0);
            draw->AddRectFilled(kb_p0, ImVec2(kb_p0.x + keybed_w, kb_p0.y + grid_h), IM_COL32(10, 14, 20, 255));
            draw->AddLine(ImVec2(kb_p0.x + keybed_w, kb_p0.y), ImVec2(kb_p0.x + keybed_w, kb_p0.y + grid_h), IM_COL32(32, 48, 68, 255));

            int hovered_key_pitch = -1;
            bool is_keybed_hovered = (io.MousePos.x >= kb_p0.x && io.MousePos.x <= kb_p0.x + keybed_w &&
                                      io.MousePos.y >= grid_y0 && io.MousePos.y <= grid_y0 + grid_h);

            for (int r = 0; r < total_visible_keys; r++) {
                int pitch = base_pitch + (total_visible_keys - 1 - r);
                int note_idx = pitch % 12;
                bool is_black = (note_idx == 1 || note_idx == 3 || note_idx == 6 || note_idx == 8 || note_idx == 10);

                float ky0 = grid_y0 + r * row_h;
                float ky1 = ky0 + row_h;

                ImVec2 key_min = ImVec2(kb_p0.x, ky0);
                ImVec2 key_max = ImVec2(kb_p0.x + keybed_w, ky1 - 1.0f);

                bool key_hovered = io.MousePos.x >= key_min.x && io.MousePos.x <= key_max.x && io.MousePos.y >= key_min.y && io.MousePos.y <= key_max.y;
                if (key_hovered) hovered_key_pitch = pitch;

                ImU32 key_col = is_black ? IM_COL32(20, 25, 34, 255) : IM_COL32(180, 195, 210, 240);
                if (key_hovered) key_col = IM_COL32(0, 220, 255, 230);

                draw->AddRectFilled(key_min, key_max, key_col, 1.5f);

                // Mostra o nome da nota (ex: C2, D#2, F2)
                char c_lbl[12];
                snprintf(c_lbl, sizeof(c_lbl), "%s%d", note_names[note_idx], pitch / 12);
                ImU32 txt_col = is_black ? IM_COL32(255, 255, 255, 230) : IM_COL32(10, 15, 22, 255);
                if (note_idx == 0) txt_col = is_black ? IM_COL32(0, 229, 255, 255) : IM_COL32(0, 140, 180, 255);
                draw->AddText(ImVec2(key_min.x + 3.0f, ky0 + (row_h - 13.0f) * 0.5f), txt_col, c_lbl);

                // Linha de Grade Horizontal de Fundo
                ImU32 lane_col = is_black ? IM_COL32(10, 14, 20, 255) : IM_COL32(14, 19, 28, 255);
                draw->AddRectFilled(ImVec2(grid_x0, ky0), ImVec2(grid_x0 + grid_w, ky1), lane_col);
                draw->AddLine(ImVec2(grid_x0, ky1), ImVec2(grid_x0 + grid_w, ky1), IM_COL32(22, 32, 45, 120));
            }

            // Clique no teclado para auditar a nota
            if (hovered_key_pitch >= 0 && io.MouseClicked[0] && io.MousePos.x < grid_x0) {
                auditNote(hovered_key_pitch, 0.25f, 0.90f);
            }

            // Scroll vertical no keybed ou grid para mudar a oitava suavemente
            bool is_grid_hovered = (io.MousePos.x >= grid_x0 && io.MousePos.x <= grid_x0 + grid_w &&
                                    io.MousePos.y >= grid_y0 && io.MousePos.y <= grid_y0 + grid_h);

            if (io.MouseWheel != 0.0f && (is_grid_hovered || is_keybed_hovered)) {
                base_pitch = std::clamp(base_pitch - (int)io.MouseWheel, 0, 127 - total_visible_keys);
            }

            // ── LINHAS DE GRADE VERTICAIS DE TEMPO (PASSOS / COMPASSOS) ───────
            for (int s = 0; s <= total_steps; s++) {
                float sx = grid_x0 + ((float)s / (float)total_steps) * grid_w;
                if (s % 16 == 0) {
                    draw->AddLine(ImVec2(sx, grid_y0), ImVec2(sx, grid_y0 + grid_h), IM_COL32(0, 229, 255, 80), 1.2f);
                } else if (s % 4 == 0) {
                    draw->AddLine(ImVec2(sx, grid_y0), ImVec2(sx, grid_y0 + grid_h), IM_COL32(255, 255, 255, 25), 1.0f);
                } else {
                    draw->AddLine(ImVec2(sx, grid_y0), ImVec2(sx, grid_y0 + grid_h), IM_COL32(255, 255, 255, 10), 1.0f);
                }
            }

            int hovered_note_idx = -1;
            bool hovering_resize_edge = false;

            // ── DESENHAR NOTAS MIDI EXISTENTES NO GRID ─────────────────────────
            for (int i = 0; i < (int)notes.size(); ++i) {
                auto& n = notes[i];
                if (n.pitch < base_pitch || n.pitch >= base_pitch + total_visible_keys) continue;

                int row = (total_visible_keys - 1) - (n.pitch - base_pitch);
                float ny0 = grid_y0 + row * row_h + 1.0f;
                float ny1 = ny0 + row_h - 2.0f;

                float nx0 = grid_x0 + (n.start_time / view_total_time) * grid_w;
                float nx1 = nx0 + (n.duration / view_total_time) * grid_w;
                if (nx1 - nx0 < 6.0f) nx1 = nx0 + 6.0f;

                bool n_hovered = is_grid_hovered && (io.MousePos.x >= nx0 && io.MousePos.x <= nx1 && io.MousePos.y >= ny0 && io.MousePos.y <= ny1);
                if (n_hovered) {
                    hovered_note_idx = i;
                    if (io.MousePos.x >= nx1 - 6.0f) hovering_resize_edge = true;
                }

                // Cores de Neon Ciano de Alta Definição
                ImU32 aura_col = n_hovered ? IM_COL32(0, 240, 255, 100) : IM_COL32(0, 220, 255, 65);
                ImU32 body_col = n_hovered ? IM_COL32(40, 250, 255, 255) : IM_COL32(0, 220, 255, 240);
                ImU32 border_col = n_hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(200, 248, 255, 230);

                // Glow Aura
                draw->AddRectFilled(ImVec2(nx0 - 1.0f, ny0 - 1.0f), ImVec2(nx1 + 1.0f, ny1 + 1.0f), aura_col, 3.0f);
                // Corpo da Nota
                draw->AddRectFilled(ImVec2(nx0, ny0), ImVec2(nx1, ny1), body_col, 2.5f);
                // Núcleo iluminado
                draw->AddLine(ImVec2(nx0 + 1.0f, (ny0 + ny1) * 0.5f), ImVec2(nx1 - 1.0f, (ny0 + ny1) * 0.5f), IM_COL32(255, 255, 255, 220), 1.0f);
                // Borda
                draw->AddRect(ImVec2(nx0, ny0), ImVec2(nx1, ny1), border_col, 2.5f, 0, 1.2f);

                // Rótulo da nota dentro do bloco se houver espaço
                if (nx1 - nx0 > 18.0f) {
                    char n_lbl[8];
                    snprintf(n_lbl, sizeof(n_lbl), "%s%d", note_names[n.pitch % 12], n.pitch / 12);
                    draw->AddText(ImVec2(nx0 + 3.0f, ny0 + 0.5f), IM_COL32(5, 25, 35, 255), n_lbl);
                }
            }

            // Cursor de redimensionamento de nota na borda direita
            if (hovering_resize_edge || is_resizing_note) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }

            // Tooltip da nota sobrevoada
            if (hovered_note_idx >= 0 && hovered_note_idx < (int)notes.size()) {
                auto& hn = notes[hovered_note_idx];
                ImGui::SetTooltip("Nota: %s%d (Pitch: %d) | Início: %.2fs | Dur: %.2fs | Vel: %.0f%%", 
                    note_names[hn.pitch % 12], hn.pitch / 12, hn.pitch, hn.start_time, hn.duration, hn.velocity * 100.0f);
            }

            // ── CLIQUES E ARRASTE DE NOTAS NO GRID ────────────────────────────
            if (is_grid_hovered) {
                // 1. Botão Direito: Apagar Nota Imediatamente
                if (io.MouseClicked[1] && hovered_note_idx >= 0) {
                    clip_mgr.pushUndo();
                    notes.erase(notes.begin() + hovered_note_idx);
                    hovered_note_idx = -1;
                }
                // 2. Botão Esquerdo em Nota Existente: Iniciar Arraste ou Redimensionamento
                else if (io.MouseClicked[0] && hovered_note_idx >= 0) {
                    dragging_note_idx = hovered_note_idx;
                    is_resizing_note = hovering_resize_edge;
                    orig_note_start = notes[hovered_note_idx].start_time;
                    orig_note_dur = notes[hovered_note_idx].duration;
                    orig_note_pitch = notes[hovered_note_idx].pitch;
                    drag_start_mouse = io.MousePos;
                    auditNote(orig_note_pitch, orig_note_dur, notes[hovered_note_idx].velocity);
                }
                // 3. Botão Esquerdo em Espaço Vazio: Criar Nova Nota com Snap
                else if (io.MouseClicked[0] && hovered_note_idx == -1) {
                    float norm_x = (io.MousePos.x - grid_x0) / grid_w;
                    float click_time = norm_x * view_total_time;
                    float snapped_time = std::floor(click_time / step_sec) * step_sec;
                    if (snapped_time < 0.0f) snapped_time = 0.0f;

                    int click_row = std::clamp((int)((io.MousePos.y - grid_y0) / row_h), 0, total_visible_keys - 1);
                    int click_pitch = base_pitch + (total_visible_keys - 1 - click_row);

                    clip_mgr.pushUndo();
                    notes.push_back(KuroDSP::MidiNote(click_pitch, snapped_time, step_sec * 0.85f, 0.90f, 1.0f, active_track_idx));
                    dragging_note_idx = (int)notes.size() - 1;
                    is_resizing_note = false;
                    orig_note_start = snapped_time;
                    orig_note_dur = step_sec * 0.85f;
                    orig_note_pitch = click_pitch;
                    drag_start_mouse = io.MousePos;
                    auditNote(click_pitch, step_sec * 0.85f, 0.90f);
                }
            }

            // Processamento do Arraste de Nota em Andamento
            if (dragging_note_idx >= 0 && dragging_note_idx < (int)notes.size() && io.MouseDown[0]) {
                auto& dn = notes[dragging_note_idx];
                float dx = io.MousePos.x - drag_start_mouse.x;
                float dy = io.MousePos.y - drag_start_mouse.y;

                if (is_resizing_note) {
                    float dt = (dx / grid_w) * view_total_time;
                    float new_dur = std::max(step_sec * 0.25f, orig_note_dur + dt);
                    dn.duration = std::round(new_dur / (step_sec * 0.5f)) * (step_sec * 0.5f);
                    if (dn.duration < step_sec * 0.25f) dn.duration = step_sec * 0.25f;
                } else {
                    float dt = (dx / grid_w) * view_total_time;
                    float new_st = std::max(0.0f, orig_note_start + dt);
                    dn.start_time = std::round(new_st / step_sec) * step_sec;

                    int pitch_shift = -(int)std::round(dy / row_h);
                    int new_p = std::clamp(orig_note_pitch + pitch_shift, 0, 127);
                    if (new_p != dn.pitch) {
                        dn.pitch = new_p;
                        auditNote(new_p, dn.duration, dn.velocity);
                    }
                }
            }

            if (io.MouseReleased[0]) {
                dragging_note_idx = -1;
                is_resizing_note = false;
            }

            // ── 4. VELOCITY LANE INFERIOR (RÉGUA DE VELOCIDADE DAS NOTAS) ─────
            float vel_y0 = grid_y0 + grid_h + 2.0f;
            draw->AddRectFilled(ImVec2(body_p0.x, vel_y0), ImVec2(body_p0.x + body_sz.x, vel_y0 + vel_h), IM_COL32(8, 12, 18, 255));
            draw->AddLine(ImVec2(body_p0.x, vel_y0), ImVec2(body_p0.x + body_sz.x, vel_y0), IM_COL32(28, 40, 56, 255));
            draw->AddText(ImVec2(body_p0.x + 6.0f, vel_y0 + 4.0f), IM_COL32(110, 130, 160, 220), "VEL");

            bool is_vel_hovered = (io.MousePos.x >= grid_x0 && io.MousePos.x <= grid_x0 + grid_w &&
                                   io.MousePos.y >= vel_y0 && io.MousePos.y <= vel_y0 + vel_h);

            for (size_t i = 0; i < notes.size(); ++i) {
                auto& n = notes[i];
                float nx_mid = grid_x0 + ((n.start_time + n.duration * 0.5f) / view_total_time) * grid_w;
                float v_bar_h = std::clamp(n.velocity, 0.05f, 1.0f) * (vel_h - 4.0f);
                float vy_bot = vel_y0 + vel_h - 2.0f;
                float vy_top = vy_bot - v_bar_h;

                bool n_vel_hovered = is_vel_hovered && std::abs(io.MousePos.x - nx_mid) < 8.0f;
                if (n_vel_hovered && io.MouseDown[0]) {
                    float new_v = (vy_bot - io.MousePos.y) / (vel_h - 4.0f);
                    n.velocity = std::clamp(new_v, 0.05f, 1.0f);
                }

                ImU32 v_col = n_vel_hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 229, 255, 220);
                draw->AddLine(ImVec2(nx_mid, vy_bot), ImVec2(nx_mid, vy_top), v_col, 2.0f);
                draw->AddCircleFilled(ImVec2(nx_mid, vy_top), 2.5f, v_col);
            }

            // ── 5. LASER PLAYHEAD EM TEMPO REAL ───────────────────────────────
            if (::is_playing) {
                float play_sec = (float)timeline.getMasterFrame() / 44100.0f;
                float rel_play_sec = std::fmod(play_sec, view_total_time);
                float pl_x = grid_x0 + (rel_play_sec / view_total_time) * grid_w;

                if (pl_x >= grid_x0 && pl_x <= grid_x0 + grid_w) {
                    draw->AddLine(ImVec2(pl_x, body_p0.y), ImVec2(pl_x, vel_y0 + vel_h), IM_COL32(255, 255, 255, 75), 3.0f);
                    draw->AddLine(ImVec2(pl_x, body_p0.y), ImVec2(pl_x, vel_y0 + vel_h), IM_COL32(255, 255, 255, 255), 1.2f);
                    draw->AddTriangleFilled(ImVec2(pl_x - 4.0f, body_p0.y), ImVec2(pl_x + 4.0f, body_p0.y), ImVec2(pl_x, body_p0.y + 6.0f), IM_COL32(255, 255, 255, 255));
                }
            }
        }
    };

    inline ClipDetailDockUI g_clip_detail_dock_ui;

} // namespace KuroUI
