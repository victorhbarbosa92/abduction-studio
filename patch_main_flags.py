import os

file_path = "src/main.cpp"
with open(file_path, "r", encoding="utf-8") as f:
    content = f.read()

target1 = """        } else if (arg == "--test-cloud-hub") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.cloud_hub_active_tab = 1;"""

replacement1 = """        } else if (arg == "--test-cloud-hub" || arg == "--test-radar-tab") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.search_modal_view = 0;
            if (arg == "--test-radar-tab") {
                KuroUI::g_dj_studio_ui.cloud_hub_active_tab = 3;
            } else {
                KuroUI::g_dj_studio_ui.cloud_hub_active_tab = 1;
            }"""

if target1 not in content:
    print("Error: target1 not found in main.cpp!")
    exit(1)
content = content.replace(target1, replacement1, 1)

target2 = """        } else if (arg == "--test-master-source-matrix") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.search_modal_view = 1;"""

replacement2 = """        } else if (arg == "--test-master-source-matrix" || arg == "--test-harmonic-dna") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            KuroUI::g_dj_studio_ui.show_search_modal = true;
            KuroUI::g_dj_studio_ui.search_modal_view = 1;
            if (arg == "--test-harmonic-dna") {
                KuroUI::g_dj_studio_ui.sources_tab_filter = 5;
            } else {
                KuroUI::g_dj_studio_ui.sources_tab_filter = 0;
            }"""

if target2 not in content:
    print("Error: target2 not found in main.cpp!")
    exit(1)
content = content.replace(target2, replacement2, 1)

tmp_path = file_path + ".tmp"
with open(tmp_path, "w", encoding="utf-8") as f:
    f.write(content)

os.system(f'cmd /c move /y "{tmp_path.replace("/", chr(92))}" "{file_path.replace("/", chr(92))}"')
print("main.cpp updated successfully!")
