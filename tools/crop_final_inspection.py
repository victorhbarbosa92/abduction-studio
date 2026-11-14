from PIL import Image
import os

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity\brain\b9e05024-c94b-463e-ab5e-730f4a59ba8d"
src_img_path = os.path.join(artifact_dir, "live_abduction_hub_split_dashboard.png")

if os.path.exists(src_img_path):
    img = Image.open(src_img_path)
    w, h = img.size
    print(f"Source image size: {w}x{h}")

    # 1. 1080p Overview
    aspect = w / h
    new_h = 1080
    new_w = int(new_h * aspect)
    img_1080 = img.resize((new_w, new_h), Image.Resampling.LANCZOS)
    img_1080.save(os.path.join(artifact_dir, "live_hub_v2_1080p_final.png"))
    print("Saved live_hub_v2_1080p_final.png")

    # 2. Right Pane crop (Hero Box + 4 source cards + Lookahead)
    # The modal occupies 98.5% width, 96.5% height
    # Right pane is from ~55% to 98% width, from ~18% to 92% height
    crop_right = img.crop((int(w * 0.53), int(h * 0.16), int(w * 0.985), int(h * 0.93)))
    crop_right.save(os.path.join(artifact_dir, "live_right_pane_final.png"))
    print("Saved live_right_pane_final.png")

    # 3. Audition Dock crop
    crop_dock = img.crop((int(w * 0.01), int(h * 0.90), int(w * 0.99), int(h * 0.98)))
    crop_dock.save(os.path.join(artifact_dir, "live_dock_final.png"))
    print("Saved live_dock_final.png")

    # 4. Left Pane crop
    crop_left = img.crop((int(w * 0.015), int(h * 0.16), int(w * 0.54), int(h * 0.93)))
    crop_left.save(os.path.join(artifact_dir, "live_left_pane_final.png"))
    print("Saved live_left_pane_final.png")
else:
    print("Source image not found!")

