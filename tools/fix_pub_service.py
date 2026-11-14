path = "C:/Users/o_vic/.gemini/antigravity-ide/scratch/abduction_studio_v2/src/core/SpotifyDJService.h"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

content = content.replace("        bool show_dig_deeper_view = false;\n", "")
content = content.replace("    public:\n", "    public:\n        bool show_dig_deeper_view = false;\n")

with open(path, "w", encoding="utf-8") as f:
    f.write(content)

print("Moved show_dig_deeper_view to public!")

