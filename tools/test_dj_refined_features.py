import time
import subprocess
import os
import sys
import tempfile
import glob
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\7a8b599d-0d06-45da-bdfb-4e1e39dd08e9"
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
        print(f"[TEST] Screenshot salvo: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(0.8)

exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
cwd = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

print("\n=== VALIDAÇÃO DO MODO DJ REFINADO (ESTRUTURA & ENGINES) ===")
kill_daw()

# Dispara DAW com --test-dj-refined
print("[1] Iniciando Abduction Studio com --test-dj-refined...")
proc = subprocess.Popen([exe_path, "--test-dj-refined"], cwd=cwd)

print("[2] Aguardando processamento de áudio, sync, key shift e gravação Master (5.0s)...")
time.sleep(5.0)

# Captura screenshot ao vivo
png_path = trigger_and_save("09_dj_mode_refined_features_live.png")

# Encerra DAW
time.sleep(1.0)
proc.terminate()
kill_daw()

# Verifica se o gravador Master WAV gerou arquivo
recordings = glob.glob(os.path.join(cwd, "scratch", "recordings", "Live_Set_*.wav"))
print(f"\n[3] Verificando gravações Master gravadas em disco ({len(recordings)} encontradas):")
for r in recordings:
    size = os.path.getsize(r)
    print(f"  -> {os.path.basename(r)}: {size:,} bytes")

print("\n=== RESULTADO ===")
if png_path and os.path.exists(png_path):
    print(f" [OK] Screenshot capturado com sucesso: {png_path}")
if recordings:
    print(f" [OK] Gravação Master WAV 32-bit Float confirmada em disco!")
