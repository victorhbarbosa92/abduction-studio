import os

file_path = "src/core/SpotifyDJService.h"
with open(file_path, "r", encoding="utf-8") as f:
    content = f.read()

target = """        void searchSourcesAsync(const std::string& track_id, const std::string& title, const std::string& artist) {
            if (is_searching_sources.load()) return;
            is_searching_sources = true;
            show_sources_modal = true;
            sources_track_id = track_id;
            sources_track_title = title;
            sources_track_artist = artist;
            {
                std::lock_guard<std::mutex> lock(sources_mutex);
                current_sources.clear();
            }
            status_message = "Buscando fontes disponíveis para: " + title;"""

replacement = """        void searchSourcesAsync(const std::string& track_id, const std::string& title, const std::string& artist) {
            if (is_searching_sources.load()) return;
            sources_track_id = track_id;
            sources_track_title = title;
            sources_track_artist = artist;

            auto tracks_dir = getTracksDirectory();
            auto out_json = tracks_dir / ("sources_" + track_id + ".json");
            std::error_code ec;
            if (std::filesystem::exists(out_json, ec) && std::filesystem::file_size(out_json, ec) > 100) {
                parseSourcesJsonFile(out_json);
                is_searching_sources = false;
                show_sources_modal = true;
                return;
            }

            is_searching_sources = true;
            show_sources_modal = true;
            {
                std::lock_guard<std::mutex> lock(sources_mutex);
                current_sources.clear();
            }
            status_message = "Buscando fontes disponíveis para: " + title;"""

if target not in content:
    print("Error: target not found in SpotifyDJService.h!")
    exit(1)

content = content.replace(target, replacement, 1)

tmp_path = file_path + ".tmp"
with open(tmp_path, "w", encoding="utf-8") as f:
    f.write(content)

os.system(f'cmd /c move /y "{tmp_path.replace("/", chr(92))}" "{file_path.replace("/", chr(92))}"')
print("SpotifyDJService.h updated successfully!")
