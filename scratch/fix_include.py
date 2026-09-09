with open(r'src/ui/StudioUI.h', 'r', encoding='utf-8') as f:
    lines = f.readlines()

# Remove any existing KuroPlaylistUI.h include
lines = [l for l in lines if 'KuroPlaylistUI.h' not in l]

# Insert after KuroPsytranceRollingBassUI.h
insert_idx = -1
for i, l in enumerate(lines[:120]):
    if 'KuroPsytranceRollingBassUI.h' in l:
        insert_idx = i + 1
        break

if insert_idx != -1:
    lines.insert(insert_idx, '#include "KuroPlaylistUI.h"\n')
    print(f"Inserted include at line {insert_idx + 1}")
else:
    print("Could not find KuroPsytranceRollingBassUI.h")
    exit(1)

with open(r'src/ui/StudioUI.h', 'w', encoding='utf-8') as f:
    f.writelines(lines)

print("Updated StudioUI.h successfully!")
