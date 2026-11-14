import os
import sys
import zipfile
import shutil
import time

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = script_dir
    user_home = os.path.expanduser("~")
    
    desktop_dir = os.path.join(user_home, "Desktop")
    scratch_dir = os.path.abspath(os.path.join(project_root, ".."))
    
    desktop_zip = os.path.join(desktop_dir, "AbductionStudioV2_FullProject.zip")
    scratch_zip = os.path.join(scratch_dir, "AbductionStudioV2_FullProject.zip")
    
    print("=" * 75)
    print("    EMPACOTAMENTO COMPLETO E UNIVERSAL - ABDUCTION STUDIO V2")
    print("=" * 75)
    print(f"Origem do Projeto: {project_root}")
    print(f"Destino Scratch:   {scratch_zip}")
    print(f"Destino Desktop:   {desktop_zip}")
    print("-" * 75)
    
    exclude_dirs = {'.git', 'build', '.vs', '.idea', '__pycache__', 'test_unzip'}
    exclude_exts = {'.obj', '.pdb', '.tlog', '.ilk', '.log', '.tmp'}
    
    t0 = time.time()
    
    # Cria o zip no scratch primeiro
    with zipfile.ZipFile(scratch_zip, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
        file_count = 0
        for root, dirs, files in os.walk(project_root):
            dirs[:] = [d for d in dirs if d not in exclude_dirs]
            for f in files:
                ext = os.path.splitext(f)[1].lower()
                if ext in exclude_exts:
                    continue
                # Evita incluir o próprio zip dentro do zip caso esteja na pasta
                if f.lower() == "abductionstudiov2_fullproject.zip":
                    continue
                    
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(full_path, project_root)
                z.write(full_path, rel_path)
                file_count += 1
                if file_count % 150 == 0:
                    sys.stdout.write(f"\rArquivando: {file_count} arquivos...")
                    sys.stdout.flush()

    sys.stdout.write(f"\rArquivamento base concluído: {file_count} arquivos compactados.\n")
    sys.stdout.flush()

    # Copia para o Desktop se a pasta existir
    if os.path.exists(desktop_dir):
        print(f"Copiando pacote para a Área de Trabalho: {desktop_zip}...")
        shutil.copyfile(scratch_zip, desktop_zip)

    sz_mb = os.path.getsize(scratch_zip) / (1024 * 1024)
    dt = time.time() - t0
    print("-" * 75)
    print(f"[CONCLUÍDO COM SUCESSO] Pacote gerado em {dt:.1f}s ({sz_mb:.2f} MB)")
    print(f"Arquivo gerado em: {scratch_zip}")
    if os.path.exists(desktop_dir):
        print(f"                   {desktop_zip}")
    print("=" * 75)

if __name__ == "__main__":
    main()
