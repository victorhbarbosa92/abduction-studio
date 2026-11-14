from PIL import Image
import os

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\9ac91113-f394-4081-89c3-11f84ee2583d"
rb_crop_path = os.path.join(artifact_dir, "crop_rb_user.png")
rb_img = Image.open(rb_crop_path)

# Let's crop a 60x60 pixel region of a kick drum from Rekordbox
kick_patch = rb_img.crop((50, 5, 110, 55))
kick_patch_zoom = kick_patch.resize((kick_patch.width * 8, kick_patch.height * 8), Image.Resampling.NEAREST)
kick_patch_zoom.save(os.path.join(artifact_dir, "rb_kick_pixel_zoom.png"))

print("Saved rb_kick_pixel_zoom.png")
