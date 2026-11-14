import re
import sys

# Set stdout to utf-8
sys.stdout.reconfigure(encoding='utf-8')

with open("src/ui/KuroDJStudioUI.h", "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    # Check conditional pushes
    if re.search(r'if\s*\(.*PushStyleColor', line):
        print(f"Conditional push on one line: L{i+1}: {line.strip()}")
    # Check conditional pops
    if re.search(r'if\s*\(.*PopStyleColor', line):
        print(f"Conditional pop on one line: L{i+1}: {line.strip()}")

for i in range(len(lines)-5):
    l0 = lines[i].strip()
    l1 = lines[i+1].strip()
    # Check if if statement has NO body on the same line and no curly brace
    m = re.match(r'^if\s*\([^)]*\)\s*$', l0)
    if m:
        if "PushStyleColor" in l1:
            print(f"Conditional push multiline: L{i+1}-L{i+2}: {l0} => {l1}")
        if "PopStyleColor" in l1:
            print(f"Conditional pop multiline: L{i+1}-L{i+2}: {l0} => {l1}")
