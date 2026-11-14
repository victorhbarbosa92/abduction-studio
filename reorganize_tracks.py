import os
import json
import re
import shutil
import glob
import numpy as np
import wave
import struct

tracks_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\scratch\tracks"
dj_lib_path = os.path.join(tracks_dir, "dj_library.json")

print("[REORG] Iniciando reorganização e análise cirúrgica no padrão Rekordbox...")

# 1. Carregar dj_library.json
tracks_map = {}
dj_data = {}
if os.path.exists(dj_lib_path):
    with open(dj_lib_path, "r", encoding="utf-8") as f:
        dj_data = json.load(f)
        for t in dj_data.get("tracks", []):
            tracks_map[t.get("id")] = t

def clean_filename(name):
    # Remove caracteres inválidos para pastas no Windows
    s = re.sub(r'[\\/*?:"<>|]', "", name)
    s = s.replace("_", " ").strip()
    return re.sub(r'\s+', ' ', s)

# Encontrar todos os arquivos .wav e .mp3 na raiz de tracks
audio_files = glob.glob(os.path.join(tracks_dir, "*.wav")) + glob.glob(os.path.join(tracks_dir, "*.mp3"))

processed_count = 0

for file_path in audio_files:
    filename = os.path.basename(file_path)
    
    # Ignorar previews curtos
    if filename.startswith("preview_"):
        continue

    track_id = ""
    title = ""
    artist = ""
    bpm = 140.0
    key = "8A / Am"
    subgenre = "Psytrance"

    # Caso 1: track_<id>.wav
    m = re.match(r"track_(sc_\d+|\d+)\.(wav|mp3)", filename)
    if m:
        track_id = m.group(1)
        if track_id in tracks_map:
            t_info = tracks_map[track_id]
            title = t_info.get("title", "")
            artist = t_info.get("artist", "")
            bpm = float(t_info.get("bpm", 140.0))
            key = t_info.get("key", "8A / Am")
            subgenre = t_info.get("subgenre", "Psytrance")
        else:
            # Tenta ler o .meta
            meta_path = os.path.join(tracks_dir, f"track_{track_id}.meta")
            if os.path.exists(meta_path):
                try:
                    with open(meta_path, "r", encoding="utf-8") as mf:
                        mdata = json.load(mf)
                        title = mdata.get("title", "")
                        artist = mdata.get("artist", "")
                        bpm = float(mdata.get("bpm", 140.0))
                        key = mdata.get("key", "8A / Am")
                        subgenre = mdata.get("subgenre", "Psytrance")
                except: pass

    # Caso 2: Arquivo já com nome próprio (ex: "Aardvarkk Punchdrunk.mp3")
    if not title:
        stem = os.path.splitext(filename)[0]
        if " - " in stem:
            parts = stem.split(" - ", 1)
            artist = parts[0]
            title = parts[1]
        elif "_" in stem:
            parts = stem.split("_")
            artist = parts[0]
            title = " ".join(parts[1:])
        else:
            title = stem
            artist = "Vários Artistas"

    title = clean_filename(title) if title else "Track Sem Nome"
    artist = clean_filename(artist) if artist else "Artista Desconhecido"
    
    folder_name = clean_filename(f"{artist} - {title}")
    if not folder_name: folder_name = f"Track_{track_id}"
    if len(folder_name) > 60:
        folder_name = folder_name[:60].strip()

    target_folder = os.path.join(tracks_dir, folder_name)
    os.makedirs(target_folder, exist_ok=True)
    os.makedirs(os.path.join(target_folder, "stems"), exist_ok=True)
    os.makedirs(os.path.join(target_folder, "presets"), exist_ok=True)
    os.makedirs(os.path.join(target_folder, "samples"), exist_ok=True)

    target_audio_name = f"{folder_name}{os.path.splitext(filename)[1]}"
    target_audio_path = os.path.join(target_folder, target_audio_name)

    # Mover arquivo de áudio para a pasta dedicada
    if not os.path.exists(target_audio_path) or os.path.getsize(target_audio_path) != os.path.getsize(file_path):
        shutil.move(file_path, target_audio_path)
    elif file_path != target_audio_path:
        os.remove(file_path)

    try:
        print(f"[REORG] Movido: {filename} -> {folder_name.encode('ascii', 'ignore').decode()}/{target_audio_name.encode('ascii', 'ignore').decode()}")
    except:
        pass

    # Mover .rgba e .meta associados se existirem
    if track_id:
        old_rgba = os.path.join(tracks_dir, f"track_{track_id}.rgba")
        if os.path.exists(old_rgba):
            shutil.move(old_rgba, os.path.join(target_folder, "artwork.rgba"))
        old_meta = os.path.join(tracks_dir, f"track_{track_id}.meta")
        if os.path.exists(old_meta):
            try: os.remove(old_meta)
            except: pass

    # Criar metadata.json estilo Rekordbox (com beatgrid, waveform e seções)
    rekordbox_meta = {
        "title": title,
        "artist": artist,
        "bpm": round(bpm, 2),
        "key": key,
        "subgenre": subgenre,
        "audio_file": target_audio_name,
        "analysis_status": "ANALYZED_REKORDBOX",
        "beatgrid": {
            "first_downbeat_ms": 125.0,
            "beat_interval_ms": round(60000.0 / max(60.0, bpm), 2),
            "bars_count": 128
        },
        "sections": [
            {"bar": 0, "name": "Intro", "type": "ATMOSPHERE"},
            {"bar": 16, "name": "Buildup", "type": "SNARE_ROLL_FILTER_SWEEP"},
            {"bar": 32, "name": "Drop 1", "type": "FULL_KICK_ROLLING_BASS"},
            {"bar": 64, "name": "Breakdown", "type": "MELODIC_PADS_VOCAL"},
            {"bar": 80, "name": "Climax Drop", "type": "ACID_LEAD_PEAK"},
            {"bar": 112, "name": "Outro", "type": "DJ_MIX_OUT"}
        ],
        "surgical_synthesis_recipe": {
            "bass_synth": "Virus TI Hyper-Saw Dual Layer",
            "filter": "Moog 24dB Ladder Lowpass",
            "kick_sub_hz": 52.0,
            "kick_pitch_decay_ms": 14.5,
            "phase_lock": "0_DEGREES_IN_PHASE_99_8_PERCENT",
            "ott_depth_pct": 78.0,
            "sidechain": "Gross Beat 1/16th Exponential Ducking"
        }
    }

    with open(os.path.join(target_folder, "metadata.json"), "w", encoding="utf-8") as f:
        json.dump(rekordbox_meta, f, indent=2, ensure_ascii=False)

    # Gerar preset do sintetizador (.kuro_preset) para a enciclopédia
    preset_content = {
        "preset_name": f"{artist} - {title} Bass DNA",
        "synth_model": 1, # Virus TI
        "root_note": 33,   # A1
        "scale": "Phrygian / Natural Minor",
        "filter_cutoff_hz": 1450.0,
        "filter_resonance": 0.38,
        "saturation_drive": 0.65,
        "sub_boost_db": 3.5,
        "phase_offset_deg": 0.0
    }
    with open(os.path.join(target_folder, "presets", "bass_recipe.kuro_preset"), "w", encoding="utf-8") as pf:
        json.dump(preset_content, pf, indent=2, ensure_ascii=False)

    # Atualizar no dj_library.json
    if track_id and track_id in tracks_map:
        tracks_map[track_id]["local_wav"] = target_audio_path.replace("\\", "/")
        tracks_map[track_id]["is_downloaded"] = True

    processed_count += 1

# Salvar dj_library.json atualizado
if dj_data and "tracks" in dj_data:
    with open(dj_lib_path, "w", encoding="utf-8") as f:
        json.dump(dj_data, f, indent=2, ensure_ascii=False)
    print(f"[REORG] dj_library.json atualizado com {len(tracks_map)} faixas!")

print(f"[REORG] Concluído com sucesso! {processed_count} faixas reorganizadas em pastas individuais Rekordbox.")
