import os
import sys
import time
import wave
import numpy as np

def load_wav_file(wav_path):
    with wave.open(wav_path, 'rb') as wf:
        n_ch = wf.getnchannels()
        sr = wf.getframerate()
        n_frames = wf.getnframes()
        sampwidth = wf.getsampwidth()
        raw_bytes = wf.readframes(n_frames)
        
    if sampwidth == 2:
        data = np.frombuffer(raw_bytes, dtype=np.int16).astype(np.float32) / 32768.0
    elif sampwidth == 3:
        raw_int8 = np.frombuffer(raw_bytes, dtype=np.uint8)
        n_samples = len(raw_int8) // 3
        raw_int8 = raw_int8[:n_samples * 3].reshape(-1, 3)
        b0 = raw_int8[:, 0].astype(np.int32)
        b1 = raw_int8[:, 1].astype(np.int32)
        b2 = raw_int8[:, 2].astype(np.int32)
        raw32 = (b0) | (b1 << 8) | (b2 << 16)
        raw32 = np.where(raw32 >= 0x800000, raw32 - 0x1000000, raw32)
        data = raw32.astype(np.float32) / 8388608.0
    elif sampwidth == 4:
        data = np.frombuffer(raw_bytes, dtype=np.float32)
    else:
        data = np.frombuffer(raw_bytes, dtype=np.int16).astype(np.float32) / 32768.0

    if n_ch == 2:
        data = data.reshape(-1, 2)
    else:
        data = np.column_stack((data, data))
        
    return sr, data

def save_wav_file(path, data, sr):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if data.ndim == 1:
        data = np.column_stack((data, data))
    data = np.clip(data, -1.0, 1.0)
    int16_data = (data * 32767.0).astype(np.int16)
    with wave.open(path, 'wb') as wf:
        wf.setnchannels(2)
        wf.setsampwidth(2)
        wf.setframerate(sr)
        wf.writeframes(int16_data.tobytes())

def apply_fade(data, fade_samples=256):
    if len(data) <= fade_samples * 2:
        return data
    out = np.copy(data)
    fade_in = np.linspace(0.0, 1.0, fade_samples)
    fade_out = np.linspace(1.0, 0.0, fade_samples)
    if out.ndim == 2:
        fade_in = fade_in[:, None]
        fade_out = fade_out[:, None]
    out[:fade_samples] *= fade_in
    out[-fade_samples:] *= fade_out
    return out

def find_best_energy_start(audio_mono, sr, bpm=138.0):
    """Encontra o início do drop/groove mais nítido alinhado ao compasso"""
    beat_samples = int(sr * 60.0 / bpm)
    bar_samples = beat_samples * 4
    n_bars = len(audio_mono) // bar_samples
    
    best_bar = 0
    max_energy = 0.0
    for b in range(max(1, n_bars - 8)):
        chunk = audio_mono[b * bar_samples : (b + 1) * bar_samples]
        energy = np.mean(chunk ** 2)
        if energy > max_energy:
            max_energy = energy
            best_bar = b
            
    return best_bar * bar_samples, beat_samples, bar_samples

def build_sample_pack(stems_dir, song_title, output_base_dir=r"C:\NovaDAW\samples"):
    print(f"[SAMPLE-PACK] Gerando Kit de Samples & Loops para: '{song_title}'", flush=True)
    clean_title = song_title.replace(" ", "_").replace("-", "_")
    pack_dir = os.path.join(output_base_dir, clean_title)
    os.makedirs(pack_dir, exist_ok=True)
    
    # Mapeamento de pastas
    folders = {
        "kicks": os.path.join(pack_dir, "01_Kicks_Punches"),
        "sub": os.path.join(pack_dir, "02_Sub_Bass"),
        "bass_loops": os.path.join(pack_dir, "03_Rolling_Bass_Loops"),
        "snares": os.path.join(pack_dir, "04_Snares_Claps"),
        "hats": os.path.join(pack_dir, "05_HiHats_Cymbals"),
        "perc": os.path.join(pack_dir, "06_Percussion_Shakers"),
        "leads": os.path.join(pack_dir, "07_Psy_Leads_Arps_Loops"),
        "vocals_fx": os.path.join(pack_dir, "08_Vocals_FX_Ambience")
    }
    for f in folders.values():
        os.makedirs(f, exist_ok=True)

    # Identificar stems
    stems = {}
    for entry in os.listdir(stems_dir):
        if entry.lower().endswith(".wav"):
            full_p = os.path.join(stems_dir, entry)
            fn = entry.lower()
            if "01_psy_kick" in fn or "kick" in fn: stems["kick"] = full_p
            elif "02_sub_bass" in fn or "sub" in fn: stems["sub"] = full_p
            elif "03_rolling" in fn or "rolling_bass" in fn or "bass" in fn: stems["bass"] = full_p
            elif "04_snare" in fn or "snare" in fn or "clap" in fn: stems["snare"] = full_p
            elif "05_offbeat" in fn or "open_hat" in fn or "hat" in fn: stems["hats"] = full_p
            elif "06_closed" in fn or "perc" in fn or "shaker" in fn: stems["perc"] = full_p
            elif "07_psy_lead" in fn or "lead" in fn or "arp" in fn: stems["leads"] = full_p
            elif "08_vocal" in fn or "vox" in fn or "fx" in fn or "ambience" in fn: stems["vox_fx"] = full_p

    BPM = 138.0

    # 1. KICK SAMPLES & LOOPS
    if "kick" in stems:
        sr, k_data = load_wav_file(stems["kick"])
        k_mono = np.mean(k_data, axis=1)
        start_idx, beat_s, bar_s = find_best_energy_start(k_mono, sr, BPM)
        
        # One-shots (1 batida isolada com fade natural)
        one_shot_1 = apply_fade(k_data[start_idx : start_idx + int(sr * 0.45)])
        save_wav_file(os.path.join(folders["kicks"], f"{clean_title}_Kick_Punch_Hit_01.wav"), one_shot_1, sr)
        
        one_shot_2 = apply_fade(k_data[start_idx + beat_s : start_idx + beat_s + int(sr * 0.45)])
        save_wav_file(os.path.join(folders["kicks"], f"{clean_title}_Kick_Sub_Punch_02.wav"), one_shot_2, sr)
        
        # Loops perfeitos sincronizados (1 compasso e 2 compassos)
        loop_1bar = apply_fade(k_data[start_idx : start_idx + bar_s])
        save_wav_file(os.path.join(folders["kicks"], f"{clean_title}_Kick_Loop_138BPM_1Bar.wav"), loop_1bar, sr)
        
        loop_2bar = apply_fade(k_data[start_idx : start_idx + bar_s * 2])
        save_wav_file(os.path.join(folders["kicks"], f"{clean_title}_Kick_Loop_138BPM_2Bars.wav"), loop_2bar, sr)

    # 2. SUB BASS
    if "sub" in stems:
        sr, sub_data = load_wav_file(stems["sub"])
        sub_mono = np.mean(sub_data, axis=1)
        start_idx, beat_s, bar_s = find_best_energy_start(sub_mono, sr, BPM)
        
        sub_hit_1 = apply_fade(sub_data[start_idx : start_idx + int(sr * 0.60)])
        save_wav_file(os.path.join(folders["sub"], f"{clean_title}_Sub_Bass_Hit_C1.wav"), sub_hit_1, sr)
        
        sub_sustain = apply_fade(sub_data[start_idx : start_idx + bar_s])
        save_wav_file(os.path.join(folders["sub"], f"{clean_title}_Sub_Tone_Sustain_1Bar.wav"), sub_sustain, sr)

    # 3. ROLLING BASS LOOPS & HITS
    if "bass" in stems:
        sr, b_data = load_wav_file(stems["bass"])
        b_mono = np.mean(b_data, axis=1)
        start_idx, beat_s, bar_s = find_best_energy_start(b_mono, sr, BPM)
        
        # One-shot single 16th hit
        bass_hit = apply_fade(b_data[start_idx : start_idx + int(sr * 0.25)])
        save_wav_file(os.path.join(folders["bass_loops"], f"{clean_title}_Rolling_Bass_Stab_Hit.wav"), bass_hit, sr)
        
        # 1 Bar, 2 Bars and 4 Bars Loops
        b_loop_1 = apply_fade(b_data[start_idx : start_idx + bar_s])
        save_wav_file(os.path.join(folders["bass_loops"], f"{clean_title}_Rolling_Bass_Loop_138BPM_1Bar.wav"), b_loop_1, sr)
        
        b_loop_2 = apply_fade(b_data[start_idx : start_idx + bar_s * 2])
        save_wav_file(os.path.join(folders["bass_loops"], f"{clean_title}_Rolling_Bass_Loop_138BPM_2Bars.wav"), b_loop_2, sr)
        
        b_loop_4 = apply_fade(b_data[start_idx : start_idx + bar_s * 4])
        save_wav_file(os.path.join(folders["bass_loops"], f"{clean_title}_Rolling_Bass_Loop_138BPM_4Bars.wav"), b_loop_4, sr)

    # 4. SNARES & CLAPS
    if "snare" in stems:
        sr, sn_data = load_wav_file(stems["snare"])
        sn_mono = np.mean(sn_data, axis=1)
        start_idx, beat_s, bar_s = find_best_energy_start(sn_mono, sr, BPM)
        
        snare_hit = apply_fade(sn_data[start_idx + beat_s : start_idx + beat_s + int(sr * 0.50)])
        save_wav_file(os.path.join(folders["snares"], f"{clean_title}_Snare_Hit_01.wav"), snare_hit, sr)
        
        clap_hit = apply_fade(sn_data[start_idx + beat_s * 3 : start_idx + beat_s * 3 + int(sr * 0.50)])
        save_wav_file(os.path.join(folders["snares"], f"{clean_title}_Smash_Clap_01.wav"), clap_hit, sr)
        
        sn_loop = apply_fade(sn_data[start_idx : start_idx + bar_s * 2])
        save_wav_file(os.path.join(folders["snares"], f"{clean_title}_Snare_Clap_Loop_138BPM_2Bars.wav"), sn_loop, sr)

    # 5. HIHATS & CYMBALS
    if "hats" in stems:
        sr, h_data = load_wav_file(stems["hats"])
        h_mono = np.mean(h_data, axis=1)
        start_idx, beat_s, bar_s = find_best_energy_start(h_mono, sr, BPM)
        
        open_hat_hit = apply_fade(h_data[start_idx + int(beat_s * 0.5) : start_idx + int(beat_s * 0.5) + int(sr * 0.35)])
        save_wav_file(os.path.join(folders["hats"], f"{clean_title}_Offbeat_Open_Hat_01.wav"), open_hat_hit, sr)
        
        hat_loop = apply_fade(h_data[start_idx : start_idx + bar_s * 2])
        save_wav_file(os.path.join(folders["hats"], f"{clean_title}_Offbeat_Hats_Loop_138BPM_2Bars.wav"), hat_loop, sr)

    # 6. PERCUSSION & SHAKERS
    if "perc" in stems:
        sr, p_data = load_wav_file(stems["perc"])
        p_mono = np.mean(p_data, axis=1)
        start_idx, beat_s, bar_s = find_best_energy_start(p_mono, sr, BPM)
        
        shaker_loop = apply_fade(p_data[start_idx : start_idx + bar_s * 2])
        save_wav_file(os.path.join(folders["perc"], f"{clean_title}_Shaker_Groove_Loop_138BPM_2Bars.wav"), shaker_loop, sr)
        
        perc_hit = apply_fade(p_data[start_idx : start_idx + int(sr * 0.25)])
        save_wav_file(os.path.join(folders["perc"], f"{clean_title}_Tribal_Perc_Hit_01.wav"), perc_hit, sr)

    # 7. PSY LEADS & ARPS
    if "leads" in stems:
        sr, l_data = load_wav_file(stems["leads"])
        l_mono = np.mean(l_data, axis=1)
        start_idx, beat_s, bar_s = find_best_energy_start(l_mono, sr, BPM)
        
        # Lead Riff Hooks (2 Bars e 4 Bars)
        lead_hook_2b = apply_fade(l_data[start_idx : start_idx + bar_s * 2])
        save_wav_file(os.path.join(folders["leads"], f"{clean_title}_Psy_Lead_Hook_138BPM_2Bars.wav"), lead_hook_2b, sr)
        
        lead_hook_4b = apply_fade(l_data[start_idx : start_idx + bar_s * 4])
        save_wav_file(os.path.join(folders["leads"], f"{clean_title}_Psy_Lead_Main_Hook_138BPM_4Bars.wav"), lead_hook_4b, sr)
        
        lead_stab = apply_fade(l_data[start_idx : start_idx + int(sr * 0.50)])
        save_wav_file(os.path.join(folders["leads"], f"{clean_title}_Synth_Stab_Hit.wav"), lead_stab, sr)

    # 8. VOCALS & FX AMBIENCE
    if "vox_fx" in stems:
        sr, v_data = load_wav_file(stems["vox_fx"])
        v_mono = np.mean(v_data, axis=1)
        start_idx, beat_s, bar_s = find_best_energy_start(v_mono, sr, BPM)
        
        vocal_phrase = apply_fade(v_data[start_idx : start_idx + bar_s * 2])
        save_wav_file(os.path.join(folders["vocals_fx"], f"{clean_title}_Vocal_Phrase_Hook_2Bars.wav"), vocal_phrase, sr)
        
        laser_fx = apply_fade(v_data[start_idx + int(sr * 1.5) : start_idx + int(sr * 1.5) + int(sr * 0.60)])
        save_wav_file(os.path.join(folders["vocals_fx"], f"{clean_title}_Psy_Laser_Zap_FX.wav"), laser_fx, sr)
        
        atmosphere_pad = apply_fade(v_data[start_idx : start_idx + bar_s * 4])
        save_wav_file(os.path.join(folders["vocals_fx"], f"{clean_title}_Atmosphere_Pad_Loop_4Bars.wav"), atmosphere_pad, sr)

    print(f"[SAMPLE-PACK SUCCESS] Kit de Samples & Loops criado com sucesso em: {pack_dir}", flush=True)
    return pack_dir

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Uso: python auto_sample_pack_builder.py <stems_dir> <song_title>", flush=True)
        sys.exit(1)
        
    s_dir = sys.argv[1]
    title = sys.argv[2]
    build_sample_pack(s_dir, title)
