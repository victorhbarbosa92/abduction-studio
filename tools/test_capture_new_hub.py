import os
import sys
import time
import subprocess
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity\brain\b9e05024-c94b-463e-ab5e-730f4a59ba8d"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
    if os.path.exists(bmp_file):
        try: os.remove(bmp_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("trigger\n")

    captured = False
    for _ in range(60):
        time.sleep(0.1)
        if os.path.exists(bmp_file) and os.path.getsize(bmp_file) > 1000:
            captured = True
            break

    if captured:
        png_path = os.path.join(artifact_dir, png_name)
        img = Image.open(bmp_file)
        img.save(png_path)
        try: os.remove(bmp_file)
        except: pass
        print(f"[TEST] Live Screenshot capturada com sucesso: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

# Kill any existing instance
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
time.sleep(1.0)

# Launch with --test-search-modal
exe_path = r"c:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\AbductionStudioV2.exe"
cwd = r"c:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("[TEST] Iniciando AbductionStudioV2 com --test-search-modal...")
proc = subprocess.Popen([exe_path, "--test-search-modal"], cwd=cwd)

time.sleep(4.0)

out = trigger_and_save("live_abduction_hub_split_dashboard.png")

time.sleep(0.5)
proc.terminate()
time.sleep(0.5)
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)

if out:
    print(f"[TEST] CONCLUÍDO: Imagem gerada em {out}")
else:
    print("[TEST] FALHA: Não foi possível obter o screenshot.")

