import wave
import numpy as np
from scipy import signal
from PIL import Image, ImageDraw

# Load audio
wav_path = 'scratch/test_cliff.wav'
with wave.open(wav_path, 'rb') as wf:
    sr = wf.getframerate()
    n_ch = wf.getnchannels()
    n_frames = wf.getnframes()
    skip = int(sr * 4.2)
    wf.setpos(skip)
    count = int(sr * 4.0)
    data = wf.readframes(count)
    audio = np.frombuffer(data, dtype=np.int16).astype(np.float32) / 32768.0
    if n_ch == 2:
        audio = audio.reshape(-1, 2)
        mono = (audio[:, 0] + audio[:, 1]) * 0.5
    else:
        mono = audio

# Linkwitz-Riley crossover
b_l, a_l = signal.butter(2, 250.0 / (sr * 0.5), btype='low')
low = signal.lfilter(b_l, a_l, signal.lfilter(b_l, a_l, mono))
b_h, a_h = signal.butter(2, 2500.0 / (sr * 0.5), btype='high')
high = signal.lfilter(b_h, a_h, signal.lfilter(b_h, a_h, mono))
mid = mono - low - high

STEP = 96
num_bins = len(mono) // STEP

lows  = np.zeros(num_bins, dtype=np.float32)
mids  = np.zeros(num_bins, dtype=np.float32)
highs = np.zeros(num_bins, dtype=np.float32)
peaks = np.zeros(num_bins, dtype=np.float32)

for b in range(num_bins):
    s = b * STEP
    e = s + STEP
    lows[b]  = np.max(np.abs(low[s:e]))
    mids[b]  = np.max(np.abs(mid[s:e]))
    highs[b] = np.max(np.abs(high[s:e]))
    peaks[b] = np.max(np.abs(mono[s:e]))

# Normalization
max_l = max(np.max(lows), 0.001)
max_m = max(np.max(mids), 0.001)
max_h_val = max(np.max(highs), 0.001)
max_p = max(np.max(peaks), 0.001)

lows  = np.clip(lows  / max_l, 0.0, 1.0)
mids  = np.clip(mids  / max_m, 0.0, 1.0)
highs = np.clip(highs / max_h_val, 0.0, 1.0)
peaks = np.clip(peaks / max_p, 0.0, 1.0)

img_w = 700
img_h = 320
im = Image.new('RGB', (img_w, img_h), (8, 10, 14))
draw = ImageDraw.Draw(im)

max_h = 42

# -------------------------------------------------------------
# 1. ESTILO ANTERIOR: "QUADRADO / BLOCOS SOLTOS"
# -------------------------------------------------------------
draw.text((20, 10), "1. ESTILO ANTERIOR: QUADRADO (Blocos Retangulares com Bordas Retas e Degraus 90°)", fill=(240, 100, 100))
mid_y1 = 70

bar_w = 2
bar_gap = 1
bar_step = 3
num_bars = (img_w - 40) // bar_step
bin_indices = np.linspace(0, num_bins - 1, num_bars).astype(int)

for i, b_idx in enumerate(bin_indices):
    x0 = 20 + i * bar_step
    x1 = x0 + bar_w
    l, m, h = lows[b_idx], mids[b_idx], highs[b_idx]
    
    hg = float(np.round((h ** 0.85) * (max_h * 0.32)))
    hc = float(np.round((m ** 0.85) * (max_h * 0.42)))
    hp = float(np.round(((l - 0.08) / 0.92) ** 0.80 * (max_h * 0.62))) if l > 0.08 else 0.0
    tot = hg + hc + hp
    if tot > max_h:
        sc = max_h / tot; hg = np.round(hg * sc); hc = np.round(hc * sc); hp = max_h - hg - hc
    if tot < 1.0: hg = 1.0
    
    # Sharp flat rectangles
    if hg > 0:
        draw.rectangle([x0, mid_y1 - hg, x1 - 1, mid_y1 + hg], fill=(57, 255, 20))
    if hc > 0:
        draw.rectangle([x0, mid_y1 - hg - hc, x1 - 1, mid_y1 - hg], fill=(0, 229, 255))
        draw.rectangle([x0, mid_y1 + hg, x1 - 1, mid_y1 + hg + hc], fill=(0, 229, 255))
    if hp > 0:
        draw.rectangle([x0, mid_y1 - hg - hc - hp, x1 - 1, mid_y1 - hg - hc], fill=(255, 0, 128))
        draw.rectangle([x0, mid_y1 + hg + hc, x1 - 1, mid_y1 + hg + hc + hp], fill=(255, 0, 128))
    draw.line([x0, mid_y1, x1 - 1, mid_y1], fill=(255, 255, 255))

# -------------------------------------------------------------
# 2. NOVO ESTILO: "PARECE UM DESENHO" (Agulhas Finas Densas, Gradiente Vertical Suave e Silhueta Organica)
# -------------------------------------------------------------
draw.text((20, 160), "2. NOVO ESTILO: 'PARECE UM DESENHO' (Densidade 1px, Gradiente Vertical Suave e Curvas Flutuantes)", fill=(100, 240, 150))
mid_y2 = 230

# Usamos passo de 1px com interpolação suave
num_lines = img_w - 40
x_coords = np.linspace(0, num_bins - 1, num_lines)

# Envelope follower / smoothing across bins (para criar a curva orgânica de desenho)
lows_smooth  = np.interp(x_coords, np.arange(num_bins), lows)
mids_smooth  = np.interp(x_coords, np.arange(num_bins), mids)
highs_smooth = np.interp(x_coords, np.arange(num_bins), highs)

# Aplicar um leve gaussian/box filter para eliminar serrilhado quadrado
k = np.ones(5) / 5.0
lows_smooth  = np.convolve(lows_smooth, k, mode='same')
mids_smooth  = np.convolve(mids_smooth, k, mode='same')
highs_smooth = np.convolve(highs_smooth, k, mode='same')

for x in range(num_lines):
    cur_x = 20 + x
    l = lows_smooth[x]
    m = mids_smooth[x]
    h = highs_smooth[x]
    
    hg = (h ** 0.85) * (max_h * 0.32)
    hc = (m ** 0.85) * (max_h * 0.42)
    hp = (((l - 0.08) / 0.92) ** 0.80 * (max_h * 0.62)) if l > 0.08 else 0.0
    tot = hg + hc + hp
    if tot > max_h:
        sc = max_h / tot; hg *= sc; hc *= sc; hp = max_h - hg - hc
    if tot < 1.0: hg = 1.0
    
    # Desenhar linha vertical com transição de cor suave pixel a pixel (como no Rekordbox)
    tot_int = int(np.round(tot))
    y_high_limit = int(np.round(hg))
    y_mid_limit  = int(np.round(hg + hc))
    
    for dy in range(tot_int + 1):
        # Determinar cor suave dependendo da distância dy
        if dy <= y_high_limit:
            # Núcleo Verde/Branco com transição
            t = dy / max(1.0, y_high_limit)
            r = int(220 * (1.0 - t) + 57 * t)
            g = 255
            b = int(220 * (1.0 - t) + 20 * t)
        elif dy <= y_mid_limit:
            # Corpo Ciano
            t = (dy - y_high_limit) / max(1.0, y_mid_limit - y_high_limit)
            r = int(57 * (1.0 - t) + 0 * t)
            g = int(255 * (1.0 - t) + 229 * t)
            b = int(20 * (1.0 - t) + 255 * t)
        else:
            # Camada Rosa
            t = (dy - y_mid_limit) / max(1.0, tot_int - y_mid_limit)
            r = int(0 * (1.0 - t) + 255 * t)
            g = int(229 * (1.0 - t) + 0 * t)
            b = int(255 * (1.0 - t) + 128 * t)
            
        col = (r, g, b)
        # Topo
        draw.point((cur_x, mid_y2 - dy), fill=col)
        # Base (simetria)
        draw.point((cur_x, mid_y2 + dy), fill=col)

im.save('comparison_blocky_vs_drawing.png')
print("Saved comparison_blocky_vs_drawing.png")
