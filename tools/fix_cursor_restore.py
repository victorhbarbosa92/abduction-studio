path = "C:/Users/o_vic/.gemini/antigravity-ide/scratch/abduction_studio_v2/src/ui/KuroDJStudioUI.h"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# 1. Header Banner cursor restoration
h_target = """                    float ddh_h = 66.0f;
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

h_repl = """                    float ddh_h = 56.0f;
                    rdl->AddRectFilled(ddh_pos, ImVec2(ddh_pos.x + ddh_w, ddh_pos.y + ddh_h), IM_COL32(18, 10, 32, 255), 5.0f);
                    rdl->AddRect(ddh_pos, ImVec2(ddh_pos.x + ddh_w, ddh_pos.y + ddh_h), IM_COL32(190, 60, 255, 180), 5.0f, 0, 1.2f);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x + 12.0f, ddh_pos.y + 8.0f));
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

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x + 12.0f, ddh_pos.y + 30.0f));
                    std::string ref_title_disp = spotify_service.getDigDeeperRefTitle().empty() ? hero_title : spotify_service.getDigDeeperRefTitle();
                    ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.88f, 0.90f), "DNA Acústico de Referência: \\"%s\\" • Tom: %s • BPM: %.1f",
                        ref_title_disp.c_str(), k_str.c_str(), selected_source_track_bpm > 20.0 ? selected_source_track_bpm : 145.0);

                    ImGui::SetCursorScreenPos(ImVec2(ddh_pos.x, ddh_pos.y + ddh_h + 12.0f));"""

content = content.replace(h_target, h_repl)

# 2. Match Card cursor restoration at end of each card
end_card_target = """                                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Carregar no Deck %d e registrar reforço de transição no Grafo de Conhecimento", db.deck + 1);
                                ImGui::PopStyleVar(2);
                                ImGui::PopStyleColor(3);
                            }

                            ImGui::PopID();
                            ImGui::Spacing();
                        }"""

end_card_repl = """                                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Carregar no Deck %d e registrar reforço de transição no Grafo de Conhecimento", db.deck + 1);
                                ImGui::PopStyleVar(2);
                                ImGui::PopStyleColor(3);
                            }

                            ImGui::PopID();
                            // Restaura o cursor para baixo do cartão com margem limpa
                            ImGui::SetCursorScreenPos(ImVec2(item_p.x, item_p.y + item_h + 8.0f));
                        }"""

content = content.replace(end_card_target, end_card_repl)

# Remove the initial Dummy in card since SetCursorScreenPos at the end governs the position
dummy_card_target = """                            float item_w = ImGui::GetContentRegionAvail().x;
                            float item_h = 120.0f;
                            ImVec2 item_p = ImGui::GetCursorScreenPos();
                            ImGui::Dummy(ImVec2(item_w, item_h));"""

dummy_card_repl = """                            float item_w = ImGui::GetContentRegionAvail().x;
                            float item_h = 110.0f;
                            ImVec2 item_p = ImGui::GetCursorScreenPos();"""

content = content.replace(dummy_card_target, dummy_card_repl)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)

print("Fixed cursor restoration at end of cards!")

