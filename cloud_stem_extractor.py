import os
import sys
import time
import subprocess

def process_file_fast(input_wav, output_dir):
    os.makedirs(output_dir, exist_ok=True)
    basename = os.path.splitext(os.path.basename(input_wav))[0]
    
    print(f"[STEM-EXTRACTOR] Processando '{input_wav}' para separação de stems 4-Band...")
    
    # Executa a separação através do edm_stem_extractor local ultra-rápido
    script_dir = os.path.dirname(os.path.abspath(__file__))
    edm_script = os.path.join(script_dir, "edm_stem_extractor.py")
    
    if os.path.exists(edm_script):
        cmd = [sys.executable, edm_script, input_wav, output_dir]
        ret = subprocess.call(cmd)
        return ret == 0
    else:
        print("[STEM-EXTRACTOR ERROR] Script edm_stem_extractor.py não encontrado.")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Uso: python cloud_stem_extractor.py <input_audio.wav> <output_dir>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    out_dir = sys.argv[2]
    success = process_file_fast(input_file, out_dir)
    sys.exit(0 if success else 1)
