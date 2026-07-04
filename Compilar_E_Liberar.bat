@echo off
chcp 65001 > nul
echo ==========================================
echo   ABDUCTION STUDIO - COMPILADOR KURU
echo ==========================================
echo.

echo [1] Matando instancias abertas...
taskkill /IM "Abduction Studio.exe" /F 2>nul
timeout /t 1 /nobreak > nul

echo.
echo [2] Compilando o projeto...
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERRO] Compilacao falhou!
    pause
    exit /b 1
)

echo.
echo [3] Assinando digitalmente com certificado confiavel...
powershell -ExecutionPolicy Bypass -Command ^
  "$exe = 'C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe'; ^
   $cert = Get-ChildItem 'Cert:\CurrentUser\My' | Where-Object Subject -match 'NovaDAW Developer' | Sort-Object NotAfter -Descending | Select-Object -First 1; ^
   if ($cert) { ^
     Set-AuthenticodeSignature -FilePath $exe -Certificate $cert | Out-Null; ^
     Write-Host '  OK: Assinado com certificado confiavel' ^
   } else { ^
     Write-Host '  AVISO: Certificado nao encontrado. Execute Setup_Seguranca.bat primeiro!' ^
   }"

echo.
echo [4] Removendo todas as travas de seguranca...
powershell -ExecutionPolicy Bypass -Command ^
  "$exe = 'C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe'; ^
   Unblock-File -Path $exe -ErrorAction SilentlyContinue; ^
   Remove-Item -Path ($exe + ':Zone.Identifier') -ErrorAction SilentlyContinue; ^
   $bytes = [System.IO.File]::ReadAllBytes($exe); ^
   [System.IO.File]::WriteAllBytes($exe, $bytes); ^
   Write-Host '  OK: Travas removidas'"

echo.
echo [5] Garantindo exclusao no Windows Defender...
powershell -ExecutionPolicy Bypass -Command ^
  "Add-MpPreference -ExclusionPath 'C:\NovaDAW\build\NovaDAW_artefacts\Release' -ErrorAction SilentlyContinue; ^
   Add-MpPreference -ExclusionProcess 'Abduction Studio.exe' -ErrorAction SilentlyContinue"

echo.
echo [6] Abrindo Abduction Studio...
powershell -ExecutionPolicy Bypass -Command ^
  "$exe = 'C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe'; ^
   try { ^
     $proc = [System.Diagnostics.Process]::Start($exe); ^
     Write-Host ('  OK: Abduction Studio iniciado (PID: ' + $proc.Id + ')') ^
   } catch { ^
     Write-Host '  AVISO: Abertura automatica falhou. Use o atalho do Desktop.' ^
   }"

echo.
echo [7] Sincronizando com GitHub...
git add -A
git diff --cached --quiet
if %ERRORLEVEL% NEQ 0 (
    for /f "tokens=1-3 delims=/ " %%a in ('date /t') do set DATA=%%c-%%b-%%a
    for /f "tokens=1 delims= " %%a in ('time /t') do set HORA=%%a
    git commit -m "build: Kuru Edition - %DATA% %HORA%"
    git push origin main 2>nul
    echo   OK: Codigo enviado para o GitHub
) else (
    echo   Sem alteracoes para enviar.
)

echo.
echo ==========================================
echo   MISSAO CUMPRIDA, KURU!
echo ==========================================
pause
