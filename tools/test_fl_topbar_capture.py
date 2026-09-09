import os
import time
import subprocess
import tempfile
from PIL import Image

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_file = os.path.join(temp_dir, "abduction_screenshot.bmp")
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"

# Remove old files if existing
if os.path.exists(trigger_file):
    os.remove(trigger_file)
if os.path.exists(bmp_file):
    os.remove(bmp_file)

# Kill any existing instance
subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
time.sleep(1)

# Launch DAW
exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")

print("Waiting for DAW to render...")
time.sleep(3.5)

# Trigger screenshot
with open(trigger_file, "w") as f:
    f.write("snap\n")

print("Screenshot triggered, waiting for file...")
captured = False
for _ in range(25):
    if os.path.exists(bmp_file) and os.path.getsize(bmp_file) > 1000:
        captured = True
        break
    time.sleep(0.2)

if captured:
    time.sleep(0.3)
    img = Image.open(bmp_file)
    # Save full maximized capture
    out_full = os.path.join(artifact_dir, "fl_topbar_live_maximized.png")
    img.save(out_full)
    print(f"Saved full capture: {out_full} ({img.size[0]}x{img.size[1]})")

    # Save closeup of the double-deck topbar (y: 0 to 120)
    topbar_crop = img.crop((0, 0, img.size[0], min(140, img.size[1])))
    out_crop = os.path.join(artifact_dir, "fl_topbar_deck_closeup.png")
    topbar_crop.save(out_crop)
    print(f"Saved topbar closeup: {out_crop}")
else:
    print("Failed to capture screenshot!")

# Close app
proc.terminate()
time.sleep(1)
subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
print("Done.")
