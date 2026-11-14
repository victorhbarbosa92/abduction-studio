import os
import shutil
import json

tracks_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\scratch\tracks"
export_dir = os.path.join(tracks_dir, "export")

print("Cleaning tracks directory:", tracks_dir)

# Remove all files in tracks_dir
for item in os.listdir(tracks_dir):
    p = os.path.join(tracks_dir, item)
    if os.path.isfile(p):
        os.remove(p)
        print("Removed file:", item)

# Clean export dir
if os.path.exists(export_dir):
    for item in os.listdir(export_dir):
        p = os.path.join(export_dir, item)
        if os.path.isdir(p):
            shutil.rmtree(p, ignore_errors=True)
            print("Removed export dir:", item)
        elif os.path.isfile(p):
            os.remove(p)
            print("Removed export file:", item)

# Create empty dj_library.json
empty_lib = {
    "version": "1.0",
    "tracks": []
}
with open(os.path.join(tracks_dir, "dj_library.json"), "w", encoding="utf-8") as f:
    json.dump(empty_lib, f, indent=2)
print("Created empty dj_library.json")

# Create empty hot_cues_db.json
with open(os.path.join(tracks_dir, "hot_cues_db.json"), "w", encoding="utf-8") as f:
    json.dump({}, f, indent=2)
print("Created empty hot_cues_db.json")

print("\n[OK] Todas as músicas foram limpas com sucesso! Crates e biblioteca 100% zerados.")
