import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\b1269489-3dde-438a-b7d9-08532ab26e64"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_screen_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")
trigger_cmd_file = os.path.join(temp_dir, "abduction_home_hub_cmd.txt")
trigger_stepseq_file = os.path.join(temp_dir, "abduction_stepseq_cmd.txt")

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
        print(f"[TEST] Screenshot salva com sucesso: {png_path} ({img.size[0]}x{img.size[1]})")
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
    print("[TEST] 3. Disparando template_0 (Full-On Psytrance 148 BPM)...")
    with open(trigger_cmd_file, "w") as f:
        f.write("template_0\n")

    time.sleep(1.5)

    print("[TEST] 4. Focando Channel Rack / Step Sequencer...")
    with open(trigger_stepseq_file, "w") as f:
        f.write("open\n")

    time.sleep(1.5)

    print("[TEST] 5. Capturando tela do Step Sequencer com os mini-knobs de Volume, Pan e Pitch...")
    trigger_and_save("step_sequencer_mini_knobs_live.png")

finally:
    print("[TEST] 6. Encerrando DAW de teste...")
    try:
        proc.terminate()
        proc.wait(timeout=2.0)
    except:
        proc.kill()
    print("[TEST] Finalizado com sucesso!")
