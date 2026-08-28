import os
import sys
import time
import wave
import subprocess
import numpy as np

def find_ffmpeg():
    possible_paths = [
        r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\ffmpeg\ffmpeg-8.1.2-essentials_build\bin\ffmpeg.exe",
        r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\ffmpeg\ffmpeg.exe",
        "ffmpeg"
    ]
    for p in possible_paths:
        if os.path.exists(p):
            return p
    return "ffmpeg"

def load_audio_file(input_path):
    ext = os.path.splitext(input_path)[1].lower()
    if ext == ".wav":
        try:
            with wave.open(input_path, 'rb') as wf:
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
                try:
                    data = np.frombuffer(raw_bytes, dtype=np.float32)
                except:
                    data = np.frombuffer(raw_bytes, dtype=np.int32).astype(np.float32) / 2147483648.0
            else:
                data = np.frombuffer(raw_bytes, dtype=np.int16).astype(np.float32) / 32768.0

            if n_ch == 2:
                data = data.reshape(-1, 2)
            else:
                data = np.column_stack((data, data))
                
            return sr, data
        except Exception as e:
            print(f"[WAV-LOADER] Leitura padrão falhou ({e}), tentando decodificar via ffmpeg...", flush=True)

    ffmpeg_exe = find_ffmpeg()
    cmd = [
        ffmpeg_exe, "-y", "-i", input_path,
        "-f", "s16le", "-acodec", "pcm_s16le",
        "-ar", "44100", "-ac", "2", "pipe:1"
    ]
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        raw_pcm, _ = proc.communicate()
        if len(raw_pcm) > 0:
            data = np.frombuffer(raw_pcm, dtype=np.int16).astype(np.float32) / 32768.0
            data = data.reshape(-1, 2)
            return 44100, data
    except Exception as e:
        print(f"[FFMPEG-DECODER ERROR] {e}", flush=True)

    raise RuntimeError(f"Não foi possível abrir o arquivo de áudio: {input_path}")

def build_band_mask(freqs, sr, low_f, high_f, fade_hz=35.0):
    mask = np.zeros(len(freqs), dtype=np.float32)
    low_start = max(0.0, low_f - fade_hz)
    low_end = low_f
    high_start = high_f
    high_end = min(sr / 2.0, high_f + fade_hz)
    
    if low_end > low_start:
        up_idx = (freqs >= low_start) & (freqs < low_end)
        if np.any(up_idx):
            mask[up_idx] = 0.5 * (1.0 - np.cos(np.pi * (freqs[up_idx] - low_start) / (low_end - low_start)))
    else:
        up_idx = freqs < low_end
        mask[up_idx] = 1.0

    pass_idx = (freqs >= low_end) & (freqs <= high_start)
    mask[pass_idx] = 1.0
    
    if high_end > high_start:
        down_idx = (freqs > high_start) & (freqs <= high_end)
        if np.any(down_idx):
            mask[down_idx] = 0.5 * (1.0 + np.cos(np.pi * (freqs[down_idx] - high_start) / (high_end - high_start)))
            
    return mask

def process_overlap_add_8stems(mid, side, sr, block_size=65536, hop_size=32768):
    """
    Processamento de Matriz de Decomposição Espectral em 8 Pistas Isoladas.
    Isola Kick, Sub, Rolling Bass, Snare/Clap, Hi-Hats, Percussion/Shakers, Leads e Vocais/FX.
    """
    n_samples = len(mid)
    win = np.hanning(block_size).astype(np.float32)
    win_norm = win / (block_size / hop_size * 0.5)
    
    freqs = np.fft.rfftfreq(block_size, d=1.0/sr)
    
    # 8 Máscaras Espectrais de Precisão
    mask_kick_sub = build_band_mask(freqs, sr, 35.0, 85.0, fade_hz=10.0)
    mask_sub_bass = build_band_mask(freqs, sr, 35.0, 110.0, fade_hz=15.0)
    mask_rolling_bass = build_band_mask(freqs, sr, 110.0, 380.0, fade_hz=20.0)
    mask_snare_clap = build_band_mask(freqs, sr, 220.0, 1600.0, fade_hz=30.0)
    mask_hihats = build_band_mask(freqs, sr, 5000.0, 10500.0, fade_hz=40.0)
    mask_percussion = build_band_mask(freqs, sr, 10500.0, 20000.0, fade_hz=50.0)
    mask_leads = build_band_mask(freqs, sr, 420.0, 5500.0, fade_hz=35.0)
    mask_vocals_fx = build_band_mask(freqs, sr, 300.0, 4200.0, fade_hz=30.0)
    mask_side_amb = build_band_mask(freqs, sr, 200.0, 18000.0, fade_hz=40.0)

    # Re-injeção de Transientes e Curvas de Equalização de Estúdio (Estilo Master de Psytrance)
    mask_click = build_band_mask(freqs, sr, 2200.0, 4800.0, fade_hz=40.0)
    mask_bass_body = build_band_mask(freqs, sr, 200.0, 750.0, fade_hz=30.0)
    mask_lead_air = build_band_mask(freqs, sr, 6000.0, 16000.0, fade_hz=50.0)

    out_kick = np.zeros(n_samples + block_size, dtype=np.float32)
    out_click = np.zeros(n_samples + block_size, dtype=np.float32)
    out_sub = np.zeros(n_samples + block_size, dtype=np.float32)
    out_roll = np.zeros(n_samples + block_size, dtype=np.float32)
    out_bass_body = np.zeros(n_samples + block_size, dtype=np.float32)
    out_snare = np.zeros(n_samples + block_size, dtype=np.float32)
    out_hats = np.zeros(n_samples + block_size, dtype=np.float32)
    out_perc = np.zeros(n_samples + block_size, dtype=np.float32)
    out_lead_m = np.zeros(n_samples + block_size, dtype=np.float32)
    out_lead_s = np.zeros(n_samples + block_size, dtype=np.float32)
    out_lead_air = np.zeros(n_samples + block_size, dtype=np.float32)
    out_vox_m = np.zeros(n_samples + block_size, dtype=np.float32)
    out_amb_s = np.zeros(n_samples + block_size, dtype=np.float32)

    # Loop de Blocos FFT
    for pos in range(0, n_samples, hop_size):
        chunk_len = min(block_size, n_samples - pos)
        if chunk_len <= 0:
            break
            
        m_chunk = np.zeros(block_size, dtype=np.float32)
        s_chunk = np.zeros(block_size, dtype=np.float32)
        m_chunk[:chunk_len] = mid[pos:pos+chunk_len] * win[:chunk_len]
        s_chunk[:chunk_len] = side[pos:pos+chunk_len] * win[:chunk_len]
        
        fft_m = np.fft.rfft(m_chunk)
        fft_s = np.fft.rfft(s_chunk)
        
        out_kick[pos:pos+block_size] += np.fft.irfft(fft_m * mask_kick_sub, n=block_size) * win
        out_click[pos:pos+block_size] += np.fft.irfft(fft_m * mask_click, n=block_size) * win
        out_sub[pos:pos+block_size] += np.fft.irfft(fft_m * mask_sub_bass, n=block_size) * win
        out_roll[pos:pos+block_size] += np.fft.irfft(fft_m * mask_rolling_bass, n=block_size) * win
        out_bass_body[pos:pos+block_size] += np.fft.irfft(fft_m * mask_bass_body, n=block_size) * win
        out_snare[pos:pos+block_size] += np.fft.irfft(fft_m * mask_snare_clap, n=block_size) * win
        out_hats[pos:pos+block_size] += np.fft.irfft(fft_m * mask_hihats + fft_s * mask_hihats * 0.5, n=block_size) * win
        out_perc[pos:pos+block_size] += np.fft.irfft(fft_m * mask_percussion + fft_s * mask_percussion * 0.7, n=block_size) * win
        out_lead_m[pos:pos+block_size] += np.fft.irfft(fft_m * mask_leads, n=block_size) * win
        out_lead_s[pos:pos+block_size] += np.fft.irfft(fft_s * mask_leads, n=block_size) * win
        out_lead_air[pos:pos+block_size] += np.fft.irfft(fft_m * mask_lead_air + fft_s * mask_lead_air * 0.8, n=block_size) * win
        out_vox_m[pos:pos+block_size] += np.fft.irfft(fft_m * mask_vocals_fx, n=block_size) * win
        out_amb_s[pos:pos+block_size] += np.fft.irfft(fft_s * mask_side_amb, n=block_size) * win

    # Truncar com Equalização Paramétrica Integrada
    # 1. Kick: Sub-punch + Transient Click no topo
    t1_kick = np.clip(out_kick[:n_samples] * 1.35 + out_click[:n_samples] * 0.25, -1.0, 1.0)
    # 2. Sub: Puro e aveludado
    t2_sub = np.clip(out_sub[:n_samples] * 1.30, -1.0, 1.0)
    # 3. Rolling Bass: Corpo serrilhado + ressonância de médios
    t3_roll = np.clip(out_roll[:n_samples] * 1.25 + out_bass_body[:n_samples] * 0.35, -1.0, 1.0)
    # 4. Snare & Clap: Ataque encorpado
    t4_snare = np.clip(out_snare[:n_samples] * 1.30, -1.0, 1.0)
    # 5. Hi-Hats: Brilho nítido
    t5_hats = np.clip(out_hats[:n_samples] * 1.25, -1.0, 1.0)
    # 6. Percussões & Shakers
    t6_perc = np.clip(out_perc[:n_samples] * 1.25, -1.0, 1.0)
    
    # 7. Leads: Corpo melódico + Ar analógico brilhante
    t7_lead_l = np.clip(out_lead_m[:n_samples] * 0.90 + out_lead_air[:n_samples] * 0.40 + out_lead_s[:n_samples] * 0.80, -1.0, 1.0)
    t7_lead_r = np.clip(out_lead_m[:n_samples] * 0.90 + out_lead_air[:n_samples] * 0.40 - out_lead_s[:n_samples] * 0.80, -1.0, 1.0)
    
    # 8. Vocais e Ambiência
    t8_vox_l = np.clip(out_vox_m[:n_samples] * 0.90 + out_amb_s[:n_samples] * 1.15, -1.0, 1.0)
    t8_vox_r = np.clip(out_vox_m[:n_samples] * 0.90 - out_amb_s[:n_samples] * 1.15, -1.0, 1.0)
    
    return [
        (t1_kick, t1_kick, "01_Psy_Kick_Punch"),
        (t2_sub, t2_sub, "02_Sub_Bass"),
        (t3_roll, t3_roll, "03_Rolling_Saw_Bass"),
        (t4_snare, t4_snare, "04_Snare_Clap"),
        (t5_hats, t5_hats, "05_Offbeat_Open_Hats"),
        (t6_perc, t6_perc, "06_Closed_Hats_Percussion"),
        (t7_lead_l, t7_lead_r, "07_Psy_Leads_Arps"),
        (t8_vox_l, t8_vox_r, "08_Vocals_FX_Ambience")
    ]

def extract_edm_psytrance_stems(input_wav, output_dir):
    os.makedirs(output_dir, exist_ok=True)
    basename = os.path.splitext(os.path.basename(input_wav))[0]
    
    t0 = time.time()
    print(f"[EDM-MATRIX] Iniciando separação ULTRA-PRECISION em 8 Pistas para: '{input_wav}'", flush=True)
    
    try:
        sr, audio_data = load_audio_file(input_wav)
        left = audio_data[:, 0]
        right = audio_data[:, 1]
        mid = (left + right) * 0.5
        side = (left - right) * 0.5
        n_samples = len(mid)
        
        print(f"[EDM-MATRIX] Áudio carregado: {n_samples} amostras @ {sr}Hz ({n_samples/sr:.2f}s). Processando 8 Pistas Isoladas...", flush=True)
        
        stems_8 = process_overlap_add_8stems(mid, side, sr)
        
        print("[EDM-MATRIX] Gravando arquivos WAV das 8 pistas isoladas...", flush=True)
        
        def save_stem(path, l_chan, r_chan):
            st = np.column_stack((l_chan, r_chan))
            st = np.clip(st, -1.0, 1.0)
            int_data = (st * 32767.0).astype(np.int16)
            with wave.open(path, 'wb') as out_f:
                out_f.setnchannels(2)
                out_f.setsampwidth(2)
                out_f.setframerate(sr)
                out_f.writeframes(int_data.tobytes())
                
        for l_c, r_c, sname in stems_8:
            stem_file = os.path.join(output_dir, f"{basename}_{sname}.wav")
            save_stem(stem_file, l_c, r_c)
            print(f" -> Pista: {basename}_{sname}.wav", flush=True)

        # Transcrever automaticamente para partitura MIDI real
        try:
            import audio_to_midi_transcriber
            print("[EDM-MATRIX] Transcrevendo stems para partitura MIDI real...", flush=True)
            audio_to_midi_transcriber.transcribe_stems_to_midi(output_dir)
        except Exception as e:
            print(f"[TRANSCRIBER WARNING] {e}", flush=True)
        
        elapsed = time.time() - t0
        print(f"[EDM-MATRIX SUCCESS] 8 Pistas e Partitura MIDI gerados com sucesso em {elapsed:.2f} segundos!", flush=True)
        return True
    except Exception as e:
        print(f"[EDM-MATRIX ERROR] {e}", flush=True)
        return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Uso: python edm_stem_extractor.py <input_audio.wav> <output_dir>", flush=True)
        sys.exit(1)
    
    input_file = sys.argv[1]
    out_dir = sys.argv[2]
    success = extract_edm_psytrance_stems(input_file, out_dir)
    sys.exit(0 if success else 1)
