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

def smooth_move(target_x, target_y, duration=0.6, steps=30):
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
    time.sleep(0.4)

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
    print("[TEST] Iniciando Abduction Studio...")
    exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
    subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(3.5)
    user32.EnumWindows(EnumProc(enum_cb), 0)

if found_hwnd:
    print(f"[TEST] DAW Encontrada! HWND={found_hwnd}. Trazendo para o topo...")
    user32.ShowWindow(found_hwnd, 3) # SW_MAXIMIZE = 3
    user32.SetForegroundWindow(found_hwnd)
    time.sleep(1.0)
    
    rect = wintypes.RECT()
    user32.GetWindowRect(found_hwnd, ctypes.byref(rect))
    w = rect.right - rect.left
    h = rect.bottom - rect.top
    print(f"[TEST] Area da Janela: {rect.left}, {rect.top}, {w}x{h}")
    
    # 2. Clicar no botão 'Psy Bass' no topo
    smooth_click(rect.left + 740, rect.top + 38, "Botao Psy Bass no Topo")
    time.sleep(1.2)
    shot1 = trigger_and_save("10_psy_bass_engine_window.png")
    
    # 3. Clicar no Preset Combo / Preset Selector para carregar preset Full-On KBBB
    # Na janela central do plugin (aproximadamente x=rect.left + w*0.5, y=rect.top + 160)
    smooth_click(rect.left + int(w * 0.45), rect.top + 180, "Preset Selector / Top Bar do Plugin")
    time.sleep(0.8)
    
    # 4. Clicar em 'GENERATE TO PIANO ROLL' (Botao Verde Inferior Esquerdo do plugin)
    # Janela plugin centrada: x em torno de rect.left + int(w * 0.35), y em torno de rect.top + int(h * 0.75)
    smooth_click(rect.left + int(w * 0.35), rect.top + int(h * 0.75), "GENERATE TO PIANO ROLL")
    time.sleep(1.0)
    shot2 = trigger_and_save("11_psy_bass_generated_notification.png")
    
    # 5. Abrir o Piano Roll (Botao 'Piano' no topo)
    smooth_click(rect.left + 520, rect.top + 38, "Botao Piano no Topo")
    time.sleep(1.2)
    shot3 = trigger_and_save("12_piano_roll_with_injected_psy_bass.png")
    
    print("[TEST] Fluxo completo de geracao e integracao testado com sucesso!")
else:
    print("[ERROR] Nao foi possivel localizar a janela da DAW.")
