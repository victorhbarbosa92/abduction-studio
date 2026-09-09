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
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"
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
        png_path = os.path.join(artifact_dir, png_name)
        img = Image.open(bmp_out_file)
        img.save(png_path)
        try: os.remove(bmp_out_file)
        except: pass
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    else:
        print(f"[TEST] Falha ao capturar {png_name}")
        return None

MOUSEEVENTF_MOVE = 0x0001

def get_window_rect():
    hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
    if not hwnd:
        return None
    rect = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    return hwnd, rect.left, rect.top

def client_hover(win_info, x, y):
    hwnd, wx, wy = win_info
    pt = wintypes.POINT(int(x), int(y))
    user32.ClientToScreen(hwnd, ctypes.byref(pt))
    user32.SetCursorPos(pt.x, pt.y)
    time.sleep(0.05)
    user32.mouse_event(MOUSEEVENTF_MOVE, 0, 0, 0, 0)
    time.sleep(0.7)

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(0.8)

kill_daw()

app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

# Iniciar com o menu do Channel Rack já forçado
proc = subprocess.Popen([app_path, "--test-cr-menu"], cwd=cwd)
print(f"[TEST] AbductionStudioV2 iniciado com PID {proc.pid}")
time.sleep(4.0)

win_info = None
for _ in range(30):
    win_info = get_window_rect()
    if win_info: break
    time.sleep(0.2)

if not win_info:
    print("[TEST] Janela não encontrada!")
    proc.terminate()
    sys.exit(1)

# 1. Hover sobre "Carregar Amostra / Bateria (Sample)" (x~100, y=100)
print("[TEST] Hover sobre Carregar Amostra (y=100)...")
client_hover(win_info, 100, 100)

# 2. Hover sobre "🌀 Sonicspore PRYZMA (FREE Psytrance Pack)" (x~320, y=100)
print("[TEST] Hover sobre Sonicspore PRYZMA...")
client_hover(win_info, 320, 100)

# 3. Hover sobre "🥊 Kicks (Punch & Click)" (x~560, y=100)
print("[TEST] Hover sobre Kicks...")
client_hover(win_info, 560, 100)

trigger_and_save("29_sonicspore_pryzma_channel_rack_menu.png")

kill_daw()
print("[TEST] Concluído!")
