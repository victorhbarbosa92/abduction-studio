import time
import subprocess
import os
import tempfile
from PIL import Image

current_conv_id = "24ffefe9-1e17-4c77-bc66-417fa648fcd0"
artifact_dir = os.path.join(r"C:\Users\o_vic\.gemini\antigravity\brain", current_conv_id)
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name, crop_waveform=True):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("trigger\n")

    captured = False
    for _ in range(80):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            captured = True
            break

    if captured:
        png_path = os.path.join(artifact_dir, png_name)
        img = Image.open(bmp_out_file)
        img.save(png_path)
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")

        if crop_waveform and img.size[0] >= 1200:
            # Recorta a baia das waveforms no topo
            # Em 1920x1080 ou 2560x1440, a waveform fica no terço superior
            w, h = img.size
            wf_crop = img.crop((0, int(h * 0.05), w, int(h * 0.35)))
            crop_path = os.path.join(artifact_dir, "crop_" + png_name)
            wf_crop.save(crop_path)
            print(f"[TEST] Waveform Crop salvo: {crop_path}")

        try: os.remove(bmp_out_file)
        except: pass
        return png_path
    print(f"[TEST] Falha ao capturar screenshot para {png_name}.")
    return None

def run_test(flag, name):
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(1.0)
    work_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"
    exe_path = os.path.join(work_dir, "AbductionStudioV2.exe")

    print(f"[TEST] Iniciando {exe_path} {flag}...")
    proc = subprocess.Popen([exe_path, flag], cwd=work_dir)
    time.sleep(7.0)

    print(f"[TEST] Disparando captura para {name}...")
    trigger_and_save(name)

    time.sleep(0.5)
    try:
        proc.terminate()
        proc.wait(timeout=2.0)
    except:
        proc.kill()

def main():
    # 1. Teste do modo 3-Band Multi-Lane Rekordbox (Padrão 1px contínuo nas 3 baias)
    run_test("--test-wave-3bm", "rekordbox_3band_multi_lane.png")

    # 2. Teste do modo 3-Band Fused CDJ-3000 (Fusão concêntrica sobreposta)
    run_test("--test-wave-3bf", "rekordbox_3band_fused_cdj3000.png")

    # 3. Teste da Fusão Híbrida (Mesh + Neon)
    run_test("--test-wave-hyb", "rekordbox_hybrid_fusion.png")

if __name__ == "__main__":
    main()

