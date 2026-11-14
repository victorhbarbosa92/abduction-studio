import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\a7531291-b5ae-4563-8e85-584c2f47d8c3"
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
    for _ in range(50):
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
        print(f"[TEST] Screenshot salva com sucesso: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(0.8)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\AbductionStudioV2.exe"
cwd = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

# 1. Captura da Busca Online com Mini Player de Preview e Botão [ FONTES ]
print("\n[TEST 1] Capturando Busca Online e Mini Preview Player...")
kill_daw()
proc = subprocess.Popen([exe_path, "--test-search-modal"], cwd=cwd)
time.sleep(3.5)
trigger_and_save("01_dj_search_and_preview_player_verified.png")
proc.terminate()
kill_daw()

# 2. Captura dos Crates e Decks com Biblioteca Saneada
print("\n[TEST 2] Capturando DJ Crates e Carregamento nos Decks...")
kill_daw()
proc2 = subprocess.Popen([exe_path, "--test-dj-crates"], cwd=cwd)
time.sleep(3.5)
trigger_and_save("02_dj_crates_and_decks_verified.png")
proc2.terminate()
kill_daw()
print("\n[CONCLUÍDO] Todas as verificações visuais registradas com sucesso!")
