import re

path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h'
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# Locate renderRekordboxPads block
idx_start = c.find("void renderRekordboxPads(KuroAudio::DJDeck& deck, ImVec2 pos, float w, float h, KuroAudio::DJEngine& engine) {")
assert idx_start != -1, "renderRekordboxPads not found"

# Find end of renderRekordboxPads
idx_end = c.find("void renderRekordboxTempoSlider(", idx_start)
if idx_end == -1:
    idx_end = c.find("void renderRekordboxPitchFaderWidget(", idx_start)
assert idx_end != -1, "renderRekordboxPitchFaderWidget not found"

old_pads_func = c[idx_start:idx_end]

new_pads_func = """void renderRekordboxPads(KuroAudio::DJDeck& deck, ImVec2 pos, float w, float h, KuroAudio::DJEngine& engine) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImFont* f_bld = g_font_rekordbox_bold ? g_font_rekordbox_bold : ImGui::GetFont();
            ImFont* f_reg = g_font_rekordbox_regular ? g_font_rekordbox_regular : ImGui::GetFont();
            auto& metrics = KuroUI::DJLayoutMetrics::getInstance();

            // Configuração da Grade de Pads: 0 = 8 Pads (4x2), 1 = 16 Pads Wide (8x2), 2 = 16 Pads Matrix (4x4)
            int grid_mode = metrics.pad_grid_mode;
            int num_pads = (grid_mode == 0) ? 8 : 16;
            int num_cols = (grid_mode == 1) ? 8 : 4;
            int num_rows = (grid_mode == 2) ? 4 : 2;

            float drop_h = std::clamp(20.0f * metrics.ui_font_scale, 20.0f, 28.0f);
            float pad_gap_x = std::clamp(metrics.S(metrics.pad_gap), 2.0f, 6.0f);
            float pad_gap_y = std::clamp(metrics.S(metrics.pad_gap), 2.0f, 6.0f);

            float total_gaps_x = pad_gap_x * (float)(num_cols - 1);
            float pad_w = std::max(22.0f, (w - total_gaps_x) / (float)num_cols);

            float avail_pad_h = std::max(36.0f, h - drop_h - 6.0f);
            float total_gaps_y = pad_gap_y * (float)(num_rows - 1);
            float pad_h = std::clamp((avail_pad_h - total_gaps_y) / (float)num_rows, 18.0f, 95.0f);

            int mode = std::clamp(rekordbox_pad_mode[deck.deck_id], 0, 5);

            // 16 Hot Cue Letters & Colors
            const char* cue_letters[16] = {
                "A", "B", "C", "D", "E", "F", "G", "H",
                "I", "J", "K", "L", "M", "N", "O", "P"
            };
            const ImU32 cue_colors[16] = {
                IM_COL32(235, 45, 80, 255),   // A: Pink/Red
                IM_COL32(255, 145, 0, 255),   // B: Orange
                IM_COL32(240, 210, 30, 255),  // C: Yellow
                IM_COL32(35, 215, 85, 255),   // D: Green
                IM_COL32(0, 200, 245, 255),   // E: Cyan
                IM_COL32(30, 110, 245, 255),  // F: Blue
                IM_COL32(170, 50, 245, 255),  // G: Purple
                IM_COL32(240, 50, 175, 255),  // H: Magenta
                IM_COL32(255, 80, 120, 255),  // I: Rose
                IM_COL32(255, 170, 50, 255),  // J: Amber
                IM_COL32(255, 230, 70, 255),  // K: Lime Gold
                IM_COL32(70, 240, 120, 255),  // L: Mint
                IM_COL32(50, 220, 255, 255),  // M: Sky Blue
                IM_COL32(80, 140, 255, 255),  // N: Cobalt
                IM_COL32(200, 80, 255, 255),  // O: Violet
                IM_COL32(255, 80, 210, 255)   // P: Hot Pink
            };

            const char* loop_labels[16] = {
                "1/32", "1/16", "1/8", "1/4", "1/2", "1", "2", "4",
                "8", "16", "32", "64", "128", "256", "ROLL 1", "ROLL 2"
            };
            const double loop_beats[16] = {
                0.03125, 0.0625, 0.125, 0.25, 0.5, 1.0, 2.0, 4.0,
                8.0, 16.0, 32.0, 64.0, 128.0, 256.0, 1.0, 2.0
            };

            const char* fx_labels[16] = {
                "ROLL 1/2", "ROLL 1", "ECHO", "REVERB", "HPF", "CRUSH", "FLANGER", "BEAT FX",
                "SPIRAL", "HELIX", "PITCH", "VINYL BRK", "TRANS", "ROLL 4", "LPF SLIP", "SPACE"
            };

            const char* smp_labels[16] = {
                "CLAP", "PSY LASER", "SUB DROP", "SIREN", "SNARE", "CRASH", "AIR HORN", "VOCAL",
                "808 BASS", "SHAKER", "GLITCH", "REV CYM", "ACID", "CHORD", "SCRATCH", "NOISE"
            };

            const char* stem_labels[16] = {
                "VOCAL", "MELODY", "DRUMS", "BASS", "VOC SOLO", "MEL SOLO", "DRM SOLO", "BAS SOLO",
                "VOC ECHO", "MEL ECHO", "DRM BREAK", "TENSION", "VOC MUTE", "MEL MUTE", "DRM MUTE", "ALL ON"
            };

            const char* jump_labels[16] = {
                "< 1/2", "1/2 >", "< 1", "1 >", "< 2", "2 >", "< 4", "4 >",
                "< 8", "8 >", "< 16", "16 >", "< 32", "32 >", "< 64", "64 >"
            };
            const double jump_beats[16] = {
                -0.5, 0.5, -1.0, 1.0, -2.0, 2.0, -4.0, 4.0,
                -8.0, 8.0, -16.0, 16.0, -32.0, 32.0, -64.0, 64.0
            };

            for (int p = 0; p < num_pads; p++) {
                int col = p % num_cols;
                int row = p / num_cols;
                ImVec2 p_min = ImVec2(pos.x + (float)col * (pad_w + pad_gap_x), pos.y + (float)row * (pad_h + pad_gap_y));
                ImVec2 p_max = ImVec2(p_min.x + pad_w, p_min.y + pad_h);

                char main_txt[24] = "";
                char sub_txt[24] = "";
                bool active = false;
                ImU32 pad_color = IM_COL32(16, 20, 28, 255);
                ImU32 border_color = IM_COL32(32, 42, 56, 220);

                if (mode == 0) { // HOT CUE
                    snprintf(main_txt, sizeof(main_txt), "%s", cue_letters[p]);
                    active = deck.hot_cues[p].active;
                    if (active) {
                        pad_color = cue_colors[p];
                        border_color = IM_COL32(255, 255, 255, 245);
                        int t_sec = (int)deck.hot_cues[p].time_sec;
                        snprintf(sub_txt, sizeof(sub_txt), "%02d:%02d", t_sec / 60, t_sec % 60);
                    }
                } else if (mode == 1) { // BEAT LOOP
                    snprintf(main_txt, sizeof(main_txt), "%s", loop_labels[p]);
                    active = (deck.loop_active && std::abs(deck.loop_beats - loop_beats[p]) < 0.001);
                    if (active) {
                        pad_color = IM_COL32(255, 180, 20, 255);
                        border_color = IM_COL32(255, 255, 255, 255);
                    } else {
                        pad_color = (p < num_pads / 2) ? IM_COL32(14, 34, 52, 255) : IM_COL32(38, 30, 16, 255);
                        border_color = (p < num_pads / 2) ? IM_COL32(0, 160, 230, 180) : IM_COL32(230, 140, 20, 180);
                    }
                } else if (mode == 2) { // PAD FX
                    snprintf(main_txt, sizeof(main_txt), "%s", fx_labels[p]);
                    pad_color = IM_COL32(18, 24, 34, 255);
                    border_color = IM_COL32(36, 52, 74, 220);
                } else if (mode == 3) { // SAMPLER
                    if (p < 16) {
                        snprintf(main_txt, sizeof(main_txt), "%s", engine.samplers[p].name.c_str());
                        active = engine.samplers[p].is_playing;
                        pad_color = active ? IM_COL32(0, 255, 140, 255) : IM_COL32(16, 32, 26, 255);
                        border_color = active ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 180, 120, 180);
                    }
                } else if (mode == 4) { // STEMS
                    snprintf(main_txt, sizeof(main_txt), "%s", stem_labels[p]);
                    pad_color = IM_COL32(24, 20, 36, 255);
                    border_color = IM_COL32(140, 70, 220, 180);
                } else if (mode == 5) { // BEAT JUMP
                    snprintf(main_txt, sizeof(main_txt), "%s", jump_labels[p]);
                    bool is_fwd = (p % 2 == 1);
                    pad_color = is_fwd ? IM_COL32(36, 22, 6, 255) : IM_COL32(10, 26, 38, 255);
                    border_color = is_fwd ? IM_COL32(255, 150, 0, 200) : IM_COL32(0, 190, 235, 200);
                }

                char p_id[32];
                snprintf(p_id, sizeof(p_id), "##RekPad_%d_%d", deck.deck_id, p);
                ImGui::SetCursorScreenPos(p_min);
                ImGui::InvisibleButton(p_id, ImVec2(pad_w, pad_h));

                if (ImGui::IsItemClicked(0)) {
                    if (mode == 0) {
                        if (deck.hot_cues[p].active) deck.jumpToHotCue(p);
                        else deck.setHotCue(p);
                    } else if (mode == 1) {
                        deck.toggleLoop(loop_beats[p]);
                    } else if (mode == 2) {
                        if (p == 0) deck.toggleLoop(0.5);
                        else if (p == 1) deck.toggleLoop(1.0);
                        else if (p == 2) { engine.beat_fx.type = 0; engine.beat_fx.enabled = !engine.beat_fx.enabled; }
                        else if (p == 3) { engine.beat_fx.type = 1; engine.beat_fx.enabled = !engine.beat_fx.enabled; }
                        else if (p == 4) deck.sound_color_fx_mode = (deck.sound_color_fx_mode == 0 ? -1 : 0);
                        else if (p == 5) deck.sound_color_fx_mode = (deck.sound_color_fx_mode == 2 ? -1 : 2);
                        else if (p == 6) { engine.beat_fx.type = 2; engine.beat_fx.enabled = !engine.beat_fx.enabled; }
                        else if (p == 7) engine.beat_fx.enabled = !engine.beat_fx.enabled;
                    } else if (mode == 3) {
                        engine.triggerSampler(p);
                    } else if (mode == 4) {
                        if (p == 0) deck.toggleStemVocal();
                        else if (p == 1) deck.toggleStemMelody();
                        else if (p == 2) deck.toggleStemDrums();
                        else if (p == 3) deck.soloStemVocal();
                        else if (p == 4) deck.triggerVocalEchoOut();
                        else if (p == 5) deck.soloStemMelody();
                        else if (p == 6) deck.toggleStemDrums();
                        else if (p == 7) deck.resetStems();
                    } else if (mode == 5) {
                        deck.beatJump(jump_beats[p]);
                    }
                } else if (ImGui::IsItemClicked(1)) {
                    if (mode == 0) {
                        deck.deleteHotCue(p);
                    }
                }

                bool hov = ImGui::IsItemHovered();
                bool held = ImGui::IsItemActive();

                if (hov && !active) {
                    pad_color = IM_COL32(32, 42, 58, 255);
                    border_color = IM_COL32(0, 180, 255, 240);
                }

                // Cova do socket e tecla de silicone com deslocamento físico
                dl->AddRectFilled(ImVec2(p_min.x - 1.0f, p_min.y - 1.0f), ImVec2(p_max.x + 1.0f, p_max.y + 1.0f), IM_COL32(7, 9, 14, 255), 4.0f);
                dl->AddRect(ImVec2(p_min.x - 1.0f, p_min.y - 1.0f), ImVec2(p_max.x + 1.0f, p_max.y + 1.0f), IM_COL32(20, 26, 36, 180), 4.0f, 0, 1.0f);

                float press_dy = held ? 1.0f : 0.0f;
                ImVec2 c_min(p_min.x + 0.5f, p_min.y + 0.5f + press_dy);
                ImVec2 c_max(p_max.x - 0.5f, p_max.y - 0.5f + press_dy);

                if (active) {
                    ImVec4 c_v4 = ImGui::ColorConvertU32ToFloat4(pad_color);
                    ImU32 halo = ImGui::ColorConvertFloat4ToU32(ImVec4(c_v4.x, c_v4.y, c_v4.z, 0.28f));
                    dl->AddRectFilled(ImVec2(c_min.x - 2.5f, c_min.y - 2.5f), ImVec2(c_max.x + 2.5f, c_max.y + 2.5f), halo, 5.0f);
                }

                dl->AddRectFilled(c_min, c_max, pad_color, 3.0f);
                if (held) {
                    dl->AddRectFilled(c_min, c_max, IM_COL32(255, 255, 255, 60), 3.0f);
                } else if (hov) {
                    dl->AddRectFilled(c_min, c_max, IM_COL32(255, 255, 255, 22), 3.0f);
                }
                dl->AddRect(c_min, c_max, border_color, 3.0f, 0, active ? 1.5f : (hov ? 1.2f : 1.0f));

                // Chanfros 3D
                dl->AddLine(ImVec2(c_min.x + 2.0f, c_min.y + 1.0f), ImVec2(c_max.x - 2.0f, c_min.y + 1.0f), 
                            active ? IM_COL32(255, 255, 255, 180) : IM_COL32(255, 255, 255, 50), 1.0f);
                dl->AddLine(ImVec2(c_min.x + 2.0f, c_max.y - 0.5f), ImVec2(c_max.x - 2.0f, c_max.y - 0.5f), 
                            IM_COL32(4, 6, 9, 230), 1.2f);

                // Tipografia Profissional com Auto-Fit e Proteção Universal contra Colisão
                float font_sz = (mode == 0) ? 14.0f : ((mode == 1) ? 12.0f : 10.5f);
                ImU32 txt_col = active ? IM_COL32(255, 255, 255, 255) : IM_COL32(220, 230, 245, 240);

                if (sub_txt[0] != '\\0' && pad_w >= 44.0f) {
                    ImVec2 top_min(c_min.x + 2.0f, c_min.y + 2.0f);
                    ImVec2 top_max(c_max.x - 2.0f, c_min.y + pad_h * 0.60f);
                    DJLayoutEditorUI::DrawFittedText(dl, f_bld, font_sz, top_min, top_max, txt_col, main_txt, 2.0f, 1.0f);

                    ImVec2 bot_min(c_min.x + 2.0f, c_min.y + pad_h * 0.55f);
                    ImVec2 bot_max(c_max.x - 2.0f, c_max.y - 2.0f);
                    DJLayoutEditorUI::DrawFittedText(dl, f_reg, 9.5f, bot_min, bot_max, active ? IM_COL32(255, 255, 255, 220) : IM_COL32(140, 160, 185, 200), sub_txt, 2.0f, 1.0f);
                } else {
                    DJLayoutEditorUI::DrawFittedText(dl, f_bld, font_sz, c_min, c_max, txt_col, main_txt, 2.0f, 1.0f);
                }
            }

            // Barra Inferior de Modo com Auto-Fit
            float drop_y = pos.y + pad_h * (float)num_rows + pad_gap_y * (float)(num_rows - 1) + 3.0f;
            float mode_btn_w = std::clamp(w * 0.08f, 24.0f, 38.0f);
            float combo_w = std::clamp(w * 0.32f, 85.0f, 130.0f);

            const char* mode_titles[] = { "HOT CUE", "BEAT LOOP", "PAD FX 1", "SAMPLER", "STEMS", "BEAT JUMP" };
            const char* cur_mode_title = mode_titles[mode];

            ImVec2 drop_pos = ImVec2(pos.x, drop_y);
            ImVec2 drop_max = ImVec2(drop_pos.x + combo_w, drop_pos.y + drop_h);
            dl->AddRectFilled(drop_pos, drop_max, IM_COL32(12, 16, 22, 255), 3.0f);
            dl->AddRect(drop_pos, drop_max, IM_COL32(32, 42, 58, 220), 3.0f, 0, 1.0f);

            DJLayoutEditorUI::DrawFittedText(dl, f_bld, 11.0f, drop_pos, drop_max,
                                             (mode == 5) ? IM_COL32(255, 170, 0, 255) : IM_COL32(0, 190, 255, 255),
                                             cur_mode_title, 4.0f, 1.0f);

            char drop_btn_id[32]; snprintf(drop_btn_id, sizeof(drop_btn_id), "##PadModeDrop_%d", deck.deck_id);
            ImGui::SetCursorScreenPos(drop_pos);
            if (ImGui::InvisibleButton(drop_btn_id, ImVec2(combo_w, drop_h))) {
                setPadMode(deck.deck_id, (mode + 1) % 6, engine);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Modo Atual: %s (Clique para alternar)", cur_mode_title);

            // Icones [::], [|||], [FX], [SMP], [STM], [JMP]
            const char* ic_labels[6] = { "::", "|||", "FX", "SMP", "STM", "JMP" };
            for (int im = 0; im < 6; im++) {
                float ic_x = drop_pos.x + combo_w + 3.0f + (float)im * (mode_btn_w + 2.5f);
                if (ic_x + mode_btn_w > pos.x + w) break;
                ImVec2 ic_min = ImVec2(ic_x, drop_y);
                ImVec2 ic_max = ImVec2(ic_x + mode_btn_w, drop_y + drop_h);

                bool is_active_m = (mode == im);
                dl->AddRectFilled(ic_min, ic_max, is_active_m ? IM_COL32(0, 110, 220, 255) : IM_COL32(16, 20, 28, 255), 3.0f);
                dl->AddRect(ic_min, ic_max, is_active_m ? IM_COL32(255, 255, 255, 220) : IM_COL32(36, 48, 64, 220), 3.0f, 0, 1.0f);

                DJLayoutEditorUI::DrawFittedText(dl, f_reg, 10.5f, ic_min, ic_max, IM_COL32(220, 235, 255, 240), ic_labels[im], 1.0f, 1.0f);

                char ic_id[32]; snprintf(ic_id, sizeof(ic_id), "##PadModBtn_%d_%d", deck.deck_id, im);
                ImGui::SetCursorScreenPos(ic_min);
                if (ImGui::InvisibleButton(ic_id, ImVec2(mode_btn_w, drop_h))) {
                    setPadMode(deck.deck_id, im, engine);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Mudar para modo %s", mode_titles[im]);
            }
        }\n\n        """

c = c[:idx_start] + new_pads_func + c[idx_end:]

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

print("renderRekordboxPads successfully updated for 16 Pads Mode and anti-collision!")
