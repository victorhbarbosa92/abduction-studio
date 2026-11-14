Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$proc = Start-Process -FilePath ".\AbductionStudioV2.exe" -ArgumentList "--open-dj" -PassThru
Start-Sleep -Seconds 5

$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bitmap = New-Object System.Drawing.Bitmap $screen.Width, $screen.Height
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.CopyFromScreen($screen.X, $screen.Y, 0, 0, $bitmap.Size)

$artPath = "C:\Users\o_vic\.gemini\antigravity-ide\brain\787ac822-c556-4e45-9247-d1cb82f2f490\abduction_studio_live_validation.png"
$bitmap.Save($artPath, [System.Drawing.Imaging.ImageFormat]::Png)

$deskPath = "C:\Users\o_vic\Desktop\Referencias_TopBar_UI\12_Validacao_Live_Ambiente.png"
$bitmap.Save($deskPath, [System.Drawing.Imaging.ImageFormat]::Png)

$graphics.Dispose()
$bitmap.Dispose()

Stop-Process -Id $proc.Id -Force
Write-Host "Live validation capture saved successfully!"
