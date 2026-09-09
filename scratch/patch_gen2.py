with open('scratch/generate_playlist_ui.py', 'r', encoding='utf-8') as f:
    code = f.read()

old_stems = "const std::vector<float>* p_stem_buf = (::g_ai_engine && t < MAX_TRACKS) ? &::g_ai_engine->stems_buffers[t] : nullptr;"
new_stems = "const std::vector<float>* p_stem_buf = (::g_ai_engine && t < MAX_TRACKS) ? &::g_ai_engine->getStemBuffer(t) : nullptr;"

assert old_stems in code, "old_stems not found"
code = code.replace(old_stems, new_stems)

with open('scratch/generate_playlist_ui.py', 'w', encoding='utf-8') as f:
    f.write(code)

print("scratch/generate_playlist_ui.py updated with getStemBuffer.")
