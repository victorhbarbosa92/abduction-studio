#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "imgui.h"
#include "../core/ClipManager.h"

namespace KuroUI {

    class AudioClipEditorUI {
    public:
        bool is_open = false;
        int active_track_idx = -1;
        int active_clip_idx = -1;

        void open(int track_idx, int clip_idx) {
            active_track_idx = track_idx;
            active_clip_idx = clip_idx;
            is_open = true;
        }

        void close() {
            is_open = false;
            active_track_idx = -1;
            active_clip_idx = -1;
        }

        bool show_as_popup = false;

        void Render(ClipManager& clip_mgr) {
            if (!is_open || !show_as_popup || active_track_idx < 0 || active_track_idx >= MAX_TRACKS) return;
            RenderPopup(clip_mgr);
        }

        void RenderPopup(ClipManager& clip_mgr) {
            if (!is_open || !show_as_popup || active_track_idx < 0 || active_track_idx >= MAX_TRACKS) return;

            std::lock_guard<std::mutex> lock(clip_mgr.clip_mutex);
            if (active_clip_idx < 0 || active_clip_idx >= (int)clip_mgr.track_clips[active_track_idx].size()) {
                is_open = false;
                return;
            }

            AudioClip& clip = clip_mgr.track_clips[active_track_idx][active_clip_idx];

            ImGui::SetNextWindowSize(ImVec2(520, 440), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.11f, 0.16f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.70f, 1.00f, 0.60f));

            char win_title[128];
            snprintf(win_title, sizeof(win_title), "AUDIO CLIP SETTINGS - %s###AudioClipModal", clip.name.c_str());

            if (ImGui::Begin(win_title, &is_open, ImGuiWindowFlags_NoCollapse)) {
                ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "CLIP PROPERTIES (ABLETON STYLE)");
                ImGui::Separator();
                ImGui::Spacing();

                char name_buf[64];
                strncpy(name_buf, clip.name.c_str(), sizeof(name_buf));
                name_buf[sizeof(name_buf) - 1] = '\0';
                ImGui::SetNextItemWidth(260);
                if (ImGui::InputText("Nome do Clipe", name_buf, sizeof(name_buf))) {
                    clip.name = name_buf;
                }

                ImGui::SameLine(0, 15);
                if (ImGui::Button("Make Unique")) {
                    clip_mgr.makeClipUnique(active_track_idx, active_clip_idx, false);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(ImVec4(1.00f, 0.65f, 0.10f, 1.0f), "WARP & TIME STRETCH");
                ImGui::Checkbox("Enable Warp", &clip.enable_stretch);
                if (clip.enable_stretch) {
                    ImGui::SameLine(0, 15);
                    ImGui::SetNextItemWidth(140);
                    ImGui::SliderFloat("Ratio", &clip.time_stretch_ratio, 0.25f, 4.0f, "%.2fx");
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.40f, 0.90f, 0.60f, 1.0f), "PITCH, GAIN & ENVELOPES");
                ImGui::SetNextItemWidth(180);
                ImGui::SliderFloat("Pitch (Semitons)", &clip.pitch_shift_semitones, -24.0f, +24.0f, "%+.1f st");
                ImGui::SameLine(0, 10);
                if (ImGui::Button("Reset Pitch")) clip.pitch_shift_semitones = 0.0f;

                ImGui::SetNextItemWidth(180);
                ImGui::SliderFloat("Ganho (dB)", &clip.gain_db, -24.0f, +12.0f, "%.1f dB");
                ImGui::SameLine(0, 15);
                bool rev = clip.is_reversed;
                if (rev) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.6f, 0.9f));
                if (ImGui::Button(rev ? "REVERSED" : "REVERSE", ImVec2(90, 24))) {
                    clip.is_reversed = !clip.is_reversed;
                }
                if (rev) ImGui::PopStyleColor();

                ImGui::SameLine(0, 10);
                ImGui::Checkbox("Norm", &clip.is_normalized);

                ImGui::Spacing();
                if (ImGui::Button("OK / Salvar", ImVec2(120, 24))) {
                    is_open = false;
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }

        // ── GAVETA INFERIOR FIXA DE DETALHES (ABLETON LIVE DETAIL DOCK STYLE) ──
        void RenderDetailDock(ClipManager& clip_mgr, ImVec2 pos, ImVec2 size) {
            if (!is_open || active_track_idx < 0 || active_track_idx >= MAX_TRACKS) return;

            std::lock_guard<std::mutex> lock(clip_mgr.clip_mutex);
            if (active_clip_idx < 0 || active_clip_idx >= (int)clip_mgr.track_clips[active_track_idx].size()) {
                is_open = false;
                return;
            }

            AudioClip& clip = clip_mgr.track_clips[active_track_idx][active_clip_idx];

            ImDrawList* draw = ImGui::GetWindowDrawList();
            
            // Fundo do Dock: Alumínio grafite escovado com borda neon superior
            draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(11, 15, 22, 255), 6.0f);
            draw->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 229, 255, 140), 6.0f, 0, 1.5f);
            draw->AddLine(pos, ImVec2(pos.x + size.x, pos.y), IM_COL32(0, 229, 255, 230), 2.0f); // Top Glow Line

            // Cabeçalho e Painel de Controles
            float left_w = (std::min)(360.0f, size.x * 0.40f);
            ImGui::SetCursorScreenPos(ImVec2(pos.x + 12.0f, pos.y + 8.0f));
            ImGui::BeginGroup();

            // 1. Linha Superior: Nome do Clipe + Botão Fechar Dock
            ImGui::TextColored(ImVec4(0.0f, 0.95f, 1.0f, 1.0f), "🎛️ CLIP DETAIL DOCK: %s", clip.name.c_str());
            ImGui::SameLine(left_w - 65.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.1f, 0.15f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            if (ImGui::Button("✖ Fechar", ImVec2(60, 20))) {
                is_open = false;
            }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Recolher painel de detalhes do clipe");

            ImGui::TextDisabled("Faixa %d | Duracao: %.2fs | Start: %.2fs", active_track_idx + 1, clip.length_sec, clip.start_time_sec);
            ImGui::Spacing();

            // 2. SEÇÃO DE WARP / TIME-STRETCH (ESTILO ABLETON LIVE)
            bool is_warp = clip.enable_stretch;
            if (is_warp) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.55f, 0.0f, 0.95f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.28f, 0.9f));
            if (ImGui::Button(is_warp ? "WARP [ON]" : "WARP [OFF]", ImVec2(80, 22))) {
                clip.enable_stretch = !clip.enable_stretch;
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ativa/desativa esticamento temporal elástico (Warping)");

            ImGui::SameLine(0, 6);
            ImGui::SetNextItemWidth(86);
            if (clip.enable_stretch) {
                ImGui::SliderFloat("##Ratio", &clip.time_stretch_ratio, 0.25f, 4.0f, "%.2fx");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Velocidade do Warp (0.5x = metade, 2.0x = dobro)");
            } else {
                float def_r = 1.0f;
                ImGui::BeginDisabled();
                ImGui::SliderFloat("##RatioDis", &def_r, 0.25f, 4.0f, "1.00x");
                ImGui::EndDisabled();
            }

            ImGui::SameLine(0, 4);
            if (ImGui::Button("/2", ImVec2(24, 22))) { clip.time_stretch_ratio = std::clamp(clip.time_stretch_ratio * 0.5f, 0.25f, 4.0f); clip.enable_stretch = true; }
            ImGui::SameLine(0, 2);
            if (ImGui::Button("1x", ImVec2(24, 22))) { clip.time_stretch_ratio = 1.0f; }
            ImGui::SameLine(0, 2);
            if (ImGui::Button("*2", ImVec2(24, 22))) { clip.time_stretch_ratio = std::clamp(clip.time_stretch_ratio * 2.0f, 0.25f, 4.0f); clip.enable_stretch = true; }

            ImGui::Spacing();

            // 3. SEÇÃO DE PITCH SHIFT & TRANSPOSIÇÃO
            ImGui::SetNextItemWidth(110);
            ImGui::SliderFloat("Pitch##Dock", &clip.pitch_shift_semitones, -24.0f, 24.0f, "%+.1f st");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ajuste de afinação em semitons (-24 a +24)");

            ImGui::SameLine(0, 4);
            if (ImGui::Button("-12", ImVec2(28, 20))) clip.pitch_shift_semitones = std::clamp(clip.pitch_shift_semitones - 12.0f, -24.0f, 24.0f);
            ImGui::SameLine(0, 2);
            if (ImGui::Button("0", ImVec2(22, 20))) clip.pitch_shift_semitones = 0.0f;
            ImGui::SameLine(0, 2);
            if (ImGui::Button("+12", ImVec2(28, 20))) clip.pitch_shift_semitones = std::clamp(clip.pitch_shift_semitones + 12.0f, -24.0f, 24.0f);

            ImGui::Spacing();

            // 4. SEÇÃO DE GANHO, REVERSE E NORMALIZAÇÃO
            ImGui::SetNextItemWidth(110);
            ImGui::SliderFloat("Gain##Dock", &clip.gain_db, -24.0f, 12.0f, "%.1f dB");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ganho do clipe em decibéis");

            ImGui::SameLine(0, 8);
            bool rev = clip.is_reversed;
            if (rev) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.15f, 0.55f, 0.95f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.28f, 0.9f));
            if (ImGui::Button(rev ? "REVERSED" : "REVERSE", ImVec2(76, 20))) {
                clip.is_reversed = !clip.is_reversed;
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Inverte o áudio da amostra (Backwards)");

            ImGui::SameLine(0, 6);
            ImGui::Checkbox("Norm", &clip.is_normalized);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Normalizar para 0 dB Peak");

            // 5. SLIDERS COMPACTOS DE FADE IN / OUT
            ImGui::SetNextItemWidth(90);
            ImGui::SliderFloat("Fade In", &clip.fade_in_sec, 0.0f, (std::min)(2.0f, clip.length_sec * 0.5f), "%.2fs");
            ImGui::SameLine(0, 10);
            ImGui::SetNextItemWidth(90);
            ImGui::SliderFloat("Fade Out", &clip.fade_out_sec, 0.0f, (std::min)(2.0f, clip.length_sec * 0.5f), "%.2fs");

            ImGui::EndGroup();

            // ─────────────────────────────────────────────────────────────────
            // 2. PAINEL CENTRAL/DIREITO: FORMA DE ONDA DE ALTA RESOLUÇÃO (ABLETON STYLE)
            // ─────────────────────────────────────────────────────────────────
            float wave_x0 = pos.x + left_w + 14.0f;
            float wave_w = size.x - left_w - 24.0f;
            float wave_y0 = pos.y + 24.0f;
            float wave_h = size.y - 32.0f;

            if (wave_w > 60.0f && wave_h > 20.0f) {
                // Caixa da Waveform
                draw->AddRectFilled(ImVec2(wave_x0, wave_y0), ImVec2(wave_x0 + wave_w, wave_y0 + wave_h), IM_COL32(5, 8, 14, 255), 5.0f);
                draw->AddRect(ImVec2(wave_x0, wave_y0), ImVec2(wave_x0 + wave_w, wave_y0 + wave_h), IM_COL32(18, 32, 48, 255), 5.0f);

                // Régua superior de tempo do clipe (0.0s ... length_sec)
                float ruler_h = 16.0f;
                draw->AddRectFilled(ImVec2(wave_x0, wave_y0), ImVec2(wave_x0 + wave_w, wave_y0 + ruler_h), IM_COL32(9, 14, 22, 255), 5.0f);
                draw->AddLine(ImVec2(wave_x0, wave_y0 + ruler_h), ImVec2(wave_x0 + wave_w, wave_y0 + ruler_h), IM_COL32(25, 45, 68, 200));

                int num_ticks = 8;
                for (int tk = 0; tk <= num_ticks; ++tk) {
                    float tk_frac = (float)tk / (float)num_ticks;
                    float tk_x = wave_x0 + tk_frac * wave_w;
                    float tk_time = tk_frac * clip.length_sec;
                    char tk_str[16];
                    snprintf(tk_str, sizeof(tk_str), "%.2fs", tk_time);
                    draw->AddLine(ImVec2(tk_x, wave_y0 + ruler_h - 4.0f), ImVec2(tk_x, wave_y0 + ruler_h), IM_COL32(0, 180, 230, 150));
                    if (tk < num_ticks) {
                        draw->AddText(ImVec2(tk_x + 3.0f, wave_y0 + 1.0f), IM_COL32(140, 180, 210, 220), tk_str);
                    }
                }

                float wave_draw_y0 = wave_y0 + ruler_h;
                float wave_draw_h = wave_h - ruler_h;
                float mid_y = wave_draw_y0 + wave_draw_h * 0.5f;
                draw->AddLine(ImVec2(wave_x0, mid_y), ImVec2(wave_x0 + wave_w, mid_y), IM_COL32(0, 180, 230, 60));

                // Gradiente de preenchimento e forma de onda
                for (float wx = wave_x0 + 2.0f; wx < wave_x0 + wave_w - 2.0f; wx += 2.0f) {
                    float prog = (wx - wave_x0) / wave_w;
                    if (clip.is_reversed) prog = 1.0f - prog;

                    float amp = (std::sin(prog * 45.0f) * 0.55f + std::cos(prog * 18.0f) * 0.35f + std::sin(prog * 90.0f) * 0.10f) * (wave_draw_h * 0.42f);
                    
                    // Fades
                    if (clip.fade_in_sec > 0.0f && prog * clip.length_sec < clip.fade_in_sec) {
                        amp *= (prog * clip.length_sec) / clip.fade_in_sec;
                    }
                    if (clip.fade_out_sec > 0.0f && (1.0f - prog) * clip.length_sec < clip.fade_out_sec) {
                        amp *= ((1.0f - prog) * clip.length_sec) / clip.fade_out_sec;
                    }

                    // Multiplica pelo ganho dB
                    float lin_gain = std::pow(10.0f, clip.gain_db / 20.0f);
                    amp = std::clamp(amp * lin_gain, -(wave_draw_h * 0.48f), wave_draw_h * 0.48f);

                    ImU32 col_top = clip.enable_stretch ? IM_COL32(255, 150, 20, 230) : IM_COL32(0, 229, 255, 230);
                    draw->AddLine(ImVec2(wx, mid_y - std::abs(amp)), ImVec2(wx, mid_y + std::abs(amp)), col_top, 1.6f);
                }

                // Visualização de Fade In e Fade Out sombreados
                if (clip.fade_in_sec > 0.0f) {
                    float fi_w = (clip.fade_in_sec / clip.length_sec) * wave_w;
                    draw->AddRectFilled(ImVec2(wave_x0, wave_draw_y0), ImVec2(wave_x0 + fi_w, wave_y0 + wave_h), IM_COL32(0, 229, 255, 30));
                    draw->AddLine(ImVec2(wave_x0, wave_y0 + wave_h), ImVec2(wave_x0 + fi_w, wave_draw_y0), IM_COL32(0, 229, 255, 180), 1.5f);
                }
                if (clip.fade_out_sec > 0.0f) {
                    float fo_w = (clip.fade_out_sec / clip.length_sec) * wave_w;
                    draw->AddRectFilled(ImVec2(wave_x0 + wave_w - fo_w, wave_draw_y0), ImVec2(wave_x0 + wave_w, wave_y0 + wave_h), IM_COL32(255, 80, 80, 30));
                    draw->AddLine(ImVec2(wave_x0 + wave_w - fo_w, wave_draw_y0), ImVec2(wave_x0 + wave_w, wave_y0 + wave_h), IM_COL32(255, 80, 80, 180), 1.5f);
                }
            }
        }
    };

    extern AudioClipEditorUI g_audio_clip_editor_ui;

} // namespace KuroUI

using KuroUI::g_audio_clip_editor_ui;

