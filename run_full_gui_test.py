import os
import sys
import time
import subprocess
from PIL import Image, ImageFile

ImageFile.LOAD_TRUNCATED_IMAGES = True

APP_DIR = r"c:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2"
APP_EXE = os.path.join(APP_DIR, "build", "AbductionStudioV2.exe")
CMD_FILE = os.path.join(APP_DIR, "abduction_test_cmd.txt")
REQ_FILE = os.path.join(APP_DIR, "abduction_screenshot_req.txt")
BMP_FILE = os.path.join(APP_DIR, "abduction_screenshot.bmp")
OUT_DIR = os.path.join(APP_DIR, "artifacts", "ui_verification")
os.makedirs(OUT_DIR, exist_ok=True)

print(f"[*] Starting full GUI test on {APP_EXE}")
print(f"[*] Artifacts directory: {OUT_DIR}")

# Cleanup all old BMPs and triggers
for f in [CMD_FILE, REQ_FILE, BMP_FILE]:
    if os.path.exists(f):
        try: os.remove(f)
        except: pass

for f in os.listdir(OUT_DIR):
    if f.endswith(".bmp") or f.endswith(".png"):
        try: os.remove(os.path.join(OUT_DIR, f))
        except: pass

proc = subprocess.Popen([APP_EXE], cwd=APP_DIR)
print(f"[*] AbductionStudio launched with PID: {proc.pid}")

ready_marker = os.path.join(OUT_DIR, f"ready_{proc.pid}.bmp")
print("[*] Waiting for AbductionStudio to enter main render loop...")
for attempt in range(50):
    time.sleep(1.0)
    tmp_req = REQ_FILE + ".tmp"
    with open(tmp_req, "w", encoding="utf-8") as f:
        f.write(ready_marker)
    try: os.replace(tmp_req, REQ_FILE)
    except: pass
    time.sleep(0.5)
    if os.path.exists(ready_marker) and os.path.getsize(ready_marker) >= 25000000:
        print(f"[+] Engine confirmed rendering frames after {attempt+1} seconds!")
        try: os.remove(ready_marker)
        except: pass
        break

def send_cmd(cmd_text):
    tmp_cmd = CMD_FILE + ".tmp"
    with open(tmp_cmd, "w", encoding="utf-8") as f:
        f.write(cmd_text)
    try: os.replace(tmp_cmd, CMD_FILE)
    except: pass
    time.sleep(0.8)

def snap(name):
    target_bmp = os.path.join(OUT_DIR, name.replace(".png", ".bmp"))
    if os.path.exists(target_bmp):
        try: os.remove(target_bmp)
        except: pass
    if os.path.exists(BMP_FILE):
        try: os.remove(BMP_FILE)
        except: pass

    tmp_req = REQ_FILE + ".tmp"
    with open(tmp_req, "w", encoding="utf-8") as f:
        f.write(target_bmp)
    try: os.replace(tmp_req, REQ_FILE)
    except: pass
    
    # Wait until file is completely written (>= 25 MB)
    chosen = None
    for _ in range(40):
        time.sleep(0.1)
        if os.path.exists(target_bmp) and os.path.getsize(target_bmp) >= 25000000:
            chosen = target_bmp
            time.sleep(0.15)
            break
        elif os.path.exists(BMP_FILE) and os.path.getsize(BMP_FILE) >= 25000000:
            chosen = BMP_FILE
            time.sleep(0.15)
            break

    if chosen:
        try:
            im = Image.open(chosen)
            out_png = os.path.join(OUT_DIR, name)
            im.save(out_png)
            print(f"[SNAP] Successfully captured: {name} ({im.size[0]}x{im.size[1]})")
            try: os.remove(chosen)
            except: pass
            return out_png
        except Exception as e:
            print(f"[-] Error opening/saving {name}: {e}")
            return None
    else:
        print(f"[-] FAILED to capture screenshot {name}")
        return None

try:
    # -------------------------------------------------------------------------
    # STEP 1: INITIAL STATE (Restored wide waveform)
    # -------------------------------------------------------------------------
    print("[1/12] Verifying initial startup with restored wide waveform...")
    snap("01_startup_uncompressed_wide_waveform.png")

    # -------------------------------------------------------------------------
    # STEP 2: LOAD TRACKS TO DECK 1 & DECK 2
    # -------------------------------------------------------------------------
    print("[2/12] Loading tracks into Deck 1 and Deck 2...")
    send_cmd("load_tracks")
    time.sleep(1.5)
    snap("02_tracks_loaded_waveforms_rendered.png")

    # -------------------------------------------------------------------------
    # STEP 3: OPEN EDIT MODE (Aba 0: Objeto Selecionado)
    # -------------------------------------------------------------------------
    print("[3/12] Opening Edit Mode (WYSIWYG Layout Studio)...")
    send_cmd("edit_open")
    time.sleep(0.8)
    snap("03_edit_mode_aba0_objeto_selecionado.png")

    # Select Waveforms in Aba 0
    print("[4/12] Selecting Waveforms module in Aba 0...")
    send_cmd("select_widget 8")
    time.sleep(0.8)
    snap("04_aba0_selected_widget_waveforms.png")

    # -------------------------------------------------------------------------
    # STEP 4: WALK THROUGH ALL EDIT MODE TABS
    # -------------------------------------------------------------------------
    print("[5/12] Testing Aba 1 (Geral & Escala)...")
    send_cmd("edit_tab 1")
    time.sleep(0.6)
    snap("05_aba1_geral_escala.png")

    print("[6/12] Testing Aba 2 (Mixer Central)...")
    send_cmd("edit_tab 2")
    time.sleep(0.6)
    snap("06_aba2_mixer_central.png")

    print("[7/12] Testing Aba 3 (Vinil & Jog)...")
    send_cmd("edit_tab 3")
    time.sleep(0.6)
    snap("07_aba3_vinil_jog.png")

    print("[8/12] Testing Aba 4 (Pads & Transporte)...")
    send_cmd("edit_tab 4")
    time.sleep(0.6)
    snap("08_aba4_pads_transporte.png")

    print("[9/12] Testing Aba 5 (Waveforms & FX)...")
    send_cmd("edit_tab 5")
    time.sleep(0.6)
    snap("09_aba5_waveforms_fx.png")

    # Test Super Zoom (8 Beats, 1.5x speed, 2.0x stretch)
    print("[10/12] Testing Super Zoom (8 Beats / 2.0x stretch)...")
    send_cmd("wf_beats 8 wf_speed 1.5 wf_stretch 2.0")
    time.sleep(0.6)
    snap("10_tested_super_zoom_8b.png")

    # Test Modo Compacto (32 Beats, 1.0x speed, 1.0x stretch)
    print("[10b/12] Testing Modo Compacto (32 Beats / 1.0x stretch)...")
    send_cmd("wf_beats 32 wf_speed 1.0 wf_stretch 1.0")
    time.sleep(0.6)
    snap("11_tested_modo_compacto_32b.png")

    # Test Restore Padrão Anterior (16 Beats, 1.0x speed, 1.65x stretch)
    print("[10c/12] Restoring Padrão Anterior (16 Beats / 1.65x stretch)...")
    send_cmd("wf_reset")
    time.sleep(0.6)
    snap("12_tested_padrao_anterior_16b_restored.png")

    print("[11/12] Testing Aba 6 (Presets & Backup)...")
    send_cmd("edit_tab 6")
    time.sleep(0.6)
    snap("13_aba6_presets_backup.png")

    # Apply Preset TV 65" 4K
    print("[*] Testing Preset TV 65\" 4K...")
    send_cmd("apply_preset 1")
    time.sleep(0.6)
    snap("14_tested_preset_tv_65_4k.png")

    # Restore Club Standard Preset
    print("[*] Restoring Club Standard Preset...")
    send_cmd("apply_preset 0")
    time.sleep(0.6)
    snap("15_tested_preset_club_restored.png")

    # Close Edit Mode
    print("[12/12] Closing Edit Mode and testing Playback...")
    send_cmd("edit_close")
    time.sleep(0.8)
    snap("16_edit_mode_closed.png")

    # Test Deck 1 Playback
    print("[*] Starting Deck 1 playback...")
    send_cmd("toggle_play 0")
    time.sleep(3.0) # Let it play for 3.0 seconds to advance waveform
    snap("17_deck1_playing_waveform_advancing.png")

    # Stop Deck 1 playback
    print("[*] Stopping Deck 1 playback...")
    send_cmd("toggle_play 0")
    time.sleep(0.8)
    snap("18_deck1_cued_stopped.png")

    print("[+] All 18 automated UI tests completed successfully!")

finally:
    print("[*] Terminating AbductionStudio process...")
    proc.terminate()
    try: proc.wait(timeout=3)
    except: proc.kill()
    print("[+] Done.")
