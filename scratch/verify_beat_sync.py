import subprocess
import time
import os
import tempfile
from PIL import Image

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_file = os.path.join(temp_dir, "abduction_screenshot.bmp")
output_png = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\99c43430-c66d-4980-bd72-557c18180e32\dj_sync_verified.png"

exe_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"
exe_path = os.path.join(exe_dir, "build", "Release", "AbductionStudioV2.exe")

if os.path.exists(bmp_file):
    try:
        os.remove(bmp_file)
    except Exception:
        pass

print("Starting app with --test-beat-sync...")
proc = subprocess.Popen([exe_path, "--test-beat-sync"], cwd=exe_dir)
time.sleep(4.0)

# Create trigger file
print("Triggering screenshot...")
with open(trigger_file, "w") as f:
    f.write("snap\n")

# Wait for bmp
for _ in range(30):
    time.sleep(0.2)
    if os.path.exists(bmp_file):
        print("Found bmp!")
        break

time.sleep(0.5)
proc.terminate()
try:
    proc.wait(timeout=2)
except Exception:
    proc.kill()

if os.path.exists(bmp_file):
    im = Image.open(bmp_file)
    im.save(output_png, "PNG")
    print(f"Successfully saved native capture to {output_png} (Size: {im.size})")
else:
    print("Screenshot bmp was not generated!")
