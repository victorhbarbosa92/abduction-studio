with open('scratch/generate_playlist_ui.py', 'r', encoding='utf-8') as f:
    code = f.read()

old_unload = """                        if (ImGui::MenuItem("Remover Sample da Faixa")) {
                            g_piano_synth.unloadTrackSample(t);
                        }"""

new_unload = """                        if (ImGui::MenuItem("Remover Sample da Faixa")) {
                            g_piano_synth.getDrumSample(t).loaded = false;
                            g_piano_synth.getDrumSample(t).sample_data.clear();
                        }"""

old_midi = """                        if (current_tool == TOOL_DRAW || current_tool == TOOL_PAINT) {
                            if (ImGui::IsMouseDoubleClicked(0)) {
                                g_clip_manager.addMidiClip(clicked_track, click_time, bar_len_sec * 4.0f, g_clip_manager.current_pattern_idx);
                                show_piano_roll = true;
                            } else {
                                g_clip_manager.addMidiClip(clicked_track, click_time, bar_len_sec * 4.0f, g_clip_manager.current_pattern_idx);
                            }
                        }"""

new_midi = """                        if (current_tool == TOOL_DRAW || current_tool == TOOL_PAINT) {
                            MidiClip mc;
                            mc.id = g_clip_manager.next_id++;
                            mc.start_time_sec = click_time;
                            mc.length_sec = bar_len_sec * 4.0f;
                            mc.pattern_id = g_clip_manager.current_pattern_idx;
                            mc.name = "Pattern " + std::to_string(mc.pattern_id + 1);
                            g_clip_manager.track_midi_clips[clicked_track].push_back(mc);
                            if (ImGui::IsMouseDoubleClicked(0)) {
                                show_piano_roll = true;
                            }
                        }"""

assert old_unload in code, "old_unload not found"
code = code.replace(old_unload, new_unload)

assert old_midi in code, "old_midi not found"
code = code.replace(old_midi, new_midi)

with open('scratch/generate_playlist_ui.py', 'w', encoding='utf-8') as f:
    f.write(code)

print("scratch/generate_playlist_ui.py updated.")
