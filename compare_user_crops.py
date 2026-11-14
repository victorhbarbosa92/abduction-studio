from PIL import Image
import os

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\9ac91113-f394-4081-89c3-11f84ee2583d"
rb_img_path = os.path.join(artifact_dir, ".user_uploaded", "media_1793390851848.png")
ab_img_path = os.path.join(artifact_dir, ".user_uploaded", "media_1793390867618.png")

img_rb = Image.open(rb_img_path)
img_ab = Image.open(ab_img_path)

print(f"Rekordbox reference size: {img_rb.size}")
print(f"Abduction current size: {img_ab.size}")

# Let's inspect a 100x100 patch of both:
# In Rekordbox, let's take a sample of the kick drum (red region)
# Let's find where the kick is in both images
crop_rb = img_rb.crop((100, 10, 400, 110))
crop_rb.save(os.path.join(artifact_dir, "crop_rb_user.png"))

crop_ab = img_ab.crop((100, 10, 400, 110))
crop_ab.save(os.path.join(artifact_dir, "crop_ab_user.png"))

print("Saved crop_rb_user.png and crop_ab_user.png")
