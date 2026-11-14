import os
import time
import numpy as np
import soundfile as sf
from scipy import signal

def surgical_slice_track(wav_path, output_base_dir=r"C:\NovaDAW\samples"):
    song_title = os.path.splitext(os.path.basename(wav_path))[0]
    clean_title = song_title.replace(" ", "_").replace("-", "_")
    pack_dir = os.path.join(output_base_dir, clean_title)
    os.makedirs(pack_dir, exist_ok=True)
    
    print(f"[SURGICAL-SLICER] Iniciando fatiamento cirúrgico de áudio puro para: '{song_title}'", flush=True)
    audio, sr = sf.read(wav_path, dtype='float32')
    if audio.ndim == 1:
        audio = np.column_stack((audio, audio))
        
    n_samples = len(audio)
    duration_sec = n_samples / sr
    print(f"[SURGICAL-SLICER] Áudio carregado: {n_samples} amostras @ {sr}Hz ({duration_sec:.2f}s)", flush=True)
    
    BPM = 138.0
    beat_samples = int(sr * 60.0 / BPM)       # ~19174 samples @ 44.1k
    bar_samples = beat_samples * 4            # ~76695 samples (1.739s)
    
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
        
    def save_sample(folder_key, filename, data, fade_ms=3.0):
        fade_s = int(sr * (fade_ms / 1000.0))
        out = np.copy(data)
        if len(out) > fade_s * 2:
            fade_in = np.linspace(0.0, 1.0, fade_s)[:, None]
            fade_out = np.linspace(1.0, 0.0, fade_s)[:, None]
            out[:fade_s] *= fade_in
            out[-fade_s:] *= fade_out
        out = np.clip(out, -1.0, 1.0)
        dest = os.path.join(folders[folder_key], filename)
        sf.write(dest, out, sr)
        print(f" -> [OK] Salvo: {filename} ({len(out)/sr:.3f}s)", flush=True)

    # 1. DETECÇÃO DE ENERGIA AO LONGO DA MÚSICA EM BLOCOS DE COMPASSO
    n_bars = n_samples // bar_samples
    bar_energies = np.zeros(n_bars)
    audio_mono = np.mean(audio, axis=1)
    
    for b in range(n_bars):
        c = audio_mono[b * bar_samples : (b + 1) * bar_samples]
        bar_energies[b] = np.sqrt(np.mean(c ** 2)) # RMS
        
    # Encontrar:
    # A) Intro (Kick isolado / subido inicial) -> primeiro trecho com batida
    # B) Main Drop (Pico máximo de energia contínua)
    # C) Breakdown / Pausa (Vale de energia no meio da música com melodias/synths puros)
    
    # ── A. KICKS CIRÚRGICOS ONE-SHOT ──────────────────────────────────────────
    # Procura um transiente forte de Kick nos primeiros 40 segundos
    intro_search_len = min(n_samples, int(sr * 45.0))
    # Filtro passa-baixa leve para detectar o ataque do bumbo
    sos_kick_det = signal.butter(2, 120.0 / (0.5 * sr), btype='low', output='sos')
    low_env = np.abs(signal.sosfilt(sos_kick_det, audio_mono[:intro_search_len]))
    
    # Picos de kick
    peaks, _ = signal.find_peaks(low_env, distance=int(beat_samples * 0.85), prominence=0.15)
    
    if len(peaks) > 0:
        for k_idx, p_idx in enumerate(peaks[:4]):
            # Recuo de 10ms antes do pico para pegar o click inicial
            start_s = max(0, p_idx - int(sr * 0.005))
            # Ajuste fino para o zero-crossing mais próximo antes do transiente
            while start_s > 0 and np.sign(audio_mono[start_s]) == np.sign(audio_mono[start_s - 1]):
                start_s -= 1
            
            # Kick completo de 420ms (ataque + corpo + decaimento sub)
            kick_len = int(sr * 0.42)
            if start_s + kick_len < n_samples:
                k_data = audio[start_s : start_s + kick_len]
                save_sample("kicks", f"{clean_title}_Psy_Kick_Master_Punch_{k_idx+1:02d}.wav", k_data, fade_ms=5.0)

    # ── B. ROLLING BASS & GROOVE LOOPS (DROP PRINCIPAL) ───────────────────────
    # Localiza o Drop com maior energia contínua
    drop_bar = np.argmax(bar_energies)
    # Alinha ao compasso
    drop_start = drop_bar * bar_samples
    
    # 1 Bar Loop (4 Beats)
    loop_1bar = audio[drop_start : drop_start + bar_samples]
    save_sample("bass_loops", f"{clean_title}_Rolling_Groove_Loop_138BPM_1Bar.wav", loop_1bar, fade_ms=2.0)
    
    # 2 Bars Loop (8 Beats)
    loop_2bars = audio[drop_start : drop_start + bar_samples * 2]
    save_sample("bass_loops", f"{clean_title}_Rolling_Groove_Loop_138BPM_2Bars.wav", loop_2bars, fade_ms=2.0)
    
    # 4 Bars Loop (16 Beats)
    loop_4bars = audio[drop_start : drop_start + bar_samples * 4]
    save_sample("bass_loops", f"{clean_title}_Rolling_Groove_Loop_138BPM_4Bars.wav", loop_4bars, fade_ms=2.0)

    # Rolling Bass Isolated (Zero-phase highpass 95Hz para tirar o kick e focar nos 16ths do bass)
    sos_bass_iso = signal.butter(4, 95.0 / (0.5 * sr), btype='high', output='sos')
    bass_only_loop = signal.sosfiltfilt(sos_bass_iso, loop_2bars, axis=0) * 1.25
    save_sample("bass_loops", f"{clean_title}_Pure_Rolling_Bass_Loop_138BPM_2Bars.wav", bass_only_loop, fade_ms=2.0)

    # ── C. BREAKDOWN / SYNTH LEADS & ARPS (MELODIAS PURAS SEM BATERIA) ───────
    # O breakdown fica entre 35% e 70% da música onde a energia é mais suave
    mid_start_bar = int(n_bars * 0.35)
    mid_end_bar = int(n_bars * 0.70)
    if mid_end_bar > mid_start_bar:
        breakdown_bar = mid_start_bar + np.argmin(bar_energies[mid_start_bar:mid_end_bar])
        bd_start = breakdown_bar * bar_samples
        
        # 4 Bars e 8 Bars de Melodia Pura
        lead_4b = audio[bd_start : bd_start + bar_samples * 4]
        save_sample("leads", f"{clean_title}_Psy_Lead_Melody_Hook_138BPM_4Bars.wav", lead_4b, fade_ms=4.0)
        
        if bd_start + bar_samples * 8 < n_samples:
            lead_8b = audio[bd_start : bd_start + bar_samples * 8]
            save_sample("leads", f"{clean_title}_Psy_Lead_Theme_Progression_138BPM_8Bars.wav", lead_8b, fade_ms=4.0)

    # ── D. BUILD-UP / VOCALS, RISERS & LASER SFX ──────────────────────────────
    # A transição / build-up fica 4 compassos antes do drop
    bu_start = max(0, drop_start - bar_samples * 4)
    bu_data = audio[bu_start : drop_start]
    save_sample("vocals_fx", f"{clean_title}_Cosmic_Riser_BuildUp_138BPM_4Bars.wav", bu_data, fade_ms=3.0)

    # SFX Laser / Zap (Extraído da região de alta frequência)
    sos_air = signal.butter(4, 3000.0 / (0.5 * sr), btype='high', output='sos')
    sfx_zap = signal.sosfiltfilt(sos_air, bu_data[:int(sr * 0.8)], axis=0) * 1.4
    save_sample("vocals_fx", f"{clean_title}_Psy_Laser_Zap_FX.wav", sfx_zap, fade_ms=5.0)

    # ── E. SNARES, CLAPS & OPEN HATS ──────────────────────────────────────────
    # Pega o clap no 2º beat do groove do drop
    clap_pos = drop_start + beat_samples
    clap_sample = audio[clap_pos : clap_pos + int(sr * 0.45)]
    save_sample("snares", f"{clean_title}_Psy_Smash_Clap_Hit_01.wav", clap_sample, fade_ms=3.0)
    
    # Open Hat no offbeat (meio tempo entre beat 1 e 2)
    hat_pos = drop_start + int(beat_samples * 0.5)
    hat_sample = audio[hat_pos : hat_pos + int(sr * 0.30)]
    save_sample("hats", f"{clean_title}_Psy_Offbeat_Open_Hat_01.wav", hat_sample, fade_ms=3.0)

    print(f"[SURGICAL-SLICER SUCCESS] Pack de Samples e Loops Cirúrgicos gerado em: {pack_dir}", flush=True)
    return pack_dir

if __name__ == "__main__":
    t1 = r"C:\NovaDAW\tracks\Perception - Different Way.wav"
    t2 = r"C:\NovaDAW\tracks\Perception - Life Process.wav"
    
    if os.path.exists(t1):
        surgical_slice_track(t1)
    if os.path.exists(t2):
        surgical_slice_track(t2)
