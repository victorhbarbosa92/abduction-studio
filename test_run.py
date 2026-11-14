import subprocess
import time

proc = subprocess.Popen(["./AbductionStudioV2.exe", "--test-deck-play"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
for i in range(20):
    time.sleep(1)
    ret = proc.poll()
    print(f"Sec {i+1}: return code = {ret}")
    if ret is not None:
        stdout, stderr = proc.communicate()
        print("CRASHED / EXITED!")
        print("STDOUT:\n", stdout.decode('latin1', errors='replace'))
        print("STDERR:\n", stderr.decode('latin1', errors='replace'))
        break

if ret is None:
    print("Process still running after 20 seconds. Terminating and reading pipes...")
    proc.terminate()
    try:
        stdout, stderr = proc.communicate(timeout=3)
    except:
        proc.kill()
        stdout, stderr = proc.communicate()
    print("STDOUT:\n", stdout.decode('latin1', errors='replace'))
    print("STDERR:\n", stderr.decode('latin1', errors='replace'))
