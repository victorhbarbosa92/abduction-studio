# Criar certificado de desenvolvedor com identidade Kuru
Set-StrictMode -Off
$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "==========================================" -ForegroundColor Magenta
Write-Host "  ABDUCTION STUDIO - ASSINAR COMO KURU" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Magenta
Write-Host ""

# Remover certificado antigo se existir
$old = Get-ChildItem "Cert:\CurrentUser\My" | Where-Object { $_.Subject -like "*NovaDAW Developer*" -or $_.Subject -like "*Kuru*" }
if ($old) {
    $old | Remove-Item -Force -ErrorAction SilentlyContinue
    Write-Host "[*] Certificado anterior removido." -ForegroundColor Gray
}

# Criar novo certificado com identidade Kuru
Write-Host "[1] Criando certificado de desenvolvedor..." -ForegroundColor Yellow
$cert = New-SelfSignedCertificate `
    -Subject "CN=Kuru, O=Abduction Studio, OU=Desenvolvedor Principal, L=Brasil, C=BR" `
    -CertStoreLocation "Cert:\CurrentUser\My" `
    -KeyUsage DigitalSignature `
    -Type CodeSigningCert `
    -NotAfter (Get-Date).AddYears(10) `
    -FriendlyName "Kuru - Abduction Studio Developer"

Write-Host "  OK!" -ForegroundColor Green
Write-Host ""
Write-Host "  Desenvolvedor : Kuru" -ForegroundColor Cyan
Write-Host "  Organizacao   : Abduction Studio" -ForegroundColor Cyan
Write-Host "  Pais          : Brasil" -ForegroundColor Cyan
$inicio = $cert.NotBefore.ToString("dd/MM/yyyy")
$fim = $cert.NotAfter.ToString("dd/MM/yyyy")
Write-Host "  Validade      : $inicio ate $fim" -ForegroundColor Cyan
$thumb = $cert.Thumbprint
Write-Host "  Thumbprint    : $thumb" -ForegroundColor Gray

# Instalar em TrustedPublisher CurrentUser
Write-Host ""
Write-Host "[2] Instalando em TrustedPublisher..." -ForegroundColor Yellow
$store = New-Object System.Security.Cryptography.X509Certificates.X509Store("TrustedPublisher", "CurrentUser")
$store.Open("ReadWrite")
$store.Add($cert)
$store.Close()
Write-Host "  OK: TrustedPublisher (CurrentUser)" -ForegroundColor Green

# Tentar LocalMachine (precisa admin)
try {
    $ml = New-Object System.Security.Cryptography.X509Certificates.X509Store("TrustedPublisher", "LocalMachine")
    $ml.Open("ReadWrite")
    $ml.Add($cert)
    $ml.Close()
    $rl = New-Object System.Security.Cryptography.X509Certificates.X509Store("Root", "LocalMachine")
    $rl.Open("ReadWrite")
    $rl.Add($cert)
    $rl.Close()
    Write-Host "  OK: TrustedPublisher + Root (LocalMachine)" -ForegroundColor Green
} catch {
    Write-Host "  INFO: LocalMachine requer Admin. Use Setup_Seguranca.bat para isso." -ForegroundColor Cyan
}

# Assinar o exe com o novo certificado
$exe = "C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe"
Write-Host ""
Write-Host "[3] Assinando Abduction Studio.exe como Kuru..." -ForegroundColor Yellow
if (Test-Path $exe) {
    $result = Set-AuthenticodeSignature -FilePath $exe -Certificate $cert
    Unblock-File -Path $exe -ErrorAction SilentlyContinue
    Remove-Item -Path ($exe + ":Zone.Identifier") -Force -ErrorAction SilentlyContinue
    $signerName = $result.SignerCertificate.Subject
    Write-Host "  OK: Assinado!" -ForegroundColor Green
    Write-Host "  Assinado por: $signerName" -ForegroundColor Cyan
} else {
    Write-Host "  INFO: Exe nao encontrado. Compile primeiro." -ForegroundColor Gray
}

# Atualizar Setup_Seguranca.bat para usar "Kuru" no filtro
Write-Host ""
Write-Host "[4] Atualizando scripts para referenciar certificado Kuru..." -ForegroundColor Yellow
$batContent = Get-Content "C:\NovaDAW\Setup_Seguranca.bat" -Raw
$batContent = $batContent -replace "NovaDAW Developer", "Kuru"
Set-Content "C:\NovaDAW\Setup_Seguranca.bat" $batContent -Encoding UTF8
Write-Host "  OK: Scripts atualizados" -ForegroundColor Green

Write-Host ""
Write-Host "==========================================" -ForegroundColor Magenta
Write-Host "  Kuru agora eh o desenvolvedor oficial" -ForegroundColor Cyan
Write-Host "  do Abduction Studio. Assinado e lacrado." -ForegroundColor White
Write-Host "==========================================" -ForegroundColor Magenta
Write-Host ""
