with open(r'C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h', 'r', encoding='utf-8') as f:
    lines = f.readlines()
for i, l in enumerate(lines):
    if 'rekordbox' in l.lower() and ('addtext' in l.lower() or 'text(' in l.lower() or 'button(' in l.lower() or 'drawfitted' in l.lower()):
        print(f'{i+1}: {l.strip()}')

