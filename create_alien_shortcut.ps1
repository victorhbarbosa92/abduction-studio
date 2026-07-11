$desktop = [Environment]::GetFolderPath("Desktop")
$WshShell = New-Object -comObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("$desktop\AbductionStudioV2_Alien.lnk")
$target = Join-Path $env:USERPROFILE ".gemini\antigravity-ide\scratch\abduction_studio_v2\build\Debug\AbductionStudioV2.exe"
$workdir = Join-Path $env:USERPROFILE ".gemini\antigravity-ide\scratch\abduction_studio_v2\build\Debug"
$Shortcut.TargetPath = $target
$Shortcut.WorkingDirectory = $workdir
$Shortcut.Save()
Write-Host "Atalho atualizado."
