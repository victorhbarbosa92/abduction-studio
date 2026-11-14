import sys

path = 'src/ui/StudioUI.h'
with open(path, 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

old_popup_block = '''            // =====================================================================
            // 6. POP-UP DE GAVETA DE APLICATIVOS (AO CLICAR NA PASTA)
            // =====================================================================
            if (g_open_folder_idx >= 0 && g_open_folder_idx < 5) {
                ImGui::OpenPopup("##AndroidAppDrawerPopup");
            }

            ImGui::SetNextWindowPos(ImVec2(cx - 275.0f, cy - 170.0f), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(550.0f, 340.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.10f, 0.12f, 0.17f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.80f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);

            if (ImGui::BeginPopup("##AndroidAppDrawerPopup")) {
                int f = g_open_folder_idx;
                if (f >= 0 && f < 5) {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "FOLDER: %s", folders[f].title);
                    ImGui::SameLine(0, 10);
                    ImGui::TextDisabled("| %s", folders[f].subtitle);
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 65);
                    if (ImGui::Button("X Fechar", ImVec2(60, 20))) {
                        g_open_folder_idx = -1;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::Separator();
                    ImGui::Spacing();'''

new_popup_block = '''            // =====================================================================
            // 6. GAVETA DE APLICATIVOS FLUTUANTE ESTILO ANDROID (AO CLICAR NA PASTA)
            // =====================================================================
            if (g_open_folder_idx >= 0 && g_open_folder_idx < 5) {
                int f = g_open_folder_idx;
                ImGui::SetNextWindowPos(ImVec2(cx - 280.0f, cy - 180.0f), ImGuiCond_Appearing);
                ImGui::SetNextWindowSize(ImVec2(560.0f, 360.0f));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.12f, 0.17f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, folders[f].glow_color);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

                bool drawer_open = true;
                char win_id[64];
                snprintf(win_id, sizeof(win_id), "PASTA: %s (Abduction Hub)###AndroidFolderDrawer", folders[f].title);
                if (ImGui::Begin(win_id, &drawer_open, ImGuiWindowFlags_NoCollapse)) {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "PASTA: %s", folders[f].title);
                    ImGui::SameLine(0, 10);
                    ImGui::TextDisabled("| %s", folders[f].subtitle);
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 65);
                    if (ImGui::Button("X Fechar", ImVec2(60, 20))) {
                        drawer_open = false;
                    }
                    ImGui::Separator();
                    ImGui::Spacing();'''

old_popup_end = '''                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);'''

new_popup_end = '''                }
                ImGui::End();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                if (!drawer_open) {
                    g_open_folder_idx = -1;
                }
            }'''

if old_popup_block in content and old_popup_end in content:
    content = content.replace(old_popup_block, new_popup_block)
    content = content.replace(old_popup_end, new_popup_end)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print("Drawer updated to floating Android window successfully!")
else:
    print("Popup block not found exactly, will check")
