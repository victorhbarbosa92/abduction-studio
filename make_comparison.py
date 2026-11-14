import os
from PIL import Image

artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\9ac91113-f394-4081-89c3-11f84ee2583d"
rb_ref = os.path.join(artifact_dir, ".user_uploaded", "media_1793389222284.png")
live_crop = os.path.join(artifact_dir, "rekordbox_1to1_crop.png")

if os.path.exists(rb_ref) and os.path.exists(live_crop):
    img_rb = Image.open(rb_ref)
    img_live = Image.open(live_crop)
    
    # We want a direct comparison of the waveform area:
    # Let's crop the live waveform bounds: y from 50 to 195
    live_wf = img_live.crop((0, 50, img_live.width, 195))
    
    # Resize both to same width if needed
    target_w = 1200
    rb_wf = img_rb.resize((target_w, int(img_rb.height * (target_w / img_rb.width))), Image.Resampling.LANCZOS)
    live_wf_resized = live_wf.resize((target_w, int(live_wf.height * (target_w / live_wf.width))), Image.Resampling.LANCZOS)
    
    total_h = rb_wf.height + live_wf_resized.height + 40
    comp = Image.new("RGBA", (target_w, total_h), (12, 14, 18, 255))
    comp.paste(rb_wf, (0, 0))
    comp.paste(live_wf_resized, (0, rb_wf.height + 40))
    
    comp_path = os.path.join(artifact_dir, "rekordbox_vs_abduction_1to1.png")
    comp.save(comp_path)
    print(f"Comparativo salvo em: {comp_path}")
