$exe = (Get-Item "build\Debug\AbductionStudioV2.exe").FullName
$workDir = (Get-Item "build\Debug").FullName
$desktop = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::Desktop)

Write-Host "Target EXE: $exe"
Write-Host "Work Dir: $workDir"
Write-Host "Desktop: $desktop"

$WshShell = New-Object -ComObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("$desktop\novo abduction studio.lnk")
$Shortcut.TargetPath = $exe
$Shortcut.WorkingDirectory = $workDir
$Shortcut.Description = "Novo Abduction Studio"
$Shortcut.Save()

Write-Host "Atalho recriado com sucesso com caminho absoluto real!"
