import os
import sys
import time
import subprocess
import tempfile
from PIL import Image

def main():
    print("[TEST] Iniciando Abduction Studio V2...")
    exe_path = os.path.abspath("AbductionStudioV2.exe")
    if not os.path.exists(exe_path):
        print(f"[ERRO] Executavel nao encontrado: {exe_path}")
        sys.exit(1)

    proc = subprocess.Popen([exe_path], cwd=os.path.dirname(exe_path))
    time.sleep(3.5)

    temp_dir = tempfile.gettempdir()
    pm_trigger = os.path.join(temp_dir, "abduction_open_plugin_manager.txt")
    sc_req = os.path.join(temp_dir, "abduction_screenshot_req.txt")
    sc_out = os.path.join(temp_dir, "abduction_screenshot.bmp")

    if os.path.exists(sc_out):
        try: os.remove(sc_out)
        except: pass

    print("[TEST] Disparando abertura do Gerenciador de Plugins VST3 / CLAP...")
    with open(pm_trigger, "w") as f:
        f.write("open")

    time.sleep(1.5)

    print("[TEST] Solicitando captura de tela via Framebuffer...")
    with open(sc_req, "w") as f:
        f.write("snap")

    # Esperar captura
    for _ in range(30):
        time.sleep(0.2)
        if os.path.exists(sc_out) and os.path.getsize(sc_out) > 1000:
            break

    if os.path.exists(sc_out) and os.path.getsize(sc_out) > 1000:
        print(f"[TEST] Captura realizada com sucesso! ({os.path.getsize(sc_out)} bytes)")
        dest_png = os.path.join(os.path.dirname(exe_path), "vst3_clap_plugin_manager_verified.png")
        img = Image.open(sc_out)
        img.save(dest_png, "PNG")
        print(f"[TEST] Imagem salva em: {dest_png}")
    else:
        print("[ERRO] Screenshot nao gerada a tempo!")

    print("[TEST] Encerrando Abduction Studio V2...")
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except:
        proc.kill()

    print("[TEST] Teste concluido!")

if __name__ == "__main__":
    main()
