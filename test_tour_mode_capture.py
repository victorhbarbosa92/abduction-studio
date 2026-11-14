import time
import subprocess
import os
import tempfile
from PIL import Image

current_conv_id = "b1a74557-b12f-4341-b15f-052f54b66685"
artifact_dir = os.path.join(r"C:\Users\o_vic\.gemini\antigravity\brain", current_conv_id)
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
        print(f"[TEST] Screenshot salva com sucesso: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    print("[TEST] Falha ao capturar screenshot.")
    return None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(1.0)

    work_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"
    exe_path = os.path.join(work_dir, "AbductionStudioV2.exe")

    print(f"[TEST] Iniciando {exe_path} --test-tour-mode...")
    proc = subprocess.Popen([exe_path, "--test-tour-mode"], cwd=work_dir)
    time.sleep(6.5)

    print("[TEST] Disparando captura de tela do Guia Interativo...")
    shot = trigger_and_save("abduction_tour_guide_live.png")
    
    time.sleep(1.0)
    try:
        proc.terminate()
        proc.wait(timeout=3.0)
    except:
        proc.kill()

if __name__ == "__main__":
    main()

