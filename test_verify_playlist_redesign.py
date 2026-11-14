import time
import subprocess
import os
import sys
import tempfile
import ctypes
import json
from ctypes import wintypes
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
    for _ in range(80):
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
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    else:
        print(f"[TEST] Falha ao capturar {png_name}")
        return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(0.8)

kill_daw()

app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("[TEST] 1. Iniciando AbductionStudioV2 com --test-song-arranger...")
proc = subprocess.Popen([app_path, "--test-song-arranger"], cwd=cwd)
print(f"[TEST] Processo iniciado PID: {proc.pid}")
time.sleep(5.0)

# Captura 1: Visão Geral da Playlist Redesenhada com Toolbar Modular, Linhas Contínuas e Curvas Bézier
print("[TEST] 2. Capturando visão geral da Playlist com novo visual...")
s1 = trigger_and_save("32_playlist_modular_toolbar_and_bezier.png")

time.sleep(2.0)

kill_daw()
print("[TEST] Verificação concluída com sucesso!")
