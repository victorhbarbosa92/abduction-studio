import wave
import numpy as np
from scipy import signal
from PIL import Image, ImageDraw

wav_path = 'scratch/test_cliff.wav'
with wave.open(wav_path, 'rb') as wf:
    sr = wf.getframerate()
    n_ch = wf.getnchannels()
    n_frames = wf.getnframes()
    skip = int(sr * 4)
    wf.setpos(skip)
    count = int(sr * 16)
    data = wf.readframes(count)
    audio = np.frombuffer(data, dtype=np.int16).astype(np.float32) / 32768.0
    if n_ch == 2:
        audio = audio.reshape(-1, 2)
        mono = (audio[:, 0] + audio[:, 1]) * 0.5
    else:
        mono = audio

# Linkwitz-Riley crossover (4th order: 2 cascaded Butterworth 2nd order)
# Low: 250 Hz
b_l, a_l = signal.butter(2, 250.0 / (sr * 0.5), btype='low')
low = signal.lfilter(b_l, a_l, signal.lfilter(b_l, a_l, mono))

# High: 2500 Hz
b_h, a_h = signal.butter(2, 2500.0 / (sr * 0.5), btype='high')
high = signal.lfilter(b_h, a_h, signal.lfilter(b_h, a_h, mono))

# Mid: complementary mono - low - high
mid = mono - low - high

STEP = 128
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

# Dynamic normalization (preserve true acoustic balance)
max_val = max(np.max(lows), np.max(mids), np.max(highs), np.max(peaks), 0.001)
lows  = np.clip(lows  / max_val * 1.05, 0.0, 1.0)
mids  = np.clip(mids  / max_val * 1.35, 0.0, 1.0)
highs = np.clip(highs / max_val * 1.65, 0.0, 1.0)
peaks = np.clip(peaks / max_val * 1.00, 0.0, 1.0)

img_w = 1100
img_h = 360
im = Image.new('RGB', (img_w, img_h), (0, 0, 0))
draw = ImageDraw.Draw(im)

bar_w = 2
bar_gap = 1
bar_step = 3
num_bars = (img_w - 60) // bar_step
bin_indices = np.linspace(0, num_bins - 1, num_bars).astype(int)
max_h = 55

# -------------------------------------------------------------
# 1. PIONEER REKORDBOX 3BAND (CDJ-3000 Standard)
# -------------------------------------------------------------
draw.text((30, 10), "PIONEER REKORDBOX 3BAND (CDJ-3000 OFICIAL: Blue Low, Amber Mid, White High)", fill=(220, 220, 220))
mid_y1 = 85
draw.rectangle([25, mid_y1 - max_h - 10, img_w - 25, mid_y1 + max_h + 10], fill=(4, 6, 8))
draw.line([25, mid_y1, img_w - 25, mid_y1], fill=(22, 30, 42))

for i, b_idx in enumerate(bin_indices):
    x0 = 30 + i * bar_step
    x1 = x0 + bar_w
    l, m, h = lows[b_idx], mids[b_idx], highs[b_idx]
    
    # Each band can independently reach up to max_h!
    hl = int((l ** 0.85) * max_h)
    hm = int((m ** 0.85) * max_h)
    hh = int((h ** 0.85) * max_h)
    
    # 1. Low: Blue (#0073FF)
    if hl > 0:
        draw.rectangle([x0, mid_y1 - hl, x1 - 1, mid_y1 + hl], fill=(0, 115, 255))
    # 2. Mid: Amber Orange (#FF8C00)
    if hm > 0:
        draw.rectangle([x0, mid_y1 - hm, x1 - 1, mid_y1 + hm], fill=(255, 140, 0))
    # 3. High: Crisp White (#FFFFFF)
    if hh > 0:
        draw.rectangle([x0, mid_y1 - hh, x1 - 1, mid_y1 + hh], fill=(255, 255, 255))

# -------------------------------------------------------------
# 2. PIONEER REKORDBOX RGB (CDJ-2000NXS2 Standard)
# -------------------------------------------------------------
draw.text((30, 185), "PIONEER REKORDBOX RGB (CDJ-2000NXS2 OFICIAL: Magenta Low, Cyan Mid, Lime High)", fill=(220, 220, 220))
mid_y2 = 265
draw.rectangle([25, mid_y2 - max_h - 10, img_w - 25, mid_y2 + max_h + 10], fill=(4, 6, 8))
draw.line([25, mid_y2, img_w - 25, mid_y2], fill=(22, 30, 42))

for i, b_idx in enumerate(bin_indices):
    x0 = 30 + i * bar_step
    x1 = x0 + bar_w
    l, m, h = lows[b_idx], mids[b_idx], highs[b_idx]
    
    hl = int((l ** 0.85) * max_h)
    hm = int((m ** 0.85) * max_h)
    hh = int((h ** 0.85) * max_h)
    
    # 1. Low: Neon Magenta (#FF0080)
    if hl > 0:
        draw.rectangle([x0, mid_y2 - hl, x1 - 1, mid_y2 + hl], fill=(255, 0, 128))
    # 2. Mid: Cyan (#00E5FF)
    if hm > 0:
        draw.rectangle([x0, mid_y2 - hm, x1 - 1, mid_y2 + hm], fill=(0, 229, 255))
    # 3. High: Lime Green (#39FF14)
    if hh > 0:
        draw.rectangle([x0, mid_y2 - hh, x1 - 1, mid_y2 + hh], fill=(57, 255, 20))
    # 4. White core for sharp high transients
    if h > 0.40:
        hw = int((h ** 1.1) * (max_h * 0.70))
        draw.rectangle([x0, mid_y2 - hw, x1 - 1, mid_y2 + hw], fill=(240, 255, 240))

im.save('rekordbox_true_layering.png')
print("Saved rekordbox_true_layering.png successfully!")
