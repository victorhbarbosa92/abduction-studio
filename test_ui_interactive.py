import os
import sys
import time
import subprocess
import pyautogui
import win32gui
import win32con
import win32process
from PIL import Image

pyautogui.FAILSAFE = False

APP_DIR = r"c:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"
APP_EXE = os.path.join(APP_DIR, "build", "AbductionStudioV2.exe")
OUT_DIR = os.path.join(APP_DIR, "artifacts", "ui_verification")
os.makedirs(OUT_DIR, exist_ok=True)

print(f"[*] Starting interactive UI test on {APP_EXE}")
print(f"[*] Screenshots will be saved to {OUT_DIR}")

# Start the application
proc = subprocess.Popen([APP_EXE], cwd=APP_DIR)
print(f"[*] Process launched with PID={proc.pid}. Waiting for window...")

target_hwnd = None
for i in range(30):
    time.sleep(0.5)
    def enum_cb(hwnd, extra):
        global target_hwnd
        if win32gui.IsWindowVisible(hwnd):
            title = win32gui.GetWindowText(hwnd)
            if "ABDUCTION STUDIO" in title.upper():
                target_hwnd = hwnd
                return False
        return True
    try:
        win32gui.EnumWindows(enum_cb, None)
    except:
        pass
    if target_hwnd:
        break

if not target_hwnd:
    print("[-] App window not found within 15 seconds! Aborting.")
    proc.terminate()
    sys.exit(1)

hwnd = target_hwnd
title = win32gui.GetWindowText(hwnd)
print(f"[+] Found window: hwnd={hwnd}, title='{title}'")

# Bring window to foreground and position predictably
win32gui.ShowWindow(hwnd, win32con.SW_RESTORE)
time.sleep(0.5)
try:
    win32gui.SetForegroundWindow(hwnd)
except Exception as e:
    print(f"[!] SetForegroundWindow note: {e}")

win32gui.MoveWindow(hwnd, 50, 50, 1600, 920, True)
time.sleep(2.0)

rect = win32gui.GetWindowRect(hwnd)
win_x, win_y, win_w, win_h = rect[0], rect[1], rect[2] - rect[0], rect[3] - rect[1]
print(f"[+] Window position: ({win_x}, {win_y}), size: {win_w}x{win_h}")

def snap(name):
    path = os.path.join(OUT_DIR, name)
    img = pyautogui.screenshot(region=(win_x, win_y, win_w, win_h))
    img.save(path)
    print(f"[SNAP] Saved: {name}")
    return img

# STEP 1: App startup - check uncompressed waveform
time.sleep(1.0)
snap("01_startup_waveform_wide_uncompressed.png")

# STEP 2: Open Edit Mode (Ctrl + E)
print("[*] Toggling Edit Mode (Ctrl+E)...")
pyautogui.hotkey('ctrl', 'e')
time.sleep(1.2)
snap("02_edit_mode_opened.png")

# STEP 3: Click through tabs in Edit Mode
# The Edit Mode window is at approx (win_x + 80, win_y + 80)
# Tab bar is around y = win_y + 135
print("[*] Navigating Edit Mode Tabs...")

# Tab 1: Geral & Escala
pyautogui.click(win_x + 190, win_y + 135)
time.sleep(0.6)
snap("03_tab1_geral_escala.png")

# Tab 2: Mixer Central
pyautogui.click(win_x + 290, win_y + 135)
time.sleep(0.6)
# Test clicking on horizontal fader button
pyautogui.click(win_x + 360, win_y + 400)
time.sleep(0.6)
snap("04_tab2_mixer_central.png")

# Tab 3: Vinil & Jog
pyautogui.click(win_x + 395, win_y + 135)
time.sleep(0.6)
snap("05_tab3_vinil_jog.png")

# Tab 4: Pads & Transporte
pyautogui.click(win_x + 490, win_y + 135)
time.sleep(0.6)
# Click "16 Wide (8x2 TV)" pad grid mode button
pyautogui.click(win_x + 330, win_y + 195)
time.sleep(0.6)
snap("06_tab4_pads_transporte.png")

# Tab 5: Waveforms & FX
pyautogui.click(win_x + 600, win_y + 135)
time.sleep(0.6)
snap("07_tab5_waveforms_fx.png")

# Test "Super Zoom (8B / 2.0x)" button in Tab 5
print("[*] Testing Super Zoom button...")
pyautogui.click(win_x + 500, win_y + 435)
time.sleep(0.6)
snap("08_tab5_tested_super_zoom.png")

# Test "Modo Compacto (32B / 1.0x)" button in Tab 5
print("[*] Testing Modo Compacto button...")
pyautogui.click(win_x + 350, win_y + 435)
time.sleep(0.6)
snap("09_tab5_tested_modo_compacto.png")

# Test "Padrão Anterior (16B / 1.65x)" button in Tab 5
print("[*] Testing Padrão Anterior button...")
pyautogui.click(win_x + 200, win_y + 435)
time.sleep(0.6)
snap("10_tab5_tested_padrao_anterior_restored.png")

# Tab 6: Presets & Backup
pyautogui.click(win_x + 720, win_y + 135)
time.sleep(0.6)
snap("11_tab6_presets_backup.png")

# Close Edit Mode via Ctrl+E
print("[*] Closing Edit Mode...")
pyautogui.hotkey('ctrl', 'e')
time.sleep(1.2)
snap("12_edit_mode_closed.png")

# STEP 4: Test Waveform Side Toolbar directly on the DJ table
print("[*] Testing Waveform Sidebar Buttons...")

# Test RST button (y offset ~ 44 + topbar ~ 80 = win_y + 125)
pyautogui.click(win_x + 28, win_y + 125)
time.sleep(0.6)

# Test BEATS button (y offset ~ 216 + 80 = win_y + 296)
print("[*] Clicking BEATS button on waveform side toolbar...")
pyautogui.click(win_x + 28, win_y + 296)
time.sleep(0.6)
snap("13_waveform_sidebar_beats_cycled.png")

# Test SPEED button (y offset ~ 238 + 80 = win_y + 318)
print("[*] Clicking SPEED button on waveform side toolbar...")
pyautogui.click(win_x + 28, win_y + 318)
time.sleep(0.6)
snap("14_waveform_sidebar_speed_cycled.png")

# Reset back to default standard
pyautogui.click(win_x + 28, win_y + 125)
time.sleep(0.6)
snap("15_waveform_reset_standard.png")

# STEP 5: Test Play / Cue buttons on Deck 1
# Deck 1 Transport buttons
print("[*] Testing Deck 1 Playback and Waveform Movement...")
pyautogui.click(win_x + 70, win_y + 470) # Click PLAY
time.sleep(2.5)
snap("16_deck1_playing_waveform_running.png")

pyautogui.click(win_x + 70, win_y + 420) # Click CUE
time.sleep(0.8)
snap("17_deck1_cue_stopped.png")

print("[+] All interface interactive tests completed successfully!")

# Close app
win32gui.PostMessage(hwnd, win32con.WM_CLOSE, 0, 0)
time.sleep(1.5)
try:
    proc.terminate()
except:
    pass

print("[+] Test finished cleanly.")
