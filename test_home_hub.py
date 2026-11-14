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

    # Captura 1: Home Hub aberto na inicialização
    shot1 = trigger_and_save("home_hub_live_startup.png")

    # Fecha o Home Hub via trigger para testar entrada na DAW
    print("[TEST] 4. Fechando o Home Hub para entrar na DAW...")
    with open(trigger_cmd_file, "w") as f:
        f.write("close\n")
    time.sleep(1.2)

    # Captura 2: DAW limpa em execução após fechar o Home Hub
    shot2 = trigger_and_save("daw_workspace_after_close.png")

    # Reabre o Home Hub via trigger
    print("[TEST] 5. Reabrindo o Home Hub...")
    with open(trigger_cmd_file, "w") as f:
        f.write("open\n")
    time.sleep(1.2)

    # Captura 3: Home Hub reaberto
    shot3 = trigger_and_save("home_hub_reopened.png")

finally:
    print("[TEST] Encerrando DAW de teste...")
    proc.terminate()
    try:
        proc.wait(timeout=2.0)
    except:
        proc.kill()
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    print("[TEST] Concluído com sucesso!")
