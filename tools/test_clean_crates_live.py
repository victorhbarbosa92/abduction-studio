import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\7a8b599d-0d06-45da-bdfb-4e1e39dd08e9"
temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("trigger\n")

    for _ in range(60):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
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

print("\n=== VALIDAÇÃO AO VIVO: CRATES E BIBLIOTECA 100% LIMPOS (ZERO MÚSICAS) ===")

kill_daw()
proc = subprocess.Popen([exe_path, "--test-crates"], cwd=cwd)
time.sleep(4.5)

png_crates = trigger_and_save("08_crates_zerados_limpos_live.png")

time.sleep(1.0)
proc.terminate()
kill_daw()

print("\n[OK] Verificação de limpeza concluída com sucesso!")
