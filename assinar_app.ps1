param (
    [string]$Name = "Abduction Developer"
)

Write-Host "Iniciando criacao de certificado auto-assinado no nome de '$Name'..." -ForegroundColor Cyan

# Remove old certificates with the same subject to avoid cluttering
Get-ChildItem Cert:\CurrentUser\My | Where-Object { $_.Subject -eq "CN=$Name" } | Remove-Item -ErrorAction SilentlyContinue

$cert = New-SelfSignedCertificate -Type CodeSigning -Subject "CN=$Name" -FriendlyName "Abduction Studio - $Name" -CertStoreLocation Cert:\CurrentUser\My

if ($cert) {
    Write-Host "Certificado criado com sucesso! Thumbprint: $($cert.Thumbprint)" -ForegroundColor Green
    
    # Exporta o certificado para arquivo .cer para o usuario instalar manualmente (evita travar em prompts invisiveis)
    $certPath = "abduction_cert.cer"
    Export-Certificate -Cert $cert -FilePath $certPath -Type CERT | Out-Null
    Write-Host "Certificado exportado para '$certPath' no diretorio raiz." -ForegroundColor Green
    
    # Assina os executaveis Release e Debug
    $exes = @("build\Release\AbductionStudioV2.exe", "build\Debug\AbductionStudioV2.exe")
    foreach ($exePath in $exes) {
        if (Test-Path $exePath) {
            Write-Host "Assinando executavel: $exePath" -ForegroundColor Cyan
            Set-AuthenticodeSignature -FilePath $exePath -Certificate $cert
            Write-Host "Executavel assinado com sucesso: $exePath" -ForegroundColor Green
        }
    }
} else {
    Write-Host "Erro ao criar certificado!" -ForegroundColor Red
}
