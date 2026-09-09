import sys

# 1. Update main.cpp: Remove bottom inspector dock, dock DESKTOP in the center
main_cpp_path = 'src/main.cpp'
with open(main_cpp_path, 'r', encoding='utf-8', errors='ignore') as f:
    main_content = f.read()

old_dock_block = '''            // 1. Split Left para BROWSER (18% da largura)
            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.18f, NULL, &dock_main_id);

            // 2. Split do restante em Topo (PLAYLIST 72%) e Base (INSPECTOR 28%)
            ImGuiID dock_id_inspector = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.28f, NULL, &dock_main_id);
            ImGuiID dock_id_playlist = dock_main_id; // Top right (72% da altura)

            // ── Ancorar painéis ──────────────────
            ImGui::DockBuilderDockWindow("BROWSER", dock_id_left);
            ImGui::DockBuilderDockWindow("Browser", dock_id_left);
            ImGui::DockBuilderDockWindow("PLAYLIST", dock_id_playlist);
            ImGui::DockBuilderDockWindow("Playlist - [Arrangement View]", dock_id_playlist);
            ImGui::DockBuilderDockWindow("INSPECTOR", dock_id_inspector);
            ImGui::DockBuilderDockWindow("Inspector", dock_id_inspector);'''

new_dock_block = '''            // 1. Split Left para BROWSER (18% da largura)
            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.18f, NULL, &dock_main_id);

            // 2. O restante do espaço central é a Área de Trabalho Limpa (DESKTOP) do Abduction Studio
            ImGuiID dock_id_desktop = dock_main_id;

            // ── Ancorar painéis ──────────────────
            ImGui::DockBuilderDockWindow("BROWSER", dock_id_left);
            ImGui::DockBuilderDockWindow("Browser", dock_id_left);
            ImGui::DockBuilderDockWindow("DESKTOP", dock_id_desktop);'''

if old_dock_block in main_content:
    main_content = main_content.replace(old_dock_block, new_dock_block)
    with open(main_cpp_path, 'w', encoding='utf-8') as f:
        f.write(main_content)
    print("main.cpp updated successfully!")
else:
    print("Dock block in main.cpp not found!")

# 2. Update StudioUI.h: Set show_inspector = false, show_playlist = false, show_piano_roll = false, show_step_sequencer = false
studio_path = 'src/ui/StudioUI.h'
with open(studio_path, 'r', encoding='utf-8', errors='ignore') as f:
    studio_content = f.read()

# Change default flags
studio_content = studio_content.replace('inline bool show_inspector = true;', 'inline bool show_inspector = false;')
studio_content = studio_content.replace('inline bool show_playlist = true;', 'inline bool show_playlist = false;')
studio_content = studio_content.replace('inline bool show_piano_roll = true;', 'inline bool show_piano_roll = false;')
studio_content = studio_content.replace('inline bool show_step_sequencer = true;', 'inline bool show_step_sequencer = false;')

with open(studio_path, 'w', encoding='utf-8') as f:
    f.write(studio_content)
print("Flags in StudioUI.h updated successfully!")
