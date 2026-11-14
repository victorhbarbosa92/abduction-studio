import re

with open(r'c:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h', 'r', encoding='utf-8') as f:
    for idx, line in enumerate(f, 1):
        if re.search(r'TEMPO|\u00b1|PHASE|DELETE|DROP', line):
            if any(k in line for k in ['TEMPO', 'PHASE', 'DELETE', 'DROP', '%']):
                if 'pitch_percent' in line or 'TEMPO' in line or 'PHASE' in line:
                    print(f'{idx}: {line.strip()[:120]}')
