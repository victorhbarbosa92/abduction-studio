import subprocess
import time
import ctypes
import os

user32 = ctypes.windll.user32

proc = subprocess.Popen(["./AbductionStudioV2.exe"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
time.sleep(3)

# Find window
hwnd = user32.FindWindowA(None, b"Abduction Studio V4.0 - Flagship Edition")
print("Window handle:", hwnd)

if hwnd:
    user32.SetForegroundWindow(hwnd)
    time.sleep(0.5)
    # Send F11 key (VK_F11 = 0x7A)
    VK_F11 = 0x7A
    user32.keybd_event(VK_F11, 0, 0, 0) # key down
    time.sleep(0.1)
    user32.keybd_event(VK_F11, 0, 2, 0) # key up
    print("Sent F11 key to window!")

for i in range(10):
    time.sleep(1)
    ret = proc.poll()
    print(f"Sec {i+1} after F11: ret = {ret}")
    if ret is not None:
        stdout, stderr = proc.communicate()
        print("APP CRASHED OR CLOSED!")
        print("Return code:", ret)
        print("STDOUT:\n", stdout.decode('latin1', errors='replace')[-1500:])
        print("STDERR:\n", stderr.decode('latin1', errors='replace')[-1500:])
        break

if ret is None:
    print("App is still running normally.")
    proc.terminate()
    try: proc.wait(timeout=2)
    except: proc.kill()
