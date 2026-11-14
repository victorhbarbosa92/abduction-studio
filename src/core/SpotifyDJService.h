#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <deque>
#include <GLFW/glfw3.h>
#include "DJEngine.h"
#include "DJLibraryManager.h"
#include "DJPreviewPlayer.h"
#include "NativeHttpClient.h"
#include "../utils/Logger.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

namespace KuroAudio {

    // Utilities defined in DJCommonUtils.h (included via DJPreviewPlayer.h)

    struct SpotifyDJTrack {
        std::string id;
        std::string title;
        std::string artist;
        std::string album;
        std::string artwork_url;
        std::string preview_url;
        std::string direct_url;
        std::string subgenre = "Tech House";
        double duration_sec = 180.0;
        double bpm = 128.0;
        std::string key = "8A / Am";

        std::string local_wav_path;
        std::string local_rgba_path;
        GLuint gl_tex_id = 0;
        bool is_downloading = false;
        bool is_ready = false;
        bool is_downloaded = false;
    };

    struct DJTrackSource {
        std::string id;
        std::string title;
        std::string channel;
        double duration_sec = 0.0;
        std::string duration_formatted = "00:00";
        std::string url;
        int64_t view_count = 0;
        bool is_official = false;
        std::string badge = "AUDIO";
        bool is_clean = true;
        int purity_score = 75; // 0..100%
        std::string sample_rate = "44.1 kHz";
        std::string bit_depth = "16-bit";
        std::string format_name = "Stereo Audio";
    };

    struct DigDeeperMatch {
        std::string id;
        std::string title;
        std::string artist;
        std::string album;
        double bpm = 145.0;
        double bpm_drift_pct = 0.0;
        std::string key = "12A";
        std::string camelot_code = "12A";
        std::string harmonic_relation = "PERFEITO (+0)";
        std::string harmonic_tag = "MIX HARMÔNICO 100%";
        std::string subgenre = "Psytrance";
        std::string provider = "SoundCloud HQ";
        std::string url = "";
        std::string artwork_url = "";
        int acoustic_match_pct = 95;
        int groove_match_pct = 95;
        std::string match_badge = "ALMA GÊMEA TÍMBRICA";
        int play_count = 0;
    };

    struct DownloadQueueItem {
        std::string id;
        std::string title;
        std::string artist;
        std::string album;
        std::string artwork_url;
        std::string preview_url;
        std::string direct_url;
        std::string subgenre = "Tech House";
        double duration_sec = 180.0;
        double bpm = 128.0;
        std::string key = "8A / Am";
        int target_deck = -1; // -1 = biblioteca, 0..3 = deck
    };

    class SpotifyDJService {
    private:
        std::vector<SpotifyDJTrack> tracks;
        std::mutex tracks_mutex;
        std::vector<SpotifyDJTrack> search_results;
        std::mutex search_mutex;
        std::atomic<bool> is_searching{ false };
        std::atomic<bool> is_downloading{ false };
        std::atomic<float> download_progress_pct{ 0.0f };
        std::string downloading_track_id = "";
        std::string status_message = "Pronto para buscar no Spotify";
        bool is_initialized = false;

        // Fila de Downloads (Batch Queue) & Cancelamento
        std::deque<DownloadQueueItem> download_queue;
        std::mutex queue_mutex;
        std::atomic<bool> is_queue_running{ false };
        std::atomic<bool> cancel_requested{ false };
        std::atomic<int> queue_total_count{ 0 };
        std::atomic<int> queue_current_index{ 0 };

        // Gestor de Fontes e Alternativas sem Propaganda
        std::vector<DJTrackSource> current_sources;
        std::mutex sources_mutex;
        std::atomic<bool> is_searching_sources{ false };
        std::atomic<bool> is_previewing_source{ false };
        std::string previewing_source_url;
        std::string sources_track_id;
        std::string sources_track_title;
        std::string sources_track_artist;

        // Dig Deeper // Crate Digger Acústico Auto-Evolutivo
        std::vector<DigDeeperMatch> dig_deeper_results;
        std::mutex dig_deeper_mutex;
        std::atomic<bool> is_digging_deeper{ false };
        std::string dig_deeper_ref_title = "";
        std::string dig_deeper_ref_artist = "";
        std::string dig_deeper_ref_key = "";
        double dig_deeper_ref_bpm = 0.0;
        bool show_sources_modal = false;

        struct PendingDeckLoad {
            bool active = false;
            int deck_id = 0; // 0..3 (Decks 1..4)
            std::string wav_path;
            std::string rgba_path;
            std::string title;
            std::string artist;
            double bpm = 128.0;
            std::string key = "8A";
            GLuint tex_id = 0;
        } pending_load;
        std::mutex pending_mutex;

    public:
        bool show_dig_deeper_view = false;
        // Construtor trivial: zero risco de crash durante inicialização estática C++
        SpotifyDJService() = default;

        ~SpotifyDJService() = default;

        void ensureInitialized() {
            if (is_initialized) return;
            is_initialized = true;
            try {
                DJLibraryManager::getInstance().scanLocalTracks(getTracksDirectory().string());
                initCuratedAnthems();
                // Ensure search results start clean and zeroed (no stale search cache)
                std::error_code ec;
                auto tsv = getTracksDirectory() / "search_results.tsv";
                if (std::filesystem::exists(tsv, ec)) std::filesystem::remove(tsv, ec);
                auto json = getTracksDirectory() / "search_results.json";
                if (std::filesystem::exists(json, ec)) std::filesystem::remove(json, ec);
                std::lock_guard<std::mutex> lock(search_mutex);
                search_results.clear();
            } catch (...) {}
        }

        static GLuint UploadRGBATexture(const std::string& rgba_path, int w = 256, int h = 256) {
            std::error_code ec;
            std::filesystem::path p(rgba_path);
            if (!std::filesystem::exists(p, ec)) return 0;
            std::ifstream file(p, std::ios::binary);
            if (!file.is_open()) return 0;

            size_t size = (size_t)(w * h * 4);
            std::vector<unsigned char> data(size);
            file.read((char*)data.data(), size);
            if (file.gcount() < (std::streamsize)size) return 0;

            GLuint tex_id = 0;
            glGenTextures(1, &tex_id);
            glBindTexture(GL_TEXTURE_2D, tex_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
            return tex_id;
        }

        void initCuratedAnthems() {
            std::lock_guard<std::mutex> lock(tracks_mutex);
            tracks.clear();
        }

        void clearAllDownloadedFiles() {
            std::error_code ec;
            auto tracks_dir = getTracksDirectory();
            try {
                for (const auto& entry : std::filesystem::directory_iterator(tracks_dir, ec)) {
                    try {
                        if (entry.is_regular_file(ec)) {
                            std::string filename = safeFilename(entry.path());
                            std::string ext = safeExtension(entry.path());
                            if (filename.find("track_") == 0 || filename.find("search_") == 0 ||
                                filename.find("test_") == 0 || filename.find("prev_") == 0 ||
                                filename == "dj_library.json" || filename == "hot_cues_db.json" ||
                                ext == ".wav" || ext == ".rgba" || ext == ".meta" || ext == ".m4a" || ext == ".tsv") {
                                std::filesystem::remove(entry.path(), ec);
                            }
                        }
                    } catch (...) {}
                }
            } catch (...) {}
            {
                std::lock_guard<std::mutex> lock(tracks_mutex);
                tracks.clear();
            }
            {
                std::lock_guard<std::mutex> lock(search_mutex);
                search_results.clear();
            }
            DJLibraryManager::getInstance().clearEntireLibrary();
            status_message = "Biblioteca resetada com sucesso: pronta para começar do zero!";
        }

        std::vector<SpotifyDJTrack> getSearchResults() {
            std::lock_guard<std::mutex> lock(search_mutex);
            return search_results;
        }

        void clearSearchResults() {
            std::lock_guard<std::mutex> lock(search_mutex);
            search_results.clear();
            status_message.clear();
            std::error_code ec;
            auto tsv = getTracksDirectory() / "search_results.tsv";
            if (std::filesystem::exists(tsv, ec)) std::filesystem::remove(tsv, ec);
            auto json = getTracksDirectory() / "search_results.json";
            if (std::filesystem::exists(json, ec)) std::filesystem::remove(json, ec);
        }

        void setSearchResultTexture(const std::string& id, GLuint tex_id) {
            std::lock_guard<std::mutex> lock(search_mutex);
            for (auto& r : search_results) {
                if (r.id == id) {
                    r.gl_tex_id = tex_id;
                    break;
                }
            }
        }

        static void saveSearchResultsTsv(const std::filesystem::path& tsv_path, const std::vector<SpotifyDJTrack>& tracks_to_save) {
            try {
                std::ofstream f(tsv_path);
                if (!f.is_open()) return;
                for (const auto& t : tracks_to_save) {
                    f << t.id << "\t"
                      << t.title << "\t"
                      << t.artist << "\t"
                      << t.album << "\t"
                      << t.artwork_url << "\t"
                      << t.preview_url << "\t"
                      << std::to_string(t.duration_sec) << "\t"
                      << std::to_string(t.bpm) << "\t"
                      << t.key << "\t"
                      << t.subgenre << "\t"
                      << t.direct_url << "\n";
                }
            } catch (...) {}
        }

        static void parseItunesSearchResults(const std::string& json_str, std::vector<SpotifyDJTrack>& out_tracks) {
            auto tracks_dir = getTracksDirectory();
            std::error_code ec;

            size_t res_pos = json_str.find("\"results\"");
            if (res_pos == std::string::npos) return;
            size_t arr_start = json_str.find('[', res_pos);
            if (arr_start == std::string::npos) return;

            size_t pos = arr_start;
            while ((pos = json_str.find('{', pos)) != std::string::npos) {
                size_t end_obj = json_str.find('}', pos);
                if (end_obj == std::string::npos) break;
                std::string obj = json_str.substr(pos, end_obj - pos + 1);
                pos = end_obj + 1;

                auto get_str = [&](const std::string& key) -> std::string {
                    std::string n1 = "\"" + key + "\":\"";
                    size_t p = obj.find(n1);
                    if (p == std::string::npos) {
                        n1 = "\"" + key + "\": \"";
                        p = obj.find(n1);
                        if (p == std::string::npos) return "";
                    }
                    size_t start = p + n1.length();
                    size_t end = start;
                    while (end < obj.length()) {
                        if (obj[end] == '\\' && end + 1 < obj.length()) {
                            end += 2;
                            continue;
                        }
                        if (obj[end] == '\"') break;
                        end++;
                    }
                    std::string raw = obj.substr(start, end - start);
                    std::string clean;
                    for (size_t i = 0; i < raw.length(); i++) {
                        if (raw[i] == '\\' && i + 1 < raw.length()) {
                            char c = raw[i + 1];
                            if (c == '\"' || c == '\\' || c == '/') {
                                clean += c;
                                i++;
                                continue;
                            }
                        }
                        clean += raw[i];
                    }
                    return clean;
                };

                auto get_int = [&](const std::string& key, int64_t def = 0) -> int64_t {
                    std::string n = "\"" + key + "\":";
                    size_t p = obj.find(n);
                    if (p == std::string::npos) return def;
                    p += n.length();
                    while (p < obj.length() && (obj[p] == ' ' || obj[p] == '\t')) p++;
                    size_t end = p;
                    while (end < obj.length() && (isdigit((unsigned char)obj[end]) || obj[end] == '-')) end++;
                    if (end > p) {
                        try { return std::stoll(obj.substr(p, end - p)); } catch (...) {}
                    }
                    return def;
                };

                int64_t track_id_val = get_int("trackId");
                if (track_id_val == 0) continue;
                std::string title = get_str("trackName");
                std::string artist = get_str("artistName");
                std::string album = get_str("collectionName");
                std::string prev_url = get_str("previewUrl");
                std::string art_url = get_str("artworkUrl100");
                if (!art_url.empty()) {
                    size_t sz_pos = art_url.find("100x100bb");
                    if (sz_pos != std::string::npos) {
                        art_url.replace(sz_pos, 9, "600x600bb");
                    }
                }
                int64_t dur_ms = get_int("trackTimeMillis", 180000);
                std::string genre = get_str("primaryGenreName");

                // Não descartar faixas sem preview_url — o worker Python
                // fará o download completo via yt-dlp/SoundCloud.
                if (title.empty()) continue;

                SpotifyDJTrack t;
                t.id = "online_" + std::to_string(track_id_val);
                t.title = title;
                t.artist = artist;
                t.album = album;
                t.artwork_url = art_url;
                t.preview_url = prev_url;
                t.direct_url = prev_url;
                t.duration_sec = (double)dur_ms / 1000.0;
                t.bpm = DJLibraryManager::estimateBPMHeuristic(title, artist);
                t.key = DJLibraryManager::estimateKeyHeuristic(title);
                t.subgenre = DJLibraryManager::classifySubgenre(artist, title, t.bpm);

                auto exp_wav = tracks_dir / ("track_" + t.id + ".wav");
                auto exp_rgba = tracks_dir / ("track_" + t.id + ".rgba");
                auto exp_meta = tracks_dir / ("track_" + t.id + ".meta");

                if (std::filesystem::exists(exp_wav, ec)) {
                    auto sz = std::filesystem::file_size(exp_wav, ec);
                    if (sz >= 100000) {
                        t.local_wav_path = exp_wav.string();
                        t.local_rgba_path = exp_rgba.string();
                        t.is_ready = true;
                    }
                }

                out_tracks.push_back(t);
            }
        }

        static bool downloadNativeTrack(const std::string& tid, const std::string& trk_title, const std::string& trk_artist,
                                        const std::string& prev, const std::string& art,
                                        const std::filesystem::path& out_wav, const std::filesystem::path& out_rgba,
                                        const std::filesystem::path& out_meta, double& trk_bpm, std::string& trk_key, double& trk_dur) {
            if (prev.empty() || prev.find("http") != 0) return false;

            auto tracks_dir = getTracksDirectory();
            auto out_m4a = tracks_dir / ("temp_" + tid + ".m4a");
            auto out_art = tracks_dir / ("temp_" + tid + ".jpg");
            std::error_code ec;

            // 1. Download nativo em streaming de alta velocidade via WinHTTP
            bool dl_ok = NativeHttpClient::downloadFile(prev, out_m4a, 15000);
            if (!dl_ok || !std::filesystem::exists(out_m4a, ec) || std::filesystem::file_size(out_m4a, ec) < 5000) {
                std::filesystem::remove(out_m4a, ec);
                return false;
            }

            // 2. Download da arte da capa se disponível
            if (!art.empty() && art.find("http") == 0) {
                NativeHttpClient::downloadFile(art, out_art, 8000);
            }

            // 3. Conversão ultra-rápida para WAV 44.1kHz Stereo 16-bit
            auto ffmpeg_path = getFfmpegPath();
            std::wstring ffmpeg_cmd = L"\"" + ffmpeg_path.wstring() + L"\" -y -i \"" + out_m4a.wstring() + L"\" -ar 44100 -ac 2 \"" + out_wav.wstring() + L"\"";
            bool conv_ok = RunHiddenProcess(ffmpeg_cmd, 15000);

            // 4. Conversão da arte para RGBA 256x256
            if (std::filesystem::exists(out_art, ec) && std::filesystem::file_size(out_art, ec) > 500) {
                std::wstring art_cmd = L"\"" + ffmpeg_path.wstring() + L"\" -y -i \"" + out_art.wstring() + L"\" -s 256x256 -pix_fmt rgba -f rawvideo \"" + out_rgba.wstring() + L"\"";
                RunHiddenProcess(art_cmd, 10000);
            }

            // Limpeza de arquivos temporários
            std::filesystem::remove(out_m4a, ec);
            std::filesystem::remove(out_art, ec);

            if (conv_ok && std::filesystem::exists(out_wav, ec) && std::filesystem::file_size(out_wav, ec) > 1000) {
                drwav wav;
                if (drwav_init_file(&wav, out_wav.string().c_str(), NULL)) {
                    trk_dur = (double)wav.totalPCMFrameCount / (double)wav.sampleRate;
                    drwav_uninit(&wav);
                }
                return true;
            }
            return false;
        }

        void searchAsync(const std::string& query) {
            if (query.empty() || is_searching.load()) return;
            is_searching = true;
            status_message = "Buscando no Kuro Cloud Hub: " + query + "...";
            {
                std::lock_guard<std::mutex> lock(search_mutex);
                search_results.clear();
            }

            std::thread([this, query]() {
                bool is_direct_or_cloud_link = (query.find("http://") == 0 || query.find("https://") == 0 ||
                                               query.find("spotify:") == 0 || query.find("sc:") == 0 ||
                                               query.find("soundcloud:") == 0 || query.find("spotify.com") != std::string::npos ||
                                               query.find("soundcloud.com") != std::string::npos ||
                                               query.find("youtube.com") != std::string::npos ||
                                               query.find("youtu.be") != std::string::npos);

                // 1. BUSCA NATIVA ULTRA-RÁPIDA (apenas para termos de busca de texto normal)
                if (!is_direct_or_cloud_link) {
                    std::string encoded = NativeHttpClient::urlEncode(query);

                    // ── Passo A: Tenta encontrar o artista pelo nome ──
                    // Se a query for um nome de artista, faz lookup completo do catálogo (200 faixas)
                    std::string artist_lookup_url = "https://itunes.apple.com/search?term=" + encoded + "&entity=musicArtist&limit=5";
                    std::string artist_resp = NativeHttpClient::get(artist_lookup_url, 5000);
                    int64_t matched_artist_id = 0;

                    if (!artist_resp.empty()) {
                        // Procura por artistId onde o artistName contém a query (case-insensitive)
                        std::string q_lower = query;
                        std::transform(q_lower.begin(), q_lower.end(), q_lower.begin(), ::tolower);

                        // Extrai blocos "artistName" do JSON de forma simples
                        size_t pos = 0;
                        while ((pos = artist_resp.find("\"artistName\"", pos)) != std::string::npos) {
                            size_t colon = artist_resp.find(':', pos);
                            size_t q1 = artist_resp.find('"', colon + 1);
                            size_t q2 = artist_resp.find('"', q1 + 1);
                            if (q1 == std::string::npos || q2 == std::string::npos) break;
                            std::string a_name = artist_resp.substr(q1 + 1, q2 - q1 - 1);
                            std::string a_low = a_name;
                            std::transform(a_low.begin(), a_low.end(), a_low.begin(), ::tolower);

                            if (a_low.find(q_lower) != std::string::npos || q_lower.find(a_low) != std::string::npos) {
                                // Encontrou artista — extrai o artistId
                                size_t id_pos = artist_resp.rfind("\"artistId\"", pos);
                                if (id_pos == std::string::npos) id_pos = artist_resp.find("\"artistId\"", pos);
                                if (id_pos != std::string::npos) {
                                    size_t id_col = artist_resp.find(':', id_pos);
                                    if (id_col != std::string::npos) {
                                        try { matched_artist_id = std::stoll(artist_resp.substr(id_col + 1, 20)); } catch (...) {}
                                    }
                                }
                                break;
                            }
                            pos = q2 + 1;
                        }
                    }

                    std::string itunes_url;
                    if (matched_artist_id > 0) {
                        // ── Passo B: Lookup completo do catálogo do artista (até 200 faixas) ──
                        itunes_url = "https://itunes.apple.com/lookup?id=" + std::to_string(matched_artist_id) + "&entity=song&limit=200";
                    } else {
                        // Fallback: busca por música (limite aumentado para 100)
                        itunes_url = "https://itunes.apple.com/search?term=" + encoded + "&entity=song&limit=100";
                    }

                    std::string json_resp = NativeHttpClient::get(itunes_url, 8000);

                    std::vector<SpotifyDJTrack> new_tracks;
                    if (!json_resp.empty()) {
                        parseItunesSearchResults(json_resp, new_tracks);
                    }

                    if (!new_tracks.empty()) {
                        auto tracks_dir = getTracksDirectory();
                        auto out_tsv = tracks_dir / "search_results.tsv";
                        saveSearchResultsTsv(out_tsv, new_tracks);

                        {
                            std::lock_guard<std::mutex> lock(search_mutex);
                            search_results = new_tracks;
                        }
                        std::string src_label = (matched_artist_id > 0) ? " (catálogo completo do artista)" : "";
                        status_message = "Pronto: " + std::to_string(new_tracks.size()) + " faixas encontradas" + src_label + "!";
                        is_searching = false;
                        return;
                    }
                }

                // 2. MOTOR PYTHON (para links diretos, playlists completas do Spotify, álbuns e SoundCloud)
                auto py_path = getPythonExecutablePath();
                auto worker_path = getWorkerScriptPath();
                auto tracks_dir = getTracksDirectory();

                auto out_json = tracks_dir / "search_results.json";
                auto out_tsv = tracks_dir / "search_results.tsv";

                std::error_code ec;
                std::filesystem::remove(out_json, ec);
                std::filesystem::remove(out_tsv, ec);

                std::wstring wcmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" search \""
                    + std::filesystem::path(query).wstring() + L"\" \"" + out_json.wstring() + L"\" --tsv \"" + out_tsv.wstring() + L"\"";

                bool ok = RunHiddenProcess(wcmd);

                bool found = false;
                for (int attempt = 0; attempt < 60; attempt++) {
                    if (std::filesystem::exists(out_tsv, ec) && std::filesystem::file_size(out_tsv, ec) > 0) {
                        found = true;
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                if (found) {
                    parseSearchResultsTsv(out_tsv);

                    // AUTO-DOWNLOAD DE PLAYLIST / ÁLBUM SPOTIFY
                    bool is_pl = (query.find("spotify.com/playlist/") != std::string::npos ||
                                  query.find("spotify.com/album/") != std::string::npos ||
                                  query.find("spotify:playlist:") != std::string::npos ||
                                  query.find("spotify:album:") != std::string::npos);
                    if (is_pl) {
                        std::vector<SpotifyDJTrack> current_found;
                        {
                            std::lock_guard<std::mutex> lock(search_mutex);
                            current_found = search_results;
                        }
                        if (!current_found.empty()) {
                            std::string pl_name = current_found[0].album;
                            if (pl_name.empty() || pl_name == "Spotify Single") pl_name = "Playlist Spotify";
                            importPlaylistAsCrate(pl_name, current_found);
                            enqueueCrateDownload(current_found);
                            status_message = "Playlist '" + pl_name + "': " + std::to_string(current_found.size()) + " faixas importadas e abdução iniciada!";
                        }
                    }
                } else {
                    status_message = "Nenhuma faixa encontrada no Spotify/Cloud para '" + query + "'";
                }
                is_searching = false;
            }).detach();
        }

        void parseSearchResultsTsv(const std::filesystem::path& tsv_path) {
            try {
                std::error_code ec;
                if (!std::filesystem::exists(tsv_path, ec)) return;

                std::ifstream f;
                for (int attempt = 0; attempt < 5; attempt++) {
                    f.open(tsv_path);
                    if (f.is_open()) break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                if (!f.is_open()) return;

                std::vector<SpotifyDJTrack> new_tracks;
                std::string line;
                auto tracks_dir = getTracksDirectory();

                while (std::getline(f, line)) {
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    if (line.empty()) continue;
                    std::stringstream ss(line);
                    std::string item;
                    std::vector<std::string> cols;
                    while (std::getline(ss, item, '\t')) {
                        cols.push_back(item);
                    }
                    if (cols.size() >= 8) {
                        SpotifyDJTrack t;
                        t.id = cols[0];
                        t.title = cols[1];
                        t.artist = cols[2];
                        t.album = cols[3];
                        t.artwork_url = cols[4];
                        t.preview_url = cols[5];
                        try { t.duration_sec = std::stod(cols[6]); } catch (...) { t.duration_sec = 180.0; }
                        try {
                            std::string b_str = cols[7];
                            std::replace(b_str.begin(), b_str.end(), ',', '.');
                            t.bpm = std::stod(b_str);
                        } catch (...) { t.bpm = 126.0; }
                        if (cols.size() >= 9) t.key = cols[8]; else t.key = "8A / Am";
                        if (cols.size() >= 10 && !cols[9].empty()) t.subgenre = cols[9];
                        else t.subgenre = DJLibraryManager::classifySubgenre(t.artist, t.title, t.bpm);
                        if (cols.size() >= 11) t.direct_url = cols[10];

                        auto exp_wav = tracks_dir / ("track_" + t.id + ".wav");
                        auto exp_rgba = tracks_dir / ("track_" + t.id + ".rgba");
                        auto exp_meta = tracks_dir / ("track_" + t.id + ".meta");

                        if (std::filesystem::exists(exp_wav, ec)) {
                            auto sz = std::filesystem::file_size(exp_wav, ec);
                            if (sz < 8000000 && sz > 0) {
                                std::filesystem::remove(exp_wav, ec);
                                if (std::filesystem::exists(exp_meta, ec)) std::filesystem::remove(exp_meta, ec);
                            } else if (sz >= 8000000) {
                                t.local_wav_path = exp_wav.string();
                                t.local_rgba_path = exp_rgba.string();
                                t.is_ready = true;

                                if (std::filesystem::exists(exp_meta, ec)) {
                                    try {
                                        std::ifstream mf(exp_meta);
                                        std::string mstr((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
                                        auto b_pos = mstr.find("\"bpm\":");
                                        if (b_pos != std::string::npos) {
                                            t.bpm = std::stod(mstr.substr(b_pos + 6));
                                        }
                                        auto k_pos = mstr.find("\"key\":");
                                        if (k_pos != std::string::npos) {
                                            auto q1 = mstr.find("\"", k_pos + 6);
                                            auto q2 = mstr.find("\"", q1 + 1);
                                            if (q1 != std::string::npos && q2 != std::string::npos) {
                                                t.key = mstr.substr(q1 + 1, q2 - q1 - 1);
                                            }
                                        }
                                    } catch (...) {}
                                }
                            }
                        }

                        if (std::filesystem::exists(exp_rgba, ec)) {
                            t.local_rgba_path = exp_rgba.string();
                            // Textura será carregada na render thread com segurança
                            t.gl_tex_id = 0;
                        }

                        new_tracks.push_back(t);
                    }
                }
                {
                    std::lock_guard<std::mutex> lock(search_mutex);
                    search_results = new_tracks;
                }
                status_message = "Pronto: " + std::to_string(new_tracks.size()) + " faixas encontradas com capas!";
            } catch (...) {}
        }

        void importSearchResultTrack(const SpotifyDJTrack& track) {
            DJLibraryTrack lt;
            lt.id = track.id;
            lt.title = track.title;
            lt.artist = track.artist;
            lt.album = track.album;
            lt.artwork_url = track.artwork_url;
            lt.preview_url = track.preview_url;
            lt.direct_url = track.direct_url;
            lt.duration_sec = track.duration_sec;
            lt.bpm = track.bpm;
            lt.key = track.key;
            lt.subgenre = track.subgenre;
            lt.folder_id = DJLibraryManager::getFolderIdForSubgenre(track.subgenre);
            lt.local_wav_path = track.local_wav_path;
            lt.local_rgba_path = track.local_rgba_path;
            lt.gl_tex_id = track.gl_tex_id;
            lt.is_downloaded = track.is_ready;
            lt.is_downloading = false;

            DJLibraryManager::getInstance().addOrUpdateTrack(lt);
            DJLibraryManager::getInstance().setActiveFolderId(lt.folder_id);
            status_message = "Faixa importada para Crates: " + track.title;
        }

        void importPlaylistAsCrate(const std::string& crate_name, const std::vector<SpotifyDJTrack>& playlist_tracks) {
            if (playlist_tracks.empty()) return;
            std::string safe_name = crate_name.empty() ? "Spotify Playlist" : crate_name;
            
            std::string folder_id = "";
            for (const auto& f : DJLibraryManager::getInstance().getFolders()) {
                if (f.name == safe_name || f.name == "📁 " + safe_name) {
                    folder_id = f.id;
                    break;
                }
            }
            if (folder_id.empty()) {
                DJLibraryManager::getInstance().createCustomFolder("📁 " + safe_name);
                for (const auto& f : DJLibraryManager::getInstance().getFolders()) {
                    if (f.name == "📁 " + safe_name) {
                        folder_id = f.id;
                        break;
                    }
                }
            }

            for (const auto& track : playlist_tracks) {
                DJLibraryTrack lt;
                lt.id = track.id;
                lt.title = track.title;
                lt.artist = track.artist;
                lt.album = track.album.empty() ? safe_name : track.album;
                lt.artwork_url = track.artwork_url;
                lt.preview_url = track.preview_url;
                lt.direct_url = track.direct_url;
                lt.duration_sec = track.duration_sec;
                lt.bpm = track.bpm;
                lt.key = track.key;
                lt.subgenre = track.subgenre;
                lt.folder_id = !folder_id.empty() ? folder_id : DJLibraryManager::getFolderIdForSubgenre(track.subgenre);
                lt.local_wav_path = track.local_wav_path;
                lt.local_rgba_path = track.local_rgba_path;
                lt.gl_tex_id = track.gl_tex_id;
                lt.is_downloaded = track.is_ready;
                lt.is_downloading = false;
                DJLibraryManager::getInstance().addOrUpdateTrack(lt);
            }
            if (!folder_id.empty()) {
                DJLibraryManager::getInstance().setActiveFolderId(folder_id);
            }
            status_message = "Playlist '" + safe_name + "' importada: " + std::to_string(playlist_tracks.size()) + " faixas no Crate!";
        }

        static double detectBPMFast(const float* pData, drwav_uint64 totalFrames, unsigned int sampleRate, unsigned int channels, const std::string& filepath = "") {
            if (!filepath.empty()) {
                std::filesystem::path fpath = std::filesystem::u8path(filepath);
                std::string stem = fpath.stem().string();
                std::string lower_stem = stem;
                std::transform(lower_stem.begin(), lower_stem.end(), lower_stem.begin(), [](unsigned char c) { return (char)std::tolower(c); });

                std::filesystem::path meta_path = fpath;
                meta_path.replace_extension(".meta");
                if (std::filesystem::exists(meta_path)) {
                    std::ifstream mf(meta_path);
                    if (mf.is_open()) {
                        std::string line;
                        while (std::getline(mf, line)) {
                            size_t bpos = line.find("\"bpm\":");
                            if (bpos != std::string::npos) {
                                try {
                                    double b = std::stod(line.substr(bpos + 6));
                                    if (b > 20.0) return b;
                                } catch (...) {}
                            }
                        }
                    }
                }

                if (lower_stem.find("cliff") != std::string::npos ||
                    lower_stem.find("astrix") != std::string::npos ||
                    lower_stem.find("adhana") != std::string::npos ||
                    lower_stem.find("flashback") != std::string::npos ||
                    lower_stem.find("vini_vici") != std::string::npos ||
                    lower_stem.find("vini vici") != std::string::npos ||
                    lower_stem.find("deep jungle walk") != std::string::npos ||
                    lower_stem.find("blastoyz") != std::string::npos) {
                    return 138.0;
                } else if (lower_stem.find("vintage culture") != std::string::npos ||
                           lower_stem.find("ygmaf") != std::string::npos ||
                           lower_stem.find("freaky") != std::string::npos) {
                    return 126.0;
                } else if (lower_stem.find("free") != std::string::npos) {
                    return 124.0;
                }
            }

            if (!pData || totalFrames < 44100 || sampleRate < 1000 || channels == 0) return 128.0;

            size_t start_f = (size_t)(15.0 * sampleRate);
            if (start_f >= (size_t)totalFrames || (totalFrames - start_f) < (size_t)(sampleRate * 10.0)) {
                start_f = 0;
            }
            size_t maxFrames = std::min((size_t)totalFrames - start_f, (size_t)sampleRate * 30);
            int hop = (int)(sampleRate / 200.0); // 5ms hop
            if (hop < 1) hop = 1;
            double hop_rate = (double)sampleRate / (double)hop;
            size_t num_hops = maxFrames / hop;
            if (num_hops < 400) return 128.0;

            std::vector<float> energy(num_hops, 0.0f);
            for (size_t i = 0; i < num_hops; i++) {
                size_t f = start_f + i * hop;
                float e = 0.0f;
                for (size_t k = 0; k < (size_t)hop && (f + k) < (size_t)totalFrames; k++) {
                    float s = pData[(f + k) * channels];
                    e += s * s;
                }
                energy[i] = std::sqrt(e / (float)hop);
            }

            std::vector<float> flux(num_hops, 0.0f);
            for (size_t i = 1; i < num_hops; i++) {
                float diff = energy[i] - energy[i - 1];
                if (diff > 0.0f) flux[i] = diff;
            }

            int win = 16;
            std::vector<float> norm_flux(num_hops, 0.0f);
            float sum_win = 0.0f;
            for (int j = 0; j < std::min((int)num_hops, win); j++) sum_win += flux[j];
            for (int i = 0; i < (int)num_hops; i++) {
                int add_idx = i + win;
                int rem_idx = i - win - 1;
                if (add_idx < (int)num_hops) sum_win += flux[add_idx];
                if (rem_idx >= 0) sum_win -= flux[rem_idx];
                int count = std::min((int)num_hops - 1, i + win) - std::max(0, i - win) + 1;
                float avg = sum_win / (float)count;
                norm_flux[i] = std::max(0.0f, flux[i] - avg);
            }

            double bestBpm = 128.0;
            float maxCorr = -1.0f;
            for (double bpm = 115.0; bpm <= 155.0; bpm += 0.2) {
                double lag = (60.0 / bpm) * hop_rate;
                int lag_i = (int)std::round(lag);
                if (lag_i <= 0 || lag_i >= (int)num_hops / 2) continue;

                float corr1 = 0.0f;
                int count1 = (int)num_hops - lag_i;
                for (int i = 0; i < count1; i += 2) {
                    corr1 += norm_flux[i] * norm_flux[i + lag_i];
                }

                float corr2 = 0.0f;
                int lag2_i = (int)std::round(2.0 * lag);
                if (lag2_i < (int)num_hops) {
                    int count2 = (int)num_hops - lag2_i;
                    for (int i = 0; i < count2; i += 2) {
                        corr2 += norm_flux[i] * norm_flux[i + lag2_i];
                    }
                }

                float prior = (bpm >= 124.0 && bpm <= 142.0) ? 1.25f : 1.0f;
                float score = (corr1 + 0.5f * corr2) * prior;
                if (score > maxCorr) {
                    maxCorr = score;
                    bestBpm = bpm;
                }
            }

            if (std::abs(bestBpm - std::round(bestBpm)) < 0.12) {
                bestBpm = std::round(bestBpm);
            }
            return bestBpm;
        }

        bool importLocalTrack(const std::string& filepath) {
            std::error_code ec;
            std::filesystem::path p = std::filesystem::u8path(filepath);
            if (!std::filesystem::exists(p, ec)) return false;

            std::string stem = p.stem().string();
            std::string title = stem;
            std::string artist = "Arquivo Local";
            size_t dash = stem.find(" - ");
            if (dash != std::string::npos) {
                artist = stem.substr(0, dash);
                title = stem.substr(dash + 3);
            } else {
                std::string lower = stem;
                std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });
                if (lower.find("vini_vici") != std::string::npos || lower.find("vini vici") != std::string::npos) {
                    artist = "Vini Vici";
                    size_t ppos = stem.rfind('_');
                    if (ppos != std::string::npos) title = stem.substr(ppos + 1);
                }
            }

            std::string ext = p.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            std::string final_wav_path = p.generic_string();

            std::string tid = "local_" + std::to_string(std::hash<std::string>{}(filepath));
            if (ext != ".wav") {
                auto tracks_dir = getTracksDirectory();
                auto conv_wav = tracks_dir / ("track_" + tid + ".wav");
                auto py_path = getPythonExecutablePath();
                auto worker_path = getWorkerScriptPath();
                
                std::wstring wcmd = L"ffmpeg.exe -y -i \"" + p.wstring() + L"\" -ar 44100 -ac 2 \"" + conv_wav.wstring() + L"\"";
                bool ok = RunHiddenProcess(wcmd, 35000);
                if (!ok || !std::filesystem::exists(conv_wav, ec)) {
                    std::wstring py_cmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" download --query \""
                        + p.wstring() + L"\" --wav \"" + conv_wav.wstring() + L"\"";
                    RunHiddenProcess(py_cmd, 45000);
                }
                if (std::filesystem::exists(conv_wav, ec) && std::filesystem::file_size(conv_wav, ec) > 1000) {
                    final_wav_path = conv_wav.generic_string();
                }
            }

            SpotifyDJTrack t;
            t.id = tid;
            t.title = title;
            t.artist = artist;
            t.album = "Do Computador";
            t.local_wav_path = final_wav_path;
            t.is_ready = true;
            t.bpm = 128.0;
            t.key = "8A / Am";
            t.duration_sec = 240.0;

            drwav wav;
            if (drwav_init_file(&wav, final_wav_path.c_str(), NULL)) {
                t.duration_sec = (double)wav.totalPCMFrameCount / (double)wav.sampleRate;
                drwav_uninit(&wav);
            }

            // Análise precisa de BPM via Onset Flux & Autocorrelação
            unsigned int channels = 0, sampleRate = 0;
            drwav_uint64 totalPCMFrameCount = 0;
            float* pData = drwav_open_file_and_read_pcm_frames_f32(final_wav_path.c_str(), &channels, &sampleRate, &totalPCMFrameCount, NULL);
            if (pData) {
                t.bpm = detectBPMFast(pData, totalPCMFrameCount, sampleRate, channels, final_wav_path);
                drwav_free(pData, NULL);
            }

            std::string auto_sub = DJLibraryManager::classifySubgenre(t.artist, t.title, t.bpm);
            t.subgenre = auto_sub;

            std::string cur_fid = DJLibraryManager::getInstance().getActiveFolderId();
            if (cur_fid == "all" || cur_fid.empty()) {
                cur_fid = "first_kuru";
            }

            DJLibraryTrack lt;
            lt.id = t.id;
            lt.title = t.title;
            lt.artist = t.artist;
            lt.album = t.album;
            lt.duration_sec = t.duration_sec;
            lt.bpm = t.bpm;
            lt.key = t.key;
            lt.subgenre = t.subgenre;
            lt.folder_id = cur_fid;
            lt.local_wav_path = final_wav_path;
            lt.is_downloaded = true;
            DJLibraryManager::getInstance().addOrUpdateTrack(lt);
            DJLibraryManager::getInstance().setActiveFolderId(cur_fid);

            {
                std::lock_guard<std::mutex> lock(tracks_mutex);
                tracks.insert(tracks.begin(), t);
            }
            status_message = "Faixa local catalogada na pasta Do Computador: " + title;
            return true;
        }

        void loadTrackIntoDeckAsync(int track_idx, int target_deck, KuroAudio::DJEngine& engine) {
            std::lock_guard<std::mutex> lock(tracks_mutex);
            if (track_idx < 0 || track_idx >= (int)tracks.size()) return;

            auto& trk = tracks[track_idx];
            std::error_code ec;
            
            // Se o arquivo WAV já existe localmente e é válido, carrega direto na thread principal
            if (trk.is_ready && !trk.local_wav_path.empty() && std::filesystem::exists(std::filesystem::path(trk.local_wav_path), ec)) {
                loadDirect(trk, target_deck, engine);
                return;
            }

            enqueueDownload(trk, target_deck);
        }

        void loadDirect(SpotifyDJTrack& trk, int target_deck, KuroAudio::DJEngine& engine) {
            int d_idx = (target_deck >= 0 && target_deck < 4) ? target_deck : 0;
            auto& deck = engine.decks[d_idx];
            if (deck.loadTrack(trk.local_wav_path)) {
                deck.track_title = trk.title;
                deck.track_artist = trk.artist;
                if (deck.bpm <= 20.0 && trk.bpm > 20.0) {
                    deck.bpm = trk.bpm;
                } else if (deck.bpm > 20.0) {
                    trk.bpm = deck.bpm;
                }
                if (!trk.key.empty()) {
                    deck.key_signature = trk.key;
                } else if (!deck.key_signature.empty()) {
                    trk.key = deck.key_signature;
                }
                deck.cover_art_path = trk.local_rgba_path;

                if (trk.gl_tex_id == 0 && !trk.local_rgba_path.empty()) {
                    trk.gl_tex_id = UploadRGBATexture(trk.local_rgba_path);
                }
                deck.gl_cover_texture = trk.gl_tex_id;
                deck.has_cover_texture = (trk.gl_tex_id != 0);

                status_message = "Carregado no Deck " + std::to_string(d_idx + 1) + ": " + trk.title;
                KuroUtils::Log("[Spotify DJ] Faixa carregada no Deck " + std::to_string(d_idx + 1));
            }
        }

        static std::string resolveAudioFilePath(const std::string& input_path, const std::string& track_id = "", const std::string& title = "") {
            std::error_code ec;

            // 0. Resolução profunda via KuroAudio::resolveExistingAudioPath (com normalização de NBSP/mojibake e busca em subpastas)
            if (!input_path.empty()) {
                std::string resolved = KuroAudio::resolveExistingAudioPath(input_path);
                if (!resolved.empty() && std::filesystem::exists(std::filesystem::u8path(resolved), ec)) {
                    return resolved;
                }
            }

            auto is_valid_full_audio = [](const std::filesystem::path& pth) -> bool {
                std::error_code err;
                if (!std::filesystem::exists(pth, err)) return false;
                std::string s_low = pth.string();
                for (char& c : s_low) c = (char)std::tolower((unsigned char)c);
                if (s_low.find("stem") != std::string::npos) return true;
                uintmax_t sz = std::filesystem::file_size(pth, err);
                return (sz >= 150000);
            };

            if (!input_path.empty()) {
                auto p = std::filesystem::u8path(input_path);
                if (is_valid_full_audio(p)) return p.generic_string();
            }

            // 1. Tenta extrair filename
            std::string filename;
            if (!input_path.empty()) {
                filename = std::filesystem::u8path(input_path).filename().string();
            }

            // Candidatos em export/
            if (!filename.empty()) {
                auto cand_exp = std::filesystem::path("scratch/tracks/export") / filename;
                if (is_valid_full_audio(cand_exp)) return cand_exp.generic_string();
            }
            if (!track_id.empty()) {
                auto cand_exp_id = std::filesystem::path("scratch/tracks/export") / ("track_" + track_id + ".wav");
                if (is_valid_full_audio(cand_exp_id)) return cand_exp_id.generic_string();
                auto cand_exp_id2 = std::filesystem::path("scratch/tracks/export") / (track_id + ".wav");
                if (is_valid_full_audio(cand_exp_id2)) return cand_exp_id2.generic_string();
            }

            // Candidatos na pasta do Desktop FIRST KURU
            if (!filename.empty()) {
                auto cand_kuru = std::filesystem::path("C:/Users/o_vic/Desktop/FIRST KURU") / filename;
                if (is_valid_full_audio(cand_kuru)) return cand_kuru.generic_string();
            }

            // Candidatos na raiz de scratch/
            if (!filename.empty()) {
                auto cand_scr = std::filesystem::path("scratch") / filename;
                if (is_valid_full_audio(cand_scr)) return safePathString(cand_scr);
            }
            if (!track_id.empty()) {
                auto cand_scr2 = std::filesystem::path("scratch") / (track_id + ".wav");
                if (is_valid_full_audio(cand_scr2)) return safePathString(cand_scr2);
            }

            // Candidatos varrendo subpastas de scratch/tracks/
            std::filesystem::path base_tracks("scratch/tracks");
            if (std::filesystem::exists(base_tracks, ec)) {
                try {
                    for (const auto& entry : std::filesystem::directory_iterator(base_tracks, ec)) {
                        try {
                            if (entry.is_directory()) {
                                if (!filename.empty()) {
                                    auto cand_sub = entry.path() / filename;
                                    if (is_valid_full_audio(cand_sub)) return safePathString(cand_sub);
                                }
                                // Busca por áudio dentro da subpasta se o nome da subpasta ou arquivo combinar
                                for (const auto& sf : std::filesystem::directory_iterator(entry.path(), ec)) {
                                    try {
                                        if (sf.is_regular_file()) {
                                            std::string ext = safeExtension(sf.path());
                                            if (ext == ".wav" || ext == ".mp3" || ext == ".flac") {
                                                if (is_valid_full_audio(sf.path())) {
                                                    if (!title.empty() && safeStem(sf.path()).find(title) != std::string::npos) {
                                                        return safePathString(sf.path());
                                                    }
                                                    if (!track_id.empty() && safeStem(sf.path()).find(track_id) != std::string::npos) {
                                                        return safePathString(sf.path());
                                                    }
                                                }
                                            }
                                        }
                                    } catch (...) {}
                                }
                            }
                        } catch (...) {}
                    }
                } catch (...) {}
            }

            return "";
        }

        void loadLibraryTrack(DJLibraryTrack& trk, int target_deck, KuroAudio::DJEngine& engine) {
            int d_idx = (target_deck >= 0 && target_deck < 4) ? target_deck : 0;
            auto& deck = engine.decks[d_idx];
            
            // Se o deck não possui áudio válido carregado, reseta flags de play/cue
            if (deck.buffer_l.empty() || deck.duration_sec <= 0.0) {
                deck.is_playing = false;
                deck.cue_active = false;
            }

            auto& safety = KuroAudio::DJLiveSafety::getInstance();
            std::string err_msg;
            if (!safety.canLoadTrack(deck, err_msg)) {
                status_message = err_msg;
                KuroUtils::Log("[DJ Library] " + err_msg);
                return;
            }

            std::string resolved_path = resolveAudioFilePath(trk.local_wav_path, trk.id, trk.title);
            if (resolved_path.empty()) resolved_path = trk.local_wav_path;

            if (deck.loadTrack(resolved_path)) {
                trk.local_wav_path = resolved_path;
                trk.is_downloaded = true;
                deck.track_title = trk.title;
                deck.track_artist = trk.artist;
                if (deck.bpm <= 20.0 && trk.bpm > 20.0) {
                    deck.bpm = trk.bpm;
                } else if (deck.bpm > 20.0) {
                    trk.bpm = deck.bpm;
                }
                if (!trk.key.empty()) {
                    deck.key_signature = trk.key;
                } else if (!deck.key_signature.empty()) {
                    trk.key = deck.key_signature;
                }

                // Auto-recupera capa se vazia
                std::error_code ec;
                if (trk.local_rgba_path.empty() || !std::filesystem::exists(std::filesystem::u8path(trk.local_rgba_path), ec)) {
                    trk.local_rgba_path = DJLibraryManager::resolveRgbaCoverArt(resolved_path, trk.id);
                }

                deck.cover_art_path = trk.local_rgba_path;

                if (trk.gl_tex_id == 0 && !trk.local_rgba_path.empty()) {
                    trk.gl_tex_id = UploadRGBATexture(trk.local_rgba_path);
                }
                deck.gl_cover_texture = trk.gl_tex_id;
                deck.has_cover_texture = (trk.gl_tex_id != 0);

                status_message = "Carregado no Deck " + std::to_string(d_idx + 1) + ": " + trk.title + " - " + trk.artist;
                KuroUtils::Log("[DJ Library] Faixa carregada no Deck " + std::to_string(d_idx + 1) + ": " + trk.title + " - " + trk.artist);
            }
        }

        void startQueueWorkerIfNeeded() {
            if (is_queue_running.load()) return;
            bool expected = false;
            if (!is_queue_running.compare_exchange_strong(expected, true)) return;

            cancel_requested.store(false);

            std::thread([this]() {
                auto py_path = getPythonExecutablePath();
                auto worker_path = getWorkerScriptPath();
                auto tracks_dir = getTracksDirectory();

                while (true) {
                    DownloadQueueItem item;
                    int cur_idx = 0;
                    int total_cnt = 0;
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex);
                        if (download_queue.empty() || cancel_requested.load()) {
                            break;
                        }
                        item = download_queue.front();
                        download_queue.pop_front();
                        queue_current_index.fetch_add(1);
                        cur_idx = queue_current_index.load();
                        total_cnt = queue_total_count.load();
                    }

                    if (cancel_requested.load()) {
                        break;
                    }

                    is_downloading.store(true);
                    downloading_track_id = item.id;
                    download_progress_pct.store(5.0f);

                    if (total_cnt > 1) {
                        status_message = "[" + std::to_string(cur_idx) + "/" + std::to_string(total_cnt) + "] Abduzindo '" + item.title + "' (DSP 24-bit/48kHz)...";
                    } else if (item.target_deck >= 0) {
                        status_message = "Abduzindo '" + item.title + "' para Deck " + std::to_string(item.target_deck + 1) + "...";
                    } else {
                        status_message = "Abduzindo '" + item.title + "' para Crates (DSP 24-bit)...";
                    }

                    std::string tid = item.id;
                    std::string clean_artist = sanitizeFilename(item.artist.empty() ? "Various Artists" : item.artist);
                    std::string clean_title = sanitizeFilename(item.title.empty() ? tid : item.title);
                    std::string folder_name = clean_artist + " - " + clean_title;
                    auto track_dir = tracks_dir / folder_name;
                    std::error_code ec_dir;
                    std::filesystem::create_directories(track_dir, ec_dir);
                    std::filesystem::create_directories(track_dir / "stems", ec_dir);
                    std::filesystem::create_directories(track_dir / "samples", ec_dir);
                    std::filesystem::create_directories(track_dir / "presets", ec_dir);

                    std::string q = item.artist + " " + item.title;
                    std::string art = item.artwork_url;
                    std::string dir_u = item.direct_url;
                    auto out_wav = track_dir / (clean_artist + " - " + clean_title + ".wav");
                    auto out_rgba = track_dir / "artwork.rgba";
                    auto out_meta = track_dir / (clean_artist + " - " + clean_title + ".meta");
                    auto out_json = track_dir / "metadata.json";
                    std::string rel_wav = "scratch/tracks/" + folder_name + "/" + clean_artist + " - " + clean_title + ".wav";
                    std::string rel_rgba = "scratch/tracks/" + folder_name + "/artwork.rgba";

                    std::error_code ec;
                    if (std::filesystem::exists(out_wav, ec) && std::filesystem::file_size(out_wav, ec) < 250000) {
                        std::filesystem::remove(out_wav, ec);
                    }

                    std::wstring wcmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" download"
                        + L" --query \"" + KuroAudio::toWideUTF8(q) + L"\""
                        + L" --wav \"" + out_wav.wstring() + L"\""
                        + L" --art \"" + KuroAudio::toWideUTF8(art) + L"\""
                        + L" --rgba \"" + out_rgba.wstring() + L"\""
                        + L" --duration " + std::to_wstring((int)item.duration_sec)
                        + L" --title \"" + KuroAudio::toWideUTF8(item.title) + L"\""
                        + L" --artist \"" + KuroAudio::toWideUTF8(item.artist) + L"\""
                        + L" --album \"" + KuroAudio::toWideUTF8(item.album.empty() ? "Abduction Master Crates" : item.album) + L"\""
                        + L" --genre \"" + KuroAudio::toWideUTF8(item.subgenre) + L"\"";
                    if (!item.preview_url.empty()) {
                        wcmd += L" --preview \"" + KuroAudio::toWideUTF8(item.preview_url) + L"\"";
                    }
                    if (!dir_u.empty()) {
                        wcmd += L" --url \"" + KuroAudio::toWideUTF8(dir_u) + L"\"";
                    }

                    std::atomic<bool> monitor_stop{false};
                    auto prog_file = out_wav.string() + ".progress";
                    std::thread monitor_th([this, prog_file, &monitor_stop]() {
                        float last_pct = 8.0f;
                        int idle_ticks = 0;
                        while (!monitor_stop.load() && !cancel_requested.load()) {
                            std::error_code m_ec;
                            float val = -1.0f;
                            if (std::filesystem::exists(prog_file, m_ec)) {
                                try {
                                    std::ifstream pf(prog_file);
                                    if (pf >> val) {
                                        val = std::clamp(val, 5.0f, 99.0f);
                                    }
                                } catch (...) {}
                            }
                            if (val > last_pct) {
                                last_pct = val;
                                idle_ticks = 0;
                            } else {
                                idle_ticks++;
                                if (idle_ticks > 2 && last_pct < 95.0f) {
                                    last_pct = std::min(95.0f, last_pct + 0.3f);
                                }
                            }
                            download_progress_pct.store(std::clamp(last_pct, 0.0f, 99.0f));
                            std::this_thread::sleep_for(std::chrono::milliseconds(180));
                        }
                    });

                    bool ok = RunCancellableHiddenProcess(wcmd, cancel_requested, 180000);
                    monitor_stop.store(true);
                    if (monitor_th.joinable()) monitor_th.join();

                    std::filesystem::remove(prog_file, ec);

                    if (cancel_requested.load()) {
                        std::filesystem::remove(out_wav, ec);
                        download_progress_pct.store(0.0f);
                        is_downloading.store(false);
                        downloading_track_id = "";
                        status_message = "Abdução cancelada pelo DJ.";
                        break;
                    }

                    bool wav_ok = std::filesystem::exists(out_wav, ec) && (std::filesystem::file_size(out_wav, ec) > 250000);

                    if (wav_ok) {
                        double trk_bpm = item.bpm;
                        std::string trk_key = item.key;
                        double trk_dur = item.duration_sec;

                        drwav wav;
                        if (drwav_init_file(&wav, out_wav.string().c_str(), NULL)) {
                            trk_dur = (double)wav.totalPCMFrameCount / (double)wav.sampleRate;
                            drwav_uninit(&wav);
                        }
                        std::string meta_title = "";
                        std::string meta_artist = "";
                        std::string meta_album = "";
                        std::string meta_genre = "";
                        double meta_bpm = 0.0;
                        std::string meta_key = "";
                        double meta_dur = 0.0;

                        if (std::filesystem::exists(out_meta, ec)) {
                            DJLibraryManager::readMetaJson(out_meta, meta_title, meta_artist, meta_album, meta_genre, meta_bpm, meta_key, meta_dur);
                        }
                        DJLibraryManager::extractWavEmbeddedTags(out_wav, meta_title, meta_artist, meta_album, meta_genre, meta_bpm, meta_key);

                        if (meta_bpm > 20.0) trk_bpm = meta_bpm;
                        if (!meta_key.empty()) trk_key = meta_key;
                        if (meta_dur > 1.0) trk_dur = meta_dur;

                        if (DJLibraryManager::isDummyTitle(item.title) && !meta_title.empty()) item.title = meta_title;
                        if (DJLibraryManager::isDummyArtist(item.artist) && !meta_artist.empty()) item.artist = meta_artist;
                        if (item.title.empty() && !meta_title.empty()) item.title = meta_title;
                        if (item.artist.empty() && !meta_artist.empty()) item.artist = meta_artist;

                        std::string final_rgba = out_rgba.string();
                        if (!std::filesystem::exists(out_rgba, ec)) {
                            final_rgba = DJLibraryManager::resolveRgbaCoverArt(out_wav.string(), tid);
                        }

                        if (item.target_deck >= 0 && item.target_deck < 4) {
                            std::lock_guard<std::mutex> lock(pending_mutex);
                            pending_load.active = true;
                            pending_load.deck_id = item.target_deck;
                            pending_load.wav_path = out_wav.string();
                            pending_load.rgba_path = final_rgba;
                            pending_load.title = item.title;
                            pending_load.artist = item.artist;
                            pending_load.bpm = trk_bpm;
                            pending_load.key = trk_key;
                        }

                        DJLibraryTrack updated_trk;
                        updated_trk.id = tid;
                        updated_trk.title = item.title;
                        updated_trk.artist = item.artist;
                        updated_trk.album = !meta_album.empty() ? meta_album : "Abduction Master Crates";
                        updated_trk.bpm = trk_bpm;
                        updated_trk.key = trk_key;
                        updated_trk.duration_sec = trk_dur;
                        updated_trk.subgenre = !meta_genre.empty() ? DJLibraryManager::classifySubgenre(item.artist, item.title, trk_bpm, meta_genre) : DJLibraryManager::classifySubgenre(item.artist, item.title, trk_bpm);
                        updated_trk.folder_id = DJLibraryManager::getFolderIdForSubgenre(updated_trk.subgenre);
                        try {
                            std::ofstream jf(out_json);
                            if (jf.is_open()) {
                                jf << "{\n"
                                   << "  \"title\": \"" << escapeJson(item.title) << "\",\n"
                                   << "  \"artist\": \"" << escapeJson(item.artist) << "\",\n"
                                   << "  \"bpm\": " << trk_bpm << ",\n"
                                   << "  \"key\": \"" << escapeJson(trk_key) << "\",\n"
                                   << "  \"subgenre\": \"" << escapeJson(updated_trk.subgenre) << "\",\n"
                                   << "  \"audio_file\": \"" << escapeJson(clean_artist + " - " + clean_title + ".wav") << "\",\n"
                                   << "  \"analysis_status\": \"ANALYZED_REKORDBOX\"\n"
                                   << "}\n";
                                jf.close();
                            }
                        } catch (...) {}
                        updated_trk.local_wav_path = rel_wav;
                        updated_trk.local_rgba_path = rel_rgba;
                        updated_trk.is_downloaded = true;
                        updated_trk.is_downloading = false;
                        DJLibraryManager::getInstance().addOrUpdateTrack(updated_trk);

                        download_progress_pct.store(100.0f);
                        if (total_cnt > 1) {
                            status_message = "[" + std::to_string(cur_idx) + "/" + std::to_string(total_cnt) + "] Pronto: '" + item.title + "' abduzida!";
                        } else if (item.target_deck >= 0) {
                            status_message = "Pronto: '" + item.title + "' carregada no Deck " + std::to_string(item.target_deck + 1);
                        } else {
                            status_message = "Pronto: '" + item.title + "' abduzida para Crates!";
                        }
                    } else {
                        download_progress_pct.store(0.0f);
                        status_message = "Falha no canal de download de '" + item.title + "'.";
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                }

                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    if (download_queue.empty()) {
                        queue_total_count.store(0);
                        queue_current_index.store(0);
                    }
                }
                is_downloading.store(false);
                downloading_track_id = "";
                download_progress_pct.store(0.0f);
                is_queue_running.store(false);
                cancel_requested.store(false);
            }).detach();
        }

        void enqueueDownloadItem(const DownloadQueueItem& item) {
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                download_queue.push_back(item);
                queue_total_count.fetch_add(1);
            }
            if (is_downloading.load()) {
                status_message = "Faixa '" + item.title + "' adicionada à fila (" + std::to_string(download_queue.size()) + " aguardando)...";
            }
            startQueueWorkerIfNeeded();
        }

        void enqueueDownload(const SpotifyDJTrack& trk, int target_deck = -1) {
            DownloadQueueItem item;
            item.id = trk.id;
            item.title = trk.title;
            item.artist = trk.artist;
            item.album = trk.album;
            item.artwork_url = trk.artwork_url;
            item.preview_url = trk.preview_url;
            item.direct_url = trk.direct_url;
            item.subgenre = trk.subgenre;
            item.duration_sec = trk.duration_sec > 0 ? trk.duration_sec : 180.0;
            item.bpm = trk.bpm > 0 ? trk.bpm : 128.0;
            item.key = trk.key.empty() ? "8A / Am" : trk.key;
            item.target_deck = target_deck;
            enqueueDownloadItem(item);
        }

        void enqueueLibraryTrack(const DJLibraryTrack& trk, int target_deck = -1) {
            DownloadQueueItem item;
            item.id = trk.id;
            item.title = trk.title;
            item.artist = trk.artist;
            item.album = trk.album;
            item.artwork_url = trk.local_rgba_path.empty() ? trk.artwork_url : trk.local_rgba_path;
            item.preview_url = trk.preview_url;
            item.direct_url = trk.direct_url;
            item.subgenre = trk.subgenre;
            item.duration_sec = trk.duration_sec > 0 ? trk.duration_sec : 180.0;
            item.bpm = trk.bpm > 0 ? trk.bpm : 128.0;
            item.key = trk.key.empty() ? "8A / Am" : trk.key;
            item.target_deck = target_deck;
            enqueueDownloadItem(item);
        }

        void enqueueCrateDownload(const std::vector<SpotifyDJTrack>& tracks_list) {
            int added = 0;
            auto tracks_dir = getTracksDirectory();
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                for (const auto& trk : tracks_list) {
                    auto out_wav = tracks_dir / ("track_" + trk.id + ".wav");
                    std::error_code ec;
                    if (std::filesystem::exists(out_wav, ec) && std::filesystem::file_size(out_wav, ec) > 250000) {
                        continue;
                    }
                    DownloadQueueItem item;
                    item.id = trk.id;
                    item.title = trk.title;
                    item.artist = trk.artist;
                    item.album = trk.album;
                    item.artwork_url = trk.artwork_url;
                    item.preview_url = trk.preview_url;
                    item.direct_url = trk.direct_url;
                    item.subgenre = trk.subgenre;
                    item.duration_sec = trk.duration_sec > 0 ? trk.duration_sec : 180.0;
                    item.bpm = trk.bpm > 0 ? trk.bpm : 128.0;
                    item.key = trk.key.empty() ? "8A / Am" : trk.key;
                    item.target_deck = -1;
                    download_queue.push_back(item);
                    added++;
                }
                queue_total_count.fetch_add(added);
            }
            if (added > 0) {
                status_message = "Crate enfileirado: " + std::to_string(added) + " faixas adicionadas à fila de abdução!";
                startQueueWorkerIfNeeded();
            } else {
                status_message = "Todas as faixas deste Crate já estão abduzidas na biblioteca!";
            }
        }

        void enqueueLibraryCrateDownload(const std::vector<DJLibraryTrack>& tracks_list) {
            int added = 0;
            auto tracks_dir = getTracksDirectory();
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                for (const auto& trk : tracks_list) {
                    auto out_wav = tracks_dir / ("track_" + trk.id + ".wav");
                    std::error_code ec;
                    if (trk.is_downloaded || (std::filesystem::exists(out_wav, ec) && std::filesystem::file_size(out_wav, ec) > 250000)) {
                        continue;
                    }
                    DownloadQueueItem item;
                    item.id = trk.id;
                    item.title = trk.title;
                    item.artist = trk.artist;
                    item.album = trk.album;
                    item.artwork_url = trk.local_rgba_path.empty() ? trk.artwork_url : trk.local_rgba_path;
                    item.preview_url = trk.preview_url;
                    item.direct_url = trk.direct_url;
                    item.subgenre = trk.subgenre;
                    item.duration_sec = trk.duration_sec > 0 ? trk.duration_sec : 180.0;
                    item.bpm = trk.bpm > 0 ? trk.bpm : 128.0;
                    item.key = trk.key.empty() ? "8A / Am" : trk.key;
                    item.target_deck = -1;
                    download_queue.push_back(item);
                    added++;
                }
                queue_total_count.fetch_add(added);
            }
            if (added > 0) {
                status_message = "Crate enfileirado: " + std::to_string(added) + " faixas adicionadas à fila de abdução!";
                startQueueWorkerIfNeeded();
            } else {
                status_message = "Todas as faixas deste Crate já estão abduzidas na biblioteca!";
            }
        }

        void downloadTrackAsync(SpotifyDJTrack trk, int target_deck = -1, KuroAudio::DJEngine* engine = nullptr) {
            std::string resolved = resolveAudioFilePath(trk.local_wav_path, trk.id, trk.title);
            if (!resolved.empty()) {
                trk.local_wav_path = resolved;
                trk.is_ready = true;
                trk.is_downloaded = true;
                if (engine) loadDirect(trk, target_deck, *engine);
                return;
            }
            enqueueDownload(trk, target_deck);
        }

        void downloadLibraryTrackAsync(DJLibraryTrack& trk, int target_deck, KuroAudio::DJEngine& engine) {
            std::string resolved = resolveAudioFilePath(trk.local_wav_path, trk.id, trk.title);
            if (!resolved.empty()) {
                trk.local_wav_path = resolved;
                trk.is_downloaded = true;
                loadLibraryTrack(trk, target_deck, engine);
                return;
            }
            enqueueLibraryTrack(trk, target_deck);
        }

        void pollPendingLoads(KuroAudio::DJEngine& engine) {
            std::lock_guard<std::mutex> lock(pending_mutex);
            if (!pending_load.active) return;

            int d_idx = (pending_load.deck_id >= 0 && pending_load.deck_id < 4) ? pending_load.deck_id : 0;
            auto& deck = engine.decks[d_idx];
            if (deck.loadTrack(pending_load.wav_path)) {
                deck.track_title = pending_load.title;
                deck.track_artist = pending_load.artist;
                if (deck.bpm <= 20.0 && pending_load.bpm > 20.0) {
                    deck.bpm = pending_load.bpm;
                }
                if (!pending_load.key.empty()) {
                    deck.key_signature = pending_load.key;
                } else if (!deck.key_signature.empty()) {
                    pending_load.key = deck.key_signature;
                }
                deck.cover_art_path = pending_load.rgba_path;

                GLuint tex = UploadRGBATexture(pending_load.rgba_path);
                deck.gl_cover_texture = tex;
                deck.has_cover_texture = (tex != 0);

                KuroUtils::Log("[Spotify DJ] Faixa carregada no Deck " + std::to_string(d_idx + 1));
            }
            pending_load.active = false;
        }

        std::vector<SpotifyDJTrack> getTracks() {
            std::lock_guard<std::mutex> lock(tracks_mutex);
            return tracks;
        }

        bool isSearching() const { return is_searching.load(); }
        bool isDownloading() const { return is_downloading.load(); }
        float getDownloadProgressPct() const { return std::clamp(download_progress_pct.load(), 0.0f, 100.0f); }
        void setDownloadProgressPct(float p) { download_progress_pct.store(std::clamp(p, 0.0f, 100.0f)); }
        std::string getDownloadingTrackId() const { return downloading_track_id; }
        std::string getStatus() const { return status_message; }

        void cancelCurrentDownload() {
            cancel_requested.store(true);
            status_message = "Cancelando abdução...";
        }

        void clearDownloadQueue() {
            std::lock_guard<std::mutex> lock(queue_mutex);
            download_queue.clear();
            queue_total_count.store(0);
            queue_current_index.store(0);
            cancel_requested.store(true);
            status_message = "Fila de abdução limpa.";
        }

        int getQueueRemainingCount() {
            std::lock_guard<std::mutex> lock(queue_mutex);
            return (int)download_queue.size();
        }

        int getQueueTotalCount() const { return queue_total_count.load(); }
        int getQueueCurrentIndex() const { return queue_current_index.load(); }
        bool isQueueActive() const { return is_queue_running.load() || is_downloading.load(); }

        // Gestão de Fontes e Alternativas sem Propaganda
        bool getShowSourcesModal() const { return show_sources_modal; }
        void setShowSourcesModal(bool show) { show_sources_modal = show; }
        bool isSearchingSources() const { return is_searching_sources.load(); }
        bool isPreviewingSource() const { return is_previewing_source.load(); }
        std::string getPreviewingUrl() const { return previewing_source_url; }
        std::string getSourcesTrackTitle() const { return sources_track_title; }
        std::string getSourcesTrackArtist() const { return sources_track_artist; }
        std::string getSourcesTrackId() const { return sources_track_id; }
        void setSourcesTrackInfo(const std::string& id, const std::string& title, const std::string& artist) {
            sources_track_id = id;
            sources_track_title = title;
            sources_track_artist = artist;
        }

        std::vector<DJTrackSource> getCurrentSources() {
            std::lock_guard<std::mutex> lock(sources_mutex);
            return current_sources;
        }

        void searchSourcesAsync(const std::string& track_id, const std::string& title, const std::string& artist) {
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
            status_message = "Buscando fontes disponíveis para: " + title;

            std::thread([this, track_id, title, artist]() {
                auto py_path = getPythonExecutablePath();
                auto worker_path = getWorkerScriptPath();
                auto tracks_dir = getTracksDirectory();
                auto out_json = tracks_dir / ("sources_" + track_id + ".json");
                std::error_code ec;
                std::filesystem::remove(out_json, ec);

                std::string q = artist + " " + title;
                std::wstring wcmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" search_sources"
                    + L" --query \"" + KuroAudio::toWideUTF8(q) + L"\""
                    + L" --out_json \"" + out_json.wstring() + L"\"";

                RunHiddenProcess(wcmd, 35000);

                if (std::filesystem::exists(out_json, ec)) {
                    parseSourcesJsonFile(out_json);
                } else {
                    status_message = "Nenhuma fonte adicional encontrada para: " + title;
                }
                is_searching_sources = false;
            }).detach();
        }

        void parseSourcesJsonFile(const std::filesystem::path& json_path) {
            std::ifstream f(json_path);
            if (!f.is_open()) return;
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            std::vector<DJTrackSource> parsed;

            size_t pos = 0;
            while ((pos = content.find('{', pos)) != std::string::npos) {
                size_t end_obj = content.find('}', pos);
                if (end_obj == std::string::npos) break;
                std::string obj = content.substr(pos, end_obj - pos + 1);
                pos = end_obj + 1;

                auto get_field = [&](const std::string& key) -> std::string {
                    std::string needle = "\"" + key + "\":";
                    size_t kpos = obj.find(needle);
                    if (kpos == std::string::npos) return "";
                    size_t vstart = obj.find_first_not_of(" \t\r\n", kpos + needle.length());
                    if (vstart == std::string::npos) return "";
                    if (obj[vstart] == '\"') {
                        size_t vend = obj.find('\"', vstart + 1);
                        if (vend != std::string::npos) return obj.substr(vstart + 1, vend - vstart - 1);
                    } else {
                        size_t vend = obj.find_first_of(",}\r\n", vstart);
                        if (vend != std::string::npos) return obj.substr(vstart, vend - vstart);
                    }
                    return "";
                };

                DJTrackSource s;
                s.id = get_field("id");
                s.title = get_field("title");
                s.channel = get_field("channel");
                s.url = get_field("url");
                s.duration_formatted = get_field("duration_formatted");
                try { s.duration_sec = std::stod(get_field("duration_sec")); } catch (...) { s.duration_sec = 0.0; }
                s.is_official = (obj.find("\"is_official\": true") != std::string::npos);
                s.badge = get_field("badge");
                if (s.badge.empty()) s.badge = s.is_official ? "OFICIAL (TOPIC)" : "AUDIO";
                s.is_clean = (obj.find("\"is_clean\": false") == std::string::npos);

                try { 
                    std::string ps = get_field("purity_score");
                    if (!ps.empty()) s.purity_score = std::stoi(ps);
                    else {
                        if (s.badge.find("STUDIO MASTER") != std::string::npos) s.purity_score = 98;
                        else if (s.badge.find("TIDAL") != std::string::npos) s.purity_score = 94;
                        else if (s.badge.find("SOUNDCLOUD") != std::string::npos) s.purity_score = 86;
                        else if (s.badge.find("TOPIC") != std::string::npos) s.purity_score = 82;
                        else if (s.badge.find("SPOTIFY") != std::string::npos) s.purity_score = 76;
                        else s.purity_score = s.is_clean ? 70 : 25;
                    }
                } catch (...) { s.purity_score = s.is_clean ? 70 : 25; }

                s.sample_rate = get_field("sample_rate");
                if (s.sample_rate.empty()) {
                    if (s.purity_score >= 95) s.sample_rate = "48.0 kHz";
                    else if (s.purity_score >= 90) s.sample_rate = "96.0 kHz";
                    else s.sample_rate = "44.1 kHz";
                }

                s.bit_depth = get_field("bit_depth");
                if (s.bit_depth.empty()) {
                    if (s.purity_score >= 90) s.bit_depth = "24-bit";
                    else if (s.purity_score >= 70) s.bit_depth = "16-bit";
                    else s.bit_depth = "Compressed";
                }

                s.format_name = get_field("format_name");
                if (s.format_name.empty()) {
                    if (s.purity_score >= 95) s.format_name = "FLAC Master";
                    else if (s.purity_score >= 90) s.format_name = "MQA Lossless";
                    else if (s.purity_score >= 80) s.format_name = "Opus / WAV HQ";
                    else s.format_name = "Standard AAC";
                }

                if (!s.id.empty() && !s.title.empty()) {
                    parsed.push_back(s);
                }
            }

            {
                std::lock_guard<std::mutex> lock(sources_mutex);
                current_sources = parsed;
            }
            status_message = "Encontradas " + std::to_string(parsed.size()) + " fontes disponíveis!";
        }

        void previewSourceAsync(const std::string& source_url, const std::string& title, const std::string& channel) {
            status_message = "Carregando preview da fonte (" + channel + ")...";
            previewing_source_url = source_url;
            is_previewing_source = true;
            auto& prev = DJPreviewPlayer::getInstance();
            prev.stop();

            std::thread([this, source_url, title, channel]() {
                auto py_path = getPythonExecutablePath();
                auto worker_path = getWorkerScriptPath();
                auto tracks_dir = getTracksDirectory();
                auto out_wav = tracks_dir / "temp_source_preview.wav";
                std::error_code ec;
                std::filesystem::remove(out_wav, ec);

                std::wstring wcmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" preview_source"
                    + L" --url \"" + KuroAudio::toWideUTF8(source_url) + L"\""
                    + L" --out_wav \"" + out_wav.wstring() + L"\"";

                bool ok = RunHiddenProcess(wcmd, 35000);
                if (ok && std::filesystem::exists(out_wav, ec) && std::filesystem::file_size(out_wav, ec) > 1000) {
                    DJPreviewPlayer::getInstance().play(out_wav.generic_string(), source_url, title + " [" + channel + "]", channel);
                    status_message = "Tocando preview da fonte: " + channel;
                } else {
                    status_message = "Falha ao gerar preview da fonte selecionada.";
                    previewing_source_url = "";
                }
                is_previewing_source = false;
            }).detach();
        }

        void downloadSourceDirectAsync(const std::string& track_id, const std::string& title, const std::string& artist, const std::string& source_url, int target_deck, const std::string& subgenre = "") {
            auto tracks_dir = getTracksDirectory();
            std::string clean_artist = sanitizeFilename(artist.empty() ? "Various Artists" : artist);
            std::string clean_title = sanitizeFilename(title.empty() ? track_id : title);
            std::string folder_name = clean_artist + " - " + clean_title;
            auto track_dir = tracks_dir / folder_name;
            std::error_code ec_dir;
            std::filesystem::create_directories(track_dir, ec_dir);
            std::filesystem::create_directories(track_dir / "stems", ec_dir);
            std::filesystem::create_directories(track_dir / "samples", ec_dir);
            std::filesystem::create_directories(track_dir / "presets", ec_dir);

            auto out_wav = track_dir / (clean_artist + " - " + clean_title + ".wav");
            auto out_rgba = track_dir / "artwork.rgba";
            auto out_meta = track_dir / (clean_artist + " - " + clean_title + ".meta");
            auto out_json = track_dir / "metadata.json";
            std::string rel_wav = "scratch/tracks/" + folder_name + "/" + clean_artist + " - " + clean_title + ".wav";
            std::string rel_rgba = "scratch/tracks/" + folder_name + "/artwork.rgba";

            // Fallback para arquivo solto caso já existisse antes
            auto old_wav = tracks_dir / ("track_" + track_id + ".wav");
            if (!std::filesystem::exists(out_wav, ec_dir) && std::filesystem::exists(old_wav, ec_dir)) {
                out_wav = old_wav;
            }

            // Limpa arquivos anteriores se o DJ solicitou explicitamente abduzir esta fonte
            std::error_code ec_fast;
            std::filesystem::remove(out_wav, ec_fast);
            std::filesystem::remove(out_meta, ec_fast);
            if (false) {
                double trk_bpm = 128.0;
                std::string trk_key = "8A / Am";
                if (std::filesystem::exists(out_meta, ec_fast)) {
                    try {
                        std::ifstream mf(out_meta);
                        std::string mstr((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
                        auto b_pos = mstr.find("\"bpm\":");
                        if (b_pos != std::string::npos) trk_bpm = std::stod(mstr.substr(b_pos + 6));
                        auto k_pos = mstr.find("\"key\":");
                        if (k_pos != std::string::npos) {
                            auto q1 = mstr.find("\"", k_pos + 6);
                            auto q2 = mstr.find("\"", q1 + 1);
                            if (q1 != std::string::npos && q2 != std::string::npos) {
                                trk_key = mstr.substr(q1 + 1, q2 - q1 - 1);
                            }
                        }
                    } catch (...) {}
                }

                if (target_deck >= 0 && target_deck < 4) {
                    std::lock_guard<std::mutex> lock(pending_mutex);
                    pending_load.active = true;
                    pending_load.deck_id = target_deck;
                    pending_load.wav_path = out_wav.generic_string();
                    pending_load.rgba_path = out_rgba.generic_string();
                    pending_load.title = title;
                    pending_load.artist = artist;
                    pending_load.bpm = trk_bpm;
                    pending_load.key = trk_key;
                }

                DJLibraryTrack updated_trk;
                updated_trk.id = track_id;
                updated_trk.title = title;
                updated_trk.artist = artist;
                updated_trk.bpm = trk_bpm;
                updated_trk.key = trk_key;
                updated_trk.subgenre = DJLibraryManager::classifySubgenre(artist, title, trk_bpm);
                updated_trk.folder_id = DJLibraryManager::getFolderIdForSubgenre(updated_trk.subgenre);
                updated_trk.local_wav_path = out_wav.generic_string();
                updated_trk.local_rgba_path = out_rgba.generic_string();
                updated_trk.is_downloaded = true;
                updated_trk.is_downloading = false;
                DJLibraryManager::getInstance().addOrUpdateTrack(updated_trk);

                download_progress_pct.store(100.0f);
                if (target_deck >= 0) {
                    status_message = "Pronto: '" + title + "' carregada no Deck " + std::to_string(target_deck + 1);
                } else {
                    status_message = "Pronto: '" + title + "' abduzida para Crates (Master em disco)!";
                }
                is_downloading = false;
                downloading_track_id = "";
                return;
            }

            DownloadQueueItem item;
            item.id = track_id;
            item.title = title;
            item.artist = artist;
            item.album = "Abduction Master Crates";
            item.artwork_url = out_rgba.generic_string();
            item.preview_url = "";
            item.direct_url = source_url;
            item.subgenre = subgenre.empty() ? DJLibraryManager::classifySubgenre(artist, title, 128.0) : subgenre;
            item.duration_sec = 180.0;
            item.bpm = 128.0;
            item.key = "8A / Am";
            item.target_deck = target_deck;
            enqueueDownloadItem(item);
        }

        // =========================================================================
        // DIG DEEPER // CRATE DIGGER ACÚSTICO AUTO-EVOLUTIVO
        // =========================================================================
        bool isDiggingDeeper() const { return is_digging_deeper.load(); }

        std::vector<DigDeeperMatch> getDigDeeperResults() {
            std::lock_guard<std::mutex> lock(dig_deeper_mutex);
            return dig_deeper_results;
        }

        std::string getDigDeeperRefTitle() const { return dig_deeper_ref_title; }
        std::string getDigDeeperRefArtist() const { return dig_deeper_ref_artist; }
        std::string getDigDeeperRefKey() const { return dig_deeper_ref_key; }
        double getDigDeeperRefBpm() const { return dig_deeper_ref_bpm; }

        void parseDigDeeperJsonFile(const std::filesystem::path& json_path) {
            std::ifstream f(json_path);
            if (!f.is_open()) return;
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            std::vector<DigDeeperMatch> parsed;

            // 1. Extrai referência se existir
            size_t ref_pos = content.find("\"reference\":");
            if (ref_pos != std::string::npos) {
                size_t robj_start = content.find('{', ref_pos);
                size_t robj_end = content.find('}', robj_start);
                if (robj_start != std::string::npos && robj_end != std::string::npos) {
                    std::string robj = content.substr(robj_start, robj_end - robj_start + 1);
                    auto get_rfield = [&](const std::string& key) -> std::string {
                        std::string needle = "\"" + key + "\":";
                        size_t kpos = robj.find(needle);
                        if (kpos == std::string::npos) return "";
                        size_t vstart = robj.find_first_not_of(" \t\r\n", kpos + needle.length());
                        if (vstart == std::string::npos) return "";
                        if (robj[vstart] == '\"') {
                            size_t vend = robj.find('\"', vstart + 1);
                            if (vend != std::string::npos) return robj.substr(vstart + 1, vend - vstart - 1);
                        } else {
                            size_t vend = robj.find_first_of(",}\r\n", vstart);
                            if (vend != std::string::npos) return robj.substr(vstart, vend - vstart);
                        }
                        return "";
                    };
                    std::string rt = get_rfield("title");
                    if (!rt.empty()) dig_deeper_ref_title = rt;
                    std::string ra = get_rfield("artist");
                    if (!ra.empty()) dig_deeper_ref_artist = ra;
                    std::string rk = get_rfield("key");
                    if (!rk.empty()) dig_deeper_ref_key = rk;
                    std::string rb = get_rfield("bpm");
                    if (!rb.empty()) { try { dig_deeper_ref_bpm = std::stod(rb); } catch (...) {} }
                }
            }

            // 2. Extrai array results
            size_t res_array = content.find("\"results\":");
            size_t pos = (res_array != std::string::npos) ? res_array : 0;
            while ((pos = content.find('{', pos)) != std::string::npos) {
                if (ref_pos != std::string::npos && pos >= ref_pos && pos < ref_pos + 250) {
                    pos++;
                    continue;
                }
                size_t end_obj = content.find('}', pos);
                if (end_obj == std::string::npos) break;
                std::string obj = content.substr(pos, end_obj - pos + 1);
                pos = end_obj + 1;

                auto get_field = [&](const std::string& key) -> std::string {
                    std::string needle = "\"" + key + "\":";
                    size_t kpos = obj.find(needle);
                    if (kpos == std::string::npos) return "";
                    size_t vstart = obj.find_first_not_of(" \t\r\n", kpos + needle.length());
                    if (vstart == std::string::npos) return "";
                    if (obj[vstart] == '\"') {
                        size_t vend = obj.find('\"', vstart + 1);
                        if (vend != std::string::npos) return obj.substr(vstart + 1, vend - vstart - 1);
                    } else {
                        size_t vend = obj.find_first_of(",}\r\n", vstart);
                        if (vend != std::string::npos) return obj.substr(vstart, vend - vstart);
                    }
                    return "";
                };

                DigDeeperMatch m;
                m.id = get_field("id");
                m.title = get_field("title");
                m.artist = get_field("artist");
                m.album = get_field("album");
                try { m.bpm = std::stod(get_field("bpm")); } catch (...) { m.bpm = 145.0; }
                try { m.bpm_drift_pct = std::stod(get_field("bpm_drift_pct")); } catch (...) { m.bpm_drift_pct = 0.0; }
                m.key = get_field("key");
                m.camelot_code = get_field("camelot_code");
                m.harmonic_relation = get_field("harmonic_relation");
                m.harmonic_tag = get_field("harmonic_tag");
                m.subgenre = get_field("subgenre");
                m.provider = get_field("provider");
                m.url = get_field("url");
                m.artwork_url = get_field("artwork_url");
                try { m.acoustic_match_pct = std::stoi(get_field("acoustic_match_pct")); } catch (...) { m.acoustic_match_pct = 95; }
                try { m.groove_match_pct = std::stoi(get_field("groove_match_pct")); } catch (...) { m.groove_match_pct = 95; }
                m.match_badge = get_field("match_badge");
                try { m.play_count = std::stoi(get_field("play_count")); } catch (...) { m.play_count = 0; }

                if (!m.id.empty() && !m.title.empty()) {
                    parsed.push_back(m);
                }
            }

            {
                std::lock_guard<std::mutex> lock(dig_deeper_mutex);
                dig_deeper_results = parsed;
            }
            status_message = "Dig Deeper: " + std::to_string(parsed.size()) + " almas gêmeas acústicas encontradas!";
        }

        void digDeeperAsync(const std::string& query, const std::string& artist = "", const std::string& key = "", double bpm = 0.0) {
            if (is_digging_deeper.load()) return;
            is_digging_deeper.store(true);
            dig_deeper_ref_title = query;
            dig_deeper_ref_artist = artist;
            dig_deeper_ref_key = key;
            dig_deeper_ref_bpm = bpm;
            status_message = "🔮 Dig Deeper: minerando almas gêmeas acústicas para '" + query + "'...";
            show_dig_deeper_view = true;

            std::thread([this, query, artist, key, bpm]() {
                auto py_path = getPythonExecutablePath();
                auto worker_path = getWorkerScriptPath();
                auto tracks_dir = getTracksDirectory();
                auto out_json = tracks_dir / "dig_deeper_results.json";

                std::error_code ec;
                std::filesystem::remove(out_json, ec);

                std::wstring wcmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" dig_deeper --query \""
                    + std::filesystem::path(query).wstring() + L"\"";
                if (!artist.empty()) {
                    wcmd += L" --artist \"" + std::filesystem::path(artist).wstring() + L"\"";
                }
                if (!key.empty()) {
                    wcmd += L" --key \"" + std::filesystem::path(key).wstring() + L"\"";
                }
                if (bpm > 0.0) {
                    wcmd += L" --bpm " + std::to_wstring(bpm);
                }
                wcmd += L" --out_json \"" + out_json.wstring() + L"\"";

                bool ok = RunHiddenProcess(wcmd);
                if (ok && std::filesystem::exists(out_json, ec)) {
                    parseDigDeeperJsonFile(out_json);
                } else {
                    status_message = "Dig Deeper: Nenhuma correspondência acústica recente para '" + query + "'.";
                }
                is_digging_deeper.store(false);
            }).detach();
        }

        void recordDigFeedbackAsync(const std::string& ref_id, const std::string& target_id, const std::string& action = "load_deck", int deck = 0) {
            std::thread([this, ref_id, target_id, action, deck]() {
                auto py_path = getPythonExecutablePath();
                auto worker_path = getWorkerScriptPath();
                std::wstring wcmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" dig_feedback --ref \""
                    + std::filesystem::path(ref_id).wstring() + L"\" --target \""
                    + std::filesystem::path(target_id).wstring() + L"\" --feedback_action \""
                    + std::filesystem::path(action).wstring() + L"\" --deck " + std::to_wstring(deck);
                RunHiddenProcess(wcmd);
            }).detach();
        }
    };

} // namespace KuroAudio
