import wave
import numpy as np
from scipy import signal
from PIL import Image, ImageDraw

# Load test track
wav_path = 'scratch/test_cliff.wav'
with wave.open(wav_path, 'rb') as wf:
    sr = wf.getframerate()
    n_ch = wf.getnchannels()
    n_frames = wf.getnframes()
    # Read 12 seconds with dynamic section
    skip = int(sr * 12)
    wf.setpos(skip)
    count = int(sr * 12)
    data = wf.readframes(count)
    audio = np.frombuffer(data, dtype=np.int16).astype(np.float32) / 32768.0
    if n_ch == 2:
        audio = audio.reshape(-1, 2)
        mono = (audio[:, 0] + audio[:, 1]) * 0.5
    else:
        mono = audio

# DSP: Linkwitz-Riley 4th order (2 cascaded butterworth 2nd order)
# Low-pass at 250 Hz
b_l, a_l = signal.butter(2, 250.0 / (sr * 0.5), btype='low')
low1 = signal.lfilter(b_l, a_l, mono)
low_sig = signal.lfilter(b_l, a_l, low1)

# High-pass at 2500 Hz
b_h, a_h = signal.butter(2, 2500.0 / (sr * 0.5), btype='high')
high1 = signal.lfilter(b_h, a_h, mono)
high_sig = signal.lfilter(b_h, a_h, high1)

# Mid-pass: complementary or band-pass
b_m, a_m = signal.butter(2, [250.0 / (sr * 0.5), 2500.0 / (sr * 0.5)], btype='band')
mid1 = signal.lfilter(b_m, a_m, mono)
mid_sig = signal.lfilter(b_m, a_m, mid1)

STEP = 128
num_bins = len(mono) // STEP

peaks = np.zeros(num_bins, dtype=np.float32)
lows  = np.zeros(num_bins, dtype=np.float32)
mids  = np.zeros(num_bins, dtype=np.float32)
highs = np.zeros(num_bins, dtype=np.float32)

for b in range(num_bins):
    s = b * STEP
    e = s + STEP
    peaks[b] = np.max(np.abs(mono[s:e]))
    lows[b]  = np.max(np.abs(low_sig[s:e]))
    mids[b]  = np.max(np.abs(mid_sig[s:e]))
    highs[b] = np.max(np.abs(high_sig[s:e]))

# Scale to realistic 0..1 ranges
lows = np.clip(lows * 1.50, 0.0, 1.0)
mids = np.clip(mids * 1.65, 0.0, 1.0)
highs = np.clip(highs * 2.10, 0.0, 1.0)
peaks = np.clip(peaks, 0.0, 1.0)

img_w = 1100
img_h = 500
im = Image.new('RGB', (img_w, img_h), (0, 0, 0))
draw = ImageDraw.Draw(im)

bar_w = 2
bar_gap = 1
bar_step = 3
num_bars = (img_w - 60) // bar_step
bin_indices = np.linspace(0, num_bins - 1, num_bars).astype(int)
max_h = 58

# -------------------------------------------------------------
# PIONEER REKORDBOX 3BAND (CDJ-3000 Standard)
# -------------------------------------------------------------
draw.text((30, 15), "PIONEER REKORDBOX 3BAND (CDJ-3000 Oficial: Blue Lows, Amber Mids, White Highs)", fill=(220, 220, 220))
mid_y1 = 120

# Draw background lane
draw.rectangle([25, mid_y1 - max_h - 10, img_w - 25, mid_y1 + max_h + 10], fill=(5, 7, 10))
draw.line([25, mid_y1, img_w - 25, mid_y1], fill=(20, 28, 40))

for i, b_idx in enumerate(bin_indices):
    x0 = 30 + i * bar_step
    x1 = x0 + bar_w
    l, m, h, p = lows[b_idx], mids[b_idx], highs[b_idx], peaks[b_idx]
    
    # In Rekordbox 3Band:
    # Each band has an independent amplitude height:
    hl = int((l ** 0.85) * max_h)
    hm = int((m ** 0.88) * (max_h * 0.85))
    hh = int((h ** 0.92) * (max_h * 0.75))
    
    # Low = Royal Electric Blue
    if hl > 0:
        draw.rectangle([x0, mid_y1 - hl, x1 - 1, mid_y1 + hl], fill=(0, 115, 255))
    # Mid = Vibrant Amber Orange (drawn over low)
    if hm > 0:
        draw.rectangle([x0, mid_y1 - hm, x1 - 1, mid_y1 + hm], fill=(255, 140, 0))
    # High = Pure Crisp White Needle (drawn over mid)
    if hh > 0:
        draw.rectangle([x0, mid_y1 - hh, x1 - 1, mid_y1 + hh], fill=(255, 255, 255))

# -------------------------------------------------------------
# PIONEER REKORDBOX RGB (CDJ-2000NXS2 Standard)
# -------------------------------------------------------------
draw.text((30, 250), "PIONEER REKORDBOX RGB (CDJ-2000NXS2: Magenta Lows, Cyan Mids, Green Highs + White Spikes)", fill=(220, 220, 220))
mid_y2 = 360

# Draw background lane
draw.rectangle([25, mid_y2 - max_h - 10, img_w - 25, mid_y2 + max_h + 10], fill=(5, 7, 10))
draw.line([25, mid_y2, img_w - 25, mid_y2], fill=(20, 28, 40))

for i, b_idx in enumerate(bin_indices):
    x0 = 30 + i * bar_step
    x1 = x0 + bar_w
    l, m, h, p = lows[b_idx], mids[b_idx], highs[b_idx], peaks[b_idx]
    
    # In Rekordbox RGB:
    # Kicks project deep Magenta Pink outwards.
    # Mids form the cyan musical body.
    # Highs form the lime green transients and white spikes!
    hl = int((l ** 0.82) * max_h)
    hm = int((m ** 0.88) * (max_h * 0.80))
    hg = int((h ** 0.92) * (max_h * 0.65))
    
    # Draw Pink Low envelope
    if hl > 0:
        draw.rectangle([x0, mid_y2 - hl, x1 - 1, mid_y2 + hl], fill=(255, 0, 128))
    # Draw Cyan Mid envelope
    if hm > 0:
        draw.rectangle([x0, mid_y2 - hm, x1 - 1, mid_y2 + hm], fill=(0, 229, 255))
    # Draw Lime Green High envelope
    if hg > 0:
        draw.rectangle([x0, mid_y2 - hg, x1 - 1, mid_y2 + hg], fill=(57, 255, 20))
    # Transient White core (when high energy is strong)
    if h > 0.4:
        hw = int((h ** 1.1) * (max_h * 0.45))
        draw.rectangle([x0, mid_y2 - hw, x1 - 1, mid_y2 + hw], fill=(230, 255, 240))
    # Zero crossing hairline
    draw.line([x0, mid_y2, x1 - 1, mid_y2], fill=(255, 255, 255))

im.save('rekordbox_exact_simulation.png')
print("Saved rekordbox_exact_simulation.png successfully!")
