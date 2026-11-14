import os
import sys
import time
import subprocess
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity\brain\b9e05024-c94b-463e-ab5e-730f4a59ba8d"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
    if os.path.exists(bmp_file):
        try: os.remove(bmp_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("trigger\n")

    captured = False
    for _ in range(60):
        time.sleep(0.1)
        if os.path.exists(bmp_file) and os.path.getsize(bmp_file) > 1000:
            captured = True
            break

    if captured:
        png_path = os.path.join(artifact_dir, png_name)
        img = Image.open(bmp_file)
        img.save(png_path)
        try: os.remove(bmp_file)
        except: pass
        print(f"[TEST] Live Screenshot capturada com sucesso: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

def crop_box(src_png, out_png, box):
    p = os.path.join(artifact_dir, src_png)
    if os.path.exists(p):
        img = Image.open(p)
        cropped = img.crop(box)
        out_p = os.path.join(artifact_dir, out_png)
        cropped.save(out_p)
        print(f"[TEST] Crop salvo: {out_png} ({cropped.size[0]}x{cropped.size[1]})")

def run_capture(flag, img_name):
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"], capture_output=True)
    time.sleep(1.0)

    exe_path = r"c:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\AbductionStudioV2.exe"
    cwd = r"c:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

    print(f"[TEST] Iniciando AbductionStudioV2 com {flag}...")
    proc = subprocess.Popen([exe_path, flag], cwd=cwd)
    time.sleep(3.5)

    res = trigger_and_save(img_name)

    try:
        proc.terminate()
        proc.wait(timeout=2.0)
    except:
        proc.kill()
    time.sleep(1.0)
    return res

if __name__ == "__main__":
    # 1. Captura com tela padrão mostrando os novos botões 🔮 DIG DEEPER no Omnibar e Hero Box
    run_capture("--test-search-modal", "live_hub_dig_deeper_default.png")

    # 2. Captura com a visão DIG DEEPER ativa mostrando as 23 almas gêmeas acústicas
    run_capture("--test-dig-deeper", "live_hub_dig_deeper_active.png")

    # 3. Crops detalhados
    # Crop da visão Dig Deeper no painel direito
    crop_box("live_hub_dig_deeper_active.png", "crop_dig_deeper_matches.png", (1030, 90, 1895, 960))
    # Crop do Hero Box com o botão Dig Deeper
    crop_box("live_hub_dig_deeper_default.png", "crop_hero_box_dig_deeper.png", (1030, 90, 1895, 340))
    # Crop do Omnibar com o novo botão 🔮 DIG DEEPER
    crop_box("live_hub_dig_deeper_default.png", "crop_omnibar_dig_deeper.png", (25, 45, 1895, 130))

    print("[TEST] Todas as capturas e recortes foram concluídos!")

