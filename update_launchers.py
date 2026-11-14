import os
import shutil
import subprocess

def update_all():
    # 1. Matar qualquer processo antigo do AbductionStudioV2
    subprocess.run(["taskkill", "/F", "/IM", "AbductionStudioV2.exe"], capture_output=True)
    
    base_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2"
    release_exe = os.path.join(base_dir, "build", "Release", "AbductionStudioV2.exe")
    debug_exe = os.path.join(base_dir, "build", "Debug", "AbductionStudioV2.exe")
    
    # 2. Copiar Release para Debug para garantir que ambos estejam idênticos
    if os.path.exists(release_exe):
        os.makedirs(os.path.dirname(debug_exe), exist_ok=True)
        shutil.copy2(release_exe, debug_exe)
        print("[UPDATE] Release executável copiado com sucesso para a pasta Debug!")
        
    # 3. Garantir cópia de assets (samples e soundbanks) em ambas as pastas
    assets_src = os.path.join(base_dir, "assets")
    for build_type in ["Release", "Debug"]:
        assets_dst = os.path.join(base_dir, "build", build_type, "assets")
        if os.path.exists(assets_src):
            shutil.copytree(assets_src, assets_dst, dirs_exist_ok=True)
            print(f"[UPDATE] Assets sincronizados com sucesso em build/{build_type}/assets!")

    # 4. Atualizar o atalho da Área de Trabalho 'novo abduction studio.lnk'
    desktop_lnk = r"C:\Users\USUÁRIO\Desktop\novo abduction studio.lnk"
    ps_cmd = f"""
    $sh = New-Object -ComObject WScript.Shell
    $sc = $sh.CreateShortcut('{desktop_lnk}')
    $sc.TargetPath = '{release_exe}'
    $sc.WorkingDirectory = '{os.path.dirname(release_exe)}'
    $sc.Description = 'Abduction Studio V2 (Release)'
    $sc.Save()
    """
    subprocess.run(["powershell", "-NoProfile", "-Command", ps_cmd], capture_output=True)
    print(f"[UPDATE] Atalho da Área de Trabalho atualizado com sucesso para: {release_exe}")

if __name__ == "__main__":
    update_all()
