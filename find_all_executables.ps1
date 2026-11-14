$desktop = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::Desktop)
$scratch = "C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch"

Write-Host "=== 1. VERIFICANDO ARQUIVOS NA AREA DE TRABALHO ==="
Get-ChildItem -Path $desktop -Filter "*abduction*" -Recurse -ErrorAction SilentlyContinue | ForEach-Object {
    Write-Host "Desktop item: " $_.FullName
}

Write-Host "=== 2. LOCALIZANDO TODOS OS EXECUTAVEIS NO DISCO ==="
Get-ChildItem -Path $scratch -Filter "*.exe" -Recurse -ErrorAction SilentlyContinue | ForEach-Object {
    Write-Host "Executable found: " $_.FullName
}
