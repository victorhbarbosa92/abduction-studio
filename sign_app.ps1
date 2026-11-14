$cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject "CN=AbductionStudioDev" -CertStoreLocation "Cert:\CurrentUser\My"

# Copy certificate to Trusted Root Certification Authorities to trust it on Windows
$rootStore = New-Object System.Security.Cryptography.X509Certificates.X509Store("Root", "CurrentUser")
$rootStore.Open("ReadWrite")
$rootStore.Add($cert)
$rootStore.Close()

$targets = @(
    "build\Release\AbductionStudioV2.exe",
    "C:\Users\USUÁRIO\AbductionStudioV2\AbductionStudioV2.exe"
)

foreach ($target in $targets) {
    if (Test-Path $target) {
        Set-AuthenticodeSignature -FilePath $target -Certificate $cert
        Write-Host "Assinado com sucesso: $target"
    }
}
