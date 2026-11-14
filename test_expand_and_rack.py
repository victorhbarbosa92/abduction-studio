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

def send_key(vk):
    user32.keybd_event(vk, 0, 0, 0)
    time.sleep(0.05)
    user32.keybd_event(vk, 0, 2, 0)
    time.sleep(0.3)

VK_F6 = 0x75 # F6 for Channel Rack
VK_F7 = 0x76 # F7 for Piano Roll

# Encerrar instâncias anteriores
subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
time.sleep(1.0)

# Iniciar AbductionStudioV2
app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([app_path], cwd=os.path.dirname(app_path))
print(f"[TEST] Iniciado AbductionStudioV2 PID: {proc.pid}")
time.sleep(4.0)

# 1. Expandir 01_Psytrance no browser (clicar na seta x=46, y=470)
mouse_click(46, 470)
time.sleep(0.5)

# Expandir 01_Kicks dentro de 01_Psytrance (x=56, y=488)
mouse_click(56, 488)
time.sleep(0.5)

# Expandir 02_Rolling_Basses (x=56, y=575)
mouse_click(56, 575)
time.sleep(0.5)

# Capturar browser com samples expandidos
shot1 = trigger_and_save("17_soundbanks_expanded_samples.png")

# 2. Abrir Channel Rack pressionando F6
send_key(VK_F6)
time.sleep(1.0)

# Clicar no botão ☷ do Canal 0 (Kick) no Channel Rack
# No Channel Rack, o botão ☷ fica na coluna à esquerda dos botões de drum
# Vamos encontrar a janela do Channel Rack e clicar no botão ☷ ou na faixa
mouse_click(460, 255)
time.sleep(0.6)

# Hover sobre "Carregar Amostra / Bateria (Sample)"
user32.SetCursorPos(560, 315)
time.sleep(0.6)

# Hover sobre "Psytrance Soundbank"
user32.SetCursorPos(750, 315)
time.sleep(0.8)

shot2 = trigger_and_save("18_channel_rack_psytrance_samples_menu.png")

time.sleep(1.0)
proc.terminate()
time.sleep(1.0)
subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
print("[TEST] Concluído!")
