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
#include <GLFW/glfw3.h>
#include "DJEngine.h"
#include "DJLibraryManager.h"
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

    inline std::filesystem::path getAppDirectory() {
#ifdef _WIN32
        wchar_t buffer[MAX_PATH] = { 0 };
        if (GetModuleFileNameW(NULL, buffer, MAX_PATH)) {
            return std::filesystem::path(buffer).parent_path();
        }
#endif
        std::error_code ec;
        return std::filesystem::current_path(ec);
    }

    inline std::filesystem::path getPythonExecutablePath() {
        std::error_code ec;
#ifdef _WIN32
        wchar_t* localappdata = nullptr;
        size_t len = 0;
        if (_wdupenv_s(&localappdata, &len, L"LOCALAPPDATA") == 0 && localappdata != nullptr) {
            std::filesystem::path p_env = std::filesystem::path(localappdata) / L"Python" / L"pythoncore-3.14-64" / L"python.exe";
            free(localappdata);
            if (std::filesystem::exists(p_env, ec)) {
                return p_env;
            }
        }
#endif
        return std::filesystem::path(L"python.exe");
    }

    inline std::filesystem::path getWorkerScriptPath() {
        std::error_code ec;
        auto exe_dir = getAppDirectory();
        
        auto p1 = exe_dir / "tools" / "spotify_dj_worker.py";
        if (std::filesystem::exists(p1, ec)) return p1;

        auto p2 = exe_dir.parent_path().parent_path() / "tools" / "spotify_dj_worker.py";
        if (std::filesystem::exists(p2, ec)) return p2;

        auto p3 = std::filesystem::current_path(ec) / "tools" / "spotify_dj_worker.py";
        if (std::filesystem::exists(p3, ec)) return p3;

        return p1;
    }

    inline std::filesystem::path getTracksDirectory() {
        std::error_code ec;
        auto exe_dir = getAppDirectory();

        auto p1 = exe_dir.parent_path().parent_path() / "scratch" / "tracks";
        if (std::filesystem::exists(p1, ec)) return p1;

        auto p2 = exe_dir / "scratch" / "tracks";
        if (std::filesystem::exists(p2, ec)) return p2;

        auto p3 = std::filesystem::current_path(ec) / "scratch" / "tracks";
        if (std::filesystem::exists(p3, ec)) return p3;

        std::filesystem::create_directories(p2, ec);
        return p2;
    }

    inline bool RunHiddenProcess(const std::wstring& cmd, int timeout_ms = 90000) {
#ifdef _WIN32
        if (cmd.empty()) return false;

        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        std::vector<wchar_t> cmd_buf(cmd.begin(), cmd.end());
        cmd_buf.push_back(L'\0');

        BOOL ok = CreateProcessW(
            NULL,
            cmd_buf.data(),
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi
        );

        if (!ok) {
            return false;
        }

        WaitForSingleObject(pi.hProcess, timeout_ms);
        DWORD exit_code = 1;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return (exit_code == 0);
#else
        return false;
#endif
    }

    struct SpotifyDJTrack {
        std::string id;
        std::string title;
        std::string artist;
        std::string album;
        std::string artwork_url;
        std::string preview_url;
        std::string subgenre = "Tech House";
        double duration_sec = 180.0;
        double bpm = 128.0;
        std::string key = "8A / Am";

        std::string local_wav_path;
        std::string local_rgba_path;
        GLuint gl_tex_id = 0;
        bool is_downloading = false;
        bool is_ready = false;
    };

    class SpotifyDJService {
    private:
        std::vector<SpotifyDJTrack> tracks;
        std::mutex tracks_mutex;
        std::vector<SpotifyDJTrack> search_results;
        std::mutex search_mutex;
        std::atomic<bool> is_searching{ false };
        std::atomic<bool> is_downloading{ false };
        std::string status_message = "Pronto para buscar no Spotify";
        bool is_initialized = false;

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
        // Construtor trivial: zero risco de crash durante inicialização estática C++
        SpotifyDJService() = default;

        ~SpotifyDJService() = default;

        void ensureInitialized() {
            if (is_initialized) return;
            is_initialized = true;
            try {
                DJLibraryManager::getInstance().scanLocalTracks(getTracksDirectory().string());
                initCuratedAnthems();
                std::error_code ec;
                auto tsv = getTracksDirectory() / "search_results.tsv";
                if (std::filesystem::exists(tsv, ec)) {
                    parseSearchResultsTsv(tsv);
                }
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
            for (const auto& entry : std::filesystem::directory_iterator(tracks_dir, ec)) {
                if (entry.is_regular_file(ec)) {
                    std::string filename = entry.path().filename().string();
                    if (filename.find("track_") == 0 || filename.find("search_") == 0 || filename == "dj_library.json") {
                        std::filesystem::remove(entry.path(), ec);
                    }
                }
            }
            {
                std::lock_guard<std::mutex> lock(tracks_mutex);
                tracks.clear();
            }
            {
                std::lock_guard<std::mutex> lock(search_mutex);
                search_results.clear();
            }
            DJLibraryManager::getInstance().clearAllDownloadedTracks();
            status_message = "Biblioteca resetada com sucesso: pronta para começar do zero!";
        }

        std::vector<SpotifyDJTrack> getSearchResults() {
            std::lock_guard<std::mutex> lock(search_mutex);
            return search_results;
        }

        void clearSearchResults() {
            std::lock_guard<std::mutex> lock(search_mutex);
            search_results.clear();
        }

        void searchAsync(const std::string& query) {
            if (query.empty() || is_searching.load()) return;
            is_searching = true;
            status_message = "Pesquisando faixas: " + query + "...";

            std::thread([this, query]() {
                auto py_path = getPythonExecutablePath();
                auto worker_path = getWorkerScriptPath();
                auto tracks_dir = getTracksDirectory();

                auto out_json = tracks_dir / "search_results.json";
                auto out_tsv = tracks_dir / "search_results.tsv";

                std::wstring wcmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" search \""
                    + std::filesystem::path(query).wstring() + L"\" \"" + out_json.wstring() + L"\" --tsv \"" + out_tsv.wstring() + L"\"";

                bool ok = RunHiddenProcess(wcmd);

                std::error_code ec;
                if (std::filesystem::exists(out_tsv, ec)) {
                    parseSearchResultsTsv(out_tsv);
                    status_message = "Pronto: faixas encontradas para '" + query + "'";
                } else {
                    status_message = "Falha ao buscar faixas no Spotify/Cloud.";
                }
                is_searching = false;
            }).detach();
        }

        void parseSearchResultsTsv(const std::filesystem::path& tsv_path) {
            try {
                std::error_code ec;
                if (!std::filesystem::exists(tsv_path, ec)) return;
                std::ifstream f(tsv_path);
                if (!f.is_open()) return;
                std::vector<SpotifyDJTrack> new_tracks;
                std::string line;
                auto tracks_dir = getTracksDirectory();

                while (std::getline(f, line)) {
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
                        try { t.bpm = std::stod(cols[7]); } catch (...) { t.bpm = 126.0; }
                        if (cols.size() >= 9) t.key = cols[8]; else t.key = "8A / Am";
                        if (cols.size() >= 10 && !cols[9].empty()) t.subgenre = cols[9];
                        else t.subgenre = DJLibraryManager::classifySubgenre(t.artist, t.title, t.bpm);

                        auto exp_wav = tracks_dir / ("track_" + t.id + ".wav");
                        auto exp_rgba = tracks_dir / ("track_" + t.id + ".rgba");
                        auto exp_meta = tracks_dir / ("track_" + t.id + ".meta");

                        if (std::filesystem::exists(exp_wav, ec)) {
                            auto sz = std::filesystem::file_size(exp_wav, ec);
                            if (sz < 8000000 && sz > 0) {
                                // Arquivo menor que 8MB é um preview antigo incompleto (<45s), descarta
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
                            t.gl_tex_id = UploadRGBATexture(exp_rgba.string());
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

        static double detectBPMFast(const float* pData, drwav_uint64 totalFrames, unsigned int sampleRate, unsigned int channels) {
            if (!pData || totalFrames < 44100 || sampleRate < 1000 || channels == 0) return 128.0;
            size_t maxFrames = std::min((size_t)totalFrames, (size_t)sampleRate * 20); // 20s analysis
            int hop = (int)(sampleRate / 100); // 10ms hop
            if (hop < 1) hop = 1;
            std::vector<float> energy;
            for (size_t f = 0; f < maxFrames; f += hop) {
                float e = 0.0f;
                for (size_t k = 0; k < (size_t)hop && (f + k) < maxFrames; k++) {
                    float s = pData[(f + k) * channels];
                    e += s * s;
                }
                energy.push_back(e);
            }
            if (energy.size() < 200) return 128.0;

            double bestBpm = 128.0;
            float maxCorr = -1.0f;
            for (int bpm = 115; bpm <= 160; bpm++) {
                int lag = (int)((60.0 / bpm) * 100.0);
                if (lag <= 0 || lag >= (int)energy.size() / 2) continue;
                float corr = 0.0f;
                int count = (int)energy.size() - lag;
                for (int i = 0; i < count; i++) {
                    corr += energy[i] * energy[i + lag];
                }
                if (corr > maxCorr) {
                    maxCorr = corr;
                    bestBpm = (double)bpm;
                }
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
            }

            SpotifyDJTrack t;
            t.id = "local_" + std::to_string(std::hash<std::string>{}(filepath));
            t.title = title;
            t.artist = artist;
            t.album = "Do Computador";
            t.local_wav_path = filepath;
            t.is_ready = true;
            t.bpm = 128.0;
            t.key = "8A / Am";
            t.duration_sec = 240.0;

            drwav wav;
            if (drwav_init_file(&wav, filepath.c_str(), NULL)) {
                t.duration_sec = (double)wav.totalPCMFrameCount / (double)wav.sampleRate;
                drwav_uninit(&wav);
            }

            // Análise rápida de BPM via dr_wav
            unsigned int channels = 0, sampleRate = 0;
            drwav_uint64 totalPCMFrameCount = 0;
            float* pData = drwav_open_file_and_read_pcm_frames_f32(filepath.c_str(), &channels, &sampleRate, &totalPCMFrameCount, NULL);
            if (pData) {
                t.bpm = detectBPMFast(pData, totalPCMFrameCount, sampleRate, channels);
                drwav_free(pData, NULL);
            }

            t.subgenre = "Do Computador";

            // Registra no DJLibraryManager EXCLUSIVAMENTE na pasta de músicas do computador
            DJLibraryTrack lt;
            lt.id = t.id;
            lt.title = t.title;
            lt.artist = t.artist;
            lt.album = t.album;
            lt.duration_sec = t.duration_sec;
            lt.bpm = t.bpm;
            lt.key = t.key;
            lt.subgenre = t.subgenre;
            lt.folder_id = "computer_files"; // Pasta dedicada isolada do computador!
            lt.local_wav_path = filepath;
            lt.is_downloaded = true;
            DJLibraryManager::getInstance().addOrUpdateTrack(lt);
            DJLibraryManager::getInstance().setActiveFolderId("computer_files");

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

            if (is_downloading.load()) {
                status_message = "Download em andamento, aguarde...";
                return;
            }

            is_downloading = true;
            trk.is_downloading = true;
            status_message = "Baixando '" + trk.title + "' para Deck " + std::string(target_deck == 0 ? "A" : "B") + "...";

            auto py_path = getPythonExecutablePath();
            auto worker_path = getWorkerScriptPath();
            auto tracks_dir = getTracksDirectory();

            std::string tid = trk.id;
            std::string q = trk.artist + " " + trk.title;
            std::string prev = trk.preview_url;
            std::string art = trk.artwork_url;
            auto out_wav = tracks_dir / ("track_" + tid + ".wav");
            auto out_rgba = tracks_dir / ("track_" + tid + ".rgba");
            auto out_meta = tracks_dir / ("track_" + tid + ".meta");

            std::thread([this, py_path, worker_path, track_idx, tid, q, prev, art, out_wav, out_rgba, out_meta, target_deck, trk_title = trk.title, trk_artist = trk.artist, trk_bpm = trk.bpm, trk_key = trk.key]() mutable {
                std::wstring wcmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" download"
                    + L" --query \"" + std::filesystem::path(q).wstring() + L"\""
                    + L" --wav \"" + out_wav.wstring() + L"\""
                    + L" --art \"" + std::filesystem::path(art).wstring() + L"\""
                    + L" --rgba \"" + out_rgba.wstring() + L"\""
                    + L" --preview \"" + std::filesystem::path(prev).wstring() + L"\"";

                bool ok = RunHiddenProcess(wcmd);

                std::error_code ec;
                bool wav_ok = std::filesystem::exists(out_wav, ec) && (std::filesystem::file_size(out_wav, ec) > 1000);

                if (wav_ok) {
                    if (std::filesystem::exists(out_meta, ec)) {
                        try {
                            std::ifstream mf(out_meta);
                            std::string mstr((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
                            auto b_pos = mstr.find("\"bpm\":");
                            if (b_pos != std::string::npos) {
                                trk_bpm = std::stod(mstr.substr(b_pos + 6));
                            }
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

                    {
                        std::lock_guard<std::mutex> lock(pending_mutex);
                        pending_load.active = true;
                        pending_load.deck_id = target_deck;
                        pending_load.wav_path = out_wav.string();
                        pending_load.rgba_path = out_rgba.string();
                        pending_load.title = trk_title;
                        pending_load.artist = trk_artist;
                        pending_load.bpm = trk_bpm;
                        pending_load.key = trk_key;
                    }
                    {
                        std::lock_guard<std::mutex> lock(tracks_mutex);
                        if (track_idx < (int)tracks.size()) {
                            tracks[track_idx].local_wav_path = out_wav.string();
                            tracks[track_idx].local_rgba_path = out_rgba.string();
                            tracks[track_idx].bpm = trk_bpm;
                            tracks[track_idx].key = trk_key;
                            tracks[track_idx].is_ready = true;
                            tracks[track_idx].is_downloading = false;
                        }
                    }

                    // Atualiza no DJLibraryManager
                    DJLibraryTrack lt;
                    lt.id = tid;
                    lt.title = trk_title;
                    lt.artist = trk_artist;
                    lt.bpm = trk_bpm;
                    lt.key = trk_key;
                    lt.subgenre = DJLibraryManager::classifySubgenre(trk_artist, trk_title, trk_bpm);
                    lt.folder_id = DJLibraryManager::getFolderIdForSubgenre(lt.subgenre);
                    lt.local_wav_path = out_wav.string();
                    lt.local_rgba_path = out_rgba.string();
                    lt.is_downloaded = true;
                    lt.is_downloading = false;
                    DJLibraryManager::getInstance().addOrUpdateTrack(lt);

                    status_message = "Sucesso: '" + trk_title + "' pronta no Deck " + std::to_string(target_deck + 1);
                } else {
                    {
                        std::lock_guard<std::mutex> lock(tracks_mutex);
                        if (track_idx < (int)tracks.size()) {
                            tracks[track_idx].is_downloading = false;
                        }
                    }
                    status_message = "Erro ao baixar faixa '" + trk_title + "'";
                }
                is_downloading = false;
            }).detach();
        }

        void loadDirect(SpotifyDJTrack& trk, int target_deck, KuroAudio::DJEngine& engine) {
            int d_idx = (target_deck >= 0 && target_deck < 4) ? target_deck : 0;
            auto& deck = engine.decks[d_idx];
            if (deck.loadTrack(trk.local_wav_path)) {
                deck.track_title = trk.title;
                deck.track_artist = trk.artist;
                deck.bpm = trk.bpm;
                deck.key_signature = trk.key;
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

        void loadLibraryTrack(DJLibraryTrack& trk, int target_deck, KuroAudio::DJEngine& engine) {
            int d_idx = (target_deck >= 0 && target_deck < 4) ? target_deck : 0;
            auto& deck = engine.decks[d_idx];
            if (deck.loadTrack(trk.local_wav_path)) {
                deck.track_title = trk.title;
                deck.track_artist = trk.artist;
                deck.bpm = trk.bpm;
                deck.key_signature = trk.key;
                deck.cover_art_path = trk.local_rgba_path;

                if (trk.gl_tex_id == 0 && !trk.local_rgba_path.empty()) {
                    trk.gl_tex_id = UploadRGBATexture(trk.local_rgba_path);
                }
                deck.gl_cover_texture = trk.gl_tex_id;
                deck.has_cover_texture = (trk.gl_tex_id != 0);

                status_message = "Carregado no Deck " + std::to_string(d_idx + 1) + ": " + trk.title;
                KuroUtils::Log("[DJ Library] Faixa carregada no Deck " + std::to_string(d_idx + 1) + ": " + trk.title);
            }
        }

        void downloadLibraryTrackAsync(DJLibraryTrack& trk, int target_deck, KuroAudio::DJEngine& engine) {
            std::error_code ec;
            if (trk.is_downloaded && !trk.local_wav_path.empty() && std::filesystem::exists(std::filesystem::path(trk.local_wav_path), ec)) {
                loadLibraryTrack(trk, target_deck, engine);
                return;
            }

            if (is_downloading.load()) {
                status_message = "Download em andamento, aguarde...";
                return;
            }

            is_downloading = true;
            trk.is_downloading = true;
            status_message = "Baixando '" + trk.title + "' para Deck " + std::to_string(target_deck + 1) + "...";

            auto py_path = getPythonExecutablePath();
            auto worker_path = getWorkerScriptPath();
            auto tracks_dir = getTracksDirectory();

            std::string tid = trk.id;
            std::string q = trk.artist + " " + trk.title;
            std::string prev = trk.preview_url;
            std::string art = trk.artwork_url;
            auto out_wav = tracks_dir / ("track_" + tid + ".wav");
            auto out_rgba = tracks_dir / ("track_" + tid + ".rgba");
            auto out_meta = tracks_dir / ("track_" + tid + ".meta");

            std::thread([this, py_path, worker_path, tid, q, prev, art, out_wav, out_rgba, out_meta, target_deck, trk_title = trk.title, trk_artist = trk.artist, trk_bpm = trk.bpm, trk_key = trk.key]() mutable {
                std::wstring wcmd = L"\"" + py_path.wstring() + L"\" \"" + worker_path.wstring() + L"\" download"
                    + L" --query \"" + std::filesystem::path(q).wstring() + L"\""
                    + L" --wav \"" + out_wav.wstring() + L"\""
                    + L" --art \"" + std::filesystem::path(art).wstring() + L"\""
                    + L" --rgba \"" + out_rgba.wstring() + L"\""
                    + L" --preview \"" + std::filesystem::path(prev).wstring() + L"\"";

                bool ok = RunHiddenProcess(wcmd);
                std::error_code ec;
                bool wav_ok = std::filesystem::exists(out_wav, ec) && (std::filesystem::file_size(out_wav, ec) > 1000);

                if (wav_ok) {
                    if (std::filesystem::exists(out_meta, ec)) {
                        try {
                            std::ifstream mf(out_meta);
                            std::string mstr((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
                            auto b_pos = mstr.find("\"bpm\":");
                            if (b_pos != std::string::npos) {
                                trk_bpm = std::stod(mstr.substr(b_pos + 6));
                            }
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

                    {
                        std::lock_guard<std::mutex> lock(pending_mutex);
                        pending_load.active = true;
                        pending_load.deck_id = target_deck;
                        pending_load.wav_path = out_wav.string();
                        pending_load.rgba_path = out_rgba.string();
                        pending_load.title = trk_title;
                        pending_load.artist = trk_artist;
                        pending_load.bpm = trk_bpm;
                        pending_load.key = trk_key;
                    }

                    DJLibraryTrack updated_trk;
                    updated_trk.id = tid;
                    updated_trk.title = trk_title;
                    updated_trk.artist = trk_artist;
                    updated_trk.bpm = trk_bpm;
                    updated_trk.key = trk_key;
                    updated_trk.subgenre = DJLibraryManager::classifySubgenre(trk_artist, trk_title, trk_bpm);
                    updated_trk.folder_id = DJLibraryManager::getFolderIdForSubgenre(updated_trk.subgenre);
                    updated_trk.local_wav_path = out_wav.string();
                    updated_trk.local_rgba_path = out_rgba.string();
                    updated_trk.is_downloaded = true;
                    updated_trk.is_downloading = false;
                    DJLibraryManager::getInstance().addOrUpdateTrack(updated_trk);

                    status_message = "Sucesso: '" + trk_title + "' pronta no Deck " + std::to_string(target_deck + 1);
                } else {
                    status_message = "Erro ao baixar faixa '" + trk_title + "'";
                }
                is_downloading = false;
            }).detach();
        }

        void pollPendingLoads(KuroAudio::DJEngine& engine) {
            std::lock_guard<std::mutex> lock(pending_mutex);
            if (!pending_load.active) return;

            int d_idx = (pending_load.deck_id >= 0 && pending_load.deck_id < 4) ? pending_load.deck_id : 0;
            auto& deck = engine.decks[d_idx];
            if (deck.loadTrack(pending_load.wav_path)) {
                deck.track_title = pending_load.title;
                deck.track_artist = pending_load.artist;
                deck.bpm = pending_load.bpm;
                deck.key_signature = pending_load.key;
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
        std::string getStatus() const { return status_message; }
    };

} // namespace KuroAudio
