import time
import subprocess
import os
import sys
import tempfile
import ctypes
from ctypes import wintypes
from PIL import Image

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
user32 = ctypes.windll.user32
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"
os.makedirs(artifact_dir, exist_ok=True)

# 1. Finalizar processos antigos
os.system("taskkill /F /IM AbductionStudioV2.exe >nul 2>&1")
time.sleep(0.5)

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
        print(f"[TEST] Screenshot salva: {png_path} ({img.size[0]}x{img.size[1]})")
        return png_path
    return None

# Iniciar DAW de forma persistente
exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
print("[TEST] Iniciando Abduction Studio V2...")
subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2", creationflags=0x00000008) # DETACHED_PROCESS
time.sleep(4.0)

found_hwnd = None
def enum_cb(h, l):
    global found_hwnd
    buf = ctypes.create_unicode_buffer(512)
    user32.GetWindowTextW(h, buf, 512)
    if "Abduction Studio" in buf.value:
        found_hwnd = h
        return False
    return True

EnumProc = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
user32.EnumWindows(EnumProc(enum_cb), 0)

if found_hwnd:
    print(f"[TEST] DAW Encontrada! HWND={found_hwnd}. Maximizando e ativando foco...")
    user32.ShowWindow(found_hwnd, 3) # SW_MAXIMIZE
    user32.SetForegroundWindow(found_hwnd)
    time.sleep(1.0)
    
    # 1. Screenshot de Inicialização: Piano Roll ativo
    shot1 = trigger_and_save("daw_startup_piano_roll.png")
    
    # 2. Fechar Piano Roll usando o atalho F7
    print("[TEST] Enviando WM_KEYDOWN para F7...")
    WM_KEYDOWN = 0x0100
    WM_KEYUP = 0x0101
    VK_F7 = 0x76
    VK_F6 = 0x75
    user32.SendMessageW(found_hwnd, WM_KEYDOWN, VK_F7, 0)
    time.sleep(0.08)
    user32.SendMessageW(found_hwnd, WM_KEYUP, VK_F7, 0)
    time.sleep(1.0)
    shot2 = trigger_and_save("daw_step_sequencer_channel_rack.png")
    
    # 3. Pressionar F6 para alternar o Channel Rack
    user32.SendMessageW(found_hwnd, WM_KEYDOWN, VK_F6, 0)
    time.sleep(0.08)
    user32.SendMessageW(found_hwnd, WM_KEYUP, VK_F6, 0)
    time.sleep(1.0)
    shot3 = trigger_and_save("daw_after_f6_toggle.png")
    
    # 4. Pressionar F7 para reabrir o Piano Roll
    user32.SendMessageW(found_hwnd, WM_KEYDOWN, VK_F7, 0)
    time.sleep(0.08)
    user32.SendMessageW(found_hwnd, WM_KEYUP, VK_F7, 0)
    time.sleep(1.0)
    shot4 = trigger_and_save("daw_piano_roll_restored.png")
    
    print("[TEST] Teste concluído com sucesso!")
else:
    print("[ERROR] HWND da DAW não encontrado.")
