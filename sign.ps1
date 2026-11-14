# ==============================================================================
# ABDUCTION STUDIO V2 - PERSISTENT CODE SIGNING, UNBLOCK & SHORTCUT PIPELINE
# Executado automaticamente pelo CMake (POST_BUILD) e por Iniciar_DAW.bat
# ==============================================================================
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$targetDir = $PSScriptRoot
if (-not $targetDir) { $targetDir = (Get-Location).Path }

Write-Host "--- [Auto-Sign] Protegendo e Assinando Abduction Studio V2 ---" -ForegroundColor Cyan

# 1. Localiza ou Cria o Certificado de Desenvolvedor Confiável
$certSubject = "CN=Abduction Studio Developer"
$cert = Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert -ErrorAction SilentlyContinue | Where-Object { $_.Subject -eq $certSubject } | Select-Object -First 1

if (-not $cert) {
    Write-Host "[Auto-Sign] Gerando certificado Code Signing..." -ForegroundColor Yellow
    $cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject $certSubject -FriendlyName "Abduction Studio Developer" -CertStoreLocation "Cert:\CurrentUser\My" -NotAfter (Get-Date).AddYears(10)
}

if ($cert) {
    # Exporta .cer para o diretório raiz para fácil inclusão no Trusted Root
    $cerPath = Join-Path $targetDir "abduction.cer"
    Export-Certificate -Cert $cert -FilePath $cerPath -Force | Out-Null

    # Registra silenciosamente em TrustedPublisher e CA do usuário atual (sem prompt de GUI)
    try {
        $pubStore = New-Object System.Security.Cryptography.X509Certificates.X509Store("TrustedPublisher", "CurrentUser")
        $pubStore.Open("ReadWrite")
        if (-not ($pubStore.Certificates | Where-Object { $_.Thumbprint -eq $cert.Thumbprint })) {
            $pubStore.Add($cert)
        }
        $pubStore.Close()
    } catch {}

    try {
        $caStore = New-Object System.Security.Cryptography.X509Certificates.X509Store("CA", "CurrentUser")
        $caStore.Open("ReadWrite")
        if (-not ($caStore.Certificates | Where-Object { $_.Thumbprint -eq $cert.Thumbprint })) {
            $caStore.Add($cert)
        }
        $caStore.Close()
    } catch {}

    # 2. Alvos para assinatura digital e desbloqueio
    $targets = @(
        (Join-Path $targetDir "AbductionStudioV2.exe"),
        (Join-Path $targetDir "build\AbductionStudioV2.exe"),
        (Join-Path $targetDir "build\Release\AbductionStudioV2.exe")
    )

    foreach ($exe in $targets) {
        if (Test-Path $exe) {
            # Remove o bloqueio 'Mark of the Web' (Zone.Identifier)
            Unblock-File -Path $exe -ErrorAction SilentlyContinue
            
            # Aplica a assinatura Authenticode com o certificado
            $sig = Set-AuthenticodeSignature -FilePath $exe -Certificate $cert -ErrorAction SilentlyContinue
            $statusStr = if ($sig.Status) { $sig.Status.ToString() } else { "Signed" }
            Write-Host "[Auto-Sign] Executavel assinado: $(Split-Path $exe -Leaf) [$statusStr]" -ForegroundColor Green
        }
    }
}

# 3. Desbloqueia todas as DLLs e executáveis
Get-ChildItem -Path $targetDir -Filter "*.exe" -ErrorAction SilentlyContinue | ForEach-Object { Unblock-File $_.FullName -ErrorAction SilentlyContinue }
Get-ChildItem -Path $targetDir -Filter "*.dll" -ErrorAction SilentlyContinue | ForEach-Object { Unblock-File $_.FullName -ErrorAction SilentlyContinue }
if (Test-Path (Join-Path $targetDir "build")) {
    Get-ChildItem -Path (Join-Path $targetDir "build") -Filter "*.dll" -ErrorAction SilentlyContinue | ForEach-Object { Unblock-File $_.FullName -ErrorAction SilentlyContinue }
    Get-ChildItem -Path (Join-Path $targetDir "build") -Filter "*.exe" -ErrorAction SilentlyContinue | ForEach-Object { Unblock-File $_.FullName -ErrorAction SilentlyContinue }
}

# 4. Atualiza/Recria automaticamente o atalho da Área de Trabalho (Desktop Shortcut)
try {
    $desktopPath = [Environment]::GetFolderPath("Desktop")
    $shortcutPath = Join-Path $desktopPath "Abduction Studio V2.lnk"
    $mainExe = Join-Path $targetDir "AbductionStudioV2.exe"
    if (Test-Path $mainExe) {
        $wshell = New-Object -ComObject WScript.Shell
        $shortcut = $wshell.CreateShortcut($shortcutPath)
        $shortcut.TargetPath = $mainExe
        $shortcut.WorkingDirectory = $targetDir
        $shortcut.Description = "Abduction Studio V2 - Flagship Edition"
        $icoPath = Join-Path $targetDir "plugins\MonkSynth.vst3\PlugIn.ico"
        if (Test-Path $icoPath) {
            $shortcut.IconLocation = "$icoPath,0"
        }
        $shortcut.Save()
        Unblock-File -Path $shortcutPath -ErrorAction SilentlyContinue
        Write-Host "[Auto-Sign] Atalho na Area de Trabalho atualizado com sucesso: $shortcutPath" -ForegroundColor Green
    }
} catch {
    Write-Host "[Auto-Sign] Aviso ao criar atalho no Desktop: $_" -ForegroundColor Yellow
}

Write-Host "--- [Auto-Sign] Concluido com sucesso ---" -ForegroundColor Green
