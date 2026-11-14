import os
import math
import wave
import numpy as np

def create_soundbank():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir = os.path.join(script_dir, "assets", "samples")
    os.makedirs(output_dir, exist_ok=True)
    
    sr = 44100
    
    def save_wav(filename, data_l, data_r=None):
        path = os.path.join(output_dir, filename)
        if data_r is None:
            data_r = data_l
        data_l = np.clip(data_l, -1.0, 1.0)
        data_r = np.clip(data_r, -1.0, 1.0)
        st = np.column_stack((data_l, data_r))
        int_data = (st * 32767.0).astype(np.int16)
        with wave.open(path, 'wb') as wf:
            wf.setnchannels(2)
            wf.setsampwidth(2)
            wf.setframerate(sr)
            wf.writeframes(int_data.tobytes())
        print(f"[SOUND-BANK] Gerado: {filename} ({len(data_l)/sr:.3f}s)")

    # =========================================================================
    # 1. KICKS
    # =========================================================================
    # 1.1 Psytrance Kick 140BPM (Punch + Sub tail)
    dur = 0.22 # ~220ms
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    # Pitch envelope: 260Hz -> 52Hz
    freq_env = 52.0 + 210.0 * np.exp(-t / 0.025)
    phase = 2.0 * np.pi * np.cumsum(freq_env) / sr
    amp_env = np.exp(-t / 0.08)
    # Transient click
    click = np.random.uniform(-1.0, 1.0, len(t)) * np.exp(-t / 0.003) * 0.4
    psy_kick = (np.sin(phase) + click) * amp_env
    psy_kick = np.tanh(psy_kick * 1.2)
    save_wav("Psytrance_Kick_140BPM.wav", psy_kick)

    # 1.2 Hardstyle Sub Kick
    dur = 0.35
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    freq_env = 45.0 + 350.0 * np.exp(-t / 0.018)
    phase = 2.0 * np.pi * np.cumsum(freq_env) / sr
    amp_env = np.exp(-t / 0.12)
    hard_kick = np.tanh(np.sin(phase) * 2.5) * amp_env * 0.9
    save_wav("Hardstyle_Sub_Kick.wav", hard_kick)

    # 1.3 Clean 808 Sub Kick
    dur = 0.60
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    freq_env = 42.0 + 120.0 * np.exp(-t / 0.015)
    phase = 2.0 * np.pi * np.cumsum(freq_env) / sr
    amp_env = np.exp(-t / 0.22)
    clean_808 = np.sin(phase) * amp_env
    save_wav("Clean_808_Sub_Kick.wav", clean_808)

    # 1.4 Slap Punch Kick
    dur = 0.18
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    freq_env = 65.0 + 180.0 * np.exp(-t / 0.010)
    phase = 2.0 * np.pi * np.cumsum(freq_env) / sr
    amp_env = np.exp(-t / 0.05)
    slap_kick = (np.sin(phase) + np.sin(phase * 2.0) * 0.3) * amp_env
    save_wav("Slap_Punch_Kick.wav", slap_kick)

    # =========================================================================
    # 2. BASSLINES
    # =========================================================================
    # 2.1 KBB Rolling Psy Bass 16th (140BPM note)
    dur = 0.107 # 1/16th at 140BPM
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    f_bass = 65.41 # C2
    saw = 2.0 * (t * f_bass - np.floor(0.5 + t * f_bass))
    env_cutoff = np.exp(-t / 0.035)
    # Simple lowpass filter simulation
    b_rolling = saw * (0.3 + 0.7 * env_cutoff) * np.exp(-t / 0.07)
    save_wav("Rolling_Psy_Bass_16th.wav", b_rolling)

    # 2.2 Acid 303 Saw Hit
    dur = 0.25
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    f_303 = 110.0 # A2
    saw_303 = 2.0 * (t * f_303 - np.floor(0.5 + t * f_303))
    # Resonant formant sweep
    res = np.sin(2.0 * np.pi * (800.0 + 2200.0 * np.exp(-t / 0.04)) * t) * 0.4
    acid_hit = np.tanh((saw_303 + res) * 2.0) * np.exp(-t / 0.09) * 0.8
    save_wav("Acid_303_Saw_Hit.wav", acid_hit)

    # 2.3 Sub Sine 40Hz
    dur = 0.50
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    sub_sine = np.sin(2.0 * np.pi * 40.0 * t) * np.exp(-t / 0.18)
    save_wav("Sub_Sine_40Hz.wav", sub_sine)

    # 2.4 Reese Distortion Bass
    dur = 0.40
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    f_c1 = 55.0
    saw1 = 2.0 * (t * (f_c1 - 0.7) - np.floor(0.5 + t * (f_c1 - 0.7)))
    saw2 = 2.0 * (t * (f_c1 + 0.7) - np.floor(0.5 + t * (f_c1 + 0.7)))
    reese = np.tanh((saw1 + saw2) * 1.5) * np.exp(-t / 0.15) * 0.7
    save_wav("Reese_Distortion_Bass.wav", reese)

    # =========================================================================
    # 3. SNARES & CLAPS
    # =========================================================================
    # 3.1 909 Tight Snare
    dur = 0.20
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    body = (np.sin(2.0 * np.pi * 185.0 * t) + np.sin(2.0 * np.pi * 330.0 * t) * 0.5) * np.exp(-t / 0.035)
    noise = np.random.uniform(-1.0, 1.0, len(t)) * np.exp(-t / 0.065) * 0.8
    snare_909 = (body * 0.6 + noise * 0.7)
    save_wav("909_Tight_Snare.wav", snare_909)

    # 3.2 808 Crisp Clap (Stereo spread)
    dur = 0.25
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    clap_l = np.zeros_like(t)
    clap_r = np.zeros_like(t)
    for delay, gain in [(0.0, 0.5), (0.012, 0.7), (0.024, 0.9), (0.036, 1.0)]:
        idx = int(delay * sr)
        if idx < len(t):
            sub_t = t[:len(t)-idx]
            env = np.exp(-sub_t / 0.04) * gain
            nl = np.random.uniform(-1.0, 1.0, len(sub_t)) * env
            nr = np.random.uniform(-1.0, 1.0, len(sub_t)) * env
            clap_l[idx:] += nl
            clap_r[idx:] += nr
    save_wav("808_Crisp_Clap.wav", clap_l * 0.6, clap_r * 0.6)

    # 3.3 EDM Smash Clap
    dur = 0.35
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    noise = np.random.uniform(-1.0, 1.0, len(t)) * np.exp(-t / 0.09)
    smash = np.tanh(noise * 2.2) * 0.8
    save_wav("EDM_Smash_Clap.wav", smash)

    # 3.4 Ghost Snare Click
    dur = 0.08
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    ghost = np.sin(2.0 * np.pi * 950.0 * t) * np.exp(-t / 0.015)
    save_wav("Ghost_Snare_Click.wav", ghost)

    # =========================================================================
    # 4. HIHATS & PERCUSSION
    # =========================================================================
    # 4.1 Closed Metal Hat
    dur = 0.05
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    freqs = [298.0, 365.0, 420.0, 537.0, 652.0, 800.0]
    sq = np.zeros_like(t)
    for f in freqs:
        sq += np.sign(np.sin(2.0 * np.pi * f * 5.0 * t))
    hat = sq * np.exp(-t / 0.018) * 0.25
    save_wav("Closed_Metal_Hat.wav", hat)

    # 4.2 Open Psy Hat
    dur = 0.25
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    sq = np.zeros_like(t)
    for f in freqs:
        sq += np.sign(np.sin(2.0 * np.pi * f * 5.0 * t))
    open_hat = sq * np.exp(-t / 0.085) * 0.22
    save_wav("Open_Psy_Hat.wav", open_hat)

    # 4.3 Psy Click Perc
    dur = 0.03
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    click_perc = np.sin(2.0 * np.pi * 2400.0 * t) * np.exp(-t / 0.005)
    save_wav("Psy_Click_Perc.wav", click_perc)

    # 4.4 Shaker Loop Hit
    dur = 0.07
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    shaker = np.random.uniform(-1.0, 1.0, len(t)) * (t / 0.03) * np.exp(-t / 0.02) * 0.7
    save_wav("Shaker_Groove_Hit.wav", shaker)

    # =========================================================================
    # 5. SFX & RISERS
    # =========================================================================
    # 5.1 Psy Zap Laser
    dur = 0.12
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    zap_f = 120.0 + 4500.0 * np.exp(-t / 0.018)
    zap_phase = 2.0 * np.pi * np.cumsum(zap_f) / sr
    zap = np.sin(zap_phase) * np.exp(-t / 0.04)
    save_wav("Psy_Zap_Laser.wav", zap)

    # 5.2 Alien Downlifter
    dur = 1.2
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    down_f = 80.0 + 3200.0 * np.exp(-t / 0.35)
    down_fm = np.sin(2.0 * np.pi * 12.0 * t) * 40.0
    down_phase = 2.0 * np.pi * np.cumsum(down_f + down_fm) / sr
    alien_down = np.sin(down_phase) * np.exp(-t / 0.45)
    save_wav("Alien_Downlifter.wav", alien_down)

    # 5.3 White Noise Sweep Riser
    dur = 1.5
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    noise = np.random.uniform(-1.0, 1.0, len(t))
    rise_env = (t / dur) ** 2.0
    sweep = noise * rise_env * 0.6
    save_wav("White_Noise_Sweep.wav", sweep)

    # 5.4 Cyber Sub Impact
    dur = 0.8
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    boom_f = 35.0 + 120.0 * np.exp(-t / 0.04)
    boom_phase = 2.0 * np.pi * np.cumsum(boom_f) / sr
    impact = (np.sin(boom_phase) + np.random.uniform(-1.0, 1.0, len(t)) * np.exp(-t / 0.008) * 0.4) * np.exp(-t / 0.25)
    save_wav("Cyber_Sub_Impact.wav", impact)

    print(f"\n[SOUND-BANK SUCCESS] Todos os 20 samples EDM & Psytrance foram sintetizados com sucesso em '{output_dir}'!")

if __name__ == "__main__":
    create_soundbank()
