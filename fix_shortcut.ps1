$WshShell = New-Object -comObject WScript.Shell
$DesktopPath = [Environment]::GetFolderPath('Desktop')
$Shortcut = $WshShell.CreateShortcut("$DesktopPath\Abduction Studio V2.lnk")
$Shortcut.TargetPath = (Resolve-Path "build\Release\AbductionStudio.exe").Path
$Shortcut.WorkingDirectory = (Resolve-Path "build\Release").Path
$Shortcut.Save()
