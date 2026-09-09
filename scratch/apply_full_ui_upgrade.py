import os
import sys

print("Applying full UI upgrade to KuroPlaylistUI.h...")

with open("src/ui/KuroPlaylistUI.h", "r", encoding="utf-8") as f:
    code = f.read()

# 1. Read icon methods and toolbar
with open("scratch/patch_toolbar.py", "r", encoding="utf-8") as f:
    py_txt = f.read()
    s_idx = py_txt.find("new_toolbar_and_clips = r'''") + len("new_toolbar_and_clips = r'''")
    e_idx = py_txt.rfind("'''")
    toolbar_code = py_txt[s_idx:e_idx].strip()

# 2. Find start of renderModernToolbar and end of it
start_toolbar = code.find("        void renderModernToolbar(float bpm, float bar_len_sec) {")
assert start_toolbar != -1, "start_toolbar not found"

# Find start of renderTracksAndClips
start_tracks = code.find("        template<typename FSnap>\n        void renderTracksAndClips(")
assert start_tracks != -1, "start_tracks not found"

# Replace renderModernToolbar with toolbar_code
code = code[:start_toolbar] + toolbar_code + "\n\n" + code[start_tracks:]

# 3. Now replace renderClipsForTrack
start_clips = code.find("        template<typename FSnap>\n        void renderClipsForTrack(")
assert start_clips != -1, "start_clips not found"

end_clips = code.find("            // 3. CLIPS DE AUTOMACAO COM CURVAS BEZIER E GRADIENTE", start_clips)
assert end_clips != -1, "end_clips marker not found"

new_clips_code = r'''        template<typename FSnap>
        void renderClipsForTrack(ImDrawList* draw, int t, float ty0, float ty1, float grid_start_x, float grid_view_w, float time_to_px, float bar_len_sec, ImU32 trk_color, float snap_sec, FSnap snap_time, bool is_hovered) {
            ImGuiIO& io = ImGui::GetIO();
            float trk_h = ty1 - ty0;

            // ── 1. CLIPS DE AUDIO COM FORMA DE ONDA ESTEREO REAL (WAVEFORMS) ──
            auto& a_clips = g_clip_manager.track_clips[t];
            for (int i = 0; i < (int)a_clips.size(); ++i) {
                auto& ac = a_clips[i];
                float cx0 = grid_start_x + (ac.start_time_sec * time_to_px) - playlist_scroll_x;
                float cx1 = cx0 + (ac.length_sec * time_to_px);
                if (cx1 < grid_start_x || cx0 > grid_start_x + grid_view_w) continue;

                float draw_x0 = (std::max)(cx0, grid_start_x);
                float draw_x1 = (std::min)(cx1, grid_start_x + grid_view_w);

                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), IM_COL32(14, 22, 34, 235), 5.0f);
                draw->AddRect(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), trk_color, 5.0f);

                // Barra de título do clipe de áudio
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty0 + 16.0f), IM_COL32(10, 16, 26, 255), 5.0f);
                draw->AddText(ImVec2(draw_x0 + 6.0f, ty0 + 3.0f), IM_COL32(220, 245, 255, 255), ac.name.c_str());

                // Linha zero central da forma de onda estéreo
                float wave_top = ty0 + 17.0f;
                float wave_bot = ty1 - 3.0f;
                float wave_h = wave_bot - wave_top;
                float mid_y = wave_top + wave_h * 0.5f;

                draw->AddLine(ImVec2(draw_x0, mid_y), ImVec2(draw_x1, mid_y), IM_COL32(40, 75, 110, 180), 1.0f);

                // Dados de áudio PCM reais ou sintetizados com fidelidade analógica
                const std::vector<float>* p_stem_buf = (::g_ai_engine && t < MAX_TRACKS) ? &::g_ai_engine->getStemBuffer(t) : nullptr;
                auto& ds = g_piano_synth.getDrumSample(t);

                float step_px = 2.0f;
                float sr = 44100.0f;

                for (float px = draw_x0; px < draw_x1; px += step_px) {
                    float clip_t = ((px - cx0) / time_to_px) + ac.source_offset_sec;
                    float amp_l = 0.0f;
                    float amp_r = 0.0f;

                    if (ds.loaded && !ds.sample_data.empty()) {
                        size_t frame_idx = (size_t)(clip_t * ds.sample_rate);
                        if (frame_idx < (size_t)ds.total_frames) {
                            if (ds.channels >= 2) {
                                amp_l = std::abs(ds.sample_data[frame_idx * ds.channels]);
                                amp_r = std::abs(ds.sample_data[frame_idx * ds.channels + 1]);
                            } else {
                                amp_l = amp_r = std::abs(ds.sample_data[frame_idx]);
                            }
                        }
                    } else if (p_stem_buf && !p_stem_buf->empty()) {
                        size_t s_idx = (size_t)(clip_t * sr);
                        if (s_idx < p_stem_buf->size()) {
                            amp_l = amp_r = std::abs((*p_stem_buf)[s_idx]);
                        }
                    } else {
                        // Forma de onda processual dinâmica estéreo (Transient Attack + Harmonic Decay)
                        float rel_bar_t = fmodf(clip_t, bar_len_sec * 0.25f);
                        float env = std::exp(-rel_bar_t * 9.0f);
                        float osc = std::sin(rel_bar_t * 620.0f) * 0.6f + std::sin(rel_bar_t * 1280.0f) * 0.4f;
                        amp_l = std::clamp(std::abs(osc) * env * 0.95f + 0.04f, 0.03f, 0.95f);
                        amp_r = std::clamp(amp_l * 0.88f + std::sin(rel_bar_t * 400.0f) * 0.05f, 0.03f, 0.95f);
                    }

                    amp_l = std::clamp(amp_l, 0.02f, 0.98f);
                    amp_r = std::clamp(amp_r, 0.02f, 0.98f);

                    float h_l = amp_l * (wave_h * 0.46f);
                    float h_r = amp_r * (wave_h * 0.46f);

                    // Canal L (Ciano Elétrico)
                    draw->AddLine(ImVec2(px, mid_y - h_l), ImVec2(px, mid_y - 0.5f), IM_COL32(0, 230, 255, 235), 1.5f);
                    // Canal R (Roxo/Magenta)
                    draw->AddLine(ImVec2(px, mid_y + 0.5f), ImVec2(px, mid_y + h_r), IM_COL32(190, 110, 255, 235), 1.5f);
                }

                ImVec2 hit_p0(draw_x0, ty0 + 2.0f);
                ImVec2 hit_p1(draw_x1, ty1 - 2.0f);
                if (is_hovered && ImGui::IsMouseHoveringRect(hit_p0, hit_p1)) {
                    if (current_tool == TOOL_DELETE && ImGui::IsMouseClicked(0)) {
                        a_clips.erase(a_clips.begin() + i);
                        --i;
                        continue;
                    }
                    if (current_tool == TOOL_SLICE && ImGui::IsMouseClicked(0)) {
                        float cut_time = snap_time((io.MousePos.x - grid_start_x + playlist_scroll_x) / time_to_px);
                        if (cut_time > ac.start_time_sec + 0.05f && cut_time < ac.start_time_sec + ac.length_sec - 0.05f) {
                            AudioClip ac2 = ac;
                            float first_len = cut_time - ac.start_time_sec;
                            ac2.start_time_sec = cut_time;
                            ac2.length_sec = ac.length_sec - first_len;
                            ac2.source_offset_sec = ac.source_offset_sec + first_len;
                            ac.length_sec = first_len;
                            a_clips.insert(a_clips.begin() + i + 1, ac2);
                            break;
                        }
                    }
                }
            }

            // ── 2. CLIPS DE PADRAO MIDI COM NOTAS VISIVEIS EM NEON CIANO ──
            auto& m_clips = g_clip_manager.track_midi_clips[t];
            for (int i = 0; i < (int)m_clips.size(); ++i) {
                auto& mc = m_clips[i];
                float cx0 = grid_start_x + (mc.start_time_sec * time_to_px) - playlist_scroll_x;
                float cx1 = cx0 + (mc.length_sec * time_to_px);
                if (cx1 < grid_start_x || cx0 > grid_start_x + grid_view_w) continue;

                float draw_x0 = (std::max)(cx0, grid_start_x);
                float draw_x1 = (std::min)(cx1, grid_start_x + grid_view_w);

                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), IM_COL32(20, 16, 32, 240), 5.0f);
                draw->AddRect(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty1 - 2.0f), trk_color, 5.0f);

                // Barra de título do clipe MIDI
                draw->AddRectFilled(ImVec2(draw_x0, ty0 + 2.0f), ImVec2(draw_x1, ty0 + 16.0f), IM_COL32(14, 10, 24, 255), 5.0f);
                draw->AddText(ImVec2(draw_x0 + 6.0f, ty0 + 3.0f), IM_COL32(245, 225, 255, 255), mc.name.c_str());

                // Busca o Pattern correspondente pelo ID real (não índice cru de array)
                const Pattern* p = nullptr;
                for (const auto& pat : g_clip_manager.global_patterns) {
                    if (pat.id == mc.pattern_id) { p = &pat; break; }
                }
                if (!p && mc.pattern_id >= 0 && mc.pattern_id < (int)g_clip_manager.global_patterns.size()) {
                    p = &g_clip_manager.global_patterns[mc.pattern_id];
                }

                if (p) {
                    std::vector<int> target_channels;
                    if (t >= 0 && t < MAX_TRACKS && !p->getChannelNotes(t).empty()) {
                        target_channels.push_back(t);
                    } else {
                        for (int ch = 0; ch < MAX_TRACKS; ch++) {
                            if (!p->getChannelNotes(ch).empty()) target_channels.push_back(ch);
                        }
                    }

                    float pat_len = 0.0f;
                    int min_pitch = 127;
                    int max_pitch = 0;
                    for (int ch : target_channels) {
                        for (const auto& n : p->getChannelNotes(ch)) {
                            if (n.start_time + n.duration > pat_len) pat_len = n.start_time + n.duration;
                            if (n.pitch < min_pitch) min_pitch = n.pitch;
                            if (n.pitch > max_pitch) max_pitch = n.pitch;
                        }
                    }
                    if (pat_len <= 0.001f) pat_len = bar_len_sec * 4.0f;
                    float bars = std::ceil(pat_len / bar_len_sec);
                    if (bars < 1.0f) bars = 1.0f;
                    pat_len = bars * bar_len_sec;
                    if (min_pitch > max_pitch) { min_pitch = 36; max_pitch = 60; }

                    float grid_inner_y0 = ty0 + 17.0f;
                    float grid_inner_h = (trk_h - 22.0f);

                    // Renderiza as notas MIDI e repetições de loop por todo o clipe
                    for (float loop_start = 0.0f; loop_start < mc.length_sec; loop_start += pat_len) {
                        if (loop_start > 0.0f) {
                            float div_x = cx0 + (loop_start * time_to_px);
                            if (div_x >= draw_x0 && div_x <= draw_x1) {
                                draw->AddLine(ImVec2(div_x, ty0 + 16.0f), ImVec2(div_x, ty1 - 2.0f), IM_COL32(255, 255, 255, 35), 1.0f);
                            }
                        }

                        for (int ch : target_channels) {
                            for (const auto& note : p->getChannelNotes(ch)) {
                                float note_start_rel = loop_start + note.start_time;
                                if (note_start_rel >= mc.length_sec) continue;
                                float note_dur_rel = note.duration;
                                if (note_start_rel + note_dur_rel > mc.length_sec) {
                                    note_dur_rel = mc.length_sec - note_start_rel;
                                }

                                float note_x0 = cx0 + (note_start_rel * time_to_px);
                                float note_x1 = note_x0 + (note_dur_rel * time_to_px);
                                if (note_x1 - note_x0 < 3.0f) note_x1 = note_x0 + 3.0f;

                                if (note_x1 < draw_x0 || note_x0 > draw_x1) continue;

                                float n_draw_x0 = (std::max)(note_x0, draw_x0);
                                float n_draw_x1 = (std::min)(note_x1, draw_x1);

                                float note_y = 0.0f;
                                if (max_pitch > min_pitch) {
                                    float pitch_norm = (float)(note.pitch - min_pitch) / (float)(max_pitch - min_pitch);
                                    note_y = (ty1 - 6.0f) - pitch_norm * (grid_inner_h - 8.0f);
                                } else {
                                    note_y = grid_inner_y0 + grid_inner_h * 0.5f;
                                }

                                // NOTAS MIDI BRILHANTES EM CIANO NEON COM NÚCLEO BRANCO
                                draw->AddRectFilled(ImVec2(n_draw_x0, note_y - 2.5f), ImVec2(n_draw_x1, note_y + 2.5f), IM_COL32(0, 235, 255, 245), 2.0f);
                                draw->AddRectFilled(ImVec2(n_draw_x0, note_y - 1.0f), ImVec2(n_draw_x1, note_y + 1.0f), IM_COL32(230, 255, 255, 255), 1.0f);
                            }
                        }
                    }
                }

                ImVec2 hit_p0(draw_x0, ty0 + 2.0f);
                ImVec2 hit_p1(draw_x1, ty1 - 2.0f);
                if (is_hovered && ImGui::IsMouseHoveringRect(hit_p0, hit_p1)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        g_clip_manager.current_pattern_idx = mc.pattern_id;
                        show_piano_roll = true;
                    }
                    if (current_tool == TOOL_DELETE && ImGui::IsMouseClicked(0)) {
                        m_clips.erase(m_clips.begin() + i);
                        --i;
                        continue;
                    }
                }
            }

'''

code = code[:start_clips] + new_clips_code + code[end_clips:]

with open("src/ui/KuroPlaylistUI.h", "w", encoding="utf-8") as f:
    f.write(code)

print("KuroPlaylistUI.h fully updated!")
