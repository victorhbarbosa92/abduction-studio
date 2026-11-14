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
    else:
        print("[TEST] Falha ao capturar screenshot pelo trigger file.")
        return None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.5)

    exe_path = "AbductionStudioV2.exe"
    
    # 1. Teste Dock Direita
    print(f"[TEST 1/2] Iniciando {exe_path} com --test-dj-drawer (Dock Direita)...")
    proc1 = subprocess.Popen([exe_path, "--test-dj-drawer"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(4.5)
    shot_right = trigger_and_save("dj_side_drawer_dock_right.png")
    trigger_and_save("dj_side_drawer_live_verified.png")
    time.sleep(1.0)
    proc1.terminate()
    try: proc1.wait(timeout=3)
    except: proc1.kill()

    time.sleep(1.0)

    # 2. Teste Dock Esquerda
    print(f"[TEST 2/2] Iniciando {exe_path} com --test-dock-left (Dock Esquerda)...")
    proc2 = subprocess.Popen([exe_path, "--test-dock-left"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(4.5)
    shot_left = trigger_and_save("dj_side_drawer_dock_left.png")
    time.sleep(1.0)
    proc2.terminate()
    try: proc2.wait(timeout=3)
    except: proc2.kill()

    if shot_right and shot_left:
        print(f"[TEST] SUCESSO: Ambas as posições capturadas com sucesso!")
        sys.exit(0)
    else:
        print("[TEST] ERRO: Falha ao gerar capturas.")
        sys.exit(1)

if __name__ == "__main__":
    main()
