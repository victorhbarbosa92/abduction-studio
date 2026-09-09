import time
import subprocess
import os
import sys
import tempfile
import ctypes
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

user32 = ctypes.windll.user32
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"
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
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    else:
        print(f"[TEST] Falha ao capturar {png_name}")
        return None

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
MOUSEEVENTF_RIGHTDOWN = 0x0008
MOUSEEVENTF_RIGHTUP = 0x0010

def mouse_click(x, y):
    user32.SetCursorPos(int(x), int(y))
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.4)

def mouse_right_click(x, y):
    user32.SetCursorPos(int(x), int(y))
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0)
    time.sleep(0.1)
    user32.mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0)
    time.sleep(0.4)

# Encerrar instâncias anteriores
subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
time.sleep(1.0)

# Iniciar AbductionStudioV2
app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([app_path], cwd=os.path.dirname(app_path))
print(f"[TEST] Iniciado AbductionStudioV2 PID: {proc.pid}")
time.sleep(4.0)

# Capturar tela inicial mostrando o Browser à esquerda com SAMPLES
shot1 = trigger_and_save("15_electronic_soundbanks_browser.png")

# Se o browser não estiver expandido ou se precisarmos clicar no menu do canal
# Vamos clicar no botão 4-dots do Channel Rack para abrir o menu de instrumentos e samples
# Posição típica do 4-dots no Channel Rack: x=340, y=280 ou similar
mouse_click(320, 240)
time.sleep(0.5)
# Mover o cursor para abrir o submenu de samples
user32.SetCursorPos(420, 310)
time.sleep(0.8)
shot2 = trigger_and_save("16_channel_rack_soundbanks_menu.png")

time.sleep(1.0)
proc.terminate()
time.sleep(1.0)
subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
print("[TEST] Concluído com sucesso!")
