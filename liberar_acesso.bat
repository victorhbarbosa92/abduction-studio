@echo off
echo Solicitando permissoes de administrador para adicionar excecao no Windows Defender e desbloquear os executaveis...
powershell -Command "Start-Process powershell -ArgumentList '-Command \"Add-MpPreference -ExclusionPath ''C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2''; Get-ChildItem -Path ''C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2'' -Recurse | Unblock-File\"' -Verb RunAs"
echo.
echo Processo concluido! Se a janela de confirmacao de administrador (UAC) apareceu e voce aceitou, o bloqueio foi retirado.
echo Tente abrir o AbductionStudioV2.exe novamente.
pause
