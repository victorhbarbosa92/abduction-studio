with open('src/ui/KuroDJStudioUI.h', 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

# Define hot_cue_letters right at the start of renderStackedRekordboxWaveforms
old_target = 'float center_x = wave_x + wave_w * 0.5f;'
new_target = 'float center_x = wave_x + wave_w * 0.5f;\n            const char* hot_cue_letters[8] = { "A", "B", "C", "D", "E", "F", "G", "H" };'

if 'const char* hot_cue_letters[8]' not in content[:content.find('void renderUSBExportDrawer')]:
    content = content.replace(old_target, new_target, 1)

with open('src/ui/KuroDJStudioUI.h', 'w', encoding='utf-8') as f:
    f.write(content)

print("Fixed hot_cue_letters scope.")
