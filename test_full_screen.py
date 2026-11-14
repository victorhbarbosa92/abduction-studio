import time
import subprocess
import os

artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\1f79439e-0624-4173-b22c-45fb0be163fa"
os.makedirs(artifact_dir, exist_ok=True)
png_path = os.path.join(artifact_dir, "desktop_stem_slicer_verified.png")

ps_script = f'''
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bitmap = New-Object System.Drawing.Bitmap $screen.Width, $screen.Height
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.CopyFromScreen($screen.X, $screen.Y, 0, 0, $bitmap.Size)
$bitmap.Save("{png_path}", [System.Drawing.Imaging.ImageFormat]::Png)
$graphics.Dispose()
$bitmap.Dispose()
Write-Host "Full screen captured to {png_path}"
'''

subprocess.run(["powershell", "-Command", ps_script])
