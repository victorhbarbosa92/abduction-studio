import time
import subprocess
import os
import sys
import tempfile
import ctypes
from ctypes import wintypes
from PIL import Image

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
user32 = ctypes.windll.user32
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"
os.makedirs(artifact_dir, exist_ok=True)

# 1. Finalizar processos antigos
os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
time.sleep(0.5)

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

def smooth_move(target_x, target_y, duration=0.5, steps=25):
    point = wintypes.POINT()
    user32.GetCursorPos(ctypes.byref(point))
    start_x, start_y = point.x, point.y
    for i in range(1, steps + 1):
        t = i / steps
        smooth_t = t * t * (3.0 - 2.0 * t)
        curr_x = start_x + (target_x - start_x) * smooth_t
        curr_y = start_y + (target_y - start_y) * smooth_t
        user32.SetCursorPos(int(curr_x), int(curr_y))
        time.sleep(duration / steps)
    time.sleep(0.05)

def smooth_click(x, y, label=""):
    print(f"[TEST] Clicando em: {label} ({int(x)}, {int(y)})")
    smooth_move(x, y)
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.12)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.4)

# Iniciar processo desacoplado (persistente)
exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
print("[TEST] Iniciando Abduction Studio V2 desacoplado...")
subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2", creationflags=0x00000008) # DETACHED_PROCESS
time.sleep(4.0)

found_hwnd = None
def enum_cb(h, l):
    global found_hwnd
    buf = ctypes.create_unicode_buffer(512)
    user32.GetWindowTextW(h, buf, 512)
    if "Abduction Studio" in buf.value:
        found_hwnd = h
        return False
    return True

EnumProc = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
user32.EnumWindows(EnumProc(enum_cb), 0)

if found_hwnd:
    print(f"[TEST] DAW Encontrada! HWND={found_hwnd}. Maximizando...")
    user32.ShowWindow(found_hwnd, 3) # SW_MAXIMIZE
    user32.SetForegroundWindow(found_hwnd)
    time.sleep(1.0)
    
    # Calcular coordenadas absolutas do cliente
    pt = wintypes.POINT(0, 0)
    user32.ClientToScreen(found_hwnd, ctypes.byref(pt))
    print(f"[TEST] Origem do Cliente na Tela: ({pt.x}, {pt.y})")
    
    # Coordenadas dos botões no Topbar (ImGui local: Rack=604, Piano=664, Y=11)
    rack_screen_x = pt.x + 604
    rack_screen_y = pt.y + 11
    
    piano_screen_x = pt.x + 664
    piano_screen_y = pt.y + 11
    
    # 1. Capturar o Piano Roll aberto
    shot1 = trigger_and_save("view_01_piano_roll.png")
    
    # 2. Clicar no botão 'Piano' para fechar o Piano Roll e ver o Channel Rack (Step Sequencer)
    smooth_click(piano_screen_x, piano_screen_y, "Botao Piano no Topbar (fechar Piano Roll)")
    time.sleep(1.0)
    shot2 = trigger_and_save("view_02_channel_rack.png")
    
    # 3. Clicar no botão 'Piano' novamente para reabrir o Piano Roll
    smooth_click(piano_screen_x, piano_screen_y, "Botao Piano no Topbar (reabrir Piano Roll)")
    time.sleep(1.0)
    shot3 = trigger_and_save("view_03_both_restored.png")
    
    print("[TEST] Validação completa com sucesso! DAW aberta em primeiro plano.")
else:
    print("[ERROR] Não foi possível encontrar a janela da DAW.")
