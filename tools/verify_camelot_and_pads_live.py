import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\7a8b599d-0d06-45da-bdfb-4e1e39dd08e9"
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
        print(f"[TEST] Screenshot salvo: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(0.8)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
cwd = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("\n=== VALIDAÇÃO VISUAL DO RADAR CAMELOT E SISTEMA DE MIXAGEM HARMÔNICA ===")

# TESTE 1: RADAR CAMELOT
print("\n[1] Testando Modal do Radar Camelot (--test-camelot-radar)...")
kill_daw()
proc1 = subprocess.Popen([exe_path, "--test-camelot-radar"], cwd=cwd)
time.sleep(4.5)
png1 = trigger_and_save("04_camelot_radar_modal_live.png")
proc1.terminate()
kill_daw()

# TESTE 2: CRATES COM CAMELOT SORT E BADGES
print("\n[2] Testando Crates com Camelot Badges e Sort (--test-dj-crates)...")
proc2 = subprocess.Popen([exe_path, "--test-dj-crates"], cwd=cwd)
time.sleep(4.5)
png2 = trigger_and_save("05_camelot_crates_badges_live.png")
proc2.terminate()
kill_daw()

# TESTE 3: SEARCH MODAL COM CAMELOT CODES
print("\n[3] Testando Search Modal com Camelot Codes (--test-search-modal)...")
proc3 = subprocess.Popen([exe_path, "--test-search-modal"], cwd=cwd)
time.sleep(4.5)
png3 = trigger_and_save("06_search_modal_camelot_badges_live.png")
proc3.terminate()
kill_daw()

print("\n=== RESULTADOS DA VALIDAÇÃO VISUAL ===")
if png1: print(f" - Print 1 (Radar Camelot): {png1}")
if png2: print(f" - Print 2 (Crates com Badges): {png2}")
if png3: print(f" - Print 3 (Busca com Camelot): {png3}")
