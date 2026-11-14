import os
import glob
import json
import sys

# Ensure tools directory is in sys.path
tools_dir = os.path.abspath("tools")
if tools_dir not in sys.path:
    sys.path.insert(0, tools_dir)

from rekordbox_audio_analyzer import analyze_track_rekordbox

tracks_dir = os.path.abspath("scratch/tracks")
wavs = sorted(glob.glob(os.path.join(tracks_dir, "*.wav")))
# Filter out non-track temporary audio files
wavs = [w for w in wavs if not os.path.basename(w).startswith("temp_") and not os.path.basename(w).startswith("test_")]

print(f"=== REPROCESSAMENTO REKORDBOX ML: {len(wavs)} FAIXAS COMPLETAS ===")
calibrated = {}

for i, w in enumerate(wavs):
    fname = os.path.basename(w)
    track_id = fname.replace("track_", "").replace("preview_", "").replace(".wav", "")
    print(f"[{i+1}/{len(wavs)}] Processando {fname}...")
    try:
        res = analyze_track_rekordbox(w)
        meta_path = os.path.splitext(w)[0] + ".meta"
        
        # Load existing meta if available to preserve title/artist/album/subgenre
        old_data = {}
        if os.path.exists(meta_path):
            try:
                old_data = json.load(open(meta_path, "r", encoding="utf-8", errors="ignore"))
            except: pass
            
        merged = {**old_data, **res}
        with open(meta_path, "w", encoding="utf-8") as f:
            json.dump(merged, f, indent=2, ensure_ascii=False)
            
        calibrated[track_id] = merged
        print(f"   -> BPM: {res['bpm']} | Tom: {res['key']} | Downbeat 1.1: {res['first_beat_offset_ms']:.2f}ms (frame {res['first_beat_frame']})")
    except Exception as e:
        print(f"   [ERRO]: {e}")

# 2. Atualizar dj_library.json
lib_path = os.path.join(tracks_dir, "dj_library.json")
if os.path.exists(lib_path):
    try:
        lib_data = json.load(open(lib_path, "r", encoding="utf-8", errors="ignore"))
        tracks = lib_data.get("tracks", [])
        updated_count = 0
        for t in tracks:
            tid = t.get("id")
            if tid in calibrated:
                c = calibrated[tid]
                t["bpm"] = c["bpm"]
                t["key"] = c["key"]
                updated_count += 1
            else:
                # Checa por correspondencia de wav_path
                lp = t.get("local_wav_path") or ""
                for cid, c in calibrated.items():
                    if cid in lp or os.path.basename(lp) == f"track_{cid}.wav":
                        t["bpm"] = c["bpm"]
                        t["key"] = c["key"]
                        updated_count += 1
                        break
        with open(lib_path, "w", encoding="utf-8") as f:
            json.dump(lib_data, f, indent=2, ensure_ascii=False)
        print(f"\n[dj_library.json] Atualizadas {updated_count} faixas com BPM de alta precisao Rekordbox!")
    except Exception as e:
        print(f"Erro ao atualizar dj_library.json: {e}")

# 3. Atualizar search_results.tsv se existir
tsv_path = os.path.join(tracks_dir, "search_results.tsv")
if os.path.exists(tsv_path):
    try:
        lines = open(tsv_path, "r", encoding="utf-8", errors="ignore").readlines()
        new_lines = []
        for line in lines:
            parts = line.strip().split("\t")
            if len(parts) >= 8:
                tid = parts[0]
                if tid in calibrated:
                    c = calibrated[tid]
                    parts[7] = f"{c['bpm']:.2f}"
                    if len(parts) >= 9:
                        parts[8] = c["key"]
                new_lines.append("\t".join(parts) + "\n")
            else:
                new_lines.append(line)
        with open(tsv_path, "w", encoding="utf-8") as f:
            f.writelines(new_lines)
        print(f"[search_results.tsv] Valores de BPM e Key sincronizados com sucesso!")
    except Exception as e:
        print(f"Erro ao atualizar TSV: {e}")

print("\n=== CONCLUIDO COM SUCESSO! ===")
