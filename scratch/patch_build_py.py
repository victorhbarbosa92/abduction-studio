with open("scratch/build_perfect_playlist.py", "r", encoding="utf-8") as f:
    py = f.read()

target = 'with open("scratch/generate_playlist_ui.py", "r", encoding="utf-8") as f:\n    orig = f.read()'
replacement = '''with open("scratch/generate_playlist_ui.py", "r", encoding="utf-8") as f:
    text = f.read()

s = text.find("r'''") + 4
e = text.rfind("'''")
orig = text[s:e].strip()'''

assert target in py, "target not found"
py = py.replace(target, replacement)

with open("scratch/build_perfect_playlist.py", "w", encoding="utf-8") as f:
    f.write(py)

print("Updated build_perfect_playlist.py to extract pure C++ code!")
