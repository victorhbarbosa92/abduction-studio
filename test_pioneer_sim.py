import numpy as np
from PIL import Image
import math
import os

# Let's load the user's Rekordbox reference crop
artifact_dir = r"C:\Users\o_vic\.gemini\antigravity-ide\brain\9ac91113-f394-4081-89c3-11f84ee2583d"
rb_crop_path = os.path.join(artifact_dir, "crop_rb_user.png")
rb_img = Image.open(rb_crop_path)

# Let's simulate Pioneer's exact algorithm on a synthesized track (kicks + snares + hats + bass)
w = rb_img.width
h = rb_img.height // 2 # one deck height
mid_y = h // 2
max_half_h = int(h * 0.42)

sim_img = Image.new("RGBA", (w, rb_img.height), (0, 0, 0, 255))
pixels = sim_img.load()

# Let's generate 4 bars of audio data
# Pioneer color function from z2.m.q:
def pioneer_color(low, mid, high):
    max_v = max(low, max(mid, high), 1e-4)
    r = low / max_v
    g = mid / max_v
    b = high / max_v
    max_rb = max(r, b)
    damp = min(max_rb, 0.5) * 0.3
    g = (1.0 - damp) * g
    b = b * 1.3
    r = max(0.0, min(1.0, r))
    g = max(0.0, min(1.0, g))
    b = max(0.0, min(1.0, b))
    ir = int(r * 255.0)
    ig = int(g * 255.0)
    ib = int(b * 255.0)
    return (ir, ig, ib)

# Simulate Deck 1 (top) and Deck 2 (bottom)
for deck_idx in [0, 1]:
    deck_mid_y = (rb_img.height // 4) if deck_idx == 0 else (rb_img.height * 3 // 4)
    for x in range(w):
        # Beat simulation (kicks every 75 px, snares offset by 37.5 px)
        beat_phase = (x % 75) / 75.0
        snare_phase = ((x + 37) % 75) / 75.0
        
        if deck_idx == 0:
            # Kick heavy (low dominates)
            low = math.exp(-beat_phase * 12.0) * 0.95 + 0.05
            snare = math.exp(-snare_phase * 14.0) * 0.85
            high = snare * 0.8 + 0.15 * math.sin(x * 0.4)**2
            mid = snare * 0.6 + 0.25 * math.sin(x * 0.15)**2
        else:
            # Deck 2: Intro (cyan hats/synth dominates, high > low)
            low = 0.08 + 0.05 * math.sin(x * 0.1)**2
            mid = 0.45 * math.sin(x * 0.2)**2 + 0.1
            high = 0.85 * math.sin(x * 0.5)**2 + 0.2

        amp = max(low, max(mid, high))
        pos_h = int(amp * max_half_h)
        neg_h = int(amp * max_half_h * (0.85 + 0.15 * math.cos(x * 0.3)))
        
        cr, cg, cb = pioneer_color(low, mid, high)
        
        # Draw Pioneer 1px column with Gouraud gradient
        top_y = deck_mid_y - pos_h
        bot_y = deck_mid_y + neg_h
        
        for y in range(top_y, bot_y + 1):
            if 0 <= y < rb_img.height:
                dist_norm = abs(y - deck_mid_y) / max(1, pos_h if y < deck_mid_y else neg_h)
                # Outer falloff 0.6x
                falloff = 1.0 - 0.40 * dist_norm
                px_r = int(cr * falloff)
                px_g = int(cg * falloff)
                px_b = int(cb * falloff)
                pixels[x, y] = (px_r, px_g, px_b, 255)

# Save simulated image and side-by-side with crop_rb_user.png
sim_path = os.path.join(artifact_dir, "pioneer_exact_simulation.png")
sim_img.save(sim_path)

# Composite
comp_h = rb_img.height * 2 + 20
comp = Image.new("RGBA", (w, comp_h), (12, 14, 18, 255))
comp.paste(rb_img, (0, 0))
comp.paste(sim_img, (0, rb_img.height + 20))
comp_path = os.path.join(artifact_dir, "pioneer_sim_vs_rb.png")
comp.save(comp_path)
print("Saved pioneer_sim_vs_rb.png")
