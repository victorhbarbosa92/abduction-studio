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
        print(f"[TEST] Screenshot salva: {png_path}")
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

hwnd = user32.GetForegroundWindow()
rect = wintypes.RECT()
user32.GetWindowRect(hwnd, ctypes.byref(rect))
print(f"[TEST] Window Rect: left={rect.left}, top={rect.top}, right={rect.right}, bottom={rect.bottom}")

# A barra superior do ImGui está dentro da janela do cliente.
# O botão 'Psy Bass' fica em torno de x = rect.left + 740, y = rect.top + 38
print("[TEST] Clicando no botao 'Psy Bass'...")
mouse_click(rect.left + 740, rect.top + 38)
time.sleep(1.0)
shot1 = trigger_and_save("08_psy_bass_window_opened.png")

# Clicar no botão 'Piano' (x = rect.left + 520, y = rect.top + 38)
print("[TEST] Clicando no botao 'Piano'...")
mouse_click(rect.left + 520, rect.top + 38)
time.sleep(1.0)
shot2 = trigger_and_save("09_piano_roll_opened.png")

print("[TEST] Finalizado!")
