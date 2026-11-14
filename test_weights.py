import numpy as np

# Test the weighting:
def test_weights(l, m, h, w_l=2.2, w_m=0.85, w_h=1.4):
    l_w = l * w_l
    m_w = m * w_m
    h_w = h * w_h
    max_v = max(l_w, max(m_w, h_w), 1e-4)
    r = l_w / max_v
    g = m_w / max_v
    b = h_w / max_v
    max_rb = max(r, b)
    damp = min(max_rb, 0.5) * 0.4
    g = (1.0 - damp) * g
    b = b * 1.3
    r = min(1.0, max(0.0, r))
    g = min(1.0, max(0.0, g))
    b = min(1.0, max(0.0, b))
    return int(r * 255), int(g * 255), int(b * 255)

print("Kick (l=0.8, m=0.6, h=0.2):", test_weights(0.8, 0.6, 0.2))
print("Snare (l=0.4, m=0.5, h=0.7):", test_weights(0.4, 0.5, 0.7))
print("Hat (l=0.05, m=0.2, h=0.8):", test_weights(0.05, 0.2, 0.8))
print("Mid synth (l=0.2, m=0.8, h=0.2):", test_weights(0.2, 0.8, 0.2))
