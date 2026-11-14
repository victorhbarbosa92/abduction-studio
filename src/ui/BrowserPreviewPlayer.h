#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "../core/ClipManager.h"
#include "../core/TimelineManager.h"
#include "../audio/SynthEngine.h"
#include "../thirdparty/dr_wav.h"
#include "ChannelInstrumentManager.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

extern KuroAudio::SynthEngine g_piano_synth;
extern int selected_track_idx;
extern std::string track_names[MAX_TRACKS];

namespace KuroUI {

    class BrowserPreviewPlayer {
    public:
        std::string current_path = "";
        std::string current_name = "Nenhum sample selecionado";
        std::string sample_info = "-- kHz • -- s";
        float duration_sec = 0.0f;
        double play_start_time = 0.0;
        bool is_playing = false;
        bool auto_preview = true;
        bool loop_preview = false;
        float preview_volume = 0.90f;
        std::vector<float> waveform_peaks;

        BrowserPreviewPlayer() {
            waveform_peaks.assign(120, 0.06f);
        }

        static inline std::wstring to_wide_str(const std::string& str) {
            if (str.empty()) return std::wstring();
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
            std::wstring wstr(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
            return wstr;
        }

        void loadSample(const std::string& path, const std::string& name, bool start_play = true) {
            current_path = path;
            current_name = name;
            waveform_peaks.assign(120, 0.05f);
            duration_sec = 0.0f;

            if (std::filesystem::exists(to_fs_path(path))) {
                drwav wav;
                bool opened = false;
#ifdef _WIN32
                std::wstring wpath = to_wide_str(path);
                if (!wpath.empty() && drwav_init_file_w(&wav, wpath.c_str(), NULL)) {
                    opened = true;
                }
#endif
                if (!opened && drwav_init_file(&wav, path.c_str(), NULL)) {
                    opened = true;
                }

                if (opened) {
                    duration_sec = (wav.sampleRate > 0) ? (float)wav.totalPCMFrameCount / (float)wav.sampleRate : 0.0f;
                    char info_buf[96];
                    snprintf(info_buf, sizeof(info_buf), "%.1f kHz • %.2fs • %d ch", (float)wav.sampleRate / 1000.0f, duration_sec, (int)wav.channels);
                    sample_info = info_buf;

                    if (wav.totalPCMFrameCount > 0) {
                        drwav_uint64 step = std::max((drwav_uint64)1, wav.totalPCMFrameCount / 120);
                        std::vector<float> chunk(step * wav.channels);
                        for (int p = 0; p < 120; p++) {
                            drwav_uint64 frames_read = drwav_read_pcm_frames_f32(&wav, step, chunk.data());
                            if (frames_read == 0) break;
                            float peak = 0.0f;
                            for (drwav_uint64 i = 0; i < frames_read * wav.channels; ++i) {
                                float val = std::abs(chunk[i]);
                                if (val > peak) peak = val;
                            }
                            waveform_peaks[p] = std::clamp(peak, 0.03f, 1.0f);
                        }
                    }
                    drwav_uninit(&wav);
                } else {
                    duration_sec = 2.0f;
                    sample_info = "Audio Preview";
                    for (int p = 0; p < 120; p++) {
                        waveform_peaks[p] = std::abs(std::sin(p * 0.16f) * 0.55f + std::cos(p * 0.06f) * 0.35f);
                    }
                }
            }

            if (start_play) {
                play();
            } else {
                stop();
            }
        }

        void play() {
            if (current_path.empty() || !std::filesystem::exists(to_fs_path(current_path))) return;

#ifdef _WIN32
            DWORD vol = (DWORD)(std::clamp(preview_volume, 0.0f, 1.0f) * 65535.0f);
            waveOutSetVolume(0, MAKELONG(vol, vol));

            DWORD flags = SND_ASYNC | SND_FILENAME;
            if (loop_preview) flags |= SND_LOOP;
            PlaySoundA(current_path.c_str(), NULL, flags);
#endif
            play_start_time = ImGui::GetTime();
            is_playing = true;
        }

        void stop() {
#ifdef _WIN32
            PlaySoundA(NULL, NULL, 0);
#endif
            is_playing = false;
            play_start_time = 0.0;
        }

        void togglePlay() {
            if (is_playing) stop();
            else play();
        }

        void RenderDock(float width, float height, ClipManager& clip_mgr, KuroDSP::TimelineManager& timeline) {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 p1 = ImVec2(p0.x + width, p0.y + height);

            // Fundo escuro com borda sutil e friso superior ciano neon
            draw->AddRectFilled(p0, p1, IM_COL32(6, 10, 16, 255));
            draw->AddLine(p0, ImVec2(p1.x, p0.y), IM_COL32(0, 229, 255, 200), 1.5f);
            draw->AddRect(p0, p1, IM_COL32(18, 28, 42, 255));

            float pad_x = 8.0f;
            float cur_y = p0.y + 6.0f;

            // ── LINHA 1: Ícone + Nome do Sample + Info Técnica ──
            ImGui::SetCursorScreenPos(ImVec2(p0.x + pad_x, cur_y));
            ImGui::BeginGroup();
            
            // Ícone animado se tocando
            if (is_playing) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "🔊");
            } else {
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "🎵");
            }
            ImGui::SameLine(0, 5.0f);

            // Nome do sample encurtado com tooltip
            std::string disp_name = current_name;
            if (disp_name.length() > 24) disp_name = disp_name.substr(0, 22) + "..";
            ImGui::TextColored(ImVec4(0.95f, 0.98f, 1.0f, 1.0f), "%s", disp_name.c_str());
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\nCaminho: %s", current_name.c_str(), current_path.c_str());

            // Info técnica à direita
            float info_w = ImGui::CalcTextSize(sample_info.c_str()).x;
            float info_x = p1.x - info_w - pad_x;
            if (info_x > p0.x + 100.0f) {
                ImGui::SameLine(info_x - (p0.x + pad_x));
                ImGui::TextColored(ImVec4(0.40f, 0.70f, 0.90f, 0.85f), "%s", sample_info.c_str());
            }
            ImGui::EndGroup();

            cur_y += 20.0f;

            // ── LINHA 2: Mini-Forma de Onda Interativa (Ableton & FL Studio Style) ──
            float wf_x0 = p0.x + pad_x;
            float wf_w = width - (pad_x * 2.0f);
            float wf_y0 = cur_y;
            float wf_h = 36.0f;
            ImVec2 wf_p0(wf_x0, wf_y0);
            ImVec2 wf_p1(wf_x0 + wf_w, wf_y0 + wf_h);

            draw->AddRectFilled(wf_p0, wf_p1, IM_COL32(4, 7, 12, 255), 3.0f);
            draw->AddRect(wf_p0, wf_p1, IM_COL32(22, 34, 50, 255), 3.0f);

            float mid_y = wf_y0 + wf_h * 0.5f;
            draw->AddLine(ImVec2(wf_x0, mid_y), ImVec2(wf_p1.x, mid_y), IM_COL32(0, 180, 220, 60));

            // Desenhar os 120 picos de onda
            int num_pts = (int)waveform_peaks.size();
            if (num_pts > 0 && wf_w > 10.0f) {
                float step_w = wf_w / (float)num_pts;
                ImU32 wave_col = loop_preview ? IM_COL32(255, 160, 40, 230) : IM_COL32(0, 229, 255, 230);
                for (int i = 0; i < num_pts; i++) {
                    float bx = wf_x0 + i * step_w;
                    float amp = waveform_peaks[i] * (wf_h * 0.44f);
                    draw->AddLine(ImVec2(bx, mid_y - amp), ImVec2(bx, mid_y + amp), wave_col, 1.4f);
                }
            }

            // Playhead Cursor em tempo real
            if (is_playing && duration_sec > 0.05f) {
                double now = ImGui::GetTime();
                float elapsed = (float)(now - play_start_time);
                float prog = elapsed / duration_sec;
                if (prog >= 1.0f) {
                    if (loop_preview) {
                        play_start_time = now;
                        prog = 0.0f;
                    } else {
                        is_playing = false;
                        prog = 0.0f;
                    }
                }
                float hx = wf_x0 + prog * wf_w;
                draw->AddLine(ImVec2(hx, wf_y0), ImVec2(hx, wf_y0 + wf_h), IM_COL32(255, 255, 255, 255), 1.8f);
                draw->AddRectFilled(ImVec2(hx - 2.0f, wf_y0), ImVec2(hx + 2.0f, wf_y0 + wf_h), IM_COL32(0, 229, 255, 70));
            }

            // Interação na Onda (Clique para tocar / arrastar direto para o Arranger ou Rack)
            ImGui::SetCursorScreenPos(wf_p0);
            if (ImGui::InvisibleButton("##wf_click_audition", ImVec2(wf_w, wf_h))) {
                togglePlay();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Clique: Tocar / Pausar\nArraste: Soltar sample na Playlist ou Channel Rack");
            }

            // O próprio visualizador da forma de onda é Drag & Drop Source!
            if (!current_path.empty() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("DND_SAMPLE_PATH", current_path.c_str(), current_path.length() + 1);
                ImGui::Text("Sample: %s", current_name.c_str());
                ImGui::EndDragDropSource();
            }

            cur_y += wf_h + 6.0f;

            // ── LINHA 3: Botões de Transporte e Ações Rápidas ──
            ImGui::SetCursorScreenPos(ImVec2(p0.x + pad_x, cur_y));

            // Botão Play / Pause
            if (is_playing) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.70f, 0.40f, 0.90f));
                if (ImGui::Button("⏸##bp_play", ImVec2(28, 22))) togglePlay();
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.16f, 0.24f, 0.90f));
                if (ImGui::Button("▶##bp_play", ImVec2(28, 22))) togglePlay();
                ImGui::PopStyleColor();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip(is_playing ? "Pausar Preview" : "Tocar Preview");

            ImGui::SameLine(0, 4);

            // Botão Stop
            if (ImGui::Button("⏹##bp_stop", ImVec2(24, 22))) {
                stop();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Parar Reprodução");

            ImGui::SameLine(0, 4);

            // Botão Loop
            if (loop_preview) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.60f, 0.10f, 0.90f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.14f, 0.20f, 0.80f));
            if (ImGui::Button("🔁##bp_loop", ImVec2(24, 22))) {
                loop_preview = !loop_preview;
                if (is_playing) play();
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip(loop_preview ? "Loop Ativado" : "Ativar Reprodução Contínua");

            ImGui::SameLine(0, 6);

            // Checkbox Auto Preview (Tocar automaticamente ao clicar ou usar setas)
            ImGui::Checkbox("Auto", &auto_preview);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pré-escuta automática de 1 clique ao navegar nas amostras");

            ImGui::SameLine(0, 6);

            // Botão Inserir no Channel Rack
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.50f, 0.75f, 0.85f));
            if (ImGui::Button("➕ Rack", ImVec2(56, 22))) {
                if (!current_path.empty() && selected_track_idx >= 0 && selected_track_idx < MAX_TRACKS) {
                    g_channel_slots[selected_track_idx].type = ChannelInstrumentType::AUDIO_SAMPLE;
                    g_channel_slots[selected_track_idx].sample_name = current_name;
                    track_names[selected_track_idx] = current_name;
                    {
                        std::lock_guard<std::mutex> lock(g_piano_synth.getMutex());
                        g_piano_synth.getDrumSample(selected_track_idx).load(current_path);
                    }
                    g_piano_synth.triggerNote(60, 0.35f, 0.95f, selected_track_idx);
                }
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Carregar este sample no canal atualmente selecionado no Channel Rack");

            ImGui::SameLine(0, 4);

            // Botão Inserir na Timeline Playlist
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.65f, 0.45f, 0.85f));
            if (ImGui::Button("➕ Track", ImVec2(58, 22))) {
                if (!current_path.empty()) {
                    int target_t = selected_track_idx >= 0 ? selected_track_idx : 0;
                    g_piano_synth.loadTrackSample(target_t, current_path.c_str());
                    track_names[target_t] = "[SMP] " + current_name;

                    float bpm = timeline.getBPM();
                    if (bpm < 10.0f) bpm = 140.0f;
                    float bar_len_sec = (60.0f / bpm) * 4.0f;
                    float playhead_sec = (float)timeline.getMasterFrame() / 44100.0f;
                    float snap_pos = std::round(playhead_sec / (bar_len_sec * 0.25f)) * (bar_len_sec * 0.25f);

                    AudioClip ac;
                    ac.id = clip_mgr.next_id++;
                    ac.start_time_sec = snap_pos;
                    float dur = bar_len_sec * 4.0f;
                    auto& ds = g_piano_synth.getDrumSample(target_t);
                    if (ds.loaded && ds.sample_rate > 0 && ds.total_frames > 0) {
                        dur = (float)ds.total_frames / (float)ds.sample_rate;
                    }
                    ac.length_sec = dur;
                    ac.source_offset_sec = 0.0f;
                    ac.name = current_name;
                    ac.file_path = current_path;

                    clip_mgr.track_clips[target_t].push_back(ac);
                    g_audio_clip_editor_ui.open(target_t, (int)clip_mgr.track_clips[target_t].size() - 1);
                }
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Inserir este sample como clipe de áudio na timeline da Playlist");

            // Define o cursor do ImGui para o final da região do dock e submete item delimitador
            ImGui::SetCursorScreenPos(ImVec2(p0.x, p1.y));
            ImGui::Dummy(ImVec2(width, 0.0f));
        }
    };

    inline BrowserPreviewPlayer g_browser_preview_player;

} // namespace KuroUI
