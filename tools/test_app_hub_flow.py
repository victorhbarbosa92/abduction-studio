import os
import time
import subprocess
from PIL import Image

temp_dir = os.environ.get("TEMP", "C:\\Windows\\Temp")
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_file = os.path.join(temp_dir, "abduction_screenshot.bmp")
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"

def trigger_and_save(png_name):
    if os.path.exists(bmp_file):
        try: os.remove(bmp_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("snap\n")

    captured = False
    for _ in range(40):
        time.sleep(0.1)
        if os.path.exists(bmp_file) and os.path.getsize(bmp_file) > 1000:
            captured = True
            break

    if captured:
        png_path = os.path.join(artifact_dir, png_name)
        img = Image.open(bmp_file)
        img.save(png_path)
        try: os.remove(bmp_file)
        except: pass
        print(f"[TEST] Salvo: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Falha ao capturar {png_name}")
    return None

# Kill any existing instance
subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
time.sleep(1)

# Launch DAW
exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")

print("Waiting for DAW...")
time.sleep(3.5)

# 1. Clean desktop capture
trigger_and_save("01_abduction_desktop_hub_home.png")

proc.terminate()
print("Done.")
