import os
import glob
import json
import shutil
import re

TRACKS_DIR = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\scratch\tracks"
LIB_PATH = os.path.join(TRACKS_DIR, "dj_library.json")

def sanitize(name):
    # Remove chars invalid in Windows filenames
    clean = re.sub(r'[\\/*?:"<>|]', "", name)
    clean = re.sub(r'\s+', " ", clean).strip()
    return clean

def main():
    if not os.path.exists(LIB_PATH):
        print("Library file not found:", LIB_PATH)
        return

    with open(LIB_PATH, "r", encoding="utf-8") as f:
        lib = json.load(f)

    tracks = lib.get("tracks", [])
    print(f"Total tracks in library: {len(tracks)}")

    loose_wavs = glob.glob(os.path.join(TRACKS_DIR, "*.wav"))
    print(f"Loose WAVs in tracks root: {len(loose_wavs)}")
    for w in loose_wavs[:15]:
        print("  ", os.path.basename(w))

    export_wavs = glob.glob(os.path.join(TRACKS_DIR, "export", "*.wav"))
    print(f"WAVs in export: {len(export_wavs)}")
    for w in export_wavs[:15]:
        print("  ", os.path.basename(w))

if __name__ == "__main__":
    main()
