$cert = Get-Item Cert:\CurrentUser\My\C57EBF8F0F57BBDA009B836F90854E81A3131D4F
Export-Certificate -Cert $cert -FilePath ".\kuru.cer" -Force
Import-Certificate -FilePath ".\kuru.cer" -CertStoreLocation Cert:\CurrentUser\Root
Import-Certificate -FilePath ".\kuru.cer" -CertStoreLocation Cert:\CurrentUser\TrustedPublisher
Write-Host "Certificado instalado em Trusted Root e Trusted Publisher com sucesso!"
