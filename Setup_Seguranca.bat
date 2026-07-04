@echo off
chcp 65001 > nul
echo.
echo ==========================================
echo   ABDUCTION STUDIO - SETUP INICIAL
echo   Instalando certificado de confianca...
echo ==========================================
echo.

REM Verificar se esta rodando como admin
net session >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] Precisamos de permissao de Administrador.
    echo     Aguarde a janela de UAC aparecer...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

echo [1] Criando certificado de assinatura de codigo...
powershell -ExecutionPolicy Bypass -Command ^
  "$cert = New-SelfSignedCertificate -Subject 'CN=NovaDAW Developer,O=AbductionStudio,C=BR' -CertStoreLocation 'Cert:\CurrentUser\My' -KeyUsage DigitalSignature -Type CodeSigningCert -NotAfter (Get-Date).AddYears(10); Write-Host ('  Certificado criado: ' + $cert.Thumbprint)"

echo.
echo [2] Instalando certificado nas lojas de confianca da maquina...
powershell -ExecutionPolicy Bypass -Command ^
  "$cert = Get-ChildItem -Path 'Cert:\CurrentUser\My' | Where-Object Subject -match 'NovaDAW Developer' | Select-Object -First 1; ^
   $store = New-Object System.Security.Cryptography.X509Certificates.X509Store('TrustedPublisher','LocalMachine'); ^
   $store.Open('ReadWrite'); $store.Add($cert); $store.Close(); ^
   $root = New-Object System.Security.Cryptography.X509Certificates.X509Store('Root','LocalMachine'); ^
   $root.Open('ReadWrite'); $root.Add($cert); $root.Close(); ^
   Write-Host '  OK: Certificado instalado em TrustedPublisher e Root (LocalMachine)'"

echo.
echo [3] Adicionando exclusoes no Windows Defender...
powershell -ExecutionPolicy Bypass -Command ^
  "Add-MpPreference -ExclusionPath 'C:\NovaDAW' -ErrorAction SilentlyContinue; ^
   Add-MpPreference -ExclusionPath 'C:\NovaDAW\build' -ErrorAction SilentlyContinue; ^
   Add-MpPreference -ExclusionProcess 'Abduction Studio.exe' -ErrorAction SilentlyContinue; ^
   Write-Host '  OK: Exclusoes adicionadas no Defender'"

echo.
echo [4] Assinando e desbloqueando o executavel atual...
powershell -ExecutionPolicy Bypass -Command ^
  "$exe = 'C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe'; ^
   if (Test-Path $exe) { ^
     $cert = Get-ChildItem 'Cert:\CurrentUser\My' | Where-Object Subject -match 'NovaDAW Developer' | Select-Object -First 1; ^
     Set-AuthenticodeSignature -FilePath $exe -Certificate $cert | Out-Null; ^
     Unblock-File -Path $exe -ErrorAction SilentlyContinue; ^
     Remove-Item -Path ($exe + ':Zone.Identifier') -ErrorAction SilentlyContinue; ^
     Write-Host '  OK: Executavel assinado e desbloqueado' ^
   } else { Write-Host '  AVISO: Compile primeiro com Compilar_E_Liberar.bat' }"

echo.
echo [5] Criando atalho no Desktop...
powershell -ExecutionPolicy Bypass -Command ^
  "$ws = New-Object -ComObject WScript.Shell; ^
   $desktop = [Environment]::GetFolderPath('Desktop'); ^
   $shortcut = $ws.CreateShortcut($desktop + '\Abduction Studio.lnk'); ^
   $shortcut.TargetPath = 'C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe'; ^
   $shortcut.WorkingDirectory = 'C:\NovaDAW\build\NovaDAW_artefacts\Release'; ^
   $shortcut.IconLocation = 'C:\NovaDAW\resources\icon.ico,0'; ^
   $shortcut.Description = 'Abduction Studio - Kuru Edition'; ^
   $shortcut.Save(); ^
   Write-Host '  OK: Atalho criado no Desktop'"

echo.
echo ==========================================
echo   SETUP CONCLUIDO!
echo   Agora abra pelo atalho no Desktop.
echo   Dois cliques = Abduction Studio abre!
echo ==========================================
echo.
pause
