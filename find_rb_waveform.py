import re

with open(r'C:\Program Files\rekordbox\rekordbox 7.2.18\rekordbox.exe', 'rb') as f:
    data = f.read()

targets = [b'AVDetailedWaveformBuilder', b'AVThreeBandDetailedWaveformBuilder', b'AVViewZoomWaveform', b'WaveformColor', b'WaveformRenderer']

for target in targets:
    pos = 0
    print('=== Target:', target.decode(), '===')
    found = 0
    while found < 5:
        pos = data.find(target, pos)
        if pos == -1: break
        start = max(0, pos - 200)
        end = min(len(data), pos + 300)
        chunk = data[start:end]
        strings = re.findall(rb'[\x20-\x7e]{4,}', chunk)
        print(f'Offset 0x{pos:X}:', [s.decode(errors='ignore') for s in strings])
        pos += len(target)
        found += 1
