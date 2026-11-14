import os
import sys
import time
import subprocess
import tempfile
from PIL import Image

temp_dir = tempfile.gettempdir()
trigger_p = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out = os.path.join(temp_dir, "abduction_screenshot.bmp")

if os.path.exists(trigger_p):
    try: os.remove(trigger_p)
    except: pass
if os.path.exists(bmp_out):
    try: os.remove(bmp_out)
    except: pass

print("[TEST] Launching AbductionStudioV2.exe --open-dj...")
proc = subprocess.Popen(["./AbductionStudioV2.exe", "--open-dj"])

print("[TEST] Waiting 4 seconds for initialization...")
time.sleep(4)

print("[TEST] Triggering OpenGL framebuffer capture...")
with open(trigger_p, "w") as f:
    f.write("trigger\n")

captured = False
for _ in range(40):
    time.sleep(0.1)
    if os.path.exists(bmp_out) and os.path.getsize(bmp_out) > 1000:
        captured = True
        break

if captured:
    print(f"[TEST] BMP generated successfully! Size: {os.path.getsize(bmp_out)} bytes.")
    img = Image.open(bmp_out)
    print(f"[TEST] Resolution: {img.size[0]}x{img.size[1]}")
    
    art_path = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\787ac822-c556-4e45-9247-d1cb82f2f490\abduction_studio_live_validation.png"
    desk_path = r"C:\Users\o_vic\Desktop\Referencias_TopBar_UI\12_Validacao_Live_Ambiente.png"
    
    img.save(art_path)
    img.save(desk_path)
    print(f"[TEST] Saved to artifact: {art_path}")
    print(f"[TEST] Saved to desktop: {desk_path}")
    
    try: os.remove(bmp_out)
    except: pass
else:
    print("[TEST] Warning: BMP capture timed out.")

print("[TEST] Terminating AbductionStudioV2...")
proc.terminate()
try:
    proc.wait(timeout=3)
except:
    proc.kill()

print("[TEST] Validation run complete.")
