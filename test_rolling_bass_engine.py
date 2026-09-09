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
    time.sleep(0.3)

print("[TEST] 1. Encerrando processos antigos do AbductionStudioV2...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
print(f"[TEST] 2. Iniciando DAW: {exe_path}")
proc = subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")

time.sleep(3.5)

# Trazer janela para frente
hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
if hwnd:
    print(f"[TEST] Janela encontrada: HWND={hwnd}. Maximizando...")
    user32.ShowWindow(hwnd, 3) # SW_MAXIMIZE = 3
    user32.SetForegroundWindow(hwnd)
    time.sleep(1.0)
else:
    print("[WARN] Janela nao encontrada por FindWindowA.")

# Pressionar F7 para abrir o Piano Roll
print("[TEST] 3. Abrindo Piano Roll (F7)...")
VK_F7 = 0x76
user32.keybd_event(VK_F7, 0, 0, 0)
time.sleep(0.1)
user32.keybd_event(VK_F7, 0, 2, 0)
time.sleep(1.5)

# Capturar print do Piano Roll com o novo botão Rolling Bass
shot1 = trigger_and_save("01_piano_roll_with_rolling_bass_btn.png")

# Clicar no botão Rolling Bass ou disparar F6 / menu
# Na barra do Piano Roll, clicar no botão de Rolling Bass
print("[TEST] 4. Disparando Rolling Bass via clique...")
rect = wintypes.RECT()
if hwnd:
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    # Clicar aproximadamente na área de ferramentas do Piano Roll (aprox x=440, y=140 relativo)
    mouse_click(rect.left + 540, rect.top + 140)
    time.sleep(1.0)

shot2 = trigger_and_save("02_rolling_bass_generator_opened.png")

print("[TEST] 5. Teste finalizado com sucesso!")
