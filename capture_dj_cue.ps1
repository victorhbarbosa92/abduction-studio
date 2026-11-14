Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$proc = Start-Process -FilePath ".\AbductionStudioV2.exe" -ArgumentList "--open-dj" -PassThru
Start-Sleep -Seconds 4

$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bitmap = New-Object System.Drawing.Bitmap $screen.Width, $screen.Height
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.CopyFromScreen($screen.X, $screen.Y, 0, 0, $bitmap.Size)
$outPath = "C:\Users\o_vic\.gemini\antigravity-ide\brain\482c3b41-3541-456a-91d2-33b35dfea643\mixer_cue_fixed.png"
$bitmap.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)
$graphics.Dispose()
$bitmap.Dispose()

Stop-Process -Id $proc.Id -Force
Write-Host "Captured screenshot to $outPath"
