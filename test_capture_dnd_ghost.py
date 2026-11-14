import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\c58cf74c-6c8d-41d2-869c-fa6dcaceb8ee"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_screen_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")
trigger_cmd_file = os.path.join(temp_dir, "abduction_home_hub_cmd.txt")
trigger_dnd_file = os.path.join(temp_dir, "abduction_dnd_demo.txt")

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

print("[TEST] 1. Encerrando instancias antigas...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
print(f"[TEST] 2. Iniciando DAW: {exe_path}")
proc = subprocess.Popen([exe_path], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(3.5)

try:
    print("[TEST] 3. Fechando o Home Hub para entrar na Playlist...")
    with open(trigger_cmd_file, "w") as f:
        f.write("close\n")
    time.sleep(1.2)

    print("[TEST] 4. Ativando Ghost Holografico Ciano de Drag & Drop...")
    with open(trigger_dnd_file, "w") as f:
        f.write("ghost\n")
    time.sleep(0.6)

    shot_ghost = trigger_and_save("daw_playlist_dnd_ghost_live.png")

    print("[TEST] 5. Acionando soltura do sample (Drop) na Trilha 2...")
    with open(trigger_dnd_file, "w") as f:
        f.write("drop\n")
    time.sleep(0.8)

    shot_dropped = trigger_and_save("daw_playlist_after_drop_live.png")

finally:
    print("[TEST] 6. Encerrando DAW de teste...")
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    print("[TEST] Finalizado com sucesso!")
