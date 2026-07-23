@echo off
title Iniciar Abduction Studio V2
echo Iniciando a DAW Abduction Studio V2...
cd /d "%~dp0"
if exist "build\Release\AbductionStudioV2.exe" (
    start build\Release\AbductionStudioV2.exe
) else (
    start build\Debug\AbductionStudioV2.exe
)
exit

