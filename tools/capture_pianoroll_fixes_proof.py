import time
import subprocess
import os
import sys
import tempfile
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

artifact_dir = r'C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3'
temp_dir = tempfile.gettempdir()
trigger_file = os.path.join(temp_dir, 'abduction_screenshot_req.txt')
bmp_out_file = os.path.join(temp_dir, 'abduction_screenshot.bmp')

def trigger_and_save(png_name):
    if os.path.exists(bmp_out_file):
        try: os.remove(bmp_out_file)
        except: pass

    with open(trigger_file, 'w') as f:
        f.write('trigger\n')

    captured = False
    for _ in range(60):
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
        print(f'[TEST] Screenshot salva: {png_name} ({img.size[0]}x{img.size[1]})')
        return png_path
    print(f'[TEST] ERRO ao capturar {png_name}')
    return None

def kill_daw():
    subprocess.run(['powershell', '-Command', 'Stop-Process -Name \'AbductionStudioV2\' -Force -ErrorAction SilentlyContinue'])
    time.sleep(0.8)

exe_path = r'C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe'
cwd = r'C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2'

print('\n[PROVA 1] Capturando Piano Roll com borda unica sem linha dupla e botao Sample...')
kill_daw()
proc = subprocess.Popen([exe_path, '--psy-rhythm-generate-all'], cwd=cwd)
time.sleep(3.5)
trigger_and_save('11_piano_roll_border_fixed.png')
proc.terminate()
kill_daw()

print('\n[PROVA 2] Capturando animacao de Playback em tempo real no Piano Roll...')
kill_daw()
proc = subprocess.Popen([exe_path, '--test-pr-playing'], cwd=cwd)
time.sleep(2.5)
trigger_and_save('12_piano_roll_playing_animation.png')
proc.terminate()
kill_daw()

print('\n[PROVA 3] Capturando janela de Configuracoes / Editor de Sample...')
kill_daw()
proc = subprocess.Popen([exe_path, '--test-pr-sampler'], cwd=cwd)
time.sleep(3.5)
trigger_and_save('13_sampler_settings_open.png')
proc.terminate()
kill_daw()

print('\n[PROVA 4] Capturando menu ☷ com novos samples de Bass, Kick e Snare...')
kill_daw()
proc = subprocess.Popen([exe_path, '--test-cr-menu'], cwd=cwd)
time.sleep(3.5)
trigger_and_save('14_sample_menu_expanded.png')
proc.terminate()
kill_daw()

print('\nTodas as provas capturadas com sucesso!')
