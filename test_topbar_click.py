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

def smooth_move(target_x, target_y, duration=0.5, steps=25):
    point = wintypes.POINT()
    user32.GetCursorPos(ctypes.byref(point))
    start_x, start_y = point.x, point.y
    for i in range(1, steps + 1):
        t = i / steps
        smooth_t = t * t * (3.0 - 2.0 * t)
        curr_x = start_x + (target_x - start_x) * smooth_t
        curr_y = start_y + (target_y - start_y) * smooth_t
        user32.SetCursorPos(int(curr_x), int(curr_y))
        time.sleep(duration / steps)
    time.sleep(0.05)

def smooth_click(x, y, label=""):
    print(f"[TEST] Clicando em: {label} ({int(x)}, {int(y)})")
    smooth_move(x, y)
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.12)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.5)

# 1. Localizar janela
found_hwnd = None
def enum_cb(h, l):
    global found_hwnd
    buf = ctypes.create_unicode_buffer(512)
    user32.GetWindowTextW(h, buf, 512)
    if "Abduction Studio" in buf.value:
        found_hwnd = h
        return False
    return True

EnumProc = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
user32.EnumWindows(EnumProc(enum_cb), 0)

if not found_hwnd:
    exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
    subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(3.5)
    user32.EnumWindows(EnumProc(enum_cb), 0)

if found_hwnd:
    user32.ShowWindow(found_hwnd, 3) # SW_MAXIMIZE = 3
    user32.SetForegroundWindow(found_hwnd)
    time.sleep(0.8)
    
    rect = wintypes.RECT()
    user32.GetWindowRect(found_hwnd, ctypes.byref(rect))
    
    # Clicar exatamente no botão 'Psy Bass' no menu superior (y=15 absoluto em tela cheia maximizada)
    # x=735, y=14
    smooth_click(rect.left + 745, rect.top + 16, "Botao Psy Bass no Topo (y=16)")
    time.sleep(1.0)
    shot1 = trigger_and_save("13_psy_bass_engine_opened_perfect.png")
    
    # Clicar no botão 'GENERATE TO PIANO ROLL' (Botao Verde Inferior Esquerdo do plugin)
    smooth_click(rect.left + 350, rect.top + 640, "GENERATE TO PIANO ROLL")
    time.sleep(1.0)
    shot2 = trigger_and_save("14_psy_bass_generated_feedback.png")
    
    # Clicar no botão 'Piano' no topo (x=520, y=16)
    smooth_click(rect.left + 520, rect.top + 16, "Botao Piano no Topo (y=16)")
    time.sleep(1.0)
    shot3 = trigger_and_save("15_piano_roll_with_injected_psy_bass.png")

print("[TEST] Validação finalizada!")
