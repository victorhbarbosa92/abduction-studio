import zipfile
import os
import sys
import shutil

zip_path = r"C:\Users\USUÁRIO\Downloads\Sonicspore - PRYZMA - FREE Psytrance Sample Pack.zip"
target_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\assets\samples\Sonicspore_PRYZMA"
os.makedirs(target_dir, exist_ok=True)

print(f"Lendo zip: {zip_path}")
with zipfile.ZipFile(zip_path, 'r') as z:
    for member in z.infolist():
        if member.is_dir():
            continue
        
        # Ignorar arquivos que não são WAV ou PDF informativos se houver
        filename = os.path.basename(member.filename)
        if not filename.lower().endswith(('.wav', '.aif', '.flac', '.mp3')):
            continue
            
        parts = member.filename.replace('\\', '/').split('/')
        # parts pode ser: ['Sonicspore - PRYZMA - FREE Psytrance Sample Pack', '01 Kick', 'Sonicspore _ FPS _ Kick 01 - 144 BPM.wav']
        if len(parts) >= 3:
            category = parts[1].strip()
            # Limpar nome da categoria (ex: "01 Kick" -> "01_Kicks")
            clean_cat = category.replace(' ', '_')
            dest_folder = os.path.join(target_dir, clean_cat)
            os.makedirs(dest_folder, exist_ok=True)
            dest_file = os.path.join(dest_folder, filename)
        elif len(parts) == 2:
            dest_file = os.path.join(target_dir, filename)
        else:
            continue
            
        with z.open(member) as source, open(dest_file, "wb") as target:
            shutil.copyfileobj(source, target)

print("Extracao concluida com sucesso para assets/samples/Sonicspore_PRYZMA!")

# Contar arquivos extraídos
extracted = []
for root, dirs, files in os.walk(target_dir):
    for f in files:
        if f.lower().endswith(('.wav', '.aif', '.flac', '.mp3')):
            extracted.append(os.path.join(root, f))
print(f"Total de samples extraidos: {len(extracted)}")
