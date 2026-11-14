import sys

with open('src/ui/KuroDJStudioUI.h', 'r', encoding='utf-8', errors='ignore') as f:
    text = f.read()

pt1 = 'int waveform_color_mode = 0;'
pt2 = 'capsuleBtn("##TopCapsulePads"'
pt3 = 'void renderGridEditDrawer(KuroAudio::DJEngine& engine, float w) {'
pt4 = 'void renderStackedRekordboxWaveforms(KuroAudio::DJEngine& engine, float w, float h) {'

print('pt1 (waveform_color_mode):', pt1 in text)
print('pt2 (TopCapsulePads):', pt2 in text)
print('pt3 (renderGridEditDrawer):', pt3 in text)
print('pt4 (renderStackedRekordboxWaveforms):', pt4 in text)
