Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$proc = Start-Process -FilePath ".\AbductionStudioV2.exe" -ArgumentList "--open-dj" -PassThru
Start-Sleep -Seconds 4

$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bitmap = New-Object System.Drawing.Bitmap $screen.Width, $screen.Height
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.CopyFromScreen($screen.X, $screen.Y, 0, 0, $bitmap.Size)
$bitmap.Save("C:\Users\o_vic\.gemini\antigravity-ide\brain\a7531291-b5ae-4563-8e85-584c2f47d8c3\vortex_dj_skin_active.png", [System.Drawing.Imaging.ImageFormat]::Png)
$graphics.Dispose()
$bitmap.Dispose()

Stop-Process -Id $proc.Id -Force
Write-Host "Screenshot saved to vortex_dj_skin_active.png"
