import os
import math
import numpy as np
from PIL import Image

asset_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\assets\ui\rolling_bass"
os.makedirs(asset_dir, exist_ok=True)

def create_photorealistic_knob(size=256, has_indicator_notch=True):
    """
    Gera uma textura de knob metálico fotorrealista:
    - Fundo 100% transparente (Alpha = 0).
    - Corpo circular com acabamento em metal escovado anisotrópico.
    - Chanfro 3D metálico com iluminação direcional.
    - Sulcos concêntricos de usinagem industrial.
    - Marcador entalhado em neon ciano a 12 horas.
    - Borda externa perfeitamente anti-aliased sub-pixel.
    """
    R = size / 2.0
    cx, cy = R, R
    
    # Grid 2D completo (size x size)
    y_idx, x_idx = np.mgrid[:size, :size]
    dx = x_idx - cx
    dy = y_idx - cy
    dist = np.sqrt(dx*dx + dy*dy)
    angle = np.arctan2(dy, dx) # -pi to pi
    
    # Raio do corpo do knob (deixa 3px de margem para anti-aliasing suave na borda)
    knob_radius = R - 4.0
    
    img_data = np.zeros((size, size, 4), dtype=np.float32)
    
    # 1. Metal escovado anisotrópico (radial brushed texture)
    brushed_specular = 0.5 + 0.5 * np.cos(2.0 * (angle - 0.785))
    brushed_specular = np.power(brushed_specular, 1.8)
    
    # Micro-estrias de usinagem (ruído angular de alta frequência)
    np.random.seed(42)
    num_streaks = 360
    ang_norm = (angle + np.pi) / (2.0 * np.pi) * num_streaks
    random_samples = np.random.uniform(0.93, 1.07, num_streaks)
    angular_noise = np.interp(ang_norm.flatten(), np.arange(num_streaks), random_samples).reshape(size, size)
    
    # Sulcos concêntricos sutis
    concentric_grooves = 0.95 + 0.05 * np.sin(dist * 1.2)
    
    # Cor base: dark titanium / obsidian
    base_r, base_g, base_b = 28.0, 38.0, 48.0
    
    r_ch = base_r * (0.8 + 0.5 * brushed_specular) * angular_noise * concentric_grooves
    g_ch = base_g * (0.8 + 0.55 * brushed_specular) * angular_noise * concentric_grooves
    b_ch = base_b * (0.8 + 0.65 * brushed_specular) * angular_noise * concentric_grooves
    
    # Iluminação 3D global (fonte de luz no topo-esquerdo)
    light_dir_x, light_dir_y = -0.6, -0.8
    norm_light = (dx * light_dir_x + dy * light_dir_y) / (knob_radius + 1e-5)
    light_factor = np.clip(1.0 - norm_light * 0.28, 0.72, 1.28)
    
    r_ch *= light_factor
    g_ch *= light_factor
    b_ch *= light_factor
    
    # Chanfro Externo Metálico (Bevel Ring)
    bevel_mask = (dist >= (knob_radius - 8.0)) & (dist <= knob_radius)
    bevel_pos = (dist - (knob_radius - 8.0)) / 8.0
    rim_highlight = np.sin(bevel_pos * np.pi)
    rim_light = np.clip(1.0 - (dx * -0.7 + dy * -0.7) / knob_radius, 0.4, 1.6)
    
    r_ch[bevel_mask] = (r_ch[bevel_mask] * 0.6 + 60.0 * rim_highlight[bevel_mask] * rim_light[bevel_mask])
    g_ch[bevel_mask] = (g_ch[bevel_mask] * 0.6 + 90.0 * rim_highlight[bevel_mask] * rim_light[bevel_mask])
    b_ch[bevel_mask] = (b_ch[bevel_mask] * 0.6 + 115.0 * rim_highlight[bevel_mask] * rim_light[bevel_mask])
    
    # Anel rebaixado interno
    groove_ring = np.exp(-np.power(dist - (knob_radius - 9.0), 2) / 2.0)
    r_ch -= groove_ring * 16.0
    g_ch -= groove_ring * 20.0
    b_ch -= groove_ring * 24.0
    
    # Centro com ligeira concavidade cônica
    center_depth = np.clip(1.0 - np.exp(-dist / 25.0) * 0.35, 0.65, 1.0)
    r_ch *= center_depth
    g_ch *= center_depth
    b_ch *= center_depth
    
    # Marcador Notch Neon Ciano a 12h (topo)
    if has_indicator_notch:
        notch_w = size * 0.030
        y_top = -knob_radius + 7.0
        y_bot = -knob_radius * 0.42
        
        notch_mask = (np.abs(dx) <= notch_w) & (dy <= y_bot) & (dy >= y_top)
        
        # Cor neon ciano puro
        r_ch[notch_mask] = 0.0
        g_ch[notch_mask] = 238.0
        b_ch[notch_mask] = 255.0
        
        # Brilho suave nas bordas
        glow_dist = np.abs(dx)
        glow_w = notch_w * 2.2
        glow_mask = (glow_dist <= glow_w) & (dy <= (y_bot + 2.0)) & (dy >= (y_top - 2.0))
        glow_factor = np.clip(1.0 - glow_dist / glow_w, 0.0, 1.0)
        
        g_ch[glow_mask] = np.maximum(g_ch[glow_mask], 180.0 * glow_factor[glow_mask])
        b_ch[glow_mask] = np.maximum(b_ch[glow_mask], 240.0 * glow_factor[glow_mask])
    
    # MÁSCARA ALFA SUAVE ANTI-ALIASED
    # 255 no interior até knob_radius - 1.2, transição suave para 0 além de knob_radius
    alpha = np.clip((knob_radius - dist) / 1.5 + 0.5, 0.0, 1.0) * 255.0
    
    img_data[..., 0] = np.clip(r_ch, 0.0, 255.0)
    img_data[..., 1] = np.clip(g_ch, 0.0, 255.0)
    img_data[..., 2] = np.clip(b_ch, 0.0, 255.0)
    img_data[..., 3] = alpha
    
    return Image.fromarray(img_data.astype(np.uint8), mode="RGBA")

def apply_circular_mask_to_image(img_path, margin=2.0):
    if not os.path.exists(img_path):
        return
    im = Image.open(img_path).convert("RGBA")
    w, h = im.size
    cx, cy = w / 2.0, h / 2.0
    radius = min(cx, cy) - margin
    
    y_idx, x_idx = np.mgrid[:h, :w]
    dist = np.sqrt((x_idx - cx)**2 + (y_idx - cy)**2)
    
    alpha = np.clip((radius - dist) / 1.5 + 0.5, 0.0, 1.0)
    
    arr = np.array(im, dtype=np.float32)
    arr[..., 3] = arr[..., 3] * alpha
    
    out = Image.fromarray(arr.astype(np.uint8), mode="RGBA")
    out.save(img_path)
    print(f"Masked circle on: {img_path}")

# 1. Gerar knob_metallic.png perfeito
knob_img = create_photorealistic_knob(size=256, has_indicator_notch=True)
knob_path = os.path.join(asset_dir, "knob_metallic.png")
knob_img.save(knob_path)
print(f"Generated: {knob_path} (256x256 RGBA, 100% transparent background, anti-aliased)")

# 2. Gerar dial_phase_retrigger.png perfeito
dial_img = create_photorealistic_knob(size=256, has_indicator_notch=True)
dial_path = os.path.join(asset_dir, "dial_phase_retrigger.png")
dial_img.save(dial_path)
print(f"Generated: {dial_path} (256x256 RGBA, 100% transparent background, anti-aliased)")

# 3. Aplicar máscara circular anti-aliased aos botões de onda
for btn_name in ["btn_wave_saw.png", "btn_wave_square.png", "btn_wave_subsine.png"]:
    apply_circular_mask_to_image(os.path.join(asset_dir, btn_name), margin=3.0)

print("All textures processed with perfect alpha transparency!")
