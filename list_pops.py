with open("src/ui/KuroDJStudioUI.h", "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    if "PopStyleColor" in line:
        # Look backwards up to 50 lines to find PushStyleColor in the same scope/block
        # Print the line and surrounding context
        print(f"Line {i+1}: {line.strip()}")
