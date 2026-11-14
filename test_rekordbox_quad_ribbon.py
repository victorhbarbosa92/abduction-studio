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

max_l = max(np.max(lows), 0.001)
max_m = max(np.max(mids), 0.001)
max_h_val = max(np.max(highs), 0.001)
max_p = max(np.max(peaks), 0.001)

lows  = np.clip(lows  / max_l, 0.0, 1.0)
mids  = np.clip(mids  / max_m, 0.0, 1.0)
highs = np.clip(highs / max_h_val, 0.0, 1.0)
peaks = np.clip(peaks / max_p, 0.0, 1.0)

img_w = 720
img_h = 360
im = Image.new('RGB', (img_w, img_h), (6, 8, 12))
draw = ImageDraw.Draw(im)

max_h = 45
mid_y = 75

# -------------------------------------------------------------------------
# Estilo A: Pioneer Rekordbox Fine Vector Needles (1.5px pitch, antialiased lines)
# -------------------------------------------------------------------------
draw.text((20, 10), "A. REKORDBOX: AGULHAS VETORIAIS DE ALTA DENSIDADE (Passo 1.5px, Traço Fino e Fundo Macio)", fill=(200, 200, 200))
step = 1.5
num_needles = int((img_w - 40) / step)
indices = np.linspace(0, num_bins - 1, num_needles)

# Interpolate & smooth
l_interp = np.interp(indices, np.arange(num_bins), lows)
m_interp = np.interp(indices, np.arange(num_bins), mids)
h_interp = np.interp(indices, np.arange(num_bins), highs)

# Leve suavização de envelope
k = np.array([0.15, 0.70, 0.15])
l_interp = np.convolve(l_interp, k, mode='same')
m_interp = np.convolve(m_interp, k, mode='same')
h_interp = np.convolve(h_interp, k, mode='same')

for i in range(num_needles):
    cx = 20 + i * step
    l, m, h = l_interp[i], m_interp[i], h_interp[i]
    
    hg = (h ** 0.85) * (max_h * 0.32)
    hc = (m ** 0.85) * (max_h * 0.42)
    hp = (((l - 0.08) / 0.92) ** 0.80 * (max_h * 0.62)) if l > 0.08 else 0.0
    tot = hg + hc + hp
    if tot > max_h:
        sc = max_h / tot; hg *= sc; hc *= sc; hp = max_h - hg - hc
    if tot < 1.0: hg = 1.0
    
    # 1. Low: Pink (Magenta)
    if hp > 0.5:
        draw.line([(cx, mid_y - hg - hc - hp), (cx, mid_y - hg - hc)], fill=(255, 0, 128), width=1)
        draw.line([(cx, mid_y + hg + hc), (cx, mid_y + hg + hc + hp)], fill=(255, 0, 128), width=1)
    # 2. Mid: Cyan
    if hc > 0.5:
        draw.line([(cx, mid_y - hg - hc), (cx, mid_y - hg)], fill=(0, 229, 255), width=1)
        draw.line([(cx, mid_y + hg), (cx, mid_y + hg + hc)], fill=(0, 229, 255), width=1)
    # 3. High: Lime Green
    if hg > 0.5:
        draw.line([(cx, mid_y - hg), (cx, mid_y + hg)], fill=(57, 255, 20), width=1)
    # 4. White core
    draw.point((cx, mid_y), fill=(240, 255, 240))

# -------------------------------------------------------------------------
# Estilo B: Traktor / Rekordbox Continuous Polygon Mesh (Faixas Trapezoidais Conectadas)
# -------------------------------------------------------------------------
draw.text((20, 180), "B. TRAKTOR / REKORDBOX: MALHA VETORIAL CONTÍNUA (Trapézios Conectados Sem Serrilhado)", fill=(100, 240, 150))
mid_y2 = 260

pts_low_top = []
pts_low_bot = []
pts_mid_top = []
pts_mid_bot = []
pts_hig_top = []
pts_hig_bot = []

for i in range(num_needles):
    cx = 20 + i * step
    l, m, h = l_interp[i], m_interp[i], h_interp[i]
    
    hg = (h ** 0.85) * (max_h * 0.32)
    hc = (m ** 0.85) * (max_h * 0.42)
    hp = (((l - 0.08) / 0.92) ** 0.80 * (max_h * 0.62)) if l > 0.08 else 0.0
    tot = hg + hc + hp
    if tot > max_h:
        sc = max_h / tot; hg *= sc; hc *= sc; hp = max_h - hg - hc
    if tot < 1.0: hg = 1.0
    
    pts_hig_top.append((cx, mid_y2 - hg))
    pts_hig_bot.append((cx, mid_y2 + hg))
    pts_mid_top.append((cx, mid_y2 - hg - hc))
    pts_mid_bot.append((cx, mid_y2 + hg + hc))
    pts_low_top.append((cx, mid_y2 - hg - hc - hp))
    pts_low_bot.append((cx, mid_y2 + hg + hc + hp))

# Draw connected quads for each band
for i in range(num_needles - 1):
    # Low Top Quad
    p1 = pts_low_top[i]
    p2 = pts_low_top[i+1]
    p3 = pts_mid_top[i+1]
    p4 = pts_mid_top[i]
    if abs(p1[1] - p4[1]) > 0.5 or abs(p2[1] - p3[1]) > 0.5:
        draw.polygon([p1, p2, p3, p4], fill=(255, 0, 128))
    # Low Bot Quad
    p1_b = pts_mid_bot[i]
    p2_b = pts_mid_bot[i+1]
    p3_b = pts_low_bot[i+1]
    p4_b = pts_low_bot[i]
    if abs(p4_b[1] - p1_b[1]) > 0.5 or abs(p3_b[1] - p2_b[1]) > 0.5:
        draw.polygon([p1_b, p2_b, p3_b, p4_b], fill=(255, 0, 128))
        
    # Mid Top Quad
    draw.polygon([pts_mid_top[i], pts_mid_top[i+1], pts_hig_top[i+1], pts_hig_top[i]], fill=(0, 229, 255))
    # Mid Bot Quad
    draw.polygon([pts_hig_bot[i], pts_hig_bot[i+1], pts_mid_bot[i+1], pts_mid_bot[i]], fill=(0, 229, 255))
    
    # High Core Quad
    draw.polygon([pts_hig_top[i], pts_hig_top[i+1], pts_hig_bot[i+1], pts_hig_bot[i]], fill=(57, 255, 20))
    # Centerline
    draw.line([(pts_hig_top[i][0], mid_y2), (pts_hig_top[i+1][0], mid_y2)], fill=(240, 255, 240))

im.save('comparison_needles_vs_mesh.png')
print("Saved comparison_needles_vs_mesh.png")
