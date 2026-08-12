import glob
import re
import os

files = glob.glob(r'src/**/*.h', recursive=True) + glob.glob(r'src/**/*.cpp', recursive=True)

# Lista de todos os nomes de macros ICON_FA_... conhecidos
fa_macro_names = [
    'ICON_FA_SLIDERS', 'ICON_FA_MUSIC', 'ICON_FA_PLAY', 'ICON_FA_STOP', 'ICON_FA_CIRCLE',
    'ICON_FA_CLOUD', 'ICON_FA_VOLUME_HIGH', 'ICON_FA_FOLDER', 'ICON_FA_SCISSORS',
    'ICON_FA_PALETTE', 'ICON_FA_ROBOT', 'ICON_FA_QUESTION', 'ICON_FA_WAND_MAGIC_SPARKLES',
    'ICON_FA_CART_SHOPPING', 'ICON_FA_GEM', 'ICON_FA_BOLT', 'ICON_FA_ROCKET',
    'ICON_FA_WAVE_SQUARE', 'ICON_FA_COMPASS', 'ICON_FA_PLUG', 'ICON_FA_DRUM', 'ICON_FA_DICE',
    'ICON_FA_CHART_LINE', 'ICON_FA_ARROW_TREND_DOWN', 'ICON_FA_MAGNIFYING_GLASS',
    'ICON_FA_HOURGLASS_HALF', 'ICON_FA_CHECK', 'ICON_FA_TRASH', 'ICON_FA_EYE', 'ICON_FA_PENCIL',
    'ICON_FA_BRUSH', 'ICON_FA_BROOM', 'ICON_FA_ARROW_POINTER', 'ICON_FA_PEN', 'ICON_FA_FLAG',
    'ICON_FA_STAR', 'ICON_FA_STOPWATCH', 'ICON_FA_HEADPHONES', 'ICON_FA_DNAS', 'ICON_FA_VIOLIN',
    'ICON_FA_GUITAR', 'ICON_FA_COMMENT', 'ICON_FA_MICROPHONE', 'ICON_FA_CHART_BAR',
    'ICON_FA_HANDS_PRRAYING', 'ICON_FA_EXPLOSION', 'ICON_FA_FLOPPY_DISK', 'ICON_FA_RADIO',
    'ICON_FA_FOLDER_OPEN', 'ICON_FA_DIAGRAM_PROJECT', 'ICON_FA_WRENCH'
]

# Regex para encontrar " ... ICON_FA_SOMETHING ... " dentro de aspas duplas
# Exemplo: " ICON_FA_SLIDERS  Mixer Panel" -> " " ICON_FA_SLIDERS " Mixer Panel"

pattern = re.compile(r'"([^"]*?)(ICON_FA_[A_Z0-9_]+)([^"]*?)"')

for filepath in files:
    if 'IconsFontAwesome6.h' in filepath or 'dr_wav.h' in filepath:
        continue
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    original = content
    
    # Processa linha por linha para substituir tokens ICON_FA_ dentro de strings
    new_lines = []
    for line in content.splitlines(True):
        # Encontra todas as aspas duplas da linha
        # Sub-substituição cuidadosa
        for macro in fa_macro_names:
            # Se a macro está dentro de aspas ex: "   ICON_FA_DRUM  Kicks"
            # Vamos substituir por "   " ICON_FA_DRUM "  Kicks"
            # Tratando casos onde está no início, meio ou fim da string
            line = re.sub(r'"([^"]*?)\b' + macro + r'\b([^"]*?)"', r'"\1" ' + macro + r' "\2"', line)
        
        # Limpa strings vazias produzidas por concatenação ex: "" ICON_FA_XYZ " text" -> ICON_FA_XYZ " text"
        line = line.replace('"" ', '').replace(' ""', '')
        new_lines.append(line)

    new_content = ''.join(new_lines)
    if new_content != original:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f'Successfully fixed FontAwesome macro string concatenation in {os.path.basename(filepath)}')
