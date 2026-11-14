import time
import subprocess
import os
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\011ed2a3-c60e-4221-a150-cb05a1dbf1c0"
temp_dir = tempfile.gettempdir()
trigger_reconst = os.path.join(temp_dir, "abduction_reconstruct_surgical.txt")
trigger_shot = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_wait():
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass
    with open(trigger_shot, "w") as f:
        f.write("trigger\n")
    for _ in range(60):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            png_path = os.path.join(artifact_dir, "abduction_surgical_reconstructed_playlist.png")
            img = Image.open(bmp_out_file)
            img.save(png_path)
            try: os.remove(bmp_out_file)
            except: pass
            print(f"[TEST] Playlist Screenshot Salva: {png_path} ({img.size[0]}x{img.size[1]})")
            return png_path
    return None

print("[TEST] 1. Matando instâncias antigas...")
subprocess.run(["powershell", "-NoProfile", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

print("[TEST] 2. Iniciando DAW via Invoke-CimMethod...")
launch_cmd = """
Invoke-CimMethod -ClassName Win32_Process -MethodName Create -Arguments @{CommandLine='C:\\Users\\o_vic\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2\\AbductionStudioV2.exe'; CurrentDirectory='C:\\Users\\o_vic\\.gemini\\antigravity-ide\\scratch\\abduction_studio_v2'}
"""
subprocess.run(["powershell", "-NoProfile", "-Command", launch_cmd])
time.sleep(5.0)

print("[TEST] 3. Disparando reconstrução cirúrgica na Playlist...")
with open(trigger_reconst, "w") as f:
    f.write("reconstruct\n")
time.sleep(2.0)

print("[TEST] 4. Capturando tela da Playlist...")
res = trigger_and_wait()
if res:
    print("[TEST] Sucesso absoluto!")
else:
    print("[TEST] Falha na captura.")

time.sleep(1.0)
subprocess.run(["powershell", "-NoProfile", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
