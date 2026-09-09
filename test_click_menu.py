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
    for _ in range(60):
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

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004

def get_window_rect():
    hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
    if not hwnd:
        return None
    rect = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    return hwnd, rect.left, rect.top

def client_click(win_info, x, y):
    hwnd, wx, wy = win_info
    # Na maioria das janelas GLFW sem borda custom, a barra de título tem ~31px e borda 8px
    pt = wintypes.POINT(int(x), int(y))
    user32.ClientToScreen(hwnd, ctypes.byref(pt))
    user32.SetCursorPos(pt.x, pt.y)
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.4)

def client_hover(win_info, x, y):
    hwnd, wx, wy = win_info
    pt = wintypes.POINT(int(x), int(y))
    user32.ClientToScreen(hwnd, ctypes.byref(pt))
    user32.SetCursorPos(pt.x, pt.y)
    time.sleep(0.4)

subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
time.sleep(1.0)

app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([app_path], cwd=os.path.dirname(app_path))
print(f"[TEST] Iniciado AbductionStudioV2 PID: {proc.pid}")
time.sleep(4.0)

win_info = None
for _ in range(30):
    win_info = get_window_rect()
    if win_info:
        break
    time.sleep(0.2)

if not win_info:
    print("[TEST] Janela não encontrada!")
    proc.terminate()
    sys.exit(1)

print(f"[TEST] Janela encontrada em ({win_info[1]}, {win_info[2]})")

# 1. Focar a janela
client_click(win_info, 500, 20)
time.sleep(0.3)

# 2. Clicar no botão ☷ do Track 1 (x=383, y=273)
client_click(win_info, 383, 273)
time.sleep(0.6)

# 3. Hover sobre o submenu "🥁 Carregar Amostra / Bateria (Sample)"
# O popup abre logo abaixo do botão ☷ (x=383, y=295). "Carregar Amostra" é a 3ª opção (~y=350)
client_hover(win_info, 480, 350)
time.sleep(0.6)

# 4. Hover sobre o submenu "🌀 Psytrance Soundbank" que abre à direita
client_hover(win_info, 720, 350)
time.sleep(0.8)

shot = trigger_and_save("20_channel_rack_soundbanks_menu_real.png")

time.sleep(1.0)
proc.terminate()
time.sleep(1.0)
subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
print("[TEST] Concluído!")
