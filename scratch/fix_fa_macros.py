import glob
import re
import os

files = glob.glob(r'src/**/*.h', recursive=True) + glob.glob(r'src/**/*.cpp', recursive=True)

# Pattern matching " ICON_FA_SOMETHING " or "ICON_FA_SOMETHING " inside string literals
pattern = re.compile(r'"(\s*)ICON_FA_([A_Z0-9_]+)(\s*)"')
pattern_embedded = re.compile(r'"([^"]*?)ICON_FA_([A_Z0-9_]+)([^"]*?)"')

for filepath in files:
    if 'IconsFontAwesome6.h' in filepath:
        continue
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # Function to replace embedded ICON_FA_ inside string literals with proper macro concatenation
    def replace_match(m):
        prefix = m.group(1)
        macro_name = 'ICON_FA_' + m.group(2)
        suffix = m.group(3)
        
        parts = []
        if prefix:
            parts.append(f'"{prefix}"')
        parts.append(macro_name)
        if suffix:
            parts.append(f'"{suffix}"')
        
        return ' '.join(parts)

    new_content = pattern_embedded.sub(replace_match, content)

    if new_content != content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f'Fixed FontAwesome macro concatenation in {os.path.basename(filepath)}')
