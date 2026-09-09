#!/usr/bin/env python3
"""
================================================================================
KURO UI ASSET PROCESSOR & SLICER (STANDARD PIPELINE)
================================================================================
Script padrão oficial do ecossistema Abduction Studio / Kuro DSP para:
1. Recorte cirúrgico de elementos de UI a partir de mockups (Knobs, Dials, Botões, Telas).
2. Aplicação de Máscaras Alfa Anti-Aliased Sub-Pixel (Circular e Cantos Arredondados).
3. Eliminação automática de cantos quadrados e artefatos de compressão JPEG.
4. Geração procedural de Knobs metálicos escovados fotorrealistas em 360°.
5. Compilação automática de pacotes de textura binários (.bin) de carregamento ultrarrápido em C++.
================================================================================
"""

import os
import sys
import json
import struct
import argparse
import numpy as np
from PIL import Image, ImageFilter

def apply_circular_alpha_mask(image, margin=2.0, feather=1.5):
    """
    Aplica uma máscara alfa circular com anti-aliasing sub-pixel perfeito.
    Qualquer pixel fora do raio é tornado 100% transparente (Alpha = 0).
    """
    if isinstance(image, str):
        im = Image.open(image).convert("RGBA")
    else:
        im = image.convert("RGBA")
    
    w, h = im.size
    cx, cy = w / 2.0, h / 2.0
    radius = min(cx, cy) - margin
    
    y_idx, x_idx = np.mgrid[:h, :w]
    dist = np.sqrt((x_idx - cx)**2 + (y_idx - cy)**2)
    
    # Anti-aliasing suave entre (radius - feather) e radius
    alpha_mask = np.clip((radius - dist) / feather + 0.5, 0.0, 1.0)
    
    arr = np.array(im, dtype=np.float32)
    arr[..., 3] = arr[..., 3] * alpha_mask
    
    return Image.fromarray(arr.astype(np.uint8), mode="RGBA")

def apply_rounded_rect_alpha_mask(image, corner_radius=8.0, feather=1.5):
    """
    Aplica uma máscara alfa com cantos arredondados e borda anti-aliased.
    """
    if isinstance(image, str):
        im = Image.open(image).convert("RGBA")
    else:
        im = image.convert("RGBA")
        
    w, h = im.size
    r = float(corner_radius)
    
    y_idx, x_idx = np.mgrid[:h, :w]
    
    # Distância das 4 quinas
    dx = np.maximum(0.0, np.maximum(r - x_idx, x_idx - (w - 1.0 - r)))
    dy = np.maximum(0.0, np.maximum(r - y_idx, y_idx - (h - 1.0 - r)))
    dist_corner = np.sqrt(dx*dx + dy*dy)
    
    mask = np.clip((r - dist_corner) / feather + 0.5, 0.0, 1.0)
    
    arr = np.array(im, dtype=np.float32)
    arr[..., 3] = arr[..., 3] * mask
    
    return Image.fromarray(arr.astype(np.uint8), mode="RGBA")

def generate_photorealistic_knob(
    size=256, 
    base_color=(28, 38, 48), 
    notch_color=(0, 238, 255), 
    has_notch=True, 
    notch_width_ratio=0.030,
    concentric_strength=0.05
):
    """
    Gera uma textura de knob metálico fotorrealista com transparência pura:
    - Acabamento em titânio escuro escovado radialmente (anisotrópico).
    - Chanfro metálico 3D (Bevel Ring) com iluminação direcional.
    - Sulcos concêntricos de usinagem industrial.
    - Marcador entalhado em neon a 12 horas.
    - Fundo 100% transparente com anti-aliasing de borda.
    """
    R = size / 2.0
    cx, cy = R, R
    
    y_idx, x_idx = np.mgrid[:size, :size]
    dx = x_idx - cx
    dy = y_idx - cy
    dist = np.sqrt(dx*dx + dy*dy)
    angle = np.arctan2(dy, dx)
    
    knob_radius = R - 4.0
    img_data = np.zeros((size, size, 4), dtype=np.float32)
    
    # 1. Reflexo metálico anisotrópico
    brushed_specular = 0.5 + 0.5 * np.cos(2.0 * (angle - 0.785))
    brushed_specular = np.power(brushed_specular, 1.8)
    
    # Micro-estrias de usinagem
    np.random.seed(42)
    num_streaks = 360
    ang_norm = (angle + np.pi) / (2.0 * np.pi) * num_streaks
    random_samples = np.random.uniform(0.93, 1.07, num_streaks)
    angular_noise = np.interp(ang_norm.flatten(), np.arange(num_streaks), random_samples).reshape(size, size)
    
    concentric_grooves = 1.0 - concentric_strength + concentric_strength * np.sin(dist * 1.2)
    
    base_r, base_g, base_b = [float(c) for c in base_color]
    r_ch = base_r * (0.8 + 0.5 * brushed_specular) * angular_noise * concentric_grooves
    g_ch = base_g * (0.8 + 0.55 * brushed_specular) * angular_noise * concentric_grooves
    b_ch = base_b * (0.8 + 0.65 * brushed_specular) * angular_noise * concentric_grooves
    
    # Iluminação 3D de fonte no topo-esquerdo
    light_dir_x, light_dir_y = -0.6, -0.8
    norm_light = (dx * light_dir_x + dy * light_dir_y) / (knob_radius + 1e-5)
    light_factor = np.clip(1.0 - norm_light * 0.28, 0.72, 1.28)
    
    r_ch *= light_factor
    g_ch *= light_factor
    b_ch *= light_factor
    
    # Chanfro externo (Bevel Rim)
    bevel_mask = (dist >= (knob_radius - 8.0)) & (dist <= knob_radius)
    bevel_pos = (dist - (knob_radius - 8.0)) / 8.0
    rim_highlight = np.sin(bevel_pos * np.pi)
    rim_light = np.clip(1.0 - (dx * -0.7 + dy * -0.7) / knob_radius, 0.4, 1.6)
    
    r_ch[bevel_mask] = (r_ch[bevel_mask] * 0.6 + 60.0 * rim_highlight[bevel_mask] * rim_light[bevel_mask])
    g_ch[bevel_mask] = (g_ch[bevel_mask] * 0.6 + 90.0 * rim_highlight[bevel_mask] * rim_light[bevel_mask])
    b_ch[bevel_mask] = (b_ch[bevel_mask] * 0.6 + 115.0 * rim_highlight[bevel_mask] * rim_light[bevel_mask])
    
    # Sulco rebaixado interno
    groove_ring = np.exp(-np.power(dist - (knob_radius - 9.0), 2) / 2.0)
    r_ch -= groove_ring * 16.0
    g_ch -= groove_ring * 20.0
    b_ch -= groove_ring * 24.0
    
    # Centro com concavidade sutil
    center_depth = np.clip(1.0 - np.exp(-dist / 25.0) * 0.35, 0.65, 1.0)
    r_ch *= center_depth
    g_ch *= center_depth
    b_ch *= center_depth
    
    # Marcador Notch no Topo (12 horas)
    if has_notch:
        notch_w = size * notch_width_ratio
        y_top = -knob_radius + 7.0
        y_bot = -knob_radius * 0.42
        
        notch_mask = (np.abs(dx) <= notch_w) & (dy <= y_bot) & (dy >= y_top)
        
        nr, ng, nb = [float(c) for c in notch_color]
        r_ch[notch_mask] = nr
        g_ch[notch_mask] = ng
        b_ch[notch_mask] = nb
        
        glow_dist = np.abs(dx)
        glow_w = notch_w * 2.2
        glow_mask = (glow_dist <= glow_w) & (dy <= (y_bot + 2.0)) & (dy >= (y_top - 2.0))
        glow_factor = np.clip(1.0 - glow_dist / glow_w, 0.0, 1.0)
        
        g_ch[glow_mask] = np.maximum(g_ch[glow_mask], (ng * 0.75) * glow_factor[glow_mask])
        b_ch[glow_mask] = np.maximum(b_ch[glow_mask], nb * glow_factor[glow_mask])
    
    # Máscara Alfa Sub-pixel anti-aliased
    alpha = np.clip((knob_radius - dist) / 1.5 + 0.5, 0.0, 1.0) * 255.0
    
    img_data[..., 0] = np.clip(r_ch, 0.0, 255.0)
    img_data[..., 1] = np.clip(g_ch, 0.0, 255.0)
    img_data[..., 2] = np.clip(b_ch, 0.0, 255.0)
    img_data[..., 3] = alpha
    
    return Image.fromarray(img_data.astype(np.uint8), mode="RGBA")

def compile_texture_pack(png_dict, output_bin_path):
    """
    Empacota um dicionário de texturas {nome: Image} em um arquivo binário KUROTEX1.
    Formato:
    - 8 bytes Magic: 'KUROTEX1'
    - uint32: total_textures
    - TOC: total_textures * (32 bytes char name + uint32 w + uint32 h + uint32 byte_size)
    - Payloads brutos RGBA
    """
    records = []
    for name, im in png_dict.items():
        rgba = im.convert("RGBA")
        w, h = rgba.size
        raw_data = rgba.tobytes()
        records.append((name, w, h, raw_data))
    
    with open(output_bin_path, "wb") as f:
        f.write(b"KUROTEX1")
        f.write(struct.pack("<I", len(records)))
        
        for name, w, h, data in records:
            name_padded = name.encode('ascii')[:31].ljust(32, b'\0')
            f.write(name_padded)
            f.write(struct.pack("<III", w, h, len(data)))
            
        for name, w, h, data in records:
            f.write(data)
            
    print(f"[KUROTEX] Pacote binário compilado: {output_bin_path} ({os.path.getsize(output_bin_path)} bytes, {len(records)} texturas).")

def process_ui_recipe(mockup_path, recipe_path_or_dict, output_dir, compile_bin=True, bin_name="textures.bin"):
    """
    Executa a receita de recorte, máscara e processamento de UI a partir do mockup.
    """
    os.makedirs(output_dir, exist_ok=True)
    
    if isinstance(recipe_path_or_dict, str):
        with open(recipe_path_or_dict, "r", encoding="utf-8") as f:
            recipe = json.load(f)
    else:
        recipe = recipe_path_or_dict
        
    mockup_img = None
    if mockup_path and os.path.exists(mockup_path):
        mockup_img = Image.open(mockup_path).convert("RGBA")
        print(f"[PROCESSOR] Mockup carregado: {mockup_path} ({mockup_img.size[0]}x{mockup_img.size[1]})")
        
    processed_images = {}
    
    for item in recipe.get("elements", []):
        name = item["name"]
        elem_type = item.get("type", "raw")
        
        # 1. Obter imagem base
        if elem_type == "procedural_knob":
            size = item.get("size", 256)
            base_col = item.get("base_color", [28, 38, 48])
            notch_col = item.get("notch_color", [0, 238, 255])
            has_notch = item.get("has_notch", True)
            im = generate_photorealistic_knob(size=size, base_color=base_col, notch_color=notch_col, has_notch=has_notch)
        elif mockup_img is not None and "crop" in item:
            crop_box = tuple(item["crop"]) # (x1, y1, x2, y2)
            im = mockup_img.crop(crop_box)
        else:
            print(f"[WARN] Elemento {name} sem imagem base ignorado.")
            continue
            
        # 2. Redimensionamento opcional
        if "resize" in item:
            rw, rh = item["resize"]
            im = im.resize((rw, rh), Image.Resampling.LANCZOS)
            
        # 3. Aplicação de Máscaras Alfa
        if elem_type == "circle":
            margin = item.get("margin", 2.0)
            feather = item.get("feather", 1.5)
            im = apply_circular_alpha_mask(im, margin=margin, feather=feather)
        elif elem_type == "rounded_rect":
            radius = item.get("corner_radius", 8.0)
            feather = item.get("feather", 1.5)
            im = apply_rounded_rect_alpha_mask(im, corner_radius=radius, feather=feather)
            
        # 4. Salvar PNG individual transparente
        out_png = os.path.join(output_dir, f"{name}.png")
        im.save(out_png, "PNG")
        print(f"[PROCESSOR] Extraído: {out_png} ({im.size[0]}x{im.size[1]} RGBA)")
        processed_images[name] = im
        
    if compile_bin and processed_images:
        bin_path = os.path.join(output_dir, bin_name)
        compile_texture_pack(processed_images, bin_path)
        
    print(f"[SUCCESS] Todos os {len(processed_images)} elementos foram processados com sucesso!")
    return processed_images

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Kuro UI Asset Processor & Slicer")
    parser.add_argument("--mockup", type=str, help="Caminho para o mockup de referência (JPG/PNG)")
    parser.add_argument("--recipe", type=str, help="Caminho para o arquivo JSON de receita de recorte")
    parser.add_argument("--output", type=str, default="assets/ui", help="Diretório de saída para os PNGs e .bin")
    parser.add_argument("--generate-knob", action="store_true", help="Gera um knob fotorrealista isolado")
    parser.add_argument("--knob-out", type=str, default="knob_metallic.png", help="Nome do arquivo de saída do knob")
    parser.add_argument("--size", type=int, default=256, help="Tamanho do knob em pixels")
    
    args = parser.parse_args()
    
    if args.generate_knob:
        knob = generate_photorealistic_knob(size=args.size)
        knob.save(args.knob_out)
        print(f"Knob gerado em: {args.knob_out} ({args.size}x{args.size})")
    elif args.recipe:
        process_ui_recipe(args.mockup, args.recipe, args.output)
    else:
        parser.print_help()
