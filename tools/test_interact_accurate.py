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
        print(f"[TEST] Screenshot salva: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
KEYEVENTF_KEYUP = 0x0002
VK_SPACE = 0x20
VK_F7 = 0x76
VK_F11 = 0x7A

def get_client_origin():
    hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
    if not hwnd:
        return None, 0, 0
    user32.ShowWindow(hwnd, 9) # SW_RESTORE
    user32.SetForegroundWindow(hwnd)
    time.sleep(0.2)
    pt = wintypes.POINT(0, 0)
    user32.ClientToScreen(hwnd, ctypes.byref(pt))
    return hwnd, pt.x, pt.y

def click_client(cx, cy, x, y, delay=0.3):
    user32.SetCursorPos(cx + int(x), cy + int(y))
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(delay)

def move_client(cx, cy, x, y, delay=0.1):
    user32.SetCursorPos(cx + int(x), cy + int(y))
    time.sleep(delay)

def press_key(vk):
    user32.keybd_event(vk, 0, 0, 0)
    time.sleep(0.06)
    user32.keybd_event(vk, 0, KEYEVENTF_KEYUP, 0)
    time.sleep(0.4)

print("[TEST] 1. Reiniciando DAW com --open-psy-bass...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([exe_path, "--open-psy-bass"], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(3.5)

hwnd, cx, cy = get_client_origin()
print(f"[TEST] Client origin: ({cx}, {cy})")

# 1. Foto inicial
trigger_and_save("01_kuro_psy_bass_initial_window.png")

# 2. Interagir com os combos
# Kuro Bass window está em (120, 70) no cliente 1280x720.
# Synth combo está em x = 120 + 228 + 60 = 408, y = 70 + 26 + 14 + 10 = 120.
print("[TEST] Clicando no Synth Engine combo...")
click_client(cx, cy, 408, 120)
time.sleep(0.3)
click_client(cx, cy, 408, 155) # Seleciona opção 1 (Virus TI Hyper-Saw)
time.sleep(0.4)

# Root Note combo em x = 120 + 228 + 163 + 193 + 35 = 739, y = 120
print("[TEST] Clicando no Root Note combo...")
click_client(cx, cy, 739, 120)
time.sleep(0.3)
# Desce para A1: F#1 é o selecionado. Clicar um pouco mais abaixo na lista
click_client(cx, cy, 739, 280)
time.sleep(0.4)

# Scale combo em x = 120 + 228 + 163 + 193 + 82 + 50 = 846, y = 120
print("[TEST] Clicando no Scale combo...")
click_client(cx, cy, 846, 120)
time.sleep(0.3)
# Seleciona Natural Minor (segunda opção)
click_client(cx, cy, 846, 155)
time.sleep(0.4)

trigger_and_save("02_kuro_psy_bass_modified_settings.png")

# 3. Disparar Audition Loop
# Audition button: x = 120 + 228 + 163 + 193 + 82 + 118 + 70 = 974, y = 120
print("[TEST] Disparando Audition Loop...")
click_client(cx, cy, 974, 120)
time.sleep(1.2)
trigger_and_save("03_kuro_psy_bass_audition_playing.png")

# 4. Generate to Piano Roll
# Botão Generate to Piano Roll: x = 120 + 20 + 200 = 340, y = 70 + 70 + 378 + 12 + 20 = 550
print("[TEST] Clicando em GENERATE TO PIANO ROLL...")
click_client(cx, cy, 340, 550)
time.sleep(0.8)
trigger_and_save("04_kuro_psy_bass_generated_to_piano_roll.png")

# Parar audition
press_key(VK_SPACE)
time.sleep(0.3)

# 5. Fechar janela do Kuro Bass via F11
print("[TEST] Fechando Kuro Bass via atalho F11...")
press_key(VK_F11)
time.sleep(0.6)

# 6. Testar OLED Hint Display
# Passar o mouse sobre o Snap Selector (x = 390, y = 60)
print("[TEST] Posicionando mouse no Snap Selector para ativar texto longo...")
move_client(cx, cy, 390, 60, delay=1.2)
trigger_and_save("05_oled_hint_text_clipped.png")

# 7. Abrir Piano Roll via F7
print("[TEST] Abrindo Piano Roll via F7...")
press_key(VK_F7)
time.sleep(1.0)
trigger_and_save("06_piano_roll_with_generated_bassline.png")

print("[TEST] Finalizado com sucesso!")
proc.terminate()
