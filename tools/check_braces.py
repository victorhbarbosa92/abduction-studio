import re

with open('src/ui/KuroDJStudioUI.h', 'r', encoding='utf-8') as f:
    text = f.read()

# Strip string literals and comments
clean = re.sub(r'"(\\.|[^"\\])*"', '""', text)
clean = re.sub(r'//.*', '', clean)
clean = re.sub(r'/\*.*?\*/', '', clean, flags=re.DOTALL)

open_c = clean.count('{')
close_c = clean.count('}')
print(f"CLEAN BRACES: open={open_c}, close={close_c}, diff={open_c - close_c}")

# Check balance line by line
lines = clean.split('\n')
bal = 0
for idx, line in enumerate(lines):
    delta = line.count('{') - line.count('}')
    bal += delta
    if bal < 0:
        print(f"Negative balance at line {idx+1}: {line}")
        break
print(f"Final balance: {bal}")

