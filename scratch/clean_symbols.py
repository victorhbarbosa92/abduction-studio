import glob
import os

replacements = {
    '🎛️': ' ICON_FA_SLIDERS ',
    '🎛': ' ICON_FA_SLIDERS ',
    '▾': 'v',
    '🎹': ' ICON_FA_MUSIC ',
    '▶': ' ICON_FA_PLAY ',
    '⏹': ' ICON_FA_STOP ',
    '⏺': ' ICON_FA_CIRCLE ',
    '☁️': ' ICON_FA_CLOUD ',
    '☁': ' ICON_FA_CLOUD ',
    '🔊': ' ICON_FA_VOLUME_HIGH ',
    '📁': ' ICON_FA_FOLDER ',
    '✂️': ' ICON_FA_SCISSORS ',
    '✂': ' ICON_FA_SCISSORS ',
    '🎨': ' ICON_FA_PALETTE ',
    '🤖': ' ICON_FA_ROBOT ',
    '❓': ' ICON_FA_QUESTION ',
    '👽': ' ICON_FA_WAND_MAGIC_SPARKLES ',
    '🛒': ' ICON_FA_CART_SHOPPING ',
    '💎': ' ICON_FA_GEM ',
    '⚡': ' ICON_FA_BOLT ',
    '🚀': ' ICON_FA_ROCKET ',
    '🌌': ' ICON_FA_WAVE_SQUARE ',
    '🌀': ' ICON_FA_COMPASS ',
    '🧩': ' ICON_FA_PLUG ',
    '🎚️': ' ICON_FA_SLIDERS ',
    '🎚': ' ICON_FA_SLIDERS ',
    '🥁': ' ICON_FA_DRUM ',
    '🎲': ' ICON_FA_DICE ',
    '📈': ' ICON_FA_CHART_LINE ',
    '📉': ' ICON_FA_ARROW_TREND_DOWN ',
    '🌊': ' ICON_FA_WAVE_SQUARE ',
    '🔍': ' ICON_FA_MAGNIFYING_GLASS ',
    '⌛': ' ICON_FA_HOURGLASS_HALF ',
    '✅': ' ICON_FA_CHECK ',
    '✔': ' ICON_FA_CHECK ',
    '✓': ' ICON_FA_CHECK ',
    '🗑': ' ICON_FA_TRASH ',
    '👁': ' ICON_FA_EYE ',
    '✏': ' ICON_FA_PENCIL ',
    '🖌': ' ICON_FA_BRUSH ',
    '🧹': ' ICON_FA_BROOM ',
    '↖': ' ICON_FA_ARROW_POINTER ',
    '🖊': ' ICON_FA_PEN ',
    '🚩': ' ICON_FA_FLAG ',
    '⭐': ' ICON_FA_STAR ',
    '⏱': ' ICON_FA_STOPWATCH ',
    '🎧': ' ICON_FA_HEADPHONES ',
    '🧬': ' ICON_FA_DNAS ',
    '🎻': ' ICON_FA_VIOLIN ',
    '🎸': ' ICON_FA_GUITAR ',
    '🛸': ' ICON_FA_ROCKET ',
    '👄': ' ICON_FA_COMMENT ',
    '🎤': ' ICON_FA_MICROPHONE ',
    '📊': ' ICON_FA_CHART_BAR ',
    '➕': '+',
    '🛐': ' ICON_FA_HANDS_PRRAYING ',
    '💥': ' ICON_FA_EXPLOSION ',
    '💾': ' ICON_FA_FLOPPY_DISK ',
    '📻': ' ICON_FA_RADIO ',
    '🪕': ' ICON_FA_MUSIC ',
    '🎵': ' ICON_FA_MUSIC ',
    '←': '<-',
    '─': '-',
    '—': '-',
    '×': 'x'
}

all_files = glob.glob(r'src/**/*.h', recursive=True) + glob.glob(r'src/**/*.cpp', recursive=True)
for filepath in all_files:
    if 'IconsFontAwesome6.h' in filepath or 'dr_wav.h' in filepath or 'miniz.' in filepath:
        continue
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    modified = False
    for k, v in replacements.items():
        if k in content:
            content = content.replace(k, v)
            modified = True
            
    if modified:
        if 'IconsFontAwesome6.h' not in content and ('ui/' in filepath.replace('\\', '/')):
            content = '#include "IconsFontAwesome6.h"\n' + content
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f'Cleaned symbols in {os.path.basename(filepath)}')
