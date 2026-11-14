import time
import subprocess
import os
import sys
import tempfile
import ctypes
import ctypes.wintypes as wt
from PIL import Image

user32 = ctypes.windll.user32

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\725a1335-8fa2-4938-9595-4adffb7b22cd"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_filename):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("trigger\n")

    captured = False
    for _ in range(80):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            captured = True
            break

    if captured:
        img = Image.open(bmp_out_file)
        dest_path = os.path.join(artifact_dir, png_filename)
        img.save(dest_path)
        print(f"[TEST] Screenshot salva: {dest_path} ({img.size[0]}x{img.size[1]})")
        try: os.remove(bmp_out_file)
        except: pass
        return dest_path
    else:
        print(f"[TEST] Timeout aguardando captura para {png_filename}")
        return None

def find_app_window():
    return user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")

def press_space_key(hwnd):
    # Bring window to foreground and send space
    user32.SetForegroundWindow(hwnd)
    time.sleep(0.1)
    # WM_KEYDOWN = 0x0100, WM_KEYUP = 0x0101, VK_SPACE = 0x20
    user32.PostMessageA(hwnd, 0x0100, 0x20, 0)
    time.sleep(0.05)
    user32.PostMessageA(hwnd, 0x0101, 0x20, 0)
    time.sleep(0.2)

def move_mouse_to_relative(hwnd, rel_x, rel_y):
    rect = wt.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    target_x = rect.left + rel_x
    target_y = rect.top + rel_y
    user32.SetCursorPos(target_x, target_y)

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.8)

    print("[TEST] Iniciando AbductionStudioV2.exe com --test-split-library...")
    proc = subprocess.Popen(["AbductionStudioV2.exe", "--test-split-library"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(4.5)

    hwnd = find_app_window()
    print(f"[TEST] HWND da janela: {hwnd}")

    if hwnd:
        user32.SetForegroundWindow(hwnd)
        # Move cursor to hover over track title in library
        move_mouse_to_relative(hwnd, 400, 520)
        time.sleep(0.5)

    print("\n--- 1. Capturando Split Mode com Hover ---")
    s1 = trigger_and_save("01_split_mode_hover_laser.png")

    if hwnd:
        print("\n--- Pressionando BARRA DE ESPAÇO -> Big Library Mode ---")
        press_space_key(hwnd)
        time.sleep(1.0)
        move_mouse_to_relative(hwnd, 420, 310)
        time.sleep(0.5)

    print("\n--- 2. Capturando Big Library Mode (Mixxx Style) ---")
    s2 = trigger_and_save("02_big_library_mode.png")

    if hwnd:
        print("\n--- Pressionando BARRA DE ESPAÇO -> Performance Mode ---")
        press_space_key(hwnd)
        time.sleep(1.0)

    print("\n--- 3. Capturando Performance Mode (Full Screen Decks) ---")
    s3 = trigger_and_save("03_performance_mode.png")

    if hwnd:
        print("\n--- Pressionando BARRA DE ESPAÇO -> Split Mode Restored ---")
        press_space_key(hwnd)
        time.sleep(1.0)

    print("\n--- 4. Capturando Split Mode Restaurado ---")
    s4 = trigger_and_save("04_split_mode_restored.png")

    print("[TEST] Encerrando app...")
    proc.terminate()
    try: proc.wait(timeout=3)
    except: proc.kill()

    print("\n[RESUMO DOS TESTES]")
    print(f"1. Split Mode: {s1}")
    print(f"2. Big Library Mode: {s2}")
    print(f"3. Performance Mode: {s3}")
    print(f"4. Split Restored: {s4}")

if __name__ == "__main__":
    main()
