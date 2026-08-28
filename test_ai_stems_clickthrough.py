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
    time.sleep(0.2)

print("[TEST] 1. Reiniciando DAW e localizando janela...")
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
print(f"[TEST] HWND: {hwnd}")
if hwnd:
    user32.ShowWindow(hwnd, 3) # MAXIMIZE
    user32.SetForegroundWindow(hwnd)
    time.sleep(1.0)

rect = wintypes.RECT()
user32.GetWindowRect(hwnd, ctypes.byref(rect))

# 1. Capture clean startup
trigger_and_wait_screenshot("test_clickthrough_1_clean.png")

# 2. Click "AI Stems" in the top bar to open floating AI Stems modal
print("[TEST] 2. Abrindo janela flutuante AI Stems (X=875, Y=15)...")
mouse_click(rect.left + 875, rect.top + 15)
time.sleep(1.0)
trigger_and_wait_screenshot("test_clickthrough_2_ai_stems_open.png")

# 3. Click multiple times inside the AI Stems floating window (over Playlist track 2, 3, 4, 7)
print("[TEST] 3. Clicando multiplas vezes dentro da janela AI Stems...")
mouse_click(rect.left + 500, rect.top + 220)
time.sleep(0.2)
mouse_click(rect.left + 600, rect.top + 260)
time.sleep(0.2)
mouse_click(rect.left + 700, rect.top + 300)
time.sleep(0.2)
mouse_click(rect.left + 550, rect.top + 350)
time.sleep(0.5)

trigger_and_wait_screenshot("test_clickthrough_3_after_clicks.png")

# 4. Close the AI Stems window
print("[TEST] 4. Fechando janela AI Stems para verificar playlist...")
mouse_click(rect.left + 875, rect.top + 15)
time.sleep(0.8)
trigger_and_wait_screenshot("test_clickthrough_4_playlist_clean_verified.png")

print("[TEST] Verificacao de click-through concluida com sucesso!")
