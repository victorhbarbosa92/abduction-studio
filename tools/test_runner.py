# -*- coding: utf-8 -*-
import sys, os

src_path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h'

with open(src_path, 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

print('Source length:', len(content))
