import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\b1d97b6e-a2c2-4d2a-809e-71f0705b4ac2"
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
    for _ in range(70):
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
        return png_path, img
    return None, None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.6)

    print("[TEST] Iniciando AbductionStudioV2.exe com --test-deck-play...")
    proc = subprocess.Popen(["AbductionStudioV2.exe", "--test-deck-play"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(4.5)

    print("[TEST] Disparando captura de tela cheia...")
    shot_path, img = trigger_and_save("abduction_rekordbox_full.png")

    if img is not None:
        w, h = img.size
        print(f"[TEST] Dimensões da janela capturada: {w}x{h}")
        # Crop zoomed waveforms (central-top area)
        # Typically the waveforms span across width with Y between ~50 and ~350
        y1 = int(h * 0.05)
        y2 = int(h * 0.45)
        crop_wave = img.crop((0, y1, w, y2))
        wave_crop_path = os.path.join(artifact_dir, "abduction_rekordbox_wave_crop.png")
        crop_wave.save(wave_crop_path)
        print(f"[TEST] Crop da waveform ampliada salvo: {wave_crop_path}")

        # Crop overview / mini deck waveforms
        y3 = int(h * 0.40)
        y4 = int(h * 0.70)
        crop_overview = img.crop((0, y3, w, y4))
        overview_crop_path = os.path.join(artifact_dir, "abduction_rekordbox_overview_crop.png")
        crop_overview.save(overview_crop_path)
        print(f"[TEST] Crop dos decks/visão geral salvo: {overview_crop_path}")

    time.sleep(1.0)
    print("[TEST] Encerrando app...")
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except:
        proc.kill()

if __name__ == "__main__":
    main()
