import os
from PIL import Image

asset_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\assets\ui\rolling_bass"
out_header = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroRollingBassTextureEmbed.h"

files = [
    ("bg_chassis", "bg_chassis_1040x580.png"),
    ("btn_generate_pianoroll", "btn_generate_pianoroll.png"),
    ("btn_generate_playlist", "btn_generate_playlist.png"),
    ("btn_kbbb", "btn_kbbb.png"),
    ("btn_kbb", "btn_kbb.png"),
    ("btn_kb", "btn_kb.png"),
    ("btn_octave", "btn_octave.png"),
    ("btn_wave_saw", "btn_wave_saw.png"),
    ("btn_wave_square", "btn_wave_square.png"),
    ("btn_wave_subsine", "btn_wave_subsine.png"),
    ("dial_phase_retrigger", "dial_phase_retrigger.png"),
    ("knob_metallic", "knob_metallic.png"),
    ("kuro_logo", "kuro_logo.png"),
    ("eq_bar_sample", "eq_bar_sample.png")
]

header_lines = [
    "#pragma once",
    "#include <GLFW/glfw3.h>",
    "#include \"imgui.h\"",
    "#include <vector>",
    "#include <cmath>",
    "",
    "namespace KuroUI {",
    "",
    "    struct RollingBassTextures {",
    "        GLuint tex_bg_chassis = 0;",
    "        GLuint tex_btn_gen_pr = 0;",
    "        GLuint tex_btn_gen_pl = 0;",
    "        GLuint tex_btn_kbbb = 0;",
    "        GLuint tex_btn_kbb = 0;",
    "        GLuint tex_btn_kb = 0;",
    "        GLuint tex_btn_octave = 0;",
    "        GLuint tex_btn_wave_saw = 0;",
    "        GLuint tex_btn_wave_square = 0;",
    "        GLuint tex_btn_wave_subsine = 0;",
    "        GLuint tex_dial_phase = 0;",
    "        GLuint tex_knob_metallic = 0;",
    "        GLuint tex_kuro_logo = 0;",
    "        GLuint tex_eq_bar = 0;",
    "        bool is_loaded = false;",
    "",
    "        static GLuint UploadTexture(int w, int h, const unsigned char* data) {",
    "            if (!data || w <= 0 || h <= 0) return 0;",
    "            GLuint tex_id = 0;",
    "            glGenTextures(1, &tex_id);",
    "            glBindTexture(GL_TEXTURE_2D, tex_id);",
    "            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);",
    "            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);",
    "            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);",
    "            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);",
    "            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);",
    "            return tex_id;",
    "        }",
    ""
]

# Process each image to RGBA
for var_name, filename in files:
    filepath = os.path.join(asset_dir, filename)
    im = Image.open(filepath).convert("RGBA")
    w, h = im.size
    data = list(im.getdata())
    flat_bytes = [byte for pixel in data for byte in pixel]
    
    header_lines.append(f"        // Texture: {filename} ({w}x{h})")
    header_lines.append(f"        static const int {var_name}_w = {w};")
    header_lines.append(f"        static const int {var_name}_h = {h};")
    header_lines.append(f"        static const unsigned char* Get_{var_name}_bytes() {{")
    
    # Store in chunks for compiler speed
    byte_strs = [str(b) for b in flat_bytes]
    header_lines.append(f"            static const unsigned char s_data[{len(byte_strs)}] = {{")
    
    # 20 bytes per line
    chunk_size = 20
    for i in range(0, len(byte_strs), chunk_size):
        chunk = byte_strs[i:i + chunk_size]
        header_lines.append("                " + ", ".join(chunk) + ",")
        
    header_lines.append("            };")
    header_lines.append("            return s_data;")
    header_lines.append("        }")
    header_lines.append("")

header_lines.extend([
    "        void Init() {",
    "            if (is_loaded) return;",
    "            tex_bg_chassis = UploadTexture(bg_chassis_w, bg_chassis_h, Get_bg_chassis_bytes());",
    "            tex_btn_gen_pr = UploadTexture(btn_generate_pianoroll_w, btn_generate_pianoroll_h, Get_btn_generate_pianoroll_bytes());",
    "            tex_btn_gen_pl = UploadTexture(btn_generate_playlist_w, btn_generate_playlist_h, Get_btn_generate_playlist_bytes());",
    "            tex_btn_kbbb = UploadTexture(btn_kbbb_w, btn_kbbb_h, Get_btn_kbbb_bytes());",
    "            tex_btn_kbb = UploadTexture(btn_kbb_w, btn_kbb_h, Get_btn_kbb_bytes());",
    "            tex_btn_kb = UploadTexture(btn_kb_w, btn_kb_h, Get_btn_kb_bytes());",
    "            tex_btn_octave = UploadTexture(btn_octave_w, btn_octave_h, Get_btn_octave_bytes());",
    "            tex_btn_wave_saw = UploadTexture(btn_wave_saw_w, btn_wave_saw_h, Get_btn_wave_saw_bytes());",
    "            tex_btn_wave_square = UploadTexture(btn_wave_square_w, btn_wave_square_h, Get_btn_wave_square_bytes());",
    "            tex_btn_wave_subsine = UploadTexture(btn_wave_subsine_w, btn_wave_subsine_h, Get_btn_wave_subsine_bytes());",
    "            tex_dial_phase = UploadTexture(dial_phase_retrigger_w, dial_phase_retrigger_h, Get_dial_phase_retrigger_bytes());",
    "            tex_knob_metallic = UploadTexture(knob_metallic_w, knob_metallic_h, Get_knob_metallic_bytes());",
    "            tex_kuro_logo = UploadTexture(kuro_logo_w, kuro_logo_h, Get_kuro_logo_bytes());",
    "            tex_eq_bar = UploadTexture(eq_bar_sample_w, eq_bar_sample_h, Get_eq_bar_sample_bytes());",
    "            is_loaded = true;",
    "        }",
    "    };",
    "",
    "    inline RollingBassTextures g_rolling_bass_textures;",
    "",
    "} // namespace KuroUI"
])

with open(out_header, "w", encoding="utf-8") as f:
    f.write("\n".join(header_lines))

print(f"Generated {out_header} ({len(header_lines)} lines).")
