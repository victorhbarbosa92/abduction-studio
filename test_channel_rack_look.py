import time
import subprocess
import os
import tempfile
import ctypes
from ctypes import wintypes
from PIL import Image

user32 = ctypes.windll.user32
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\a60eab68-4cf5-49c9-8f65-7395e5b71c01"
temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_wait_screenshot(png_name):
    png_path = os.path.join(artifact_dir, png_name)
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("trigger\n")

    captured = False
    for _ in range(60):
        time.sleep(0.05)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            captured = True
            break

    if captured and os.path.exists(bmp_out_file):
        try:
            time.sleep(0.05)
            img = Image.open(bmp_out_file)
            img.save(png_path)
            os.remove(bmp_out_file)
            print(f"[TEST] Screenshot gravada: {png_path} ({img.size[0]}x{img.size[1]})")
            return png_path
        except Exception as e:
            print(f"[TEST] Erro convertendo BMP para PNG: {e}")
    else:
        print(f"[TEST] Aviso: BMP nao gerado para {png_name}")
    return None

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004

def mouse_click(x, y):
    user32.SetCursorPos(int(x), int(y))
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.3)

print("[TEST] Reiniciando DAW e abrindo Channel Rack pelo botao RACK...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(3.0)

found_hwnd = None
def enum_cb(h, l):
    buf = ctypes.create_unicode_buffer(512)
    user32.GetWindowTextW(h, buf, 512)
    if "Abduction Studio" in buf.value:
        global found_hwnd
        found_hwnd = h
        return False
    return True

EnumWindowsProc = ctypes.WINFUNCTYPE(ctypes.c_bool, wintypes.HWND, wintypes.LPARAM)
user32.EnumWindows(EnumWindowsProc(enum_cb), 0)

hwnd = found_hwnd
if hwnd:
    user32.ShowWindow(hwnd, 3) # MAXIMIZE
    user32.SetForegroundWindow(hwnd)
    time.sleep(1.0)

rect = wintypes.RECT()
user32.GetWindowRect(hwnd, ctypes.byref(rect))

# Click "Rack" button in the top menu bar at screen coordinates X = rect.left + 605, Y = rect.top + 38
print("[TEST] Clicando no botao 'Rack' no menu superior (X=605, Y=38)...")
mouse_click(rect.left + 605, rect.top + 38)
time.sleep(1.5)

# Fill 4-on-the-floor kick, clap, hihats on the Channel Rack
# Kick (Row 0): steps 0, 4, 8, 12
mouse_click(rect.left + 540, rect.top + 270) # step 0
mouse_click(rect.left + 608, rect.top + 270) # step 4
mouse_click(rect.left + 676, rect.top + 270) # step 8
mouse_click(rect.left + 744, rect.top + 270) # step 12

# Clap (Row 1): steps 4, 12
mouse_click(rect.left + 608, rect.top + 296) # step 4
mouse_click(rect.left + 744, rect.top + 296) # step 12

# HiHat (Row 2): every 2 steps
for s in range(0, 16, 2):
    offset_x = s * 17 + ((s // 4) * 4)
    mouse_click(rect.left + 540 + offset_x, rect.top + 322)

# Snare (Row 3): steps 4, 12
mouse_click(rect.left + 608, rect.top + 348) # step 4
mouse_click(rect.left + 744, rect.top + 348) # step 12

# FLEX Bass (Row 4): steps 1, 2, 3, 5, 6, 7
mouse_click(rect.left + 557, rect.top + 374) # step 1
mouse_click(rect.left + 574, rect.top + 374) # step 2
mouse_click(rect.left + 591, rect.top + 374) # step 3
mouse_click(rect.left + 625, rect.top + 374) # step 5
mouse_click(rect.left + 642, rect.top + 374) # step 6
mouse_click(rect.left + 659, rect.top + 374) # step 7

time.sleep(0.5)
trigger_and_wait_screenshot("test_fl_channel_rack_exact.png")
print("[TEST] Screenshot capturada com sucesso!")
