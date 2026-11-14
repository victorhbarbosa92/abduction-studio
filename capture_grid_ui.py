import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\7793848f-82b4-4d0c-af9a-f0d04c3cd4e2"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_screen_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

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
        print(f"[TEST] Screenshot salva: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    else:
        print(f"[TEST] Falha ao capturar screenshot: {png_name}")
    return None

print("[TEST] 1. Encerrando instancias anteriores...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

print("[TEST] 2. Iniciando com --test-grid-editor...")
proc = subprocess.Popen(["./AbductionStudioV2.exe", "--test-grid-editor"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

time.sleep(3.0)

# Fecha o Home Hub se estiver aberto
cmd_file = os.path.join(temp_dir, "abduction_home_hub_cmd.txt")
with open(cmd_file, "w") as f:
    f.write("close\n")

time.sleep(1.5)

print("[TEST] 3. Capturando tela com Beatgrid Editor aberto...")
path1 = trigger_and_save("dj_grid_editor_with_button.png")

if path1 and os.path.exists(path1):
    img = Image.open(path1)
    # Crop da regiao superior central (waveform + grid editor drawer)
    crop_ge = img.crop((40, 28, 1240, 240))
    crop_ge_path = os.path.join(artifact_dir, "dj_grid_editor_cropped.png")
    crop_ge.save(crop_ge_path)
    print(f"[TEST] Crop do Grid Editor salvo: {crop_ge_path}")

time.sleep(0.5)
proc.terminate()
try: proc.wait(timeout=2)
except: proc.kill()

print("[TEST] 4. Concluido com sucesso!")
