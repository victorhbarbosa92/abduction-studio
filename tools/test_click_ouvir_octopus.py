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

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004

def mouse_click(sx, sy):
    user32.SetCursorPos(int(sx), int(sy))
    time.sleep(0.15)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.3)

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(0.8)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
cwd = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("\n=== TESTE INTERATIVO DE CLIQUE: OUVIR TRACK REMOTA (OCTOPUS) ===")
kill_daw()

print(f"[1] Iniciando {exe_path} com --test-search-modal...")
proc = subprocess.Popen([exe_path, "--test-search-modal"], cwd=cwd)
time.sleep(3.5)

hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
if hwnd:
    user32.ShowWindow(hwnd, 3) # SW_MAXIMIZE
    user32.SetForegroundWindow(hwnd)
    time.sleep(1.0)
    print("[2] Janela maximizada em primeiro plano.")

# Clica no botão OUVIR da linha 1 (Octopus) em (1320, 255)
print("[3] Clicando no botao [ OUVIR ] da faixa Octopus em (1320, 255)...")
mouse_click(1320, 255)

# Aguarda download e início da reprodução (3 segundos)
print("[4] Aguardando download e decodificacao PCM...")
time.sleep(4.0)

# Captura screenshot da reprodução
png = trigger_and_save("03_preview_player_remote_itunes_playing.png")

time.sleep(2.0)
proc.terminate()
kill_daw()

print(f"[SUCESSO] Teste concluído: {png}")
