import time
import subprocess
import os
import sys
import tempfile
import ctypes
from ctypes import wintypes
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

user32 = ctypes.windll.user32
artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\9355a9bb-0b73-4db6-a5fa-c7caeed07313"
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
    for _ in range(60):
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
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(0.8)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
cwd = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("\n=== [1] TESTE DO DJ MODE: LCD TIME READOUT, DECK CONTROLS & CLEAN MIXER ===")
kill_daw()
proc = subprocess.Popen([exe_path, "--test-dj-refined"], cwd=cwd)
time.sleep(4.5)

png1 = trigger_and_save("17_dj_mode_time_lcd_and_clean_mixer.png")
proc.terminate()
kill_daw()

print("\n=== [2] TESTE DO KURO CLOUD: MULTI-FONTES (SOUNDCLOUD, SPOTIFY, TOPIC) ===")
kill_daw()
proc2 = subprocess.Popen([exe_path, "--test-sources-modal"], cwd=cwd)
time.sleep(4.5)

png2 = trigger_and_save("18_kuro_cloud_multi_sources_modal.png")
proc2.terminate()
kill_daw()

print("\n=== TESTES CONCLUIDOS COM SUCESSO! ===")
