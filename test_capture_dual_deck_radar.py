import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\d6b51782-3eec-4b8b-b44e-8765b2612319"
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
    return None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.5)

    exe_path = r"build\Release\AbductionStudioV2.exe"
    if not os.path.exists(exe_path):
        exe_path = "AbductionStudioV2.exe"

    print(f"[TEST] Iniciando {exe_path} com --test-hover-radar...")
    proc = subprocess.Popen([exe_path, "--test-hover-radar"], cwd=".")
    time.sleep(4.5)

    print("[TEST] Capturando screenshot do Feixe Harmônico e Previsão de Encaixe...")
    shot = trigger_and_save("dj_hover_beam_verified.png")

    time.sleep(1.0)
    print("[TEST] Encerrando AbductionStudioV2...")
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except:
        os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")

    if shot and os.path.exists(shot):
        print("[SUCCESS] Teste concluido com sucesso!")
        sys.exit(0)
    else:
        print("[FAILED] Nao foi possivel capturar a screenshot.")
        sys.exit(1)

if __name__ == "__main__":
    main()
