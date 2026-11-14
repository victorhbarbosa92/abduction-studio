import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\53ce9b1b-931c-47fc-86d5-4ec2ac6836e4"
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
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    return None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.5)

    print("[TEST] Iniciando AbductionStudioV2.exe com --test-deck-play...")
    proc = subprocess.Popen(["AbductionStudioV2.exe", "--test-deck-play"], cwd=".")
    time.sleep(4.0)

    print("[TEST] Capturando screenshot de reprodução ativa no Deck 1...")
    shot = trigger_and_save("abduction_dj_deck_playing.png")

    time.sleep(1.0)
    print("[TEST] Encerrando AbductionStudioV2...")
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except:
        proc.kill()

    if shot and os.path.exists(shot):
        print(f"[TEST] SUCESSO! Screenshot capturada: {shot}")

if __name__ == "__main__":
    main()
