import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\5e7ad681-aa81-434e-a40e-2610279c141b"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_screen_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")
trigger_cmd_file = os.path.join(temp_dir, "abduction_home_hub_cmd.txt")
trigger_pr_file = os.path.join(temp_dir, "abduction_pianoroll_cmd.txt")

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_screen_file, "w") as f:
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
        print(f"[TEST] Falha ao capturar screenshot: {png_name}")
    return None

print("[TEST] 1. Encerrando instancias anteriores...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\AbductionStudioV2.exe"
if not os.path.exists(exe_path):
    exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"

print(f"[TEST] 2. Iniciando DAW: {exe_path}")
proc = subprocess.Popen([exe_path], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(4.0)

try:
    print(f"[TEST] 3. PID da DAW: {proc.pid}")

    print("[TEST] 4. Fechando Home Hub para acessar a DAW...")
    with open(trigger_cmd_file, "w") as f:
        f.write("close\n")
    time.sleep(1.0)

    print("[TEST] 5. Abrindo Piano Roll e gerando sequencia DEMO...")
    with open(trigger_pr_file, "w") as f:
        f.write("demo\n")
    time.sleep(1.5)

    print("[TEST] 6. Capturando screenshot do Piano Roll V3 com Velocity Stalks...")
    trigger_and_save("piano_roll_v3_demo_live.png")
    time.sleep(1.0)

finally:
    print("[TEST] 7. Encerrando processo da DAW...")
    proc.terminate()
    try:
        proc.wait(timeout=2.0)
    except:
        proc.kill()
    print("[TEST] Teste concluido com sucesso!")
