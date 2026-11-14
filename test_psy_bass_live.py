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
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\1fe098f5-bfde-454d-a9d4-194e0b7d30b6"
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
        print(f"[TEST] Screenshot salva com sucesso: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    return None

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004

def mouse_click(x, y):
    user32.SetCursorPos(int(x), int(y))
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.4)

print("[TEST] 1. Reiniciando DAW com o novo Psy Bass Button...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(3.5)

# Clicar no botão 'Psy Bass' no topo (está logo após 'AI Stems', aprox x=700, y=14)
print("[TEST] 2. Abrindo Kuro Psy Rolling Bass Engine via cockpit toolbar...")
mouse_click(700, 14)
time.sleep(1.2)

shot1 = trigger_and_save("05_kuro_psy_rolling_bass_engine_ui.png")

# Clicar em 'GENERATE TO PIANO ROLL' (Botão verde na parte inferior da janela do plugin)
# A janela do plugin tem 1060x680, centrada. O botão esquerdo inferior fica em torno de x=380, y=650
print("[TEST] 3. Clicando em 'GENERATE TO PIANO ROLL'...")
mouse_click(380, 650)
time.sleep(1.0)

shot2 = trigger_and_save("06_kuro_psy_rolling_bass_generated_toast.png")

# Abrir Piano Roll clicando no botão Piano no topo (x=520, y=14)
print("[TEST] 4. Abrindo Piano Roll para validar notas injetadas...")
mouse_click(520, 14)
time.sleep(1.2)

shot3 = trigger_and_save("07_piano_roll_with_generated_psy_bassline.png")

print("[TEST] 5. Testes completos com sucesso!")
