import sys
import os

path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h'

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# -----------------------------------------------------------------------------
# 1. Update zoomSideBtn in renderStackedRekordboxWaveforms
# -----------------------------------------------------------------------------
old_side_btn = """            // Botoes de Controle / Zoom Verticais na lateral esquerda (^, v, +, RST, -, 1L/2L)
            float ctrl_w = 26.0f;
            float wave_x = pos.x + ctrl_w;
            float wave_w = w - ctrl_w;
            float center_x = wave_x + wave_w * 0.5f;
            const char* hot_cue_letters[8] = { "A", "B", "C", "D", "E", "F", "G", "H" };

            dl->AddRectFilled(pos, ImVec2(pos.x + ctrl_w, pos.y + total_stack_h), IM_COL32(12, 15, 20, 255));
            dl->AddLine(ImVec2(pos.x + ctrl_w, pos.y), ImVec2(pos.x + ctrl_w, pos.y + total_stack_h), IM_COL32(24, 30, 42, 255), 1.0f);

            auto zoomSideBtn = [&](const char* id, const char* lbl, float y_off, auto on_click) {
                ImVec2 bp = ImVec2(pos.x + 2.0f, pos.y + y_off);
                dl->AddRectFilled(bp, ImVec2(bp.x + 22.0f, bp.y + 16.0f), IM_COL32(20, 25, 34, 255), 2.0f);
                dl->AddRect(bp, ImVec2(bp.x + 22.0f, bp.y + 16.0f), IM_COL32(38, 48, 65, 200), 2.0f, 0, 1.0f);
                ImVec2 sz = ImGui::CalcTextSize(lbl);
                dl->AddText(ImVec2(bp.x + (22.0f - sz.x) * 0.5f, bp.y + 1.0f), IM_COL32(190, 205, 225, 240), lbl);
                ImGui::SetCursorScreenPos(bp);
                if (ImGui::InvisibleButton(id, ImVec2(22.0f, 16.0f))) on_click();
            };"""

new_side_btn = """            // Botoes de Controle / Zoom Verticais na lateral esquerda com Ergonomia Touch & Auto-Fit
            auto& layout_metrics = KuroUI::DJLayoutMetrics::getInstance();
            float ctrl_w = std::clamp(layout_metrics.waveform_toolbar_width, 32.0f, 60.0f);
            float btn_w = ctrl_w - 4.0f;
            float btn_h = 24.0f;
            float wave_x = pos.x + ctrl_w;
            float wave_w = w - ctrl_w;
            float center_x = wave_x + wave_w * 0.5f;
            const char* hot_cue_letters[8] = { "A", "B", "C", "D", "E", "F", "G", "H" };

            dl->AddRectFilled(pos, ImVec2(pos.x + ctrl_w, pos.y + total_stack_h), IM_COL32(12, 15, 20, 255));
            dl->AddLine(ImVec2(pos.x + ctrl_w, pos.y), ImVec2(pos.x + ctrl_w, pos.y + total_stack_h), IM_COL32(24, 30, 42, 255), 1.0f);

            auto zoomSideBtn = [&](const char* id, const char* lbl, float y_off, auto on_click) {
                ImVec2 bp = ImVec2(pos.x + 2.0f, pos.y + y_off);
                ImVec2 bp_max = ImVec2(bp.x + btn_w, bp.y + btn_h);
                ImGui::SetCursorScreenPos(bp);
                bool clicked = ImGui::InvisibleButton(id, ImVec2(btn_w, btn_h));
                bool is_hov = ImGui::IsItemHovered();
                bool is_act = ImGui::IsItemActive();
                if (clicked) on_click();

                ImU32 bg = is_act ? IM_COL32(0, 110, 180, 255) : (is_hov ? IM_COL32(32, 42, 58, 255) : IM_COL32(18, 22, 30, 255));
                ImU32 border = is_act ? IM_COL32(0, 220, 255, 255) : (is_hov ? IM_COL32(0, 190, 240, 200) : IM_COL32(40, 50, 68, 200));

                dl->AddRectFilled(bp, bp_max, bg, 3.0f);
                dl->AddRect(bp, bp_max, border, 3.0f, 0, 1.0f);

                ImFont* f_btn = g_font_control_small ? g_font_control_small : ImGui::GetFont();
                DJLayoutEditorUI::DrawFittedText(dl, f_btn, 11.0f * layout_metrics.ui_font_scale, bp, bp_max,
                                                 is_act ? IM_COL32(255, 255, 255, 255) : (is_hov ? IM_COL32(0, 230, 255, 255) : IM_COL32(190, 205, 225, 240)),
                                                 lbl, 2.0f, 1.5f);
            };"""

assert old_side_btn in c, "old_side_btn not found"
c = c.replace(old_side_btn, new_side_btn)

# -----------------------------------------------------------------------------
# 2. Update CUE and PLAY positioning in renderRekordboxDeck
# -----------------------------------------------------------------------------
old_transport_pos = """            // Posicionamento vertical com espaçamento garantido 100% contido (CUE acima, PLAY abaixo)
            float cue_cy = lower_y + 4.0f + r_btn;
            float play_cy = lower_y + lower_h - 4.0f - r_btn;"""

new_transport_pos = """            // Posicionamento vertical com espaçamento mínimo rígido (CUE e PLAY agrupados, sempre visíveis)
            float t_cx = cur_left_x + transport_w * 0.5f + metrics.transport_offset_x;
            float gap_cue_play = std::clamp(metrics.cue_play_gap, 2.0f, 24.0f);
            float cue_cy = lower_y + 4.0f + r_btn + metrics.transport_offset_y;
            float play_cy = cue_cy + r_btn * 2.0f + gap_cue_play;"""

assert old_transport_pos in c, "old_transport_pos not found"
c = c.replace(old_transport_pos, new_transport_pos)

# -----------------------------------------------------------------------------
# 3. Update WIDGET_TRANSPORT Gizmo to include offset and safe bounds
# -----------------------------------------------------------------------------
old_transport_gizmo = """            DJLayoutEditorUI::DrawCanvasGizmo(WIDGET_TRANSPORT, "Play / Cue",
                                             ImVec2(cur_left_x - 2.0f, lower_y),
                                             ImVec2(cur_left_x + transport_w + 2.0f, lower_y + lower_h),
                                             nullptr, &metrics.transport_button_radius, 1.0f, 12.0f, 42.0f);"""

new_transport_gizmo = """            DJLayoutEditorUI::DrawCanvasGizmo(WIDGET_TRANSPORT, "Play / Cue",
                                             ImVec2(cur_left_x - 2.0f, lower_y),
                                             ImVec2(cur_left_x + transport_w + 2.0f, play_cy + r_btn + 4.0f),
                                             nullptr, &metrics.transport_button_radius, 1.0f, 12.0f, 48.0f,
                                             &metrics.transport_offset_x, &metrics.transport_offset_y);"""

assert old_transport_gizmo in c, "old_transport_gizmo not found"
c = c.replace(old_transport_gizmo, new_transport_gizmo)

# -----------------------------------------------------------------------------
# 4. Update Headphone Section in Central Mixer
# -----------------------------------------------------------------------------
old_hp_sec = """            // =========================================================================
            // BARRA SUPERIOR DE MONITORAMENTO DE FONES (HEADPHONE CUE SECTION)
            // Fica no topo do mixer onde há amplo espaço visual e acesso direto estilo Pioneer DJM
            // =========================================================================
            float hp_sec_y = pos.y + central_wave_h + 7.0f;
            float hp_sec_h = 22.0f;
            ImVec2 hp_rect_min = ImVec2(pos.x + 4.0f, hp_sec_y);
            ImVec2 hp_rect_max = ImVec2(pos.x + w - 4.0f, hp_sec_y + hp_sec_h);"""

new_hp_sec = """            // =========================================================================
            // BARRA SUPERIOR DE MONITORAMENTO DE FONES (HEADPHONE CUE SECTION - WIDGET_HEADPHONES)
            // Desacoplada e ajustavel para nunca cobrir o topo do mixer nem os knobs
            // =========================================================================
            auto& metrics = KuroUI::DJLayoutMetrics::getInstance();
            float hp_sec_y = pos.y + central_wave_h + 6.0f + metrics.headphone_offset_y;
            float hp_sec_h = std::clamp(metrics.headphone_bar_height, 20.0f, 44.0f);
            float hp_sec_w = std::clamp(metrics.headphone_bar_width, 160.0f, w - 8.0f);
            float hp_sec_x = pos.x + 4.0f + (w - 8.0f - hp_sec_w) * 0.5f + metrics.headphone_offset_x;
            ImVec2 hp_rect_min = ImVec2(hp_sec_x, hp_sec_y);
            ImVec2 hp_rect_max = ImVec2(hp_sec_x + hp_sec_w, hp_sec_y + hp_sec_h);"""

assert old_hp_sec in c, "old_hp_sec not found"
c = c.replace(old_hp_sec, new_hp_sec)

# -----------------------------------------------------------------------------
# 5. Update drawRekordboxKnob with Gold & Black Luxury styling for COMP
# -----------------------------------------------------------------------------
old_knob_body = """                // Corpo do knob (Dark Rekordbox Cap)
                dl->AddCircleFilled(center, radius + 1.0f, IM_COL32(10, 12, 16, 255), 24);
                dl->AddCircle(center, radius + 1.0f, is_active ? IM_COL32(0, 140, 240, 255) : (is_hovered ? IM_COL32(70, 85, 110, 255) : IM_COL32(38, 44, 55, 220)), 24, 1.2f);
                dl->AddCircleFilled(center, radius - 1.0f, IM_COL32(24, 28, 36, 255), 24);

                // Indicador radial de precisão (Âmbar Pioneer quando COMP > 0, Branco caso contrário)
                ImU32 ptr_col = (is_comp && *p_val > 0.01f) ? IM_COL32(255, 175, 45, 255) : IM_COL32(255, 255, 255, 255);
                ImVec2 p_in = ImVec2(center.x + std::cos(ang) * (radius * 0.22f), center.y + std::sin(ang) * (radius * 0.22f));
                ImVec2 p_out = ImVec2(center.x + std::cos(ang) * (radius - 1.2f), center.y + std::sin(ang) * (radius - 1.2f));
                dl->AddLine(p_in, p_out, ptr_col, 2.0f);"""

new_knob_body = """                // Corpo do knob: Gold & Black Luxury para COMP, Dark Rekordbox para os demais
                if (is_comp) {
                    // Aro chanfrado serrilhado de Ouro Champanhe usinado
                    dl->AddCircleFilled(center, radius + 3.0f, IM_COL32(235, 195, 65, 255), 32);
                    dl->AddCircle(center, radius + 3.0f, IM_COL32(185, 145, 30, 255), 32, 1.2f);
                    for (int g = 0; g < 16; g++) {
                        float ga = (float)g * (2.0f * (float)M_PI / 16.0f);
                        ImVec2 gp1(center.x + std::cos(ga) * (radius + 1.0f), center.y + std::sin(ga) * (radius + 1.0f));
                        ImVec2 gp2(center.x + std::cos(ga) * (radius + 3.0f), center.y + std::sin(ga) * (radius + 3.0f));
                        dl->AddLine(gp1, gp2, IM_COL32(130, 95, 15, 220), 1.0f);
                    }
                    // Miolo preto fosco acetinado Obsidian
                    dl->AddCircleFilled(center, radius, IM_COL32(14, 16, 20, 255), 32);
                    dl->AddCircle(center, radius, is_active ? IM_COL32(255, 220, 90, 255) : (is_hovered ? IM_COL32(215, 175, 45, 255) : IM_COL32(65, 52, 20, 240)), 32, 1.5f);
                    dl->AddCircleFilled(center, radius - 2.0f, IM_COL32(22, 24, 30, 255), 32);

                    // Ponteiro agulha Ouro Brilhante com ponta incandescente
                    ImU32 gold_ptr = IM_COL32(255, 220, 85, 255);
                    ImVec2 p_in = ImVec2(center.x + std::cos(ang) * (radius * 0.22f), center.y + std::sin(ang) * (radius * 0.22f));
                    ImVec2 p_out = ImVec2(center.x + std::cos(ang) * (radius - 1.2f), center.y + std::sin(ang) * (radius - 1.2f));
                    dl->AddLine(p_in, p_out, gold_ptr, 2.5f);
                    dl->AddCircleFilled(p_out, 1.5f, IM_COL32(255, 245, 180, 255));
                } else {
                    dl->AddCircleFilled(center, radius + 1.0f, IM_COL32(10, 12, 16, 255), 24);
                    dl->AddCircle(center, radius + 1.0f, is_active ? IM_COL32(0, 140, 240, 255) : (is_hovered ? IM_COL32(70, 85, 110, 255) : IM_COL32(38, 44, 55, 220)), 24, 1.2f);
                    dl->AddCircleFilled(center, radius - 1.0f, IM_COL32(24, 28, 36, 255), 24);

                    ImU32 ptr_col = IM_COL32(255, 255, 255, 255);
                    ImVec2 p_in = ImVec2(center.x + std::cos(ang) * (radius * 0.22f), center.y + std::sin(ang) * (radius * 0.22f));
                    ImVec2 p_out = ImVec2(center.x + std::cos(ang) * (radius - 1.2f), center.y + std::sin(ang) * (radius - 1.2f));
                    dl->AddLine(p_in, p_out, ptr_col, 2.0f);
                }"""

assert old_knob_body in c, "old_knob_body not found"
c = c.replace(old_knob_body, new_knob_body)

# -----------------------------------------------------------------------------
# 6. Swap TRIM and COMP in Mixer Strips (COMP 1st, TRIM 2nd)
# -----------------------------------------------------------------------------
old_strip_order = """                    // 1. TRIM
                    char t_id[32]; snprintf(t_id, sizeof(t_id), "##RekTrim_%d", id);
                    drawRekordboxKnob(t_id, "TRIM", &deck.gain, 0.0f, 2.0f, 1.0f, ch_cx, cy, k_r);

                    // 2. COMP (Pioneer DJM-V10 / DJM-A9)
                    cy += k_step;
                    char c_id[32]; snprintf(c_id, sizeof(c_id), "##RekComp_%d", id);
                    drawRekordboxKnob(c_id, "COMP", &deck.comp, 0.0f, 1.0f, 0.0f, ch_cx, cy, k_r, deck.compressor.gain_reduction, true);"""

new_strip_order = """                    // 1. COMP (PIONEER DJM-V10 / DJM-A9 - KNOB DOURADO & PRETO LUXURY DESTACADO)
                    float comp_r = std::clamp(k_r * metrics.comp_knob_scale, 8.0f, 28.0f);
                    char c_id[32]; snprintf(c_id, sizeof(c_id), "##RekComp_%d", id);
                    drawRekordboxKnob(c_id, "COMP", &deck.comp, 0.0f, 1.0f, 0.0f, ch_cx, cy, comp_r, deck.compressor.gain_reduction, true);

                    // 2. TRIM (GAIN)
                    cy += k_step;
                    char t_id[32]; snprintf(t_id, sizeof(t_id), "##RekTrim_%d", id);
                    drawRekordboxKnob(t_id, "TRIM", &deck.gain, 0.0f, 2.0f, 1.0f, ch_cx, cy, k_r);"""

assert old_strip_order in c, "old_strip_order not found"
c = c.replace(old_strip_order, new_strip_order)

# Also swap in compact mode
old_compact_order = """                    float cy = trim_cy;
                    char t_id[32]; snprintf(t_id, sizeof(t_id), "##RekTrim_%d", id);
                    drawRekordboxKnob(t_id, "TRIM", &deck.gain, 0.0f, 2.0f, 1.0f, knobs_cx, cy, k_r);

                    cy += k_step;
                    char c_id[32]; snprintf(c_id, sizeof(c_id), "##RekComp_%d", id);
                    drawRekordboxKnob(c_id, "COMP", &deck.comp, 0.0f, 1.0f, 0.0f, knobs_cx, cy, k_r, deck.compressor.gain_reduction, true);"""

new_compact_order = """                    float cy = trim_cy;
                    float comp_r = std::clamp(k_r * metrics.comp_knob_scale, 7.0f, 22.0f);
                    char c_id[32]; snprintf(c_id, sizeof(c_id), "##RekComp_%d", id);
                    drawRekordboxKnob(c_id, "COMP", &deck.comp, 0.0f, 1.0f, 0.0f, knobs_cx, cy, comp_r, deck.compressor.gain_reduction, true);

                    cy += k_step;
                    char t_id[32]; snprintf(t_id, sizeof(t_id), "##RekTrim_%d", id);
                    drawRekordboxKnob(t_id, "TRIM", &deck.gain, 0.0f, 2.0f, 1.0f, knobs_cx, cy, k_r);"""

assert old_compact_order in c, "old_compact_order not found"
c = c.replace(old_compact_order, new_compact_order)

# -----------------------------------------------------------------------------
# 7. Add WIDGET_HEADPHONES Gizmo and fix Knobs/Faders AABB collision boundaries
# -----------------------------------------------------------------------------
old_gizmos = """            DJLayoutEditorUI::DrawCanvasGizmo(WIDGET_KNOBS_EQ, "Knobs EQ",
                                             ImVec2(pos.x + 4.0f, div_y + 2.0f),
                                             ImVec2(pos.x + w - 4.0f, bot_sec_y - 4.0f),
                                             nullptr, &metrics.knob_radius_eq, 1.0f, 8.0f, 30.0f,
                                             nullptr, &metrics.mixer_knobs_offset_y);

            DJLayoutEditorUI::DrawCanvasGizmo(WIDGET_CHANNEL_FADERS, "Faders de Volume",
                                             ImVec2(pos.x + 6.0f, div_y + 110.0f),
                                             ImVec2(pos.x + w - 6.0f, bot_sec_y - 2.0f),
                                             &metrics.channel_fader_orientation, &metrics.fader_height, 5.0f, 40.0f, 220.0f,
                                             nullptr, &metrics.mixer_faders_offset_y);"""

new_gizmos = """            // Gizmo da Barra de Fone (WIDGET_HEADPHONES)
            DJLayoutEditorUI::DrawCanvasGizmo(WIDGET_HEADPHONES, "Fones de Ouvido",
                                             hp_rect_min, hp_rect_max,
                                             nullptr, &metrics.headphone_bar_height, 2.0f, 18.0f, 44.0f,
                                             &metrics.headphone_offset_x, &metrics.headphone_offset_y);

            // Gizmo dos Knobs de EQ com fronteira física de colisão (acima do CUE)
            float knobs_bot_y = std::min(bot_sec_y - 50.0f, div_y + 150.0f + metrics.mixer_knobs_offset_y);
            DJLayoutEditorUI::DrawCanvasGizmo(WIDGET_KNOBS_EQ, "Knobs EQ & COMP",
                                             ImVec2(pos.x + 4.0f, div_y + 2.0f),
                                             ImVec2(pos.x + w - 4.0f, knobs_bot_y),
                                             nullptr, &metrics.knob_radius_eq, 1.0f, 8.0f, 30.0f,
                                             nullptr, &metrics.mixer_knobs_offset_y);

            // Gizmo dos Faders de Volume estritamente contido abaixo do CUE
            DJLayoutEditorUI::DrawCanvasGizmo(WIDGET_CHANNEL_FADERS, "Faders de Volume",
                                             ImVec2(pos.x + 6.0f, knobs_bot_y + 6.0f),
                                             ImVec2(pos.x + w - 6.0f, bot_sec_y - 2.0f),
                                             &metrics.channel_fader_orientation, &metrics.fader_height, 5.0f, 40.0f, 220.0f,
                                             nullptr, &metrics.mixer_faders_offset_y);"""

assert old_gizmos in c, "old_gizmos not found"
c = c.replace(old_gizmos, new_gizmos)

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

print("KuroDJStudioUI.h step 3 successfully updated!")

