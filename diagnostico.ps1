# ==============================================================================
#            ABDUCTION STUDIO V2 - DIAGNOSTICO E AUTO-REPARO DE SISTEMA
# ==============================================================================
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$appDir = $PSScriptRoot
Write-Host "==============================================================================" -ForegroundColor Cyan
Write-Host "            ABDUCTION STUDIO V2 - DIAGNOSTICO E AUTO-REPARO DE SISTEMA" -ForegroundColor Cyan
Write-Host "==============================================================================" -ForegroundColor Cyan
Write-Host "Pasta do Projeto: $appDir`n"

# 1. Desbloqueio de arquivos
Write-Host "[1/6] Desbloqueando arquivos (removendo bloqueio do Windows Defender/SmartScreen)..." -ForegroundColor Yellow
try {
    Get-ChildItem -Path $appDir -Recurse -File | Unblock-File -ErrorAction SilentlyContinue
    Write-Host "      [OK] Todos os arquivos foram desbloqueados com sucesso." -ForegroundColor Green
} catch {
    Write-Host "      [AVISO] Nao foi possivel desbloquear todos os arquivos automaticamente." -ForegroundColor DarkYellow
}

# 2. Visual C++ Redistributable
Write-Host "`n[2/6] Verificando Microsoft Visual C++ 2015-2022 Redistributable (x64)..." -ForegroundColor Yellow
$vcInstalled = $false
try {
    $vcKey = Get-ItemProperty -Path 'HKLM:\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64' -ErrorAction SilentlyContinue
    if ($vcKey -and $vcKey.Installed -eq 1) {
        $vcInstalled = $true
    }
} catch {}

if ($vcInstalled) {
    Write-Host "      [OK] Microsoft Visual C++ 2015-2022 (x64) esta instalado no sistema!" -ForegroundColor Green
} else {
    Write-Host "      [ALERTA] Visual C++ Redistributable NAO encontrado no registro do Windows!" -ForegroundColor Red
    $installer = Join-Path $appDir "vc_redist.x64.exe"
    if (Test-Path $installer) {
        Write-Host "      Iniciando instalacao do instalador incluso no pacote..." -ForegroundColor Cyan
        Start-Process -FilePath $installer -ArgumentList "/install /passive /norestart" -Wait
        Write-Host "      [OK] Instalacao do Visual C++ concluida!" -ForegroundColor Green
    } else {
        Write-Host "      [ERRO] Arquivo 'vc_redist.x64.exe' nao encontrado na pasta raiz." -ForegroundColor Red
    }
}

# 3. Verificacao de arquivos essenciais
Write-Host "`n[3/6] Verificando arquivos essenciais da DAW..." -ForegroundColor Yellow
$filesToCheck = @(
    "AbductionStudioV2.exe",
    "onnxruntime.dll",
    "vcruntime140.dll",
    "msvcp140.dll",
    "src\ui\Roboto-Regular.ttf",
    "src\ui\fa-solid-900.ttf"
)

$missingCount = 0
foreach ($f in $filesToCheck) {
    $p = Join-Path $appDir $f
    if (-not (Test-Path $p)) {
        Write-Host "      [FALTANDO] $f" -ForegroundColor Red
        $missingCount++
    }
}

if ($missingCount -eq 0) {
    Write-Host "      [OK] Executavel principal, bibliotecas de IA e fontes presentes." -ForegroundColor Green
} else {
    Write-Host "      [ERRO] $missingCount arquivos essenciais estao faltando! Extraia todo o ZIP novamente." -ForegroundColor Red
}

# 4. Processador
Write-Host "`n[4/6] Verificando hardware e processador..." -ForegroundColor Yellow
try {
    $cpu = (Get-CimInstance Win32_Processor | Select-Object -First 1).Name.Trim()
    Write-Host "      Processador: $cpu" -ForegroundColor Cyan
} catch {
    Write-Host "      Nao foi possivel obter detalhes do processador." -ForegroundColor Gray
}

# 5. Audio WASAPI
Write-Host "`n[5/6] Verificando dispositivos de audio..." -ForegroundColor Yellow
try {
    $audioDevices = Get-CimInstance Win32_SoundDevice | Select-Object -ExpandProperty Name
    if ($audioDevices) {
        Write-Host "      Dispositivos encontrados:" -ForegroundColor Green
        foreach ($d in $audioDevices) {
            Write-Host "       - $d" -ForegroundColor White
        }
    } else {
        Write-Host "      [AVISO] Nenhum dispositivo de som detectado. Conecte fones ou caixas de som." -ForegroundColor DarkYellow
    }
} catch {
    Write-Host "      Falha ao consultar dispositivos de som." -ForegroundColor Gray
}

# 6. Teste do Executavel
Write-Host "`n[6/6] Testando execucao do Abduction Studio V2..." -ForegroundColor Yellow
$exePath = Join-Path $appDir "AbductionStudioV2.exe"
if (Test-Path $exePath) {
    Write-Host "      Tentando iniciar processo de teste..." -ForegroundColor Cyan
    $p = Start-Process -FilePath $exePath -PassThru -ErrorAction SilentlyContinue
    if ($p) {
        Start-Sleep -Seconds 2
        if (-not $p.HasExited) {
            Write-Host "      [SUCESSO] O Abduction Studio abriu e esta respondendo perfeitamente!" -ForegroundColor Green
            Write-Host "      Fechando teste preliminar..." -ForegroundColor Cyan
            Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
        } else {
            $hexCode = "0x" + [System.Convert]::ToString($p.ExitCode, 16).ToUpper()
            Write-Host "      [ALERTA] O processo fechou rapidamente com codigo de saida: $($p.ExitCode) ($hexCode)" -ForegroundColor Red
            if ($p.ExitCode -eq -1073741795 -or $hexCode -eq "0xC000001D") {
                Write-Host "      [MOTIVO] STATUS_ILLEGAL_INSTRUCTION: A CPU tentou executar uma instrucao nao suportada pelo hardware (ex: falta de AVX/AVX2 no processador)." -ForegroundColor Yellow
                Write-Host "      [SOLUCAO] Utilize a versao universal x64 (SSE2) do Abduction Studio, compativel com processadores mais antigos." -ForegroundColor Green
            } elseif ($p.ExitCode -eq -1073741515 -or $hexCode -eq "0xC0000135") {
                Write-Host "      [MOTIVO] STATUS_DLL_NOT_FOUND: Uma DLL necessaria nao foi encontrada." -ForegroundColor Yellow
                Write-Host "      [SOLUCAO] Execute o instalador 'vc_redist.x64.exe' incluso nesta pasta." -ForegroundColor Green
            }
            $crashLog = Join-Path $appDir "crash_log.txt"
            if (Test-Path $crashLog) {
                Write-Host "      Detalhes do crash_log.txt:" -ForegroundColor Red
                Get-Content $crashLog | ForEach-Object { Write-Host "        $_" -ForegroundColor DarkRed }
            }
        }
    } else {
        Write-Host "      [ERRO] O Windows impediu o inicio do processo. Verifique as permissoes do Defender/SmartScreen." -ForegroundColor Red
    }
}

Write-Host "`n==============================================================================" -ForegroundColor Cyan
Write-Host "Diagnostico finalizado. Se tudo estiver verde [OK], abra 'Iniciar_DAW.bat'." -ForegroundColor Cyan
Write-Host "==============================================================================" -ForegroundColor Cyan
