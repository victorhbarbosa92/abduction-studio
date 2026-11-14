import os
import subprocess

p_show = "C:/Users/o_vic/Desktop/FIRST KURU/Oksha - Tlalaxco (Original Mix).wav"
p_win = os.path.normpath(p_show)

print("Exists:", os.path.exists(p_win))
print("Slash command:", f'explorer.exe /select,"{p_show}"')
print("Backslash command:", f'explorer.exe /select,"{p_win}"')

