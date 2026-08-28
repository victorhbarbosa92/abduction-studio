$WshShell = New-Object -comObject WScript.Shell
$Desktop = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::Desktop)
$Shortcut = $WshShell.CreateShortcut("$Desktop\novo abduction studio.lnk")
$Shortcut.TargetPath = "C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Debug\AbductionStudioV2.exe"
$Shortcut.WorkingDirectory = "C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Debug"
$Shortcut.Description = "Novo Abduction Studio"
$Shortcut.Save()
Write-Host "Atalho criado com sucesso na Area de Trabalho: $Desktop\novo abduction studio.lnk"
