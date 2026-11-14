import os
import sys

UI_HEADER = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h"

with open(UI_HEADER, "r", encoding="utf-8") as f:
    text = f.read()

# 1. Fix custom_pads enum scope
old_init = """            for (int d = 0; d < 4; d++) {
                custom_pads[d][0] = { KuroAudio::FX_SLIP_ROLL, "SLIP ROLL", false, false, 0.85f, 0.5f };
                custom_pads[d][1] = { KuroAudio::FX_VINYL_BRAKE, "VINYL BRK", false, false, 0.90f, 1.0f };
                custom_pads[d][2] = { KuroAudio::FX_ENIGMA_JET, "ENIGMA JET", false, false, 0.80f, 1.0f };
                custom_pads[d][3] = { KuroAudio::FX_HELIX, "HELIX", false, false, 0.80f, 1.0f };
                custom_pads[d][4] = { KuroAudio::FX_MOBIUS_SAW, "MOBIUS SAW", false, false, 0.75f, 1.0f };
                custom_pads[d][5] = { KuroAudio::FX_PITCH, "PITCH", false, false, 0.75f, 1.0f };
                custom_pads[d][6] = { KuroAudio::FX_REVERB_UP, "REVERB UP", false, false, 0.85f, 1.0f };
                custom_pads[d][7] = { KuroAudio::FX_LOW_CUT_ECHO, "LOW CUT", true, false, 0.85f, 0.75f };
            }"""

new_init = """            for (int d = 0; d < 4; d++) {
                custom_pads[d][0] = { KuroAudio::DJBeatFXUnit::FX_SLIP_ROLL, "SLIP ROLL", false, false, 0.85f, 0.5f };
                custom_pads[d][1] = { KuroAudio::DJBeatFXUnit::FX_VINYL_BRAKE, "VINYL BRK", false, false, 0.90f, 1.0f };
                custom_pads[d][2] = { KuroAudio::DJBeatFXUnit::FX_ENIGMA_JET, "ENIGMA JET", false, false, 0.80f, 1.0f };
                custom_pads[d][3] = { KuroAudio::DJBeatFXUnit::FX_HELIX, "HELIX", false, false, 0.80f, 1.0f };
                custom_pads[d][4] = { KuroAudio::DJBeatFXUnit::FX_MOBIUS_SAW, "MOBIUS SAW", false, false, 0.75f, 1.0f };
                custom_pads[d][5] = { KuroAudio::DJBeatFXUnit::FX_PITCH, "PITCH", false, false, 0.75f, 1.0f };
                custom_pads[d][6] = { KuroAudio::DJBeatFXUnit::FX_REVERB_UP, "REVERB UP", false, false, 0.85f, 1.0f };
                custom_pads[d][7] = { KuroAudio::DJBeatFXUnit::FX_LOW_CUT_ECHO, "LOW CUT", true, false, 0.85f, 0.75f };
            }"""

if old_init in text:
    text = text.replace(old_init, new_init, 1)
    print("1. Fixed custom_pads enum scope.")
else:
    print("Warning: old_init not found!")

# 2. Fix hotkeys
old_hotkeys = """                // Beat FX Toggle / Hold: Tecla B
                if (ImGui::IsKeyPressed(ImGuiKey_B, false)) {
                    if (ImGui::GetIO().KeyShift) {
                        rekordbox_fx1_hold = true;
                        engine.beat_fx[0].enabled = true;
                    } else {
                        engine.beat_fx[0].enabled = !engine.beat_fx[0].enabled;
                    }
                }
                if (ImGui::IsKeyReleased(ImGuiKey_B) && rekordbox_fx1_hold) {
                    rekordbox_fx1_hold = false;
                    engine.beat_fx[0].enabled = false;
                }

                // TAP TEMPO: Tecla T
                if (ImGui::IsKeyPressed(ImGuiKey_T, false)) {
                    double now_s = ImGui::GetTime();
                    double tap_bpm = tap_trackers[0].registerTap(now_s);
                    if (tap_bpm > 30.0) {
                        engine.beat_fx[0].bpm = tap_bpm;
                        engine.beat_fx[0].auto_bpm = false;
                    }
                }

                // Fração de Batida (Beat < e Beat >): Teclas - e = (ou +)
                if (ImGui::IsKeyPressed(ImGuiKey_Minus, false)) {
                    rekordbox_fx1_beat_idx = std::max(0, rekordbox_fx1_beat_idx - 1);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_Equal, false)) {
                    rekordbox_fx1_beat_idx = std::min(5, rekordbox_fx1_beat_idx + 1);
                }

                // FX Depth / Dry-Wet: Ctrl + Seta Cima / Seta Baixo
                if (ImGui::GetIO().KeyCtrl) {
                    if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
                        rekordbox_fx1_param = std::min(1.0f, rekordbox_fx1_param + 0.015f);
                    }
                    if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
                        rekordbox_fx1_param = std::max(0.0f, rekordbox_fx1_param - 0.015f);
                    }
                }

                // Disparadores rápidos de Efeitos (F2 a F6):
                if (ImGui::IsKeyPressed(ImGuiKey_F2, false)) {
                    // F2: Vinyl Brake
                    int d = engine.getMasterDeckId();
                    engine.decks[d].is_stopping_vinyl = true;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F3, false)) {
                    // F3: Slip Roll
                    int d = engine.getMasterDeckId();
                    engine.decks[d].triggerSlipRoll(0.5);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F4, false)) {
                    // F4: Echo
                    engine.beat_fx[0].selected_fx = 0;
                    engine.beat_fx[0].enabled = !engine.beat_fx[0].enabled;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) {
                    // F5: Reverb
                    engine.beat_fx[0].selected_fx = 1;
                    engine.beat_fx[0].enabled = !engine.beat_fx[0].enabled;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F6, false)) {
                    // F6: Spiral
                    engine.beat_fx[0].selected_fx = 4;
                    engine.beat_fx[0].enabled = !engine.beat_fx[0].enabled;
                }"""

new_hotkeys = """                // Beat FX Toggle / Hold: Tecla B
                if (ImGui::IsKeyPressed(ImGuiKey_B, false)) {
                    if (ImGui::GetIO().KeyShift) {
                        rekordbox_fx1_hold = true;
                        engine.beat_fx.enabled = true;
                    } else {
                        engine.beat_fx.enabled = !engine.beat_fx.enabled;
                    }
                }
                if (ImGui::IsKeyReleased(ImGuiKey_B) && rekordbox_fx1_hold) {
                    rekordbox_fx1_hold = false;
                    engine.beat_fx.enabled = false;
                }

                // TAP TEMPO: Tecla T
                if (ImGui::IsKeyPressed(ImGuiKey_T, false)) {
                    double now_s = ImGui::GetTime();
                    double tap_bpm = tap_trackers[0].registerTap(now_s);
                    if (tap_bpm > 30.0) {
                        engine.beat_fx.bpm = tap_bpm;
                        engine.beat_fx.auto_bpm = false;
                    }
                }

                // Fração de Batida (Beat < e Beat >): Teclas - e = (ou +)
                if (ImGui::IsKeyPressed(ImGuiKey_Minus, false)) {
                    rekordbox_fx1_beat_idx = std::max(0, rekordbox_fx1_beat_idx - 1);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_Equal, false)) {
                    rekordbox_fx1_beat_idx = std::min(5, rekordbox_fx1_beat_idx + 1);
                }

                // FX Depth / Dry-Wet: Ctrl + Seta Cima / Seta Baixo
                if (ImGui::GetIO().KeyCtrl) {
                    if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
                        rekordbox_fx1_param = std::min(1.0f, rekordbox_fx1_param + 0.015f);
                    }
                    if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
                        rekordbox_fx1_param = std::max(0.0f, rekordbox_fx1_param - 0.015f);
                    }
                }

                // Disparadores rápidos de Efeitos (F2 a F6):
                if (ImGui::IsKeyPressed(ImGuiKey_F2, false)) {
                    // F2: Vinyl Brake
                    int d = engine.getMasterDeckId();
                    engine.decks[d].is_stopping_vinyl = true;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F3, false)) {
                    // F3: Slip Roll
                    int d = engine.getMasterDeckId();
                    engine.decks[d].triggerSlipRoll(0.5);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F4, false)) {
                    // F4: Echo
                    engine.beat_fx.type = KuroAudio::DJBeatFXUnit::FX_ECHO;
                    engine.beat_fx.enabled = !engine.beat_fx.enabled;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) {
                    // F5: Reverb
                    engine.beat_fx.type = KuroAudio::DJBeatFXUnit::FX_REVERB;
                    engine.beat_fx.enabled = !engine.beat_fx.enabled;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_F6, false)) {
                    // F6: Spiral
                    engine.beat_fx.type = KuroAudio::DJBeatFXUnit::FX_SPIRAL;
                    engine.beat_fx.enabled = !engine.beat_fx.enabled;
                }"""

if old_hotkeys in text:
    text = text.replace(old_hotkeys, new_hotkeys, 1)
    print("2. Fixed hotkeys.")
else:
    print("Warning: old_hotkeys not found!")

# 3. Add applyCustomPadFX method before renderPerformancePads
apply_custom_fx_code = """        void applyCustomPadFX(KuroAudio::DJDeck& deck, int pad_idx, const CustomPadConfig& cpad, KuroAudio::DJEngine& engine) {
            auto& fx_unit = (deck.deck_id % 2 == 0) ? engine.beat_fx : engine.beat_fx2;
            if (cpad.is_active) {
                if (cpad.fx_type == KuroAudio::DJBeatFXUnit::FX_SLIP_ROLL) {
                    deck.triggerSlipRoll(cpad.beat_fraction);
                } else if (cpad.fx_type == KuroAudio::DJBeatFXUnit::FX_VINYL_BRAKE) {
                    deck.is_stopping_vinyl = true;
                } else {
                    fx_unit.channel_assign = deck.deck_id;
                    fx_unit.type = cpad.fx_type;
                    fx_unit.dry_wet = cpad.depth;
                    fx_unit.beat_fraction = cpad.beat_fraction;
                    fx_unit.enabled = true;
                }
            } else {
                if (cpad.fx_type == KuroAudio::DJBeatFXUnit::FX_SLIP_ROLL) {
                    if (deck.pad_roll_active) deck.releasePadRoll();
                } else if (cpad.fx_type != KuroAudio::DJBeatFXUnit::FX_VINYL_BRAKE) {
                    bool any_other = false;
                    for (int p = 0; p < 8; p++) {
                        if (p != pad_idx && custom_pads[deck.deck_id][p].is_active) {
                            any_other = true;
                            break;
                        }
                    }
                    if (!any_other) {
                        fx_unit.enabled = false;
                    }
                }
            }
        }

        void renderPerformancePads"""

if "void applyCustomPadFX(" not in text:
    text = text.replace("        void renderPerformancePads", apply_custom_fx_code, 1)
    print("3. Added applyCustomPadFX.")

# 4. Update renderPerformancePads to include CUSTOM mode (7 modes)
old_pads_fn = """        void renderPerformancePads(KuroAudio::DJDeck& deck, float w, ImVec4 theme_col, KuroAudio::DJEngine& engine) {
            const char* modes[] = { "HOT CUE", "BEAT LOOP", "PAD FX 1", "SAMPLER", "STEMS", "BEAT JUMP" };
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "PAD MODE:");
            ImGui::SameLine();
            for (int m = 0; m < 6; m++) {
                bool is_cur = (rekordbox_pad_mode[deck.deck_id] == m);
                if (is_cur) ImGui::PushStyleColor(ImGuiCol_Button, theme_col);
                else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.14f, 0.20f, 0.8f));

                if (ImGui::Button(modes[m], ImVec2(54, 17))) {
                    setPadMode(deck.deck_id, m, engine);
                }
                ImGui::PopStyleColor();
                ImGui::SameLine(0, 3);
            }
            ImGui::NewLine();

            float pad_w = (w - 24.0f) * 0.25f;
            float pad_h = 19.0f;
            int cur_m = std::clamp(rekordbox_pad_mode[deck.deck_id], 0, 5);

            const double loop_beats_tbl[8] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0 };
            const char* loop_lbls[8] = { "1/4", "1/2", "1", "2", "4", "8", "16", "32" };
            const double jump_beats_tbl[8] = { -1.0, 1.0, -2.0, 2.0, -4.0, 4.0, -8.0, 8.0 };
            const char* jump_lbls[8] = { "< 1", "1 >", "< 2", "2 >", "< 4", "4 >", "< 8", "8 >" };
            const char* stem_lbls[8] = { "VOC", "MEL", "DRUM", "SOLO", "ECHO", "INST", "BRK", "RST" };

            for (int i = 0; i < 8; i++) {
                ImGui::PushID(i);
                uint32_t col = deck.hot_cues[i].color_rgba;
                bool is_active = false;
                if (cur_m == 0) is_active = deck.hot_cues[i].active;
                else if (cur_m == 1) is_active = (deck.loop_active && std::abs(deck.loop_beats - loop_beats_tbl[i]) < 0.001);
                else if (cur_m == 3) is_active = (i < 8 && engine.samplers[i].is_playing);
                else if (cur_m == 4) {
                    if (i == 0) is_active = deck.stem_vocal_active.load();
                    else if (i == 1) is_active = deck.stem_melody_active.load();
                    else if (i == 2) is_active = deck.stem_drum_active.load();
                }

                if (is_active) {
                    ImVec4 pad_c = (cur_m == 1) ? ImVec4(1.0f, 0.65f, 0.05f, 1.0f) : ImGui::ColorConvertU32ToFloat4(col);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(pad_c.x * 0.75f, pad_c.y * 0.75f, pad_c.z * 0.75f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.65f, 0.7f, 1.0f));
                }

                char lbl[32];
                if (cur_m == 0) snprintf(lbl, sizeof(lbl), is_active ? "CUE %d" : "[ %d ]", i + 1);
                else if (cur_m == 1) snprintf(lbl, sizeof(lbl), "%s", loop_lbls[i]);
                else if (cur_m == 2) snprintf(lbl, sizeof(lbl), "FX %d", i + 1);
                else if (cur_m == 3) snprintf(lbl, sizeof(lbl), "%s", engine.samplers[i].name.c_str());
                else if (cur_m == 4) snprintf(lbl, sizeof(lbl), "%s", stem_lbls[i]);
                else snprintf(lbl, sizeof(lbl), "%s", jump_lbls[i]);

                if (ImGui::Button(lbl, ImVec2(pad_w, pad_h))) {
                    if (cur_m == 0) {
                        if (deck.hot_cues[i].active) deck.jumpToHotCue(i);
                        else deck.setHotCue(i);
                    } else if (cur_m == 1) {
                        deck.toggleLoop(loop_beats_tbl[i]);
                    } else if (cur_m == 2) {
                        if (i == 0) deck.toggleLoop(0.5);
                        else if (i == 1) deck.toggleLoop(1.0);
                        else engine.beat_fx.enabled = !engine.beat_fx.enabled;
                    } else if (cur_m == 3) {
                        engine.triggerSampler(i);
                    } else if (cur_m == 4) {
                        if (i == 0) deck.toggleStemVocal();
                        else if (i == 1) deck.toggleStemMelody();
                        else if (i == 2) deck.toggleStemDrums();
                        else if (i == 7) deck.resetStems();
                    } else if (cur_m == 5) {
                        deck.beatJump(jump_beats_tbl[i]);
                    }
                }

                if (cur_m == 0 && ImGui::IsItemClicked(1)) {
                    deck.deleteHotCue(i);
                }

                ImGui::PopStyleColor(2);
                ImGui::PopID();

                if ((i + 1) % 4 != 0) ImGui::SameLine(0, 4);
            }
        }"""

new_pads_fn = """        void renderPerformancePads(KuroAudio::DJDeck& deck, float w, ImVec4 theme_col, KuroAudio::DJEngine& engine) {
            const char* modes[] = { "HOT CUE", "BEAT LOOP", "PAD FX 1", "SAMPLER", "STEMS", "BEAT JUMP", "CUSTOM" };
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "PAD MODE:");
            ImGui::SameLine();
            for (int m = 0; m < 7; m++) {
                bool is_cur = (rekordbox_pad_mode[deck.deck_id] == m);
                if (is_cur) ImGui::PushStyleColor(ImGuiCol_Button, theme_col);
                else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.14f, 0.20f, 0.8f));

                if (ImGui::Button(modes[m], ImVec2(m == 6 ? 56 : 50, 17))) {
                    setPadMode(deck.deck_id, m, engine);
                }
                ImGui::PopStyleColor();
                ImGui::SameLine(0, 3);
            }
            ImGui::NewLine();

            float pad_w = (w - 24.0f) * 0.25f;
            float pad_h = 19.0f;
            int cur_m = std::clamp(rekordbox_pad_mode[deck.deck_id], 0, 6);

            const double loop_beats_tbl[8] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0 };
            const char* loop_lbls[8] = { "1/4", "1/2", "1", "2", "4", "8", "16", "32" };
            const double jump_beats_tbl[8] = { -1.0, 1.0, -2.0, 2.0, -4.0, 4.0, -8.0, 8.0 };
            const char* jump_lbls[8] = { "< 1", "1 >", "< 2", "2 >", "< 4", "4 >", "< 8", "8 >" };
            const char* stem_lbls[8] = { "VOC", "MEL", "DRUM", "SOLO", "ECHO", "INST", "BRK", "RST" };

            for (int i = 0; i < 8; i++) {
                ImGui::PushID(i);
                uint32_t col = deck.hot_cues[i].color_rgba;
                bool is_active = false;
                if (cur_m == 0) is_active = deck.hot_cues[i].active;
                else if (cur_m == 1) is_active = (deck.loop_active && std::abs(deck.loop_beats - loop_beats_tbl[i]) < 0.001);
                else if (cur_m == 3) is_active = (i < 8 && engine.samplers[i].is_playing);
                else if (cur_m == 4) {
                    if (i == 0) is_active = deck.stem_vocal_active.load();
                    else if (i == 1) is_active = deck.stem_melody_active.load();
                    else if (i == 2) is_active = deck.stem_drum_active.load();
                } else if (cur_m == 6) {
                    is_active = custom_pads[deck.deck_id][i].is_active;
                }

                if (is_active) {
                    ImVec4 pad_c = (cur_m == 1) ? ImVec4(1.0f, 0.65f, 0.05f, 1.0f) :
                                   (cur_m == 6) ? ImVec4(0.0f, 0.95f, 0.85f, 1.0f) : ImGui::ColorConvertU32ToFloat4(col);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(pad_c.x * 0.75f, pad_c.y * 0.75f, pad_c.z * 0.75f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.65f, 0.7f, 1.0f));
                }

                char lbl[32];
                if (cur_m == 0) snprintf(lbl, sizeof(lbl), is_active ? "CUE %d" : "[ %d ]", i + 1);
                else if (cur_m == 1) snprintf(lbl, sizeof(lbl), "%s", loop_lbls[i]);
                else if (cur_m == 2) snprintf(lbl, sizeof(lbl), "FX %d", i + 1);
                else if (cur_m == 3) snprintf(lbl, sizeof(lbl), "%s", engine.samplers[i].name.c_str());
                else if (cur_m == 4) snprintf(lbl, sizeof(lbl), "%s", stem_lbls[i]);
                else if (cur_m == 6) snprintf(lbl, sizeof(lbl), "%s", custom_pads[deck.deck_id][i].name.c_str());
                else snprintf(lbl, sizeof(lbl), "%s", jump_lbls[i]);

                if (cur_m == 6) {
                    auto& cpad = custom_pads[deck.deck_id][i];
                    if (cpad.is_toggle) {
                        if (ImGui::Button(lbl, ImVec2(pad_w, pad_h))) {
                            cpad.is_active = !cpad.is_active;
                            applyCustomPadFX(deck, i, cpad, engine);
                        }
                    } else {
                        // Modo Hold / Gate
                        ImGui::Button(lbl, ImVec2(pad_w, pad_h));
                        bool held = ImGui::IsItemActive();
                        if (held != cpad.is_active) {
                            cpad.is_active = held;
                            applyCustomPadFX(deck, i, cpad, engine);
                        }
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Custom FX Pad %d: %s [%s]\\n(Clique c/ botao direito para configurar)", 
                                          i + 1, cpad.name.c_str(), cpad.is_toggle ? "TOGGLE" : "HOLD");
                    }
                    if (ImGui::BeginPopupContextItem()) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.85f, 1.0f), "CONFIGURAR PAD %d (%s)", i + 1, cpad.name.c_str());
                        ImGui::Separator();
                        ImGui::Checkbox("Modo Alternancia (Toggle)", &cpad.is_toggle);
                        ImGui::SliderFloat("Intensidade (Dry/Wet)", &cpad.depth, 0.1f, 1.0f, "%.2f");
                        
                        // Selecao de Tipo de Efeito
                        if (ImGui::BeginCombo("Efeito", KuroAudio::DJBeatFXUnit::getFXName(cpad.fx_type))) {
                            for (int fx = 0; fx < KuroAudio::DJBeatFXUnit::FX_COUNT; fx++) {
                                bool is_sel = (cpad.fx_type == fx);
                                if (ImGui::Selectable(KuroAudio::DJBeatFXUnit::getFXName(fx), is_sel)) {
                                    cpad.fx_type = fx;
                                    cpad.name = KuroAudio::DJBeatFXUnit::getFXName(fx);
                                }
                                if (is_sel) ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        ImGui::EndPopup();
                    }
                } else {
                    if (ImGui::Button(lbl, ImVec2(pad_w, pad_h))) {
                        if (cur_m == 0) {
                            if (deck.hot_cues[i].active) deck.jumpToHotCue(i);
                            else deck.setHotCue(i);
                        } else if (cur_m == 1) {
                            deck.toggleLoop(loop_beats_tbl[i]);
                        } else if (cur_m == 2) {
                            if (i == 0) deck.toggleLoop(0.5);
                            else if (i == 1) deck.toggleLoop(1.0);
                            else engine.beat_fx.enabled = !engine.beat_fx.enabled;
                        } else if (cur_m == 3) {
                            engine.triggerSampler(i);
                        } else if (cur_m == 4) {
                            if (i == 0) deck.toggleStemVocal();
                            else if (i == 1) deck.toggleStemMelody();
                            else if (i == 2) deck.toggleStemDrums();
                            else if (i == 7) deck.resetStems();
                        } else if (cur_m == 5) {
                            deck.beatJump(jump_beats_tbl[i]);
                        }
                    }

                    if (cur_m == 0 && ImGui::IsItemClicked(1)) {
                        deck.deleteHotCue(i);
                    }
                }

                ImGui::PopStyleColor(2);
                ImGui::PopID();

                if ((i + 1) % 4 != 0) ImGui::SameLine(0, 4);
            }
        }"""

if old_pads_fn in text:
    text = text.replace(old_pads_fn, new_pads_fn, 1)
    print("4. Updated renderPerformancePads with CUSTOM tab.")
else:
    print("Warning: old_pads_fn not found!")

with open(UI_HEADER, "w", encoding="utf-8") as f:
    f.write(text)

print("KuroDJStudioUI.h patch part 3 finished.")

