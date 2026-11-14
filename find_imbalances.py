import re

with open("src/ui/KuroDJStudioUI.h", "r", encoding="utf-8", errors="ignore") as f:
    code = f.read()

# Let's find all functions in KuroDJStudioUI.h
# Or check every function line by line

lines = code.splitlines()

current_func = "GLOBAL"
stack = []
warnings = []

for idx, line in enumerate(lines, 1):
    m_func = re.match(r'^\s*(?:static\s+|inline\s+|virtual\s+)*(?:void|bool|int|float|double|ImVec2|std::string)\s+([A-Za-z0-9_]+)\s*\(', line)
    if m_func:
        # Check previous function stack
        if stack:
            warnings.append(f"Function {current_func} ended with unpopped styles: {len(stack)} at line {idx}")
            stack = []
        current_func = f"{m_func.group(1)} (L{idx})"
        
    # Check pushes
    pushes = re.findall(r'ImGui::PushStyleColor\(', line)
    for _ in pushes:
        stack.append(idx)
        
    # Check pops
    pops = re.findall(r'ImGui::PopStyleColor\((\d*)\)', line)
    for p in pops:
        count = int(p) if p else 1
        for _ in range(count):
            if not stack:
                warnings.append(f"OVERPOP in {current_func} at line {idx} (PopStyleColor called with empty stack!)")
            else:
                stack.pop()

    # Check returns when stack is not empty
    if re.search(r'\breturn\b', line):
        # ignore comments
        clean_line = re.sub(r'//.*', '', line)
        if re.search(r'\breturn\b', clean_line) and stack:
            warnings.append(f"EARLY RETURN in {current_func} at line {idx} with {len(stack)} styles still pushed! Pushed at lines: {stack}")

for w in warnings:
    print(w)

print(f"Finished scan. Total warnings: {len(warnings)}")
