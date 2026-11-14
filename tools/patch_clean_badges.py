import sys

path = 'C:/Users/o_vic/.gemini/antigravity-ide/scratch/abduction_studio_v2/src/ui/KuroDJStudioUI.h'
with open(path, 'r', encoding='utf-8') as f:
    c = f.read()

# 1. Fix header right button margin
old_hdr = 'ImGui::SameLine(ddh_w - 295.0f);'
new_hdr = 'ImGui::SameLine(ddh_w - 350.0f);'
if old_hdr in c:
    c = c.replace(old_hdr, new_hdr)
    print('Header right button margin patched!')
else:
    print('WARNING: old_hdr not found')

# 2. Fix card item_h and badge rendering
old_card_block = """                            float item_w = ImGui::GetContentRegionAvail().x;
                            float item_h = 110.0f;
                            ImVec2 item_p = ImGui::GetCursorScreenPos();
                            ImGui::Dummy(ImVec2(item_w, item_h));

                            bool is_golden = (m.play_count > 0 || m.match_badge.find("OURO") != std::string::npos);
                            ImU32 border_col = is_golden ? IM_COL32(255, 200, 30, 240) :
                                              (m.acoustic_match_pct >= 95 ? IM_COL32(0, 245, 212, 200) : IM_COL32(180, 70, 240, 180));

                            rdl->AddRectFilled(item_p, ImVec2(item_p.x + item_w, item_p.y + item_h), IM_COL32(10, 15, 25, 250), 5.0f);
                            rdl->AddRect(item_p, ImVec2(item_p.x + item_w, item_p.y + item_h), border_col, 5.0f, 0, is_golden ? 1.6f : 1.1f);

                            // Badge de Match % no canto superior esquerdo (Linha 1)
                            float bx = item_p.x + 14.0f, by = item_p.y + 10.0f;
                            char pct_str[64];
                            if (is_golden) {
                                snprintf(pct_str, sizeof(pct_str), "★ %d%% TRANSIÇÃO DE OURO (GÊMEA)", m.acoustic_match_pct);
                                rdl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + 250.0f, by + 22.0f), IM_COL32(70, 50, 10, 255), 3.0f);
                                rdl->AddRect(ImVec2(bx, by), ImVec2(bx + 250.0f, by + 22.0f), IM_COL32(255, 200, 30, 255), 3.0f);
                                rdl->AddText(ImVec2(bx + 8.0f, by + 3.0f), IM_COL32(255, 225, 80, 255), pct_str);
                            } else {
                                snprintf(pct_str, sizeof(pct_str), "%d%% %s", m.acoustic_match_pct, m.match_badge.c_str());
                                rdl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + 235.0f, by + 22.0f), IM_COL32(0, 45, 50, 255), 3.0f);
                                rdl->AddRect(ImVec2(bx, by), ImVec2(bx + 235.0f, by + 22.0f), IM_COL32(0, 245, 212, 220), 3.0f);
                                rdl->AddText(ImVec2(bx + 8.0f, by + 3.0f), IM_COL32(0, 245, 212, 255), pct_str);
                            }"""

new_card_block = """                            float item_w = ImGui::GetContentRegionAvail().x;
                            float item_h = 120.0f;
                            ImVec2 item_p = ImGui::GetCursorScreenPos();
                            ImGui::Dummy(ImVec2(item_w, item_h));

                            bool is_golden = (m.play_count > 0 || m.match_badge.find("OURO") != std::string::npos);
                            ImU32 border_col = is_golden ? IM_COL32(255, 200, 30, 240) :
                                              (m.acoustic_match_pct >= 95 ? IM_COL32(0, 245, 212, 200) : IM_COL32(180, 70, 240, 180));

                            rdl->AddRectFilled(item_p, ImVec2(item_p.x + item_w, item_p.y + item_h), IM_COL32(10, 15, 25, 250), 6.0f);
                            rdl->AddRect(item_p, ImVec2(item_p.x + item_w, item_p.y + item_h), border_col, 6.0f, 0, is_golden ? 1.6f : 1.1f);

                            // Badge de Match % no canto superior esquerdo (Linha 1 com SmallButton estilizado nativo)
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x + 14.0f, item_p.y + 10.0f));
                            char pct_str[96];
                            if (is_golden) {
                                snprintf(pct_str, sizeof(pct_str), ICON_FA_STAR " %d%% TRANSIÇÃO DE OURO (GÊMEA)", m.acoustic_match_pct);
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.20f, 0.04f, 0.95f));
                                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.80f, 0.12f, 1.00f));
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.88f, 0.30f, 1.00f));
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                                ImGui::SmallButton(pct_str);
                                ImGui::PopStyleVar(2);
                                ImGui::PopStyleColor(3);
                            } else {
                                snprintf(pct_str, sizeof(pct_str), "%d%% %s", m.acoustic_match_pct, m.match_badge.c_str());
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.18f, 0.20f, 0.95f));
                                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.95f, 0.85f, 0.90f));
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.95f, 0.85f, 1.00f));
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                                ImGui::SmallButton(pct_str);
                                ImGui::PopStyleVar(2);
                                ImGui::PopStyleColor(3);
                            }"""

if old_card_block in c:
    c = c.replace(old_card_block, new_card_block)
    print("Card block patched successfully!")
else:
    print("WARNING: old_card_block not found")

with open(path, 'w', encoding='utf-8') as f:
    f.write(c)

