with open("z2_m_dump.txt", "r", encoding="utf-8") as f:
    text = f.read()

import re

# Find methods that contain invoke-virtual on k2/a;.c
methods = text.split("Virtual methods   -")[-1].split("Direct methods    -")[0]
method_blocks = methods.split("#")

for mb in method_blocks:
    if "k2/a;.c" in mb or "3340" in mb or "3341" in mb:
        lines = mb.strip().split("\n")
        print("=== Method:", lines[0] if len(lines) > 0 else "")
        for l in lines[:30]:
            print("  ", l)
