import os

# 1. Update DJCommonUtils.h
comm_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\core\DJCommonUtils.h"
with open(comm_path, "r", encoding="utf-8") as f:
    comm_c = f.read()

target_comm = "    inline std::string sanitizeFilename(const std::string& name) {"
replace_comm = """    inline std::string escapeJson(const std::string& input) {
        std::string res;
        for (char c : input) {
            if (c == '\\\\') res += "\\\\";
            else if (c == '\"') res += "\\\"";
            else if (c == '\\n') res += "\\n";
            else if (c == '\\r') res += "\\r";
            else res += c;
        }
        return res;
    }

    inline std::string sanitizeFilename(const std::string& name) {"""

if target_comm in comm_c:
    comm_c = comm_c.replace(target_comm, replace_comm, 1)
    with open(comm_path, "w", encoding="utf-8") as f:
        f.write(comm_c)
    print("Added escapeJson to DJCommonUtils.h")
else:
    print("target_comm not found in DJCommonUtils.h")

# 2. Update SpotifyDJService.h to use escapeJson instead of DJExportService::escapeJson
spot_path = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\core\SpotifyDJService.h"
with open(spot_path, "r", encoding="utf-8") as f:
    spot_c = f.read()

spot_c = spot_c.replace("DJExportService::escapeJson", "escapeJson")
with open(spot_path, "w", encoding="utf-8") as f:
    f.write(spot_c)
print("Updated SpotifyDJService.h to use escapeJson")
