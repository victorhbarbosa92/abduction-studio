import subprocess
import os

p1 = "C:/Users/o_vic/Desktop/FIRST KURU/Oksha - Tlalaxco (Original Mix).wav"
p2 = os.path.normpath(p1)

print("Exists:", os.path.exists(p2))
print("Normalized:", p2)

