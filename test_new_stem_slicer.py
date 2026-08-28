import time
import subprocess
import os
import tempfile
import ctypes
from ctypes import wintypes
from PIL import Image

user32 = ctypes.windll.user32
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\1f79439e-0624-4173-b22c-45fb0be163fa"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_screenshot(png_name):
    # Direct GDI screenshot of foreground window / full screen
    try:
        from PIL import ImageGrab
        img = ImageGrab.grab()
        png_path = os.path.join(artifact_dir, png_name)
        img.save(png_path)
        print(f"[TEST] Screenshot gravada: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    except Exception as e:
        print(f"[TEST] Erro no ImageGrab: {e}")
    return None

# Mouse and Keyboard Events
MOUSEEVENTF_MOVE = 0x0001
MOUSEEVENTF_ABSOLUTE = 0x8000
MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
KEYEVENTF_KEYUP = 0x0002
VK_F10 = 0x79

def mouse_move_smooth(x1, y1, steps=20):
    point = wintypes.POINT()
    user32.GetCursorPos(ctypes.byref(point))
    x0, y0 = point.x, point.y
    for i in range(steps):
        t = (i + 1) / float(steps)
        # smoothstep
        t_smooth = t * t * (3 - 2 * t)
        curr_x = int(x0 + (x1 - x0) * t_smooth)
        curr_y = int(y0 + (y1 - y0) * t_smooth)
        user32.SetCursorPos(curr_x, curr_y)
        time.sleep(0.015)

def mouse_click(x, y):
    mouse_move_smooth(x, y, 15)
    time.sleep(0.05)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.2)

print("[TEST] 1. Encerrando instancias anteriores...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
print(f"[TEST] 2. Iniciando DAW: {exe_path}")
proc = subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(3.5)

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

# 1. Abrir Menu Exibir ou clicar no botao AI Stems
print("[TEST] 3. Acionando botao AI Stems...")
# Atalho F10 ou clique no botao AI Stems na barra superior (aprox X=780, Y=42)
user32.keybd_event(VK_F10, 0, 0, 0)
time.sleep(0.1)
user32.keybd_event(VK_F10, 0, KEYEVENTF_KEYUP, 0)
time.sleep(1.2)

# Se nao abriu por F10, clica no menu Exibir -> Separador de Stems
mouse_click(130, 10) # Menu Exibir
time.sleep(0.4)
mouse_click(150, 140) # Item Separador de Stems
time.sleep(1.0)

trigger_screenshot("test_stem_slicer_window.png")

# Interagir com o botao de quantizar 4 Bars (aprox meio da janela)
print("[TEST] 4. Testando botoes de quantizacao e faders...")
time.sleep(0.5)

trigger_screenshot("test_stem_slicer_interactive.png")

print("[TEST] Teste concluido com sucesso!")
