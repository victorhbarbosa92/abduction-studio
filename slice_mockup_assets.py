import os
from PIL import Image, ImageDraw, ImageFilter

mockup_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\1fe098f5-bfde-454d-a9d4-194e0b7d30b6\psytrance_rolling_bass_ui_1788299805313.jpg"
out_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\assets\ui\rolling_bass"
os.makedirs(out_dir, exist_ok=True)

img = Image.open(mockup_path).convert("RGBA")
w, h = img.size
print(f"Mockup dimension: {w}x{h}")

# 1. Background Chassis (Chassi completo com os 3 cards vazados ou com fundo texturizado)
# Salvando o chassi base 1040x580 redimensionado com alta qualidade LANCZOS
chassis = img.resize((1040, 580), Image.Resampling.LANCZOS)
chassis_path = os.path.join(out_dir, "bg_chassis_1040x580.png")
chassis.save(chassis_path)
print(f"Saved: {chassis_path}")

# 2. Logo Kuro (Top-Left)
# Na imagem 1376x768: x de 40 a 420, y de 20 a 110
logo_box = (45, 25, 410, 115)
logo = img.crop(logo_box)
logo.save(os.path.join(out_dir, "kuro_logo.png"))

# 3. Waveform Buttons (Saw, Square, Sub-Sine)
# x1~70 a 170, y~170 a 270
saw_btn = img.crop((65, 175, 165, 275))
saw_btn.save(os.path.join(out_dir, "btn_wave_saw.png"))

square_btn = img.crop((180, 175, 280, 275))
square_btn.save(os.path.join(out_dir, "btn_wave_square.png"))

subsine_btn = img.crop((295, 175, 395, 275))
subsine_btn.save(os.path.join(out_dir, "btn_wave_subsine.png"))

# 4. Phase Retrigger Dial (Center Left)
# x~155 a 305, y~330 a 480
phase_dial = img.crop((150, 330, 310, 490))
phase_dial.save(os.path.join(out_dir, "dial_phase_retrigger.png"))

# 5. Metallic Knobs (Cutoff, Resonance, Transient Click)
cutoff_knob = img.crop((455, 465, 555, 565))
cutoff_knob.save(os.path.join(out_dir, "knob_metallic.png"))

# 6. Bottom Action Buttons (Generate to Piano Roll & Generate to Playlist)
# Piano Roll Button: x~75 a 665, y~650 a 735
btn_pr = img.crop((75, 650, 665, 735))
btn_pr.save(os.path.join(out_dir, "btn_generate_pianoroll.png"))

# Playlist Button: x~705 a 1295, y~650 a 735
btn_pl = img.crop((705, 650, 1295, 735))
btn_pl.save(os.path.join(out_dir, "btn_generate_playlist.png"))

# 7. Pattern Generator Quick Buttons
btn_kbbb = img.crop((960, 190, 1120, 245))
btn_kbbb.save(os.path.join(out_dir, "btn_kbbb.png"))

btn_kbb = img.crop((1135, 190, 1295, 245))
btn_kbb.save(os.path.join(out_dir, "btn_kbb.png"))

btn_kb = img.crop((960, 250, 1120, 305))
btn_kb.save(os.path.join(out_dir, "btn_kb.png"))

btn_oct = img.crop((1135, 250, 1295, 305))
btn_oct.save(os.path.join(out_dir, "btn_octave.png"))

# 8. Single Equalizer Gradient Bar (Sample from step 1 or 2)
eq_bar = img.crop((972, 355, 988, 595))
eq_bar.save(os.path.join(out_dir, "eq_bar_sample.png"))

print("All asset textures extracted successfully!")
