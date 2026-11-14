import os

file_path = "src/ui/KuroDJStudioUI.h"
with open(file_path, "r", encoding="utf-8") as f:
    content = f.read()

# 1. Add loadFollowedArtists() and saveFollowedArtists() around line 308
target_followed = """        std::vector<std::string> followed_artists;
        bool is_following_selected_artist = false;"""

replacement_followed = """        std::vector<std::string> followed_artists;
        bool is_following_selected_artist = false;

        void loadFollowedArtists() {
            auto path = KuroAudio::getTracksDirectory() / "followed_artists.json";
            std::error_code ec;
            if (!std::filesystem::exists(path, ec)) {
                // Se o arquivo nao existir, inicializa com os artistas padrao
                followed_artists = { "JOX (BR)", "Astrix", "Aura Vortex", "Blazy" };
                saveFollowedArtists();
                return;
            }
            std::ifstream f(path);
            if (!f.is_open()) return;
            std::string text((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            followed_artists.clear();
            size_t pos = 0;
            while ((pos = text.find('\"', pos)) != std::string::npos) {
                size_t vend = text.find('\"', pos + 1);
                if (vend == std::string::npos) break;
                std::string item = text.substr(pos + 1, vend - pos - 1);
                pos = vend + 1;
                if (!item.empty() && item != "artists" && item != "[" && item != "]" && item != ",") {
                    if (std::find(followed_artists.begin(), followed_artists.end(), item) == followed_artists.end()) {
                        followed_artists.push_back(item);
                    }
                }
            }
        }

        void saveFollowedArtists() {
            auto path = KuroAudio::getTracksDirectory() / "followed_artists.json";
            std::ofstream f(path);
            if (!f.is_open()) return;
            f << "[\n";
            for (size_t i = 0; i < followed_artists.size(); i++) {
                f << "  \"" << followed_artists[i] << "\"";
                if (i + 1 < followed_artists.size()) f << ",";
                f << "\n";
            }
            f << "]\n";
        }"""

if target_followed not in content:
    print("target_followed not found!")
    exit(1)
content = content.replace(target_followed, replacement_followed, 1)

# 2. In init(), call loadFollowedArtists()
target_init = """            // Carrega Hot Cues salvos anteriormente
            if (engine) engine->loadHotCues();"""

replacement_init = """            // Carrega Hot Cues salvos anteriormente
            if (engine) engine->loadHotCues();

            // Etapa 3: Carrega a base local de artistas seguidos no Radar
            loadFollowedArtists();"""

if target_init not in content:
    print("target_init not found!")
    exit(1)
content = content.replace(target_init, replacement_init, 1)

# 3. In openSourceMatrix, call loadFollowedArtists()
target_open1 = """            is_following_selected_artist = false;
            for (const auto& a : followed_artists) {
                if (a == trk.artist) { is_following_selected_artist = true; break; }
            }"""

replacement_open1 = """            loadFollowedArtists();
            is_following_selected_artist = false;
            for (const auto& a : followed_artists) {
                if (a == trk.artist) { is_following_selected_artist = true; break; }
            }"""

content = content.replace(target_open1, replacement_open1, 1)

# 4. In renderMasterSourceMatrixCard, saveFollowedArtists when clicking follow/unfollow
target_btn_follow = """                    if (ImGui::SmallButton(ICON_FA_CHECK " SEGUINDO ARTISTA")) {
                        is_following_selected_artist = false;
                        followed_artists.erase(std::remove(followed_artists.begin(), followed_artists.end(), selected_source_track_artist), followed_artists.end());
                    }
                    ImGui::PopStyleColor(3);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.18f, 0.28f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.65f, 0.85f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.92f, 1.00f, 1.0f));
                    if (ImGui::SmallButton(ICON_FA_PLUS " SEGUIR ARTISTA")) {
                        is_following_selected_artist = true;
                        followed_artists.push_back(selected_source_track_artist);
                    }"""

replacement_btn_follow = """                    if (ImGui::SmallButton(ICON_FA_CHECK " SEGUINDO ARTISTA")) {
                        is_following_selected_artist = false;
                        followed_artists.erase(std::remove(followed_artists.begin(), followed_artists.end(), selected_source_track_artist), followed_artists.end());
                        saveFollowedArtists();
                    }
                    ImGui::PopStyleColor(3);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.18f, 0.28f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.65f, 0.85f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.92f, 1.00f, 1.0f));
                    if (ImGui::SmallButton(ICON_FA_PLUS " SEGUIR ARTISTA")) {
                        is_following_selected_artist = true;
                        if (std::find(followed_artists.begin(), followed_artists.end(), selected_source_track_artist) == followed_artists.end()) {
                            followed_artists.push_back(selected_source_track_artist);
                        }
                        saveFollowedArtists();
                    }"""

if target_btn_follow not in content:
    print("target_btn_follow not found!")
    exit(1)
content = content.replace(target_btn_follow, replacement_btn_follow, 1)

# Write to temporary file and replace
tmp_path = file_path + ".tmp"
with open(tmp_path, "w", encoding="utf-8") as f:
    f.write(content)

os.system(f'cmd /c move /y "{tmp_path.replace("/", chr(92))}" "{file_path.replace("/", chr(92))}"')
print("Part 1 of Etapa 3 applied successfully!")
