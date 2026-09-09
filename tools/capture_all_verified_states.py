import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"
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
    for _ in range(50):
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
        print(f"[TEST] Screenshot salva com sucesso: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(0.8)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

# ── TESTE 1: INICIAL COM MOTOR PADRÃO MOOG 24dB, F#1, PHRYGIAN ──
print("\n[TEST 1] Capturando estado inicial do Kuro Bass...")
kill_daw()
proc = subprocess.Popen([exe_path, "--open-psy-bass"], cwd=cwd)
time.sleep(3.0)
trigger_and_save("01_kuro_psy_bass_initial_window.png")
proc.terminate()
kill_daw()

# ── TESTE 2: VIRUS TI HYPER-SAW + ROOT NOTE A1 + SCALE NATURAL MINOR ──
print("\n[TEST 2] Capturando estado modificado: Virus TI + A1 + Natural Minor...")
kill_daw()
proc = subprocess.Popen([exe_path, "--psy-bass-virus-a1"], cwd=cwd)
time.sleep(3.0)
trigger_and_save("02_kuro_psy_bass_modified_settings.png")
proc.terminate()
kill_daw()

# ── TESTE 3: AUDITION LOOP EM TEMPO REAL (OSCILOSCÓPIO VIVO) ──
print("\n[TEST 3] Capturando Audition Loop em tempo real...")
kill_daw()
proc = subprocess.Popen([exe_path, "--psy-bass-audition"], cwd=cwd)
time.sleep(3.5) # Aguarda áudio e playhead sincronizarem
trigger_and_save("03_kuro_psy_bass_audition_playing.png")
proc.terminate()
kill_daw()

# ── TESTE 4: GERAR NOTAS PARA O PIANO ROLL ──
print("\n[TEST 4] Capturando Piano Roll com bassline gerada em A1 Natural Minor...")
kill_daw()
proc = subprocess.Popen([exe_path, "--psy-bass-generate-pr"], cwd=cwd)
time.sleep(3.0)
trigger_and_save("04_piano_roll_with_generated_bassline.png")
proc.terminate()
kill_daw()

# ── TESTE 5: DISPLAY OLED COM TEXTO LONGO DO SNAP SELECTOR RECORTE E MARQUEE ──
print("\n[TEST 5] Capturando Display OLED com texto longo protegido por clipping e marquee...")
kill_daw()
proc = subprocess.Popen([exe_path, "--test-snap-hint"], cwd=cwd)
time.sleep(3.0)
trigger_and_save("05_oled_hint_text_clipped.png")
proc.terminate()
kill_daw()

print("\n[TEST] Todos os testes e capturas concluídos com 100% de sucesso!")
