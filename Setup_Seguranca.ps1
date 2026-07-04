# Setup Seguranca - Abduction Studio by Kuru
# Execucao: powershell -ExecutionPolicy Bypass -File "C:\NovaDAW\Setup_Seguranca.ps1"

Set-StrictMode -Off
$ErrorActionPreference = "SilentlyContinue"

Write-Host ""
Write-Host "==========================================" -ForegroundColor Magenta
Write-Host "  ABDUCTION STUDIO - SETUP DE SEGURANCA" -ForegroundColor Cyan
Write-Host "  por Kuru" -ForegroundColor Red
Write-Host "==========================================" -ForegroundColor Magenta
Write-Host ""

# [1] Criar ou reutilizar certificado
Write-Host "[1] Verificando certificado de assinatura..." -ForegroundColor Yellow
$cert = Get-ChildItem "Cert:\CurrentUser\My" | Where-Object { $_.Subject -like "*NovaDAW Developer*" } | Select-Object -First 1

if (-not $cert) {
    $cert = New-SelfSignedCertificate `
        -Subject "CN=NovaDAW Developer,O=AbductionStudio,C=BR" `
        -CertStoreLocation "Cert:\CurrentUser\My" `
        -KeyUsage DigitalSignature `
        -Type CodeSigningCert `
        -NotAfter (Get-Date).AddYears(10)
    Write-Host "  OK: Certificado criado - $($cert.Thumbprint)" -ForegroundColor Green
} else {
    Write-Host "  OK: Certificado ja existe - $($cert.Thumbprint)" -ForegroundColor Green
}

# [2] Instalar em TrustedPublisher CurrentUser
Write-Host ""
Write-Host "[2] Instalando em TrustedPublisher (CurrentUser)..." -ForegroundColor Yellow
try {
    $store = New-Object System.Security.Cryptography.X509Certificates.X509Store("TrustedPublisher", "CurrentUser")
    $store.Open("ReadWrite")
    $store.Add($cert)
    $store.Close()
    Write-Host "  OK: TrustedPublisher CurrentUser" -ForegroundColor Green
} catch {
    Write-Host "  ERRO: $($_.Exception.Message)" -ForegroundColor Red
}

# [3] Tentar LocalMachine (precisa de admin)
Write-Host ""
Write-Host "[3] Tentando LocalMachine (requer Admin)..." -ForegroundColor Yellow
try {
    $mstore = New-Object System.Security.Cryptography.X509Certificates.X509Store("TrustedPublisher", "LocalMachine")
    $mstore.Open("ReadWrite")
    $mstore.Add($cert)
    $mstore.Close()
    $rstore = New-Object System.Security.Cryptography.X509Certificates.X509Store("Root", "LocalMachine")
    $rstore.Open("ReadWrite")
    $rstore.Add($cert)
    $rstore.Close()
    Write-Host "  OK: TrustedPublisher + Root LocalMachine" -ForegroundColor Green
} catch {
    Write-Host "  INFO: Sem permissao Admin. CurrentUser ja e suficiente." -ForegroundColor Cyan
}

# [4] Exclusoes no Windows Defender
Write-Host ""
Write-Host "[4] Adicionando exclusoes no Windows Defender..." -ForegroundColor Yellow
Add-MpPreference -ExclusionPath "C:\NovaDAW" -ErrorAction SilentlyContinue
Add-MpPreference -ExclusionPath "C:\NovaDAW\build\NovaDAW_artefacts\Release" -ErrorAction SilentlyContinue
Add-MpPreference -ExclusionProcess "Abduction Studio.exe" -ErrorAction SilentlyContinue
Write-Host "  OK: Exclusoes configuradas" -ForegroundColor Green

# [5] Assinar e desbloquear exe se existir
$exe = "C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe"
Write-Host ""
Write-Host "[5] Assinando o executavel..." -ForegroundColor Yellow
if (Test-Path $exe) {
    Set-AuthenticodeSignature -FilePath $exe -Certificate $cert | Out-Null
    Unblock-File -Path $exe -ErrorAction SilentlyContinue
    $zoneFile = $exe + ":Zone.Identifier"
    if (Test-Path $zoneFile) { Remove-Item -Path $zoneFile -Force -ErrorAction SilentlyContinue }
    Write-Host "  OK: Exe assinado e desbloqueado" -ForegroundColor Green
} else {
    Write-Host "  INFO: Exe ainda nao compilado. Rode Compilar_E_Liberar.bat depois." -ForegroundColor Cyan
}

# [6] Criar atalho no Desktop
Write-Host ""
Write-Host "[6] Criando atalho no Desktop..." -ForegroundColor Yellow
$ws = New-Object -ComObject WScript.Shell
$desktop = [Environment]::GetFolderPath("Desktop")
$shortcutPath = Join-Path $desktop "Abduction Studio.lnk"
$shortcut = $ws.CreateShortcut($shortcutPath)
$shortcut.TargetPath = $exe
$shortcut.WorkingDirectory = "C:\NovaDAW\build\NovaDAW_artefacts\Release"
$shortcut.IconLocation = "C:\NovaDAW\resources\icon.ico,0"
$shortcut.Description = "Abduction Studio - Kuru Edition"
$shortcut.Save()
Write-Host "  OK: Atalho criado em $shortcutPath" -ForegroundColor Green

Write-Host ""
Write-Host "==========================================" -ForegroundColor Magenta
Write-Host "  SETUP COMPLETO, KURU!" -ForegroundColor Cyan
Write-Host "  Use o atalho no Desktop para abrir." -ForegroundColor White
Write-Host "  Dois cliques = Nave decola!" -ForegroundColor Red
Write-Host "==========================================" -ForegroundColor Magenta
Write-Host ""
