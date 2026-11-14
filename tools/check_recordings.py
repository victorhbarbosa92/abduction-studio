import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\9355a9bb-0b73-4db6-a5fa-c7caeed07313"
temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    captured = False
    for i in range(80):
        if not os.path.exists(trigger_file) and not os.path.exists(bmp_out_file):
            try:
                with open(trigger_file, "w") as f:
                    f.write("trigger\n")
            except: pass
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
        print(f"[TEST] Screenshot salvo: {png_name}")
        return png_path
    return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(0.8)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
cwd = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("\n=== VALIDAÇÃO DO MODAL DE GRAVAÇÕES MASTER ===")
kill_daw()

# Inicia com flag normal de refined/phase e abre o modal via flag ou clique
# Vamos verificar se o arquivo gravado Live_Set_20260913_*.wav está lá
rec_files = os.listdir(os.path.join(cwd, "scratch", "recordings"))
print(f"Total de gravações no disco: {len(rec_files)}")
for f in rec_files:
    print(f" - {f}")
