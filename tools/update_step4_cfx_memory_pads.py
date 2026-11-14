import sys
import os

path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h'

with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# -----------------------------------------------------------------------------
# 1. Update CFX Console in KuroDJStudioUI.h (Image 5 fix)
# -----------------------------------------------------------------------------
old_cfx_block = """                // Botoeira com as 4 cores de assinatura: FILTER, CRUSH, NOISE, SPACE
                float btn_w = 46.0f;
                float btn_h = 15.0f;
                float btn_gap = 2.0f;
                float total_btns_w = btn_w * 4.0f + btn_gap * 3.0f;
                float btn_start_x = cfx_center_cx - total_btns_w * 0.5f;
                float btn_y = pos.y + 14.0f;

                struct CfxDef {
                    int mode_idx;
                    const char* name;
                    ImU32 sig_col;
                    ImVec4 text_col;
                };

                const CfxDef cfx_defs[4] = {
                    { 0, "FILTER", IM_COL32(0, 229, 255, 255),   ImVec4(0.0f, 0.90f, 1.0f, 1.0f) }, // Ciano Eletrico
                    { 2, "CRUSH",  IM_COL32(255, 107, 0, 255),   ImVec4(1.0f, 0.50f, 0.10f, 1.0f) }, // Laranja Fogo
                    { 1, "NOISE",  IM_COL32(56, 189, 248, 255),  ImVec4(0.60f, 0.85f, 1.0f, 1.0f) }, // Azul Gelo / Prata
                    { 3, "SPACE",  IM_COL32(168, 85, 247, 255),  ImVec4(0.80f, 0.50f, 1.0f, 1.0f) }  // Roxo Ultravioleta
                };

                for (int i = 0; i < 4; i++) {
                    const auto& def = cfx_defs[i];
                    bool is_active = (rekordbox_cfx_type == def.mode_idx);
                    ImVec2 b_pos = ImVec2(btn_start_x + i * (btn_w + btn_gap), btn_y);

                    if (is_active) {
                        float glow_expand = 1.5f + pulse * 2.0f;
                        int glow_alpha = (int)(45 + 75 * pulse);
                        ImU32 aura_c = (def.sig_col & 0x00FFFFFF) | ((ImU32)glow_alpha << 24);
                        dl->AddRectFilled(ImVec2(b_pos.x - glow_expand, b_pos.y - glow_expand),
                                          ImVec2(b_pos.x + btn_w + glow_expand, b_pos.y + btn_h + glow_expand),
                                          aura_c, 3.0f);
                    }

                    ImU32 bg_c = is_active ? IM_COL32(18, 24, 34, 255) : IM_COL32(14, 17, 24, 220);
                    dl->AddRectFilled(b_pos, ImVec2(b_pos.x + btn_w, b_pos.y + btn_h), bg_c, 2.0f);
                    dl->AddRect(b_pos, ImVec2(b_pos.x + btn_w, b_pos.y + btn_h), 
                                is_active ? def.sig_col : (def.sig_col & 0x00FFFFFF | 0x65000000), 2.0f, 0, is_active ? 1.5f : 1.0f);

                    ImVec2 str_sz = ImGui::CalcTextSize(def.name);
                    dl->AddText(ImVec2(b_pos.x + (btn_w - str_sz.x) * 0.5f, b_pos.y + (btn_h - str_sz.y) * 0.5f),
                                is_active ? IM_COL32(255, 255, 255, 255) : ImGui::GetColorU32(def.text_col), def.name);

                    ImGui::SetCursorScreenPos(b_pos);
                    char bid[32]; snprintf(bid, sizeof(bid), "##CfxBtn_%d", def.mode_idx);
                    if (ImGui::InvisibleButton(bid, ImVec2(btn_w, btn_h))) {
                        rekordbox_cfx_type = def.mode_idx;
                        engine.decks[0].sound_color_fx_mode = def.mode_idx;
                        engine.decks[1].sound_color_fx_mode = def.mode_idx;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Sound Color FX: Ativar %s no mixer", def.name);
                }

                // 3 Knobs Centrais: CH 1 COLOR | CFX PARAM | CH 2 COLOR
                // Layout com espaçamento amplo e rótulos centralizados
                float knob_r = 7.5f;
                float item_col_w = (center_w - 16.0f) / 3.0f;
                float cfx_items_start_x = cfx_center_cx - (item_col_w * 3.0f) * 0.5f;

                // Knob 1: CH 1 COLOR (Ciano Bipolar)
                float it1_x = cfx_items_start_x;
                float it1_cx = it1_x + item_col_w * 0.5f;
                ImVec2 t1_sz = ImGui::CalcTextSize("CH 1");
                dl->AddText(ImVec2(it1_cx - t1_sz.x * 0.5f, pos.y + 31.0f), IM_COL32(0, 215, 245, 240), "CH 1");
                ImGui::SetCursorScreenPos(ImVec2(it1_cx - 13.5f, pos.y + 43.0f));
                if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob("##Ch1ColorKnob", &engine.decks[0].filter_bipolar, 
                                                                    0.0f, 1.0f, knob_r, ImVec4(0.0f, 0.85f, 1.0f, 1.0f), 
                                                                    "%.2f", 0.5f, true, false, false, false, false)) {
                    engine.decks[0].updateSoundColorFX();
                }

                // Knob 2: CFX PARAM (Branco Frio - Ressonância / Profundidade Pioneer)
                float it2_x = it1_x + item_col_w;
                float it2_cx = it2_x + item_col_w * 0.5f;
                ImVec2 t2_sz = ImGui::CalcTextSize("PARAM");
                dl->AddText(ImVec2(it2_cx - t2_sz.x * 0.5f, pos.y + 31.0f), IM_COL32(230, 240, 255, 240), "PARAM");
                ImGui::SetCursorScreenPos(ImVec2(it2_cx - 13.5f, pos.y + 43.0f));
                if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob("##CfxParamKnob", &rekordbox_cfx_param, 
                                                                    0.0f, 1.0f, knob_r, ImVec4(0.95f, 0.95f, 1.0f, 1.0f), 
                                                                    "%.2f", 0.5f, false, false, false, false, false)) {
                    engine.decks[0].sound_color_param = rekordbox_cfx_param;
                    engine.decks[1].sound_color_param = rekordbox_cfx_param;
                    engine.decks[0].updateSoundColorFX();
                    engine.decks[1].updateSoundColorFX();
                }

                // Knob 3: CH 2 COLOR (Ambar Bipolar)
                float it3_x = it2_x + item_col_w;
                float it3_cx = it3_x + item_col_w * 0.5f;
                ImVec2 t3_sz = ImGui::CalcTextSize("CH 2");
                dl->AddText(ImVec2(it3_cx - t3_sz.x * 0.5f, pos.y + 31.0f), IM_COL32(255, 160, 20, 240), "CH 2");
                ImGui::SetCursorScreenPos(ImVec2(it3_cx - 13.5f, pos.y + 43.0f));"""

new_cfx_block = """                // Botoeira com as 4 cores de assinatura: FILTER, CRUSH, NOISE, SPACE com Auto-Fit e Ergonomia Touch
                float btn_w = std::clamp((center_w - 24.0f) * 0.23f, 48.0f, 96.0f);
                float btn_h = std::clamp(h * 0.26f, 18.0f, 30.0f);
                float btn_gap = 3.0f;
                float total_btns_w = btn_w * 4.0f + btn_gap * 3.0f;
                float btn_start_x = cfx_center_cx - total_btns_w * 0.5f;
                float btn_y = pos.y + 15.0f;

                struct CfxDef {
                    int mode_idx;
                    const char* name;
                    ImU32 sig_col;
                    ImVec4 text_col;
                };

                const CfxDef cfx_defs[4] = {
                    { 0, "FILTER", IM_COL32(0, 229, 255, 255),   ImVec4(0.0f, 0.90f, 1.0f, 1.0f) }, // Ciano Eletrico
                    { 2, "CRUSH",  IM_COL32(255, 107, 0, 255),   ImVec4(1.0f, 0.50f, 0.10f, 1.0f) }, // Laranja Fogo
                    { 1, "NOISE",  IM_COL32(56, 189, 248, 255),  ImVec4(0.60f, 0.85f, 1.0f, 1.0f) }, // Azul Gelo / Prata
                    { 3, "SPACE",  IM_COL32(168, 85, 247, 255),  ImVec4(0.80f, 0.50f, 1.0f, 1.0f) }  // Roxo Ultravioleta
                };

                ImFont* f_cfx = g_font_control_small ? g_font_control_small : ImGui::GetFont();

                for (int i = 0; i < 4; i++) {
                    const auto& def = cfx_defs[i];
                    bool is_active = (rekordbox_cfx_type == def.mode_idx);
                    ImVec2 b_pos = ImVec2(btn_start_x + i * (btn_w + btn_gap), btn_y);
                    ImVec2 b_max = ImVec2(b_pos.x + btn_w, b_pos.y + btn_h);

                    if (is_active) {
                        float glow_expand = 1.5f + pulse * 2.0f;
                        int glow_alpha = (int)(45 + 75 * pulse);
                        ImU32 aura_c = (def.sig_col & 0x00FFFFFF) | ((ImU32)glow_alpha << 24);
                        dl->AddRectFilled(ImVec2(b_pos.x - glow_expand, b_pos.y - glow_expand),
                                          ImVec2(b_pos.x + btn_w + glow_expand, b_pos.y + btn_h + glow_expand),
                                          aura_c, 3.0f);
                    }

                    ImU32 bg_c = is_active ? IM_COL32(18, 24, 34, 255) : IM_COL32(14, 17, 24, 220);
                    dl->AddRectFilled(b_pos, b_max, bg_c, 2.5f);
                    dl->AddRect(b_pos, b_max, 
                                is_active ? def.sig_col : (def.sig_col & 0x00FFFFFF | 0x65000000), 2.5f, 0, is_active ? 1.5f : 1.0f);

                    DJLayoutEditorUI::DrawFittedText(dl, f_cfx, 11.0f, b_pos, b_max,
                                                     is_active ? IM_COL32(255, 255, 255, 255) : ImGui::GetColorU32(def.text_col),
                                                     def.name, 2.0f, 1.0f);

                    ImGui::SetCursorScreenPos(b_pos);
                    char bid[32]; snprintf(bid, sizeof(bid), "##CfxBtn_%d", def.mode_idx);
                    if (ImGui::InvisibleButton(bid, ImVec2(btn_w, btn_h))) {
                        rekordbox_cfx_type = def.mode_idx;
                        engine.decks[0].sound_color_fx_mode = def.mode_idx;
                        engine.decks[1].sound_color_fx_mode = def.mode_idx;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Sound Color FX: Ativar %s no mixer", def.name);
                }

                // 3 Knobs Centrais: CH 1 COLOR | CFX PARAM | CH 2 COLOR
                // Layout com espaçamento amplo, knobs de raio ergonômico para TV 65" e rótulos anti-colisão
                float knob_r = std::clamp(h * 0.17f, 10.0f, 18.0f);
                float item_col_w = (center_w - 16.0f) / 3.0f;
                float cfx_items_start_x = cfx_center_cx - (item_col_w * 3.0f) * 0.5f;
                float label_y = btn_y + btn_h + 3.0f;
                float knob_y = label_y + 14.0f + knob_r;

                // Knob 1: CH 1 COLOR (Ciano Bipolar)
                float it1_x = cfx_items_start_x;
                float it1_cx = it1_x + item_col_w * 0.5f;
                DJLayoutEditorUI::DrawFittedText(dl, f_cfx, 10.0f, ImVec2(it1_x, label_y), ImVec2(it1_x + item_col_w, label_y + 13.0f),
                                                 IM_COL32(0, 215, 245, 240), "CH 1");
                ImGui::SetCursorScreenPos(ImVec2(it1_cx - knob_r - 4.0f, knob_y - knob_r - 4.0f));
                if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob("##Ch1ColorKnob", &engine.decks[0].filter_bipolar, 
                                                                    0.0f, 1.0f, knob_r, ImVec4(0.0f, 0.85f, 1.0f, 1.0f), 
                                                                    "%.2f", 0.5f, true, false, false, false, false)) {
                    engine.decks[0].updateSoundColorFX();
                }

                // Knob 2: CFX PARAM (Branco Frio - Ressonância / Profundidade Pioneer)
                float it2_x = it1_x + item_col_w;
                float it2_cx = it2_x + item_col_w * 0.5f;
                DJLayoutEditorUI::DrawFittedText(dl, f_cfx, 10.0f, ImVec2(it2_x, label_y), ImVec2(it2_x + item_col_w, label_y + 13.0f),
                                                 IM_COL32(230, 240, 255, 240), "PARAM");
                ImGui::SetCursorScreenPos(ImVec2(it2_cx - knob_r - 4.0f, knob_y - knob_r - 4.0f));
                if (KuroUI::KuroDJStudioSkinManager::DrawSkinRotaryKnob("##CfxParamKnob", &rekordbox_cfx_param, 
                                                                    0.0f, 1.0f, knob_r, ImVec4(0.95f, 0.95f, 1.0f, 1.0f), 
                                                                    "%.2f", 0.5f, false, false, false, false, false)) {
                    engine.decks[0].sound_color_param = rekordbox_cfx_param;
                    engine.decks[1].sound_color_param = rekordbox_cfx_param;
                    engine.decks[0].updateSoundColorFX();
                    engine.decks[1].updateSoundColorFX();
                }

                // Knob 3: CH 2 COLOR (Ambar Bipolar)
                float it3_x = it2_x + item_col_w;
                float it3_cx = it3_x + item_col_w * 0.5f;
                DJLayoutEditorUI::DrawFittedText(dl, f_cfx, 10.0f, ImVec2(it3_x, label_y), ImVec2(it3_x + item_col_w, label_y + 13.0f),
                                                 IM_COL32(255, 160, 20, 240), "CH 2");
                ImGui::SetCursorScreenPos(ImVec2(it3_cx - knob_r - 4.0f, knob_y - knob_r - 4.0f));"""

assert old_cfx_block in c, "old_cfx_block not found"
c = c.replace(old_cfx_block, new_cfx_block)

# -----------------------------------------------------------------------------
# 2. Update Memory Cue Panel in KuroDJStudioUI.h (Image 3 fix)
# -----------------------------------------------------------------------------
old_mem_btn_text = """                // Texto centralizado
                ImU32 txt_col = is_active ? active_txt : (is_hov ? IM_COL32(245, 250, 255, 255) : IM_COL32(175, 190, 210, 240));
                ImVec2 t_sz = ImGui::CalcTextSize(label);
                dl->AddText(ImVec2(b_pos.x + (b_size.x - t_sz.x) * 0.5f, b_pos.y + (b_size.y - t_sz.y) * 0.5f), txt_col, label);"""

new_mem_btn_text = """                // Texto centralizado com Auto-Fit e Proteção de Colisão (Nunca vaza)
                ImU32 txt_col = is_active ? active_txt : (is_hov ? IM_COL32(245, 250, 255, 255) : IM_COL32(175, 190, 210, 240));
                auto& metrics_mc = KuroUI::DJLayoutMetrics::getInstance();
                ImFont* f_tactile = g_font_control_small ? g_font_control_small : (g_font_rekordbox_bold ? g_font_rekordbox_bold : ImGui::GetFont());
                DJLayoutEditorUI::DrawFittedText(dl, f_tactile, 11.0f * metrics_mc.ui_font_scale, b_pos, b_max, txt_col, label, 2.0f, 1.0f);"""

assert old_mem_btn_text in c, "old_mem_btn_text not found"
c = c.replace(old_mem_btn_text, new_mem_btn_text)

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

print("KuroDJStudioUI.h step 4 (CFX & Memory Cue) successfully updated!")

