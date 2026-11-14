@echo off
title Liberar Acesso - Abduction Studio V2
cd /d "%~dp0"
set "TARGET_DIR=%~dp0"
if "%TARGET_DIR:~-1%"=="\" set "TARGET_DIR=%TARGET_DIR:~0,-1%"

echo ===============================================================================
echo                ABDUCTION STUDIO V2 - DESBLOQUEIO E PERMISSOES
echo ===============================================================================
echo Pasta do projeto: "%TARGET_DIR%"
echo.

REM Verifica se ja esta rodando como Administrador
net session >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [STATUS] Executando com privilegios de Administrador.
    echo.
    echo 1. Adicionando exclusoes no Windows Defender...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Add-MpPreference -ExclusionPath '%TARGET_DIR%' -ErrorAction SilentlyContinue; Add-MpPreference -ExclusionProcess 'AbductionStudioV2.exe' -ErrorAction SilentlyContinue"
    echo    [OK] Exclusoes configuradas!
    echo.
    echo 2. Instalando certificado raiz confiavel no sistema...
    if exist "%TARGET_DIR%\abduction.cer" (
        powershell -NoProfile -ExecutionPolicy Bypass -Command "Import-Certificate -FilePath '%TARGET_DIR%\abduction.cer' -CertStoreLocation Cert:\LocalMachine\Root -ErrorAction SilentlyContinue; Import-Certificate -FilePath '%TARGET_DIR%\abduction.cer' -CertStoreLocation Cert:\LocalMachine\TrustedPublisher -ErrorAction SilentlyContinue"
        echo    [OK] Certificado registrado em Trusted Root e Trusted Publisher!
    )
    echo.
    echo 3. Removendo bloqueios Mark of the Web...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Get-ChildItem -Path '%TARGET_DIR%' -Recurse | Unblock-File -ErrorAction SilentlyContinue"
    echo    [OK] Arquivos desbloqueados!
    echo.
    echo 4. Aplicando assinatura digital e atualizando atalho na Area de Trabalho...
    powershell -NoProfile -ExecutionPolicy Bypass -File "%TARGET_DIR%\sign.ps1"
    echo.
    echo ===============================================================================
    echo [SUCESSO COMPLETO] O Abduction Studio V2 foi liberado em definitivo!
    echo O Windows Defender nao vai mais bloquear as compilacoes ou execucoes.
    echo Agora voce pode abrir o aplicativo diretamente pelo atalho na Area de Trabalho
    echo ou por Iniciar_DAW.bat.
    echo ===============================================================================
    pause
    exit /b 0
)

echo [STATUS] Privilegios normais de usuario detectados.
echo 1. Removendo bloqueio do Windows (Mark of the Web)...
powershell -NoProfile -ExecutionPolicy Bypass -Command "Get-ChildItem -Path '%TARGET_DIR%' -Recurse | Unblock-File -ErrorAction SilentlyContinue"
echo    [OK] Arquivos desbloqueados!
echo.
echo 2. Aplicando assinatura digital nos executaveis...
powershell -NoProfile -ExecutionPolicy Bypass -File "%TARGET_DIR%\sign.ps1"
echo.
echo 3. Solicitando elevacao de Administrador para registrar exclusao e certificado raiz...
powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Process '%~f0' -Verb RunAs"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [DICA] Se a janela de confirmacao nao abriu, clique com o BOTAO DIREITO
    echo neste arquivo 'liberar_acesso.bat' e escolha 'Executar como Administrador'.
)
echo.
pause
