import wave
import os
import sys

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

sample_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\assets\samples\Sonicspore_PRYZMA"

print("[VERIFY] Testando integridade dos samples WAV extraídos do PRYZMA...")
wav_count = 0
valid_count = 0
subdirs = set()

for root, dirs, files in os.walk(sample_dir):
    for f in files:
        if f.lower().endswith(".wav"):
            wav_count += 1
            full_p = os.path.join(root, f)
            rel_dir = os.path.relpath(root, sample_dir)
            subdirs.add(rel_dir)
            try:
                with wave.open(full_p, "rb") as w:
                    ch = w.getnchannels()
                    sr = w.getframerate()
                    nframes = w.getnframes()
                    dur = nframes / float(sr)
                    valid_count += 1
                    if wav_count <= 8:
                        print(f"  [OK] {rel_dir}/{f} | Canais: {ch}, Taxa: {sr}Hz, Duracao: {dur:.2f}s")
            except Exception as e:
                print(f"  [ERRO] Falha ao abrir {f}: {e}")

print(f"\n[RESULTADO]")
print(f"Total de arquivos WAV validos e decodificaveis: {valid_count} de {wav_count}")
print(f"Pastas e categorias do pack ({len(subdirs)}):")
for s in sorted(subdirs):
    print(f"  - {s}")
