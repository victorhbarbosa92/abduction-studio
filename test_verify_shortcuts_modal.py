import time
import subprocess
import os
import sys
import tempfile
import ctypes
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\9185cf81-4fad-4c51-a8f4-bf97af76252c"
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
    for _ in range(80):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            captured = True
            break

    if captured:
        img = Image.open(bmp_out_file)
        png_path = os.path.join(artifact_dir, png_name)
        img.save(png_path)
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
        try: os.remove(bmp_out_file)
        except: pass
        return png_path
    return None

def press_f1():
    VK_F1 = 0x70
    KEYEVENTF_KEYUP = 0x0002
    ctypes.windll.user32.keybd_event(VK_F1, 0, 0, 0)
    time.sleep(0.05)
    ctypes.windll.user32.keybd_event(VK_F1, 0, KEYEVENTF_KEYUP, 0)

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.5)

    exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\AbductionStudioV2.exe"
    print(f"[TEST] Iniciando {exe_path} com --test-dj-shortcuts-modal...")
    proc = subprocess.Popen([exe_path, "--test-dj-shortcuts-modal"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(4.0)

    print("[TEST] Disparando captura da Modal de Atalhos...")
    shot = trigger_and_save("rekordbox_shortcuts_modal.png")

    time.sleep(1.0)
    print("[TEST] Encerrando app...")
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except:
        proc.kill()

    if shot and os.path.exists(shot):
        print(f"[TEST] SUCESSO! Screenshot da modal gerada em: {shot}")
    else:
        print("[TEST] Falha ao capturar screenshot.")

if __name__ == "__main__":
    main()
