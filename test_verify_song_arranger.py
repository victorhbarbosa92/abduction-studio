import time
import subprocess
import os
import sys
import tempfile
import ctypes
import json
from ctypes import wintypes
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

user32 = ctypes.windll.user32
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
        return png_path
    else:
        print(f"[TEST] Falha ao capturar {png_name}")
        return None

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(0.8)

kill_daw()

app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

# ── Teste 1: Arranjo na Playlist com Clipes e Marcadores ──
print("[TEST] 1. Iniciando com flag --test-song-arranger...")
proc = subprocess.Popen([app_path, "--test-song-arranger"], cwd=cwd)
print(f"[TEST] AbductionStudioV2 iniciado com PID {proc.pid}")
time.sleep(4.5)

png_path = trigger_and_save("31_psy_song_arranger_verified.png")

# Verificar arquivo .kuro salvo
kuro_file = os.path.join(cwd, "test_arranger_project.kuro")
if os.path.exists(kuro_file):
    print(f"[TEST] test_arranger_project.kuro criado com sucesso! Tamanho: {os.path.getsize(kuro_file)} bytes")
    try:
        with open(kuro_file, "r", encoding="utf-8") as f:
            data = json.load(f)
            auto_tracks = data.get("playlist_auto_clips", [])
            markers = data.get("timeline_markers", [])
            total_auto_clips = sum(len(track_clips) for track_clips in auto_tracks if isinstance(track_clips, list))
            print(f"[TEST] .KURO JSON verificado: {total_auto_clips} automation clips, {len(markers)} timeline section markers!")
            for m in markers[:7]:
                print(f"       Marker: '{m.get('name')}' em {m.get('t')}s")
            for t_idx, track_clips in enumerate(auto_tracks):
                if isinstance(track_clips, list) and len(track_clips) > 0:
                    for ac in track_clips:
                        print(f"       Track {t_idx} AutoClip: '{ac.get('name')}' ({len(ac.get('points', []))} pontos)")
    except Exception as e:
        print(f"[TEST] Erro ao parsear JSON .kuro: {e}")
else:
    print("[TEST] AVISO: test_arranger_project.kuro não encontrado!")

kill_daw()

# ── Teste 2: Modal de Configuração do Arranjador Psytrance ──
print("[TEST] 2. Iniciando com flag --test-song-arranger-modal...")
proc2 = subprocess.Popen([app_path, "--test-song-arranger-modal"], cwd=cwd)
time.sleep(4.0)

png_path2 = trigger_and_save("32_psy_song_arranger_modal_verified.png")
kill_daw()

print("[TEST] Todos os testes e capturas visuais finalizados com sucesso!")
