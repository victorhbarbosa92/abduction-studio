with open('src/ui/KuroDJStudioUI.h', 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

target = 'float center_x = wave_x + wave_w * 0.5f;'
replacement = 'float center_x = wave_x + wave_w * 0.5f;\n            const char* hot_cue_letters[8] = { "A", "B", "C", "D", "E", "F", "G", "H" };'

# Remove the inner declaration at line 1876 if present
inner_decl = 'const char* hot_cue_letters[8] = { "A", "B", "C", "D", "E", "F", "G", "H" };'

# Check if target already has it
if target in content:
    content = content.replace(target, replacement, 1)

with open('src/ui/KuroDJStudioUI.h', 'w', encoding='utf-8') as f:
    f.write(content)

print("Applied top-level hot_cue_letters declaration.")
