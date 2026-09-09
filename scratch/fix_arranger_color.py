with open('src/core/PsySongArranger.h', 'r', encoding='utf-8') as f:
    text = f.read()

text = text.replace('ac.color = col;', '// ac.color = col;')

with open('src/core/PsySongArranger.h', 'w', encoding='utf-8') as f:
    f.write(text)

print('Updated PsySongArranger.h: removed ac.color = col;')
