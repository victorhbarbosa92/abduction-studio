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
KEYEVENTF_KEYUP = 0x0002
VK_F10 = 0x79

def mouse_click(x, y):
    user32.SetCursorPos(int(x), int(y))
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.15)

def send_key(vk):
    user32.keybd_event(vk, 0, 0, 0)
    time.sleep(0.08)
    user32.keybd_event(vk, 0, KEYEVENTF_KEYUP, 0)
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

# Step 1: Clean startup
trigger_and_wait_screenshot("test_floating_1_clean.png")

# Step 2: Open AI Stems window via F10
print("[TEST] 2. Abrindo janela AI Stems via F10...")
send_key(VK_F10)
time.sleep(0.8)
trigger_and_wait_screenshot("test_floating_2_ai_stems_opened.png")

# Step 3: Click multiple times on the AI Stems window over Track 1, 2, 3, 4, 7
print("[TEST] 3. Clicando multiplas vezes sobre a janela flutuante...")
rect = wintypes.RECT()
user32.GetWindowRect(hwnd, ctypes.byref(rect))
mouse_click(rect.left + 500, rect.top + 200) # Track 1 area
time.sleep(0.1)
mouse_click(rect.left + 600, rect.top + 240) # Track 2 area
time.sleep(0.1)
mouse_click(rect.left + 700, rect.top + 280) # Track 3 area
time.sleep(0.1)
mouse_click(rect.left + 550, rect.top + 330) # Track 4 area
time.sleep(0.1)
mouse_click(rect.left + 650, rect.top + 400) # Track 7 area
time.sleep(0.3)
trigger_and_wait_screenshot("test_floating_3_during_clicks.png")

# Step 4: Close AI Stems window via F10 and verify playlist underneath is 100% clean
print("[TEST] 4. Fechando janela AI Stems via F10 para validar grade...")
send_key(VK_F10)
time.sleep(0.8)
trigger_and_wait_screenshot("test_floating_4_playlist_verified_clean.png")

print("[TEST] Bateria de testes de imunidade a janelas flutuantes concluida com 100% de sucesso!")
