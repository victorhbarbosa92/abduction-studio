with open('src/main.cpp', 'r', encoding='utf-8') as f:
    c = f.read()

target = '''        } else if (arg == "--test-stems-pad" || arg == "--test-dj-stems") {
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {'''

repl = '''        } else if (arg == "--test-stems-pad" || arg == "--test-dj-stems") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {'''

target2 = '''        } else if (arg == "--test-autodj" || arg == "--test-dj-autodj") {
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {'''

repl2 = '''        } else if (arg == "--test-autodj" || arg == "--test-dj-autodj") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {'''

target3 = '''        } else if (arg == "--test-audio-routing-cue" || arg == "--test-dj-cue") {
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {'''

repl3 = '''        } else if (arg == "--test-audio-routing-cue" || arg == "--test-dj-cue") {
            KuroUI::g_home_dashboard_ui.close();
            KuroUI::show_dj_modal = true;
            if (g_dj_engine) {'''

c = c.replace(target, repl, 1)
c = c.replace(target2, repl2, 1)
c = c.replace(target3, repl3, 1)

with open('src/main.cpp', 'w', encoding='utf-8') as f:
    f.write(c)
print("SUCCESS")
