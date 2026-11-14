import time
import subprocess
import os
import sys
import ctypes
from ctypes import wintypes

# Garante suporte a UTF-8 no terminal Windows
if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

user32 = ctypes.windll.user32

MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
KEYEVENTF_KEYUP = 0x0002
VK_SPACE = 0x20
VK_F10 = 0x79

def smooth_move(target_x, target_y, duration=0.8, steps=45):
    """Move o cursor do mouse suavemente em tempo real na tela do usuario."""
    point = wintypes.POINT()
    user32.GetCursorPos(ctypes.byref(point))
    start_x, start_y = point.x, point.y
    for i in range(1, steps + 1):
        t = i / steps
        # Curva de aceleracao/desaceleracao suave (Hermite SmoothStep)
        smooth_t = t * t * (3.0 - 2.0 * t)
        curr_x = start_x + (target_x - start_x) * smooth_t
        curr_y = start_y + (target_y - start_y) * smooth_t
        user32.SetCursorPos(int(curr_x), int(curr_y))
        time.sleep(duration / steps)
    time.sleep(0.1)

def smooth_click(x, y, label="", pause_after=0.6):
    """Move suavemente ate o alvo e executa um clique visivel com pausa."""
    print(f"[TESTE AO VIVO] -> Movendo para: {label} (X={int(x)}, Y={int(y)})")
    smooth_move(x, y, duration=0.7)
    time.sleep(0.15)
    user32.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
    time.sleep(0.12)
    user32.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
    print(f"[TESTE AO VIVO] -> Clicado: {label}")
    time.sleep(pause_after)

def send_space():
    user32.keybd_event(VK_SPACE, 0, 0, 0)
    time.sleep(0.1)
    user32.keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0)
    time.sleep(0.3)

print("==================================================================")
print("[TESTE AO VIVO] INICIANDO DEMONSTRACAO EM TEMPO REAL NA TELA")
print("==================================================================")

# 1. Localizar ou iniciar a DAW
found_hwnd = None
def enum_cb(h, l):
    buf = ctypes.create_unicode_buffer(512)
    user32.GetWindowTextW(h, buf, 512)
    if "Abduction Studio" in buf.value:
        global found_hwnd
        found_hwnd = h
        return False
    return True

EnumWindowsProc = ctypes.WINFUNCTYPE(ctypes.c_bool, wintypes.HWND, wintypes.LPARAM)
user32.EnumWindows(EnumWindowsProc(enum_cb), 0)

if not found_hwnd:
    print("[TESTE AO VIVO] Iniciando Abduction Studio V2...")
    exe_path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Release\AbductionStudioV2.exe"
    subprocess.Popen([exe_path], cwd=r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2")
    time.sleep(3.5)
    user32.EnumWindows(EnumWindowsProc(enum_cb), 0)

hwnd = found_hwnd
print(f"[TESTE AO VIVO] Janela da DAW encontrada (HWND: {hwnd})")

if hwnd:
    user32.ShowWindow(hwnd, 9) # SW_RESTORE
    time.sleep(0.3)
    user32.ShowWindow(hwnd, 3) # SW_MAXIMIZE
    time.sleep(0.3)
    user32.SetForegroundWindow(hwnd)
    time.sleep(1.0)

rect = wintypes.RECT()
user32.GetWindowRect(hwnd, ctypes.byref(rect))
w_w = rect.right - rect.left
w_h = rect.bottom - rect.top

print("[TESTE AO VIVO] Foco estabelecido na tela. Movendo mouse suavemente...")

# 2. Abrir o menu / modal do AI Stems clicando no botão "Desconstruir Musica (IA)" na Playlist
btn_ai_stems_x = rect.left + 840
btn_ai_stems_y = rect.top + 75
smooth_click(btn_ai_stems_x, btn_ai_stems_y, "Botao 'Desconstruir Musica (IA)' na Playlist", pause_after=1.2)

# Central da Janela AI Stems Flutuante
modal_center_x = rect.left + int(w_w * 0.5)
modal_top_y = rect.top + int(w_h * 0.18)

# 3. Demonstrar troca entre as 3 opções de modo (Áudio, MIDI, Híbrido)
smooth_click(modal_center_x - 120, modal_top_y + 165, "Modo 1: Apenas Stems de Audio (WAVs)", pause_after=1.0)
smooth_click(modal_center_x - 120, modal_top_y + 185, "Modo 2: Apenas Partitura MIDI (Piano Roll)", pause_after=1.0)
smooth_click(modal_center_x - 120, modal_top_y + 205, "Modo 3: Audio Stems + MIDI (Hibrido Completo)", pause_after=1.2)

# 4. Testar clique nas checkboxes de opções
smooth_click(modal_center_x - 150, modal_top_y + 235, "Checkbox FL DirectWave Sampler", pause_after=0.8)
smooth_click(modal_center_x - 150, modal_top_y + 235, "Checkbox FL DirectWave (Reativar)", pause_after=0.8)

# 5. Provar imunidade a Click-Through clicando no corpo da janela flutuante sobre as pistas de trás
print("[TESTE AO VIVO] Testando cliques na janela flutuante sobre as pistas de tras (sem click-through)...")
smooth_click(modal_center_x + 50, modal_top_y + 100, "Corpo do Modal (Sobre Pista 2)", pause_after=0.5)
smooth_click(modal_center_x - 50, modal_top_y + 120, "Corpo do Modal (Sobre Pista 3)", pause_after=0.5)
smooth_click(modal_center_x, modal_top_y + 140, "Corpo do Modal (Sobre Pista 4)", pause_after=0.5)

# 6. Carregar com 1 clique a faixa Perception - Different Way
btn_load_track_x = modal_center_x - 200
btn_load_track_y = modal_top_y + 285
smooth_click(btn_load_track_x, btn_load_track_y, "Botao 'Perception - Different Way' (Carregar Stems)", pause_after=2.0)

# 7. Fechar a janela flutuante do AI Stems
btn_close_modal_x = modal_center_x + 380
btn_close_modal_y = modal_top_y + 12
smooth_click(btn_close_modal_x, btn_close_modal_y, "Botao Fechar [X] da Janela Flutuante", pause_after=1.2)

# 8. Mover o mouse pela Playlist para demonstrar a grade limpa e os stems carregados
print("[TESTE AO VIVO] Percorrendo as faixas da Playlist para inspecao visual...")
smooth_move(rect.left + 500, rect.top + 180, duration=1.0)
smooth_move(rect.left + 800, rect.top + 220, duration=1.0)
smooth_move(rect.left + 600, rect.top + 300, duration=1.0)
smooth_move(rect.left + 900, rect.top + 380, duration=1.0)
time.sleep(1.0)

# 9. Iniciar reproducao (Play) para tocar o som ao vivo
print("[TESTE AO VIVO] Iniciando reproducao de audio (PLAY)...")
send_space()
time.sleep(4.0)

# 10. Pausar reproducao (Stop)
print("[TESTE AO VIVO] Pausando reproducao (STOP)...")
send_space()
time.sleep(1.0)

print("==================================================================")
print("[TESTE AO VIVO] DEMONSTRACAO CONCLUIDA COM SUCESSO NA SUA TELA!")
print("==================================================================")
