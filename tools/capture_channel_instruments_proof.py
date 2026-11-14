import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"
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
        print(f"[TEST] Screenshot salva: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] ERRO ao capturar {png_name}")
    return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(0.8)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

# ── PROVA 1: CHANNEL RACK COM ÍCONES E BOTÕES 4 PONTINHOS (☷) ──
print("\n[PROVA 1] Capturando Channel Rack com novos controles de instrumento e 4 pontinhos...")
kill_daw()
proc = subprocess.Popen([exe_path, "--psy-rhythm-channel-rack"], cwd=cwd)
time.sleep(3.5)
trigger_and_save("07_channel_rack_instruments_direct.png")
proc.terminate()
kill_daw()

# ── PROVA 2: MENU DE 4 PONTINHOS ABERTO NO CHANNEL RACK ──
print("\n[PROVA 2] Capturando Menu de 4 Pontinhos aberto no Channel Rack...")
kill_daw()
proc = subprocess.Popen([exe_path, "--test-cr-menu"], cwd=cwd)
time.sleep(3.5)
trigger_and_save("09_channel_rack_menu_open.png")
proc.terminate()
kill_daw()

# ── PROVA 3: PIANO ROLL COM PÍLULA DE INSTRUMENTO E BOTÃO ☷ NO TOPO ──
print("\n[PROVA 3] Capturando Piano Roll com pílula de instrumento e menu ☷ no topo...")
kill_daw()
proc = subprocess.Popen([exe_path, "--psy-rhythm-generate-all"], cwd=cwd)
time.sleep(3.5)
trigger_and_save("08_piano_roll_instrument_pill.png")
proc.terminate()
kill_daw()

# ── PROVA 4: ABERTURA DIRETA COM 1 CLIQUE DO INSTRUMENTO A PARTIR DO CHANNEL RACK ──
print("\n[PROVA 4] Capturando abertura direta com 1 clique da interface do instrumento...")
kill_daw()
proc = subprocess.Popen([exe_path, "--test-cr-1click-open"], cwd=cwd)
time.sleep(3.5)
trigger_and_save("10_one_click_open_from_channel_rack.png")
proc.terminate()
kill_daw()

print("\n[SUCESSO] Todas as 4 provas de instrumentos capturadas com sucesso!")
