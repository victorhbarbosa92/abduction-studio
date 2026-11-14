import subprocess

dexdump = r"C:\Users\o_vic\AppData\Local\Android\Sdk\build-tools\34.0.0\dexdump.exe"
dex_file = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\wedj_pioneer\extracted\classes.dex"
proc = subprocess.Popen([dexdump, "-d", dex_file], stdout=subprocess.PIPE, text=True, errors="ignore")

recording = False
lines = []

for line in proc.stdout:
    if "Class descriptor" in line:
        if "Lz2/l;" in line:
            recording = True
        elif recording:
            break
    if recording:
        lines.append(line)

proc.kill()

with open("z2_l_dump.txt", "w", encoding="utf-8") as f:
    f.writelines(lines)

print(f"Dumped {len(lines)} lines of z2/l to z2_l_dump.txt")
