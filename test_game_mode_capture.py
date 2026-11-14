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
            time.sleep(0.6)  # Give time to flush disk
            captured = True
            break

    if captured:
        png_path = os.path.join(artifact_dir, png_name)
        img = Image.open(bmp_out_file)
        img.save(png_path)
        print(f"[TEST] Screenshot salva com sucesso: {png_path} ({img.size[0]}x{img.size[1]})")

        w, h = img.size
        crop_box = (int(w * 0.08), int(h * 0.12), int(w * 0.92), int(h * 0.88))
        cropped = img.crop(crop_box)
        crop_png = os.path.join(artifact_dir, "game_mode_centered_crop.png")
        cropped.save(crop_png)
        print(f"[TEST] Cropped screenshot salva: {crop_png}")

        detail_box = (int(w * 0.09), int(h * 0.18), int(w * 0.91), int(h * 0.81))
        detail_crop = img.crop(detail_box)
        detail_png = os.path.join(artifact_dir, "game_mode_cards_detail.png")
        detail_crop.save(detail_png)
        print(f"[TEST] Detail screenshot salva: {detail_png}")

        return png_path
    print("[TEST] Falha ao capturar screenshot.")
    return None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(1.0)

    work_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"
    exe_path = os.path.join(work_dir, "AbductionStudioV2.exe")

    print(f"[TEST] Iniciando {exe_path} --test-game-mode...")
    proc = subprocess.Popen([exe_path, "--test-game-mode"], cwd=work_dir)
    time.sleep(6.5)

    print("[TEST] Disparando captura de tela do Modo Jogo...")
    shot = trigger_and_save("game_mode_centered_window.png")
    
    time.sleep(1.0)
    try:
        proc.terminate()
        proc.wait(timeout=3.0)
    except:
        proc.kill()

if __name__ == "__main__":
    main()
