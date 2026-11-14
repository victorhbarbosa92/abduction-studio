import struct

with open(r'C:\Program Files\rekordbox\rekordbox 7.2.18\rekordbox.exe', 'rb') as f:
    data = f.read()

# Let's search for "ThreeBandDetailedWaveform" or "DetailedWaveform"
target = b'DetailedWaveform'
pos = 0
matches = []
while True:
    pos = data.find(target, pos)
    if pos == -1: break
    matches.append(pos)
    pos += len(target)

print(f"Found {len(matches)} matches for '{target.decode()}'")
for m in matches:
    chunk = data[max(0, m - 40):min(len(data), m + 100)]
    print(f"0x{m:X}: {chunk}")
