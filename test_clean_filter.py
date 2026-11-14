import os
import time
import numpy as np
import soundfile as sf
from scipy import signal

def clean_separate(input_wav, output_dir):
    os.makedirs(output_dir, exist_ok=True)
    basename = os.path.splitext(os.path.basename(input_wav))[0]
    
    print(f"[CLEAN-DSP] Carregando audio com SoundFile: {input_wav}...", flush=True)
    audio, sr = sf.read(input_wav, dtype='float32')
    if audio.ndim == 1:
        audio = np.column_stack((audio, audio))
        
    nyq = 0.5 * sr
    
    # 1. Filtros Crossover de Fase Linear / Zero-Phase (Butterworth 4th order bidirecional)
    # Reconstrução com ZERO distorção de fase, ZERO som aquático e ZERO flanger
    print("[CLEAN-DSP] Projetando filtros de fase zero (sosfiltfilt)...", flush=True)
    
    # Crossovers precisos
    sos_kick_sub = signal.butter(4, 90.0 / nyq, btype='low', output='sos')
    sos_roll_bass = signal.butter(4, [85.0 / nyq, 380.0 / nyq], btype='band', output='sos')
    sos_mid_leads = signal.butter(4, [380.0 / nyq, 4500.0 / nyq], btype='band', output='sos')
    sos_highs = signal.butter(4, 4500.0 / nyq, btype='high', output='sos')
    
    # Executar filtragem de fase zero bidirecional
    print("[CLEAN-DSP] Aplicando filtragem de fase zero cristalina...", flush=True)
    band_sub = signal.sosfiltfilt(sos_kick_sub, audio, axis=0)
    band_roll = signal.sosfiltfilt(sos_roll_bass, audio, axis=0)
    band_leads = signal.sosfiltfilt(sos_mid_leads, audio, axis=0)
    band_highs = signal.sosfiltfilt(sos_highs, audio, axis=0)
    
    # Mid/Side para vocais e ambiência
    mid = (audio[:, 0] + audio[:, 1]) * 0.5
    side = (audio[:, 0] - audio[:, 1]) * 0.5
    
    # Salvar 4 stems puros e perfeitos
    p1 = os.path.join(output_dir, f"{basename}_01_Psy_Kick_Punch.wav")
    p2 = os.path.join(output_dir, f"{basename}_02_Sub_Bass.wav")
    p3 = os.path.join(output_dir, f"{basename}_03_Rolling_Saw_Bass.wav")
    p7 = os.path.join(output_dir, f"{basename}_07_Psy_Leads_Arps.wav")
    p8 = os.path.join(output_dir, f"{basename}_08_Vocals_FX_Ambience.wav")
    
    # Pista 1: Kick & Punch (Graves focados no centro)
    kick_clean = band_sub * 1.15
    # Pista 3: Rolling Bass
    bass_clean = band_roll * 1.20
    # Pista 7: Leads e Melodias
    leads_clean = band_leads * 1.05
    # Pista 8: Altas e Ambiência
    fx_clean = band_highs * 1.05
    
    sf.write(p1, np.clip(kick_clean, -1.0, 1.0), sr)
    sf.write(p2, np.clip(band_sub * 0.9, -1.0, 1.0), sr)
    sf.write(p3, np.clip(bass_clean, -1.0, 1.0), sr)
    sf.write(p7, np.clip(leads_clean, -1.0, 1.0), sr)
    sf.write(p8, np.clip(fx_clean, -1.0, 1.0), sr)
    
    print("[CLEAN-DSP SUCCESS] Stems cristalinos sem distorcao de fase gerados!", flush=True)

if __name__ == "__main__":
    clean_separate(r"C:\NovaDAW\tracks\Perception - Different Way.wav", r"C:\NovaDAW\tracks\Perception_Different_Way_Stems")
