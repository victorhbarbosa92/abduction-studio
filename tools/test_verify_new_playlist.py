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
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\611edc90-9ec3-4e84-b411-7247e0205a46"
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
    for _ in range(70):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            captured = True
            break

    if captured:
        time.sleep(0.1)
        out_png = os.path.join(artifact_dir, png_name)
        with Image.open(bmp_out_file) as im:
            im.save(out_png, "PNG")
        try: os.remove(bmp_out_file)
        except: pass
        print(f"[SCREENSHOT OK] {png_name} salvo com sucesso ({out_png}) [{im.size[0]}x{im.size[1]}]")
        return out_png
    else:
        print(f"[ERRO] Falha ao capturar {png_name}")
        return None

def kill_daw():
    subprocess.run(['powershell', '-Command', "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(0.8)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("[TEST] Iniciando verificacao da Nova Playlist com design aprovado...")

# PROVA 1: Arranjo Psytrance Completo na Playlist Redesenhada
print("\n[PROVA 1] Executando DAW com --test-song-arranger...")
kill_daw()
proc = subprocess.Popen([exe_path, "--test-song-arranger"], cwd=cwd)
time.sleep(4.0)
trigger_and_save("01_playlist_full_psytrance_arrangement.png")
proc.terminate()
kill_daw()

# PROVA 2: Visualizacao de Curvas Bézier de Automacao
print("\n[PROVA 2] Executando DAW com --test-playlist-bezier...")
kill_daw()
proc = subprocess.Popen([exe_path, "--test-playlist-bezier"], cwd=cwd)
time.sleep(4.0)
trigger_and_save("02_playlist_bezier_automation_tracks.png")
proc.terminate()
kill_daw()

# PROVA 3: Modal do Arranjador Psytrance Integrado sobre a Playlist
print("\n[PROVA 3] Executando DAW com --test-song-arranger-modal...")
kill_daw()
proc = subprocess.Popen([exe_path, "--test-song-arranger-modal"], cwd=cwd)
time.sleep(4.0)
trigger_and_save("03_playlist_psytrance_arranger_modal.png")
proc.terminate()
kill_daw()

print("\n[TEST CONCLUIDO] Todas as provas da nova Playlist foram executadas!")
