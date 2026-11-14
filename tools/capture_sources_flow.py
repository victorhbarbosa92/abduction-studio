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
artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\a7531291-b5ae-4563-8e85-584c2f47d8c3"
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
        print(f"[TEST] Screenshot salva: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004

def get_client_origin():
    hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
    if not hwnd:
        return None, 0, 0
    user32.ShowWindow(hwnd, 9)
    user32.SetForegroundWindow(hwnd)
    time.sleep(0.2)
    pt = wintypes.POINT(0, 0)
    user32.ClientToScreen(hwnd, ctypes.byref(pt))
    return hwnd, pt.x, pt.y

def mouse_click(cx, cy):
    hwnd, ox, oy = get_client_origin()
    if not hwnd: return
    sx, sy = ox + cx, oy + cy
    user32.SetCursorPos(int(sx), int(sy))
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.3)

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(0.8)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\AbductionStudioV2.exe"
cwd = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("\n[TEST] Iniciando AbductionStudioV2 com Crates...")
kill_daw()
proc = subprocess.Popen([exe_path, "--test-dj-crates"], cwd=cwd)
time.sleep(3.5)

# Clica no botão KURU CLOUD na barra superior (aprox x=1330, y=15)
print("[TEST] Clicando em KURU CLOUD...")
mouse_click(1330, 15)
time.sleep(1.0)
trigger_and_save("03_kuru_cloud_search_modal_with_fontes.png")

# Clica no primeiro botão [ FONTES ] da tabela de busca
# O modal de busca abre centralizado, a coluna Ações fica em x~1400, y~430
print("[TEST] Clicando em [ FONTES ] para abrir o seletor de fontes sem propaganda...")
mouse_click(1420, 435)
time.sleep(1.5)
trigger_and_save("04_sources_selector_modal_verified.png")

proc.terminate()
kill_daw()
print("\n[SUCESSO] Capturas concluídas com sucesso!")
