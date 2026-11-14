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
artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\c58cf74c-6c8d-41d2-869c-fa6dcaceb8ee"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_screen_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")
trigger_cmd_file = os.path.join(temp_dir, "abduction_home_hub_cmd.txt")

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_screen_file, "w") as f:
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
    else:
        print(f"[TEST] Falha ao capturar screenshot: {png_name}")
    return None

MOUSEEVENTF_MOVE     = 0x0001
MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP   = 0x0004

def get_window_rect():
    hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
    if not hwnd:
        return None
    rect = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    return hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top

def client_to_screen(hwnd, cx, cy):
    pt = wintypes.POINT(int(cx), int(cy))
    user32.ClientToScreen(hwnd, ctypes.byref(pt))
    return pt.x, pt.y

print("[TEST] 1. Encerrando instancias anteriores...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
print(f"[TEST] 2. Iniciando DAW: {exe_path}")
proc = subprocess.Popen([exe_path], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(3.5)

try:
    # Fecha o Home Hub para exibir a Playlist
    print("[TEST] 3. Fechando o Home Hub para exibir a Playlist...")
    with open(trigger_cmd_file, "w") as f:
        f.write("close\n")
    time.sleep(1.2)

    win_info = get_window_rect()
    if not win_info:
        print("[TEST] Erro: Janela da DAW nao encontrada!")
        sys.exit(1)
    
    hwnd, wx, wy, ww, wh = win_info
    print(f"[TEST] Janela localizada em ({wx}, {wy}) tamanho {ww}x{wh}")
    user32.SetForegroundWindow(hwnd)
    time.sleep(0.3)

    # Coordenadas do item de sample no Browser (lado esquerdo)
    # Kick 01 em aprox x=80, y=440
    start_sx, start_sy = client_to_screen(hwnd, 85, 440)
    
    # Coordenadas da Track 2, Bar 3-5 na Playlist (lado direito)
    # x=520, y=355
    target_sx, target_sy = client_to_screen(hwnd, 520, 355)

    print(f"[TEST] 4. Posicionando mouse no Sample ({start_sx}, {start_sy})...")
    user32.SetCursorPos(start_sx, start_sy)
    time.sleep(0.2)

    print("[TEST] 5. Pressionando botao esquerdo e iniciando Drag & Drop...")
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.2)

    # Arraste suave em 20 passos para acionar ImGuiDragDrop
    steps = 20
    for s in range(1, steps + 1):
        cur_x = int(start_sx + (target_sx - start_sx) * (s / steps))
        cur_y = int(start_sy + (target_sy - start_sy) * (s / steps))
        user32.SetCursorPos(cur_x, cur_y)
        time.sleep(0.04)

    print(f"[TEST] 6. Mouse posicionado na Trilha 2 da Playlist ({target_sx}, {target_sy}).")
    time.sleep(0.4)

    # Captura a screenshot COM O GHOST HOLOGRAFICO CIANO ATIVO
    print("[TEST] 7. Capturando screenshot do Ghost Holografico Ciano...")
    shot_ghost = trigger_and_save("daw_playlist_dnd_ghost_live.png")

    time.sleep(0.4)

    # Solta o sample na trilha (Drop)
    print("[TEST] 8. Soltando o sample na Trilha 2 (Mouse Up)...")
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(1.0)

    # Captura a screenshot com o Clip carregado na Playlist
    print("[TEST] 9. Capturando screenshot da Playlist com o sample inserido...")
    shot_dropped = trigger_and_save("daw_playlist_after_drop_live.png")

finally:
    print("[TEST] 10. Finalizando DAW de teste...")
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    print("[TEST] Concluido com sucesso!")
