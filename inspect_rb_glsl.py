with open(r'C:\Program Files\rekordbox\rekordbox 7.2.18\rekordbox.exe', 'rb') as f:
    data = f.read()

# In MSVC x64, RTTI Complete Object Locator points to TypeDescriptor.
# The TypeDescriptor has name ".?AVAVDetailedWaveformBuilder@djplay@@" or ".?AVDetailedWaveformBuilder@djplay@@"
td_target = b'.?AVDetailedWaveformBuilder@djplay@@'
td_pos = data.find(td_target)
print(f"TypeDescriptor at 0x{td_pos:X}")

# Let's search for occurrences of "ThreeBand" or "Waveform" in string literals
import re
wave_strings = []
for m in re.finditer(rb'([a-zA-Z0-9_\-\.]{3,}\.(?:wav|png|vert|frag|glsl|hlsl))', data):
    wave_strings.append(m.group(1).decode(errors='ignore'))
print("Shaders/files found:", set(wave_strings[:20]))

# Let's search for shader source code in the binary (attribute, uniform, layout, vec4, etc.)
glsl_pos = []
for kw in [b'attribute vec', b'in vec4', b'out vec4', b'uniform mat4', b'gl_Position']:
    p = 0
    while True:
        p = data.find(kw, p)
        if p == -1: break
        glsl_pos.append((kw, p))
        p += len(kw)

print(f"GLSL occurrences: {len(glsl_pos)}")
for kw, p in glsl_pos[:10]:
    s = data[max(0, p - 50):min(len(data), p + 250)]
    clean_s = re.findall(rb'[\x20-\x7e\r\n]{4,}', s)
    print(f"  [{kw.decode()}] at 0x{p:X}:", b' '.join(clean_s)[:120])
