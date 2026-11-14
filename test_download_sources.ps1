$destExe = "C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\AbductionStudioV2.exe"
$appDir  = "C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"

Stop-Process -Name "AbductionStudioV2" -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 600

$tempDir = [System.IO.Path]::GetTempPath()
$reqFile = Join-Path $tempDir "abduction_screenshot_req.txt"
$bmpFile = Join-Path $tempDir "abduction_screenshot.bmp"

if (Test-Path $bmpFile) { Remove-Item $bmpFile -Force -ErrorAction SilentlyContinue }
if (Test-Path $reqFile) { Remove-Item $reqFile -Force -ErrorAction SilentlyContinue }

Write-Host "Iniciando Abduction Studio V2 com --test-sources-modal..."
Start-Process -FilePath $destExe -ArgumentList "--test-sources-modal" -WorkingDirectory $appDir

Start-Sleep -Seconds 4

$proc = Get-Process -Name "AbductionStudioV2" -ErrorAction SilentlyContinue
if (-not $proc) {
    Write-Host "ERRO: Processo nao encontrado!"
    exit 1
}

Write-Host "Solicitando captura de tela..."
[System.IO.File]::WriteAllText($reqFile, "capture")

Start-Sleep -Seconds 2

$destPng = "C:\Users\o_vic\.gemini\antigravity-ide\brain\3b03f12e-fab9-459b-abc2-5dae0707567d\download_sources_live_verify.png"
if (Test-Path $bmpFile) {
    Add-Type -AssemblyName System.Drawing
    $bmp = [System.Drawing.Image]::FromFile($bmpFile)
    $bmp.Save($destPng, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    Write-Host "SUCESSO: $destPng salvo com sucesso!"
} else {
    Write-Host "AVISO: BMP nao foi encontrado!"
}

Stop-Process -Name "AbductionStudioV2" -Force -ErrorAction SilentlyContinue
