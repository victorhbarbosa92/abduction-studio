import os
import time
import subprocess
import ctypes
from PIL import Image

temp_dir = os.environ.get("TEMP", "C:\\Windows\\Temp")
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"

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
    ctypes.windll.user32.SetCursorPos(x, y)
    time.sleep(0.1)
    ctypes.windll.user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.05)
    ctypes.windll.user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.2)

print("[TEST] 1. Iniciando AbductionStudioV2 em modo normal...")
proc = subprocess.Popen(["build/Release/AbductionStudioV2.exe"], cwd=".")
time.sleep(3.0)

# Capture 1: Clean desktop home
trigger_and_save("01_abduction_android_home_clean.png")

# Click on Sequenciadores folder (second folder)
# In 1280x720:
# Browser ends around x=234. Desktop width is 1046.
# Folders start around x=480..500, y=620
print("[TEST] 2. Clicando na pasta 'Sequenciadores'...")
mouse_click(630, 620)
time.sleep(0.5)

trigger_and_save("02_abduction_folder_drawer_open.png")

# Now click on "Piano Roll" inside the popup (top left button inside popup)
# Popup center is at cx=757, cy=380. Size 550x340.
# Left button is around x=620, y=320
print("[TEST] 3. Clicando no botao 'Piano Roll' da gaveta...")
mouse_click(620, 320)
time.sleep(0.5)

trigger_and_save("03_abduction_floating_piano_roll.png")

proc.terminate()
print("[TEST] Concluido com sucesso!")
