import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\99c43430-c66d-4980-bd72-557c18180e32"
temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("trigger\n")

    for _ in range(80):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            png_path = os.path.join(artifact_dir, png_name)
            img = Image.open(bmp_out_file)
            img.save(png_path)
            try: os.remove(bmp_out_file)
            except: pass
            print(f"[TEST] Salvo: {png_path}")
            return png_path
    return None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.5)

    exe_path = r"build\Release\AbductionStudioV2.exe"
    print(f"[TEST] Iniciando {exe_path} com --test-crates...")
    proc = subprocess.Popen([exe_path, "--test-crates"], cwd=".")
    time.sleep(4.5)

    shot = trigger_and_save("dj_phase3_crates_verified.png")

    time.sleep(1.0)
    proc.terminate()
    try: proc.wait(timeout=3)
    except: os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")

if __name__ == "__main__":
    main()
