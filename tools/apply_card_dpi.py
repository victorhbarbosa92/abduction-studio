path = "C:/Users/o_vic/.gemini/antigravity-ide/scratch/abduction_studio_v2/src/ui/KuroDJStudioUI.h"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Header banner 68px
h_old = """                    float ddh_h = 56.0f;
                    ImGui::Dummy(ImVec2(ddh_w, ddh_h));
                    rdl->AddRectFilled(ddh_pos, ImVec2(ddh_pos.x + ddh_w, ddh_pos.y + ddh_h), IM_COL32(18, 10, 32, 255), 5.0f);
                    rdl->AddRect(ddh_pos, ImVec2(ddh_pos.x + ddh_w, ddh_pos.y + ddh_h), IM_COL32(190, 60, 255, 180), 5.0f, 0, 1.2f);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x + 12.0f, ddh_pos.y + 9.0f));
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

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x + 12.0f, ddh_pos.y + 31.0f));
                    std::string ref_title_disp = spotify_service.getDigDeeperRefTitle().empty() ? hero_title : spotify_service.getDigDeeperRefTitle();
                    ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.88f, 0.90f), "DNA Acústico de Referência: \\"%s\\" • Tom: %s • BPM: %.1f",
                        ref_title_disp.c_str(), k_str.c_str(), selected_source_track_bpm > 20.0 ? selected_source_track_bpm : 145.0);

                    ImGui::Spacing();"""

h_new = """                    float ddh_h = 66.0f;
                    ImGui::Dummy(ImVec2(ddh_w, ddh_h));
                    rdl->AddRectFilled(ddh_pos, ImVec2(ddh_pos.x + ddh_w, ddh_pos.y + ddh_h), IM_COL32(18, 10, 32, 255), 5.0f);
                    rdl->AddRect(ddh_pos, ImVec2(ddh_pos.x + ddh_w, ddh_pos.y + ddh_h), IM_COL32(190, 60, 255, 180), 5.0f, 0, 1.2f);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x + 12.0f, ddh_pos.y + 11.0f));
                    ImGui::TextColored(ImVec4(0.92f, 0.40f, 1.0f, 1.0f), "🔮 DIG DEEPER INTELLIGENCE // MOTOR DE GARIMPO ACÚSTICO");

                    ImGui::SameLine(ddh_w - 295.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.35f, 0.20f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 1.00f, 0.50f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 1.00f, 0.60f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::SmallButton(ICON_FA_BRAIN " AUTO-EVOLUTIVO (SQLITE ATIVO)");
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(3);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x + 12.0f, ddh_pos.y + 36.0f));
                    std::string ref_title_disp = spotify_service.getDigDeeperRefTitle().empty() ? hero_title : spotify_service.getDigDeeperRefTitle();
                    ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.88f, 0.90f), "DNA Acústico de Referência: \\"%s\\" • Tom: %s • BPM: %.1f",
                        ref_title_disp.c_str(), k_str.c_str(), selected_source_track_bpm > 20.0 ? selected_source_track_bpm : 145.0);

                    ImGui::Spacing();
                    ImGui::Spacing();"""

content = content.replace(h_old, h_new)

# Card height 122px with DPI spacing
c_old = """                            float item_w = ImGui::GetContentRegionAvail().x;
                            float item_h = 104.0f;
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
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x + 12.0f, item_p.y + 38.0f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::TextUnformatted(m.title.c_str());
                            ImGui::PopStyleColor();

                            ImGui::SameLine(0, 6.0f);
                            ImGui::TextColored(col_deck_1, "• %s", m.artist.c_str());
                            ImGui::SameLine(0, 6.0f);
                            ImGui::TextColored(ImVec4(0.48f, 0.60f, 0.72f, 0.85f), "(%s)", m.album.c_str());

                            // Metadados: Tom Camelot, BPM drift, Provedor (Linha 3 - Zero Sobreposição)
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x + 12.0f, item_p.y + 70.0f));
                            ImGui::TextColored(ImVec4(0.85f, 0.40f, 1.0f, 1.0f), "TOM: %s %s", m.camelot_code.c_str(), m.harmonic_relation.c_str());
                            ImGui::SameLine(0, 10.0f);
                            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.30f, 1.0f), "%.1f BPM (%+.1f%% drift)", m.bpm, m.bpm_drift_pct);
                            ImGui::SameLine(0, 10.0f);
                            ImGui::TextColored(ImVec4(0.55f, 0.75f, 0.95f, 0.90f), "[%s]", m.provider.c_str());

                            // AÇÕES NO CANTO DIREITO DO CARD (Alinhadas verticalmente no centro)
                            float btn_area_w = 365.0f;
                            float bx_actions = item_p.x + item_w - btn_area_w - 12.0f;
                            float by_actions = item_p.y + (item_h - 30.0f) * 0.5f;"""

c_new = """                            float item_w = ImGui::GetContentRegionAvail().x;
                            float item_h = 120.0f;
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
                            }

                            // Título da faixa e Artista (Linha 2 - Espaçamento DPI Perfeito)
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x + 14.0f, item_p.y + 45.0f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::TextUnformatted(m.title.c_str());
                            ImGui::PopStyleColor();

                            ImGui::SameLine(0, 6.0f);
                            ImGui::TextColored(col_deck_1, "• %s", m.artist.c_str());
                            ImGui::SameLine(0, 6.0f);
                            ImGui::TextColored(ImVec4(0.48f, 0.60f, 0.72f, 0.85f), "(%s)", m.album.c_str());

                            // Metadados: Tom Camelot, BPM drift, Provedor (Linha 3 - Zero Sobreposição)
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x + 14.0f, item_p.y + 82.0f));
                            ImGui::TextColored(ImVec4(0.85f, 0.40f, 1.0f, 1.0f), "TOM: %s %s", m.camelot_code.c_str(), m.harmonic_relation.c_str());
                            ImGui::SameLine(0, 10.0f);
                            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.30f, 1.0f), "%.1f BPM (%+.1f%% drift)", m.bpm, m.bpm_drift_pct);
                            ImGui::SameLine(0, 10.0f);
                            ImGui::TextColored(ImVec4(0.55f, 0.75f, 0.95f, 0.90f), "[%s]", m.provider.c_str());

                            // AÇÕES NO CANTO DIREITO DO CARD (Alinhadas verticalmente no centro)
                            float btn_area_w = 370.0f;
                            float bx_actions = item_p.x + item_w - btn_area_w - 14.0f;
                            float by_actions = item_p.y + (item_h - 32.0f) * 0.5f;"""

content = content.replace(c_old, c_new)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)

print("Updated item_h to 120px and line positions to 10px, 45px, 82px!")

