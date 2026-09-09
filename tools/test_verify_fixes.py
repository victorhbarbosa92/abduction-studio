import time
import subprocess
import os
import sys
import tempfile
import ctypes
from ctypes import wintypes
from PIL import Image

if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

user32 = ctypes.windll.user32
artifact_dir = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\brain\7d52423a-f2fd-4274-a20e-6d127ec2cac3"
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
        print(f"[TEST] Screenshot salva com sucesso: {png_name} ({img.size[0]}x{img.size[1]})")
        return png_path
    print(f"[TEST] Erro ao capturar {png_name}")
    return None

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
KEYEVENTF_KEYUP = 0x0002
VK_SPACE = 0x20
VK_F7 = 0x76

def get_window_rect():
    hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
    if not hwnd:
        return None, (0, 0, 1280, 720)
    user32.ShowWindow(hwnd, 9) # SW_RESTORE
    user32.SetForegroundWindow(hwnd)
    rect = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    return hwnd, (rect.left, rect.top, rect.right, rect.bottom)

def mouse_move(x, y):
    user32.SetCursorPos(int(x), int(y))
    time.sleep(0.08)

def mouse_click(x, y):
    user32.SetCursorPos(int(x), int(y))
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.08)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    time.sleep(0.3)

def press_space():
    user32.keybd_event(VK_SPACE, 0, 0, 0)
    time.sleep(0.05)
    user32.keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0)
    time.sleep(0.3)

def press_key(vk):
    user32.keybd_event(vk, 0, 0, 0)
    time.sleep(0.06)
    user32.keybd_event(vk, 0, KEYEVENTF_KEYUP, 0)
    time.sleep(0.4)

print("[TEST] 1. Reiniciando DAW com --open-psy-bass...")
subprocess.run(["powershell", "-Command", "Stop-Process -Name 'AbductionStudioV2' -Force -ErrorAction SilentlyContinue"])
time.sleep(1.0)

exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
proc = subprocess.Popen([exe_path, "--open-psy-bass"], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")
time.sleep(3.5)

hwnd, rect = get_window_rect()
wx, wy = rect[0], rect[1]
print(f"[TEST] Janela localizada em: ({wx}, {wy})")

# ── TESTE 1: CAPTURAR KURO PSY ROLLING BASS ENGINE ABERTO ──
print("[TEST] 2. Capturando Kuro Psy Rolling Bass Engine aberto com novo layout...")
shot1 = trigger_and_save("01_kuro_psy_bass_initial_window.png")

# ── TESTE 2: INTERAGIR COM OS COMBOS DE SYNTH ENGINE, ROOT NOTE E SCALE ──
print("[TEST] 3. Interagindo com Root Note, Scale e Synth Engine...")
# A janela tem 1040x580 centrada em 1280x720: Min=(120, 70).
# Synth Engine combo: x=120 + 228 + 60 = 408, y=70 + 26 + 14 + 12 = 122
synth_combo_x = wx + 408
synth_combo_y = wy + 122
mouse_click(synth_combo_x, synth_combo_y) # Abre dropdown
time.sleep(0.3)
mouse_click(synth_combo_x, synth_combo_y + 35) # Seleciona opção 1 (Virus TI Hyper-Saw)
time.sleep(0.4)

# Root Note combo: x=120 + 228 + 163 + 193 + 35 = 739, y=122
root_combo_x = wx + 739
root_combo_y = wy + 122
mouse_click(root_combo_x, root_combo_y) # Abre dropdown
time.sleep(0.3)
# Desce para selecionar A1
mouse_click(root_combo_x, root_combo_y + 175)
time.sleep(0.4)

# Scale combo: x=120 + 228 + 163 + 193 + 82 + 50 = 846, y=122
scale_combo_x = wx + 846
scale_combo_y = wy + 122
mouse_click(scale_combo_x, scale_combo_y) # Abre dropdown
time.sleep(0.3)
mouse_click(scale_combo_x, scale_combo_y + 35) # Seleciona Natural Minor
time.sleep(0.4)

shot2 = trigger_and_save("02_kuro_psy_bass_modified_settings.png")

# ── TESTE 3: DISPARAR AUDITION LOOP ──
print("[TEST] 4. Disparando Audition Loop...")
# Botão Audition está em x=120 + 228 + 163 + 193 + 82 + 118 + 70 = 974, y=122
aud_btn_x = wx + 974
aud_btn_y = wy + 122
mouse_click(aud_btn_x, aud_btn_y)
time.sleep(1.2) # Deixa rodar para playhead e osciloscópio animarem
shot3 = trigger_and_save("03_kuro_psy_bass_audition_playing.png")

# ── TESTE 4: CLICAR EM GENERATE TO PIANO ROLL ──
print("[TEST] 5. Clicando em GENERATE TO PIANO ROLL...")
gen_pr_x = wx + 320
gen_pr_y = wy + 552
mouse_click(gen_pr_x, gen_pr_y)
time.sleep(1.0)
shot4 = trigger_and_save("04_kuro_psy_bass_generated_to_piano_roll.png")

# Parar audition
press_space()
time.sleep(0.4)

# ── TESTE 5: FECHAR KURO BASS CLICANDO NO X DA JANELA E TESTAR OLED HINT DISPLAY ──
print("[TEST] 6. Fechando Kuro Bass para testar Display OLED...")
# Botão de fechar da janela Kuro Bass (canto superior direito da janela 1040x580 em 120, 70: x=120+1040-15 = 1145, y=70+10 = 80)
mouse_click(wx + 1145, wy + 80)
time.sleep(0.8)

# Hover sobre o Seletor de Snap na barra superior para disparar dica longa
print("[TEST] 7. Posicionando mouse no Snap Selector para testar recorte do OLED...")
snap_x = wx + 390
snap_y = wy + 48
mouse_move(snap_x, snap_y)
time.sleep(1.2)
shot5 = trigger_and_save("05_oled_hint_text_clipped.png")

# ── TESTE 6: ABRIR PIANO ROLL PARA VALIDAR NOTAS GERADAS ──
print("[TEST] 8. Abrindo Piano Roll via F7...")
press_key(VK_F7)
time.sleep(1.2)
shot6 = trigger_and_save("06_piano_roll_with_generated_bassline.png")

print("[TEST] 9. Todos os testes de validação concluídos!")
time.sleep(1.0)
proc.terminate()
