@echo off
title Iniciar Abduction Studio V2
cd /d "%~dp0"
set "APP_DIR=%~dp0"
if "%APP_DIR:~-1%"=="\" set "APP_DIR=%APP_DIR:~0,-1%"

echo ===============================================================================
echo                   INICIANDO ABDUCTION STUDIO V2
echo ===============================================================================

REM 1. Executa o script persistente de assinatura digital e desbloqueio
if exist "%APP_DIR%\sign.ps1" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%APP_DIR%\sign.ps1" >nul 2>&1
) else (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Get-ChildItem -Path '%APP_DIR%' -Filter '*.exe' | Unblock-File -ErrorAction SilentlyContinue; Get-ChildItem -Path '%APP_DIR%' -Filter '*.dll' | Unblock-File -ErrorAction SilentlyContinue" >nul 2>&1
)

REM 2. Identifica o executavel correto
set "EXE_PATH="
if exist "%APP_DIR%\AbductionStudioV2.exe" (
    set "EXE_PATH=%APP_DIR%\AbductionStudioV2.exe"
) else if exist "%APP_DIR%\build\AbductionStudioV2.exe" (
    set "EXE_PATH=%APP_DIR%\build\AbductionStudioV2.exe"
) else if exist "%APP_DIR%\build\Release\AbductionStudioV2.exe" (
    set "EXE_PATH=%APP_DIR%\build\Release\AbductionStudioV2.exe"
)

if "%EXE_PATH%"=="" (
    echo [ERRO FATAL] O executavel AbductionStudioV2.exe nao foi encontrado!
    echo Compile o projeto via CMake ou extraia os arquivos completamente.
    echo.
    pause
    exit /b 1
)

echo Executavel: "%EXE_PATH%"
echo Carregando modulos de audio, graficos e inteligencia artificial...
echo.

REM 3. Inicia o processo com diretorio de trabalho correto de forma independente
powershell -NoProfile -ExecutionPolicy Bypass -Command "$res = Invoke-CimMethod -ClassName Win32_Process -MethodName Create -Arguments @{CommandLine = '%EXE_PATH%'; CurrentDirectory = '%APP_DIR%'}; if ($res.ReturnValue -ne 0) { Start-Process -FilePath '%EXE_PATH%' -WorkingDirectory '%APP_DIR%' }"

REM 4. Aguarda 2 segundos para verificar se o processo continuou em execucao
powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Sleep -Seconds 2; $p = Get-Process -Name 'AbductionStudioV2' -ErrorAction SilentlyContinue; if ($p) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo [SUCESSO] Abduction Studio V2 iniciado com sucesso!
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Sleep -Seconds 2" >nul 2>&1
    exit /b 0
)

echo.
echo ===============================================================================
echo [AVISO] O Abduction Studio fechou ou foi impedido de abrir pelo Windows.
echo.
echo SE FOR BLOQUEIO DA SEGURANCA DO WINDOWS (Defender ou Smart App Control):
echo Execute 'liberar_acesso.bat' como Administrador para registrar a exclusao
echo permanente da pasta e confiar no certificado do aplicativo.
echo ===============================================================================
echo.
pause
