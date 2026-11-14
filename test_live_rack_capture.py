import time
import subprocess
import os
import tempfile
from PIL import Image, ImageFile
ImageFile.LOAD_TRUNCATED_IMAGES = True

current_conv_id = "24ffefe9-1e17-4c77-bc66-417fa648fcd0"
artifact_dir = os.path.join(r"C:\Users\o_vic\.gemini\antigravity\brain", current_conv_id)
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
    for _ in range(80):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 20000000:
            time.sleep(0.6)
            captured = True
            break

    if captured:
        png_path = os.path.join(artifact_dir, png_name)
        img = Image.open(bmp_out_file)
        img.save(png_path)
        print(f"[TEST] Screenshot salva com sucesso: {png_path} ({img.size[0]}x{img.size[1]})")

        w, h = img.size

        # 1. Crop do Rack de Performance (Área Central Superior)
        rack_box = (0, int(h * 0.015), w, int(h * 0.28))
        cropped_rack = img.crop(rack_box)
        crop_rack_png = os.path.join(artifact_dir, "crop_live_performance_rack.png")
        cropped_rack.save(crop_rack_png)
        print(f"[TEST] Cropped Live Rack salva: {crop_rack_png}")

        # 2. Crop do Mixer Central com os novos botões rápidos [ECHO] e [303]
        mixer_box = (int(w * 0.36), int(h * 0.26), int(w * 0.64), int(h * 0.98))
        cropped_mixer = img.crop(mixer_box)
        crop_mixer_png = os.path.join(artifact_dir, "crop_mixer_performance_buttons.png")
        cropped_mixer.save(crop_mixer_png)
        print(f"[TEST] Cropped Central Mixer salva: {crop_mixer_png}")

        return png_path
    print("[TEST] Falha ao capturar screenshot.")
    return None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(1.0)

    work_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"
    exe_path = os.path.join(work_dir, "AbductionStudioV2.exe")

    print(f"[TEST] Iniciando {exe_path} --test-live-rack...")
    proc = subprocess.Popen([exe_path, "--test-live-rack"], cwd=work_dir)
    time.sleep(6.5)

    print("[TEST] Disparando captura de tela do Live Rack...")
    shot = trigger_and_save("live_performance_rack_full.png")

    time.sleep(1.0)
    try:
        proc.terminate()
        proc.wait(timeout=3.0)
    except:
        proc.kill()

if __name__ == "__main__":
    main()

