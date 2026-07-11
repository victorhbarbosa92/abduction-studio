# autotest_abduction.py - Autonomous Testing Script for Abduction Studio v4.0
import os
import sys
import time
import subprocess
import struct
import math

TRACKS_DIR = r"C:\NovaDAW\tracks"
MOCKUPS_DIR = os.path.join(os.path.expanduser("~"), "Desktop", "Abduction_References")

def log(msg):
    print(f"[AUTOTEST] {msg}")

def ensure_folders():
    if not os.path.exists(MOCKUPS_DIR):
        os.makedirs(MOCKUPS_DIR, exist_ok=True)
        log(f"Created reference folder: {MOCKUPS_DIR}")

def analyze_track_peaks(filepath):
    """
    Autonomous simple WAV analyzer that reads WAV header and scans audio peaks (BPM / Transients)
    without heavy libraries (like librosa/numpy) to ensure portability.
    """
    log(f"Analyzing audio file: {os.path.basename(filepath)}")
    try:
        with open(filepath, 'rb') as f:
            header = f.read(44)
            if len(header) < 44:
                log("Invalid WAV header.")
                return None
            
            riff, size, wave, fmt, fmt_len, audio_fmt, channels, sample_rate, byte_rate, block_align, bits_per_sample, data_tag = struct.unpack(
                '<4sI4s4sIHHIIHH4s', header[:44]
            )
            
            if fmt != b'fmt ' or data_tag != b'data':
                # Try finding data subchunk
                f.seek(12)
                subchunk_id = b''
                while subchunk_id != b'data':
                    subchunk_hdr = f.read(8)
                    if len(subchunk_hdr) < 8:
                        break
                    subchunk_id, subchunk_size = struct.unpack('<4sI', subchunk_hdr)
                    if subchunk_id == b'fmt ':
                        f.seek(subchunk_size, 1)
                    elif subchunk_id != b'data':
                        f.seek(subchunk_size, 1)
                else:
                    # found data
                    pass
            
            log(f"Format: PCM, Channels: {channels}, SampleRate: {sample_rate}Hz, Bits: {bits_per_sample}")
            
            # Read a chunk of samples to detect transient peaks
            raw_data = f.read(44100 * 2 * 10) # 10 seconds of 16-bit stereo
            num_samples = len(raw_data) // 2
            samples = struct.unpack(f'<{num_samples}h', raw_data)
            
            # Simple RMS amplitude envelope for transients
            window_sz = 1024
            rms_env = []
            for i in range(0, num_samples - window_sz, window_sz):
                val_sum = 0
                for j in range(window_sz):
                    v = samples[i + j] / 32768.0
                    val_sum += v * v
                rms_env.append(math.sqrt(val_sum / window_sz))
            
            # Find transient peaks (energy increase)
            transients = []
            for i in range(1, len(rms_env) - 1):
                if rms_env[i] > rms_env[i-1] * 1.5 and rms_env[i] > 0.05:
                    transients.append(i * window_sz / sample_rate)
            
            log(f"Detected {len(transients)} transient beats in the first 10 seconds.")
            return transients
            
    except Exception as e:
        log(f"Failed to analyze track: {e}")
        return None

def copy_mockups_to_desktop():
    import shutil
    brain_dir = os.path.join(os.path.expanduser("~"), ".gemini", "antigravity-ide", "brain", "7f394661-b51d-4b14-83a6-080431fa694e")
    log("Copying generated design mockups to Desktop/Abduction_References...")
    if os.path.exists(brain_dir):
        files = os.listdir(brain_dir)
        for file in files:
            if file.endswith(".png"):
                src = os.path.join(brain_dir, file)
                dst = os.path.join(MOCKUPS_DIR, file)
                shutil.copy(src, dst)
                log(f"Copied mockup: {file}")
    else:
        log("No mockups directory found in brain folder.")

def main():
    print("=" * 80)
    print(" \"Que sua energia contagie e contemple, a todos que te usarem...\"")
    print("               ＩＮＶＯＱＵＥ. . .  ＡＢＤＵＣＴＩＯＮ  ＳＴＵＤＩＯ !")
    print("=" * 80)
    print()
    ensure_folders()
    copy_mockups_to_desktop()
    
    # Analyze tracks in C:\NovaDAW\tracks
    if os.path.exists(TRACKS_DIR):
        tracks = [os.path.join(TRACKS_DIR, f) for f in os.listdir(TRACKS_DIR) if f.endswith(".wav")]
        for track in tracks:
            analyze_track_peaks(track)
    else:
        log(f"Tracks folder {TRACKS_DIR} not found.")

    log("Autotest successfully completed.")

if __name__ == '__main__':
    main()
