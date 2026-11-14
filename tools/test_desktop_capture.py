import socket
import time
import subprocess
import os
import shutil
from PIL import Image

def send_udp_cmd(cmd):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(cmd.encode('utf-8'), ("127.0.0.1", 9000))
    sock.close()

# Start DAW if not running
proc = subprocess.Popen(["build/Release/AbductionStudioV2.exe"], cwd=".")
time.sleep(2.5)

# Click on folder 0 (Sintetizadores) or folder 1 (Sequenciadores)
# Let's send UDP command or trigger screenshot
req_file = os.path.join(os.environ.get("TEMP", "C:\\Windows\\Temp"), "abduction_screenshot_req.txt")
out_bmp = os.path.join(os.environ.get("TEMP", "C:\\Windows\\Temp"), "abduction_framebuffer.bmp")

if os.path.exists(out_bmp):
    os.remove(out_bmp)

# Trigger screenshot of current clean desktop
with open(req_file, "w") as f:
    f.write("trigger")

for _ in range(30):
    if os.path.exists(out_bmp) and os.path.getsize(out_bmp) > 1000:
        break
    time.sleep(0.1)

if os.path.exists(out_bmp):
    img = Image.open(out_bmp)
    dest_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"
    full_path = os.path.join(dest_dir, "abduction_clean_desktop_live.png")
    img.save(full_path)
    print("Saved clean desktop screenshot:", full_path)

proc.terminate()
