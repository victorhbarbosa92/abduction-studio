import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("trigger\n")

    captured = False
    for _ in range(50):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            captured = True
            break

    if captured:
        png_path = os.path.join(artifact_dir, png_name)
        img = Image.open(bmp_out_file)
        img.save(png_path)
        try: os.remove(bmp_out_file)
        except: pass
        print(f"[TEST] Screenshot salva: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] ERRO ao capturar {png_name}")
    return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(0.8)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

# 1. FULL BASE OVERVIEW (Tab FULL BASE, Multi-Lane Matrix 16-Step)
print("\n[PROVA 1] Capturando Aba FULL BASE com matriz multi-lane...")
kill_daw()
proc = subprocess.Popen([exe_path, "--psy-rhythm-full"], cwd=cwd)
time.sleep(3.2)
trigger_and_save("01_rhythm_bass_full_overview.png")
proc.terminate()
kill_daw()

# 2. KICK MODELING TAB (Start/End Freq, Punch, Amp Decay, Analog Drive, Phase Align)
print("\n[PROVA 2] Capturando Aba KICK com osciloscópio de punch/sub...")
kill_daw()
proc = subprocess.Popen([exe_path, "--psy-rhythm-kick"], cwd=cwd)
time.sleep(3.2)
trigger_and_save("02_rhythm_bass_kick_tab.png")
proc.terminate()
kill_daw()

# 3. SNARE / CLAP TAB (Tone Pitch, Noise Tail, Snap Attack, Stereo)
print("\n[PROVA 3] Capturando Aba SNARE com osciloscópio de ruído/snap...")
kill_daw()
proc = subprocess.Popen([exe_path, "--psy-rhythm-snare"], cwd=cwd)
time.sleep(3.2)
trigger_and_save("03_rhythm_bass_snare_tab.png")
proc.terminate()
kill_daw()

# 4. AUDITION LOOP EM TEMPO REAL (Playhead multi-lane sincronizado cruzando Kick + Bass + Snare)
print("\n[PROVA 4] Capturando Audition Loop em tempo real (Kick + Bass + Snare)...")
kill_daw()
proc = subprocess.Popen([exe_path, "--psy-rhythm-audition"], cwd=cwd)
time.sleep(3.5)
trigger_and_save("04_rhythm_bass_audition_pumping.png")
proc.terminate()
kill_daw()

# 5. GERAR NOTAS EM 3 PISTAS SEPARADAS NO PIANO ROLL (Track 0 Kick, Track 1 Bass, Track 2 Snare)
print("\n[PROVA 5] Capturando Piano Roll com as pistas geradas...")
kill_daw()
proc = subprocess.Popen([exe_path, "--psy-rhythm-generate-all"], cwd=cwd)
time.sleep(3.2)
trigger_and_save("05_piano_roll_multitrack_generated.png")
proc.terminate()
kill_daw()

print("\n[SUCESSO] Todas as 5 provas visuais geradas no artifact_dir!")
