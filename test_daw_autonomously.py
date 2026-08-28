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

# Mouse and Keyboard Events
MOUSEEVENTF_MOVE = 0x0001
MOUSEEVENTF_ABSOLUTE = 0x8000
MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
KEYEVENTF_KEYUP = 0x0002
VK_CONTROL = 0x11
VK_Z = 0x5A
VK_Y = 0x59

def mouse_click(x, y):
    user32.SetCursorPos(int(x), int(y))
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.2)

def send_ctrl_z():
    user32.keybd_event(VK_CONTROL, 0, 0, 0)
    time.sleep(0.1)
    user32.keybd_event(VK_Z, 0, 0, 0)
    time.sleep(0.1)
    user32.keybd_event(VK_Z, 0, KEYEVENTF_KEYUP, 0)
    time.sleep(0.1)
    user32.keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0)
    time.sleep(0.3)

def send_ctrl_y():
    user32.keybd_event(VK_CONTROL, 0, 0, 0)
    time.sleep(0.1)
    user32.keybd_event(VK_Y, 0, 0, 0)
    time.sleep(0.1)
    user32.keybd_event(VK_Y, 0, KEYEVENTF_KEYUP, 0)
    time.sleep(0.1)
    user32.keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0)
    time.sleep(0.3)

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

# Capture initial startup screenshot
trigger_and_wait_screenshot("test_1_startup_clean.png")

# Focus and activate window
mouse_click(500, 20)
time.sleep(0.3)

# Step 1: Click on scrollbars
print("[TEST] 2. Testando cliques nas barras de rolagem...")
mouse_click(800, 380) # Barra de rolagem horizontal da Playlist
time.sleep(0.2)
mouse_click(1350, 250) # Barra de rolagem vertical da Playlist
time.sleep(0.2)
trigger_and_wait_screenshot("test_2_scrollbar_clicked.png")

# Step 2: Draw a clip on Track 2
print("[TEST] 3. Desenhando um clipe Pattern 1 na grade vazia (X=700, Y=220)...")
mouse_click(700, 220)
time.sleep(0.5)
trigger_and_wait_screenshot("test_3_clip_drawn.png")

# Step 3: Send Ctrl+Z (Undo)
print("[TEST] 4. Executando Ctrl+Z para desfazer a criacao...")
send_ctrl_z()
time.sleep(0.5)
trigger_and_wait_screenshot("test_4_after_ctrl_z_undo.png")

# Step 4: Send Ctrl+Y (Redo)
print("[TEST] 5. Executando Ctrl+Y para refazer a criacao...")
send_ctrl_y()
time.sleep(0.5)
trigger_and_wait_screenshot("test_5_after_ctrl_y_redo.png")

print("[TEST] Bateria de testes de hardware concluida com sucesso!")
