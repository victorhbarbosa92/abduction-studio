# -*- coding: utf-8 -*-
import sys, os

src_path = r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h'

with open(src_path, 'r', encoding='utf-8', errors='ignore') as f:
    text = f.read()

m1 = text.find('void renderSearchModal(KuroAudio::DJEngine')
t1 = text.find('else if (cloud_hub_active_tab == 1)', m1)
dup = text.find('if (false && cloud_hub_active_tab <= 1)', m1)
m2 = text.find('void renderCamelotRadarModal(KuroAudio::DJEngine', m1)

if m1 == -1 or t1 == -1 or dup == -1 or m2 == -1:
    print('Failed to find markers!', m1, t1, dup, m2)
    sys.exit(1)

# Find end of tab 4 right before dup
end_of_tab4 = text.rfind('}', 0, dup)
# Find the line ending
end_of_tab4 = text.find('\n', end_of_tab4) + 1

tabs_1_to_4 = text[t1:end_of_tab4]
print('Captured tabs 1 to 4, length:', len(tabs_1_to_4))
