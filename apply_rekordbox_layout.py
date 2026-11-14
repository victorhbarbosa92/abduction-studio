import sys

file_path = "src/ui/KuroDJStudioUI.h"
with open(file_path, "r", encoding="utf-8") as f:
    content = f.read()

# 1. State variables to add after dj_layout_mode
old_vars = "int dj_layout_mode = 0; // 0 = PRO CONSOLE (Tudo na tela sem corte), 1 = WAVE MAX, 2 = SPLIT GAVETA"
new_vars = """int dj_layout_mode = 0; // 0 = REKORDBOX OFICIAL 1:1, 1 = WAVE MAX, 2 = SPLIT GAVETA, 3 = RETRO PRO CONSOLE
        bool show_rekordbox_fx_bar = true;
        int rekordbox_pad_mode[2] = { 0, 0 }; // 0 = HOT CUE, 1 = BEAT LOOP, 2 = PAD FX 1, 3 = SAMPLER
        int rekordbox_fx1_type = 1;
        int rekordbox_fx2_type = 0;
        float rekordbox_fx1_param = 0.5f;
        float rekordbox_fx2_param = 0.5f;
        float rekordbox_cfx_param = 0.5f;
        bool rekordbox_fx1_ch[6] = { false, true, false, false, false, false };
        bool rekordbox_fx2_ch[6] = { false, false, false, false, true, false };"""

if old_vars in content:
    content = content.replace(old_vars, new_vars, 1)
    print("[1] Injected Rekordbox state variables")
else:
    print("[1] WARNING: old_vars not found")

# Save and verify
with open(file_path, "w", encoding="utf-8") as f:
    f.write(content)
print("Updated", file_path)
