#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cctype>
#include "DJCommonUtils.h"
#include "../ai/dr_wav.h"
#include "../ai/dr_mp3.h"

namespace KuroAudio {

    struct DJCrateFolder {
        std::string id;
        std::string name;
        std::string icon = "folder";
        bool is_system = false;
        int track_count = 0;
    };

    struct DJOverviewWavePoint {
        float peak = 0.0f;
        float low  = 0.0f;
        float mid  = 0.0f;
        float high = 0.0f;
    };

    struct DJLibraryTrack {
        std::string id;
        std::string title;
        std::string artist;
        std::string album;
        std::string subgenre;   // "Progressive Psytrance", "Psytrance / Full-On", "Tech House", etc.
        std::string folder_id;  // "prog_psy", "psy_fullon", "tech_house", etc.
        double duration_sec = 240.0;
        double bpm = 128.0;
        std::string key = "8A / Am";
        std::string local_wav_path;
        std::string local_rgba_path;
        std::string artwork_url;
        std::string preview_url;
        std::string direct_url;
        bool is_downloaded = false;
        bool is_downloading = false;
        bool has_played = false;
        int play_count = 0;
        unsigned int gl_tex_id = 0;
        int rating = 0;
        bool grid_locked = true;
        std::vector<DJOverviewWavePoint> overview_wave;
    };

    class DJLibraryManager {
    private:
        std::vector<DJCrateFolder> folders;
        std::vector<DJLibraryTrack> tracks;
        std::string active_folder_id = "all";
        std::mutex lib_mutex;
        std::string db_path = "scratch/tracks/dj_library.json";

        static std::string toLower(const std::string& str) {
            std::string s = str;
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            return s;
        }

    public:
        DJLibraryManager() {
            initDefaultFolders();
        }

        static DJLibraryManager& getInstance() {
            static DJLibraryManager instance;
            return instance;
        }

        void initDefaultFolders() {
            folders.clear();
            // Fluxo & Descoberta Inteligente
            folders.push_back({ "all", "Todas as Músicas", "music", true, 0 });
            folders.push_back({ "recent_downloaded", "Recém Baixadas / Novas", "bolt", true, 0 });
            folders.push_back({ "favorites", "Favoritas (Top Rated)", "star", true, 0 });
            folders.push_back({ "history", "Tocadas no Set", "clock", true, 0 });

            // Pastas & Crates Locais
            folders.push_back({ "first_kuru", "FIRST KURU (Importadas)", "folder", true, 0 });

            // Gêneros & Vertentes Musicais
            folders.push_back({ "prog_psy", "Prog Psytrance", "fire", true, 0 });
            folders.push_back({ "fullon_night", "Full-On Night & Twilight", "moon", true, 0 });
            folders.push_back({ "psy_fullon", "Full-On Morning", "bolt", true, 0 });
            folders.push_back({ "tech_house", "Tech House & Grooves", "compact-disc", true, 0 });
            folders.push_back({ "farofa", "Farofa & Brazilian Bass", "sun", true, 0 });
            folders.push_back({ "acid_techno", "Acid 303 & Techno", "atom", true, 0 });
            folders.push_back({ "melodic_techno", "Melodic & Deep", "water", true, 0 });
        }

        void clearAllDownloadedTracks() {
            std::lock_guard<std::mutex> lock(lib_mutex);
            tracks.clear();
            updateFolderCounts();
            saveToFile();
        }

        void clearEntireLibrary() {
            std::lock_guard<std::mutex> lock(lib_mutex);
            tracks.clear();
            updateFolderCounts();
            saveToFile();
        }

        // Classificador Cirúrgico de Vertentes da Música Eletrônica
        static std::string classifySubgenre(const std::string& artist, const std::string& title, double bpm = 128.0, const std::string& query = "") {
            std::string a = toLower(artist);
            std::string t = toLower(title);
            std::string q = toLower(query);
            std::string combined = a + " " + t + " " + q;

            // 1. Farofa & Brazilian Bass / EDM / Funk Remix (Alok, KVSH, Dubdogz, Cat Dealers, Liu, Bhaskar, etc.)
            if (combined.find("farofa") != std::string::npos ||
                combined.find("brazilian bass") != std::string::npos ||
                combined.find("brazilian") != std::string::npos ||
                combined.find("eletrofunk") != std::string::npos ||
                combined.find("funk remix") != std::string::npos ||
                combined.find("desande") != std::string::npos ||
                combined.find("alok") != std::string::npos ||
                combined.find("kvsh") != std::string::npos ||
                combined.find("dubdogz") != std::string::npos ||
                combined.find("cat dealers") != std::string::npos ||
                combined.find("liu") != std::string::npos ||
                combined.find("bhaskar") != std::string::npos ||
                combined.find("illusionize") != std::string::npos ||
                combined.find("illuzionize") != std::string::npos ||
                combined.find("grave") != std::string::npos ||
                combined.find("pop remix") != std::string::npos ||
                combined.find("edm") != std::string::npos) {
                return "Farofa & Brazilian Bass";
            }

            // 2. Full-On Night & Twilight / Darkpsy / Forest (Mekkanikka, Audiosonic, Mad Tribe, Alpha Portal, Ajja, etc.)
            if (combined.find("fullon night") != std::string::npos ||
                combined.find("full-on night") != std::string::npos ||
                combined.find("night full-on") != std::string::npos ||
                combined.find("night fullon") != std::string::npos ||
                combined.find("nightpsy") != std::string::npos ||
                combined.find("night psy") != std::string::npos ||
                combined.find("twilight") != std::string::npos ||
                combined.find("darkpsy") != std::string::npos ||
                combined.find("dark psy") != std::string::npos ||
                combined.find("forest") != std::string::npos ||
                combined.find("mekkanikka") != std::string::npos ||
                combined.find("audiosonic") != std::string::npos ||
                combined.find("mad tribe") != std::string::npos ||
                combined.find("alpha portal") != std::string::npos ||
                combined.find("space tribe") != std::string::npos ||
                combined.find("dickster") != std::string::npos ||
                combined.find("dickers") != std::string::npos ||
                combined.find("earthling") != std::string::npos ||
                combined.find("pantomiman") != std::string::npos ||
                combined.find("whiptongue") != std::string::npos ||
                combined.find("render") != std::string::npos ||
                combined.find("siloka") != std::string::npos ||
                combined.find("kabayun") != std::string::npos ||
                combined.find("southwild") != std::string::npos ||
                combined.find("jumpstreet") != std::string::npos ||
                combined.find("fungus funk") != std::string::npos ||
                combined.find("aardvarkk") != std::string::npos ||
                combined.find("aknoyd") != std::string::npos ||
                combined.find("synthetik chaos") != std::string::npos ||
                combined.find("mental projection") != std::string::npos ||
                combined.find("oksha") != std::string::npos ||
                combined.find("farbo") != std::string::npos ||
                combined.find("synthalienz") != std::string::npos) {
                return "Full-On Night & Twilight";
            }

            // 3. Progressive Psytrance (Aura Vortex, Klipsun, Neelix, Phaxe, Blazy, Vegas, etc.)
            if (combined.find("aura vortex") != std::string::npos ||
                combined.find("klipsun") != std::string::npos ||
                combined.find("cliffjumper") != std::string::npos ||
                combined.find("neelix") != std::string::npos ||
                combined.find("phaxe") != std::string::npos ||
                combined.find("morten granau") != std::string::npos ||
                combined.find("blazy") != std::string::npos ||
                combined.find("vegas") != std::string::npos ||
                combined.find("groundbass") != std::string::npos ||
                combined.find("capital monkey") != std::string::npos ||
                combined.find("progressive psy") != std::string::npos ||
                combined.find("prog psy") != std::string::npos ||
                combined.find("progpsy") != std::string::npos ||
                combined.find("prog") != std::string::npos ||
                combined.find("progressive") != std::string::npos) {
                return "Progressive Psytrance";
            }

            // 4. Psytrance / Full-On (Morning) (Astrix, Burn In Noise, Ajja, Laughing Buddha, etc.)
            if (combined.find("astrix") != std::string::npos ||
                combined.find("ajja") != std::string::npos ||
                combined.find("burn in noise") != std::string::npos ||
                combined.find("killerwatts") != std::string::npos ||
                combined.find("freak control") != std::string::npos ||
                combined.find("hypnocoustics") != std::string::npos ||
                combined.find("hypnoise") != std::string::npos ||
                combined.find("spinal fusion") != std::string::npos ||
                combined.find("imaginarium") != std::string::npos ||
                combined.find("laughing buddha") != std::string::npos ||
                combined.find("labirinto") != std::string::npos ||
                combined.find("outsiders") != std::string::npos ||
                combined.find("sixsense") != std::string::npos ||
                combined.find("octogoat") != std::string::npos ||
                combined.find("signa") != std::string::npos ||
                combined.find("psy mr") != std::string::npos ||
                combined.find("tskun") != std::string::npos ||
                combined.find("xprompt") != std::string::npos ||
                combined.find("deep jungle walk") != std::string::npos ||
                combined.find("vini vici") != std::string::npos ||
                combined.find("blastoyz") != std::string::npos ||
                combined.find("infected mushroom") != std::string::npos ||
                combined.find("ace ventura") != std::string::npos ||
                combined.find("liquid soul") != std::string::npos ||
                combined.find("tristan") != std::string::npos ||
                combined.find("avalon") != std::string::npos ||
                combined.find("skazi") != std::string::npos ||
                combined.find("psytrance") != std::string::npos ||
                combined.find("psy-trance") != std::string::npos ||
                combined.find("full on") != std::string::npos ||
                combined.find("full-on") != std::string::npos ||
                combined.find("goa") != std::string::npos ||
                combined.find("morning") != std::string::npos) {
                return "Psytrance / Full-On";
            }

            // 5. Acid Techno / Peak Time (Charlotte de Witte, Amelie Lens, Reinier Zonneveld, etc.)
            if (combined.find("charlotte de witte") != std::string::npos ||
                combined.find("amelie lens") != std::string::npos ||
                combined.find("reinier zonneveld") != std::string::npos ||
                combined.find("enrico sangiuliano") != std::string::npos ||
                combined.find("adam beyer") != std::string::npos ||
                combined.find("lilly palmer") != std::string::npos ||
                combined.find("deborah de luca") != std::string::npos ||
                combined.find("sara landry") != std::string::npos ||
                combined.find("i hate models") != std::string::npos ||
                combined.find("klangkuenstler") != std::string::npos ||
                combined.find("alignment") != std::string::npos ||
                combined.find("acid") != std::string::npos ||
                combined.find("hard techno") != std::string::npos ||
                combined.find("industrial techno") != std::string::npos ||
                combined.find("peak time") != std::string::npos) {
                return "Acid Techno";
            }

            // 6. Tech House / Bass House (Vintage Culture, Fisher, Mau P, James Hype, etc.)
            if (combined.find("vintage culture") != std::string::npos ||
                combined.find("fisher") != std::string::npos ||
                combined.find("mau p") != std::string::npos ||
                combined.find("james hype") != std::string::npos ||
                combined.find("chris lake") != std::string::npos ||
                combined.find("dom dolla") != std::string::npos ||
                combined.find("john summit") != std::string::npos ||
                combined.find("michael bibi") != std::string::npos ||
                combined.find("cloonee") != std::string::npos ||
                combined.find("pawsa") != std::string::npos ||
                combined.find("sidepiece") != std::string::npos ||
                combined.find("matroda") != std::string::npos ||
                combined.find("mochakk") != std::string::npos ||
                combined.find("acraze") != std::string::npos ||
                combined.find("tech house") != std::string::npos ||
                combined.find("bass house") != std::string::npos) {
                return "Tech House";
            }

            // 7. Melodic Techno / Progressive House (ARTBAT, Anyma, Tale of Us, etc.)
            if (combined.find("artbat") != std::string::npos ||
                combined.find("anyma") != std::string::npos ||
                combined.find("tale of us") != std::string::npos ||
                combined.find("mind against") != std::string::npos ||
                combined.find("camelphat") != std::string::npos ||
                combined.find("stephan bodzin") != std::string::npos ||
                combined.find("boris brejcha") != std::string::npos ||
                combined.find("eric prydz") != std::string::npos ||
                combined.find("tinlicker") != std::string::npos ||
                combined.find("lane 8") != std::string::npos ||
                combined.find("melodic techno") != std::string::npos ||
                combined.find("afterlife") != std::string::npos) {
                return "Melodic Techno";
            }

            // 8. Heurística Inteligente por Faixa de BPM
            if (bpm >= 144.0) return "Full-On Night & Twilight";
            if (bpm >= 137.0) return "Psytrance / Full-On";
            if (bpm >= 133.0) return "Progressive Psytrance";
            if (bpm >= 129.0) return "Acid Techno";
            if (bpm >= 123.0) return "Tech House";
            return "Farofa & Brazilian Bass";
        }

        static std::string getFolderIdForSubgenre(const std::string& subgenre) {
            std::string s = toLower(subgenre);
            if (s.find("prog") != std::string::npos) return "prog_psy";
            if (s.find("farofa") != std::string::npos || s.find("brazilian") != std::string::npos || s.find("bass") != std::string::npos) return "farofa";
            if (s.find("night") != std::string::npos || s.find("twilight") != std::string::npos || s.find("dark") != std::string::npos || s.find("forest") != std::string::npos) return "fullon_night";
            if (s.find("morning") != std::string::npos || s.find("full-on") != std::string::npos || s.find("fullon") != std::string::npos || s.find("psy") != std::string::npos) return "psy_fullon";
            if (s.find("tech") != std::string::npos) return "tech_house";
            if (s.find("acid") != std::string::npos) return "acid_techno";
            if (s.find("melodic") != std::string::npos) return "melodic_techno";
            return "all";
        }

        static double estimateBPMHeuristic(const std::string& title, const std::string& artist) {
            std::string s = artist + " " + title;
            std::string lower = toLower(s);

            // Forest & Twilight / Darkpsy (148 - 150 BPM)
            if (lower.find("fungus") != std::string::npos ||
                lower.find("kabayun") != std::string::npos ||
                lower.find("whiptongue") != std::string::npos ||
                lower.find("aardvarkk") != std::string::npos ||
                lower.find("aknoyd") != std::string::npos ||
                lower.find("forest") != std::string::npos ||
                lower.find("twilight") != std::string::npos) return 148.0;

            // Full-On Psytrance (144 - 146 BPM)
            if (lower.find("ajja") != std::string::npos ||
                lower.find("burn in noise") != std::string::npos ||
                lower.find("killerwatts") != std::string::npos ||
                lower.find("freak control") != std::string::npos ||
                lower.find("hypnoise") != std::string::npos ||
                lower.find("hypnocoustics") != std::string::npos ||
                lower.find("imaginarium") != std::string::npos ||
                lower.find("laughing buddha") != std::string::npos ||
                lower.find("labirinto") != std::string::npos ||
                lower.find("outsiders") != std::string::npos ||
                lower.find("sixsense") != std::string::npos ||
                lower.find("octogoat") != std::string::npos ||
                lower.find("signa") != std::string::npos ||
                lower.find("xprompt") != std::string::npos ||
                lower.find("astrix") != std::string::npos ||
                lower.find("deep jungle walk") != std::string::npos ||
                lower.find("vini vici") != std::string::npos ||
                lower.find("blastoyz") != std::string::npos) return 144.0;

            // Progressive Psytrance (138 BPM)
            if (lower.find("cliff") != std::string::npos ||
                lower.find("blazy") != std::string::npos ||
                lower.find("groundbass") != std::string::npos ||
                lower.find("zanon") != std::string::npos ||
                lower.find("lyktum") != std::string::npos) return 138.0;

            if (lower.find("techno") != std::string::npos || lower.find("charlotte") != std::string::npos) return 132.0;
            if (lower.find("vintage culture") != std::string::npos || lower.find("fisher") != std::string::npos || lower.find("bibi") != std::string::npos) return 126.0;
            return 140.0;
        }

        static std::string estimateKeyHeuristic(const std::string& title) {
            static const char* camelot_keys[] = {
                "8A / Am", "9A / Em", "10A / Bm", "11A / F#m", "12A / C#m", "1A / G#m",
                "2A / D#m", "3A / Bbm", "4A / Fm", "5A / Cm", "6A / Gm", "7A / Dm",
                "8B / C",  "9B / G",  "10B / D",  "11B / A",  "12B / E",  "1B / B",
                "2B / F#", "3B / Db", "4B / Ab", "5B / Eb", "6B / Bb", "7B / F"
            };
            size_t h = std::hash<std::string>{}(title);
            return camelot_keys[h % 24];
        }

        static std::string normalizePath(const std::string& p) {
            std::string out = p;
            for (char& c : out) {
                if (c == '\\') c = '/';
            }
            std::string clean;
            bool last_slash = false;
            for (size_t i = 0; i < out.size(); i++) {
                if (out[i] == '/') {
                    if (!last_slash) clean += '/';
                    last_slash = true;
                } else {
                    clean += out[i];
                    last_slash = false;
                }
            }
            return clean;
        }

        static bool isDummyTitle(const std::string& title) {
            if (title.empty()) return true;
            std::string low = toLower(title);
            if (low.rfind("track_online_", 0) == 0 || low.rfind("track_", 0) == 0 || low.rfind("online_", 0) == 0) return true;
            if (low == "temp_source_preview" || low == "untitled" || low == "musica" || low == "audio") return true;
            return false;
        }

        static bool isDummyArtist(const std::string& artist) {
            if (artist.empty()) return true;
            std::string low = toLower(artist);
            if (low == "artista local" || low == "artista" || low == "unknown" || low == "unknown artist" || low == "coleção first kuru") return true;
            return false;
        }

        static std::string resolveRgbaCoverArt(const std::string& wav_path, const std::string& id) {
            std::error_code ec;
            std::vector<std::filesystem::path> cands;

            // 1. Capas específicas prioritárias por ID (Kuro Cloud / Spotify)
            if (!id.empty()) {
                cands.push_back(std::filesystem::path("scratch/tracks") / ("track_online_" + id + ".rgba"));
                cands.push_back(std::filesystem::path("scratch/tracks") / ("track_" + id + ".rgba"));
                cands.push_back(std::filesystem::path("scratch/tracks") / (id + ".rgba"));
                if (id.rfind("online_", 0) == 0) {
                    std::string num = id.substr(7);
                    cands.push_back(std::filesystem::path("scratch/tracks") / ("track_online_" + num + ".rgba"));
                    cands.push_back(std::filesystem::path("scratch/tracks") / ("track_" + num + ".rgba"));
                    cands.push_back(std::filesystem::path("scratch/tracks") / (num + ".rgba"));
                }
                if (id.rfind("track_", 0) == 0) {
                    std::string sub = id.substr(6);
                    cands.push_back(std::filesystem::path("scratch/tracks") / ("track_online_" + sub + ".rgba"));
                    cands.push_back(std::filesystem::path("scratch/tracks") / ("track_" + sub + ".rgba"));
                    cands.push_back(std::filesystem::path("scratch/tracks") / (sub + ".rgba"));
                }
            }

            // 2. Capas específicas por nome de arquivo de áudio (stem.rgba)
            if (!wav_path.empty()) {
                try {
                    std::filesystem::path wp = std::filesystem::u8path(wav_path);
                    auto parent = wp.parent_path();
                    auto stem = wp.stem().string();
                    cands.push_back(parent / (stem + ".rgba"));
                    cands.push_back(std::filesystem::path("scratch/tracks") / (stem + ".rgba"));
                    if (stem.rfind("track_", 0) == 0) {
                        cands.push_back(std::filesystem::path("scratch/tracks") / (stem.substr(6) + ".rgba"));
                    }
                    // Apenas aceita "artwork.rgba" se for em subpasta dedicada da faixa,
                    // evitando contaminação em pastas com dezenas de músicas locais
                    std::string p_name = parent.filename().string();
                    if (p_name == stem || parent.string().find("scratch/tracks") != std::string::npos || parent.string().find("scratch\\tracks") != std::string::npos) {
                        cands.push_back(parent / "artwork.rgba");
                    }
                } catch (...) {}
            }

            for (const auto& c : cands) {
                if (std::filesystem::exists(c, ec)) {
                    return safePathString(c);
                }
            }
            return "";
        }

        static bool loadTrackWaveform(DJLibraryTrack& t) {
            if (!t.overview_wave.empty()) return true;
            std::error_code ec;
            std::vector<std::filesystem::path> cands;
            if (!t.id.empty()) {
                cands.push_back(std::filesystem::path("scratch/tracks/waveforms") / (t.id + ".wf"));
            }
            if (!t.local_wav_path.empty()) {
                try {
                    auto wp = std::filesystem::u8path(t.local_wav_path);
                    cands.push_back(wp.parent_path() / (wp.stem().string() + ".wf"));
                    cands.push_back(std::filesystem::path("scratch/tracks/waveforms") / (wp.stem().string() + ".wf"));
                } catch (...) {}
            }
            for (const auto& c : cands) {
                if (std::filesystem::exists(c, ec)) {
                    std::ifstream f(c, std::ios::binary);
                    if (f.is_open()) {
                        f.seekg(0, std::ios::end);
                        size_t bytes = (size_t)f.tellg();
                        f.seekg(0, std::ios::beg);
                        size_t count = bytes / sizeof(DJOverviewWavePoint);
                        if (count >= 16) {
                            t.overview_wave.resize(count);
                            f.read((char*)t.overview_wave.data(), count * sizeof(DJOverviewWavePoint));
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        static bool readMetaJson(const std::filesystem::path& meta_path,
                                 std::string& out_title,
                                 std::string& out_artist,
                                 std::string& out_album,
                                 std::string& out_genre,
                                 double& out_bpm,
                                 std::string& out_key,
                                 double& out_dur) {
            try {
                std::error_code ec;
                if (!std::filesystem::exists(meta_path, ec)) return false;
                std::ifstream mf(meta_path);
                if (!mf.is_open()) return false;
                std::string mstr((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());

                auto extractStr = [&](const std::string& key) -> std::string {
                    std::string pat = "\"" + key + "\":";
                    size_t p = mstr.find(pat);
                    if (p == std::string::npos) return "";
                    size_t q1 = mstr.find('"', p + pat.length());
                    if (q1 == std::string::npos) return "";
                    size_t q2 = mstr.find('"', q1 + 1);
                    if (q2 == std::string::npos) return "";
                    return mstr.substr(q1 + 1, q2 - q1 - 1);
                };
                auto extractNum = [&](const std::string& key) -> double {
                    std::string pat = "\"" + key + "\":";
                    size_t p = mstr.find(pat);
                    if (p == std::string::npos) return 0.0;
                    size_t s = mstr.find_first_not_of(" \t", p + pat.length());
                    if (s == std::string::npos) return 0.0;
                    size_t e = mstr.find_first_of(",}\r\n", s);
                    if (e == std::string::npos) e = mstr.length();
                    try { return std::stod(mstr.substr(s, e - s)); } catch (...) { return 0.0; }
                };

                std::string t = extractStr("title");
                std::string a = extractStr("artist");
                std::string al = extractStr("album");
                std::string g = extractStr("genre");
                std::string k = extractStr("key");
                double b = extractNum("bpm");
                double d = extractNum("duration_sec");

                if (!t.empty()) out_title = t;
                if (!a.empty()) out_artist = a;
                if (!al.empty()) out_album = al;
                if (!g.empty()) out_genre = g;
                if (!k.empty()) out_key = k;
                if (b > 0.0) out_bpm = b;
                if (d > 0.0) out_dur = d;
                return true;
            } catch (...) {
                return false;
            }
        }

        static bool extractWavEmbeddedTags(const std::filesystem::path& wav_path,
                                           std::string& out_title,
                                           std::string& out_artist,
                                           std::string& out_album,
                                           std::string& out_genre,
                                           double& out_bpm,
                                           std::string& out_key) {
            try {
                std::error_code ec;
                if (!std::filesystem::exists(wav_path, ec)) return false;
                size_t file_size = (size_t)std::filesystem::file_size(wav_path, ec);
                if (file_size < 12) return false;

                std::ifstream f(wav_path, std::ios::binary);
                if (!f.is_open()) return false;

                auto searchInChunk = [&](const std::vector<char>& data) {
                    if (data.size() < 8) return;
                    // 1. Procura chunk id3
                    size_t id3_pos = 0;
                    while (id3_pos + 8 < data.size()) {
                        size_t p = 0;
                        for (p = id3_pos; p + 4 <= data.size(); p++) {
                            if (data[p] == 'i' && data[p+1] == 'd' && data[p+2] == '3' && data[p+3] == ' ') break;
                        }
                        if (p + 4 > data.size()) break;
                        id3_pos = p + 4;
                        size_t sub_end = std::min(data.size(), p + 4096);
                        auto parseId3Frame = [&](const char* fourcc) -> std::string {
                            for (size_t i = p; i + 10 < sub_end; i++) {
                                if (data[i] == fourcc[0] && data[i+1] == fourcc[1] &&
                                    data[i+2] == fourcc[2] && data[i+3] == fourcc[3]) {
                                    uint32_t fsz = ((uint8_t)data[i+4] << 24) |
                                                   ((uint8_t)data[i+5] << 16) |
                                                   ((uint8_t)data[i+6] << 8)  |
                                                   ((uint8_t)data[i+7]);
                                    if (fsz > 0 && fsz < 512 && i + 10 + fsz <= data.size()) {
                                        size_t str_start = i + 11;
                                        size_t str_len = (fsz > 1) ? (fsz - 1) : 0;
                                        std::string val;
                                        for (size_t s = 0; s < str_len; s++) {
                                            char c = data[str_start + s];
                                            if (c != '\0') val += c;
                                        }
                                        return val;
                                    }
                                }
                            }
                            return "";
                        };
                        if (out_title.empty()) out_title = parseId3Frame("TIT2");
                        if (out_artist.empty()) out_artist = parseId3Frame("TPE1");
                        if (out_album.empty()) out_album = parseId3Frame("TALB");
                        if (out_genre.empty()) out_genre = parseId3Frame("TCON");
                        if (out_bpm <= 0.0) {
                            std::string b_str = parseId3Frame("TBPM");
                            if (!b_str.empty()) try { out_bpm = std::stod(b_str); } catch (...) {}
                        }
                        if (out_key.empty()) out_key = parseId3Frame("TKEY");
                    }

                    // 2. Procura chunk LIST-INFO
                    size_t info_pos = 0;
                    while (info_pos + 8 < data.size()) {
                        size_t p = 0;
                        for (p = info_pos; p + 4 <= data.size(); p++) {
                            if (data[p] == 'I' && data[p+1] == 'N' && data[p+2] == 'F' && data[p+3] == 'O') break;
                        }
                        if (p + 4 > data.size()) break;
                        info_pos = p + 4;
                        size_t sub_end = std::min(data.size(), p + 4096);
                        auto parseRiffInfoTag = [&](const char* fourcc) -> std::string {
                            for (size_t i = p; i + 8 < sub_end; i++) {
                                if (data[i] == fourcc[0] && data[i+1] == fourcc[1] &&
                                    data[i+2] == fourcc[2] && data[i+3] == fourcc[3]) {
                                    uint32_t ssz = ((uint8_t)data[i+4]) |
                                                   ((uint8_t)data[i+5] << 8) |
                                                   ((uint8_t)data[i+6] << 16) |
                                                   ((uint8_t)data[i+7] << 24);
                                    if (ssz > 0 && ssz < 512 && i + 8 + ssz <= data.size()) {
                                        std::string val;
                                        for (size_t s = 0; s < ssz; s++) {
                                            char c = data[i + 8 + s];
                                            if (c == '\0') break;
                                            val += c;
                                        }
                                        return val;
                                    }
                                }
                            }
                            return "";
                        };
                        if (out_title.empty()) out_title = parseRiffInfoTag("INAM");
                        if (out_artist.empty()) out_artist = parseRiffInfoTag("IART");
                        if (out_album.empty()) out_album = parseRiffInfoTag("IPRD");
                        if (out_genre.empty()) out_genre = parseRiffInfoTag("IGNR");
                    }
                };

                size_t head_sz = std::min((size_t)65536, file_size);
                std::vector<char> head_buf(head_sz);
                f.read(head_buf.data(), head_sz);
                searchInChunk(head_buf);

                if (out_title.empty() || out_artist.empty()) {
                    if (file_size > head_sz) {
                        size_t tail_sz = std::min((size_t)2097152, file_size);
                        f.seekg(file_size - tail_sz);
                        std::vector<char> tail_buf(tail_sz);
                        f.read(tail_buf.data(), tail_sz);
                        searchInChunk(tail_buf);
                    }
                }
                return (!out_title.empty() || !out_artist.empty());
            } catch (...) {
                return false;
            }
        }

        static std::unordered_map<std::string, DJLibraryTrack> loadSearchTsvCache(const std::filesystem::path& tsv_path) {
            std::unordered_map<std::string, DJLibraryTrack> cache;
            try {
                std::error_code ec;
                if (!std::filesystem::exists(tsv_path, ec)) return cache;
                std::ifstream f(tsv_path);
                if (!f.is_open()) return cache;
                std::string line;
                while (std::getline(f, line)) {
                    if (line.empty()) continue;
                    std::vector<std::string> cols;
                    size_t start = 0;
                    while (start < line.size()) {
                        size_t tab = line.find('\t', start);
                        if (tab == std::string::npos) {
                            cols.push_back(line.substr(start));
                            break;
                        }
                        cols.push_back(line.substr(start, tab - start));
                        start = tab + 1;
                    }
                    if (cols.size() >= 3) {
                        DJLibraryTrack t;
                        t.id = cols[0];
                        t.title = cols[1];
                        t.artist = cols[2];
                        if (cols.size() > 3) t.album = cols[3];
                        if (cols.size() > 6) {
                            std::string d_s = cols[6];
                            for (char& c : d_s) if (c == ',') c = '.';
                            try { t.duration_sec = std::stod(d_s); } catch (...) {}
                        }
                        if (cols.size() > 7) {
                            std::string b_s = cols[7];
                            for (char& c : b_s) if (c == ',') c = '.';
                            try { t.bpm = std::stod(b_s); } catch (...) {}
                        }
                        if (cols.size() > 8) t.key = cols[8];
                        if (cols.size() > 9) t.subgenre = cols[9];

                        cache[t.id] = t;
                        if (t.id.rfind("online_", 0) == 0) {
                            cache[t.id.substr(7)] = t;
                        }
                    }
                }
            } catch (...) {}
            return cache;
        }

        static void enrichTrackMetadata(DJLibraryTrack& t, const std::unordered_map<std::string, DJLibraryTrack>& tsv_cache = {}) {
            std::error_code ec;
            std::string wav_p = t.local_wav_path;
            if (wav_p.empty() || !std::filesystem::exists(std::filesystem::u8path(wav_p), ec)) {
                std::filesystem::path cand1 = std::filesystem::path("scratch/tracks") / ("track_" + t.id + ".wav");
                std::filesystem::path cand2 = std::filesystem::path("scratch/tracks") / (t.id + ".wav");
                if (std::filesystem::exists(cand1, ec)) wav_p = safePathString(cand1);
                else if (std::filesystem::exists(cand2, ec)) wav_p = safePathString(cand2);
            }
            if (!wav_p.empty()) {
                t.local_wav_path = wav_p;
                t.is_downloaded = true;
            }

            // 1. Tenta recuperar do cache TSV (search_results.tsv)
            if (!tsv_cache.empty()) {
                auto it = tsv_cache.find(t.id);
                if (it == tsv_cache.end()) {
                    std::string stripped = t.id;
                    if (stripped.rfind("track_", 0) == 0) stripped = stripped.substr(6);
                    it = tsv_cache.find(stripped);
                }
                if (it != tsv_cache.end()) {
                    if (isDummyTitle(t.title) || t.title.empty()) t.title = it->second.title;
                    if (isDummyArtist(t.artist) || t.artist.empty()) t.artist = it->second.artist;
                    if (t.album.empty()) t.album = it->second.album;
                    if (t.bpm <= 20.0 || t.bpm == 140.0) t.bpm = it->second.bpm;
                    if (t.key.empty()) t.key = it->second.key;
                    if (t.subgenre.empty() || t.subgenre == "all") t.subgenre = it->second.subgenre;
                    if (t.duration_sec <= 1.0 || t.duration_sec == 240.0) t.duration_sec = it->second.duration_sec;
                }
            }

            // 2. Tenta extrair do arquivo .meta JSON adjacente
            if (!wav_p.empty()) {
                try {
                    std::filesystem::path meta_p = std::filesystem::u8path(wav_p);
                    meta_p.replace_extension(".meta");
                    std::string m_t, m_a, m_al, m_g, m_k;
                    double m_bpm = 0.0, m_dur = 0.0;
                    if (readMetaJson(meta_p, m_t, m_a, m_al, m_g, m_bpm, m_k, m_dur)) {
                        if (!m_t.empty() && (isDummyTitle(t.title) || t.title.empty())) t.title = m_t;
                        if (!m_a.empty() && (isDummyArtist(t.artist) || t.artist.empty())) t.artist = m_a;
                        if (!m_al.empty() && t.album.empty()) t.album = m_al;
                        if (!m_g.empty() && (t.subgenre.empty() || t.subgenre == "all")) t.subgenre = m_g;
                        if (m_bpm > 20.0 && (t.bpm <= 20.0 || t.bpm == 140.0)) t.bpm = m_bpm;
                        if (!m_k.empty() && t.key.empty()) t.key = m_k;
                        if (m_dur > 1.0 && (t.duration_sec <= 1.0 || t.duration_sec == 240.0)) t.duration_sec = m_dur;
                    }
                } catch (...) {}
            }

            // 3. Tenta extrair das tags ID3 / RIFF embutidas no WAV
            if (!wav_p.empty()) {
                std::string w_t, w_a, w_al, w_g, w_k;
                double w_bpm = 0.0;
                if (extractWavEmbeddedTags(std::filesystem::u8path(wav_p), w_t, w_a, w_al, w_g, w_bpm, w_k)) {
                    if (!w_t.empty() && (isDummyTitle(t.title) || t.title.empty())) t.title = w_t;
                    if (!w_a.empty() && (isDummyArtist(t.artist) || t.artist.empty())) t.artist = w_a;
                    if (!w_al.empty() && t.album.empty()) t.album = w_al;
                    if (!w_g.empty() && (t.subgenre.empty() || t.subgenre == "all")) t.subgenre = w_g;
                    if (w_bpm > 20.0 && (t.bpm <= 20.0 || t.bpm == 140.0)) t.bpm = w_bpm;
                    if (!w_k.empty() && t.key.empty()) t.key = w_k;
                }
            }

            // 4. Medição precisa de duração
            if (!wav_p.empty() && (t.duration_sec <= 1.0 || t.duration_sec == 240.0)) {
                double real_dur = getWavFileDurationSec(wav_p);
                if (real_dur > 1.0) t.duration_sec = real_dur;
            }

            // 5. Capa RGBA
            if (t.local_rgba_path.empty() || !std::filesystem::exists(std::filesystem::u8path(t.local_rgba_path), ec)) {
                t.local_rgba_path = resolveRgbaCoverArt(wav_p, t.id);
            }

            // 6. Subgênero e pasta
            if (t.subgenre.empty() || t.subgenre == "all" || t.subgenre == "Geral") {
                t.subgenre = classifySubgenre(t.artist, t.title, t.bpm);
            }
            if (t.folder_id.empty() || t.folder_id == "all") {
                t.folder_id = getFolderIdForSubgenre(t.subgenre);
            }
        }

        // Lê a duração real diretamente do cabeçalho do arquivo WAV ou MP3 sem decodificar amostras pesadas
        static double getWavFileDurationSec(const std::string& filepath) {
            if (filepath.empty()) return 0.0;
            drwav wav;
            bool ok = false;
#ifdef _WIN32
            try {
                std::filesystem::path fp = std::filesystem::u8path(filepath);
                if (std::filesystem::exists(fp)) {
                    if (drwav_init_file_w(&wav, fp.wstring().c_str(), NULL)) {
                        ok = true;
                    }
                }
            } catch (...) {}
#endif
            if (!ok) {
                std::error_code ec;
                if (std::filesystem::exists(filepath, ec)) {
                    if (drwav_init_file(&wav, filepath.c_str(), NULL)) {
                        ok = true;
                    }
                }
            }
            if (ok) {
                double dur = 0.0;
                if (wav.sampleRate > 0) {
                    dur = (double)wav.totalPCMFrameCount / (double)wav.sampleRate;
                }
                drwav_uninit(&wav);
                return dur;
            }

            // Fallback para MP3 usando drmp3
            try {
                std::filesystem::path fp = std::filesystem::u8path(filepath);
                std::error_code ec;
                if (std::filesystem::exists(fp, ec)) {
                    std::ifstream f(fp, std::ios::binary | std::ios::ate);
                    if (f.is_open()) {
                        std::streamsize sz = f.tellg();
                        if (sz > 4096) {
                            f.seekg(0, std::ios::beg);
                            std::vector<char> fbuf((size_t)sz);
                            if (f.read(fbuf.data(), sz)) {
                                drmp3 mp3;
                                if (drmp3_init_memory(&mp3, fbuf.data(), fbuf.size(), NULL)) {
                                    drmp3_uint64 total_pcm = drmp3_get_pcm_frame_count(&mp3);
                                    double dur = 0.0;
                                    if (mp3.sampleRate > 0 && total_pcm > 0) {
                                        dur = (double)total_pcm / (double)mp3.sampleRate;
                                    }
                                    drmp3_uninit(&mp3);
                                    if (dur > 0.0) return dur;
                                }
                            }
                        }
                    }
                }
            } catch (...) {}

            return 0.0;
        }

        void refreshTrackDurationsFromDisk() {
            bool changed = false;
            for (auto& t : tracks) {
                std::string wav_to_check = t.local_wav_path;
                std::error_code ec;
                if (wav_to_check.empty() || !std::filesystem::exists(std::filesystem::u8path(wav_to_check), ec)) {
                    std::filesystem::path cand1 = std::filesystem::path("scratch/tracks") / ("track_" + t.id + ".wav");
                    std::filesystem::path cand2 = std::filesystem::path("scratch/tracks") / (t.id + ".wav");
                    std::filesystem::path cand3 = std::filesystem::path("scratch") / ("test_" + t.id + ".wav");
                    std::filesystem::path cand4 = std::filesystem::path("scratch") / (t.id + ".wav");
                    if (std::filesystem::exists(cand1, ec)) wav_to_check = safePathString(cand1);
                    else if (std::filesystem::exists(cand2, ec)) wav_to_check = safePathString(cand2);
                    else if (std::filesystem::exists(cand3, ec)) wav_to_check = safePathString(cand3);
                    else if (std::filesystem::exists(cand4, ec)) wav_to_check = safePathString(cand4);
                }

                if (!wav_to_check.empty()) {
                    double dur = getWavFileDurationSec(wav_to_check);
                    if (dur > 1.0 && (std::abs(t.duration_sec - dur) > 0.5 || t.duration_sec <= 0.0 || t.duration_sec == 240.0)) {
                        t.duration_sec = dur;
                        t.local_wav_path = wav_to_check;
                        t.is_downloaded = true;
                        changed = true;
                    }
                }
            }
            if (changed) {
                saveToFile();
            }
        }

        void markTrackPlayed(const std::string& title_or_id) {
            if (title_or_id.empty() || title_or_id == "Nenhuma Faixa Carregada") return;
            std::lock_guard<std::mutex> lock(lib_mutex);
            bool changed = false;
            for (auto& t : tracks) {
                if (t.id == title_or_id || t.title == title_or_id || (!t.local_wav_path.empty() && t.local_wav_path.find(title_or_id) != std::string::npos)) {
                    if (!t.has_played) {
                        t.has_played = true;
                        changed = true;
                    }
                    t.play_count++;
                    break;
                }
            }
            if (changed) saveToFile();
        }

        void markTrackPlayedByPath(const std::string& wav_path) {
            if (wav_path.empty()) return;
            std::lock_guard<std::mutex> lock(lib_mutex);
            bool changed = false;
            for (auto& t : tracks) {
                if (t.local_wav_path == wav_path) {
                    if (!t.has_played) {
                        t.has_played = true;
                        changed = true;
                    }
                    t.play_count++;
                    break;
                }
            }
            if (changed) saveToFile();
        }

        void loadFromFile() {
            try {
                std::filesystem::path p = KuroAudio::getLibraryDbPath();
                db_path = safePathString(p);
                std::error_code ec_p;
                if (!std::filesystem::exists(p, ec_p)) {
                    std::filesystem::path p_fallback("scratch/tracks/dj_library.json");
                    if (!std::filesystem::exists(p_fallback, ec_p)) return;
                    p = p_fallback;
                }
                std::ifstream f(p);
                if (!f.is_open()) return;
                std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                auto tsv_cache = loadSearchTsvCache("scratch/tracks/search_results.tsv");

                // Parser simples e resiliente de JSON para faixas salvas
                size_t t_pos = content.find("\"tracks\":");
                if (t_pos != std::string::npos) {
                    size_t start_arr = content.find('[', t_pos);
                    size_t end_arr = content.rfind(']');
                    if (start_arr != std::string::npos && end_arr != std::string::npos && end_arr > start_arr) {
                        std::string arr_str = content.substr(start_arr + 1, end_arr - start_arr - 1);
                        size_t obj_start = 0;
                        while ((obj_start = arr_str.find('{', obj_start)) != std::string::npos) {
                            size_t obj_end = arr_str.find('}', obj_start);
                            if (obj_end == std::string::npos) break;
                            std::string obj = arr_str.substr(obj_start, obj_end - obj_start + 1);

                            auto extractField = [&](const std::string& key) -> std::string {
                                std::string k = "\"" + key + "\":";
                                size_t kp = obj.find(k);
                                if (kp == std::string::npos) return "";
                                size_t val_s = obj.find_first_not_of(" \t", kp + k.length());
                                if (val_s == std::string::npos) return "";
                                if (obj[val_s] == '"') {
                                    size_t val_e = obj.find('"', val_s + 1);
                                    if (val_e != std::string::npos) return obj.substr(val_s + 1, val_e - val_s - 1);
                                } else {
                                    size_t val_e = obj.find_first_of(",}\r\n", val_s);
                                    if (val_e != std::string::npos) return obj.substr(val_s, val_e - val_s);
                                }
                                return "";
                            };

                            DJLibraryTrack t;
                            t.id = extractField("id");
                            t.title = extractField("title");
                            t.artist = extractField("artist");
                            t.album = extractField("album");
                            t.subgenre = extractField("subgenre");
                            t.folder_id = extractField("folder_id");
                            std::string bpm_str = extractField("bpm");
                            if (!bpm_str.empty()) try { t.bpm = std::stod(bpm_str); } catch (...) {}
                            t.key = extractField("key");
                            std::string dur_str = extractField("duration_sec");
                            if (!dur_str.empty()) try { t.duration_sec = std::stod(dur_str); } catch (...) {}
                            t.local_wav_path = normalizePath(extractField("local_wav"));
                            t.local_rgba_path = normalizePath(extractField("local_rgba"));
                            t.direct_url = extractField("direct_url");
                            std::string down_str = extractField("is_downloaded");
                            t.is_downloaded = (down_str == "true");
                            std::string played_str = extractField("has_played");
                            t.has_played = false; // Sessão limpa: reiniciado a cada abertura do aplicativo
                            std::string count_str = extractField("play_count");
                            if (!count_str.empty()) try { t.play_count = std::stoi(count_str); } catch (...) {}
                            std::string rating_str = extractField("rating");
                            if (!rating_str.empty()) try { t.rating = std::stoi(rating_str); } catch (...) {}
                            std::string grid_str = extractField("grid_locked");
                            if (!grid_str.empty()) t.grid_locked = (grid_str == "true");

                            std::error_code ec;
                            // Auto-recuperação cirúrgica do arquivo de áudio se o caminho salvo estiver corrompido ou modificado
                            bool audio_ok = false;
                            std::string resolved = KuroAudio::resolveExistingAudioPath(t.local_wav_path);
                            if (!resolved.empty() && KuroAudio::safeFileExists(resolved)) {
                                t.local_wav_path = resolved;
                                audio_ok = true;
                            } else {
                                std::string fname;
                                if (!t.local_wav_path.empty()) {
                                    fname = safeFilename(std::filesystem::u8path(t.local_wav_path));
                                }

                                // 1. Tenta em scratch/tracks/export
                                if (!fname.empty()) {
                                    auto exp1 = std::filesystem::path("scratch/tracks/export") / fname;
                                    if (std::filesystem::exists(exp1, ec)) { t.local_wav_path = safePathString(exp1); audio_ok = true; }
                                }
                                if (!audio_ok && !t.id.empty()) {
                                    auto exp2 = std::filesystem::path("scratch/tracks/export") / ("track_" + t.id + ".wav");
                                    if (std::filesystem::exists(exp2, ec)) { t.local_wav_path = safePathString(exp2); audio_ok = true; }
                                }

                                // 2. Tenta em Desktop FIRST KURU
                                if (!audio_ok && !fname.empty()) {
                                    auto kuru1 = std::filesystem::path("C:/Users/o_vic/Desktop/FIRST KURU") / fname;
                                    if (std::filesystem::exists(kuru1, ec)) { t.local_wav_path = safePathString(kuru1); audio_ok = true; }
                                }

                                // 3. Tenta em scratch/tracks/ (subpastas com metadata.json)
                                if (!audio_ok) {
                                    std::filesystem::path base_tr("scratch/tracks");
                                    if (std::filesystem::exists(base_tr, ec)) {
                                        for (const auto& entry : std::filesystem::directory_iterator(base_tr, ec)) {
                                            try {
                                                if (entry.is_directory()) {
                                                    if (!fname.empty()) {
                                                        auto sf = entry.path() / fname;
                                                        if (std::filesystem::exists(sf, ec)) { t.local_wav_path = safePathString(sf); audio_ok = true; break; }
                                                    }
                                                    // Tenta por correspondência de título ou ID
                                                    if (!t.title.empty() && safeFilename(entry.path()).find(t.title) != std::string::npos) {
                                                        for (const auto& afile : std::filesystem::directory_iterator(entry.path(), ec)) {
                                                            try {
                                                                std::string ext = safeExtension(afile.path());
                                                                if (ext == ".wav" || ext == ".mp3" || ext == ".flac") {
                                                                    t.local_wav_path = safePathString(afile.path());
                                                                    audio_ok = true;
                                                                    break;
                                                                }
                                                            } catch (...) {}
                                                        }
                                                        if (audio_ok) break;
                                                    }
                                                }
                                            } catch (...) {}
                                        }
                                    }
                                }

                                // 4. Tenta na raiz de scratch/tracks e scratch
                                if (!audio_ok) {
                                    std::filesystem::path cand1 = std::filesystem::path("scratch/tracks") / ("track_" + t.id + ".wav");
                                    std::filesystem::path cand2 = std::filesystem::path("scratch/tracks") / (t.id + ".wav");
                                    std::filesystem::path cand3 = std::filesystem::path("scratch") / ("test_" + t.id + ".wav");
                                    std::filesystem::path cand4 = std::filesystem::path("scratch") / (t.id + ".wav");
                                    if (std::filesystem::exists(cand1, ec)) { t.local_wav_path = safePathString(cand1); audio_ok = true; }
                                    else if (std::filesystem::exists(cand2, ec)) { t.local_wav_path = safePathString(cand2); audio_ok = true; }
                                    else if (std::filesystem::exists(cand3, ec)) { t.local_wav_path = safePathString(cand3); audio_ok = true; }
                                    else if (std::filesystem::exists(cand4, ec)) { t.local_wav_path = safePathString(cand4); audio_ok = true; }
                                }
                            }
                            if (audio_ok) t.is_downloaded = true;

                            // Auto-recuperação cirúrgica da arte RGBA específica
                            std::string best_cover = resolveRgbaCoverArt(t.local_wav_path, t.id);
                            if (!best_cover.empty()) {
                                t.local_rgba_path = best_cover;
                            } else if (t.local_rgba_path.find("artwork.rgba") != std::string::npos && t.id != "kuru_2010475") {
                                t.local_rgba_path.clear();
                            }

                            // Carrega a forma de onda 3-Band real da faixa
                            loadTrackWaveform(t);

                            // Auto-cura cirúrgica de metadados se faixa contiver dados dummy ou incompletos
                            if (isDummyTitle(t.title) || isDummyArtist(t.artist) || t.bpm <= 20.0 || t.bpm == 140.0 || t.key.empty() || t.duration_sec <= 1.0 || t.duration_sec == 240.0) {
                                enrichTrackMetadata(t, tsv_cache);
                            }

                            std::string a_low = toLower(t.artist);
                            std::string t_low = toLower(t.title);
                            if (a_low.find("acervo") != std::string::npos || t_low.find("acervo") != std::string::npos || a_low.find("coleção first kuru") != std::string::npos) {
                                obj_start = obj_end + 1;
                                continue;
                            }

                            if (t.id.empty() && !t.local_wav_path.empty()) {
                                std::string stem = safeStem(std::filesystem::u8path(t.local_wav_path));
                                t.id = stem;
                                if (t.id.rfind("track_", 0) == 0) t.id = t.id.substr(6);
                            }

                            if (!t.id.empty()) {
                                bool exists = false;
                                for (auto& ex : tracks) {
                                    if (ex.id == t.id) {
                                        ex = t;
                                        exists = true;
                                        break;
                                    }
                                }
                                if (!exists) {
                                    tracks.push_back(t);
                                }
                            }

                            obj_start = obj_end + 1;
                        }
                    }
                }
                refreshTrackDurationsFromDisk();
                saveToFile();
            } catch (...) {}
        }

        DJLibraryTrack* getTrackById(const std::string& id) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            for (auto& t : tracks) {
                if (t.id == id) return &t;
            }
            return nullptr;
        }

        DJLibraryTrack* getTrackByPathOrStem(const std::string& path_or_stem) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            std::filesystem::path fp;
            try { fp = std::filesystem::u8path(path_or_stem); } catch (...) {}
            std::string stem = fp.empty() ? path_or_stem : fp.stem().string();
            std::string norm_p = normalizePath(path_or_stem);

            for (auto& t : tracks) {
                if (!t.local_wav_path.empty() && normalizePath(t.local_wav_path) == norm_p) return &t;
                if (t.id == stem || t.id == path_or_stem) return &t;
                if (t.id.rfind("online_", 0) == 0 && stem.find(t.id.substr(7)) != std::string::npos) return &t;
                if (stem.rfind("track_", 0) == 0 && stem.substr(6) == t.id) return &t;
                if (t.id.rfind("track_", 0) == 0 && t.id.substr(6) == stem) return &t;
            }
            return nullptr;
        }

        void setTrackTexture(const std::string& id, unsigned int tex_id) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            for (auto& t : tracks) {
                if (t.id == id) {
                    t.gl_tex_id = tex_id;
                    break;
                }
            }
        }

        // Varredura cirúrgica de todas as músicas locais já presentes na máquina
        void scanLocalTracks(const std::string& tracks_folder = "scratch/tracks") {
            std::lock_guard<std::mutex> lock(lib_mutex);
            
            // 1. Carrega banco persistido em disco para não perder nenhuma faixa
            loadFromFile();
            auto tsv_cache = loadSearchTsvCache(std::filesystem::path(tracks_folder) / "search_results.tsv");

            std::error_code ec;
            std::filesystem::path dir(tracks_folder);
            if (!std::filesystem::exists(dir, ec)) return;

            // Catálogo conhecido de faixas de alta definição do projeto
            struct LocalKnown {
                std::string file_stem;
                std::string title;
                std::string artist;
                std::string subgenre;
                double bpm;
                std::string key;
                double dur;
            };

            std::vector<LocalKnown> known_db = {
                { "track_1894735727", "Cliffjumper", "Aura Vortex & Klipsun", "Progressive Psytrance", 138.0, "9B / G", 366.0 },
                { "test_cliff", "Cliffjumper", "Aura Vortex & Klipsun", "Progressive Psytrance", 138.0, "9B / G", 366.0 },
                { "track_demo_djw", "Deep Jungle Walk", "Astrix", "Psytrance / Full-On", 138.0, "8A / Am", 372.0 },
                { "test_astrix", "Deep Jungle Walk", "Astrix", "Psytrance / Full-On", 138.0, "8A / Am", 372.0 },
                { "track_1453852917", "Frequency", "Michael Bibi", "Tech House", 129.2, "7A / Dm", 240.0 },
                { "track_1462380037", "Otto's Chant", "Skream & Michael Bibi", "Tech House", 129.2, "8A / Am", 240.0 },
                { "track_1580631171", "Night", "JOX (BR)", "Psytrance / Full-On", 143.6, "8A / Am", 446.0 },
                { "track_1643587863", "Moonlight", "JOX (BR)", "Psytrance / Full-On", 143.6, "8A / Am", 520.0 },
                { "track_1786107495", "Thinking Pattern (Waldkind Remix)", "Airi", "Progressive Psytrance", 136.0, "10B / D", 510.0 },
                { "track_1822953181", "Flamencore", "Elberg", "Psytrance / Full-On", 143.6, "7B / F", 240.0 },
                { "track_1891678034", "Paraíso De Goa", "JOX (BR)", "Psytrance / Full-On", 143.6, "11A / F#m", 240.0 },
                { "track_6804290002", "Tambaqui", "JOX (BR)", "Psytrance / Full-On", 143.6, "7A / Dm", 520.0 },
                { "track_6797577317", "Shake It", "Atohmic, Synyster Live & Aspx", "Psytrance / Full-On", 143.6, "8A / Am", 362.0 },
                { "track_1885635546", "Endora", "Synyster Live", "Psytrance / Full-On", 143.6, "7A / Dm", 308.0 },
                { "track_1671585757", "Dystopia", "Blazy, Sighter & Basscannon", "Progressive Psytrance", 143.6, "9B / G", 460.0 },
                { "track_1191582954", "Tandava (Blazy & Gottinari Remix)", "Shanti People", "Progressive Psytrance", 143.6, "10A / Bm", 312.0 },
                { "track_1785710687", "Corpo e Canção (Festival Mix)", "Vini Vici & Blazy", "Progressive Psytrance", 143.6, "10A / Bm", 358.0 },
                { "track_1823625788", "Horizons (Phaxe Remix)", "Aura Vortex & Blazy", "Progressive Psytrance", 143.6, "8A / Am", 555.0 },
                { "track_1761353535", "Sweet Disposition (Blazy & Sighter Remix)", "The Temper Trap", "Progressive Psytrance", 136.0, "10A / Bm", 428.0 },
                { "track_1121704790", "Nuke", "Aura Vortex & Blazy", "Progressive Psytrance", 136.0, "10B / D", 344.0 },
                { "track_1659092811", "You Give Me a Feeling", "Vintage Culture & James Hype", "Tech House", 126.0, "8A / Am", 185.0 },
                { "track_1725169850", "Weak", "Vintage Culture & Maverick Sabre", "Tech House", 125.0, "9A / Em", 202.0 },
                { "test_free", "Free", "Vintage Culture", "Tech House", 124.0, "11B / A", 210.0 },
                { "track_1887636546", "Freaky 1", "Max Styler & Vintage Culture", "Tech House", 125.0, "8A / Am", 232.0 },
                { "test_ygmaf", "You Give Me a Feeling", "Vintage Culture", "Tech House", 126.0, "8A / Am", 185.0 }
            };

            auto addOrUpdateScannedTrack = [&](DJLibraryTrack& t) {
                std::string a_low = toLower(t.artist);
                std::string t_low = toLower(t.title);
                if (a_low.find("acervo") != std::string::npos || t_low.find("acervo") != std::string::npos || a_low.find("coleção first kuru") != std::string::npos) {
                    return; // Purge & Rejeita Acervo Psytrance / gerado sinteticamente
                }

                if (t.id.empty()) t.id = "trk_" + std::to_string(std::hash<std::string>{}(t.title + t.artist) % 10000000);
                if (t.folder_id.empty()) t.folder_id = getFolderIdForSubgenre(t.subgenre);
                
                std::string k_new = normalizeCleanKey(t.title, t.artist);
                bool found = false;
                for (auto& ex : tracks) {
                    bool is_same_path = (!t.local_wav_path.empty() && ex.local_wav_path == t.local_wav_path);
                    bool is_same_file = false;
                    if (!t.local_wav_path.empty() && !ex.local_wav_path.empty()) {
                        try {
                            if (safeFilename(std::filesystem::u8path(ex.local_wav_path)) == safeFilename(std::filesystem::u8path(t.local_wav_path))) {
                                is_same_file = true;
                            }
                        } catch (...) {}
                    }

                    bool same_id = (ex.id == t.id);
                    if (!same_id && !ex.id.empty() && !t.id.empty()) {
                        auto stripP = [](const std::string& s) {
                            std::string r = s;
                            if (r.rfind("track_", 0) == 0) r = r.substr(6);
                            if (r.rfind("online_", 0) == 0) r = r.substr(7);
                            return r;
                        };
                        std::string p1 = stripP(ex.id);
                        std::string p2 = stripP(t.id);
                        if (!p1.empty() && p1 == p2) same_id = true;
                    }

                    std::string k_ex = normalizeCleanKey(ex.title, ex.artist);
                    if (same_id || is_same_path || is_same_file || (!k_new.empty() && k_ex == k_new) || (!t.title.empty() && toLower(ex.title) == toLower(t.title) && (toLower(ex.artist) == toLower(t.artist) || isDummyArtist(t.artist) || isDummyArtist(ex.artist)))) {
                        found = true;
                        ex.is_downloaded = true;
                        if (t.duration_sec > ex.duration_sec || ex.local_wav_path.empty()) {
                            ex.local_wav_path = t.local_wav_path;
                            ex.duration_sec = t.duration_sec;
                        }
                        if (!t.local_rgba_path.empty()) ex.local_rgba_path = t.local_rgba_path;
                        
                        bool ex_has_dummy_meta = isDummyTitle(ex.title) || isDummyArtist(ex.artist);
                        bool t_has_real_meta = !isDummyTitle(t.title) && !isDummyArtist(t.artist);

                        if (t_has_real_meta || ex_has_dummy_meta) {
                            if (!isDummyTitle(t.title)) ex.title = t.title;
                            if (!isDummyArtist(t.artist)) ex.artist = t.artist;
                            if (!t.album.empty()) ex.album = t.album;
                        } else {
                            if (isDummyArtist(ex.artist) && !t.artist.empty()) ex.artist = t.artist;
                            if (isDummyTitle(ex.title) && !t.title.empty()) ex.title = t.title;
                        }
                        
                        if (t.bpm > 20.0 && (ex.bpm <= 20.0 || ex.bpm == 140.0 || ex_has_dummy_meta)) ex.bpm = t.bpm;
                        if (!t.key.empty() && (ex.key.empty() || ex_has_dummy_meta)) ex.key = t.key;
                        if (!t.subgenre.empty() && (ex.subgenre.empty() || ex.subgenre == "all" || ex.subgenre == "Geral" || ex_has_dummy_meta)) ex.subgenre = t.subgenre;
                        if (t.duration_sec > 1.0 && (ex.duration_sec <= 1.0 || ex.duration_sec == 240.0 || ex_has_dummy_meta)) ex.duration_sec = t.duration_sec;
                        if (t.folder_id == "first_kuru") ex.folder_id = "first_kuru";
                        else if (!t.folder_id.empty() && (ex.folder_id.empty() || ex.folder_id == "all")) ex.folder_id = t.folder_id;
                        break;
                    }
                }
                if (!found) {
                    tracks.push_back(t);
                }
            };

            // 1. Varredura dos subdiretórios de scratch/tracks
            try {
                for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
                    try {
                        if (entry.is_directory()) {
                            std::string folder_name = safeFilename(entry.path());
                            
                            if (folder_name == "export") {
                                // Pasta export com faixas renderizadas e baixadas
                                for (const auto& fe : std::filesystem::directory_iterator(entry.path(), ec)) {
                                    try {
                                        if (fe.is_regular_file()) {
                                            std::string ext = safeExtension(fe.path());
                                            if (ext == ".wav" || ext == ".mp3") {
                                                std::string stem = safeStem(fe.path());
                                                if (stem.rfind("preview_", 0) == 0 && fe.file_size(ec) < 10000000) continue;
                                                
                                                DJLibraryTrack t;
                                                t.id = stem;
                                                t.local_wav_path = safePathString(fe.path());
                                                t.is_downloaded = true;
                                                t.duration_sec = getWavFileDurationSec(t.local_wav_path);

                                                bool matched = false;
                                                for (const auto& k : known_db) {
                                                    if (k.file_stem == stem) {
                                                        t.title = k.title;
                                                        t.artist = k.artist;
                                                        t.subgenre = k.subgenre;
                                                        t.bpm = k.bpm;
                                                        t.key = k.key;
                                                        if (t.duration_sec <= 1.0) t.duration_sec = k.dur;
                                                        matched = true;
                                                        break;
                                                    }
                                                }
                                                if (!matched) {
                                                    size_t dash = stem.find(" - ");
                                                    if (dash != std::string::npos) {
                                                        t.artist = stem.substr(0, dash);
                                                        t.title = stem.substr(dash + 3);
                                                    } else {
                                                        size_t space3 = stem.find("   ");
                                                        if (space3 != std::string::npos) {
                                                            t.artist = stem.substr(0, space3);
                                                            t.title = stem.substr(space3 + 3);
                                                        } else {
                                                            t.artist = "Artista";
                                                            t.title = stem;
                                                        }
                                                    }
                                                    t.bpm = estimateBPMHeuristic(t.title, t.artist);
                                                    t.key = estimateKeyHeuristic(t.title);
                                                    t.subgenre = classifySubgenre(t.artist, t.title, t.bpm);
                                                }
                                                t.folder_id = getFolderIdForSubgenre(t.subgenre);
                                                addOrUpdateScannedTrack(t);
                                            }
                                        }
                                    } catch (...) {}
                                }
                            } else {
                                // Subpasta individual de cada música organizada
                                std::string audio_path, rgba_path;
                                for (const auto& fe : std::filesystem::directory_iterator(entry.path(), ec)) {
                                    try {
                                        if (fe.is_regular_file()) {
                                            std::string ext = safeExtension(fe.path());
                                            if (ext == ".wav" || ext == ".mp3" || ext == ".flac") {
                                                audio_path = safePathString(fe.path());
                                            } else if (ext == ".rgba") {
                                                rgba_path = safePathString(fe.path());
                                            }
                                        }
                                    } catch (...) {}
                                }

                                if (!audio_path.empty()) {
                                    DJLibraryTrack t;
                                    t.local_wav_path = audio_path;
                                    t.local_rgba_path = rgba_path;
                                    t.is_downloaded = true;
                                    t.duration_sec = getWavFileDurationSec(audio_path);

                                    auto meta_p = entry.path() / "metadata.json";
                                    bool meta_read = false;
                                    if (std::filesystem::exists(meta_p, ec)) {
                                        try {
                                            std::ifstream mf(meta_p);
                                            std::string mstr((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
                                            auto extractM = [&](const std::string& key) -> std::string {
                                                std::string k = "\"" + key + "\":";
                                                size_t kp = mstr.find(k);
                                                if (kp == std::string::npos) return "";
                                                size_t val_s = mstr.find_first_not_of(" \t", kp + k.length());
                                                if (val_s == std::string::npos) return "";
                                                if (mstr[val_s] == '"') {
                                                    size_t val_e = mstr.find('"', val_s + 1);
                                                    if (val_e != std::string::npos) return mstr.substr(val_s + 1, val_e - val_s - 1);
                                                } else {
                                                    size_t val_e = mstr.find_first_of(",}\r\n", val_s);
                                                    if (val_e != std::string::npos) return mstr.substr(val_s, val_e - val_s);
                                                }
                                                return "";
                                            };
                                            std::string m_title = extractM("title");
                                            std::string m_artist = extractM("artist");
                                            std::string m_bpm = extractM("bpm");
                                            std::string m_key = extractM("key");
                                            std::string m_sub = extractM("subgenre");

                                            if (!m_title.empty()) { t.title = m_title; meta_read = true; }
                                            if (!m_artist.empty()) t.artist = m_artist;
                                            if (!m_bpm.empty()) try { t.bpm = std::stod(m_bpm); } catch (...) {}
                                            if (!m_key.empty()) t.key = m_key;
                                            if (!m_sub.empty()) t.subgenre = m_sub;
                                        } catch (...) {}
                                    }

                                    if (!meta_read) {
                                        size_t dash = folder_name.find(" - ");
                                        if (dash != std::string::npos) {
                                            t.artist = folder_name.substr(0, dash);
                                            t.title = folder_name.substr(dash + 3);
                                        } else {
                                            t.artist = "DJ Local";
                                            t.title = folder_name;
                                        }
                                        if (t.bpm <= 0.0) t.bpm = estimateBPMHeuristic(t.title, t.artist);
                                        if (t.key.empty()) t.key = estimateKeyHeuristic(t.title);
                                        if (t.subgenre.empty()) t.subgenre = classifySubgenre(t.artist, t.title, t.bpm);
                                    }

                                    if (t.subgenre.empty()) t.subgenre = classifySubgenre(t.artist, t.title, t.bpm);
                                    t.folder_id = getFolderIdForSubgenre(t.subgenre);
                                    t.id = "sub_" + std::to_string(std::hash<std::string>{}(t.title + t.artist) % 10000000);
                                    addOrUpdateScannedTrack(t);
                                }
                            }
                        } else if (entry.is_regular_file()) {
                            std::string ext = safeExtension(entry.path());
                            if (ext == ".wav" || ext == ".mp3") {
                                std::string stem = safeStem(entry.path());
                                if (stem.rfind("preview_", 0) == 0 && entry.file_size(ec) < 10000000) continue;
                                if (stem == "temp_source_preview") continue;
                                
                                DJLibraryTrack t;
                                t.id = stem;
                                if (t.id.rfind("track_", 0) == 0) t.id = t.id.substr(6);
                                t.local_wav_path = safePathString(entry.path());
                                t.is_downloaded = true;
                                t.duration_sec = getWavFileDurationSec(t.local_wav_path);

                                enrichTrackMetadata(t, tsv_cache);

                                for (const auto& k : known_db) {
                                    if (k.file_stem == stem || k.file_stem == t.id) {
                                        t.title = k.title;
                                        t.artist = k.artist;
                                        t.subgenre = k.subgenre;
                                        t.bpm = k.bpm;
                                        t.key = k.key;
                                        if (t.duration_sec <= 1.0) t.duration_sec = k.dur;
                                        break;
                                    }
                                }

                                if (t.title.empty() || isDummyTitle(t.title)) {
                                    size_t dash_l = stem.find(" - ");
                                    if (dash_l != std::string::npos) {
                                        t.artist = stem.substr(0, dash_l);
                                        t.title = stem.substr(dash_l + 3);
                                    } else {
                                        t.title = stem;
                                        if (t.artist.empty()) t.artist = "Artista Local";
                                    }
                                }

                                if (t.bpm <= 20.0) t.bpm = estimateBPMHeuristic(t.title, t.artist);
                                if (t.key.empty()) t.key = estimateKeyHeuristic(t.title);
                                if (t.subgenre.empty() || t.subgenre == "all" || t.subgenre == "Geral") t.subgenre = classifySubgenre(t.artist, t.title, t.bpm);
                                t.folder_id = getFolderIdForSubgenre(t.subgenre);

                                if (t.local_rgba_path.empty() || !std::filesystem::exists(std::filesystem::u8path(t.local_rgba_path), ec)) {
                                    t.local_rgba_path = resolveRgbaCoverArt(t.local_wav_path, t.id);
                                }

                                addOrUpdateScannedTrack(t);
                            }
                        }
                    } catch (...) {}
                }
            } catch (...) {}

            // 2. Varredura da raiz de scratch (test_astrix.wav e test_cliff.wav)
            std::filesystem::path scratch_root("scratch");
            if (std::filesystem::exists(scratch_root, ec)) {
                try {
                    for (const auto& fe : std::filesystem::directory_iterator(scratch_root, ec)) {
                        try {
                            if (fe.is_regular_file()) {
                                std::string stem = safeStem(fe.path());
                                for (const auto& k : known_db) {
                                    if (k.file_stem == stem) {
                                        DJLibraryTrack t;
                                        t.id = stem;
                                        t.local_wav_path = safePathString(fe.path());
                                        t.is_downloaded = true;
                                        t.title = k.title;
                                        t.artist = k.artist;
                                        t.subgenre = k.subgenre;
                                        t.bpm = k.bpm;
                                        t.key = k.key;
                                        t.duration_sec = getWavFileDurationSec(t.local_wav_path);
                                        if (t.duration_sec <= 1.0) t.duration_sec = k.dur;
                                        t.folder_id = getFolderIdForSubgenre(k.subgenre);
                                        addOrUpdateScannedTrack(t);
                                        break;
                                    }
                                }
                            }
                        } catch (...) {}
                    }
                } catch (...) {}
            }

            // 3. Varredura da pasta do computador FIRST KURU (Área de Trabalho)
            std::filesystem::path kuru_dir("C:/Users/o_vic/Desktop/FIRST KURU");
            if (std::filesystem::exists(kuru_dir, ec)) {
                try {
                    for (const auto& fe : std::filesystem::directory_iterator(kuru_dir, ec)) {
                        try {
                            if (fe.is_regular_file()) {
                                std::string ext = safeExtension(fe.path());
                                if (ext == ".wav" || ext == ".mp3") {
                                    std::string stem = safeStem(fe.path());
                                    DJLibraryTrack t;
                                    t.id = "kuru_" + std::to_string(std::hash<std::string>{}(stem) % 10000000);
                                    t.local_wav_path = safePathString(fe.path());
                                    t.is_downloaded = true;
                                    t.album = "FIRST KURU";
                                    t.folder_id = "first_kuru";

                                    std::string clean = stem;
                                    if (clean.size() > 5 && (clean[2] == '-' || clean[3] == '-') && clean.find(" - ") < 6) {
                                        clean = clean.substr(clean.find(" - ") + 3);
                                    }

                                    size_t dash = clean.find(" - ");
                                    if (dash != std::string::npos) {
                                        t.artist = clean.substr(0, dash);
                                        t.title = clean.substr(dash + 3);
                                    } else {
                                        size_t space3 = clean.find("   ");
                                        if (space3 != std::string::npos) {
                                            t.artist = clean.substr(0, space3);
                                            t.title = clean.substr(space3 + 3);
                                        } else {
                                            std::string und = clean;
                                            for (char& c : und) { if (c == '_') c = ' '; }
                                            size_t d2 = und.find(" - ");
                                            if (d2 != std::string::npos) {
                                                t.artist = und.substr(0, d2);
                                                t.title = und.substr(d2 + 3);
                                            } else {
                                                t.artist = "FIRST KURU";
                                                t.title = und;
                                            }
                                        }
                                    }

                                    t.bpm = estimateBPMHeuristic(t.title, t.artist);
                                    t.subgenre = classifySubgenre(t.artist, t.title, t.bpm);
                                    if (t.subgenre == "Forest & Twilight" && t.bpm < 146.0) t.bpm = 148.0;
                                    else if (t.subgenre == "Psytrance / Full-On" && t.bpm < 144.0) t.bpm = 145.0;
                                    else if (t.subgenre == "Progressive Psytrance" && t.bpm < 136.0) t.bpm = 138.0;
                                    t.key = estimateKeyHeuristic(t.title);
                                    t.duration_sec = getWavFileDurationSec(t.local_wav_path);
                                    addOrUpdateScannedTrack(t);
                                }
                            }
                        } catch (...) {}
                    }
                } catch (...) {}
            }

            // 4. Limpeza de faixas fantasmas (previews com caminhos inexistentes)
            tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [&](const DJLibraryTrack& tr) {
                if (tr.id.rfind("preview_", 0) == 0 && (tr.artist == "DJ Local" || tr.local_wav_path.empty() || !std::filesystem::exists(std::filesystem::u8path(tr.local_wav_path), ec))) {
                    return true;
                }
                return false;
            }), tracks.end());

            updateFolderCounts();
            saveToFile();
        }

        void updateFolderCounts() {
            for (auto& f : folders) {
                if (f.id == "all") {
                    f.track_count = (int)tracks.size();
                } else if (f.id == "recent_downloaded") {
                    f.track_count = (int)std::count_if(tracks.begin(), tracks.end(), [](const DJLibraryTrack& t) {
                        return t.is_downloaded || !t.local_wav_path.empty();
                    });
                } else if (f.id == "favorites") {
                    f.track_count = (int)std::count_if(tracks.begin(), tracks.end(), [](const DJLibraryTrack& t) {
                        return t.rating >= 4;
                    });
                } else if (f.id == "history") {
                    f.track_count = (int)std::count_if(tracks.begin(), tracks.end(), [](const DJLibraryTrack& t) {
                        return t.has_played || t.play_count > 0;
                    });
                } else if (f.id == "first_kuru") {
                    f.track_count = (int)std::count_if(tracks.begin(), tracks.end(), [](const DJLibraryTrack& t) {
                        return t.folder_id == "first_kuru" || t.local_wav_path.find("FIRST KURU") != std::string::npos;
                    });
                } else {
                    f.track_count = (int)std::count_if(tracks.begin(), tracks.end(), [&](const DJLibraryTrack& t) {
                        return (t.folder_id == f.id || (!t.subgenre.empty() && getFolderIdForSubgenre(t.subgenre) == f.id));
                    });
                }
            }
        }

        // Retorna faixas filtradas para a pasta ativa (sem misturar entre crates)
        std::vector<DJLibraryTrack> getTracksForFolder(const std::string& folder_id) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            std::vector<DJLibraryTrack> res;
            for (const auto& t : tracks) {
                if (folder_id == "all") {
                    res.push_back(t);
                } else if (folder_id == "recent_downloaded") {
                    if (t.is_downloaded || !t.local_wav_path.empty()) {
                        res.push_back(t);
                    }
                } else if (folder_id == "favorites") {
                    if (t.rating >= 4) {
                        res.push_back(t);
                    }
                } else if (folder_id == "history") {
                    if (t.has_played || t.play_count > 0) {
                        res.push_back(t);
                    }
                } else if (folder_id == "first_kuru") {
                    if (t.folder_id == "first_kuru" || t.local_wav_path.find("FIRST KURU") != std::string::npos) {
                        res.push_back(t);
                    }
                } else if (t.folder_id == folder_id || (!t.subgenre.empty() && getFolderIdForSubgenre(t.subgenre) == folder_id)) {
                    res.push_back(t);
                }
            }
            return res;
        }

        void addOrUpdateTrack(const DJLibraryTrack& track) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            bool found = false;
            auto stripP = [](const std::string& s) {
                std::string r = s;
                if (r.rfind("track_", 0) == 0) r = r.substr(6);
                if (r.rfind("online_", 0) == 0) r = r.substr(7);
                return r;
            };
            std::string t_core = stripP(track.id);

            for (auto& t : tracks) {
                bool same_id = (t.id == track.id);
                if (!same_id && !t_core.empty()) {
                    if (stripP(t.id) == t_core) same_id = true;
                }
                bool same_path = (!track.local_wav_path.empty() && t.local_wav_path == track.local_wav_path);
                bool same_title_artist = (!track.title.empty() && toLower(t.title) == toLower(track.title) && (toLower(t.artist) == toLower(track.artist) || isDummyArtist(t.artist) || isDummyArtist(track.artist)));

                if (same_id || same_path || same_title_artist) {
                    if (!isDummyTitle(track.title) || isDummyTitle(t.title)) t.title = track.title;
                    if (!isDummyArtist(track.artist) || isDummyArtist(t.artist)) t.artist = track.artist;
                    if (!track.album.empty()) t.album = track.album;
                    if (track.bpm > 20.0) t.bpm = track.bpm;
                    if (!track.key.empty()) t.key = track.key;
                    if (!track.subgenre.empty() && track.subgenre != "all" && track.subgenre != "Geral") t.subgenre = track.subgenre;
                    if (track.duration_sec > 1.0) t.duration_sec = track.duration_sec;
                    if (!track.local_wav_path.empty()) t.local_wav_path = track.local_wav_path;
                    if (!track.local_rgba_path.empty()) t.local_rgba_path = track.local_rgba_path;
                    if (track.is_downloaded) t.is_downloaded = true;
                    if (track.gl_tex_id != 0) t.gl_tex_id = track.gl_tex_id;
                    if (!track.folder_id.empty() && track.folder_id != "all") t.folder_id = track.folder_id;
                    found = true;
                    break;
                }
            }
            if (!found) {
                tracks.push_back(track);
            }
            updateFolderCounts();
            saveToFile();
        }

        void setTrackSubgenre(const std::string& track_id, const std::string& new_subgenre) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            for (auto& t : tracks) {
                if (t.id == track_id) {
                    t.subgenre = new_subgenre;
                    t.folder_id = getFolderIdForSubgenre(new_subgenre);
                    break;
                }
            }
            updateFolderCounts();
            saveToFile();
        }

        void moveTrackToFolder(const std::string& track_id, const std::string& target_folder_id) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            for (auto& t : tracks) {
                if (t.id == track_id) {
                    t.folder_id = target_folder_id;
                    break;
                }
            }
            updateFolderCounts();
            saveToFile();
        }

        bool createCustomFolder(const std::string& name) {
            if (name.empty()) return false;
            std::lock_guard<std::mutex> lock(lib_mutex);
            std::string id = "custom_" + std::to_string(folders.size() + 1);
            folders.push_back({ id, name, "folder", false, 0 });
            saveToFile();
            return true;
        }

        bool renameFolder(const std::string& folder_id, const std::string& new_name) {
            if (new_name.empty()) return false;
            std::lock_guard<std::mutex> lock(lib_mutex);
            for (auto& f : folders) {
                if (f.id == folder_id) {
                    f.name = new_name;
                    saveToFile();
                    return true;
                }
            }
            return false;
        }

        static std::string normalizeCleanKey(const std::string& title, const std::string& artist) {
            std::string t = toLower(title);
            std::string a = toLower(artist);
            std::string k;
            for (char c : t) if (std::isalnum((unsigned char)c)) k += c;
            k += "_";
            for (char c : a) if (std::isalnum((unsigned char)c)) k += c;
            return k;
        }

        int countDuplicateTracks() {
            std::lock_guard<std::mutex> lock(lib_mutex);
            std::map<std::string, int> counts;
            for (const auto& t : tracks) {
                std::string k = normalizeCleanKey(t.title, t.artist);
                if (!k.empty()) counts[k]++;
            }
            int dupes = 0;
            for (const auto& [k, c] : counts) {
                if (c > 1) dupes += (c - 1);
            }
            return dupes;
        }

        int deduplicateTracks() {
            std::lock_guard<std::mutex> lock(lib_mutex);
            std::map<std::string, std::vector<DJLibraryTrack>> groups;
            for (const auto& t : tracks) {
                std::string k = normalizeCleanKey(t.title, t.artist);
                groups[k].push_back(t);
            }

            std::vector<DJLibraryTrack> new_tracks;
            int removed_count = 0;

            for (auto& [k, trk_list] : groups) {
                if (trk_list.size() == 1) {
                    new_tracks.push_back(trk_list[0]);
                } else {
                    // Escolhe a melhor faixa (maior duração, áudio válido no disco, artwork)
                    std::sort(trk_list.begin(), trk_list.end(), [](const DJLibraryTrack& a, const DJLibraryTrack& b) {
                        int score_a = 0;
                        int score_b = 0;
                        if (!a.local_wav_path.empty()) score_a += 100;
                        if (!b.local_wav_path.empty()) score_b += 100;
                        if (a.duration_sec >= 120.0) score_a += 200;
                        if (b.duration_sec >= 120.0) score_b += 200;
                        if (a.duration_sec > b.duration_sec) score_a += 50;
                        if (b.duration_sec > a.duration_sec) score_b += 50;
                        if (!a.local_rgba_path.empty()) score_a += 30;
                        if (!b.local_rgba_path.empty()) score_b += 30;
                        if (a.play_count > b.play_count) score_a += 10;
                        if (b.play_count > a.play_count) score_b += 10;
                        return score_a > score_b;
                    });

                    DJLibraryTrack best = trk_list[0];
                    for (size_t i = 1; i < trk_list.size(); i++) {
                        best.play_count += trk_list[i].play_count;
                        if (trk_list[i].has_played) best.has_played = true;
                        removed_count++;
                    }
                    new_tracks.push_back(best);
                }
            }

            tracks = new_tracks;
            updateFolderCounts();
            saveToFile();
            return removed_count;
        }

        bool deleteFolder(const std::string& folder_id) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            auto it = std::remove_if(folders.begin(), folders.end(), [&](const DJCrateFolder& f) {
                return !f.is_system && f.id == folder_id;
            });
            if (it != folders.end()) {
                folders.erase(it, folders.end());
                // Redireciona faixas para "all"
                for (auto& t : tracks) {
                    if (t.folder_id == folder_id) t.folder_id = "all";
                }
                updateFolderCounts();
                saveToFile();
                return true;
            }
            return false;
        }

        bool deleteTrack(const std::string& track_id, bool delete_file_on_disk = false) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            std::string wav_to_del, rgba_to_del;
            auto it = std::remove_if(tracks.begin(), tracks.end(), [&](const DJLibraryTrack& t) {
                if (t.id == track_id) {
                    wav_to_del = t.local_wav_path;
                    rgba_to_del = t.local_rgba_path;
                    return true;
                }
                return false;
            });
            if (it != tracks.end()) {
                tracks.erase(it, tracks.end());
                if (delete_file_on_disk) {
                    std::error_code ec;
                    if (!wav_to_del.empty() && std::filesystem::exists(std::filesystem::u8path(wav_to_del), ec)) {
                        std::filesystem::remove(std::filesystem::u8path(wav_to_del), ec);
                        std::string meta_f = std::filesystem::u8path(wav_to_del).replace_extension(".meta").string();
                        if (std::filesystem::exists(std::filesystem::u8path(meta_f), ec)) {
                            std::filesystem::remove(std::filesystem::u8path(meta_f), ec);
                        }
                    }
                    if (!rgba_to_del.empty() && std::filesystem::exists(std::filesystem::u8path(rgba_to_del), ec)) {
                        std::filesystem::remove(std::filesystem::u8path(rgba_to_del), ec);
                    }
                }
                updateFolderCounts();
                saveToFile();
                return true;
            }
            return false;
        }

        bool updateTrackMetadata(const std::string& track_id, const std::string& new_title, const std::string& new_artist, double new_bpm, const std::string& new_key, const std::string& new_subgenre) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            for (auto& t : tracks) {
                if (t.id == track_id) {
                    t.title = new_title;
                    t.artist = new_artist;
                    t.bpm = new_bpm;
                    t.key = new_key;
                    t.subgenre = new_subgenre;
                    t.folder_id = getFolderIdForSubgenre(new_subgenre);

                    // Atualiza arquivo .meta correspondente em disco
                    if (!t.local_wav_path.empty()) {
                        try {
                            std::error_code ec;
                            std::filesystem::path meta_p = std::filesystem::u8path(t.local_wav_path);
                            meta_p.replace_extension(".meta");
                            std::ofstream mf(meta_p);
                            if (mf.is_open()) {
                                mf << "{\n  \"bpm\": " << t.bpm 
                                   << ",\n  \"key\": \"" << t.key 
                                   << "\",\n  \"title\": \"" << t.title 
                                   << "\",\n  \"artist\": \"" << t.artist 
                                   << "\",\n  \"subgenre\": \"" << t.subgenre << "\"\n}\n";
                            }
                        } catch (...) {}
                    }
                    updateFolderCounts();
                    saveToFile();
                    return true;
                }
            }
            return false;
        }

        bool duplicateTrack(const std::string& track_id) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            DJLibraryTrack copy_track;
            bool found = false;
            for (const auto& t : tracks) {
                if (t.id == track_id) {
                    copy_track = t;
                    found = true;
                    break;
                }
            }
            if (found) {
                copy_track.id = copy_track.id + "_copy_" + std::to_string(std::rand() % 9000 + 1000);
                copy_track.title = copy_track.title + " (Cópia)";
                tracks.push_back(copy_track);
                updateFolderCounts();
                saveToFile();
                return true;
            }
            return false;
        }

        std::vector<DJCrateFolder> getFolders() {
            std::lock_guard<std::mutex> lock(lib_mutex);
            return folders;
        }

        std::vector<DJLibraryTrack> getAllTracks() {
            std::lock_guard<std::mutex> lock(lib_mutex);
            return tracks;
        }

        void setTrackRating(const std::string& track_id, int rating) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            for (auto& t : tracks) {
                if (t.id == track_id) {
                    t.rating = std::clamp(rating, 0, 5);
                    saveToFile();
                    break;
                }
            }
        }

        void setTrackGridLock(const std::string& track_id, bool locked) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            for (auto& t : tracks) {
                if (t.id == track_id) {
                    t.grid_locked = locked;
                    saveToFile();
                    break;
                }
            }
        }

        std::string getActiveFolderId() const { return active_folder_id; }
        void setActiveFolderId(const std::string& fid) { active_folder_id = fid; }

        void saveToFile() {
            try {
                std::filesystem::path p = KuroAudio::getLibraryDbPath();
                db_path = safePathString(p);
                std::filesystem::create_directories(p.parent_path());
                std::ofstream f(p);
                if (!f.is_open()) return;
                f.imbue(std::locale::classic());

                f << "{\n  \"folders\": [\n";
                for (size_t i = 0; i < folders.size(); i++) {
                    const auto& fold = folders[i];
                    f << "    { \"id\": \"" << fold.id << "\", \"name\": \"" << fold.name 
                      << "\", \"icon\": \"" << fold.icon << "\", \"is_system\": " << (fold.is_system ? "true" : "false") << " }"
                      << (i + 1 < folders.size() ? ",\n" : "\n");
                }
                f << "  ],\n  \"tracks\": [\n";
                for (size_t i = 0; i < tracks.size(); i++) {
                    const auto& t = tracks[i];
                    f << "    { \"id\": \"" << t.id 
                      << "\", \"title\": \"" << escapeJson(t.title) 
                      << "\", \"artist\": \"" << escapeJson(t.artist) 
                      << "\", \"album\": \"" << escapeJson(t.album) 
                      << "\", \"subgenre\": \"" << escapeJson(t.subgenre) 
                      << "\", \"folder_id\": \"" << t.folder_id 
                      << "\", \"bpm\": " << t.bpm 
                      << ", \"key\": \"" << t.key 
                      << "\", \"duration_sec\": " << t.duration_sec 
                      << ", \"local_wav\": \"" << normalizePath(t.local_wav_path) 
                      << "\", \"local_rgba\": \"" << normalizePath(t.local_rgba_path) 
                      << "\", \"is_downloaded\": " << (t.is_downloaded ? "true" : "false") 
                      << ", \"has_played\": " << (t.has_played ? "true" : "false")
                      << ", \"play_count\": " << t.play_count 
                      << ", \"rating\": " << t.rating 
                      << ", \"grid_locked\": " << (t.grid_locked ? "true" : "false") << " }"
                      << (i + 1 < tracks.size() ? ",\n" : "\n");
                }
                f << "  ]\n}\n";
            } catch (...) {}
        }

    private:
        static std::string escapeJson(const std::string& s) {
            std::string out;
            for (char c : s) {
                if (c == '"') out += "\\\"";
                else if (c == '\\') out += "\\\\";
                else out += c;
            }
            return out;
        }
    };

} // namespace KuroAudio
