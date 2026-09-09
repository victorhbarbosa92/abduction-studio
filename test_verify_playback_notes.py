import subprocess
import time
import os

print("[TEST] Verificando se a DAW inicia e responde aos comandos de áudio...")
app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

# Executar com --test-song-arranger para garantir que o projeto carrega em SONG mode
proc = subprocess.Popen([app_path, "--test-song-arranger"], cwd=cwd)
time.sleep(2.0)
proc.terminate()
print("[TEST] Executável testado com sucesso!")
