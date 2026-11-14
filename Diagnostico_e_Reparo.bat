@echo off
title Diagnostico e Auto-Reparo - Abduction Studio V2
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0diagnostico.ps1"
echo.
pause
