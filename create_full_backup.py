import os
import sys
import time
import shutil
import zipfile

def format_size(bytes_val):
    if bytes_val < 1024:
        return f"{bytes_val} B"
    elif bytes_val < 1024 * 1024:
        return f"{bytes_val / 1024:.2f} KB"
    elif bytes_val < 1024 * 1024 * 1024:
        return f"{bytes_val / (1024 * 1024):.2f} MB"
    else:
        return f"{bytes_val / (1024 * 1024 * 1024):.2f} GB"

def backup_abduction_studio():
    print("\n" + "="*70)
    print("[1/3] CRIANDO BACKUP DO ABDUCTION STUDIO V2...")
    print("="*70)
    
    user_home = os.path.expanduser("~")
    project_root = os.path.dirname(os.path.abspath(__file__))
    scratch_zip = os.path.abspath(os.path.join(project_root, "..", "AbductionStudioV2_FullProject.zip"))
    desktop_zip = os.path.join(user_home, "Desktop", "AbductionStudioV2_FullProject.zip")
    
    exclude_dirs = {'.git', 'build', '.vs', '.idea', '__pycache__'}
    exclude_exts = {'.obj', '.pdb', '.tlog', '.ilk', '.log', '.tmp'}
    
    t0 = time.time()
    print(f"Empacotando projeto a partir de: {project_root}")
    
    with zipfile.ZipFile(scratch_zip, 'w', zipfile.ZIP_DEFLATED, compresslevel=5) as z:
        file_count = 0
        for root, dirs, files in os.walk(project_root):
            dirs[:] = [d for d in dirs if d not in exclude_dirs]
            for f in files:
                ext = os.path.splitext(f)[1].lower()
                if ext in exclude_exts:
                    continue
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(full_path, project_root)
                z.write(full_path, rel_path)
                file_count += 1
                if file_count % 100 == 0:
                    sys.stdout.write(f"\rArquivos arquivados: {file_count}...")
                    sys.stdout.flush()
        
        exe_path = os.path.join(project_root, "build", "Release", "AbductionStudioV2.exe")
        if os.path.exists(exe_path):
            z.write(exe_path, "bin/AbductionStudioV2.exe")
            print(f"\n[OK] Binário Release adicionado em bin/AbductionStudioV2.exe ({format_size(os.path.getsize(exe_path))})")
        
    print(f"\nCopiando para a Área de Trabalho: {desktop_zip}...")
    shutil.copyfile(scratch_zip, desktop_zip)
    
    sz = os.path.getsize(desktop_zip)
    dt = time.time() - t0
    print(f"[CONCLUÍDO] AbductionStudioV2_FullProject.zip criado com sucesso em {dt:.1f}s!")
    print(f"            Tamanho final: {format_size(sz)}")
    print(f"            Destinos: {desktop_zip}")
    print(f"                      {scratch_zip}")

def backup_antigravity_config():
    print("\n" + "="*70)
    print("[2/3] CRIANDO BACKUP DAS CONFIGURAÇÕES GLOBAIS DO ANTIGRAVITY...")
    print("="*70)
    
    config_dir = r"C:\Users\USUÁRIO\.gemini\config"
    desktop_zip = r"C:\Users\USUÁRIO\Desktop\Antigravity_Global_Config_Backup.zip"
    scratch_zip = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\Antigravity_Global_Config_Backup.zip"
    
    t0 = time.time()
    with zipfile.ZipFile(scratch_zip, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
        for root, dirs, files in os.walk(config_dir):
            for f in files:
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(full_path, config_dir)
                z.write(full_path, rel_path)
                
    shutil.copyfile(scratch_zip, desktop_zip)
    sz = os.path.getsize(desktop_zip)
    dt = time.time() - t0
    print(f"[CONCLUÍDO] Antigravity_Global_Config_Backup.zip criado em {dt:.1f}s!")
    print(f"            Tamanho: {format_size(sz)}")
    print(f"            Destinos: {desktop_zip}")
    print(f"                      {scratch_zip}")

def backup_antigravity_brain():
    print("\n" + "="*70)
    print("[3/3] CRIANDO BACKUP DOS ARTEFATOS E SESSÕES DO ANTIGRAVITY (BRAIN)...")
    print("="*70)
    
    desktop_zip = r"C:\Users\USUÁRIO\Desktop\Antigravity_Brain_Artifacts_Backup.zip"
    scratch_zip = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\Antigravity_Brain_Artifacts_Backup.zip"
    
    brain_root = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain"
    target_sessions = [
        "99c43430-c66d-4980-bd72-557c18180e32",
        "63db5706-b1c5-42d0-99e0-08022437f0a2"
    ]
    
    t0 = time.time()
    with zipfile.ZipFile(scratch_zip, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
        for sess in target_sessions:
            sess_path = os.path.join(brain_root, sess)
            if not os.path.exists(sess_path):
                continue
            print(f"Empacotando sessão: {sess}...")
            for root, dirs, files in os.walk(sess_path):
                for f in files:
                    full_path = os.path.join(root, f)
                    rel_path = os.path.relpath(full_path, sess_path)
                    arcname = os.path.join(sess, rel_path)
                    z.write(full_path, arcname)
                    
                    if sess == "99c43430-c66d-4980-bd72-557c18180e32":
                        z.write(full_path, rel_path)

    shutil.copyfile(scratch_zip, desktop_zip)
    sz = os.path.getsize(desktop_zip)
    dt = time.time() - t0
    print(f"[CONCLUÍDO] Antigravity_Brain_Artifacts_Backup.zip criado em {dt:.1f}s!")
    print(f"            Tamanho: {format_size(sz)}")
    print(f"            Destinos: {desktop_zip}")
    print(f"                      {scratch_zip}")

if __name__ == "__main__":
    t_start = time.time()
    print("INICIANDO EXPORTAÇÃO COMPLETA DE BACKUPS ATUALIZADOS...")
    backup_abduction_studio()
    backup_antigravity_config()
    backup_antigravity_brain()
    total_time = time.time() - t_start
    print("\n" + "="*70)
    print(f"TODOS OS 3 BACKUPS FORAM ATUALIZADOS COM SUCESSO EM {total_time:.1f}s!")
    print("="*70)
