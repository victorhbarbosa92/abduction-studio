import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dirs = [
    r"C:\Users\o_vic\.gemini\antigravity-ide\brain\165222e1-f84a-46d0-943d-8225d4d46ac1",
    r"C:\Users\o_vic\.gemini\antigravity-ide\brain\82565665-83af-46c1-a78e-9bde9d9771f7",
    r"C:\Users\o_vic\.gemini\antigravity-ide\brain\3d62f279-b226-44c7-a204-a38b82603343",
    r"C:\Users\o_vic\.gemini\antigravity-ide\brain\7619a661-3c45-4dd0-9415-c7698c00afce",
    r"C:\Users\o_vic\.gemini\antigravity-ide\brain\5932d05e-9444-4275-a495-4be6d3e6f578",
    r"C:\Users\o_vic\.gemini\antigravity-ide\brain\787ac822-c556-4e45-9247-d1cb82f2f490"
]
for ad in artifact_dirs:
    os.makedirs(ad, exist_ok=True)

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
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            captured = True
            break

    if captured:
        img = Image.open(bmp_out_file)
        ret_path = None
        for ad in artifact_dirs:
            png_path = os.path.join(ad, png_name)
            img.save(png_path)
            if ret_path is None: ret_path = png_path
            print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
        try: os.remove(bmp_out_file)
        except: pass
        return ret_path
    return None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.5)

    print("[TEST] Iniciando AbductionStudioV2.exe com --test-crates-and-pads...")
    proc = subprocess.Popen(["AbductionStudioV2.exe", "--test-crates-and-pads"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(5.0)

    print("[TEST] Disparando captura...")
    shot = trigger_and_save("rekordbox_crates_and_pads_verified.png")

    time.sleep(1.0)
    print("[TEST] Encerrando app...")
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except:
        proc.kill()

    if shot and os.path.exists(shot):
        print(f"[TEST] SUCESSO! Screenshot gerada em: {shot}")
    else:
        print("[TEST] Falha ao capturar screenshot.")

if __name__ == "__main__":
    main()
