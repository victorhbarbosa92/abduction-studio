import time
import subprocess
import os
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\011ed2a3-c60e-4221-a150-cb05a1dbf1c0"
temp_dir = tempfile.gettempdir()
trigger_surg = os.path.join(temp_dir, "abduction_open_surgical_analyzer.txt")
trigger_reconst = os.path.join(temp_dir, "abduction_reconstruct_surgical.txt")
trigger_shot = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_shot, "w") as f:
        f.write("trigger\n")

    for _ in range(60):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            png_path = os.path.join(artifact_dir, png_name)
            img = Image.open(bmp_out_file)
            img.save(png_path)
            try: os.remove(bmp_out_file)
            except: pass
            print(f"[TEST] Screenshot gravada: {png_path} ({img.size[0]}x{img.size[1]})")
            return png_path

    print("[TEST] Falha ao capturar screenshot")
    return None

print("[TEST] 1. Fechando instâncias antigas...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

print("[TEST] 2. Iniciando DAW...")
subprocess.Popen(["cmd.exe", "/c", "Iniciar_DAW.bat"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(5.0)

print("[TEST] 3. Disparando Reconstrução Híbrida Direta...")
with open(trigger_reconst, "w") as f:
    f.write("reconstruct\n")
time.sleep(2.0)

print("[TEST] 4. Capturando Playlist Reconstruída...")
trigger_and_save("abduction_surgical_reconstructed_playlist.png")

time.sleep(1.0)
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
print("[TEST] Concluído!")
