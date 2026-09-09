import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\99c43430-c66d-4980-bd72-557c18180e32"
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
        png_path = os.path.join(artifact_dir, png_name)
        img = Image.open(bmp_out_file)
        img.save(png_path)
        try: os.remove(bmp_out_file)
        except: pass
        print(f"[TEST] Screenshot salva com sucesso: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    return None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.5)

    exe_path = r"build\Release\AbductionStudioV2.exe"
    print(f"[TEST] Iniciando {exe_path} com --test-phase3-dj e abrindo modal USB...")
    
    # We can pass an argument or test with python
    proc = subprocess.Popen([exe_path, "--test-phase3-dj"], cwd=".")
    time.sleep(3.0)

    # Click on the USB button in HUD using pyautogui if available or let's check
    try:
        import pyautogui
        # The window is 1280x720, "PEN DRIVE USB" button is at top right around x=860, y=18
        # Let's find window or click relative to top left
        import win32gui
        hwnd = win32gui.FindWindow(None, "Abduction Studio V2 - The Ultimate Music Workstation")
        if not hwnd:
            hwnd = win32gui.FindWindow(None, "AbductionStudioV2")
        if hwnd:
            win32gui.SetForegroundWindow(hwnd)
            rect = win32gui.GetWindowRect(hwnd)
            # HUD button is at rect[0] + 865, rect[1] + 35
            pyautogui.click(rect[0] + 880, rect[1] + 42)
            time.sleep(1.0)
    except Exception as e:
        print(f"pyautogui notice: {e}")

    shot = trigger_and_save("dj_phase3_usb_drawer.png")

    time.sleep(1.0)
    print("[TEST] Encerrando AbductionStudioV2...")
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except:
        proc.kill()

    if shot and os.path.exists(shot):
        print(f"[TEST] SUCESSO! Screenshot USB capturada: {shot}")

if __name__ == "__main__":
    main()
