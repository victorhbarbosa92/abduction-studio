import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")
target_artifact = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\a7531291-b5ae-4563-8e85-584c2f47d8c3\vortex_dj_skin_verified.png"

if os.path.exists(bmp_out_file):
    try: os.remove(bmp_out_file)
    except: pass
if os.path.exists(trigger_file):
    try: os.remove(trigger_file)
    except: pass

print("[TEST] Launching AbductionStudioV2.exe --open-dj...")
proc = subprocess.Popen(["./AbductionStudioV2.exe", "--open-dj"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")

time.sleep(3.0)

print("[TEST] Triggering internal OpenGL screenshot...")
with open(trigger_file, "w") as f:
    f.write("trigger\n")

captured = False
for i in range(40):
    time.sleep(0.1)
    if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 10000:
        captured = True
        break

if captured:
    img = Image.open(bmp_out_file)
    img.save(target_artifact)
    try: os.remove(bmp_out_file)
    except: pass
    print(f"[TEST] SUCCESS: Screenshot saved to {target_artifact} ({img.size[0]}x{img.size[1]})")
else:
    print("[TEST] FAILED: Screenshot not generated.")

proc.terminate()
try:
    proc.wait(timeout=2.0)
except:
    proc.kill()
