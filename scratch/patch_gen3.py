with open('scratch/generate_playlist_ui.py', 'r', encoding='utf-8') as f:
    code = f.read()

target = """extern bool show_piano_roll;
extern bool show_psy_arranger_modal;

namespace KuroUI {"""

replacement = """namespace KuroUI {
    extern bool show_piano_roll;
    extern bool show_psy_arranger_modal;"""

assert target in code, "target not found"
code = code.replace(target, replacement)

with open('scratch/generate_playlist_ui.py', 'w', encoding='utf-8') as f:
    f.write(code)

print("scratch/generate_playlist_ui.py updated with namespace KuroUI externs.")
