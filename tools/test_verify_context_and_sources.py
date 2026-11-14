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
artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\9355a9bb-0b73-4db6-a5fa-c7caeed07313"
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
        print(f"[TEST] Screenshot salva: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
MOUSEEVENTF_RIGHTDOWN = 0x0008
MOUSEEVENTF_RIGHTUP = 0x0010

def get_client_origin():
    hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
    if not hwnd:
        return None, 0, 0
    user32.ShowWindow(hwnd, 3) # SW_MAXIMIZE
    user32.SetForegroundWindow(hwnd)
    time.sleep(0.3)
    pt = wintypes.POINT(0, 0)
    user32.ClientToScreen(hwnd, ctypes.byref(pt))
    return hwnd, pt.x, pt.y

def mouse_click(cx, cy, right=False):
    hwnd, ox, oy = get_client_origin()
    if not hwnd: return
    sx, sy = ox + cx, oy + cy
    user32.SetCursorPos(int(sx), int(sy))
    time.sleep(0.15)
    if right:
        user32.mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0)
        time.sleep(0.08)
        user32.mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0)
    else:
        user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
        time.sleep(0.08)
        user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.3)

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(0.8)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
cwd = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("\n=== [1] TESTE DO MODAL DE EDIÇÃO DE METADADOS & BIBLIOTECA ===")
kill_daw()
proc = subprocess.Popen([exe_path, "--test-dj-context"], cwd=cwd)
time.sleep(4.0)

png1 = trigger_and_save("14_dj_context_menu_sources_live.png")
proc.terminate()
kill_daw()

print("\n=== [2] TESTE DO SELETOR DE FONTES LIMPAS NO KURO CLOUD ===")
kill_daw()
proc2 = subprocess.Popen([exe_path, "--test-sources-modal"], cwd=cwd)
time.sleep(4.0)

png2 = trigger_and_save("15_kuro_cloud_anti_ad_sources_live.png")
proc2.terminate()
kill_daw()

print("\n=== [3] TESTE DO MENU DE CONTEXTO (BOTÃO DIREITO NA FAIXA DA BIBLIOTECA) ===")
kill_daw()
proc3 = subprocess.Popen([exe_path, "--test-dj-crates"], cwd=cwd)
time.sleep(4.0)

# Clica com botão direito na linha da faixa (x=400, y=852)
print("[TEST] Clicando com botão direito na linha da música...")
mouse_click(400, 852, right=True)
time.sleep(1.0)

png3 = trigger_and_save("16_dj_library_right_click_menu_live.png")
proc3.terminate()
kill_daw()

print(f"\n[SUCESSO] Testes concluídos!")
print(f"  -> {png1}")
print(f"  -> {png2}")
print(f"  -> {png3}")
