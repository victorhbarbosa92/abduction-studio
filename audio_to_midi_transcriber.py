import os
import sys
import wave
import json
import numpy as np

def load_wav_mono(path, target_sr=22050):
    with wave.open(path, 'rb') as wf:
        n_ch = wf.getnchannels()
        sr = wf.getframerate()
        n_frames = wf.getnframes()
        sampwidth = wf.getsampwidth()
        raw_bytes = wf.readframes(n_frames)

    if sampwidth == 2:
        data = np.frombuffer(raw_bytes, dtype=np.int16).astype(np.float32) / 32768.0
    elif sampwidth == 4:
        data = np.frombuffer(raw_bytes, dtype=np.float32)
    else:
        data = np.frombuffer(raw_bytes, dtype=np.int16).astype(np.float32) / 32768.0

    if n_ch == 2:
        data = data.reshape(-1, 2)
        mono = (data[:, 0] + data[:, 1]) * 0.5
    else:
        mono = data

    if sr != target_sr:
        num_target = int(len(mono) * target_sr / sr)
        indices = np.linspace(0, len(mono) - 1, num_target)
        mono = np.interp(indices, np.arange(len(mono)), mono)
        sr = target_sr

    return mono, sr

def hz_to_midi(hz):
    if hz <= 0 or np.isnan(hz) or np.isinf(hz):
        return 0
    note = int(round(69 + 12 * np.log2(hz / 440.0)))
    return max(12, min(108, note))

def detect_band_onsets(audio, sr, min_freq=40.0, max_freq=120.0, hop_size=256, threshold_mult=1.3, min_gap_sec=0.10):
    frame_len = 1024
    freqs = np.fft.rfftfreq(frame_len, d=1.0/sr)
    min_bin = np.searchsorted(freqs, min_freq)
    max_bin = min(len(freqs) - 1, np.searchsorted(freqs, max_freq))

    energies = []
    times = []
    window = np.hanning(frame_len).astype(np.float32)

    for i in range(0, len(audio) - frame_len, hop_size):
        chunk = audio[i:i+frame_len] * window
        mag = np.abs(np.fft.rfft(chunk))
        band_e = np.sum(mag[min_bin:max_bin]**2)
        energies.append(np.sqrt(band_e + 1e-9))
        times.append(i / float(sr))

    energies = np.array(energies)
    times = np.array(times)
    if len(energies) < 3: return []

    flux = np.maximum(0, np.diff(energies, prepend=energies[0]))
    mean_f = np.mean(flux)
    std_f = np.std(flux)
    thresh = mean_f + std_f * 0.35 * threshold_mult

    onsets = []
    min_dist_frames = int(min_gap_sec * sr / hop_size)
    last_f = -min_dist_frames

    for f in range(1, len(flux) - 1):
        if flux[f] > thresh and flux[f] > flux[f-1] and flux[f] >= flux[f+1]:
            if (f - last_f) >= min_dist_frames and energies[f] > (np.max(energies) * 0.12):
                vel = min(1.0, max(0.5, energies[f] / (np.max(energies) + 1e-6)))
                onsets.append((times[f], float(vel)))
                last_f = f

    return onsets

def polyphonic_spectral_pitch_track(audio, sr, min_freq=60.0, max_freq=2200.0, max_polyphony=4, hop_size=256, min_dur=0.08):
    frame_len = 2048
    window = np.hanning(frame_len).astype(np.float32)
    freqs = np.fft.rfftfreq(frame_len, d=1.0/sr)
    min_bin = np.searchsorted(freqs, min_freq)
    max_bin = min(len(freqs) - 1, np.searchsorted(freqs, max_freq))

    events = []
    global_max_rms = np.sqrt(np.mean(audio**2)) * 3.0
    silence_thresh = max(0.002, global_max_rms * 0.025)
    active_notes = {}

    for pos in range(0, len(audio) - frame_len, hop_size):
        cur_time = pos / float(sr)
        chunk = audio[pos : pos + frame_len] * window
        rms = np.sqrt(np.mean(chunk**2))
        if rms < silence_thresh:
            for pitch, info in list(active_notes.items()):
                dur = cur_time - info["start_time"]
                if dur >= min_dur:
                    events.append({
                        "pitch": pitch,
                        "start_time": round(float(info["start_time"]), 3),
                        "duration": round(float(dur), 3),
                        "velocity": round(float(info["velocity"]), 2)
                    })
                del active_notes[pitch]
            continue
            
        mag = np.abs(np.fft.rfft(chunk))[min_bin:max_bin]
        cur_freqs = freqs[min_bin:max_bin]
        if len(mag) < 5: continue
            
        peak_indices = []
        mag_thresh = np.max(mag) * 0.20
        for i in range(1, len(mag) - 1):
            if mag[i] > mag[i-1] and mag[i] >= mag[i+1] and mag[i] > mag_thresh:
                peak_indices.append(i)
                
        peak_indices.sort(key=lambda idx: mag[idx], reverse=True)
        top_peaks = peak_indices[:max_polyphony]
        
        current_pitches = set()
        for p_idx in top_peaks:
            f = cur_freqs[p_idx]
            p = hz_to_midi(f)
            if p > 0:
                current_pitches.add(p)
                if p not in active_notes:
                    vel = min(1.0, max(0.4, (mag[p_idx] / (np.max(mag) + 1e-6)) * 0.9))
                    active_notes[p] = {"start_time": cur_time, "velocity": vel}

        for pitch, info in list(active_notes.items()):
            if pitch not in current_pitches:
                dur = cur_time - info["start_time"]
                if dur >= min_dur:
                    events.append({
                        "pitch": pitch,
                        "start_time": round(float(info["start_time"]), 3),
                        "duration": round(float(dur), 3),
                        "velocity": round(float(info["velocity"]), 2)
                    })
                del active_notes[pitch]

    total_len_time = len(audio) / float(sr)
    for pitch, info in active_notes.items():
        dur = total_len_time - info["start_time"]
        if dur >= min_dur:
            events.append({
                "pitch": pitch,
                "start_time": round(float(info["start_time"]), 3),
                "duration": round(float(dur), 3),
                "velocity": round(float(info["velocity"]), 2)
            })

    return events

def transcribe_audio_to_midi(target_path):
    print(f"[TRANSCRIBER 16-TRACKS] Transcrição Cirúrgica Granular 1-para-1: {target_path}", flush=True)
    all_channel_notes = { f"channel_{i}": [] for i in range(16) }
    out_json = os.path.splitext(target_path)[0] + "_transcription.json"
    audio, sr = load_wav_mono(target_path, target_sr=22050)

    # 1. Pista 0: Bumbo Principal (Psy Kick Click & Sub-Punch) - 40Hz a 120Hz
    print(" -> 01. Bumbo Principal (Kick)...", flush=True)
    kick_onsets = detect_band_onsets(audio, sr, min_freq=40.0, max_freq=120.0, min_gap_sec=0.35, threshold_mult=1.4)
    for t, vel in kick_onsets:
        all_channel_notes["channel_0"].append({
            "pitch": 48, "start_time": round(float(t), 3), "duration": 0.180, "velocity": round(float(vel), 2), "pan": 1.0, "channel": 0
        })

    # 2. Pista 1: Sub Bass Puro (Sub-Grave 35Hz a 75Hz)
    print(" -> 02. Sub-Grave (Sub Bass)...", flush=True)
    sub_events = polyphonic_spectral_pitch_track(audio, sr, min_freq=35.0, max_freq=75.0, max_polyphony=1, hop_size=256, min_dur=0.10)
    for ev in sub_events:
        ev["channel"] = 1; ev["pan"] = 1.0
        all_channel_notes["channel_1"].append(ev)

    # 3. Pista 2: Mid Saw Bass (Rolling Groove Serrilhado 75Hz a 280Hz)
    print(" -> 03. Mid Rolling Bass...", flush=True)
    mid_bass_events = polyphonic_spectral_pitch_track(audio, sr, min_freq=75.0, max_freq=280.0, max_polyphony=1, hop_size=256, min_dur=0.08)
    for ev in mid_bass_events:
        ev["channel"] = 2; ev["pan"] = 1.0
        all_channel_notes["channel_2"].append(ev)

    # 4. Pista 3: Caixa & Smash Clap (Snare / Clap 180Hz a 800Hz)
    print(" -> 04. Caixa & Claps (Snare)...", flush=True)
    snare_onsets = detect_band_onsets(audio, sr, min_freq=180.0, max_freq=800.0, min_gap_sec=0.35, threshold_mult=1.3)
    for t, vel in snare_onsets:
        # Se não coincidir exatamente no mesmo instante do kick puro
        all_channel_notes["channel_3"].append({
            "pitch": 52, "start_time": round(float(t), 3), "duration": 0.160, "velocity": round(float(vel), 2), "pan": 1.0, "channel": 3
        })

    # 5. Pista 4: Hi-Hat Aberto Contratempo (Offbeat Open Hat 3.5kHz a 8kHz)
    print(" -> 05. Chimbal Aberto Contratempo (Open Hat)...", flush=True)
    hat_onsets = detect_band_onsets(audio, sr, min_freq=3500.0, max_freq=8000.0, min_gap_sec=0.18, threshold_mult=1.2)
    for t, vel in hat_onsets:
        all_channel_notes["channel_4"].append({
            "pitch": 56, "start_time": round(float(t), 3), "duration": 0.120, "velocity": round(float(vel), 2), "pan": 1.0, "channel": 4
        })

    # 6. Pista 5: Shaker & Chimbal Fechado (Closed Hats / Shaker 8kHz a 14kHz)
    print(" -> 06. Shaker & Chimbal Fechado (Closed Hats)...", flush=True)
    closed_onsets = detect_band_onsets(audio, sr, min_freq=8000.0, max_freq=14000.0, min_gap_sec=0.08, threshold_mult=1.1)
    for t, vel in closed_onsets:
        all_channel_notes["channel_5"].append({
            "pitch": 58, "start_time": round(float(t), 3), "duration": 0.080, "velocity": round(float(vel * 0.8), 2), "pan": 1.0, "channel": 5
        })

    # 7. Pista 6: Percussão Tribal & Toms (Tribal Perc 250Hz a 1200Hz)
    print(" -> 07. Percussão Tribal...", flush=True)
    perc_onsets = detect_band_onsets(audio, sr, min_freq=250.0, max_freq=1200.0, min_gap_sec=0.12, threshold_mult=1.4)
    for t, vel in perc_onsets:
        all_channel_notes["channel_6"].append({
            "pitch": 60, "start_time": round(float(t), 3), "duration": 0.140, "velocity": round(float(vel), 2), "pan": 1.0, "channel": 6
        })

    # 8. Pista 7: FM Squelch & Lasers Zaps (Psy FX 1.5kHz a 4.5kHz)
    print(" -> 08. FM Squelch & Lasers...", flush=True)
    squelch_events = polyphonic_spectral_pitch_track(audio, sr, min_freq=1500.0, max_freq=4500.0, max_polyphony=2, hop_size=256, min_dur=0.06)
    for ev in squelch_events:
        ev["channel"] = 7; ev["pan"] = 1.0
        all_channel_notes["channel_7"].append(ev)

    # 9. Pista 8: Psy Lead Melodia Principal (Main Acid Lead 350Hz a 1800Hz)
    print(" -> 09. Melodia Principal Lead...", flush=True)
    lead_events = polyphonic_spectral_pitch_track(audio, sr, min_freq=350.0, max_freq=1800.0, max_polyphony=2, hop_size=256, min_dur=0.10)
    for ev in lead_events:
        ev["channel"] = 8; ev["pan"] = 1.0
        all_channel_notes["channel_8"].append(ev)

    # 10. Pista 9: Counter-Arp Sintetizador (Pluck Arps 500Hz a 2500Hz)
    print(" -> 10. Arpejo de Sintetizador...", flush=True)
    arp_events = polyphonic_spectral_pitch_track(audio, sr, min_freq=500.0, max_freq=2500.0, max_polyphony=2, hop_size=256, min_dur=0.07)
    for ev in arp_events:
        ev["channel"] = 9; ev["pan"] = 1.0
        all_channel_notes["channel_9"].append(ev)

    # 11. Pista 10: Corais & Pads Atmosféricos (Atmosphere Chords 200Hz a 2000Hz)
    print(" -> 11. Corais & Pads Harmônicos...", flush=True)
    chord_events = polyphonic_spectral_pitch_track(audio, sr, min_freq=200.0, max_freq=2000.0, max_polyphony=4, hop_size=256, min_dur=0.30)
    for ev in chord_events:
        ev["channel"] = 10; ev["pan"] = 1.0
        all_channel_notes["channel_10"].append(ev)

    # 12. Pista 11: Risers & Transições (Build-Up FX)
    print(" -> 12. Risers & Transições FX...", flush=True)
    riser_events = polyphonic_spectral_pitch_track(audio, sr, min_freq=800.0, max_freq=6000.0, max_polyphony=1, hop_size=256, min_dur=0.50)
    for ev in riser_events:
        ev["channel"] = 11; ev["pan"] = 1.0
        all_channel_notes["channel_11"].append(ev)

    with open(out_json, 'w', encoding='utf-8') as f:
        json.dump(all_channel_notes, f, indent=2)
    print(f"[TRANSCRIBER 16-TRACKS SUCCESS] Salvo em: {out_json}", flush=True)
    return out_json

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python audio_to_midi_transcriber.py <arquivo.wav>", flush=True)
        sys.exit(1)
    transcribe_audio_to_midi(sys.argv[1])
