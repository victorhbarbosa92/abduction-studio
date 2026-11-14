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
    count = int(sr * 6.0)
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

for b in range(num_bins):
    s = b * STEP
    e = s + STEP
    lows[b]  = np.max(np.abs(low[s:e]))
    mids[b]  = np.max(np.abs(mid[s:e]))
    highs[b] = np.max(np.abs(high[s:e]))

max_l = max(np.max(lows), 0.001)
max_m = max(np.max(mids), 0.001)
max_h_val = max(np.max(highs), 0.001)

lows  = np.clip(lows  / max_l, 0.0, 1.0)
mids  = np.clip(mids  / max_m, 0.0, 1.0)
highs = np.clip(highs / max_h_val, 0.0, 1.0)

img_w = 800
img_h = 320
im = Image.new('RGB', (img_w, img_h), (4, 6, 9))
draw = ImageDraw.Draw(im)

max_h = 48
mid_y1 = 80
mid_y2 = 230

# High density step: 1.5px
step = 1.5
num_pts = int((img_w - 40) / step)
indices = np.linspace(0, num_bins - 1, num_pts)

l_smooth = np.interp(indices, np.arange(num_bins), lows)
m_smooth = np.interp(indices, np.arange(num_bins), mids)
h_smooth = np.interp(indices, np.arange(num_bins), highs)

# Leve suavização de envelope para eliminar ruído quadrado
k = np.array([0.15, 0.70, 0.15])
l_smooth = np.convolve(l_smooth, k, mode='same')
m_smooth = np.convolve(m_smooth, k, mode='same')
h_smooth = np.convolve(h_smooth, k, mode='same')

# 1. RGB Mode: Malha Contínua + Micro-agulhas
draw.text((20, 10), "1. REKORDBOX RGB - MALHA CONTÍNUA VETORIAL (Sem Degraus / Sem Quadrados)", fill=(220, 220, 220))
draw.rectangle([15, mid_y1 - max_h - 10, img_w - 15, mid_y1 + max_h + 10], fill=(2, 3, 5))
draw.line([15, mid_y1, img_w - 15, mid_y1], fill=(20, 26, 36))

pts_hig_t, pts_hig_b = [], []
pts_mid_t, pts_mid_b = [], []
pts_low_t, pts_low_b = [], []

for i in range(num_pts):
    cx = 20 + i * step
    l, m, h = l_smooth[i], m_smooth[i], h_smooth[i]
    hg = (h ** 0.85) * (max_h * 0.32)
    hc = (m ** 0.85) * (max_h * 0.42)
    hp = (((l - 0.08) / 0.92) ** 0.80 * (max_h * 0.62)) if l > 0.08 else 0.0
    tot = hg + hc + hp
    if tot > max_h:
        sc = max_h / tot; hg *= sc; hc *= sc; hp = max_h - hg - hc
    if tot < 1.0: hg = 1.0
    
    pts_hig_t.append((cx, mid_y1 - hg))
    pts_hig_b.append((cx, mid_y1 + hg))
    pts_mid_t.append((cx, mid_y1 - hg - hc))
    pts_mid_b.append((cx, mid_y1 + hg + hc))
    pts_low_t.append((cx, mid_y1 - hg - hc - hp))
    pts_low_b.append((cx, mid_y1 + hg + hc + hp))

for i in range(num_pts - 1):
    # Low quads
    draw.polygon([pts_low_t[i], pts_low_t[i+1], pts_mid_t[i+1], pts_mid_t[i]], fill=(255, 0, 128))
    draw.polygon([pts_mid_b[i], pts_mid_b[i+1], pts_low_b[i+1], pts_low_b[i]], fill=(255, 0, 128))
    # Mid quads
    draw.polygon([pts_mid_t[i], pts_mid_t[i+1], pts_hig_t[i+1], pts_hig_t[i]], fill=(0, 229, 255))
    draw.polygon([pts_hig_b[i], pts_hig_b[i+1], pts_mid_b[i+1], pts_mid_b[i]], fill=(0, 229, 255))
    # High core quads
    draw.polygon([pts_hig_t[i], pts_hig_t[i+1], pts_hig_b[i+1], pts_hig_b[i]], fill=(57, 255, 20))
    # Needle overlay (delicate 1px stroke)
    if i % 2 == 0:
        draw.line([pts_low_t[i], pts_low_b[i]], fill=(255, 0, 128, 40))
draw.line([(20, mid_y1), (img_w - 20, mid_y1)], fill=(240, 255, 240))

# 2. 3Band Mode (CDJ-3000): Malha Contínua
draw.text((20, 160), "2. REKORDBOX 3BAND (CDJ-3000) - MALHA CONTÍNUA VETORIAL (Blue / Amber / White)", fill=(220, 220, 220))
draw.rectangle([15, mid_y2 - max_h - 10, img_w - 15, mid_y2 + max_h + 10], fill=(2, 3, 5))
draw.line([15, mid_y2, img_w - 15, mid_y2], fill=(20, 26, 36))

pts3_hig_t, pts3_hig_b = [], []
pts3_mid_t, pts3_mid_b = [], []
pts3_low_t, pts3_low_b = [], []

for i in range(num_pts):
    cx = 20 + i * step
    l, m, h = l_smooth[i], m_smooth[i], h_smooth[i]
    hw = (h ** 0.85) * (max_h * 0.32)
    ha = (m ** 0.85) * (max_h * 0.42)
    hb = (((l - 0.08) / 0.92) ** 0.80 * (max_h * 0.62)) if l > 0.08 else 0.0
    tot = hw + ha + hb
    if tot > max_h:
        sc = max_h / tot; hw *= sc; ha *= sc; hb = max_h - hw - ha
    if tot < 1.0: hw = 1.0
    
    pts3_hig_t.append((cx, mid_y2 - hw))
    pts3_hig_b.append((cx, mid_y2 + hw))
    pts3_mid_t.append((cx, mid_y2 - hw - ha))
    pts3_mid_b.append((cx, mid_y2 + hw + ha))
    pts3_low_t.append((cx, mid_y2 - hw - ha - hb))
    pts3_low_b.append((cx, mid_y2 + hw + ha + hb))

for i in range(num_pts - 1):
    # Blue Low quads
    draw.polygon([pts3_low_t[i], pts3_low_t[i+1], pts3_mid_t[i+1], pts3_mid_t[i]], fill=(0, 115, 255))
    draw.polygon([pts3_mid_b[i], pts3_mid_b[i+1], pts3_low_b[i+1], pts3_low_b[i]], fill=(0, 115, 255))
    # Amber Mid quads
    draw.polygon([pts3_mid_t[i], pts3_mid_t[i+1], pts3_hig_t[i+1], pts3_hig_t[i]], fill=(255, 144, 0))
    draw.polygon([pts3_hig_b[i], pts3_hig_b[i+1], pts3_mid_b[i+1], pts3_mid_b[i]], fill=(255, 144, 0))
    # White High core
    draw.polygon([pts3_hig_t[i], pts3_hig_t[i+1], pts3_hig_b[i+1], pts3_hig_b[i]], fill=(255, 255, 255))

draw.line([(20, mid_y2), (img_w - 20, mid_y2)], fill=(255, 255, 255))

im.save('hybrid_vector_mesh_preview.png')
print("Saved hybrid_vector_mesh_preview.png successfully!")
