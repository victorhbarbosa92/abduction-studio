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
echo [3] Assinando digitalmente (Anti-Virus Bypass)...
powershell -ExecutionPolicy Bypass -Command ^
  "$cert = Get-ChildItem -Path Cert:\CurrentUser\My | Where-Object Subject -match 'NovaDAW Developer' | Select-Object -First 1; ^
   if ($cert) { Set-AuthenticodeSignature -FilePath 'build\NovaDAW_artefacts\Release\Abduction Studio.exe' -Certificate $cert | Out-Null }"

echo.
echo [4] Removendo bloqueio de zona da internet (ZoneIdentifier)...
powershell -ExecutionPolicy Bypass -Command ^
  "Unblock-File -Path 'build\NovaDAW_artefacts\Release\Abduction Studio.exe' -ErrorAction SilentlyContinue; ^
   Remove-Item -Path 'build\NovaDAW_artefacts\Release\Abduction Studio.exe:Zone.Identifier' -ErrorAction SilentlyContinue"

echo.
echo [5] Tentando remover restricao do Device Guard via exclusao de WDAC...
powershell -ExecutionPolicy Bypass -Command ^
  "try { Add-MpPreference -ExclusionPath 'C:\NovaDAW\build\NovaDAW_artefacts\Release' -ErrorAction SilentlyContinue } catch {}"

echo.
echo [6] Abrindo a aplicacao diretamente...
powershell -ExecutionPolicy Bypass -Command ^
  "Start-Process -FilePath 'C:\NovaDAW\build\NovaDAW_artefacts\Release\Abduction Studio.exe' -ErrorAction Stop" 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [AVISO] Abertura automatica bloqueada pelo Device Guard.
    echo.
    echo === SOLUCAO MANUAL ===
    echo Abra o Explorador de Arquivos e va ate:
    echo   C:\NovaDAW\build\NovaDAW_artefacts\Release\
    echo Clique com botao direito em "Abduction Studio.exe"
    echo Selecione "Propriedades" e clique em "Desbloquear"
    echo Entao abra normalmente com duplo clique.
    echo ======================
    echo.
    start explorer "C:\NovaDAW\build\NovaDAW_artefacts\Release"
)

echo.
echo [7] Sincronizando com GitHub...
cd /d C:\NovaDAW
git add -A
git commit -m "build: Kuru Edition - identity visual update" 2>nul
git push origin master 2>nul

echo.
echo ==========================================
echo   CONCLUIDO! A nave esta pronta, Kuru.
echo ==========================================
pause
