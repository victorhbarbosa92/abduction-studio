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

        void Render(ClipManager& clip_mgr) {
            if (!is_open || active_track_idx < 0 || active_track_idx >= MAX_TRACKS) return;

            std::lock_guard<std::mutex> lock(clip_mgr.clip_mutex);
            if (active_clip_idx < 0 || active_clip_idx >= (int)clip_mgr.track_clips[active_track_idx].size()) {
                is_open = false;
                return;
            }

            AudioClip& clip = clip_mgr.track_clips[active_track_idx][active_clip_idx];

            ImGui::SetNextWindowSize(ImVec2(500, 420), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.11f, 0.16f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.70f, 1.00f, 0.60f));

            char win_title[128];
            snprintf(win_title, sizeof(win_title), "AUDIO CLIP SETTINGS - %s###AudioClipModal", clip.name.c_str());

            if (ImGui::Begin(win_title, &is_open, ImGuiWindowFlags_NoCollapse)) {
                
                // 1. HEADER DO CLIPE: NOME E PALETA DE CORES
                ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "CLIP PROPERTIES");
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
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clona este clipe tornando-o independente");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // 2. TIME-STRETCH & PITCH SHIFT
                ImGui::TextColored(ImVec4(1.00f, 0.75f, 0.20f, 1.0f), "TIME STRETCHING & PITCH (ELASTIC DSP)");
                
                ImGui::SetNextItemWidth(180);
                ImGui::SliderFloat("Pitch Shift (Semitons)", &clip.pitch_shift_semitones, -12.0f, +12.0f, "%.1f st");
                
                ImGui::SameLine(0, 20);
                ImGui::Checkbox("Enable Time-Stretch", &clip.enable_stretch);

                if (clip.enable_stretch) {
                    ImGui::SetNextItemWidth(180);
                    ImGui::SliderFloat("Time Stretch Ratio", &clip.time_stretch_ratio, 0.5f, 2.0f, "%.2fx");
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // 3. OPÇÕES DE ÁUDIO & ENVELOPE (REVERSE, NORMALIZE, FADES)
                ImGui::TextColored(ImVec4(0.40f, 0.90f, 0.60f, 1.0f), "PLAYBACK OPTIONS & ENVELOPES");

                // Botão de Inversão de Áudio
                bool rev = clip.is_reversed;
                if (rev) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00f, 0.30f, 0.50f, 0.90f));
                }
                if (ImGui::Button("REVERSE AUDIO", ImVec2(140, 24))) {
                    clip.is_reversed = !clip.is_reversed;
                }
                if (rev) ImGui::PopStyleColor(1);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Inverte a reproducao do audio (ideal para risers e sweeps)");

                ImGui::SameLine(0, 15);
                ImGui::Checkbox("Normalizar (0 dB Peak)", &clip.is_normalized);

                ImGui::Spacing();

                // Sliders de Fades
                ImGui::SetNextItemWidth(180);
                ImGui::SliderFloat("Fade In (sec)", &clip.fade_in_sec, 0.0f, (std::min)(4.0f, clip.length_sec * 0.5f), "%.2fs");

                ImGui::SameLine(0, 20);
                ImGui::SetNextItemWidth(180);
                ImGui::SliderFloat("Fade Out (sec)", &clip.fade_out_sec, 0.0f, (std::min)(4.0f, clip.length_sec * 0.5f), "%.2fs");

                ImGui::SetNextItemWidth(180);
                ImGui::SliderFloat("Ganho (dB)", &clip.gain_db, -24.0f, +12.0f, "%.1f dB");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // 4. DISPLAY DE FORMA DE ONDA GRÁFICA PREVIEW
                ImDrawList* draw = ImGui::GetWindowDrawList();
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImVec2 sz(ImGui::GetContentRegionAvail().x, 60.0f);

                draw->AddRectFilled(p0, ImVec2(p0.x + sz.x, p0.y + sz.y), IM_COL32(14, 20, 30, 255), 4.0f);
                draw->AddRect(p0, ImVec2(p0.x + sz.x, p0.y + sz.y), IM_COL32(0, 180, 230, 180), 4.0f);

                float mid_y = p0.y + sz.y * 0.5f;
                draw->AddLine(ImVec2(p0.x, mid_y), ImVec2(p0.x + sz.x, mid_y), IM_COL32(30, 50, 75, 180));

                for (float wx = p0.x + 4.0f; wx < p0.x + sz.x - 4.0f; wx += 2.0f) {
                    float prog = (wx - p0.x) / sz.x;
                    if (clip.is_reversed) prog = 1.0f - prog;

                    float amp = (std::sin(prog * 35.0f) * 0.6f + std::cos(prog * 10.0f) * 0.4f) * (sz.y * 0.4f);
                    
                    // Aplica fades visuais
                    if (clip.fade_in_sec > 0.0f && prog * clip.length_sec < clip.fade_in_sec) {
                        amp *= (prog * clip.length_sec) / clip.fade_in_sec;
                    }
                    if (clip.fade_out_sec > 0.0f && (1.0f - prog) * clip.length_sec < clip.fade_out_sec) {
                        amp *= ((1.0f - prog) * clip.length_sec) / clip.fade_out_sec;
                    }

                    draw->AddLine(ImVec2(wx, mid_y - std::abs(amp)), ImVec2(wx, mid_y + std::abs(amp)), IM_COL32(0, 212, 255, 220), 1.5f);
                }

                ImGui::Dummy(sz);
                ImGui::Spacing();

                // Botão Fechar
                if (ImGui::Button("OK / Salvar", ImVec2(120, 24))) {
                    is_open = false;
                }
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }
    };

} // namespace KuroUI

extern KuroUI::AudioClipEditorUI g_audio_clip_editor_ui;
