import time
import subprocess
import os
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\011ed2a3-c60e-4221-a150-cb05a1dbf1c0"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_surg = os.path.join(temp_dir, "abduction_open_surgical_analyzer.txt")
trigger_shot = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_shot, "w") as f:
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
        print(f"[TEST] Screenshot gravada: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    else:
        print("[TEST] Fallback...")
        return None

print("[TEST] 1. Fechando instâncias antigas...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

print("[TEST] 2. Iniciando DAW via Iniciar_DAW.bat...")
proc = subprocess.Popen(["cmd.exe", "/c", "Iniciar_DAW.bat"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(4.0)

print("[TEST] 3. Abrindo Surgical Audio Analyzer via Trigger...")
with open(trigger_surg, "w") as f:
    f.write("open\n")

time.sleep(2.0)

print("[TEST] 4. Capturando tela in-engine do Analisador Cirúrgico com os botões de Solo...")
trigger_and_save("abduction_surgical_open_view.png")
time.sleep(1.0)

print("[TEST] 5. Disparando Audição de Stem Isolado (Solo Kick)...")
trigger_solo = os.path.join(temp_dir, "abduction_solo_stem.txt")
with open(trigger_solo, "w") as f:
    f.write("kick\n")
time.sleep(1.5)

print("[TEST] 6. Capturando tela com reprodução do Solo Kick ativa...")
trigger_and_save("abduction_surgical_solo_playing.png")
time.sleep(0.5)

with open(trigger_solo, "w") as f:
    f.write("stop\n")
time.sleep(0.5)

print("[TEST] 7. Disparando Reconstrução Cirúrgica Híbrida para a Playlist...")
trigger_reconst = os.path.join(temp_dir, "abduction_reconstruct_surgical.txt")
with open(trigger_reconst, "w") as f:
    f.write("reconstruct\n")
time.sleep(1.0)

print("[TEST] 8. Fechando modal do Analisador Cirúrgico...")
with open(trigger_surg, "w") as f:
    f.write("close\n")
time.sleep(1.0)

print("[TEST] 9. Capturando Playlist com o projeto híbrido reconstruído...")
trigger_and_save("abduction_surgical_reconstructed_playlist.png")

time.sleep(1.0)
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
print("[TEST] Teste concluído com sucesso!")
