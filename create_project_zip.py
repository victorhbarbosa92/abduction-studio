import os
import zipfile
import shutil

project_root = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"
desktop_zip = r"C:\Users\USUÁRIO\Desktop\AbductionStudioV2_FullProject.zip"
scratch_zip = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\AbductionStudioV2_FullProject.zip"

print(f"[PACKAGE] Empacotando {project_root}...")

# Lista de exclusão de pastas
exclude_dirs = {'.git', 'build', '.vs', '.idea'}
exclude_exts = {'.obj', '.pdb', '.tlog', '.ilk', '.log', '.tmp'}

zip_targets = [desktop_zip, scratch_zip]

for target in zip_targets:
    print(f"[PACKAGE] Criando arquivo zip: {target}")
    with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
        for root, dirs, files in os.walk(project_root):
            # Filtra diretórios
            dirs[:] = [d for d in dirs if d not in exclude_dirs]
            for f in files:
                ext = os.path.splitext(f)[1].lower()
                if ext in exclude_exts:
                    continue
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(full_path, project_root)
                z.write(full_path, rel_path)

        # Inclui o executável binário compilado Release em bin/AbductionStudioV2.exe
        exe_path = os.path.join(project_root, "build", "Release", "AbductionStudioV2.exe")
        if os.path.exists(exe_path):
            z.write(exe_path, "bin/AbductionStudioV2.exe")
            print("[PACKAGE] Binário Release adicionado em bin/AbductionStudioV2.exe")

    sz_mb = os.path.getsize(target) / (1024 * 1024)
    print(f"[PACKAGE] Zip concluído com sucesso: {target} ({sz_mb:.2f} MB)")
