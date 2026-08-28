$WshShell = New-Object -ComObject WScript.Shell
$Desktop = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::Desktop)
$Shortcut = $WshShell.CreateShortcut("$Desktop\novo abduction studio.lnk")
Write-Host "LNK TargetPath: " $Shortcut.TargetPath
Write-Host "Target Exists: " (Test-Path $Shortcut.TargetPath)
