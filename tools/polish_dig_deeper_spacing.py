import sys

def polish():
    path = "C:/Users/o_vic/.gemini/antigravity-ide/scratch/abduction_studio_v2/src/ui/KuroDJStudioUI.h"
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()

    # 1. Omnibar buttons text & width: compact and 100% visible
    omni_target = """            // ── 1. OMNIBAR ROW (Fiel ao Mockup + Dig Deeper) ───────────────────
            float omni_btn_w = 175.0f;
            float omni_paste_w = 215.0f;
            float omni_dig_w = 180.0f;
            float omni_input_w = avail_w - omni_btn_w - omni_paste_w - omni_dig_w - 32.0f;"""

    omni_replacement = """            // ── 1. OMNIBAR ROW (Fiel ao Mockup + Dig Deeper Pixel-Perfect) ──────
            float omni_btn_w = 150.0f;
            float omni_paste_w = 160.0f;
            float omni_dig_w = 165.0f;
            float omni_input_w = avail_w - omni_btn_w - omni_paste_w - omni_dig_w - 32.0f;"""

    content = content.replace(omni_target, omni_replacement)

    # Button labels in Omnibar
    btn_lbl_target = """            if (ImGui::Button(ICON_FA_BOLT " ABDUZIR (BUSCAR)", ImVec2(omni_btn_w, 36.0f))) {"""
    btn_lbl_replacement = """            if (ImGui::Button(ICON_FA_BOLT " ABDUZIR", ImVec2(omni_btn_w, 36.0f))) {"""
    content = content.replace(btn_lbl_target, btn_lbl_replacement)

    paste_lbl_target = """            if (ImGui::Button(ICON_FA_PASTE " COLAR LINK SPOTIFY/SC", ImVec2(omni_paste_w, 36.0f))) {"""
    paste_lbl_replacement = """            if (ImGui::Button(ICON_FA_PASTE " COLAR LINK", ImVec2(omni_paste_w, 36.0f))) {"""
    content = content.replace(paste_lbl_target, paste_lbl_replacement)

    # 2. Dig Deeper Header Banner spacing in Right Pane
    header_target = """                    ImVec2 ddh_pos = ImGui::GetCursorScreenPos();
                    float ddh_w = ImGui::GetContentRegionAvail().x;
                    rdl->AddRectFilled(ddh_pos, ImVec2(ddh_pos.x + ddh_w, ddh_pos.y + 40.0f), IM_COL32(18, 10, 32, 255), 4.0f);
                    rdl->AddRect(ddh_pos, ImVec2(ddh_pos.x + ddh_w, ddh_pos.y + 40.0f), IM_COL32(190, 60, 255, 180), 4.0f, 0, 1.2f);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x + 8.0f, ddh_pos.y + 6.0f));
                    ImGui::TextColored(ImVec4(0.92f, 0.40f, 1.0f, 1.0f), "🔮 DIG DEEPER INTELLIGENCE // MOTOR DE GARIMPO ACÚSTICO");

                    ImGui::SameLine(ddh_w - 275.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.35f, 0.20f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 1.00f, 0.50f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 1.00f, 0.60f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::SmallButton(ICON_FA_BRAIN " AUTO-EVOLUTIVO (SQLITE ATIVO)");
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(3);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x + 8.0f, ddh_pos.y + 22.0f));
                    std::string ref_title_disp = spotify_service.getDigDeeperRefTitle().empty() ? hero_title : spotify_service.getDigDeeperRefTitle();
                    ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.88f, 0.90f), "DNA Acústico de Referência: \\"%s\\" • Tom: %s • BPM: %.1f",
                        ref_title_disp.c_str(), k_str.c_str(), selected_source_track_bpm > 20.0 ? selected_source_track_bpm : 145.0);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x, ddh_pos.y + 46.0f));"""

    header_replacement = """                    ImVec2 ddh_pos = ImGui::GetCursorScreenPos();
                    float ddh_w = ImGui::GetContentRegionAvail().x;
                    float ddh_h = 52.0f;
                    rdl->AddRectFilled(ddh_pos, ImVec2(ddh_pos.x + ddh_w, ddh_pos.y + ddh_h), IM_COL32(18, 10, 32, 255), 5.0f);
                    rdl->AddRect(ddh_pos, ImVec2(ddh_pos.x + ddh_w, ddh_pos.y + ddh_h), IM_COL32(190, 60, 255, 180), 5.0f, 0, 1.2f);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x + 10.0f, ddh_pos.y + 8.0f));
                    ImGui::TextColored(ImVec4(0.92f, 0.40f, 1.0f, 1.0f), "🔮 DIG DEEPER INTELLIGENCE // MOTOR DE GARIMPO ACÚSTICO");

                    ImGui::SameLine(ddh_w - 285.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.35f, 0.20f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 1.00f, 0.50f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 1.00f, 0.60f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::SmallButton(ICON_FA_BRAIN " AUTO-EVOLUTIVO (SQLITE ATIVO)");
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(3);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x + 10.0f, ddh_pos.y + 29.0f));
                    std::string ref_title_disp = spotify_service.getDigDeeperRefTitle().empty() ? hero_title : spotify_service.getDigDeeperRefTitle();
                    ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.88f, 0.90f), "DNA Acústico de Referência: \\"%s\\" • Tom: %s • BPM: %.1f",
                        ref_title_disp.c_str(), k_str.c_str(), selected_source_track_bpm > 20.0 ? selected_source_track_bpm : 145.0);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x, ddh_pos.y + ddh_h + 8.0f));"""

    content = content.replace(header_target, header_replacement)

    # 3. Match Card internal layout: item_h = 92px, perfectly spaced lines and deck buttons
    card_target = """                            float item_w = ImGui::GetContentRegionAvail().x;
                            float item_h = 76.0f;
                            ImVec2 item_p = ImGui::GetCursorScreenPos();
                            ImGui::Dummy(ImVec2(item_w, item_h));

                            bool is_golden = (m.play_count > 0 || m.match_badge.find("OURO") != std::string::npos);
                            ImU32 border_col = is_golden ? IM_COL32(255, 200, 30, 240) :
                                              (m.acoustic_match_pct >= 95 ? IM_COL32(0, 245, 212, 200) : IM_COL32(180, 70, 240, 180));

                            rdl->AddRectFilled(item_p, ImVec2(item_p.x + item_w, item_p.y + item_h), IM_COL32(10, 15, 25, 250), 4.0f);
                            rdl->AddRect(item_p, ImVec2(item_p.x + item_w, item_p.y + item_h), border_col, 4.0f, 0, is_golden ? 1.6f : 1.1f);

                            // Badge de Match % no canto superior esquerdo
                            float bx = item_p.x + 10.0f, by = item_p.y + 8.0f;
                            char pct_str[64];
                            if (is_golden) {
                                snprintf(pct_str, sizeof(pct_str), "★ %d%% TRANSIÇÃO DE OURO (GÊMEA)", m.acoustic_match_pct);
                                rdl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + 230.0f, by + 18.0f), IM_COL32(70, 50, 10, 255), 3.0f);
                                rdl->AddRect(ImVec2(bx, by), ImVec2(bx + 230.0f, by + 18.0f), IM_COL32(255, 200, 30, 255), 3.0f);
                                rdl->AddText(ImVec2(bx + 6.0f, by + 1.0f), IM_COL32(255, 225, 80, 255), pct_str);
                            } else {
                                snprintf(pct_str, sizeof(pct_str), "%d%% %s", m.acoustic_match_pct, m.match_badge.c_str());
                                rdl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + 215.0f, by + 18.0f), IM_COL32(0, 45, 50, 255), 3.0f);
                                rdl->AddRect(ImVec2(bx, by), ImVec2(bx + 215.0f, by + 18.0f), IM_COL32(0, 245, 212, 220), 3.0f);
                                rdl->AddText(ImVec2(bx + 6.0f, by + 1.0f), IM_COL32(0, 245, 212, 255), pct_str);
                            }

                            // Título da faixa e Artista
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x + 10.0f, item_p.y + 30.0f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::TextUnformatted(m.title.c_str());
                            ImGui::PopStyleColor();

                            ImGui::SameLine(0, 6.0f);
                            ImGui::TextColored(col_deck_1, "• %s", m.artist.c_str());
                            ImGui::SameLine(0, 6.0f);
                            ImGui::TextColored(ImVec4(0.48f, 0.60f, 0.72f, 0.85f), "(%s)", m.album.c_str());

                            // Metadados: Tom Camelot, BPM drift, Provedor
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x + 10.0f, item_p.y + 51.0f));
                            ImGui::TextColored(ImVec4(0.85f, 0.40f, 1.0f, 1.0f), "TOM: %s %s", m.camelot_code.c_str(), m.harmonic_relation.c_str());
                            ImGui::SameLine(0, 10.0f);
                            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.30f, 1.0f), "%.1f BPM (%+.1f%% drift)", m.bpm, m.bpm_drift_pct);
                            ImGui::SameLine(0, 10.0f);
                            ImGui::TextColored(ImVec4(0.55f, 0.75f, 0.95f, 0.90f), "[%s]", m.provider.c_str());

                            // AÇÕES NO CANTO DIREITO DO CARD
                            float btn_area_w = 320.0f;
                            float bx_actions = item_p.x + item_w - btn_area_w - 10.0f;
                            ImGui::SetCursorScreenPos(ImVec2(bx_actions, item_p.y + 24.0f));

                            // 1. Play Preview
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.45f, 0.42f, 0.90f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                            if (ImGui::Button(ICON_FA_PLAY "##DigPlay", ImVec2(28.0f, 26.0f))) {
                                std::string p_src = m.url.empty() ? "scratch/test_astrix.wav" : m.url;
                                preview_player.play(p_src, m.id, m.title, m.artist);
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pré-escuta no Audition Dock");
                            ImGui::PopStyleVar();
                            ImGui::PopStyleColor(2);

                            // 2. Feedback Positivo (Curtir / Ensinar o Grafo)
                            ImGui::SameLine(0, 6.0f);
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.08f, 0.28f, 0.90f));
                            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.80f, 0.25f, 1.00f, 0.80f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.60f, 1.00f, 1.0f));
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                            if (ImGui::Button(ICON_FA_THUMBS_UP "##DigLike", ImVec2(28.0f, 26.0f))) {
                                spotify_service.recordDigFeedbackAsync(hero_title, m.title, "like", 0);
                                toast_message = "Reforço Gravado: afinidade de " + m.title + " aumentada!";
                                toast_timer = 3.0f;
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ensinar ao motor: aumentar afinidade desta combinação para futuros sets!");
                            ImGui::PopStyleVar(2);
                            ImGui::PopStyleColor(3);

                            // 3. 4 Botões de Deck: ⚡ D1, ⚡ D2, ⚡ D3, ⚡ D4
                            struct DigDeckBtn { const char* lbl; ImVec4 col; int deck; };
                            DigDeckBtn dbtns[4] = {
                                { "⚡ D1", col_deck_1, 0 },
                                { "⚡ D2", col_deck_2, 1 },
                                { "⚡ D3", col_deck_3, 2 },
                                { "⚡ D4", col_deck_4, 3 },
                            };
                            for (auto& db : dbtns) {
                                ImGui::SameLine(0, 4.0f);
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(db.col.x * 0.20f, db.col.y * 0.20f, db.col.z * 0.20f, 0.95f));
                                ImGui::PushStyleColor(ImGuiCol_Border, db.col);
                                ImGui::PushStyleColor(ImGuiCol_Text, db.col);
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                                if (ImGui::Button(db.lbl, ImVec2(56.0f, 26.0f))) {"""

    card_replacement = """                            float item_w = ImGui::GetContentRegionAvail().x;
                            float item_h = 90.0f;
                            ImVec2 item_p = ImGui::GetCursorScreenPos();
                            ImGui::Dummy(ImVec2(item_w, item_h));

                            bool is_golden = (m.play_count > 0 || m.match_badge.find("OURO") != std::string::npos);
                            ImU32 border_col = is_golden ? IM_COL32(255, 200, 30, 240) :
                                              (m.acoustic_match_pct >= 95 ? IM_COL32(0, 245, 212, 200) : IM_COL32(180, 70, 240, 180));

                            rdl->AddRectFilled(item_p, ImVec2(item_p.x + item_w, item_p.y + item_h), IM_COL32(10, 15, 25, 250), 5.0f);
                            rdl->AddRect(item_p, ImVec2(item_p.x + item_w, item_p.y + item_h), border_col, 5.0f, 0, is_golden ? 1.6f : 1.1f);

                            // Badge de Match % no canto superior esquerdo (Linha 1)
                            float bx = item_p.x + 12.0f, by = item_p.y + 8.0f;
                            char pct_str[64];
                            if (is_golden) {
                                snprintf(pct_str, sizeof(pct_str), "★ %d%% TRANSIÇÃO DE OURO (GÊMEA)", m.acoustic_match_pct);
                                rdl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + 240.0f, by + 20.0f), IM_COL32(70, 50, 10, 255), 3.0f);
                                rdl->AddRect(ImVec2(bx, by), ImVec2(bx + 240.0f, by + 20.0f), IM_COL32(255, 200, 30, 255), 3.0f);
                                rdl->AddText(ImVec2(bx + 8.0f, by + 2.0f), IM_COL32(255, 225, 80, 255), pct_str);
                            } else {
                                snprintf(pct_str, sizeof(pct_str), "%d%% %s", m.acoustic_match_pct, m.match_badge.c_str());
                                rdl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + 225.0f, by + 20.0f), IM_COL32(0, 45, 50, 255), 3.0f);
                                rdl->AddRect(ImVec2(bx, by), ImVec2(bx + 225.0f, by + 20.0f), IM_COL32(0, 245, 212, 220), 3.0f);
                                rdl->AddText(ImVec2(bx + 8.0f, by + 2.0f), IM_COL32(0, 245, 212, 255), pct_str);
                            }

                            // Título da faixa e Artista (Linha 2 - Espaçamento DPI Perfeito)
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x + 12.0f, item_p.y + 35.0f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::TextUnformatted(m.title.c_str());
                            ImGui::PopStyleColor();

                            ImGui::SameLine(0, 6.0f);
                            ImGui::TextColored(col_deck_1, "• %s", m.artist.c_str());
                            ImGui::SameLine(0, 6.0f);
                            ImGui::TextColored(ImVec4(0.48f, 0.60f, 0.72f, 0.85f), "(%s)", m.album.c_str());

                            // Metadados: Tom Camelot, BPM drift, Provedor (Linha 3 - Zero Sobreposição)
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x + 12.0f, item_p.y + 62.0f));
                            ImGui::TextColored(ImVec4(0.85f, 0.40f, 1.0f, 1.0f), "TOM: %s %s", m.camelot_code.c_str(), m.harmonic_relation.c_str());
                            ImGui::SameLine(0, 10.0f);
                            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.30f, 1.0f), "%.1f BPM (%+.1f%% drift)", m.bpm, m.bpm_drift_pct);
                            ImGui::SameLine(0, 10.0f);
                            ImGui::TextColored(ImVec4(0.55f, 0.75f, 0.95f, 0.90f), "[%s]", m.provider.c_str());

                            // AÇÕES NO CANTO DIREITO DO CARD (Alinhadas verticalmente no centro)
                            float btn_area_w = 365.0f;
                            float bx_actions = item_p.x + item_w - btn_area_w - 12.0f;
                            float by_actions = item_p.y + (item_h - 30.0f) * 0.5f;
                            ImGui::SetCursorScreenPos(ImVec2(bx_actions, by_actions));

                            // 1. Play Preview
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.45f, 0.42f, 0.90f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                            if (ImGui::Button(ICON_FA_PLAY "##DigPlay", ImVec2(30.0f, 30.0f))) {
                                std::string p_src = m.url.empty() ? "scratch/test_astrix.wav" : m.url;
                                preview_player.play(p_src, m.id, m.title, m.artist);
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pré-escuta no Audition Dock");
                            ImGui::PopStyleVar();
                            ImGui::PopStyleColor(2);

                            // 2. Feedback Positivo (Curtir / Ensinar o Grafo)
                            ImGui::SameLine(0, 6.0f);
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.08f, 0.28f, 0.90f));
                            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.80f, 0.25f, 1.00f, 0.80f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.60f, 1.00f, 1.0f));
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                            if (ImGui::Button(ICON_FA_THUMBS_UP "##DigLike", ImVec2(30.0f, 30.0f))) {
                                spotify_service.recordDigFeedbackAsync(hero_title, m.title, "like", 0);
                                toast_message = "Reforço Gravado: afinidade de " + m.title + " aumentada!";
                                toast_timer = 3.0f;
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ensinar ao motor: aumentar afinidade desta combinação para futuros sets!");
                            ImGui::PopStyleVar(2);
                            ImGui::PopStyleColor(3);

                            // 3. 4 Botões de Deck: ⚡ D1, ⚡ D2, ⚡ D3, ⚡ D4 (64px de largura, sem cortes)
                            struct DigDeckBtn { const char* lbl; ImVec4 col; int deck; };
                            DigDeckBtn dbtns[4] = {
                                { "⚡ D1", col_deck_1, 0 },
                                { "⚡ D2", col_deck_2, 1 },
                                { "⚡ D3", col_deck_3, 2 },
                                { "⚡ D4", col_deck_4, 3 },
                            };
                            for (auto& db : dbtns) {
                                ImGui::SameLine(0, 4.0f);
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(db.col.x * 0.20f, db.col.y * 0.20f, db.col.z * 0.20f, 0.95f));
                                ImGui::PushStyleColor(ImGuiCol_Border, db.col);
                                ImGui::PushStyleColor(ImGuiCol_Text, db.col);
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                                if (ImGui::Button(db.lbl, ImVec2(66.0f, 30.0f))) {"""

    content = content.replace(card_target, card_replacement)

    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
    print("SUCCESS: Polished Dig Deeper layout spacing and button sizes!")

if __name__ == "__main__":
    polish()

