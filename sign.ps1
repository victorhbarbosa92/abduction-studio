$cert = Get-Item Cert:\CurrentUser\My\C57EBF8F0F57BBDA009B836F90854E81A3131D4F
if ($cert) {
    Set-AuthenticodeSignature -FilePath "build\Debug\AbductionStudioV2.exe" -Certificate $cert -ErrorAction SilentlyContinue
    Set-AuthenticodeSignature -FilePath "build\Release\AbductionStudioV2.exe" -Certificate $cert -ErrorAction SilentlyContinue
}
Unblock-File "build\Debug\AbductionStudioV2.exe" -ErrorAction SilentlyContinue
Unblock-File "build\Release\AbductionStudioV2.exe" -ErrorAction SilentlyContinue
