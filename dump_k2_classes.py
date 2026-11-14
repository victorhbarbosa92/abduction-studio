import subprocess

dexdump = r"C:\Users\o_vic\AppData\Local\Android\Sdk\build-tools\34.0.0\dexdump.exe"
dex_file = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\wedj_pioneer\extracted\classes.dex"

proc = subprocess.Popen([dexdump, "-d", dex_file], stdout=subprocess.PIPE, text=True, errors="ignore")

targets = ["Lk2/a;", "Lk2/b;", "Lk2/c;", "Lk2/d;", "Lk2/f;", "Lw2/g;"]
recording = False
current_class = ""
dumps = {t: [] for t in targets}

for line in proc.stdout:
    if "Class descriptor" in line:
        recording = False
        for t in targets:
            if t in line:
                current_class = t
                recording = True
                break
    if recording:
        dumps[current_class].append(line)

proc.kill()

for t, lines in dumps.items():
    clean_name = t.replace("L", "").replace(";", "").replace("/", "_")
    with open(f"{clean_name}_dump.txt", "w", encoding="utf-8") as f:
        f.writelines(lines)
    print(f"Dumped {len(lines)} lines to {clean_name}_dump.txt")
