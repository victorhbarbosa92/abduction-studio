import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\9ac91113-f394-4081-89c3-11f84ee2583d"
temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
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
        png_path = os.path.join(artifact_dir, png_name)
        img.save(png_path)
        print(f"[CAPTURE] Screenshot salva em {png_path} ({img.size[0]}x{img.size[1]})")
        try: os.remove(bmp_out_file)
        except: pass
        return img, png_path
    return None, None

def main():
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
    time.sleep(0.5)

    print("[CAPTURE] Iniciando AbductionStudioV2.exe com --test-deck-play...")
    proc = subprocess.Popen(["AbductionStudioV2.exe", "--test-deck-play"], cwd=r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(3.5)

    img, full_path = trigger_and_save("rekordbox_1to1_live.png")
    if img:
        w, h = img.size
        # Crop waveform area (top ~42% of window)
        crop_wave = img.crop((0, 0, w, int(h * 0.42)))
        crop_path = os.path.join(artifact_dir, "rekordbox_1to1_crop.png")
        crop_wave.save(crop_path)
        print(f"[CAPTURE] Waveform crop salva em {crop_path}")

        # Crop close-up around the center playhead
        center_x = int(w * 0.5)
        zoom_w = 400
        zoom_h = int(h * 0.38)
        zoom_crop = img.crop((center_x - zoom_w // 2, 30, center_x + zoom_w // 2, 30 + zoom_h))
        zoom_path = os.path.join(artifact_dir, "rekordbox_1to1_zoom.png")
        zoom_crop.save(zoom_path)
        print(f"[CAPTURE] Zoom playhead crop salvo em {zoom_path}")

        # Direct composite with user reference
        rb_user = os.path.join(artifact_dir, ".user_uploaded", "media_1793390851848.png")
        if os.path.exists(rb_user):
            rb_img = Image.open(rb_user)
            # Crop waveform lane from our live capture
            # Waveform lane is roughly y=55 to y=190
            our_lane = img.crop((0, 52, w, 192))
            
            target_w = 1024
            rb_resized = rb_img.resize((target_w, int(rb_img.height * (target_w / rb_img.width))), Image.Resampling.LANCZOS)
            our_resized = our_lane.resize((target_w, int(our_lane.height * (target_w / our_lane.width))), Image.Resampling.LANCZOS)
            
            comp = Image.new("RGBA", (target_w, rb_resized.height + our_resized.height + 30), (10, 12, 16, 255))
            comp.paste(rb_resized, (0, 0))
            comp.paste(our_resized, (0, rb_resized.height + 30))
            comp_path = os.path.join(artifact_dir, "pioneer_decompiled_vs_rb.png")
            comp.save(comp_path)
            print(f"[CAPTURE] Comparativo gravado em {comp_path}")

    proc.terminate()
    try: proc.wait(timeout=2)
    except: proc.kill()
    os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")

if __name__ == "__main__":
    main()
