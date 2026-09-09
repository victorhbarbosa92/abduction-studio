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
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
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
    time.sleep(0.5)

# 1. Obter todos os PIDs de AbductionStudioV2
pids_out = subprocess.getoutput('powershell "(Get-Process -Name AbductionStudioV2 -ErrorAction SilentlyContinue).Id"')
pids = [int(p.strip()) for p in pids_out.strip().split() if p.strip().isdigit()]
print(f"[TEST] Running PIDs: {pids}")

matching_hwnds = []
def enum_cb(h, l):
    pid = wintypes.DWORD()
    user32.GetWindowThreadProcessId(h, ctypes.byref(pid))
    if pid.value in pids:
        rect = wintypes.RECT()
        user32.GetWindowRect(h, ctypes.byref(rect))
        w = rect.right - rect.left
        h_sz = rect.bottom - rect.top
        if w > 400 and h_sz > 300: # Janela principal da DAW
            matching_hwnds.append(h)
    return True

EnumProc = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
user32.EnumWindows(EnumProc(enum_cb), 0)

print(f"[TEST] Matching HWNDs: {matching_hwnds}")

if matching_hwnds:
    hwnd = matching_hwnds[0]
    user32.ShowWindow(hwnd, 3) # SW_MAXIMIZE
    user32.SetForegroundWindow(hwnd)
    time.sleep(1.0)
    
    rect = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    print(f"[TEST] Window Rect: {rect.left}, {rect.top}, {rect.right}, {rect.bottom}")
    
    # Clicar no botão 'Rolling Bass' no Piano Roll (x=745, y=302)
    print("[TEST] Clicando no botao 'Rolling Bass' no Piano Roll...")
    mouse_click(rect.left + 745, rect.top + 302)
    time.sleep(1.2)
    
    shot = trigger_and_save("25_kuro_psy_rolling_bass_full_mockup_match.png")

print("[TEST] Concluído!")
