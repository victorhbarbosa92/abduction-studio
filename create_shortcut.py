import os
import sys

try:
    import win32com.client
    shell = win32com.client.Dispatch("WScript.Shell")
    desktop = shell.SpecialFolders("Desktop")
    shortcut_path = os.path.join(desktop, "novo abduction studio.lnk")
    target_exe = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Debug\AbductionStudioV2.exe"
    work_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Debug"
    
    shortcut = shell.CreateShortCut(shortcut_path)
    shortcut.Targetpath = target_exe
    shortcut.WorkingDirectory = work_dir
    shortcut.Description = "Novo Abduction Studio"
    shortcut.save()
    print("Atalho criado com sucesso via win32com:", shortcut_path)
except Exception as e:
    print("Fallback via powershell script file:", e)
    ps_content = f'''$WshShell = New-Object -comObject WScript.Shell
$Desktop = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::Desktop)
$Shortcut = $WshShell.CreateShortcut("$Desktop\\novo abduction studio.lnk")
$Shortcut.TargetPath = "{target_exe}"
$Shortcut.WorkingDirectory = "{work_dir}"
$Shortcut.Description = "Novo Abduction Studio"
$Shortcut.Save()
Write-Host "Atalho criado com sucesso na Area de Trabalho!"
'''
    with open("create_shortcut.ps1", "w", encoding="utf-8") as f:
        f.write(ps_content)
    os.system("powershell -ExecutionPolicy Bypass -File create_shortcut.ps1")
