import os
import sys
import time
import urllib.request
from urllib.error import URLError, HTTPError

MODELS = [
    "htdemucs_ft_drums.onnx",
    "htdemucs_ft_bass.onnx",
    "htdemucs_ft_other.onnx",
    "htdemucs_ft_vocals.onnx"
]

BASE_URL = "https://huggingface.co/StemSplitio/htdemucs-ft-onnx/resolve/main/"
TARGET_DIR = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\build\Debug"

def download_with_retry(url, dest_path, max_retries=5):
    retries = 0
    while retries < max_retries:
        try:
            req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
            
            # Support resuming
            existing_size = 0
            if os.path.exists(dest_path):
                existing_size = os.path.getsize(dest_path)
            
            # First, get file info
            head_req = urllib.request.Request(url, method='HEAD', headers={'User-Agent': 'Mozilla/5.0'})
            with urllib.request.urlopen(head_req, timeout=10) as response:
                total_size = int(response.headers.get('Content-Length', 0))
                
            if total_size > 0 and existing_size == total_size:
                print(f"[{dest_path}] Already fully downloaded ({total_size} bytes).")
                return True
                
            if existing_size > 0 and total_size > 0:
                print(f"[{dest_path}] Resuming from {existing_size}/{total_size} bytes...")
                req.add_header('Range', f'bytes={existing_size}-')
                mode = 'ab'
            else:
                print(f"[{dest_path}] Starting new download ({total_size} bytes)...")
                mode = 'wb'
                existing_size = 0
                
            with urllib.request.urlopen(req, timeout=15) as response:
                with open(dest_path, mode) as f:
                    downloaded = existing_size
                    while True:
                        chunk = response.read(8192 * 8)
                        if not chunk:
                            break
                        f.write(chunk)
                        downloaded += len(chunk)
                        if total_size > 0 and downloaded % (1024*1024 * 10) < (8192*8): # Print every ~10MB
                            print(f"[{dest_path}] {downloaded / 1024 / 1024:.1f} MB / {total_size / 1024 / 1024:.1f} MB ({(downloaded/total_size)*100:.1f}%)")
            
            # Verify size
            if total_size > 0 and os.path.getsize(dest_path) != total_size:
                print(f"[{dest_path}] Size mismatch. Retrying...")
                retries += 1
                time.sleep(2)
                continue
                
            print(f"[{dest_path}] Download complete!")
            return True
            
        except HTTPError as e:
            if e.code == 416: # Range Not Satisfiable (already fully downloaded or server doesn't support)
                print(f"[{dest_path}] Range error. Restarting from scratch...")
                if os.path.exists(dest_path):
                    os.remove(dest_path)
                retries += 1
                time.sleep(2)
            else:
                print(f"HTTP Error: {e.code}. Retrying...")
                retries += 1
                time.sleep(3)
        except Exception as e:
            print(f"Error: {e}. Retrying ({retries+1}/{max_retries})...")
            retries += 1
            time.sleep(3)
            
    print(f"Failed to download {url} after {max_retries} retries.")
    return False

def main():
    if not os.path.exists(TARGET_DIR):
        os.makedirs(TARGET_DIR, exist_ok=True)
        
    for model in MODELS:
        url = BASE_URL + model
        dest = os.path.join(TARGET_DIR, model)
        print(f"\n--- Downloading {model} ---")
        success = download_with_retry(url, dest)
        if not success:
            sys.exit(1)
            
    print("\nAll models downloaded successfully!")

if __name__ == '__main__':
    main()
