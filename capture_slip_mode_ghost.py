import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\7793848f-82b4-4d0c-af9a-f0d04c3cd4e2"
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
        print(f"[TEST] Screenshot salva: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    else:
        print(f"[TEST] Falha ao capturar screenshot: {png_name}")
    return None

print("[TEST] 1. Encerrando instancias anteriores...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

print("[TEST] 2. Iniciando com --test-slip-mode...")
proc = subprocess.Popen(["./AbductionStudioV2.exe", "--test-slip-mode"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

time.sleep(3.5)

# Fecha o Home Hub se estiver aberto
cmd_file = os.path.join(temp_dir, "abduction_home_hub_cmd.txt")
with open(cmd_file, "w") as f:
    f.write("close\n")

time.sleep(2.0)

print("[TEST] 3. Capturando tela do DJ Studio com Ghost Playhead no Slip Mode...")
path1 = trigger_and_save("dj_slip_mode_ghost_live.png")

if path1 and os.path.exists(path1):
    img = Image.open(path1)
    # Crop da regiao central superior das waveforms mostrando o Playhead e o Ghost Playhead
    crop_wf = img.crop((40, 28, 1240, 260))
    crop_wf_path = os.path.join(artifact_dir, "dj_slip_ghost_cropped.png")
    crop_wf.save(crop_wf_path)
    print(f"[TEST] Crop da Waveform com Ghost Playhead salvo: {crop_wf_path} ({crop_wf.size[0]}x{crop_wf.size[1]})")

    # Crop dos controles de Deck mostrando o botao SLIP ATIVO piscando e jogwheel
    crop_deck = img.crop((40, 250, 600, 680))
    crop_deck_path = os.path.join(artifact_dir, "dj_slip_deck_controls_cropped.png")
    crop_deck.save(crop_deck_path)
    print(f"[TEST] Crop dos Controles com SLIP ativo salvo: {crop_deck_path} ({crop_deck.size[0]}x{crop_deck.size[1]})")

time.sleep(0.5)
proc.terminate()
try: proc.wait(timeout=2)
except: proc.kill()

print("[TEST] 4. Concluido com sucesso!")
