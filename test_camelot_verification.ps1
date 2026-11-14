Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$artifactDir = "C:\Users\o_vic\.gemini\antigravity-ide\brain\7a8b599d-0d06-45da-bdfb-4e1e39dd08e9"

# 1. Teste do Radar Camelot
Write-Host "Iniciando teste do Radar Camelot..."
$proc1 = Start-Process -FilePath ".\AbductionStudioV2.exe" -ArgumentList "--test-camelot-radar" -PassThru
Start-Sleep -Seconds 4

$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bmp1 = New-Object System.Drawing.Bitmap $screen.Width, $screen.Height
$g1 = [System.Drawing.Graphics]::FromImage($bmp1)
$g1.CopyFromScreen($screen.X, $screen.Y, 0, 0, $bmp1.Size)
$bmp1.Save("$artifactDir\camelot_radar_modal_live.png", [System.Drawing.Imaging.ImageFormat]::Png)
$g1.Dispose()
$bmp1.Dispose()
Stop-Process -Id $proc1.Id -Force
Write-Host "Screenshot 1 salvo em camelot_radar_modal_live.png"

Start-Sleep -Seconds 1

# 2. Teste das Crates com Camelot Sort e Badges
Write-Host "Iniciando teste das Crates com Camelot Badges..."
$proc2 = Start-Process -FilePath ".\AbductionStudioV2.exe" -ArgumentList "--test-dj-crates" -PassThru
Start-Sleep -Seconds 4

$bmp2 = New-Object System.Drawing.Bitmap $screen.Width, $screen.Height
$g2 = [System.Drawing.Graphics]::FromImage($bmp2)
$g2.CopyFromScreen($screen.X, $screen.Y, 0, 0, $bmp2.Size)
$bmp2.Save("$artifactDir\camelot_crates_badges_live.png", [System.Drawing.Imaging.ImageFormat]::Png)
$g2.Dispose()
$bmp2.Dispose()
Stop-Process -Id $proc2.Id -Force
Write-Host "Screenshot 2 salvo em camelot_crates_badges_live.png"
