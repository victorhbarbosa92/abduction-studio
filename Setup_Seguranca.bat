@echo off
chcp 65001 > nul

:: Verificar se ja eh admin
net session >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Solicitando permissao de Administrador...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

echo.
echo ==========================================
echo   ABDUCTION STUDIO - DESBLOQUEIO TOTAL
echo   [Executando como Administrador]
echo ==========================================
echo.

echo [1] Desativando Smart App Control (causa raiz do bloqueio)...
reg add "HKLM\SYSTEM\CurrentControlSet\Control\CI\Policy" /v VerifiedAndReputablePolicyState /t REG_DWORD /d 0 /f
if %ERRORLEVEL% EQU 0 (
    echo   OK: Smart App Control DESATIVADO
) else (
    echo   AVISO: Falhou - tentando metodo alternativo...
    powershell -ExecutionPolicy Bypass -Command "Set-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\CI\Policy' -Name 'VerifiedAndReputablePolicyState' -Value 0 -Type DWord -Force"
)

echo.
echo [2] Instalando certificado de confianca no nivel de MAQUINA...
powershell -ExecutionPolicy Bypass -Command ^
  "$cert = Get-ChildItem 'Cert:\CurrentUser\My' | Where-Object { $_.Subject -like '*Kuru*' } | Select-Object -First 1; ^
   if (-not $cert) { ^
     $cert = New-SelfSignedCertificate -Subject 'CN=Kuru,O=AbductionStudio,C=BR' -CertStoreLocation 'Cert:\CurrentUser\My' -KeyUsage DigitalSignature -Type CodeSigningCert -NotAfter (Get-Date).AddYears(10); ^
     Write-Host ('  Certificado criado: ' + $cert.Thumbprint) ^
   }; ^
   $tp = New-Object System.Security.Cryptography.X509Certificates.X509Store('TrustedPublisher','LocalMachine'); ^
   $tp.Open('ReadWrite'); $tp.Add($cert); $tp.Close(); ^
   $root = New-Object System.Security.Cryptography.X509Certificates.X509Store('Root','LocalMachine'); ^
   $root.Open('ReadWrite'); $root.Add($cert); $root.Close(); ^
   Write-Host '  OK: Certificado instalado em TrustedPublisher + Root (LocalMachine)'"

echo.
echo [3] Adicionando exclusoes no Windows Defender...
powershell -ExecutionPolicy Bypass -Command ^
  "Add-MpPreference -ExclusionPath 'C:\NovaDAW' -Force; ^
   Add-MpPreference -ExclusionPath 'C:\NovaDAW\build\NovaDAW_artefacts\Release' -Force; ^
   Add-MpPreference -ExclusionProcess 'Abduction Studio.exe' -Force; ^
   Write-Host '  OK: Exclusoes adicionadas'"

echo.
echo [4] Assinando e desbloqueando o executavel...
powershell -ExecutionPolicy Bypass -Command ^
  "$exe = 'C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe'; ^
   if (Test-Path $exe) { ^
     $cert = Get-ChildItem 'Cert:\CurrentUser\My' | Where-Object { $_.Subject -like '*Kuru*' } | Select-Object -First 1; ^
     Set-AuthenticodeSignature -FilePath $exe -Certificate $cert | Out-Null; ^
     Unblock-File -Path $exe -ErrorAction SilentlyContinue; ^
     Remove-Item ($exe + ':Zone.Identifier') -Force -ErrorAction SilentlyContinue; ^
     Write-Host '  OK: Executavel assinado e desbloqueado' ^
   } else { Write-Host '  INFO: Compile primeiro com Compilar_E_Liberar.bat' }"

echo.
echo [5] Recriando atalho no Desktop...
powershell -ExecutionPolicy Bypass -Command ^
  "$ws = New-Object -ComObject WScript.Shell; ^
   $desktop = [Environment]::GetFolderPath('Desktop'); ^
   $sc = $ws.CreateShortcut($desktop + '\Abduction Studio.lnk'); ^
   $sc.TargetPath = 'C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe'; ^
   $sc.WorkingDirectory = 'C:\NovaDAW\build\NovaDAW_artefacts\Release'; ^
   $sc.IconLocation = 'C:\NovaDAW\resources\icon.ico,0'; ^
   $sc.Description = 'Abduction Studio - Kuru Edition'; ^
   $sc.Save(); Write-Host '  OK: Atalho recriado'"

echo.
echo [6] Abrindo Abduction Studio para testar...
start "" "C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe"

echo.
echo ==========================================
echo   DESBLOQUEIO TOTAL CONCLUIDO!
echo   O Smart App Control foi desativado.
echo   O app agora abre sem restricoes.
echo   REINICIE O PC para garantir o efeito.
echo ==========================================
echo.
pause

