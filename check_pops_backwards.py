import re

with open("src/ui/KuroDJStudioUI.h", "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()

# Let's check blocks:
# A common bug:
# if (is_playing) ImGui::PushStyleColor(...);
# ...
# ImGui::PopStyleColor(); // Unconditional!

for i, line in enumerate(lines):
    if "PopStyleColor" in line:
        # scan backwards to find matching push
        pops = 1
        m = re.search(r'PopStyleColor\((\d+)\)', line)
        if m:
            pops = int(m.group(1))
        
        pushes_found = 0
        search_lines = []
        for j in range(i - 1, max(-1, i - 120), -1):
            prev = lines[j]
            # count pushes
            p_cnt = len(re.findall(r'PushStyleColor\(', prev))
            if p_cnt > 0:
                pushes_found += p_cnt
                search_lines.append((j+1, prev.strip()))
            if pushes_found >= pops:
                break
        
        # Check if any of those pushes are inside an if statement that doesn't enclose the pop
        # Or if pushes_found < pops
        if pushes_found < pops:
            print(f"[ERROR] Line {i+1}: PopStyleColor({pops}) but only found {pushes_found} pushes backwards!")
            print(f"   Pop line: {line.strip()}")
            for sl in search_lines:
                print(f"      L{sl[0]}: {sl[1]}")
