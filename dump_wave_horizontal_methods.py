import subprocess

dexdump = r"C:\Users\o_vic\AppData\Local\Android\Sdk\build-tools\34.0.0\dexdump.exe"
dex_file = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\wedj_pioneer\extracted\classes.dex"
proc = subprocess.Popen([dexdump, "-d", dex_file], stdout=subprocess.PIPE, text=True, errors="ignore")

recording = False
current_method = ""
method_lines = {}

for line in proc.stdout:
    if "Class descriptor" in line:
        if "WaveHorizontalView;" in line:
            recording = True
        elif recording:
            break
    if recording:
        if "name          :" in line:
            current_method = line.split(":")[-1].strip().strip("'")
            method_lines[current_method] = []
        if current_method in method_lines:
            method_lines[current_method].append(line)

proc.kill()

for m in ["i", "p", "j", "o"]:
    if m in method_lines:
        with open(f"wave_horizontal_{m}.txt", "w", encoding="utf-8") as f:
            f.writelines(method_lines[m])
        print(f"Dumped WaveHorizontalView.{m} ({len(method_lines[m])} lines)")
