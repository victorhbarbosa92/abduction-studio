import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\23e2a112-bc16-40ff-b831-973e94227295"
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
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    else:
        print(f"[TEST] Falha ao capturar screenshot: {png_name}")
    return None

def run_test_and_capture(flag, output_png):
    print(f"\n--- Testando {flag} -> {output_png} ---")
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(1.0)

    exe_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
    proc = subprocess.Popen([exe_path, flag], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(3.5)

    try:
        shot = trigger_and_save(output_png)
    finally:
        subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
        time.sleep(1.0)
    return shot

# 1. Capturar Radar de Artistas Tab no Cloud Hub
run_test_and_capture("--test-radar-tab", "cloud_hub_radar_tab.png")

# 2. Capturar Conexao Harmonica DNA no Master Source Matrix
run_test_and_capture("--test-harmonic-dna", "harmonic_dna_connection.png")

# 3. Capturar Master Source Matrix Card padrão com Lookahead DNA strip & Seguir Artista
run_test_and_capture("--test-master-source-matrix", "master_source_matrix_card.png")

print("\nTodas as capturas da Etapa 3 concluidas!")
