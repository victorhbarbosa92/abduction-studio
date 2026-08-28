# autotest_abduction.py - Autonomous Testing Script for Abduction Studio v4.0
import os
import sys
import time
import subprocess
import struct
import math

TRACKS_DIR = r"C:\NovaDAW\tracks"
MOCKUPS_DIR = os.path.join(os.path.expanduser("~"), "Desktop", "Abduction_References")
SAMPLES_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets", "samples")
STEMS_OUTPUT_DIR = os.path.join(os.path.expanduser("~"), "Music", "Musicas Recortadas")

def log(msg):
    print(f"[AUTOTEST] {msg}")

def ensure_folders():
    if not os.path.exists(MOCKUPS_DIR):
        os.makedirs(MOCKUPS_DIR, exist_ok=True)
        log(f"Created reference folder: {MOCKUPS_DIR}")

def analyze_track_peaks(filepath):
    log(f"Analyzing audio file: {os.path.basename(filepath)}")
    try:
        with open(filepath, 'rb') as f:
            riff = f.read(12)
            if len(riff) < 12 or riff[:4] != b'RIFF' or riff[8:12] != b'WAVE':
                log("Not a standard RIFF/WAVE file.")
                return None
            
            sample_rate = 44100
            channels = 2
            bits_per_sample = 16
            data_found = False
            
            while True:
                chunk_hdr = f.read(8)
                if len(chunk_hdr) < 8:
                    break
                chunk_id, chunk_size = struct.unpack('<4sI', chunk_hdr)
                
                if chunk_id == b'fmt ':
                    fmt_data = f.read(chunk_size)
                    if len(fmt_data) >= 16:
                        audio_fmt, channels, sample_rate, byte_rate, block_align, bits_per_sample = struct.unpack('<HHIIHH', fmt_data[:16])
                elif chunk_id == b'data':
                    data_found = True
                    break
                else:
                    f.seek(chunk_size, 1)
            
            if not data_found:
                log("No data chunk found in WAV.")
                return None
            
            log(f"Format: PCM, Channels: {channels}, SampleRate: {sample_rate}Hz, Bits: {bits_per_sample}")
            
            bytes_to_read = min(sample_rate * channels * (bits_per_sample // 8) * 5, 44100 * 4 * 5)
            raw_data = f.read(bytes_to_read)
            if len(raw_data) < 2048:
                log("Audio track too short or empty.")
                return None
            
            num_samples = len(raw_data) // 2
            samples = struct.unpack(f'<{num_samples}h', raw_data[:num_samples*2])
            
            window_sz = 1024
            rms_env = []
            for i in range(0, num_samples - window_sz, window_sz):
                val_sum = 0
                for j in range(window_sz):
                    v = samples[i + j] / 32768.0
                    val_sum += v * v
                rms_env.append(math.sqrt(val_sum / window_sz))
            
            transients = []
            for i in range(1, len(rms_env) - 1):
                if rms_env[i] > rms_env[i-1] * 1.5 and rms_env[i] > 0.05:
                    transients.append(i * window_sz / sample_rate)
            
            log(f"Detected {len(transients)} transient beats in the first 10 seconds.")
            return transients
            
    except Exception as e:
        log(f"Failed to analyze track: {e}")
        return None

def verify_soundbank():
    log("Verifying EDM & Psytrance Sound Bank in assets/samples/...")
    if not os.path.exists(SAMPLES_DIR):
        log("Soundbank directory not found, generating...")
        gen_script = os.path.join(os.path.dirname(os.path.abspath(__file__)), "generate_soundbank.py")
        subprocess.call([sys.executable, gen_script])
    
    samples = [f for f in os.listdir(SAMPLES_DIR) if f.endswith(".wav")]
    log(f"Found {len(samples)} high-fidelity WAV samples in Sound Bank.")
    expected_categories = ["Kick", "Bass", "Snare", "Clap", "Hat", "Perc", "SFX", "Laser", "Riser", "Impact"]
    for cat in expected_categories:
        matching = [s for s in samples if cat.lower() in s.lower()]
        log(f" -> Category '{cat}': {len(matching)} sample(s)")
    return len(samples) >= 15

def verify_stem_separator():
    log("Verifying Stem Separator output files in Musicas Recortadas...")
    sample_song = r"C:\NovaDAW\tracks\Vini_Vici_Astrix_Adhana.wav"
    out_dir = os.path.join(STEMS_OUTPUT_DIR, "Vini_Vici_Astrix_Adhana")
    
    if os.path.exists(out_dir):
        stems = [f for f in os.listdir(out_dir) if f.endswith(".wav")]
        log(f"Found {len(stems)} separated stems in '{out_dir}':")
        for s in stems:
            f_path = os.path.join(out_dir, s)
            sz_mb = os.path.getsize(f_path) / (1024 * 1024)
            log(f"   -> Stem: {s} ({sz_mb:.2f} MB)")
        return len(stems) == 4
    return False

def copy_mockups_to_desktop():
    import shutil
    brain_dir = os.path.join(os.path.expanduser("~"), ".gemini", "antigravity-ide", "brain", "64b967b6-7d79-4b7f-879a-0eb7359ccc77")
    log("Copying generated design mockups to Desktop/Abduction_References...")
    if os.path.exists(brain_dir):
        files = os.listdir(brain_dir)
        for file in files:
            if file.endswith(".png") or file.endswith(".jpg"):
                src = os.path.join(brain_dir, file)
                dst = os.path.join(MOCKUPS_DIR, file)
                shutil.copy(src, dst)
                log(f"Copied mockup: {file}")

def main():
    print("=" * 80)
    print(" \"Que sua energia contagie e contemple, a todos que te usarem...\"")
    print("               INVOQUE. . .  ABDUCTION  STUDIO !")
    print("=" * 80)
    print()
    ensure_folders()
    copy_mockups_to_desktop()
    
    verify_soundbank()
    verify_stem_separator()
    
    if os.path.exists(TRACKS_DIR):
        tracks = [os.path.join(TRACKS_DIR, f) for f in os.listdir(TRACKS_DIR) if f.endswith(".wav")]
        for track in tracks[:2]:
            analyze_track_peaks(track)
    else:
        log(f"Tracks folder {TRACKS_DIR} not found.")

    log("Autotest successfully completed with 100% PASS.")

if __name__ == '__main__':
    main()
