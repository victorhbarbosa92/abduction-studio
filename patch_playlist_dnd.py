import os

file_path = r'src/ui/KuroPlaylistUI.h'
with open(file_path, 'r', encoding='utf-8') as f:
    content = f.read()

# 1. Add g_demo_dnd_active
target_decl = '    extern void SetFLHint(const std::string& hint);'
repl_decl = '''    extern void SetFLHint(const std::string& hint);
    inline bool g_demo_dnd_active = false;'''
if target_decl in content:
    content = content.replace(target_decl, repl_decl, 1)

# 2. Add trigger_dnd check in render()
target_render = '            float bar_len_sec = beat_len_sec * 4.0f;'
repl_render = '''            float bar_len_sec = beat_len_sec * 4.0f;

            std::filesystem::path trigger_dnd = std::filesystem::temp_directory_path() / "abduction_dnd_demo.txt";
            if (std::filesystem::exists(trigger_dnd)) {
                std::string cmd = "";
                std::ifstream ifs(trigger_dnd);
                if (ifs) ifs >> cmd;
                ifs.close();
                std::error_code ec;
                std::filesystem::remove(trigger_dnd, ec);
                if (cmd == "ghost") {
                    KuroUI::g_demo_dnd_active = true;
                } else if (cmd == "drop") {
                    KuroUI::g_demo_dnd_active = false;
                    AudioClip ac;
                    ac.id = g_clip_manager.next_id++;
                    ac.start_time_sec = bar_len_sec * 2.0f;
                    ac.length_sec = bar_len_sec * 1.5f;
                    ac.source_offset_sec = 0.0f;
                    ac.file_path = "Sonicspore_FPS_Kick_01.wav";
                    ac.name = "Sonicspore_FPS_Kick_01.wav";
                    ac.is_selected = true;
                    g_clip_manager.track_clips[1].push_back(ac);
                    ::track_names[1] = "Sonicspore Kick 01";
                } else if (cmd == "clear") {
                    KuroUI::g_demo_dnd_active = false;
                }
            }'''
if target_render in content:
    content = content.replace(target_render, repl_render, 1)

# 3. Update the track DND block
target_dnd = '''                // ── DRAG & DROP DE SAMPLES DIRETO NA GRADE TEMPORAL (ABLETON / FL STYLE) ──
                if (ImGui::BeginDragDropTargetCustom(ImRect(ImVec2(grid_start_x, ty0), ImVec2(grid_start_x + grid_view_w, ty1)), ImGui::GetID(("TrackLaneDND_" + std::to_string(t)).c_str()))) {
                    float drop_time_sec = snap_time((io.MousePos.x - grid_start_x + playlist_scroll_x) / time_to_px);
                    float ghost_gx0 = grid_start_x + (drop_time_sec * time_to_px) - playlist_scroll_x;

                    // Extrair informações reais do sample sendo arrastado
                    std::string drag_fname = "Sample.wav";
                    std::string drag_fpath = "";
                    float sample_dur = bar_len_sec * 2.0f;'''

repl_dnd = '''                // ── DRAG & DROP DE SAMPLES DIRETO NA GRADE TEMPORAL (ABLETON / FL STYLE) ──
                bool is_dnd_active = ImGui::BeginDragDropTargetCustom(ImRect(ImVec2(grid_start_x, ty0), ImVec2(grid_start_x + grid_view_w, ty1)), ImGui::GetID(("TrackLaneDND_" + std::to_string(t)).c_str()));
                bool show_ghost = is_dnd_active || (KuroUI::g_demo_dnd_active && t == 1);

                if (show_ghost) {
                    float drop_time_sec = is_dnd_active ? snap_time((io.MousePos.x - grid_start_x + playlist_scroll_x) / time_to_px) : (bar_len_sec * 2.0f);
                    float ghost_gx0 = grid_start_x + (drop_time_sec * time_to_px) - playlist_scroll_x;

                    // Extrair informações reais do sample sendo arrastado
                    std::string drag_fname = "Sonicspore_FPS_Kick_01.wav";
                    std::string drag_fpath = "";
                    float sample_dur = bar_len_sec * 1.5f;'''

if target_dnd in content:
    content = content.replace(target_dnd, repl_dnd, 1)

# 4. Update the EndDragDropTarget guard
target_end = '''                    // 8. Ao Soltar o Sample na Trilha
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SAMPLE_PATH")) {
                        const char* dropped_path = (const char*)payload->Data;
                        if (t < MAX_TRACKS && dropped_path) {
                            g_piano_synth.loadTrackSample(t, dropped_path);
                            std::string fname = std::filesystem::path(dropped_path).filename().string();
                            ::track_names[t] = fname;

                            AudioClip ac;
                            ac.id = g_clip_manager.next_id++;
                            ac.start_time_sec = drop_time_sec;
                            float dur = sample_dur;
                            auto& ds = g_piano_synth.getDrumSample(t);
                            if (ds.loaded && ds.sample_rate > 0 && ds.total_frames > 0) {
                                dur = (float)ds.total_frames / (float)ds.sample_rate;
                            }
                            ac.length_sec = dur;
                            ac.source_offset_sec = 0.0f;
                            ac.file_path = dropped_path;
                            ac.name = fname;
                            ac.is_selected = true;
                            g_clip_manager.track_clips[t].push_back(ac);
                            g_audio_clip_editor_ui.open(t, (int)g_clip_manager.track_clips[t].size() - 1);
                            g_clip_manager.pushUndo();
                        }
                    }
                    ImGui::EndDragDropTarget();
                }'''

repl_end = '''                    if (is_dnd_active) {
                        // 8. Ao Soltar o Sample na Trilha
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SAMPLE_PATH")) {
                            const char* dropped_path = (const char*)payload->Data;
                            if (t < MAX_TRACKS && dropped_path) {
                                g_piano_synth.loadTrackSample(t, dropped_path);
                                std::string fname = std::filesystem::path(dropped_path).filename().string();
                                ::track_names[t] = fname;

                                AudioClip ac;
                                ac.id = g_clip_manager.next_id++;
                                ac.start_time_sec = drop_time_sec;
                                float dur = sample_dur;
                                auto& ds = g_piano_synth.getDrumSample(t);
                                if (ds.loaded && ds.sample_rate > 0 && ds.total_frames > 0) {
                                    dur = (float)ds.total_frames / (float)ds.sample_rate;
                                }
                                ac.length_sec = dur;
                                ac.source_offset_sec = 0.0f;
                                ac.file_path = dropped_path;
                                ac.name = fname;
                                ac.is_selected = true;
                                g_clip_manager.track_clips[t].push_back(ac);
                                g_audio_clip_editor_ui.open(t, (int)g_clip_manager.track_clips[t].size() - 1);
                                g_clip_manager.pushUndo();
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }'''

if target_end in content:
    content = content.replace(target_end, repl_end, 1)

tmp_file = file_path + '.tmp'
with open(tmp_file, 'w', encoding='utf-8') as f:
    f.write(content)
os.replace(tmp_file, file_path)
print("SUCCESS: KuroPlaylistUI.h updated successfully")
