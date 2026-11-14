# -*- coding: utf-8 -*-
import sys, os

src_path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h'

with open(src_path, 'r', encoding='utf-8', errors='ignore') as f:
    code = f.read()

m1_str = 'void renderSearchModal(KuroAudio::DJEngine& engine) {'
m1 = code.find(m1_str)
if m1 == -1:
    print('Error: renderSearchModal not found!')
    sys.exit(1)

m2_str = 'void renderCamelotRadarModal(KuroAudio::DJEngine& engine) {'
m2 = code.find(m2_str)
if m2 == -1:
    print('Error: renderCamelotRadarModal not found!')
    sys.exit(1)

t1_str = 'else if (cloud_hub_active_tab == 1) {'
t1 = code.find(t1_str, m1)

dup_str = 'if (false && cloud_hub_active_tab <= 1) {'
dup = code.find(dup_str, m1)

print(f'm1: {m1}, t1: {t1}, dup: {dup}, m2: {m2}')
