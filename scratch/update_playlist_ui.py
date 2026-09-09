import sys

with open(r'src/ui/StudioUI.h', 'r', encoding='utf-8', errors='ignore') as f:
    lines = f.readlines()

# 1. Add #include "KuroPlaylistUI.h"
inc_idx = -1
for i, line in enumerate(lines[:350]):
    if 'inline KuroPsytranceRollingBassUI g_psy_rolling_bass_ui;' in line:
        inc_idx = i + 1
        break

if inc_idx == -1:
    print("Error: Could not find inclusion marker!")
    sys.exit(1)

lines.insert(inc_idx, '#include "KuroPlaylistUI.h"\n')
print(f"Inserted #include at line {inc_idx + 1}")

# 2. Find start of old playlist
start_idx = -1
for i, line in enumerate(lines):
    if 'if (show_playlist) {' in line and 2500 < i < 3500:
        start_idx = i
        break

if start_idx == -1:
    print("Error: Could not find start of old playlist block!")
    sys.exit(1)

# 3. Find end of old playlist
end_idx = -1
for i in range(start_idx, len(lines)):
    if '// PANEL: BROWSER' in lines[i]:
        # Walk back to find the closing brace
        for j in range(i - 1, start_idx, -1):
            if lines[j].strip() == '}':
                end_idx = j
                break
        break

if end_idx == -1:
    print("Error: Could not find end of old playlist block!")
    sys.exit(1)

print(f"Replacing lines {start_idx + 1} to {end_idx + 1}")
print("Line start:", lines[start_idx].strip())
print("Line end:", lines[end_idx].strip())

new_block = [
    '        // =============================================\n',
    '        // PANEL: PLAYLIST / SONG ARRANGER (REFORMULADO V2)\n',
    '        // =============================================\n',
    '        if (show_playlist) {\n',
    '            g_playlist_ui.render(&show_playlist);\n',
    '        }\n'
]

lines[start_idx:end_idx + 1] = new_block

with open(r'src/ui/StudioUI.h', 'w', encoding='utf-8') as f:
    f.writelines(lines)

print("Successfully replaced old playlist with KuroPlaylistUI in StudioUI.h!")
