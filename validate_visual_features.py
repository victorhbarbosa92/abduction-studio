import time
import subprocess
import os
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\aed0c282-b904-426e-be49-1194c3ac0a67"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_shot = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_capture(filename):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass
    
    with open(trigger_shot, "w") as f:
        f.write("trigger\n")
    
    captured = False
    for _ in range(80):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            captured = True
            break
            
    if captured:
        png_path = os.path.join(artifact_dir, filename)
        img = Image.open(bmp_out_file)
        img.save(png_path)
        try: os.remove(bmp_out_file)
        except: pass
        print(f"[CAPTURE] Screenshot salva com sucesso: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    else:
        print(f"[CAPTURE] Falha ao capturar {filename}")
        return None

def run_test(arg, capture_name, wait_sec=4.0):
    print(f"\n[TEST] 1. Encerrando instâncias ativas...")
    subprocess.run(["powershell", "-NoProfile", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(0.8)
    
    print(f"[TEST] 2. Iniciando AbductionStudioV2 com {arg}...")
    cwd_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"
    exe_path = os.path.join(cwd_path, "AbductionStudioV2.exe")
    proc = subprocess.Popen([exe_path, arg], cwd=cwd_path)
    
    print(f"[TEST] 3. Aguardando {wait_sec}s para renderização dos frames...")
    time.sleep(wait_sec)
    
    print(f"[TEST] 4. Disparando captura de tela interna do OpenGL...")
    res = trigger_and_capture(capture_name)
    
    time.sleep(1.0)
    proc.terminate()
    try: proc.wait(timeout=2.0)
    except: proc.kill()
    time.sleep(0.5)
    return res

if __name__ == "__main__":
    print("=== INICIANDO VALIDAÇÃO VISUAL DO ABDUCTION STUDIO V2 ===")
    
    # 6. Validação do Master Bus Multiband Mastering & Stems Bounce Multifaixa
    shot6 = run_test("--test-master-processing-and-stems", "val_master_multiband_stems.png", wait_sec=5.0)
    
    print("\n=== VALIDAÇÃO VISUAL CONCLUÍDA ===")
    print(f"Master Bus Maximus & Stems Bounce: {shot6}")


