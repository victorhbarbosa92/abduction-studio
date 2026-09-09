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
    for _ in range(70):
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

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(0.8)

kill_daw()

app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("[TEST] Iniciando AbductionStudioV2 com flag --test-engine-refinements...")
proc = subprocess.Popen([app_path, "--test-engine-refinements"], cwd=cwd)
print(f"[TEST] AbductionStudioV2 iniciado com PID {proc.pid}")
time.sleep(4.5)

png_path = trigger_and_save("30_engine_refinements_verified.png")

# Verificar arquivo .kuro salvo
kuro_file = os.path.join(cwd, "test_refinements.kuro")
if os.path.exists(kuro_file):
    print(f"[TEST] test_refinements.kuro criado com sucesso! Tamanho: {os.path.getsize(kuro_file)} bytes")
    with open(kuro_file, "r", encoding="utf-8") as f:
        content = f.read()
        print(f"[TEST] Preview do test_refinements.kuro:\n{content[:500]}...")
else:
    print("[TEST] AVISO: test_refinements.kuro não encontrado!")

kill_daw()
print("[TEST] Finalizado com sucesso!")
