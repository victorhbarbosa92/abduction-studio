with open('src/ui/KuroDJStudioUI.h', 'r', encoding='utf-8') as f:
    c = f.read()

target = '''            float hp_gain = routing.getHeadphoneGain();
            ImGui::SameLine(0, 3.0f);
            ImGui::PushItemWidth(34.0f);
            if (ImGui::SliderFloat("##HPGain", &hp_gain, 0.0f, 1.5f, "")) routing.setHeadphoneGain(hp_gain);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Fone: Volume Level");
            ImGui::PopItemWidth();'''

repl = '''            float hp_gain = routing.getHeadphoneGain();
            ImGui::SameLine(0, 3.0f);
            ImGui::PushItemWidth(34.0f);
            if (ImGui::SliderFloat("##HPGain", &hp_gain, 0.0f, 1.5f, "")) routing.setHeadphoneGain(hp_gain);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Fone: Volume Level");
            ImGui::PopItemWidth();

            // Botao MASTER CUE (PFL da Saida Master nos fones)
            bool mst_cue = engine.getCueMaster();
            ImGui::SameLine(0, 4.0f);
            float mst_btn_x = pos.x + 98.0f;
            dl->AddRectFilled(ImVec2(mst_btn_x, bot_sec_y + 2.0f), ImVec2(mst_btn_x + 30.0f, bot_sec_y + 17.0f), 
                              mst_cue ? IM_COL32(230, 130, 20, 255) : IM_COL32(22, 28, 38, 255), 2.0f);
            dl->AddRect(ImVec2(mst_btn_x, bot_sec_y + 2.0f), ImVec2(mst_btn_x + 30.0f, bot_sec_y + 17.0f), 
                        mst_cue ? IM_COL32(255, 190, 40, 255) : IM_COL32(46, 58, 76, 200), 2.0f, 0, 1.0f);
            ImVec2 mc_sz = ImGui::CalcTextSize("MST");
            dl->AddText(ImVec2(mst_btn_x + (30.0f - mc_sz.x) * 0.5f, bot_sec_y + 2.0f), 
                        mst_cue ? IM_COL32(255, 255, 255, 255) : IM_COL32(180, 195, 215, 240), "MST");
            ImGui::SetCursorScreenPos(ImVec2(mst_btn_x, bot_sec_y + 2.0f));
            if (ImGui::InvisibleButton("##RekMasterCueBtn", ImVec2(30.0f, 15.0f))) {
                engine.setCueMaster(!mst_cue);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("MASTER CUE: Enviar sinal do Master para os fones de ouvido");'''

if target in c:
    c = c.replace(target, repl, 1)
    with open('src/ui/KuroDJStudioUI.h', 'w', encoding='utf-8') as f:
        f.write(c)
    print("SUCCESS")
else:
    print("TARGET NOT FOUND")
