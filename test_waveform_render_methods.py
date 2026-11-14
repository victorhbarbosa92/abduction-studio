import wave
import numpy as np
from scipy import signal
from PIL import Image, ImageDraw

wav_path = 'scratch/test_cliff.wav'
with wave.open(wav_path, 'rb') as wf:
    sr = wf.getframerate()
    n_ch = wf.getnchannels()
    n_frames = wf.getnframes()
    skip = int(sr * 8)
    wf.setpos(skip)
    count = int(sr * 14)
    data = wf.readframes(count)
    audio = np.frombuffer(data, dtype=np.int16).astype(np.float32) / 32768.0
    if n_ch == 2:
        audio = audio.reshape(-1, 2)
        mono = (audio[:, 0] + audio[:, 1]) * 0.5
    else:
        mono = audio

b_low, a_low = signal.butter(2, 250.0 / (sr * 0.5), btype='low')
low_sig = signal.lfilter(b_low, a_low, mono)

b_high, a_high = signal.butter(2, 2500.0 / (sr * 0.5), btype='high')
high_sig = signal.lfilter(b_high, a_high, mono)

mid_sig = mono - low_sig - high_sig

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

lows = np.clip(lows * 1.35, 0.0, 1.0)
mids = np.clip(mids * 1.50, 0.0, 1.0)
highs = np.clip(highs * 1.80, 0.0, 1.0)
peaks = np.clip(peaks, 0.0, 1.0)

img_w = 900
img_h = 560
im = Image.new('RGB', (img_w, img_h), (8, 11, 16))
draw = ImageDraw.Draw(im)

bar_w = 2
bar_gap = 1
bar_step = 3
num_bars = (img_w - 40) // bar_step
bin_indices = np.linspace(0, num_bins - 1, num_bars).astype(int)
max_h = 48

# 1. Old Flawed Method
draw.text((20, 10), "1. METODO ANTERIOR (Fatia Proporcional Fixa - Listrado/Artificial)", fill=(200, 200, 200))
mid_y1 = 80
for i, b_idx in enumerate(bin_indices):
    x0 = 20 + i * bar_step
    x1 = x0 + bar_w
    l, m, h, p = lows[b_idx], mids[b_idx], highs[b_idx], peaks[b_idx]
    l_exp, m_exp, h_exp, p_exp = l**0.80, m**0.85, h**0.85, p**0.75
    sum_b = l_exp * 1.20 + m_exp * 1.05 + h_exp * 1.00 + 0.0001
    tot_h = max(1.0, min(max_h, p_exp * max_h))
    prop_g = (h_exp * 1.00) / sum_b
    prop_c = (m_exp * 1.05) / sum_b
    hg = round(tot_h * prop_g)
    hc = round(tot_h * prop_c)
    draw.rectangle([x0, mid_y1 - hg, x1 - 1, mid_y1], fill=(57, 255, 20))
    draw.rectangle([x0, mid_y1 - hg - hc, x1 - 1, mid_y1 - hg], fill=(0, 229, 255))
    draw.rectangle([x0, mid_y1 - tot_h, x1 - 1, mid_y1 - hg - hc], fill=(255, 0, 128))
    draw.rectangle([x0, mid_y1, x1 - 1, mid_y1 + hg], fill=(57, 255, 20))
    draw.rectangle([x0, mid_y1 + hg, x1 - 1, mid_y1 + hg + hc], fill=(0, 229, 255))
    draw.rectangle([x0, mid_y1 + hg + hc, x1 - 1, mid_y1 + tot_h], fill=(255, 0, 128))

# 2. Rekordbox 3Band
draw.text((20, 145), "2. REKORDBOX 3BAND (CDJ-3000: Low=Azul, Mid=Laranja, High=Branco)", fill=(200, 200, 200))
mid_y2 = 215
for i, b_idx in enumerate(bin_indices):
    x0 = 20 + i * bar_step
    x1 = x0 + bar_w
    l, m, h = lows[b_idx], mids[b_idx], highs[b_idx]
    hl = int((l ** 0.85) * max_h)
    hm = int((m ** 0.90) * (max_h * 0.82))
    hh = int((h ** 0.95) * (max_h * 0.70))
    if hl > 0:
        draw.rectangle([x0, mid_y2 - hl, x1 - 1, mid_y2 + hl], fill=(0, 115, 255))
    if hm > 0:
        draw.rectangle([x0, mid_y2 - hm, x1 - 1, mid_y2 + hm], fill=(255, 144, 0))
    if hh > 0:
        draw.rectangle([x0, mid_y2 - hh, x1 - 1, mid_y2 + hh], fill=(255, 255, 255))

# 3. Rekordbox RGB
draw.text((20, 280), "3. REKORDBOX RGB AUTENTICO (Nucleo Verde/Branco, Corpo Ciano, Picos Rosa em Graves)", fill=(200, 200, 200))
mid_y3 = 350
for i, b_idx in enumerate(bin_indices):
    x0 = 20 + i * bar_step
    x1 = x0 + bar_w
    l, m, h = lows[b_idx], mids[b_idx], highs[b_idx]
    hg = int((h ** 0.90) * (max_h * 0.45))
    hm = int((m ** 0.85) * (max_h * 0.65))
    hl = int((l ** 0.80) * (max_h * 0.95)) if l > 0.08 else 0
    if hl > 0:
        draw.rectangle([x0, mid_y3 - hl, x1 - 1, mid_y3 + hl], fill=(255, 0, 128))
    if hm > 0:
        draw.rectangle([x0, mid_y3 - hm, x1 - 1, mid_y3 + hm], fill=(0, 229, 255))
    if hg > 0:
        draw.rectangle([x0, mid_y3 - hg, x1 - 1, mid_y3 + hg], fill=(57, 255, 20))
    draw.line([x0, mid_y3, x1 - 1, mid_y3], fill=(230, 255, 230))

# 4. Traktor Pro Spectrum
draw.text((20, 420), "4. TRAKTOR PRO SPECTRUM RGB (Fusao Continua de Cores: Vermelho=Grave, Ciano=Medio, Azul=Agudo)", fill=(200, 200, 200))
mid_y4 = 490
for i, b_idx in enumerate(bin_indices):
    x0 = 20 + i * bar_step
    x1 = x0 + bar_w
    l, m, h, p = lows[b_idx], mids[b_idx], highs[b_idx], peaks[b_idx]
    tot_h = int((p ** 0.80) * max_h)
    if tot_h < 1: tot_h = 1
    sum_e = l * 1.4 + m * 1.1 + h * 1.0 + 0.0001
    r_val = int(min(255, (l * 1.8 / sum_e) * 255))
    g_val = int(min(255, (m * 1.6 / sum_e) * 255))
    b_val = int(min(255, (h * 2.0 / sum_e) * 255))
    col = (r_val, g_val, b_val)
    draw.rectangle([x0, mid_y4 - tot_h, x1 - 1, mid_y4 + tot_h], fill=col)
    core_h = max(1, int(tot_h * 0.35))
    draw.rectangle([x0, mid_y4 - core_h, x1 - 1, mid_y4 + core_h], fill=(min(255, r_val + 100), min(255, g_val + 100), min(255, b_val + 100)))

im.save('comparison_waveform_rendering_methods.png')
print("Successfully generated comparison_waveform_rendering_methods.png")
