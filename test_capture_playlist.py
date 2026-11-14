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
            print(f"[TEST] Erro: {e}")
    return None

def main():
    print("[TEST] Disparando injeção e fechamento para visualizar Playlist...")
    # 1. Injetar track
    with open(os.path.join(temp_dir, "abduction_inject_ai_track.txt"), "w") as f:
        f.write("inject\n")
    time.sleep(0.5)

    # 2. Toggle janela para fechar
    with open(os.path.join(temp_dir, "abduction_open_ai_architect.txt"), "w") as f:
        f.write("toggle\n")
    time.sleep(0.5)

    # 3. Screenshot da Playlist
    trigger_and_wait_screenshot("test_ai_architect_playlist_unveiled.png")

if __name__ == "__main__":
    main()
