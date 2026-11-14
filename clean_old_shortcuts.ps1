$desktop = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::Desktop)

Write-Host "=== 1. REMOVENDO ATALHOS E SCRIPTS ANTIGOS DA ÁREA DE TRABALHO ==="
$old_items = @(
    "$desktop\Abduction studio v2.lnk",
    "$desktop\Abduction Studio.lnk",
    "$desktop\Iniciar_Abduction_Studio.bat"
)

foreach ($item in $old_items) {
    if (Test-Path $item) {
        Remove-Item -Force -Path $item
        Write-Host "[REMOVIDO COM SUCESSO] $item"
    }
}

Write-Host "=== 2. MANTENDO O NOVO ATALHO ÚNICO E ATUALIZADO ==="
$current_exe = (Get-Item "build\Debug\AbductionStudioV2.exe").FullName
$current_dir = (Get-Item "build\Debug").FullName

$WshShell = New-Object -ComObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("$desktop\novo abduction studio.lnk")
$Shortcut.TargetPath = $current_exe
$Shortcut.WorkingDirectory = $current_dir
$Shortcut.Description = "Abduction Studio V2 (Versao Limpa Oficial)"
$Shortcut.Save()

Write-Host "[ATALHO ÚNICO OFICIAL PRONTO] $desktop\novo abduction studio.lnk -> $current_exe"
