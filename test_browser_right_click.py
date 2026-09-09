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
    return None

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
MOUSEEVENTF_RIGHTDOWN = 0x0008
MOUSEEVENTF_RIGHTUP = 0x0010
MOUSEEVENTF_WHEEL = 0x0800

def get_window_rect():
    hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
    if not hwnd:
        return None
    rect = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    return hwnd, rect.left, rect.top

def client_action(win_info, x, y, action="click"):
    hwnd, wx, wy = win_info
    pt = wintypes.POINT(int(x), int(y))
    user32.ClientToScreen(hwnd, ctypes.byref(pt))
    user32.SetCursorPos(pt.x, pt.y)
    time.sleep(0.08)
    if action == "click":
        user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
        time.sleep(0.1)
        user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    elif action == "right_click":
        user32.mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0)
        time.sleep(0.1)
        user32.mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0)
    elif action == "scroll_down":
        user32.mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -240, 0)
    elif action == "scroll_up":
        user32.mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 240, 0)
    time.sleep(0.4)

subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
time.sleep(1.0)

app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([app_path], cwd=os.path.dirname(app_path))
time.sleep(4.0)

win_info = get_window_rect()
if not win_info:
    proc.terminate()
    sys.exit(1)

# 1. Focar no browser
client_action(win_info, 100, 300, "click")
time.sleep(0.2)

# 2. Rolar o browser para baixo para ver os samples
client_action(win_info, 100, 300, "scroll_down")
time.sleep(0.3)
client_action(win_info, 100, 300, "scroll_down")
time.sleep(0.5)

# 3. Clicar com botão direito sobre um sample na lista visível
client_action(win_info, 90, 450, "right_click")
time.sleep(0.6)

shot = trigger_and_save("21_browser_sample_context_menu.png")

time.sleep(1.0)
proc.terminate()
time.sleep(1.0)
subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
print("[TEST] Concluído!")
