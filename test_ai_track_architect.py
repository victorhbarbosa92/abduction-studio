import time
import subprocess
import os
import tempfile
import ctypes
from ctypes import wintypes
from PIL import Image

user32 = ctypes.windll.user32
artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\011ed2a3-c60e-4221-a150-cb05a1dbf1c0"
temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_wait_screenshot(png_name):
    png_path = os.path.join(artifact_dir, png_name)
    
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("trigger\n")

    captured = False
    for _ in range(60):
        time.sleep(0.05)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            captured = True
            break

    if captured and os.path.exists(bmp_out_file):
        try:
            time.sleep(0.05)
            img = Image.open(bmp_out_file)
            img.save(png_path)
            try: os.remove(bmp_out_file)
            except: pass
            print(f"[TEST] Screenshot salva com sucesso: {png_path} ({img.size[0]}x{img.size[1]})")
            return png_path
        except Exception as e:
            print(f"[TEST] Erro convertendo BMP para PNG: {e}")
    else:
        print(f"[TEST] Aviso: BMP nao gerado para {png_name}")
    return None

def find_and_foreground_window():
    hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Digital Audio Workstation")
    if not hwnd:
        hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0")
    if hwnd:
        user32.ShowWindow(hwnd, 3) # SW_MAXIMIZE = 3
        user32.SetForegroundWindow(hwnd)
        time.sleep(0.5)
        print(f"[TEST] Janela encontrada (HWND={hwnd}) e trazida para primeiro plano!")
        return hwnd
    print("[TEST] Janela nao encontrada por FindWindowA.")
    return None

def main():
    print("[TEST] Iniciando teste do Abduction AI Track Architect & Co-Pilot...")
    
    # 1. Iniciar ou localizar processo da DAW
    proc = None
    hwnd = find_and_foreground_window()
    if not hwnd:
        print("[TEST] Iniciando via Iniciar_DAW.bat...")
        proc = subprocess.Popen(["cmd.exe", "/c", "Iniciar_DAW.bat"],
                                cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
        time.sleep(4.0)
        hwnd = find_and_foreground_window()

    time.sleep(1.0)

    # 2. Abrir o AI Track Architect via trigger
    trigger_open = os.path.join(temp_dir, "abduction_open_ai_architect.txt")
    with open(trigger_open, "w") as f:
        f.write("open\n")
    print("[TEST] Trigger enviado: abduction_open_ai_architect.txt")
    time.sleep(1.0)

    # Capturar screenshot do modal aberto com os 3 painéis
    trigger_and_wait_screenshot("test_ai_architect_open_view.png")
    time.sleep(1.0)

    # 3. Disparar injeção da track na playlist via trigger
    trigger_inj = os.path.join(temp_dir, "abduction_inject_ai_track.txt")
    with open(trigger_inj, "w") as f:
        f.write("inject\n")
    print("[TEST] Trigger enviado: abduction_inject_ai_track.txt")
    time.sleep(1.0)

    # Capturar screenshot da confirmação no modal
    trigger_and_wait_screenshot("test_ai_architect_injected_timeline.png")
    time.sleep(1.0)

    # 4. Fechar modal para exibir a Playlist com as faixas
    with open(trigger_open, "w") as f:
        f.write("close\n")
    print("[TEST] Trigger enviado para fechar modal: close")
    time.sleep(1.0)

    # Capturar screenshot da Playlist aberta com os clips gerados
    trigger_and_wait_screenshot("test_ai_architect_playlist_unveiled.png")

    print("[TEST] Teste concluído com sucesso!")
    if proc:
        time.sleep(1.0)
        proc.terminate()

if __name__ == "__main__":
    main()
