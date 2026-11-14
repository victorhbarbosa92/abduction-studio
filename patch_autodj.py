import sys

with open('src/ui/KuroDJStudioUI.h', 'r', encoding='utf-8') as f:
    c = f.read()

target = '''                ImGui::InvisibleButton("##TopRecEngine", ImVec2(r_rec_w, r_rec_h));
                if (ImGui::IsItemClicked(0)) {
                    engine.toggleRecording();
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip(rec_on ? "REC: Gravando Master ao vivo em WAV 32-bit! Clique para finalizar" : "REC: Gravar Master da mixagem em WAV Lossless");
            }'''

repl = '''                ImGui::InvisibleButton("##TopRecEngine", ImVec2(r_rec_w, r_rec_h));
                if (ImGui::IsItemClicked(0)) {
                    engine.toggleRecording();
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip(rec_on ? "REC: Gravando Master ao vivo em WAV 32-bit! Clique para finalizar" : "REC: Gravar Master da mixagem em WAV Lossless");
                ImGui::SameLine(0, 5.0f);
            }

            // Display Dinamico de Status do AutoDJ (se ativo)
            if (adj_en) {
                ImVec2 adj_pos = ImGui::GetCursorScreenPos();
                float flash_a = autodj.getBassSwapFlashAlpha();
                ImU32 badge_bg = (flash_a > 0.05f) ? IM_COL32(255, 40, 80, (int)(flash_a * 240)) : IM_COL32(14, 28, 24, 230);
                ImU32 badge_bd = (flash_a > 0.05f) ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 230, 140, 200);

                char adj_lbl[96];
                if (flash_a > 0.05f) {
                    snprintf(adj_lbl, sizeof(adj_lbl), "💥 BASS SWAP! Comp. %d", autodj.getCurrentTransitionBar());
                } else if (autodj.getState() == KuroAudio::AutoDJState::IN_TRANSITION) {
                    snprintf(adj_lbl, sizeof(adj_lbl), "🤖 MIX %d/%d (%d%%)", autodj.getCurrentTransitionBar(), autodj.getTransitionLengthBars(), (int)(autodj.getTransitionProgress() * 100.0f));
                } else {
                    snprintf(adj_lbl, sizeof(adj_lbl), "🤖 AutoDJ: Dk %d", autodj.getActiveDeckIdx() + 1);
                }

                ImVec2 sz = ImGui::CalcTextSize(adj_lbl);
                float b_w = sz.x + 18.0f;
                float b_h = 20.0f;
                dl->AddRectFilled(adj_pos, ImVec2(adj_pos.x + b_w, adj_pos.y + b_h), badge_bg, 10.0f);
                dl->AddRect(adj_pos, ImVec2(adj_pos.x + b_w, adj_pos.y + b_h), badge_bd, 10.0f, 0, 1.0f);
                dl->AddText(ImVec2(adj_pos.x + 9.0f, adj_pos.y + 3.0f), (flash_a > 0.05f) ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 245, 160, 255), adj_lbl);

                ImGui::InvisibleButton("##TopAutoDJStatus", ImVec2(b_w, b_h));
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\\n(Clique com botao direito para alternar Bass Swap)", autodj.getStatusMessage().c_str());
                if (ImGui::IsItemClicked(1)) {
                    autodj.setBassSwapEnabled(!autodj.isBassSwapEnabled());
                }
                ImGui::SameLine(0, 5.0f);
            }'''

if target in c:
    c = c.replace(target, repl, 1)
    with open('src/ui/KuroDJStudioUI.h', 'w', encoding='utf-8') as f:
        f.write(c)
    print("SUCCESS")
else:
    print("TARGET NOT FOUND")
