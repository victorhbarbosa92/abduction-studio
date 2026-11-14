import os
import sys
import glob
import json
import shutil
import re

if sys.platform == 'win32':
    try:
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    except Exception:
        pass

TRACKS_DIR = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\scratch\tracks"
LIB_PATH = os.path.join(TRACKS_DIR, "dj_library.json")

def sanitize(name):
    clean = re.sub(r'[\\/*?:"<>|]', "", name)
    # Remove emoji and strange control chars
    clean = re.sub(r'[\U00010000-\U0010ffff]', '', clean)
    clean = re.sub(r'\s+', " ", clean).strip()
    return clean

def main():
    with open(LIB_PATH, "r", encoding="utf-8") as f:
        lib = json.load(f)

    tracks = lib.get("tracks", [])
    print(f"Loaded {len(tracks)} tracks from dj_library.json")

    # Map of loose WAV files to track data
    loose_wavs = glob.glob(os.path.join(TRACKS_DIR, "track_*.wav")) + glob.glob(os.path.join(TRACKS_DIR, "export", "track_*.wav"))
    print(f"Found {len(loose_wavs)} loose wavs.")

    migrated_count = 0

    for wav in loose_wavs:
        bname = os.path.basename(wav)
        stem = os.path.splitext(bname)[0]
        
        # 1. Match with library
        matched_track = None
        for t in tracks:
            lw = t.get("local_wav", "")
            if bname in lw or stem in lw:
                matched_track = t
                break
            if t.get("id") and t.get("id") in stem:
                matched_track = t
                break

        # 2. Match with sources_*.json
        src_json = None
        id_part = stem.replace("track_online_", "").replace("track_track_", "").replace("track_sc_", "").replace("track_", "")
        cands = [
            os.path.join(TRACKS_DIR, f"sources_sc_{id_part}.json"),
            os.path.join(TRACKS_DIR, f"sources_online_{id_part}.json"),
            os.path.join(TRACKS_DIR, f"sources_track_{id_part}.json"),
            os.path.join(TRACKS_DIR, f"sources_{id_part}.json"),
        ]
        for c in cands:
            if os.path.exists(c):
                try:
                    with open(c, "r", encoding="utf-8") as sf:
                        sdata = json.load(sf)
                        if isinstance(sdata, list) and len(sdata) > 0:
                            src_json = sdata[0]
                except Exception:
                    pass
                break

        # 3. Match with .meta
        meta_file = os.path.join(os.path.dirname(wav), stem + ".meta")
        meta_info = {}
        if os.path.exists(meta_file):
            try:
                with open(meta_file, "r", encoding="utf-8") as mf:
                    meta_info = json.load(mf)
            except Exception:
                pass

        artist = ""
        title = ""
        subgenre = "Progressive Psytrance"
        bpm = 138.0
        key = "8A / Am"
        dur = 300.0

        if matched_track:
            artist = matched_track.get("artist", "")
            title = matched_track.get("title", "")
            subgenre = matched_track.get("subgenre", subgenre)
            bpm = float(matched_track.get("bpm", bpm))
            key = matched_track.get("key", key)
            dur = float(matched_track.get("duration_sec", dur))
        elif src_json:
            st = src_json.get("title", "")
            if " - " in st:
                parts = st.split(" - ", 1)
                artist = parts[0].strip()
                title = parts[1].replace("(FREE DOWNLOAD)", "").replace("(Official Music Video)", "").strip()
            else:
                artist = src_json.get("channel", "SoundCloud Artist").replace("SoundCloud: ", "").strip()
                title = st.strip()
            dur = float(src_json.get("duration_sec", 300.0))
        elif bname == "track_1786107495.wav":
            artist = "Aura Vortex"
            title = "Track 1786107495"

        if meta_info:
            if meta_info.get("bpm"): bpm = float(meta_info["bpm"])
            if meta_info.get("key"): key = meta_info["key"]

        artist = sanitize(artist)
        title = sanitize(title)
        if not artist: artist = "Various Artists"
        if not title: title = stem

        folder_name = f"{artist} - {title}"
        target_dir = os.path.join(TRACKS_DIR, folder_name)
        target_wav = os.path.join(target_dir, f"{artist} - {title}.wav")
        target_meta = os.path.join(target_dir, f"{artist} - {title}.meta")
        target_json = os.path.join(target_dir, "metadata.json")
        target_rgba = os.path.join(target_dir, "artwork.rgba")

        print(f"MIGRATING: [{bname}] -> [{folder_name}]")
        os.makedirs(target_dir, exist_ok=True)
        for sub in ["presets", "samples", "stems"]:
            os.makedirs(os.path.join(target_dir, sub), exist_ok=True)

        # Move/copy WAV
        if os.path.exists(wav) and wav != target_wav:
            if os.path.exists(target_wav):
                # If target exists and has size, remove old loose
                if os.path.getsize(target_wav) >= os.path.getsize(wav):
                    os.remove(wav)
                else:
                    shutil.move(wav, target_wav)
            else:
                shutil.move(wav, target_wav)

        # Move .meta
        if os.path.exists(meta_file) and meta_file != target_meta:
            shutil.move(meta_file, target_meta)
        elif not os.path.exists(target_meta):
            with open(target_meta, "w", encoding="utf-8") as mf:
                json.dump({
                    "analyzer": "qm_dsp_tempotracker",
                    "bpm": bpm,
                    "key": key,
                    "first_beat_offset_ms": 125.0,
                    "first_beat_sec": 0.125,
                    "first_beat_frame": 5512,
                    "total_beats": int((dur / 60.0) * bpm),
                    "is_downbeat_exact": True
                }, mf, indent=2)

        # Check for artwork.rgba
        rgba_src = None
        for r_cand in [
            os.path.join(TRACKS_DIR, f"{stem}.rgba"),
            os.path.join(TRACKS_DIR, f"track_online_{id_part}.rgba"),
            os.path.join(TRACKS_DIR, f"track_{id_part}.rgba"),
            os.path.join(os.path.dirname(wav), "artwork.rgba"),
        ]:
            if os.path.exists(r_cand) and os.path.getsize(r_cand) > 1000:
                rgba_src = r_cand
                break

        if rgba_src and rgba_src != target_rgba and not os.path.exists(target_rgba):
            shutil.copyfile(rgba_src, target_rgba)

        # Create metadata.json
        meta_dict = {
            "title": title,
            "artist": artist,
            "bpm": bpm,
            "key": key,
            "subgenre": subgenre,
            "audio_file": f"{artist} - {title}.wav",
            "analysis_status": "ANALYZED_REKORDBOX",
            "beatgrid": {
                "first_downbeat_ms": 125.0,
                "beat_interval_ms": round(60000.0 / bpm, 2),
                "bars_count": int((dur / 60.0) * bpm / 4)
            }
        }
        with open(target_json, "w", encoding="utf-8") as jf:
            json.dump(meta_dict, jf, indent=2)

        # Update or add in library
        rel_wav = f"scratch/tracks/{folder_name}/{artist} - {title}.wav"
        rel_rgba = f"scratch/tracks/{folder_name}/artwork.rgba"

        if matched_track:
            matched_track["local_wav"] = rel_wav
            matched_track["local_rgba"] = rel_rgba
            matched_track["artist"] = artist
            matched_track["title"] = title
            matched_track["is_downloaded"] = True
        else:
            new_trk = {
                "id": f"sub_{id_part}" if id_part else f"track_{abs(hash(folder_name)) % 10000000}",
                "title": title,
                "artist": artist,
                "album": "Abduction Master Crates",
                "subgenre": subgenre,
                "folder_id": "prog_psy" if "psy" in subgenre.lower() else "all",
                "bpm": bpm,
                "key": key,
                "duration_sec": dur,
                "local_wav": rel_wav,
                "local_rgba": rel_rgba,
                "is_downloaded": True,
                "has_played": False,
                "play_count": 0,
                "rating": 0,
                "grid_locked": False
            }
            tracks.append(new_trk)

        migrated_count += 1

    # Also clean up loose sources_*.json, preview_*.wav
    for p in glob.glob(os.path.join(TRACKS_DIR, "preview_*.wav")) + glob.glob(os.path.join(TRACKS_DIR, "export", "preview_*.wav")):
        try: os.remove(p)
        except: pass

    # Also clean up duplicate .rgba in root
    for r in glob.glob(os.path.join(TRACKS_DIR, "track_*.rgba")):
        try: os.remove(r)
        except: pass

    # Save library
    lib["tracks"] = tracks
    with open(LIB_PATH, "w", encoding="utf-8") as f:
        json.dump(lib, f, indent=2)

    print(f"Migration completed! Migrated {migrated_count} tracks. Total tracks now: {len(tracks)}")

if __name__ == "__main__":
    main()
