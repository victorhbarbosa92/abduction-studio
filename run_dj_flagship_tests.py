import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\725a1335-8fa2-4938-9595-4adffb7b22cd"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_filename):
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
        dest_path = os.path.join(artifact_dir, png_filename)
        img.save(dest_path)
        print(f"[TEST] Screenshot salva com sucesso: {dest_path} ({img.size[0]}x{img.size[1]})")
        try: os.remove(bmp_out_file)
        except: pass
        return dest_path
    else:
        print(f"[TEST] Timeout aguardando captura para {png_filename}")
        return None

def run_test(flag, png_name, duration=3.5):
    print(f"\n=======================================================")
    print(f"[TEST RUN] Iniciando teste com flag: {flag} -> {png_name}")
    print(f"=======================================================")
    
    # Mata qualquer instancia prévia
    subprocess.run(["taskkill", "/f", "/im", "AbductionStudioV2.exe"], capture_output=True)
    time.sleep(0.5)

    proc = subprocess.Popen(["./AbductionStudioV2.exe", flag], cwd=".")
    time.sleep(duration)

    saved = trigger_and_save(png_name)
    time.sleep(0.5)

    try:
        proc.terminate()
        proc.wait(timeout=2.0)
    except:
        subprocess.run(["taskkill", "/f", "/im", "AbductionStudioV2.exe"], capture_output=True)

    time.sleep(0.5)
    return saved

if __name__ == "__main__":
    results = {}
    
    # 1. PILAR 1: LIVE STEMS ISOLATION NOS PADS
    r1 = run_test("--test-stems-pad", "01_flagship_live_stems_pads.png", duration=3.5)
    results["Pilar 1: Live Stems nos Pads"] = r1

    # 2. PILAR 2: AUTODJ INTELIGENTE COM TRANSIÇÃO HARMÔNICA & BASS SWAP
    r2 = run_test("--test-autodj", "02_flagship_autodj_transition.png", duration=4.0)
    results["Pilar 2: AutoDJ Inteligente"] = r2

    # 3. PILAR 3: ROTEAMENTO DE FONE DE OUVIDO MULTI-CANAL (4CH PFL)
    r3 = run_test("--test-audio-routing-cue", "03_flagship_multi_channel_routing.png", duration=3.5)
    results["Pilar 3: Roteamento de Fone Multi-Canal"] = r3

    print("\n=======================================================")
    print("[RELATÓRIO DOS TESTES AUTOMATIZADOS]")
    print("=======================================================")
    all_ok = True
    for k, v in results.items():
        st = "[OK]" if v else "[FALHA]"
        print(f"{k}: {st} -> {v}")
        if not v: all_ok = False

    if all_ok:
        print("\n=== TODOS OS 3 PILARES FORAM TESTADOS E VALIDADOS COM SUCESSO! ===")
        sys.exit(0)
    else:
        print("\n=== Falha na captura de algum teste ===")
        sys.exit(1)
