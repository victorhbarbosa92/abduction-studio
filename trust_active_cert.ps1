$thumb = "72712207F80D795AC28A543DEF1D757046F7665D"
$cert = Get-Item "Cert:\CurrentUser\My\$thumb" -ErrorAction SilentlyContinue
if (-not $cert) {
    $cert = (Get-AuthenticodeSignature .\AbductionStudioV2.exe).SignerCertificate
}
if ($cert) {
    Export-Certificate -Cert $cert -FilePath ".\abduction_active.cer" -Force | Out-Null
    Import-Certificate -FilePath ".\abduction_active.cer" -CertStoreLocation Cert:\CurrentUser\Root | Out-Null
    Import-Certificate -FilePath ".\abduction_active.cer" -CertStoreLocation Cert:\CurrentUser\TrustedPublisher | Out-Null
    Write-Host "Certificado instalado em Trusted Root e Trusted Publisher com sucesso!" -ForegroundColor Green
    Get-AuthenticodeSignature .\AbductionStudioV2.exe
} else {
    Write-Host "Certificado nao encontrado!" -ForegroundColor Red
}
