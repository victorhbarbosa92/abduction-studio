from PIL import Image
import os

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity\brain\b9e05024-c94b-463e-ab5e-730f4a59ba8d"
src_img_path = os.path.join(artifact_dir, "live_abduction_hub_split_dashboard.png")

if os.path.exists(src_img_path):
    img = Image.open(src_img_path)
    w, h = img.size

    # Crop Hero Box and Source Cards (from y=6% to y=60%)
    crop_hero_cards = img.crop((int(w * 0.53), int(h * 0.05), int(w * 0.99), int(h * 0.60)))
    crop_hero_cards.save(os.path.join(artifact_dir, "crop_hero_cards.png"))
    print("Saved crop_hero_cards.png")

