import time
import subprocess
import os
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity\brain\285bbba1-4c01-40d8-9f9a-4fec4dd20f77"
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
    print("[TEST] Falha ao capturar screenshot.")
    return None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(1.0)

    work_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"
    exe_path = os.path.join(work_dir, "AbductionStudioV2.exe")

    print(f"[TEST] Iniciando {exe_path} --test-split-library...")
    proc = subprocess.Popen([exe_path, "--test-split-library"], cwd=work_dir)
    time.sleep(5.0)

    print("[TEST] Disparando captura de tela em 4K...")
    shot = trigger_and_save("harmonized_4k_result.png")
    
    time.sleep(1.0)
    try:
        proc.terminate()
        proc.wait(timeout=3.0)
    except:
        proc.kill()
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")

if __name__ == "__main__":
    main()
