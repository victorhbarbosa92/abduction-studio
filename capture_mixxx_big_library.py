import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\725a1335-8fa2-4938-9595-4adffb7b22cd"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_filename):
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
        img = Image.open(bmp_out_file)
        dest_path = os.path.join(artifact_dir, png_filename)
        img.save(dest_path)
        print(f"[TEST] Screenshot salva com sucesso: {dest_path} ({img.size[0]}x{img.size[1]})")
        try: os.remove(bmp_out_file)
        except: pass
        return dest_path
    else:
        print(f"[TEST] Timeout aguardando captura para {png_filename}")
        return None

def run_test(flag, png_filename):
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.8)

    print(f"\n[TEST] Iniciando AbductionStudioV2.exe com {flag}...")
    proc = subprocess.Popen(["AbductionStudioV2.exe", flag], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(4.5)

    print(f"[TEST] Disparando captura de tela para {png_filename}...")
    shot = trigger_and_save(png_filename)

    time.sleep(1.0)
    print("[TEST] Encerrando AbductionStudioV2.exe...")
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except:
        proc.kill()
    time.sleep(0.5)
    return shot

if __name__ == "__main__":
    shot1 = run_test("--test-big-library", "abduction_mixxx_big_library.png")
    shot2 = run_test("--test-split-library", "abduction_mixxx_split_library.png")
    shot3 = run_test("--test-performance-mode", "abduction_mixxx_performance_mode.png")
    print(f"\n[TEST RESULTS]\n1. Big Library: {shot1}\n2. Split Library: {shot2}\n3. Performance Mode: {shot3}")
