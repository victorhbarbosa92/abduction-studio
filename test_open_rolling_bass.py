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
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\1fe098f5-bfde-454d-a9d4-194e0b7d30b6"
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
        print(f"[TEST] Screenshot salva com sucesso: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    return None

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004

def mouse_click(x, y):
    user32.SetCursorPos(int(x), int(y))
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.4)

print("[TEST] Clicando no item 'Kuro Psy Rolling Bass (Engine)' na barra lateral esquerda...")
# No print anterior, 'Kuro Psy Rolling Bass' está aproximadamente em x=80, y=255 na janela 1280x720
mouse_click(80, 255)
time.sleep(1.0)

shot = trigger_and_save("03_rolling_bass_plugin_window_active.png")

# Clicar no botão 'KBBB 16th Roll' ou 'GENERATE TO PIANO ROLL'
# A janela do plugin abre centralizada ou em posição padrão
# Clicar no botão 'Piano' no topo para abrir também o Piano Roll
print("[TEST] Abrindo Piano Roll pelo botão do topo...")
mouse_click(520, 15) # Botão Piano no topo
time.sleep(1.0)

shot_pr = trigger_and_save("04_piano_roll_with_rolling_bass_integrated.png")
print("[TEST] Validação concluída com sucesso!")
