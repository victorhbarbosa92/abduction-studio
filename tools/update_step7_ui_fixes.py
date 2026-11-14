path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h'
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Update renderRekordboxBottomBar
start_str = "void renderRekordboxBottomBar(KuroAudio::DJEngine& engine, float w) {"
idx_start = c.find(start_str)
assert idx_start != -1, "renderRekordboxBottomBar not found"

end_str = 'if (hov) ImGui::SetTooltip("Deck 2: %s\\n%s", pad_tabs[t], pad_descs[t]);\n            }\n            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h + 4.0f));\n        }'
idx_end = c.find(end_str, idx_start)

assert idx_end != -1, "end_str of renderRekordboxBottomBar not found"
idx_end += len(end_str)

new_bottom_bar = """void renderRekordboxBottomBar(KuroAudio::DJEngine& engine, float w) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float h = std::clamp(KuroUI::UIScaleManager::Scale(28.0f), 28.0f, 40.0f);
            ImFont* f_bld = g_font_rekordbox_bold ? g_font_rekordbox_bold : ImGui::GetFont();

            dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(12, 14, 18, 255));
            dl->AddLine(ImVec2(pos.x, pos.y), ImVec2(pos.x + w, pos.y), IM_COL32(25, 30, 40, 255), 1.0f);

            // Logo rekordbox com círculo e DrawFittedText (Nunca vaza sobre as tabs de pads)
            ImVec2 logo_min = ImVec2(pos.x + 8.0f, pos.y + 4.0f);
            float circle_y = pos.y + h * 0.5f;
            dl->AddCircle(ImVec2(logo_min.x + 6.0f, circle_y), 5.5f, IM_COL32(255, 255, 255, 255), 16, 1.5f);
            ImVec2 logo_text_min = ImVec2(logo_min.x + 15.0f, pos.y + 2.0f);
            ImVec2 logo_text_max = ImVec2(logo_min.x + 95.0f, pos.y + h - 2.0f);
            DJLayoutEditorUI::DrawFittedText(dl, f_bld, 11.5f, logo_text_min, logo_text_max, IM_COL32(255, 255, 255, 255), "rekordbox", 1.0f, 1.0f);

            float tab1_x = logo_text_max.x + 10.0f;
            const char* pad_tabs[] = { "HOT CUE", "BEAT LOOP", "PAD FX", "SAMPLER", "STEMS", "BEAT JUMP" };
            const char* pad_descs[] = { 
                "Hot Cues A-P (Pontos de salto instantaneo)", 
                "Beat Loops (Loops automaticos sincronizados)", 
                "Pad FX (Efeitos de performance em tempo real)", 
                "Sampler (One-shots e vinhetas nos pads)",
                "Live Stems (Isolamento em tempo real de Vocal, Melodia e Bateria)",
                "Beat Jump (Saltos frasais quantizados de 1, 2, 4, 8, 16 a 64 batidas)"
            };

            float tab_w = std::clamp((w * 0.5f - tab1_x - 30.0f) / 6.0f, 44.0f, 68.0f);
            float tab_h = h - 6.0f;

            for (int t = 0; t < 6; t++) {
                bool act = (rekordbox_pad_mode[0] == t);
                ImVec2 t_pos = ImVec2(tab1_x + (float)t * (tab_w + 3.0f), pos.y + 3.0f);
                ImVec2 t_max = ImVec2(t_pos.x + tab_w, t_pos.y + tab_h);
                ImGui::SetCursorScreenPos(t_pos);
                char tb_id[32]; snprintf(tb_id, sizeof(tb_id), "##RekTab0_%d", t);
                if (ImGui::InvisibleButton(tb_id, ImVec2(tab_w, tab_h))) {
                    setPadMode(0, t, engine);
                }
                bool hov = ImGui::IsItemHovered();
                dl->AddRectFilled(t_pos, t_max, act ? IM_COL32(0, 132, 255, 255) : (hov ? IM_COL32(28, 36, 48, 255) : IM_COL32(18, 22, 28, 255)), 2.5f);
                dl->AddRect(t_pos, t_max, act ? IM_COL32(255, 255, 255, 220) : IM_COL32(36, 46, 62, 180), 2.5f);
                DJLayoutEditorUI::DrawFittedText(dl, f_bld, 10.5f, t_pos, t_max, act ? IM_COL32(255, 255, 255, 255) : IM_COL32(160, 175, 195, 240), pad_tabs[t], 2.0f, 1.0f);
                if (hov) ImGui::SetTooltip("Deck 1: %s\\n%s", pad_tabs[t], pad_descs[t]);
            }

            float center_x = pos.x + w * 0.5f;
            dl->AddLine(ImVec2(center_x, pos.y + 4.0f), ImVec2(center_x, pos.y + h - 4.0f), IM_COL32(40, 50, 65, 200), 1.0f);

            // Deck 2 Pad Mode Tabs à direita
            float tab2_x = center_x + 12.0f;
            for (int t = 0; t < 6; t++) {
                bool act = (rekordbox_pad_mode[1] == t);
                ImVec2 t_pos = ImVec2(tab2_x + (float)t * (tab_w + 3.0f), pos.y + 3.0f);
                ImVec2 t_max = ImVec2(t_pos.x + tab_w, t_pos.y + tab_h);
                ImGui::SetCursorScreenPos(t_pos);
                char tb_id[32]; snprintf(tb_id, sizeof(tb_id), "##RekTab1_%d", t);
                if (ImGui::InvisibleButton(tb_id, ImVec2(tab_w, tab_h))) {
                    setPadMode(1, t, engine);
                }
                bool hov = ImGui::IsItemHovered();
                dl->AddRectFilled(t_pos, t_max, act ? IM_COL32(0, 132, 255, 255) : (hov ? IM_COL32(28, 36, 48, 255) : IM_COL32(18, 22, 28, 255)), 2.5f);
                dl->AddRect(t_pos, t_max, act ? IM_COL32(255, 255, 255, 220) : IM_COL32(36, 46, 62, 180), 2.5f);
                DJLayoutEditorUI::DrawFittedText(dl, f_bld, 10.5f, t_pos, t_max, act ? IM_COL32(255, 255, 255, 255) : IM_COL32(160, 175, 195, 240), pad_tabs[t], 2.0f, 1.0f);
                if (hov) ImGui::SetTooltip("Deck 2: %s\\n%s", pad_tabs[t], pad_descs[t]);
            }
            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h + 4.0f));
        }"""

c = c[:idx_start] + new_bottom_bar + c[idx_end:]

# 2. Update Library Mini Turntable Preview (Image 6 fix - PRÉ ESCUTA & OUVIR)
old_prev_child = 'ImGui::BeginChild("##MiniTurntablePreviewPlatter", ImVec2(left_w - 14.0f, 54.0f), true, ImGuiWindowFlags_NoScrollbar);'
new_prev_child = 'float preview_h = std::clamp(ImGui::GetTextLineHeightWithSpacing() * 2.2f + 20.0f, 68.0f, 95.0f);\n            ImGui::BeginChild("##MiniTurntablePreviewPlatter", ImVec2(left_w - 14.0f, preview_h), true, ImGuiWindowFlags_NoScrollbar);'
assert old_prev_child in c, "old_prev_child not found"
c = c.replace(old_prev_child, new_prev_child)

old_btn_pos = 'ImGui::SetCursorScreenPos(ImVec2(ctrl_x, platter_p.y + 20.0f));'
new_btn_pos = 'float prev_btn_y = platter_p.y + ImGui::GetTextLineHeightWithSpacing() + 4.0f;\n            ImGui::SetCursorScreenPos(ImVec2(ctrl_x, prev_btn_y));'
assert old_btn_pos in c, "old_btn_pos not found"
c = c.replace(old_btn_pos, new_btn_pos, 1)

# 3. Camelot Wheel text size
old_cam_b = """char buf_b[8]; snprintf(buf_b, sizeof(buf_b), "%dB", h);
                        ImVec2 t_sz = ImGui::CalcTextSize(buf_b);
                        draw_list->AddText(ImVec2(t_pos.x - t_sz.x * 0.5f, t_pos.y - t_sz.y * 0.5f), 
                                           is_hov ? IM_COL32(0,0,0,255) : IM_COL32(255,255,255,240), buf_b);"""

new_cam_b = """char buf_b[8]; snprintf(buf_b, sizeof(buf_b), "%dB", h);
                        ImFont* f_wheel = g_font_control_small ? g_font_control_small : ImGui::GetFont();
                        float wheel_fnt_sz = 8.5f;
                        ImVec2 t_sz = f_wheel->CalcTextSizeA(wheel_fnt_sz, FLT_MAX, 0.0f, buf_b);
                        draw_list->AddText(f_wheel, wheel_fnt_sz, ImVec2(t_pos.x - t_sz.x * 0.5f, t_pos.y - t_sz.y * 0.5f), 
                                           is_hov ? IM_COL32(0,0,0,255) : IM_COL32(255,255,255,240), buf_b);"""
assert old_cam_b in c, "old_cam_b not found"
c = c.replace(old_cam_b, new_cam_b)

old_cam_a = """char buf_a[8]; snprintf(buf_a, sizeof(buf_a), "%dA", h);
                        ImVec2 t_sz = ImGui::CalcTextSize(buf_a);
                        draw_list->AddText(ImVec2(t_pos.x - t_sz.x * 0.5f, t_pos.y - t_sz.y * 0.5f), 
                                           is_hov ? IM_COL32(0,0,0,255) : IM_COL32(255,255,255,240), buf_a);"""

new_cam_a = """char buf_a[8]; snprintf(buf_a, sizeof(buf_a), "%dA", h);
                        ImFont* f_wheel = g_font_control_small ? g_font_control_small : ImGui::GetFont();
                        float wheel_fnt_sz = 8.0f;
                        ImVec2 t_sz = f_wheel->CalcTextSizeA(wheel_fnt_sz, FLT_MAX, 0.0f, buf_a);
                        draw_list->AddText(f_wheel, wheel_fnt_sz, ImVec2(t_pos.x - t_sz.x * 0.5f, t_pos.y - t_sz.y * 0.5f), 
                                           is_hov ? IM_COL32(0,0,0,255) : IM_COL32(255,255,255,240), buf_a);"""
assert old_cam_a in c, "old_cam_a not found"
c = c.replace(old_cam_a, new_cam_a)

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

print("KuroDJStudioUI.h step 7 (Bottom Bar, Preview Platter and Camelot Wheel) successfully updated!")
