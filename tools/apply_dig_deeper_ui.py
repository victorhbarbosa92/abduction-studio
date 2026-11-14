import sys

def apply_dig_deeper():
    path = "C:/Users/o_vic/.gemini/antigravity-ide/scratch/abduction_studio_v2/src/ui/KuroDJStudioUI.h"
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()

    # 1. Omnibar buttons upgrade (add 🔮 DIG DEEPER button)
    omni_target = """            // ── 1. OMNIBAR ROW (Fiel ao Mockup) ─────────────────────────────────
            float omni_btn_w = 210.0f;
            float omni_paste_w = 250.0f;
            float omni_input_w = avail_w - omni_btn_w - omni_paste_w - 24.0f;"""

    omni_replacement = """            // ── 1. OMNIBAR ROW (Fiel ao Mockup + Dig Deeper) ───────────────────
            float omni_btn_w = 175.0f;
            float omni_paste_w = 215.0f;
            float omni_dig_w = 180.0f;
            float omni_input_w = avail_w - omni_btn_w - omni_paste_w - omni_dig_w - 32.0f;"""

    if omni_target in content:
        content = content.replace(omni_target, omni_replacement, 1)
        print("Omnibar widths replaced.")
    else:
        print("Omnibar widths already replaced or not found.")

    # 2. Add Omnibar 🔮 DIG DEEPER button after COLAR LINK button
    paste_target = """            if (ImGui::Button(ICON_FA_PASTE " COLAR LINK SPOTIFY/SC", ImVec2(omni_paste_w, 36.0f))) {
                const char* clip = ImGui::GetClipboardText();
                if (clip && strlen(clip) > 4) {
                    strncpy(online_search_query_buf, clip, sizeof(online_search_query_buf) - 1);
                    trigger_search();
                }
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(4);"""

    paste_replacement = """            if (ImGui::Button(ICON_FA_PASTE " COLAR LINK SPOTIFY/SC", ImVec2(omni_paste_w, 36.0f))) {
                const char* clip = ImGui::GetClipboardText();
                if (clip && strlen(clip) > 4) {
                    strncpy(online_search_query_buf, clip, sizeof(online_search_query_buf) - 1);
                    trigger_search();
                }
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(4);

            ImGui::SameLine(0, 8.0f);

            // Botão 3: 🔮 DIG DEEPER - Magenta/Purple Sólido (Busca Acústica Reversa)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.48f, 0.14f, 0.65f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.20f, 0.80f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            if (ImGui::Button(ICON_FA_COMPACT_DISC " 🔮 DIG DEEPER", ImVec2(omni_dig_w, 36.0f))) {
                std::string q = online_search_query_buf;
                if (q.empty()) q = selected_source_track_title.empty() ? "Spirit Overflow" : selected_source_track_title;
                spotify_service.digDeeperAsync(q, selected_source_track_artist, selected_source_track_key, selected_source_track_bpm);
                spotify_service.show_dig_deeper_view = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Garimpar Almas Gêmeas Acústicas por análise profunda de áudio e Camelot");
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);"""

    if paste_target in content:
        content = content.replace(paste_target, paste_replacement, 1)
        print("Omnibar button replaced.")
    else:
        print("Omnibar button already replaced or not found.")

    # 3. Left Table columns width & mini 🔮 Dig button in rows
    col_target = """                        ImGui::TableSetupColumn("FONTES",              ImGuiTableColumnFlags_WidthFixed, 95.0f);
                        ImGui::TableSetupColumn("AÇÃO",                ImGuiTableColumnFlags_WidthFixed, 130.0f);"""

    col_replacement = """                        ImGui::TableSetupColumn("FONTES",              ImGuiTableColumnFlags_WidthFixed, 85.0f);
                        ImGui::TableSetupColumn("AÇÃO",                ImGuiTableColumnFlags_WidthFixed, 146.0f);"""

    if col_target in content:
        content = content.replace(col_target, col_replacement, 1)
        print("Left table columns width replaced.")
    else:
        print("Left table columns width already replaced or not found.")

    # Mini 🔮 Dig Deeper button inside Col 7
    action_target = """                                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Inspecionar matrizes de fontes no painel direito e ouvir preview");
                                ImGui::PopStyleVar(2);
                                ImGui::PopStyleColor(4);
                            }
                            ImGui::PopID();"""

    action_replacement = """                                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Inspecionar matrizes de fontes no painel direito e ouvir preview");
                                ImGui::PopStyleVar(2);
                                ImGui::PopStyleColor(4);
                            }

                            // Botão 🔮 Dig Deeper na linha da tabela
                            ImGui::SameLine(0, 4.0f);
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.10f, 0.48f, 0.90f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.18f, 0.72f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.85f, 0.30f, 1.00f, 0.85f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                            if (ImGui::Button("🔮", ImVec2(24.0f, 22.0f))) {
                                selected_source_track_id = sr.id;
                                selected_source_track_title = sr.title;
                                selected_source_track_artist = sr.artist;
                                selected_source_track_album = sr.album;
                                selected_source_track_bpm = sr.bpm;
                                selected_source_track_key = sr.key;
                                selected_source_track_subgenre = sr.subgenre;
                                spotify_service.digDeeperAsync(sr.title, sr.artist, sr.key, sr.bpm);
                                spotify_service.show_dig_deeper_view = true;
                            }
                            if (ImGui::IsItemHovered()) ImGui::SetTooltip("🔮 Dig Deeper: minerar almas gêmeas acústicas para '%s'", sr.title.c_str());
                            ImGui::PopStyleVar(2);
                            ImGui::PopStyleColor(4);

                            ImGui::PopID();"""

    if action_target in content:
        content = content.replace(action_target, action_replacement, 1)
        print("Left table row mini 🔮 Dig button added.")
    else:
        print("Left table row mini button already added or not found.")

    # 4. Right Pane Header: Alternador MATRIZ DE FONTES vs DIG DEEPER
    right_pane_target = """                // Cabeçalho do painel direito
                ImVec2 rh_pos = ImGui::GetCursorScreenPos();
                rdl->AddRectFilled(rh_pos, ImVec2(rh_pos.x + rw, rh_pos.y + 26.0f), IM_COL32(8, 15, 26, 240), 4.0f);
                rdl->AddRect(rh_pos, ImVec2(rh_pos.x + rw, rh_pos.y + 26.0f), IM_COL32(0, 220, 200, 80), 4.0f, 0, 1.0f);
                ImGui::SetCursorScreenPos(ImVec2(rh_pos.x + 8.0f, rh_pos.y + 5.0f));
                ImGui::TextColored(col_deck_1, ICON_FA_BOLT " MATRIZ DE FONTES & INSPEÇÃO ESPECTRAL");
                ImGui::SameLine(0, 8.0f);
                ImGui::TextColored(ImVec4(0.45f, 0.58f, 0.72f, 0.85f), "Selecione o provedor e a matriz com maior pureza acústica:");
                ImGui::SetCursorScreenPos(ImVec2(rh_pos.x, rh_pos.y + 32.0f));"""

    right_pane_replacement = """                // Cabeçalho do painel direito: Alternador MATRIZ DE FONTES vs DIG DEEPER
                ImVec2 rh_pos = ImGui::GetCursorScreenPos();
                rdl->AddRectFilled(rh_pos, ImVec2(rh_pos.x + rw, rh_pos.y + 28.0f), IM_COL32(8, 15, 26, 240), 4.0f);
                rdl->AddRect(rh_pos, ImVec2(rh_pos.x + rw, rh_pos.y + 28.0f), IM_COL32(0, 220, 200, 80), 4.0f, 0, 1.0f);
                ImGui::SetCursorScreenPos(ImVec2(rh_pos.x + 8.0f, rh_pos.y + 3.0f));

                bool is_dig_view = spotify_service.show_dig_deeper_view;
                if (!is_dig_view) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.45f, 0.40f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Border, col_deck_1);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.03f, 0.06f, 0.10f, 0.70f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.20f, 0.30f, 0.60f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.82f, 0.90f));
                }
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                if (ImGui::SmallButton(ICON_FA_BOLT " MATRIZ DE FONTES")) {
                    spotify_service.show_dig_deeper_view = false;
                }
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(3);

                ImGui::SameLine(0, 8.0f);

                auto dig_res_cnt = spotify_service.getDigDeeperResults();
                char dig_tab_lbl[64];
                snprintf(dig_tab_lbl, sizeof(dig_tab_lbl), "🔮 DIG DEEPER // GÊMEAS (%d)", (int)dig_res_cnt.size());
                if (is_dig_view) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.40f, 0.12f, 0.55f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.85f, 0.30f, 1.00f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.03f, 0.06f, 0.10f, 0.70f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.10f, 0.30f, 0.60f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.55f, 0.90f, 0.90f));
                }
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                if (ImGui::SmallButton(dig_tab_lbl)) {
                    spotify_service.show_dig_deeper_view = true;
                    if (dig_res_cnt.empty()) {
                        std::string h_t = selected_source_track_title.empty() ? "Spirit Overflow" : selected_source_track_title;
                        std::string h_a = selected_source_track_artist.empty() ? "Jumpstreet" : selected_source_track_artist;
                        std::string h_k = selected_source_track_key.empty() ? "12A" : selected_source_track_key;
                        spotify_service.digDeeperAsync(h_t, h_a, h_k, selected_source_track_bpm > 20.0 ? selected_source_track_bpm : 145.0);
                    }
                }
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(3);

                ImGui::SameLine(0, 10.0f);
                ImGui::TextColored(ImVec4(0.45f, 0.58f, 0.72f, 0.85f), is_dig_view ? "Garimpo por similaridade de groove e graves" : "Selecione o provedor e a matriz:");
                ImGui::SetCursorScreenPos(ImVec2(rh_pos.x, rh_pos.y + 34.0f));"""

    if right_pane_target in content:
        content = content.replace(right_pane_target, right_pane_replacement, 1)
        print("Right pane header toggle replaced.")
    else:
        print("Right pane header toggle already replaced or not found.")

    # 5. Add 🔮 DIG DEEPER button in Hero Box
    hero_badges_target = """                hero_badge("4 MATRIZES DISPONÍVEIS", ImVec4(0.03f, 0.18f, 0.09f, 0.90f), ImVec4(0.00f, 1.00f, 0.50f, 1.0f), ImVec4(0.00f, 1.00f, 0.50f, 0.90f));

                ImGui::SetCursorScreenPos(ImVec2(hero_p.x, hero_p.y + hero_h + 12.0f));"""

    hero_badges_replacement = """                hero_badge("4 MATRIZES DISPONÍVEIS", ImVec4(0.03f, 0.18f, 0.09f, 0.90f), ImVec4(0.00f, 1.00f, 0.50f, 1.0f), ImVec4(0.00f, 1.00f, 0.50f, 0.90f));

                // Botão DIG DEEPER no Hero Box (Crate Digger Acústico)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.10f, 0.48f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.50f, 0.15f, 0.68f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.85f, 0.30f, 1.00f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                if (ImGui::SmallButton(ICON_FA_COMPACT_DISC " 🔮 DIG DEEPER")) {
                    spotify_service.digDeeperAsync(hero_title, hero_art, k_str, selected_source_track_bpm > 20.0 ? selected_source_track_bpm : 145.0);
                    spotify_service.show_dig_deeper_view = true;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Localizar Almas Gêmeas Acústicas pelo som real (Waveform, Sub-grave e Camelot)");
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(4);

                ImGui::SetCursorScreenPos(ImVec2(hero_p.x, hero_p.y + hero_h + 12.0f));"""

    if hero_badges_target in content:
        content = content.replace(hero_badges_target, hero_badges_replacement, 1)
        print("Hero Box 🔮 DIG DEEPER button added.")
    else:
        print("Hero Box button already added or not found.")

    # 6. Branch between Source Cards vs Dig Deeper View
    branch_target = """                ImGui::TextColored(ImVec4(0.48f, 0.62f, 0.78f, 0.90f), "MATRIZES DE ÁUDIO DISPONÍVEIS (SELECIONE PARA DOWNLOAD):");
                ImGui::Spacing();

                // ── 4 CARDS DE PROVEDORES DE ÁUDIO (Fiel ao Mockup) ─────────────"""

    branch_replacement = """                if (!spotify_service.show_dig_deeper_view) {
                ImGui::TextColored(ImVec4(0.48f, 0.62f, 0.78f, 0.90f), "MATRIZES DE ÁUDIO DISPONÍVEIS (SELECIONE PARA DOWNLOAD):");
                ImGui::Spacing();

                // ── 4 CARDS DE PROVEDORES DE ÁUDIO (Fiel ao Mockup) ─────────────"""

    if branch_target in content:
        content = content.replace(branch_target, branch_replacement, 1)
        print("Branch if (!show_dig_deeper_view) added.")
    else:
        print("Branch target already added or not found.")

    # End of Lookahead DNA closing branch with Dig Deeper list
    dna_target = """                    ImGui::SameLine(rw - 165.0f);
                    ImGui::TextColored(d3.col, "%s", d3.right_lbl);
                }
            }
            ImGui::EndChild();"""

    dna_replacement = """                    ImGui::SameLine(rw - 165.0f);
                    ImGui::TextColored(d3.col, "%s", d3.right_lbl);
                }
                } else {
                    // ════════════════════════════════════════════════════════════════════
                    // SUB-VISÃO: DIG DEEPER INTELLIGENCE // ALMAS GÊMEAS ACÚSTICAS
                    // ════════════════════════════════════════════════════════════════════
                    ImVec2 ddh_pos = ImGui::GetCursorScreenPos();
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

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x, ddh_pos.y + 46.0f));

                    auto dig_matches = spotify_service.getDigDeeperResults();
                    if (spotify_service.isDiggingDeeper()) {
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(0.90f, 0.45f, 1.0f, 1.0f), ICON_FA_SPINNER " Minerando ondas de áudio, densidade de sub-grave e similaridade Camelot...");
                        ImGui::Spacing();
                    } else if (dig_matches.empty()) {
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(0.60f, 0.70f, 0.85f, 0.90f), "Nenhuma alma gêmea acústica gerada para esta faixa ainda.");
                        ImGui::Spacing();
                        if (ImGui::Button(ICON_FA_BOLT " DISPARAR MINERAÇÃO ACÚSTICA AGORA")) {
                            spotify_service.digDeeperAsync(hero_title, hero_art, k_str, selected_source_track_bpm > 20.0 ? selected_source_track_bpm : 145.0);
                        }
                    } else {
                        // Renderizar lista das Almas Gêmeas Acústicas
                        for (size_t di = 0; di < dig_matches.size(); di++) {
                            const auto& m = dig_matches[di];
                            ImGui::PushID((int)di + 77000);

                            float item_w = ImGui::GetContentRegionAvail().x;
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
                                if (ImGui::Button(db.lbl, ImVec2(56.0f, 26.0f))) {
                                    int target_d = db.deck;
                                    std::string p = "scratch/tracks/track_" + m.title + ".wav";
                                    if (!std::filesystem::exists(p)) p = (target_d % 2 == 0) ? "scratch/test_cliff.wav" : "scratch/test_astrix.wav";
                                    engine.decks[target_d].loadTrack(p);
                                    engine.decks[target_d].track_title = m.title;
                                    engine.decks[target_d].track_artist = m.artist;
                                    if (m.bpm > 20.0) {
                                        engine.decks[target_d].bpm = m.bpm;
                                        KuroAudio::DJGridEngine::getInstance().setBpm(target_d, m.bpm);
                                    }
                                    engine.decks[target_d].key_signature = m.camelot_code;

                                    spotify_service.recordDigFeedbackAsync(hero_title, m.title, "load_deck", target_d);

                                    toast_message = "Gêmea '" + m.title + "' carregada no Deck " + std::to_string(target_d + 1) + "! Motor aprendeu a transição.";
                                    toast_timer = 4.0f;
                                }
                                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Carregar no Deck %d e registrar reforço de transição no Grafo de Conhecimento", db.deck + 1);
                                ImGui::PopStyleVar(2);
                                ImGui::PopStyleColor(3);
                            }

                            ImGui::PopID();
                            ImGui::Spacing();
                        }
                    }
                }
            }
            ImGui::EndChild();"""

    if dna_target in content:
        content = content.replace(dna_target, dna_replacement, 1)
        print("DNA closing branch replaced with Dig Deeper UI view.")
    else:
        print("ERROR: dna_target still not found!")
        sys.exit(1)

    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
    print("SUCCESS: KuroDJStudioUI.h updated with Dig Deeper UI successfully!")

if __name__ == "__main__":
    apply_dig_deeper()
