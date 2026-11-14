import time
import subprocess
import os
import tempfile
from PIL import Image

artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\1f79439e-0624-4173-b22c-45fb0be163fa"
os.makedirs(artifact_dir, exist_ok=True)

temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, "abduction_screenshot_req.txt")
bmp_out_file = os.path.join(temp_dir, "abduction_screenshot.bmp")

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_file, "w") as f:
        f.write("trigger\n")

    captured = False
    for _ in range(50):
        time.sleep(0.1)
        if os.path.exists(bmp_out_file) and os.path.getsize(bmp_out_file) > 1000:
            captured = True
            break

    if captured:
        png_path = os.path.join(artifact_dir, png_name)
        img = Image.open(bmp_out_file)
        img.save(png_path)
        try: os.remove(bmp_out_file)
        except: pass
        print(f"[TEST] Screenshot gravada: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    else:
        print("[TEST] In-engine BMP nao gerado, tentando PowerShell CopyFromScreen...")
        ps_script = f'''
        Add-Type -AssemblyName System.Windows.Forms
        Add-Type -AssemblyName System.Drawing
        $screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
        $bitmap = New-Object System.Drawing.Bitmap $screen.Width, $screen.Height
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        $graphics.CopyFromScreen($screen.X, $screen.Y, 0, 0, $bitmap.Size)
        $bitmap.Save("{os.path.join(artifact_dir, png_name)}", [System.Drawing.Imaging.ImageFormat]::Png)
        $graphics.Dispose()
        $bitmap.Dispose()
        '''
        subprocess.run(["powershell", "-Command", ps_script])
        print(f"[TEST] Fallback gravado: {png_name}")

print("[TEST] 1. Reiniciando DAW...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(3.0)

# Iniciar com o Separador de Stems aberto
trigger_and_save("abduction_stem_slicer_live.png")
time.sleep(1.0)

print("[TEST] Bateria de testes concluida!")
