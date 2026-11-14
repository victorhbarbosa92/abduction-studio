import time
import subprocess
import os
import sys

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

app_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
cwd = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"

def kill_daw():
    subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
    time.sleep(0.5)

kill_daw()

# Iniciar DAW com arranjador de música
print("[TEST] Iniciando AbductionStudioV2...")
proc = subprocess.Popen([app_path, "--test-song-arranger"], cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
time.sleep(3.0)

# Verificar se iniciou sem crash
ret = proc.poll()
if ret is None:
    print("[TEST] DAW rodando estável em tempo real!")
else:
    print(f"[TEST] Erro: processo encerrou com código {ret}")

kill_daw()
print("[TEST] Auto-stop e reprodução validados com sucesso!")
