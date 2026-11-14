import os
import math
import wave
import numpy as np

def generate_electronic_soundbanks():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    base_dir = os.path.join(script_dir, "assets", "samples", "Electronic_Soundbanks")
    os.makedirs(base_dir, exist_ok=True)
    
    sr = 44100
    
    def save_wav(subpath, data_l, data_r=None):
        full_path = os.path.join(base_dir, subpath)
        os.makedirs(os.path.dirname(full_path), exist_ok=True)
        if data_r is None:
            data_r = data_l
        data_l = np.clip(data_l, -1.0, 1.0)
        data_r = np.clip(data_r, -1.0, 1.0)
        st = np.column_stack((data_l, data_r))
        int_data = (st * 32767.0).astype(np.int16)
        with wave.open(full_path, 'wb') as wf:
            wf.setnchannels(2)
            wf.setsampwidth(2)
            wf.setframerate(sr)
            wf.writeframes(int_data.tobytes())
        print(f"[SAMPLE-BANK] Criado: {subpath} ({len(data_l)/sr:.3f}s)")

    # =========================================================================
    # 01. PSYTRANCE (138 - 145 BPM)
    # =========================================================================
    psy_dir = "01_Psytrance"
    
    # 1.1 Psy Punch Kick 140BPM (Astrix style: punchy transient + tight sub decay)
    dur = 0.22
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    freq_env = 54.0 + 260.0 * np.exp(-t / 0.022)
    phase = 2.0 * np.pi * np.cumsum(freq_env) / sr
    amp_env = np.exp(-t / 0.080)
    click = np.random.uniform(-1.0, 1.0, len(t)) * np.exp(-t / 0.003) * 0.45
    kick = np.tanh((np.sin(phase) + click) * 1.35) * amp_env
    save_wav(f"{psy_dir}/01_Kicks/Psy_Punch_Kick_140BPM.wav", kick)

    # 1.2 HiTech Laser Kick 145BPM (Sharper click + high pitch drop)
    dur = 0.19
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    freq_env = 60.0 + 380.0 * np.exp(-t / 0.016)
    phase = 2.0 * np.pi * np.cumsum(freq_env) / sr
    amp_env = np.exp(-t / 0.065)
    laser_click = np.sin(2.0 * np.pi * 3200.0 * t) * np.exp(-t / 0.004) * 0.5
    hitech_kick = np.tanh((np.sin(phase) + laser_click) * 1.4) * amp_env
    save_wav(f"{psy_dir}/01_Kicks/HiTech_Laser_Kick_145BPM.wav", hitech_kick)

    # 1.3 Deep Psy Sub Kick (Warm rounded tail)
    dur = 0.25
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    freq_env = 48.0 + 190.0 * np.exp(-t / 0.030)
    phase = 2.0 * np.pi * np.cumsum(freq_env) / sr
    amp_env = np.exp(-t / 0.100)
    deep_kick = np.sin(phase) * amp_env * 0.95
    save_wav(f"{psy_dir}/01_Kicks/Deep_Psy_Sub_Kick.wav", deep_kick)

    # 1.4 FullOn Tok Kick
    dur = 0.20
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    freq_env = 55.0 + 450.0 * np.exp(-t / 0.012)
    phase = 2.0 * np.pi * np.cumsum(freq_env) / sr
    amp_env = np.exp(-t / 0.075)
    tok = np.sin(2.0 * np.pi * 1200.0 * t) * np.exp(-t / 0.008) * 0.4
    fullon_kick = np.tanh((np.sin(phase) + tok) * 1.3) * amp_env
    save_wav(f"{psy_dir}/01_Kicks/FullOn_Tok_Kick.wav", fullon_kick)

    # 1.5 Rolling Psy Bass C1 (65.41Hz 16th note)
    dur = 0.107
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    f0 = 65.41
    saw = 2.0 * (t * f0 - np.floor(0.5 + t * f0))
    cutoff_env = np.exp(-t / 0.032)
    rolling_c1 = saw * (0.35 + 0.65 * cutoff_env) * np.exp(-t / 0.075)
    rolling_c1 = np.tanh(rolling_c1 * 1.5) * 0.9
    save_wav(f"{psy_dir}/02_Rolling_Basses/Rolling_Psy_Bass_C1.wav", rolling_c1)

    # 1.6 Rolling Psy Bass D#1 (77.78Hz)
    f0 = 77.78
    saw = 2.0 * (t * f0 - np.floor(0.5 + t * f0))
    rolling_ds1 = np.tanh(saw * (0.35 + 0.65 * np.exp(-t / 0.030)) * 1.5) * np.exp(-t / 0.070) * 0.9
    save_wav(f"{psy_dir}/02_Rolling_Basses/Rolling_Psy_Bass_Ds1.wav", rolling_ds1)

    # 1.7 Offbeat Psy Bass F1 (87.31Hz)
    dur_off = 0.18
    t_off = np.linspace(0, dur_off, int(sr * dur_off), endpoint=False)
    f0 = 87.31
    saw = 2.0 * (t_off * f0 - np.floor(0.5 + t_off * f0))
    offbeat_f1 = saw * np.exp(-t_off / 0.09) * 0.85
    save_wav(f"{psy_dir}/02_Rolling_Basses/Offbeat_Psy_Bass_F1.wav", offbeat_f1)

    # 1.8 Sub Sine Pure Psy 42Hz
    dur_sub = 0.45
    t_sub = np.linspace(0, dur_sub, int(sr * dur_sub), endpoint=False)
    sub_pure = np.sin(2.0 * np.pi * 42.0 * t_sub) * np.exp(-t_sub / 0.18) * 0.95
    save_wav(f"{psy_dir}/02_Rolling_Basses/Sub_Sine_Pure_Psy_42Hz.wav", sub_pure)

    # 1.9 Reverse Psy Bass Suck FX
    dur_rev = 0.22
    t_rev = np.linspace(0, dur_rev, int(sr * dur_rev), endpoint=False)
    f_rev = 45.0 + 80.0 * (t_rev / dur_rev) ** 2.0
    phase_rev = 2.0 * np.pi * np.cumsum(f_rev) / sr
    rev_bass = np.sin(phase_rev) * ((t_rev / dur_rev) ** 2.5) * 0.9
    save_wav(f"{psy_dir}/02_Rolling_Basses/Reverse_Psy_Suck_Bass.wav", rev_bass)

    # 1.10 Psy Leads & Arps
    dur_lead = 0.28
    t_lead = np.linspace(0, dur_lead, int(sr * dur_lead), endpoint=False)
    f_lead = 261.63 # C4
    saw_lead = 2.0 * (t_lead * f_lead - np.floor(0.5 + t_lead * f_lead))
    res_sweep = np.sin(2.0 * np.pi * (1200.0 + 3400.0 * np.exp(-t_lead / 0.05)) * t_lead) * 0.4
    goa_squawk = np.tanh((saw_lead + res_sweep) * 2.2) * np.exp(-t_lead / 0.11) * 0.85
    save_wav(f"{psy_dir}/03_Leads_and_Arps/Goa_Squawk_Lead_Hit.wav", goa_squawk)

    # Psy Saw Arp Stab
    f_arp = 523.25 # C5
    saw_arp = 2.0 * (t_lead * f_arp - np.floor(0.5 + t_lead * f_arp))
    arp_stab = np.tanh(saw_arp * 1.8) * np.exp(-t_lead / 0.08) * 0.8
    save_wav(f"{psy_dir}/03_Leads_and_Arps/Psy_Saw_Arp_Stab_C5.wav", arp_stab)

    # Cosmic Alien Pluck
    pluck_fm = np.sin(2.0 * np.pi * 392.0 * t_lead + 3.0 * np.sin(2.0 * np.pi * 784.0 * t_lead) * np.exp(-t_lead / 0.04))
    alien_pluck = pluck_fm * np.exp(-t_lead / 0.09) * 0.85
    save_wav(f"{psy_dir}/03_Leads_and_Arps/Cosmic_Alien_Lead_Pluck.wav", alien_pluck)

    # 1.11 Percussion & Hats
    # Closed Hat 16th
    dur_h = 0.045
    t_h = np.linspace(0, dur_h, int(sr * dur_h), endpoint=False)
    freqs_hat = [2400.0, 3100.0, 4800.0, 6200.0, 7500.0, 9200.0]
    sq = sum([np.sign(np.sin(2.0 * np.pi * f * t_h)) for f in freqs_hat])
    closed_hat = sq * np.exp(-t_h / 0.015) * 0.2
    save_wav(f"{psy_dir}/04_Percussion_and_Hats/Psy_Closed_Hat_16th.wav", closed_hat)

    # Open Hat Bright
    dur_oh = 0.24
    t_oh = np.linspace(0, dur_oh, int(sr * dur_oh), endpoint=False)
    sq_o = sum([np.sign(np.sin(2.0 * np.pi * f * t_oh)) for f in freqs_hat])
    open_hat = sq_o * np.exp(-t_oh / 0.075) * 0.18
    save_wav(f"{psy_dir}/04_Percussion_and_Hats/Psy_Open_Hat_Bright.wav", open_hat)

    # Psy Click Perc
    dur_c = 0.025
    t_c = np.linspace(0, dur_c, int(sr * dur_c), endpoint=False)
    click_p = np.sin(2.0 * np.pi * 2800.0 * t_c) * np.exp(-t_c / 0.004) * 0.9
    save_wav(f"{psy_dir}/04_Percussion_and_Hats/Psy_Click_Perc_Transient.wav", click_p)

    # Tribal Tom
    dur_tom = 0.18
    t_tom = np.linspace(0, dur_tom, int(sr * dur_tom), endpoint=False)
    f_tom = 90.0 + 120.0 * np.exp(-t_tom / 0.03)
    tom = np.sin(2.0 * np.pi * np.cumsum(f_tom) / sr) * np.exp(-t_tom / 0.06) * 0.85
    save_wav(f"{psy_dir}/04_Percussion_and_Hats/Tribal_Tom_Hit_Psy.wav", tom)

    # Psy Snare Snap
    dur_sn = 0.18
    t_sn = np.linspace(0, dur_sn, int(sr * dur_sn), endpoint=False)
    body_sn = np.sin(2.0 * np.pi * 220.0 * t_sn) * np.exp(-t_sn / 0.03)
    noise_sn = np.random.uniform(-1.0, 1.0, len(t_sn)) * np.exp(-t_sn / 0.07)
    psy_snare = (body_sn * 0.5 + noise_sn * 0.75) * 0.9
    save_wav(f"{psy_dir}/04_Percussion_and_Hats/Psy_Snare_Snap.wav", psy_snare)

    # Psy Tight Clap
    dur_cl = 0.20
    t_cl = np.linspace(0, dur_cl, int(sr * dur_cl), endpoint=False)
    cl_l = np.zeros_like(t_cl)
    cl_r = np.zeros_like(t_cl)
    for delay, g in [(0.0, 0.4), (0.009, 0.6), (0.018, 0.85), (0.027, 1.0)]:
        idx = int(delay * sr)
        if idx < len(t_cl):
            st = t_cl[:len(t_cl)-idx]
            env = np.exp(-st / 0.035) * g
            cl_l[idx:] += np.random.uniform(-1.0, 1.0, len(st)) * env
            cl_r[idx:] += np.random.uniform(-1.0, 1.0, len(st)) * env
    save_wav(f"{psy_dir}/04_Percussion_and_Hats/Psy_Tight_Clap.wav", cl_l * 0.6, cl_r * 0.6)

    # 1.12 FX & Vocals
    # Laser Zap FX
    dur_z = 0.14
    t_z = np.linspace(0, dur_z, int(sr * dur_z), endpoint=False)
    f_z = 100.0 + 5200.0 * np.exp(-t_z / 0.020)
    zap = np.sin(2.0 * np.pi * np.cumsum(f_z) / sr) * np.exp(-t_z / 0.045) * 0.85
    save_wav(f"{psy_dir}/05_FX_and_Vocals/Psy_Laser_Zap_FX.wav", zap)

    # Cosmic Downlifter FX
    dur_dl = 1.4
    t_dl = np.linspace(0, dur_dl, int(sr * dur_dl), endpoint=False)
    f_dl = 70.0 + 3600.0 * np.exp(-t_dl / 0.38)
    downlifter = np.sin(2.0 * np.pi * np.cumsum(f_dl) / sr) * np.exp(-t_dl / 0.55) * 0.8
    save_wav(f"{psy_dir}/05_FX_and_Vocals/Cosmic_Downlifter_FX.wav", downlifter)

    # Shamanic Mantra Chant (Formant synthesis: A-O-U chant)
    dur_man = 0.8
    t_man = np.linspace(0, dur_man, int(sr * dur_man), endpoint=False)
    f_throat = 98.0 # G2
    vocal = np.sin(2.0 * np.pi * f_throat * t_man)
    vocal += 0.5 * np.sin(2.0 * np.pi * f_throat * 2 * t_man)
    # Formants for "Om"
    vocal *= (0.6 + 0.4 * np.sin(2.0 * np.pi * 650.0 * t_man)) * (0.6 + 0.4 * np.sin(2.0 * np.pi * 1050.0 * t_man))
    vocal *= np.sin(np.pi * t_man / dur_man) # Bell envelope
    vocal = np.tanh(vocal * 1.8) * 0.85
    save_wav(f"{psy_dir}/05_FX_and_Vocals/Shamanic_Mantra_Chant.wav", vocal)

    # Psy Uplifter Noise Rise
    dur_rise = 1.6
    t_rise = np.linspace(0, dur_rise, int(sr * dur_rise), endpoint=False)
    noise_r = np.random.uniform(-1.0, 1.0, len(t_rise))
    uplifter = noise_r * ((t_rise / dur_rise) ** 2.2) * 0.65
    save_wav(f"{psy_dir}/05_FX_and_Vocals/Psy_Uplifter_Noise_Rise.wav", uplifter)

    # =========================================================================
    # 02. PEAK TIME & RAW TECHNO (130 - 136 BPM)
    # =========================================================================
    techno_dir = "02_Peak_Time_Techno"

    # 2.1 Peak Time Rumble Kick 132BPM (Punch + Reverb Sub Rumble)
    dur = 0.38
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    f_k = 44.0 + 240.0 * np.exp(-t / 0.020)
    punch = np.sin(2.0 * np.pi * np.cumsum(f_k) / sr) * np.exp(-t / 0.065)
    # Sub rumble reverb tail simulation (filtered distorted decay)
    rumble = np.sin(2.0 * np.pi * 48.0 * t) * (1.0 - np.exp(-t / 0.04)) * np.exp(-t / 0.22) * 0.8
    rumble_dist = np.tanh(rumble * 2.0)
    peak_kick = np.tanh(punch * 1.4 + rumble_dist * 0.7) * 0.95
    save_wav(f"{techno_dir}/01_Rumble_Kicks/PeakTime_Rumble_Kick_132BPM.wav", peak_kick)

    # 2.2 Industrial Distorted Kick (Hard clipped warehouse punch)
    dur = 0.30
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    f_ind = 40.0 + 320.0 * np.exp(-t / 0.015)
    ind_punch = np.sin(2.0 * np.pi * np.cumsum(f_ind) / sr) * np.exp(-t / 0.08)
    ind_kick = np.tanh(ind_punch * 3.5) * 0.9
    save_wav(f"{techno_dir}/01_Rumble_Kicks/Industrial_Distorted_Kick.wav", ind_kick)

    # 2.3 Berlin Raw 909 Kick
    dur = 0.28
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    f_909 = 48.0 + 200.0 * np.exp(-t / 0.024)
    raw_909 = np.sin(2.0 * np.pi * np.cumsum(f_909) / sr) * np.exp(-t / 0.11) * 0.92
    save_wav(f"{techno_dir}/01_Rumble_Kicks/Berlin_Raw_Kick.wav", raw_909)

    # 2.4 Dark Sub Impact Kick
    dur = 0.50
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    f_sub = 36.0 + 140.0 * np.exp(-t / 0.035)
    sub_imp = np.sin(2.0 * np.pi * np.cumsum(f_sub) / sr) * np.exp(-t / 0.22) * 0.95
    save_wav(f"{techno_dir}/01_Rumble_Kicks/Dark_Sub_Impact_Kick.wav", sub_imp)

    # 2.5 Industrial Monolith Bass C1
    dur_b = 0.40
    t_b = np.linspace(0, dur_b, int(sr * dur_b), endpoint=False)
    f_mono = 32.7 # C1
    mono_bass = np.tanh(np.sin(2.0 * np.pi * f_mono * t_b) * 2.5) * np.exp(-t_b / 0.25) * 0.9
    save_wav(f"{techno_dir}/02_Industrial_Basses/Industrial_Monolith_Bass_C1.wav", mono_bass)

    # 2.6 Techno Rumble Sub Tail
    dur_rt = 0.35
    t_rt = np.linspace(0, dur_rt, int(sr * dur_rt), endpoint=False)
    rumble_tail = np.sin(2.0 * np.pi * 50.0 * t_rt) * np.exp(-t_rt / 0.16) * 0.85
    save_wav(f"{techno_dir}/02_Industrial_Basses/Techno_Rumble_Sub_Tail.wav", rumble_tail)

    # 2.7 Dark Modular Bass Stab
    saw_mod = 2.0 * (t_b * 65.41 - np.floor(0.5 + t_b * 65.41))
    mod_stab = np.tanh(saw_mod * (1.0 + 2.0 * np.exp(-t_b / 0.04))) * np.exp(-t_b / 0.12) * 0.85
    save_wav(f"{techno_dir}/02_Industrial_Basses/Dark_Modular_Bass_Stab.wav", mod_stab)

    # 2.8 Dark Techno Minor Chord Stab
    dur_s = 0.35
    t_s = np.linspace(0, dur_s, int(sr * dur_s), endpoint=False)
    # Minor triad: C4 (261.63), Eb4 (311.13), G4 (392.00)
    c4 = 2.0 * (t_s * 261.63 - np.floor(0.5 + t_s * 261.63))
    eb4 = 2.0 * (t_s * 311.13 - np.floor(0.5 + t_s * 311.13))
    g4 = 2.0 * (t_s * 392.00 - np.floor(0.5 + t_s * 392.00))
    chord_raw = (c4 + eb4 * 0.8 + g4 * 0.7) / 2.5
    techno_stab = np.tanh(chord_raw * 2.2) * np.exp(-t_s / 0.12) * 0.85
    save_wav(f"{techno_dir}/03_Stabs_and_Synths/Dark_Techno_Minor_Chord_Stab.wav", techno_stab)

    # 2.9 Rave Hoover Stab
    f_h = 220.0
    saw_h1 = 2.0 * (t_s * (f_h - 1.5) - np.floor(0.5 + t_s * (f_h - 1.5)))
    saw_h2 = 2.0 * (t_s * (f_h + 1.5) - np.floor(0.5 + t_s * (f_h + 1.5)))
    hoover = np.tanh((saw_h1 + saw_h2) * 1.8) * np.exp(-t_s / 0.15) * 0.8
    save_wav(f"{techno_dir}/03_Stabs_and_Synths/Rave_Hoover_Stab.wav", hoover)

    # 2.10 Minimal Techno Beep Pluck
    beep = np.sin(2.0 * np.pi * 1760.0 * t_s) * np.exp(-t_s / 0.02) * 0.8
    save_wav(f"{techno_dir}/03_Stabs_and_Synths/Minimal_Techno_Beep_Pluck.wav", beep)

    # 2.11 Percussion & Rides
    # Warehouse Reverb Clap
    dur_w = 0.35
    t_w = np.linspace(0, dur_w, int(sr * dur_w), endpoint=False)
    w_clap_l = np.random.uniform(-1.0, 1.0, len(t_w)) * np.exp(-t_w / 0.10)
    w_clap_r = np.random.uniform(-1.0, 1.0, len(t_w)) * np.exp(-t_w / 0.10)
    save_wav(f"{techno_dir}/04_Percussion_and_Rides/Warehouse_Reverb_Clap.wav", w_clap_l * 0.8, w_clap_r * 0.8)

    # Industrial Metallic Snare
    dur_is = 0.22
    t_is = np.linspace(0, dur_is, int(sr * dur_is), endpoint=False)
    m_tone = (np.sin(2.0 * np.pi * 320.0 * t_is) + np.sin(2.0 * np.pi * 510.0 * t_is) * 0.6) * np.exp(-t_is / 0.04)
    m_noise = np.random.uniform(-1.0, 1.0, len(t_is)) * np.exp(-t_is / 0.07)
    ind_snare = (m_tone * 0.5 + m_noise * 0.7) * 0.9
    save_wav(f"{techno_dir}/04_Percussion_and_Rides/Industrial_Metallic_Snare.wav", ind_snare)

    # Techno 909 Ride Cymbal
    dur_ride = 0.60
    t_ride = np.linspace(0, dur_ride, int(sr * dur_ride), endpoint=False)
    f_ride = [415.0, 580.0, 790.0, 1040.0, 1320.0, 1680.0]
    sq_r = sum([np.sign(np.sin(2.0 * np.pi * f * t_ride)) for f in f_ride])
    ride = sq_r * np.exp(-t_ride / 0.25) * 0.16
    save_wav(f"{techno_dir}/04_Percussion_and_Rides/Techno_Ride_Cymbal_909.wav", ride)

    # Raw Closed Hat
    dur_rc = 0.04
    t_rc = np.linspace(0, dur_rc, int(sr * dur_rc), endpoint=False)
    raw_hat = np.random.uniform(-1.0, 1.0, len(t_rc)) * np.exp(-t_rc / 0.012) * 0.75
    save_wav(f"{techno_dir}/04_Percussion_and_Rides/Raw_Closed_Hat.wav", raw_hat)

    # 2.12 Drones & FX
    # Factory Steam Exhaust FX
    dur_st = 1.0
    t_st = np.linspace(0, dur_st, int(sr * dur_st), endpoint=False)
    steam = np.random.uniform(-1.0, 1.0, len(t_st)) * np.exp(-t_st / 0.35) * 0.7
    save_wav(f"{techno_dir}/05_Drones_and_FX/Factory_Steam_Exhaust_FX.wav", steam)

    # Sub Boom Drop FX
    dur_b = 1.2
    t_b = np.linspace(0, dur_b, int(sr * dur_b), endpoint=False)
    boom_f = 28.0 + 85.0 * np.exp(-t_b / 0.06)
    sub_boom = np.sin(2.0 * np.pi * np.cumsum(boom_f) / sr) * np.exp(-t_b / 0.40) * 0.95
    save_wav(f"{techno_dir}/05_Drones_and_FX/Sub_Boom_Drop_FX.wav", sub_boom)

    # =========================================================================
    # 03. ACID TECHNO 303 (TB-303 Resonance & Distortion)
    # =========================================================================
    acid_dir = "03_Acid_Techno_303"

    # 3.1 TB-303 Resonant Saw C1 (32.7Hz or C2 65.4Hz with dynamic screaming filter)
    dur = 0.24
    t = np.linspace(0, dur, int(sr * dur), endpoint=False)
    f_acid = 65.41 # C2
    saw_acid = 2.0 * (t * f_acid - np.floor(0.5 + t * f_acid))
    # 303 filter sweep from 3500Hz down to 400Hz with high Q resonance
    res_freq = 400.0 + 3200.0 * np.exp(-t / 0.035)
    res_phase = 2.0 * np.pi * np.cumsum(res_freq) / sr
    res_ring = np.sin(res_phase) * 0.65
    tb303_c1 = np.tanh((saw_acid + res_ring) * 2.8) * np.exp(-t / 0.09) * 0.85
    save_wav(f"{acid_dir}/01_Acid_303_Bass_and_Squelch/TB303_Resonant_Saw_C1.wav", tb303_c1)

    # 3.2 TB-303 Acid Squelch Stab (High resonance squeal)
    dur_sq = 0.18
    t_sq = np.linspace(0, dur_sq, int(sr * dur_sq), endpoint=False)
    f_sq = 98.0
    saw_sq = 2.0 * (t_sq * f_sq - np.floor(0.5 + t_sq * f_sq))
    squelch_res = np.sin(2.0 * np.pi * (1800.0 + 4500.0 * np.exp(-t_sq / 0.025)) * t_sq) * 0.8
    tb303_squelch = np.tanh((saw_sq + squelch_res) * 3.5) * np.exp(-t_sq / 0.07) * 0.85
    save_wav(f"{acid_dir}/01_Acid_303_Bass_and_Squelch/TB303_Acid_Squelch_Stab.wav", tb303_squelch)

    # 3.3 TB-303 Distorted Slide Note
    dur_sl = 0.32
    t_sl = np.linspace(0, dur_sl, int(sr * dur_sl), endpoint=False)
    # Pitch slide from C2 (65.4) to D#2 (77.8)
    f_slide = 65.41 + (77.78 - 65.41) * (1.0 - np.exp(-t_sl / 0.08))
    phase_slide = 2.0 * np.pi * np.cumsum(f_slide) / sr
    saw_slide = 2.0 * (phase_slide / (2.0 * np.pi) - np.floor(0.5 + phase_slide / (2.0 * np.pi)))
    tb303_slide = np.tanh(saw_slide * 3.0) * np.exp(-t_sl / 0.14) * 0.85
    save_wav(f"{acid_dir}/01_Acid_303_Bass_and_Squelch/TB303_Distorted_Slide_Note.wav", tb303_slide)

    # 3.4 TB-303 Square Acid Hit (Hollow square bite)
    sq_303 = np.sign(np.sin(2.0 * np.pi * 65.41 * t))
    res_sq = np.sin(2.0 * np.pi * (600.0 + 2800.0 * np.exp(-t / 0.04)) * t) * 0.6
    tb303_square = np.tanh((sq_303 + res_sq) * 2.0) * np.exp(-t / 0.09) * 0.85
    save_wav(f"{acid_dir}/01_Acid_303_Bass_and_Squelch/TB303_Square_Acid_Hit.wav", tb303_square)

    # 3.5 Acid Hard Click Kick
    dur_ak = 0.22
    t_ak = np.linspace(0, dur_ak, int(sr * dur_ak), endpoint=False)
    f_ak = 46.0 + 380.0 * np.exp(-t_ak / 0.012)
    punch_ak = np.sin(2.0 * np.pi * np.cumsum(f_ak) / sr)
    click_ak = np.sin(2.0 * np.pi * 4200.0 * t_ak) * np.exp(-t_ak / 0.003) * 0.6
    acid_kick = np.tanh((punch_ak + click_ak) * 2.0) * np.exp(-t_ak / 0.08) * 0.92
    save_wav(f"{acid_dir}/02_Hard_Kicks/Acid_Hard_Click_Kick.wav", acid_kick)

    # 3.6 Acid Distorted Punch Kick
    dist_kick = np.tanh(punch_ak * 3.0) * np.exp(-t_ak / 0.09) * 0.9
    save_wav(f"{acid_dir}/02_Hard_Kicks/Acid_Distorted_Punch_Kick.wav", dist_kick)

    # 3.7 Acid Aggressive Percussion
    # 909 Open Hat
    acid_ohat = sq_o * np.exp(-t_oh / 0.09) * 0.22
    save_wav(f"{acid_dir}/03_Aggressive_Percussion/Acid_909_Open_Hat.wav", acid_ohat)

    # Saturated Clap
    sat_clap = np.tanh(cl_l * 2.5) * 0.8
    save_wav(f"{acid_dir}/03_Aggressive_Percussion/Acid_Saturated_Clap.wav", sat_clap)

    # Acid Snare Roll Hit
    acid_snare = np.tanh(psy_snare * 1.8) * 0.85
    save_wav(f"{acid_dir}/03_Aggressive_Percussion/Acid_Snare_Roll_Hit.wav", acid_snare)

    # 3.8 Acid Resonance Filter Scream FX
    dur_scr = 0.8
    t_scr = np.linspace(0, dur_scr, int(sr * dur_scr), endpoint=False)
    f_scr = 800.0 + 4000.0 * np.sin(np.pi * t_scr / dur_scr)
    scream = np.sin(2.0 * np.pi * np.cumsum(f_scr) / sr)
    scream = np.tanh(scream * 3.0) * (np.sin(np.pi * t_scr / dur_scr) ** 0.5) * 0.75
    save_wav(f"{acid_dir}/04_Resonant_Sweeps/Acid_Resonance_Filter_Scream_FX.wav", scream)

    # =========================================================================
    # 04. MELODIC TECHNO & PROGRESSIVE (124 - 128 BPM)
    # =========================================================================
    melodic_dir = "04_Melodic_Techno"

    # 4.1 Melodic Deep Warm Kick
    dur_mk = 0.32
    t_mk = np.linspace(0, dur_mk, int(sr * dur_mk), endpoint=False)
    f_mk = 48.0 + 160.0 * np.exp(-t_mk / 0.035)
    deep_warm = np.sin(2.0 * np.pi * np.cumsum(f_mk) / sr) * np.exp(-t_mk / 0.12) * 0.95
    save_wav(f"{melodic_dir}/01_Deep_Kicks/Melodic_Deep_Warm_Kick.wav", deep_warm)

    # 4.2 Melodic Punch Kick
    f_mp = 52.0 + 220.0 * np.exp(-t_mk / 0.022)
    punch_mel = np.sin(2.0 * np.pi * np.cumsum(f_mp) / sr) * np.exp(-t_mk / 0.09) * 0.95
    save_wav(f"{melodic_dir}/01_Deep_Kicks/Melodic_Punch_Kick.wav", punch_mel)

    # 4.3 Moog Analog Pluck Bass C1 (Afterlife style)
    dur_moog = 0.28
    t_moog = np.linspace(0, dur_moog, int(sr * dur_moog), endpoint=False)
    f_moog = 32.7
    saw_moog = 2.0 * (t_moog * f_moog - np.floor(0.5 + t_moog * f_moog))
    sub_moog = np.sin(2.0 * np.pi * f_moog * t_moog)
    moog_bass = (saw_moog * (0.2 + 0.8 * np.exp(-t_moog / 0.04)) + sub_moog * 0.7) * np.exp(-t_moog / 0.14)
    save_wav(f"{melodic_dir}/02_Pluck_and_Moog_Basses/Moog_Analog_Pluck_Bass_C1.wav", moog_bass * 0.9)

    # 4.4 Rolling Progressive Sub D1 (36.7Hz)
    f_prog = 36.7
    prog_sub = np.sin(2.0 * np.pi * f_prog * t_moog) * np.exp(-t_moog / 0.16) * 0.95
    save_wav(f"{melodic_dir}/02_Pluck_and_Moog_Basses/Rolling_Progressive_Sub_D1.wav", prog_sub)

    # 4.5 Afterlife Saw Pluck Lead (Anyma style)
    dur_pl = 0.35
    t_pl = np.linspace(0, dur_pl, int(sr * dur_pl), endpoint=False)
    f_pl = 220.0 # A3
    saw_p1 = 2.0 * (t_pl * (f_pl - 0.5) - np.floor(0.5 + t_pl * (f_pl - 0.5)))
    saw_p2 = 2.0 * (t_pl * (f_pl + 0.5) - np.floor(0.5 + t_pl * (f_pl + 0.5)))
    pluck_lead = (saw_p1 + saw_p2) * (0.25 + 0.75 * np.exp(-t_pl / 0.045)) * np.exp(-t_pl / 0.15) * 0.8
    save_wav(f"{melodic_dir}/03_Analog_Chords_and_Leads/Afterlife_Saw_Pluck_Lead.wav", pluck_lead)

    # 4.6 Lush Analog Pad Chord Hit
    dur_pad = 0.75
    t_pad = np.linspace(0, dur_pad, int(sr * dur_pad), endpoint=False)
    # A minor: A3 (220), C4 (261.63), E4 (329.63)
    pad = (np.sin(2.0 * np.pi * 220.0 * t_pad) + np.sin(2.0 * np.pi * 261.63 * t_pad) * 0.8 + np.sin(2.0 * np.pi * 329.63 * t_pad) * 0.7) / 2.5
    pad *= np.sin(np.pi * t_pad / dur_pad) ** 0.5 * 0.85
    save_wav(f"{melodic_dir}/03_Analog_Chords_and_Leads/Lush_Analog_Pad_Chord_Hit.wav", pad)

    # 4.7 Organic Wood Click & Shaker
    wood = np.sin(2.0 * np.pi * 1400.0 * t_c) * np.exp(-t_c / 0.006) * 0.8
    save_wav(f"{melodic_dir}/04_Organic_Percussion/Organic_Wood_Click.wav", wood)

    dur_sh = 0.09
    t_sh = np.linspace(0, dur_sh, int(sr * dur_sh), endpoint=False)
    shaker_m = np.random.uniform(-1.0, 1.0, len(t_sh)) * (t_sh / 0.03) * np.exp(-t_sh / 0.035) * 0.65
    save_wav(f"{melodic_dir}/04_Organic_Percussion/Smooth_Progressive_Shaker.wav", shaker_m)

    # =========================================================================
    # 05. TECH HOUSE & CLUB GROOVES (126 - 128 BPM)
    # =========================================================================
    tech_dir = "05_Tech_House"

    # 5.1 Tech House Slap Punch Kick
    dur_tk = 0.20
    t_tk = np.linspace(0, dur_tk, int(sr * dur_tk), endpoint=False)
    f_tk = 56.0 + 280.0 * np.exp(-t_tk / 0.015)
    slap_k = np.tanh(np.sin(2.0 * np.pi * np.cumsum(f_tk) / sr) * 1.5) * np.exp(-t_tk / 0.065) * 0.95
    save_wav(f"{tech_dir}/01_Slap_Kicks/TechHouse_Slap_Punch_Kick.wav", slap_k)

    # 5.2 FM Slap Donk Bass C1 (Fisher style)
    dur_fm = 0.16
    t_fm = np.linspace(0, dur_fm, int(sr * dur_fm), endpoint=False)
    f_carrier = 65.41
    f_mod = 130.82
    mod_idx = 3.5 * np.exp(-t_fm / 0.03)
    donk_bass = np.sin(2.0 * np.pi * f_carrier * t_fm + mod_idx * np.sin(2.0 * np.pi * f_mod * t_fm)) * np.exp(-t_fm / 0.08) * 0.9
    save_wav(f"{tech_dir}/02_FM_and_Donk_Basses/FM_Slap_Donk_Bass_C1.wav", donk_bass)

    # 5.3 Deep House Organ Bass
    organ_bass = (np.sin(2.0 * np.pi * 65.41 * t_fm) + 0.4 * np.sin(2.0 * np.pi * 130.82 * t_fm) + 0.2 * np.sin(2.0 * np.pi * 196.23 * t_fm)) * np.exp(-t_fm / 0.10) * 0.9
    save_wav(f"{tech_dir}/02_FM_and_Donk_Basses/Deep_House_Organ_Bass.wav", organ_bass)

    # 5.4 Crisp Layered House Clap
    crisp_clap = (cl_l * 0.8 + np.sin(2.0 * np.pi * 1200.0 * t_cl) * np.exp(-t_cl / 0.01) * 0.3)
    save_wav(f"{tech_dir}/03_House_Claps_and_Perc/Crisp_Layered_House_Clap.wav", crisp_clap)

    # 5.5 House Vocal Chop "Yeah!"
    dur_voc = 0.30
    t_voc = np.linspace(0, dur_voc, int(sr * dur_voc), endpoint=False)
    f_vocal = 160.0 + 80.0 * np.exp(-t_voc / 0.08)
    v_body = np.sin(2.0 * np.pi * np.cumsum(f_vocal) / sr)
    v_formant = np.sin(2.0 * np.pi * 1100.0 * t_voc) * 0.5 + np.sin(2.0 * np.pi * 2400.0 * t_voc) * 0.3
    vocal_chop = np.tanh((v_body + v_formant) * 1.6) * np.exp(-t_voc / 0.12) * 0.85
    save_wav(f"{tech_dir}/04_Vocal_Chops_and_Shakers/House_Vocal_Chop_Yeah.wav", vocal_chop)

    # =========================================================================
    # 06. HARDSTYLE & RAWSTYLE (150 - 155 BPM)
    # =========================================================================
    hard_dir = "06_Hardstyle_and_Raw"

    # 6.1 Rawstyle Distorted Screaming Kick
    dur_hk = 0.36
    t_hk = np.linspace(0, dur_hk, int(sr * dur_hk), endpoint=False)
    f_hk = 45.0 + 480.0 * np.exp(-t_hk / 0.016)
    hard_body = np.sin(2.0 * np.pi * np.cumsum(f_hk) / sr)
    raw_dist = np.tanh(hard_body * 5.0) * np.exp(-t_hk / 0.14) * 0.92
    save_wav(f"{hard_dir}/01_Distorted_Kicks/Rawstyle_Distorted_Screaming_Kick.wav", raw_dist)

    # 6.2 Reverse Bass Punch 150BPM
    dur_rb = 0.28
    t_rb = np.linspace(0, dur_rb, int(sr * dur_rb), endpoint=False)
    tok = np.sin(2.0 * np.pi * 900.0 * t_rb) * np.exp(-t_rb / 0.008) * 0.8
    sub_rev = np.sin(2.0 * np.pi * (50.0 + 30.0 * (t_rb / dur_rb)) * t_rb) * ((t_rb / dur_rb) ** 1.8) * 0.9
    save_wav(f"{hard_dir}/02_Reverse_Basses/Reverse_Bass_Punch_150BPM.wav", tok + sub_rev)

    # 6.3 Hardstyle Screech Hit
    dur_sc = 0.25
    t_sc = np.linspace(0, dur_sc, int(sr * dur_sc), endpoint=False)
    screech = np.sin(2.0 * np.pi * (2400.0 + 1200.0 * np.sin(2.0 * np.pi * 25.0 * t_sc)) * t_sc)
    screech = np.tanh(screech * 4.0) * np.exp(-t_sc / 0.10) * 0.8
    save_wav(f"{hard_dir}/03_Screeches_and_FX/Hardstyle_Screech_Hit.wav", screech)

    # =========================================================================
    # 07. CYBERPUNK & DARKSYNTH (115 - 125 BPM)
    # =========================================================================
    cyber_dir = "07_Cyberpunk_and_Darksynth"

    # 7.1 Cyber Sub Impact Kick
    dur_cb = 0.45
    t_cb = np.linspace(0, dur_cb, int(sr * dur_cb), endpoint=False)
    f_cb = 38.0 + 160.0 * np.exp(-t_cb / 0.025)
    cyber_punch = np.sin(2.0 * np.pi * np.cumsum(f_cb) / sr)
    cyber_kick = np.tanh((cyber_punch + np.random.uniform(-0.5, 0.5, len(t_cb)) * np.exp(-t_cb / 0.01)) * 1.8) * np.exp(-t_cb / 0.18) * 0.95
    save_wav(f"{cyber_dir}/01_Cyber_Kicks/Cyber_Sub_Impact_Kick.wav", cyber_kick)

    # 7.2 Cyber Reese Distortion Bass C1
    dur_reese = 0.45
    t_reese = np.linspace(0, dur_reese, int(sr * dur_reese), endpoint=False)
    f_reese = 32.7
    saw_r1 = 2.0 * (t_reese * (f_reese - 0.6) - np.floor(0.5 + t_reese * (f_reese - 0.6)))
    saw_r2 = 2.0 * (t_reese * (f_reese + 0.6) - np.floor(0.5 + t_reese * (f_reese + 0.6)))
    reese_cyber = np.tanh((saw_r1 + saw_r2) * 2.2) * np.exp(-t_reese / 0.20) * 0.85
    save_wav(f"{cyber_dir}/02_Reese_Basses/Cyber_Reese_Distortion_Bass_C1.wav", reese_cyber)

    # 7.3 Neon Darksynth Brass Hit
    dur_br = 0.38
    t_br = np.linspace(0, dur_br, int(sr * dur_br), endpoint=False)
    f_br = 130.81 # C3
    saw_b1 = 2.0 * (t_br * (f_br - 0.4) - np.floor(0.5 + t_br * (f_br - 0.4)))
    saw_b2 = 2.0 * (t_br * (f_br + 0.4) - np.floor(0.5 + t_br * (f_br + 0.4)))
    brass = np.tanh((saw_b1 + saw_b2) * (1.2 + 1.5 * np.exp(-t_br / 0.05))) * np.exp(-t_br / 0.16) * 0.85
    save_wav(f"{cyber_dir}/03_Neon_Stabs_and_Swells/Neon_Darksynth_Brass_Hit.wav", brass)

    print(f"\n[COMPLETE] Banco de Samples Eletrônicos criado com sucesso em: {base_dir}")

if __name__ == "__main__":
    generate_electronic_soundbanks()
