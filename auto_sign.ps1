$cert = Get-Item Cert:\CurrentUser\My\72712207F80D795AC28A543DEF1D757046F7665D
Export-Certificate -Cert $cert -FilePath "abduction.cer" -Force
Write-Host "Exportado abduction.cer"
