import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\526b0b72-1496-49be-bdae-afaa34d152bf"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_screen_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_screen_file, "w") as f:
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
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    else:
        print(f"[TEST] Falha ao capturar screenshot: {png_name}")
    return None

print("[TEST] 1. Encerrando instancias...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
print(f"[TEST] 2. Iniciando DAW: {exe_path}")
proc = subprocess.Popen([exe_path, "--test-preview-audio"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(2.0)

# Fecha o Home Dashboard se estiver aberto
hh_cmd_file = os.path.join(temp_dir, "abduction_home_hub_cmd.txt")
with open(hh_cmd_file, "w") as f:
    f.write("close\n")
time.sleep(1.5)

try:
    print(f"[TEST] 3. PID da DAW: {proc.pid}")
    shot = trigger_and_save("abduction_hub_faithful_ui.png")
finally:
    print("[TEST] 4. Encerrando DAW de teste...")
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
