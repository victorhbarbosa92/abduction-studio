import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"
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
    return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(0.8)

kill_daw()

app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

# Iniciar DAW
proc = subprocess.Popen([app_path, "--test-song-arranger"], cwd=cwd)
time.sleep(4.0)

# Chamar API ou injetar scroll na playlist para ver as faixas de automação e as curvas Bézier
# Vamos simular um clique ou scroll via script de teste
import urllib.request
import json

# Disparar screenshot da automação
s2 = trigger_and_save("33_playlist_automation_bezier_detail.png")

kill_daw()
print("[TEST] Concluído!")
