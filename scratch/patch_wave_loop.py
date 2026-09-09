with open("src/ui/KuroPlaylistUI.h", "r", encoding="utf-8") as f:
    kuro = f.read()

# 1. Scroll sync with global externs
old_scroll_def = """        float playlist_scroll_x = 0.0f;
        float playlist_scroll_y = 0.0f;"""

new_scroll_def = """        float playlist_scroll_x = 0.0f;
        float playlist_scroll_y = 0.0f;
        bool scroll_initialized = false;"""

assert old_scroll_def in kuro, "old_scroll_def not found"
kuro = kuro.replace(old_scroll_def, new_scroll_def)

# In render(): sync once if g_playlist_scroll_y or x was set
old_render_start = """            textures.Init();

            ImGui::SetNextWindowSize(ImVec2(1200, 680), ImGuiCond_FirstUseEver);"""

new_render_start = """            textures.Init();

            if (!scroll_initialized) {
                extern float g_playlist_scroll_x;
                extern float g_playlist_scroll_y;
                if (g_playlist_scroll_x > 0.0f) playlist_scroll_x = g_playlist_scroll_x;
                if (g_playlist_scroll_y > 0.0f) playlist_scroll_y = g_playlist_scroll_y;
                scroll_initialized = true;
            }

            ImGui::SetNextWindowSize(ImVec2(1200, 680), ImGuiCond_FirstUseEver);"""

assert old_render_start in kuro, "old_render_start not found"
kuro = kuro.replace(old_render_start, new_render_start)

# 2. Waveform sample looping
old_wave_loop = """                float step_px = 2.0f;
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
                    } else if (p_stem_buf && !p_stem_buf->empty()) {"""

new_wave_loop = """                float step_px = 2.0f;
                float sr = 44100.0f;
                float beat_sec = bar_len_sec * 0.25f;

                for (float px = draw_x0; px < draw_x1; px += step_px) {
                    float clip_t = ((px - cx0) / time_to_px) + ac.source_offset_sec;
                    float amp_l = 0.0f;
                    float amp_r = 0.0f;

                    if (ds.loaded && !ds.sample_data.empty() && ds.total_frames > 0) {
                        float sample_dur = (float)ds.total_frames / (float)ds.sample_rate;
                        float loop_clip_t = (sample_dur < beat_sec) ? fmodf(clip_t, beat_sec) : fmodf(clip_t, sample_dur);
                        size_t frame_idx = (size_t)(loop_clip_t * (float)ds.sample_rate);
                        if (frame_idx < (size_t)ds.total_frames) {
                            if (ds.channels >= 2) {
                                amp_l = std::abs(ds.sample_data[frame_idx * ds.channels]);
                                amp_r = std::abs(ds.sample_data[frame_idx * ds.channels + 1]);
                            } else {
                                amp_l = amp_r = std::abs(ds.sample_data[frame_idx]);
                            }
                        }
                    } else if (p_stem_buf && !p_stem_buf->empty()) {"""

assert old_wave_loop in kuro, "old_wave_loop not found"
kuro = kuro.replace(old_wave_loop, new_wave_loop)

with open("src/ui/KuroPlaylistUI.h", "w", encoding="utf-8") as f:
    f.write(kuro)

print("KuroPlaylistUI.h updated with sample looping & scroll sync!")
