import re

with open("src/ui/KuroDJStudioUI.h", "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()

push_count = 0
pop_count = 0

for i, line in enumerate(lines):
    # find PushStyleColor
    pushes = re.findall(r'ImGui::PushStyleColor\(', line)
    # find PopStyleColor(N) or PopStyleColor()
    pops = re.findall(r'ImGui::PopStyleColor\((\d*)\)', line)
    
    p_cnt = len(pushes)
    po_cnt = 0
    for p in pops:
        if p == '':
            po_cnt += 1
        else:
            po_cnt += int(p)
            
    if p_cnt > 0 or po_cnt > 0:
        # print if weird
        pass

print("Done scanning lines.")
