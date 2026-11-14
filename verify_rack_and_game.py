import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity\brain\160de739-07ad-4f60-92b1-04f0eafabcd0"
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
            time.sleep(0.5)
            captured = True
            break

    if captured:
        img = Image.open(bmp_out_file)
        png_path = os.path.join(artifact_dir, png_name)
        img.save(png_path)
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
        try: os.remove(bmp_out_file)
        except: pass
        return png_path
    print(f"[TEST] Falha ao capturar screenshot: {png_name}")
    return None

def test_feature(flag, png_name, duration=5.0):
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(1.0)

    work_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"
    exe_path = os.path.join(work_dir, "AbductionStudioV2.exe")

    args = [exe_path]
    if flag:
        args.append(flag)

    print(f"[TEST] Iniciando: {' '.join(args)}")
    proc = subprocess.Popen(args, cwd=work_dir)
    time.sleep(duration)

    # Verifica se ainda está em execução (não fechou inesperadamente)
    poll_res = proc.poll()
    if poll_res is not None:
        print(f"[TEST] ERRO: Processo encerrou com código {poll_res} antes de capturar!")
        return False

    print(f"[TEST] Processo está rodando perfeitamente (PID {proc.pid}). Solicitando captura...")
    saved = trigger_and_save(png_name)

    # Encerra graciosamente
    try:
        proc.terminate()
        proc.wait(timeout=2.0)
    except:
        os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")

    return (saved is not None)

if __name__ == "__main__":
    print("=== TESTE 1: INICIALIZAÇÃO LIMPA (PADRÃO 1:1 REKORDBOX COM MENU EXIBIR) ===")
    t1 = test_feature(None, "verify_clean_startup.png", 4.0)

    print("=== TESTE 2: RACK 19\" MODULAR DE ESTÚDIO (PREVENÇÃO DE CRASH) ===")
    t2 = test_feature("--test-rack-19", "verify_rack_19_opened.png", 4.5)

    print("=== TESTE 3: MODO GAME & DJ ACADEMY (CONSOLE 3 CARDS & VISOR ZERO COLISÃO) ===")
    t3 = test_feature("--test-game-mode", "verify_game_mode_active.png", 5.0)

    print(f"=== RESULTADOS: Clean={t1}, Rack19={t2}, GameMode={t3} ===")

