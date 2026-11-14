import os
from PIL import Image, ImageFile
ImageFile.LOAD_TRUNCATED_IMAGES = True

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity\brain\24ffefe9-1e17-4c77-bc66-417fa648fcd0"
bmp_path = r"C:\Users\o_vic\AppData\Local\Temp\abduction_screenshot.bmp"

img = Image.open(bmp_path)
full_png = os.path.join(artifact_dir, "game_mode_centered_window.png")
img.save(full_png)
print(f"Saved full screenshot: {img.size} to {full_png}")

# Center window crop
w, h = img.size
# Window was centered: hud_w ~ 0.78 * w, hud_h ~ 0.60 * h
crop_box = (int(w * 0.08), int(h * 0.12), int(w * 0.92), int(h * 0.88))
cropped = img.crop(crop_box)
crop_png = os.path.join(artifact_dir, "game_mode_centered_crop.png")
cropped.save(crop_png)
print(f"Saved cropped screenshot: {cropped.size} to {crop_png}")

# Detailed close-up of the 3 cards and buttons
detail_box = (int(w * 0.10), int(h * 0.18), int(w * 0.90), int(h * 0.80))
detail_crop = img.crop(detail_box)
detail_png = os.path.join(artifact_dir, "game_mode_cards_detail.png")
detail_crop.save(detail_png)
print(f"Saved detail crop: {detail_crop.size} to {detail_png}")

