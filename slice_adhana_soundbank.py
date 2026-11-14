import os
import wave
import numpy as np

def load_wav_stereo(path):
    with wave.open(path, 'rb') as wf:
        n_ch = wf.getnchannels()
        sr = wf.getframerate()
        n_frames = wf.getnframes()
        sampwidth = wf.getsampwidth()
        raw = wf.readframes(n_frames)
    
    if sampwidth == 2:
        data = np.frombuffer(raw, dtype=np.int16).astype(np.float32) / 32768.0
    else:
        data = np.frombuffer(raw, dtype=np.int32).astype(np.float32) / 2147483648.0

    if n_ch == 2:
        data = data.reshape(-1, 2)
    else:
        data = np.column_stack((data, data))
        
    return sr, data

def save_wav_slice(path, slice_data, sr, fade_in_ms=2.0, fade_out_ms=8.0):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    out = slice_data.copy()
    
    # Suavização nas bordas para evitar cliques (Fade In & Fade Out)
    fade_in_len = int(fade_in_ms * sr / 1000.0)
    fade_out_len = int(fade_out_ms * sr / 1000.0)
    
    if fade_in_len > 0 and len(out) > fade_in_len:
        ramp_in = np.linspace(0, 1, fade_in_len)[:, np.newaxis]
        out[:fade_in_len] *= ramp_in
        
    if fade_out_len > 0 and len(out) > fade_out_len:
        ramp_out = np.linspace(1, 0, fade_out_len)[:, np.newaxis]
        out[-fade_out_len:] *= ramp_out
        
    # Normalização de volume (-0.5 dB headroom)
    max_val = np.max(np.abs(out))
    if max_val > 0.01:
        out = (out / max_val) * 0.94
        
    int_data = (np.clip(out, -1.0, 1.0) * 32767.0).astype(np.int16)
    with wave.open(path, 'wb') as wf:
        wf.setnchannels(2)
        wf.setsampwidth(2)
        wf.setframerate(sr)
        wf.writeframes(int_data.tobytes())
    print(f"[SLICER] Gerado: {os.path.basename(path)} ({len(out)/sr:.3f}s)")

def find_transient_onset(mono_data, start_search, window_search, sr, threshold=0.15):
    """
    Localiza o início exato do transiente por busca de subida súbita de energia e zero-crossing.
    """
    search_region = mono_data[start_search : start_search + window_search]
    diff = np.diff(search_region)
    peak_idx = np.argmax(np.abs(search_region) > threshold)
    if peak_idx == 0 and not (np.abs(search_region[0]) > threshold):
        peak_idx = np.argmax(np.abs(search_region))
        
    abs_peak = start_search + peak_idx
    # Recua até o zero-crossing mais próximo antes do pico
    zc = abs_peak
    while zc > start_search and mono_data[zc] * mono_data[zc-1] > 0:
        zc -= 1
    return zc

def slice_adhana_stems():
    stems_dir = r"C:\Users\USUÁRIO\Music\Musicas Recortadas\Vini_Vici_Astrix_Adhana"
    output_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets", "samples", "adhana_signature")
    os.makedirs(output_dir, exist_ok=True)
    
    kick_file = os.path.join(stems_dir, "Vini_Vici_Astrix_Adhana_Kick_Sub.wav")
    bass_file = os.path.join(stems_dir, "Vini_Vici_Astrix_Adhana_Rolling_Bass.wav")
    lead_file = os.path.join(stems_dir, "Vini_Vici_Astrix_Adhana_Psy_Leads_Arps.wav")
    vox_file = os.path.join(stems_dir, "Vini_Vici_Astrix_Adhana_Vocals_SFX_Ambience.wav")
    
    if not os.path.exists(kick_file):
        print(f"[SLICER ERROR] Pasta de stems de Adhana não encontrada em '{stems_dir}'")
        return False
        
    print(f"[SLICER] Extraindo Sound Bank Signature da track 'Adhana' (Vini Vici & Astrix)...")
    
    # 1. FATIAMENTO DE KICK
    sr, kick_audio = load_wav_stereo(kick_file)
    mono_kick = (kick_audio[:, 0] + kick_audio[:, 1]) * 0.5
    
    # Drop principal de Adhana (~1:30 a 2:00)
    drop_sample = int(95.0 * sr) # 95s
    onset_k1 = find_transient_onset(mono_kick, drop_sample, int(2.0 * sr), sr, threshold=0.3)
    
    # 1.1 One-shot Kick Punch (220ms)
    kick_len = int(0.22 * sr)
    save_wav_slice(os.path.join(output_dir, "Adhana_Astrix_Kick_Punch_01.wav"), kick_audio[onset_k1 : onset_k1 + kick_len], sr)
    
    # 1.2 One-shot Kick Sub (320ms)
    kick_len_sub = int(0.32 * sr)
    save_wav_slice(os.path.join(output_dir, "Adhana_Astrix_Kick_Sub_02.wav"), kick_audio[onset_k1 : onset_k1 + kick_len_sub], sr)
    
    # 1.3 Kick Loop 4-on-the-floor (1 bar @ 138 BPM = 1.739s)
    loop_1bar = int((60.0 / 138.0) * 4.0 * sr)
    save_wav_slice(os.path.join(output_dir, "Adhana_Kick_Loop_138BPM.wav"), kick_audio[onset_k1 : onset_k1 + loop_1bar], sr)
    
    # 2. FATIAMENTO DE ROLLING BASS
    sr, bass_audio = load_wav_stereo(bass_file)
    mono_bass = (bass_audio[:, 0] + bass_audio[:, 1]) * 0.5
    onset_b1 = find_transient_onset(mono_bass, drop_sample, int(2.0 * sr), sr, threshold=0.25)
    
    # 2.1 One-shot Rolling Bass Hit 16th (~108ms)
    b_16th_len = int((60.0 / 138.0 / 4.0) * sr)
    save_wav_slice(os.path.join(output_dir, "Adhana_Rolling_Bass_Hit_01.wav"), bass_audio[onset_b1 : onset_b1 + b_16th_len], sr)
    
    # 2.2 Rolling Bassline Loop (1 bar @ 138 BPM)
    save_wav_slice(os.path.join(output_dir, "Adhana_Rolling_Bassline_Loop_138BPM.wav"), bass_audio[onset_b1 : onset_b1 + loop_1bar], sr)
    
    # 2.3 Acid Bass Stab (segundo drop ~2:45)
    drop2_sample = int(165.0 * sr)
    onset_b2 = find_transient_onset(mono_bass, drop2_sample, int(5.0 * sr), sr, threshold=0.20)
    save_wav_slice(os.path.join(output_dir, "Adhana_Acid_Bass_Stab_01.wav"), bass_audio[onset_b2 : onset_b2 + int(0.35 * sr)], sr)

    # 3. FATIAMENTO DE PSY LEADS & ARPS
    sr, lead_audio = load_wav_stereo(lead_file)
    mono_lead = (lead_audio[:, 0] + lead_audio[:, 1]) * 0.5
    
    # Hook melódico no clímax (~3:10)
    hook_sample = int(190.0 * sr)
    onset_l1 = find_transient_onset(mono_lead, hook_sample, int(5.0 * sr), sr, threshold=0.20)
    
    # 3.1 Hook melódico 2 compassos (~3.48s)
    lead_2bar = int((60.0 / 138.0) * 8.0 * sr)
    save_wav_slice(os.path.join(output_dir, "Adhana_Psy_Lead_Hook_2Bars.wav"), lead_audio[onset_l1 : onset_l1 + lead_2bar], sr)
    
    # 3.2 Lead One-Shot Stab
    save_wav_slice(os.path.join(output_dir, "Adhana_Lead_Stab_Hit.wav"), lead_audio[onset_l1 : onset_l1 + int(0.40 * sr)], sr)
    
    # 3.3 Acid Arpeggio 1 Bar
    arp_sample = int(135.0 * sr)
    onset_l2 = find_transient_onset(mono_lead, arp_sample, int(5.0 * sr), sr, threshold=0.15)
    save_wav_slice(os.path.join(output_dir, "Adhana_Acid_Arp_Loop_138BPM.wav"), lead_audio[onset_l2 : onset_l2 + loop_1bar], sr)

    # 4. FATIAMENTO DE VOCALS & SFX (Cânticos étnicos e mantras de Adhana)
    sr, vox_audio = load_wav_stereo(vox_file)
    mono_vox = (vox_audio[:, 0] + vox_audio[:, 1]) * 0.5
    
    # Vocal Breakdown / Mantra principal (~1:10 e ~2:20)
    vox_intro = int(68.0 * sr)
    onset_v1 = find_transient_onset(mono_vox, vox_intro, int(8.0 * sr), sr, threshold=0.10)
    
    # 4.1 Cântico Étnico "Adhana Mantra" Frase 1 (~2.8s)
    save_wav_slice(os.path.join(output_dir, "Adhana_Vocal_Mantra_Chant_01.wav"), vox_audio[onset_v1 : onset_v1 + int(2.8 * sr)], sr)
    
    # 4.2 Cântico Étnico "Adhana Mantra" Frase 2 (~3.2s)
    vox_drop = int(140.0 * sr)
    onset_v2 = find_transient_onset(mono_vox, vox_drop, int(8.0 * sr), sr, threshold=0.12)
    save_wav_slice(os.path.join(output_dir, "Adhana_Vocal_Mantra_Chant_02.wav"), vox_audio[onset_v2 : onset_v2 + int(3.2 * sr)], sr)
    
    # 4.3 Tribal Vocal Chop Curto (~400ms)
    save_wav_slice(os.path.join(output_dir, "Adhana_Tribal_Vocal_Chop.wav"), vox_audio[onset_v1 : onset_v1 + int(0.45 * sr)], sr)
    
    # 4.4 Psy Laser Zap FX
    zap_sample = int(160.0 * sr)
    onset_z = find_transient_onset(mono_vox, zap_sample, int(6.0 * sr), sr, threshold=0.15)
    save_wav_slice(os.path.join(output_dir, "Adhana_Psy_Laser_Zap_FX.wav"), vox_audio[onset_z : onset_z + int(0.80 * sr)], sr)
    
    # 4.5 Cosmic Riser FX
    riser_sample = int(90.0 * sr)
    onset_r = find_transient_onset(mono_vox, riser_sample, int(4.0 * sr), sr, threshold=0.08)
    save_wav_slice(os.path.join(output_dir, "Adhana_Cosmic_Riser_FX.wav"), vox_audio[onset_r : onset_r + int(2.0 * sr)], sr)

    print(f"\n[SLICER SUCCESS] Sound Bank 'Adhana Signature Kit' (14 samples reais) gerado com sucesso em '{output_dir}'!")
    return True

if __name__ == "__main__":
    slice_adhana_stems()
