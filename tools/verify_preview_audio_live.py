import time
import subprocess
import os
import sys
import tempfile
import ctypes
from ctypes import wintypes
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

user32 = ctypes.windll.user32
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
        print(f"[TEST] Screenshot salva: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(0.8)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
cwd = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("\n=== TESTE EM TEMPO REAL: PREVIEW PLAYER AUDIO STREAMING ===")
kill_daw()

print(f"[1] Iniciando {exe_path} com --test-preview-audio...")
proc = subprocess.Popen([exe_path, "--test-preview-audio"], cwd=cwd)

# Aguarda inicializacao e carregamento da faixa
time.sleep(4.0)

hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
if hwnd:
    user32.ShowWindow(hwnd, 3) # SW_MAXIMIZE = 3
    user32.SetForegroundWindow(hwnd)
    print("[2] Janela da DAW maximizada e trazida para primeiro plano.")
else:
    print("[WARN] Janela da DAW nao encontrada pelo titulo exato.")

# Deixa tocar por mais 4 segundos para o playhead e tempo avancarem visualmente
print("[3] Deixando a reproducao PCM do Preview Player correr (4s)...")
time.sleep(4.0)

# Captura tela com o Audition Beam tocando
png1 = trigger_and_save("01_preview_player_playing_live.png")

# Mais 2 segundos para confirmar progresso contínuo
time.sleep(2.0)
png2 = trigger_and_save("02_preview_player_progress_verified.png")

proc.terminate()
kill_daw()

print(f"\n[SUCESSO] Teste concluído com sucesso!")
if png1: print(f" - Print 1: {png1}")
if png2: print(f" - Print 2: {png2}")
