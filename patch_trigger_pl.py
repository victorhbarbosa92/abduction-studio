import os

file_path = r'src/ui/StudioUI.h'
with open(file_path, 'r', encoding='utf-8') as f:
    content = f.read()

target = '        // Trigger de arquivo para testes e automação do Piano Roll'
replacement = '''        // Trigger de arquivo para testes e automação da Playlist / Arranger
        std::filesystem::path trigger_pl = std::filesystem::temp_directory_path() / "abduction_playlist_cmd.txt";
        if (std::filesystem::exists(trigger_pl)) {
            std::string content = "";
            std::ifstream ifs(trigger_pl);
            if (ifs) ifs >> content;
            ifs.close();
            std::error_code ec;
            std::filesystem::remove(trigger_pl, ec);
            if (content == "open") {
                show_playlist = true;
            } else if (content == "close") {
                show_playlist = false;
            } else if (content == "toggle") {
                show_playlist = !show_playlist;
            }
        }

        // Trigger de arquivo para testes e automação do Piano Roll'''

if target in content:
    content = content.replace(target, replacement, 1)
    tmp_path = file_path + '.tmp'
    with open(tmp_path, 'w', encoding='utf-8') as f:
        f.write(content)
    os.replace(tmp_path, file_path)
    print("SUCCESS: StudioUI.h updated with trigger_pl")
else:
    print("WARNING: target not found in StudioUI.h")
