import os
import struct
from PIL import Image

asset_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\assets\ui\rolling_bass"
bin_out = os.path.join(asset_dir, "textures.bin")

files = [
    ("bg_chassis", "bg_chassis_1040x580.png"),
    ("btn_gen_pr", "btn_generate_pianoroll.png"),
    ("btn_gen_pl", "btn_generate_playlist.png"),
    ("btn_kbbb", "btn_kbbb.png"),
    ("btn_kbb", "btn_kbb.png"),
    ("btn_kb", "btn_kb.png"),
    ("btn_octave", "btn_octave.png"),
    ("btn_wave_saw", "btn_wave_saw.png"),
    ("btn_wave_square", "btn_wave_square.png"),
    ("btn_wave_subsine", "btn_wave_subsine.png"),
    ("dial_phase", "dial_phase_retrigger.png"),
    ("knob_metallic", "knob_metallic.png"),
    ("kuro_logo", "kuro_logo.png"),
    ("eq_bar", "eq_bar_sample.png")
]

records = []
total_bytes = 0

for tex_name, filename in files:
    filepath = os.path.join(asset_dir, filename)
    im = Image.open(filepath).convert("RGBA")
    w, h = im.size
    raw_data = im.tobytes()
    records.append((tex_name, w, h, raw_data))
    total_bytes += len(raw_data)
    print(f"Packaged: {tex_name} ({w}x{h}, {len(raw_data)} bytes)")

# Format of binary file:
# Magic: 8 bytes 'KUROTEX1'
# Num textures: uint32 (14)
# Header entries: 14 * (32 bytes name + uint32 w + uint32 h + uint32 size)
# Raw payload bytes
with open(bin_out, "wb") as f:
    f.write(b"KUROTEX1")
    f.write(struct.pack("<I", len(records)))
    
    # Write Table of Contents
    for name, w, h, data in records:
        name_padded = name.encode('ascii')[:31].ljust(32, b'\0')
        f.write(name_padded)
        f.write(struct.pack("<III", w, h, len(data)))
        
    # Write Raw Pixel Payloads
    for name, w, h, data in records:
        f.write(data)

print(f"Successfully wrote {bin_out} ({os.path.getsize(bin_out)} bytes).")
