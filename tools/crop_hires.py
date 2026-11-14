import os
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity\brain\b9e05024-c94b-463e-ab5e-730f4a59ba8d"

for fname in ["live_hub_dig_deeper_default.png", "live_hub_dig_deeper_active.png"]:
    p = os.path.join(artifact_dir, fname)
    if os.path.exists(p):
        img = Image.open(p)
        print(f"{fname}: {img.size}")
        
        # Save 1080p downsampled full view
        img_1080 = img.resize((1920, int(1920 * img.size[1] / img.size[0])), Image.Resampling.LANCZOS)
        img_1080.save(os.path.join(artifact_dir, fname.replace(".png", "_1080p.png")))
        print(f"Saved {fname.replace('.png', '_1080p.png')}")

# Now crop right pane from live_hub_dig_deeper_active.png (approx x: 2200..4000, y: 150..1900)
active_p = os.path.join(artifact_dir, "live_hub_dig_deeper_active.png")
if os.path.exists(active_p):
    img = Image.open(active_p)
    w, h = img.size
    
    # Right Pane: from 55% of width to 98%
    rp_x1 = int(w * 0.55)
    rp_x2 = int(w * 0.985)
    rp_y1 = int(h * 0.08)
    rp_y2 = int(h * 0.92)
    crop_rp = img.crop((rp_x1, rp_y1, rp_x2, rp_y2))
    crop_rp.save(os.path.join(artifact_dir, "crop_dig_deeper_right_pane_hires.png"))
    
    # Hero Box and Dig Deeper header
    crop_hero_dd = img.crop((rp_x1, rp_y1, rp_x2, int(h * 0.45)))
    crop_hero_dd.save(os.path.join(artifact_dir, "crop_hero_dd_hires.png"))

    # First few matches
    crop_matches = img.crop((rp_x1, int(h * 0.28), rp_x2, int(h * 0.85)))
    crop_matches.save(os.path.join(artifact_dir, "crop_matches_hires.png"))

# Omnibar crop from default
def_p = os.path.join(artifact_dir, "live_hub_dig_deeper_default.png")
if os.path.exists(def_p):
    img = Image.open(def_p)
    w, h = img.size
    crop_omni = img.crop((int(w * 0.02), int(h * 0.04), int(w * 0.98), int(h * 0.12)))
    crop_omni.save(os.path.join(artifact_dir, "crop_omni_hires.png"))

print("Cropping done!")

