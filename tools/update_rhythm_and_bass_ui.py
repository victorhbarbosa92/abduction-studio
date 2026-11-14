import os
import sys

ui_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroPsytranceRollingBassUI.h"

with open(ui_path, "r", encoding="utf-8") as f:
    orig = f.read()

# Let's inspect sections to replace
print("Original size:", len(orig))
