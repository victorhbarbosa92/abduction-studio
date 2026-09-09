#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cctype>

namespace KuroAudio {

    struct DJCrateFolder {
        std::string id;
        std::string name;
        std::string icon = "folder";
        bool is_system = false;
        int track_count = 0;
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
        bool is_downloaded = false;
        bool is_downloading = false;
        unsigned int gl_tex_id = 0;
    };

    class DJLibraryManager {
    private:
        std::vector<DJCrateFolder> folders;
        std::vector<DJLibraryTrack> tracks;
        std::string active_folder_id = "prog_psy";
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
            folders.push_back({ "all", "Todas as Músicas", "music", true, 0 });
            folders.push_back({ "computer_files", "Do Computador", "laptop", true, 0 });
            folders.push_back({ "prog_psy", "Progressive Psytrance", "fire", true, 0 });
            folders.push_back({ "psy_fullon", "Psytrance / Full-On", "bolt", true, 0 });
            folders.push_back({ "tech_house", "Tech House / Bass", "compact-disc", true, 0 });
            folders.push_back({ "acid_techno", "Acid Techno / Peak", "sliders", true, 0 });
            folders.push_back({ "melodic_techno", "Melodic Techno", "headphones", true, 0 });
            folders.push_back({ "cloud", "Busca Online / Nuvem", "cloud", true, 0 });
        }

        void clearAllDownloadedTracks() {
            std::lock_guard<std::mutex> lock(lib_mutex);
            std::vector<DJLibraryTrack> kept;
            for (const auto& t : tracks) {
                if (t.folder_id == "computer_files") {
                    kept.push_back(t);
                }
            }
            tracks = kept;
            updateFolderCounts();
            saveToFile();
        }

        // Classificador Cirúrgico de Vertentes da Música Eletrônica
        static std::string classifySubgenre(const std::string& artist, const std::string& title, double bpm = 128.0, const std::string& query = "") {
            std::string a = toLower(artist);
            std::string t = toLower(title);
            std::string q = toLower(query);
            std::string combined = a + " " + t + " " + q;

            // 1. Progressive Psytrance (Aura Vortex, Klipsun, Neelix, Phaxe, etc.)
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
                combined.find("progpsy") != std::string::npos) {
                return "Progressive Psytrance";
            }

            // 2. Psytrance / Full-On (Astrix, Vini Vici, Blastoyz, Infected Mushroom, etc.)
            if (combined.find("astrix") != std::string::npos ||
                combined.find("deep jungle walk") != std::string::npos ||
                combined.find("vini vici") != std::string::npos ||
                combined.find("blastoyz") != std::string::npos ||
                combined.find("infected mushroom") != std::string::npos ||
                combined.find("ace ventura") != std::string::npos ||
                combined.find("liquid soul") != std::string::npos ||
                combined.find("tristan") != std::string::npos ||
                combined.find("avalon") != std::string::npos ||
                combined.find("burn in noise") != std::string::npos ||
                combined.find("skazi") != std::string::npos ||
                combined.find("psytrance") != std::string::npos ||
                combined.find("psy-trance") != std::string::npos ||
                combined.find("full on") != std::string::npos ||
                combined.find("full-on") != std::string::npos ||
                combined.find("goa") != std::string::npos) {
                return "Psytrance / Full-On";
            }

            // 3. Acid Techno / Peak Time (Charlotte de Witte, Amelie Lens, Reinier Zonneveld, etc.)
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

            // 4. Tech House / Bass House (Vintage Culture, Fisher, Mau P, James Hype, etc.)
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

            // 5. Melodic Techno / Progressive House (ARTBAT, Anyma, Tale of Us, etc.)
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

            // 6. Heurística Inteligente por Faixa de BPM
            if (bpm >= 137.0) return "Psytrance / Full-On";
            if (bpm >= 133.0) return "Progressive Psytrance";
            if (bpm >= 129.0) return "Acid Techno";
            if (bpm >= 123.0) return "Tech House";
            return "Melodic Techno";
        }

        static std::string getFolderIdForSubgenre(const std::string& subgenre) {
            if (subgenre == "Progressive Psytrance") return "prog_psy";
            if (subgenre == "Psytrance / Full-On")   return "psy_fullon";
            if (subgenre == "Tech House")            return "tech_house";
            if (subgenre == "Acid Techno")           return "acid_techno";
            if (subgenre == "Melodic Techno")        return "melodic_techno";
            return "all";
        }

        // Varredura cirúrgica de todas as músicas locais já presentes na máquina
        void scanLocalTracks(const std::string& tracks_folder = "scratch/tracks") {
            std::lock_guard<std::mutex> lock(lib_mutex);
            std::error_code ec;
            std::filesystem::path dir(tracks_folder);
            if (!std::filesystem::exists(dir, ec)) return;

            // Catálogo conhecido de faixas de alta definição baixadas no projeto
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
                { "track_1659092811", "You Give Me a Feeling", "Vintage Culture & James Hype", "Tech House", 126.0, "8A / Am", 185.0 },
                { "track_1725169850", "Weak", "Vintage Culture & Maverick Sabre", "Tech House", 125.0, "9A / Em", 202.0 },
                { "test_free", "Free", "Vintage Culture", "Tech House", 124.0, "11B / A", 210.0 },
                { "track_1887636546", "Freaky 1", "Max Styler & Vintage Culture", "Tech House", 125.0, "8A / Am", 232.0 },
                { "test_ygmaf", "You Give Me a Feeling", "Vintage Culture", "Tech House", 126.0, "8A / Am", 185.0 }
            };

            for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
                if (entry.is_regular_file() && entry.path().extension() == ".wav") {
                    std::string stem = entry.path().stem().string();
                    std::string full_path = entry.path().string();

                    // Verifica se já está adicionada
                    bool exists = false;
                    for (auto& t : tracks) {
                        if (t.local_wav_path == full_path || t.id == stem) {
                            exists = true;
                            t.is_downloaded = true;
                            t.local_wav_path = full_path;
                            break;
                        }
                    }
                    if (exists) continue;

                    // Busca metadados conhecidos
                    DJLibraryTrack t;
                    t.id = stem;
                    t.local_wav_path = full_path;
                    t.is_downloaded = true;
                    
                    std::string rgba = (dir / (stem + ".rgba")).string();
                    if (std::filesystem::exists(rgba, ec)) {
                        t.local_rgba_path = rgba;
                    }

                    bool matched = false;
                    for (const auto& k : known_db) {
                        if (k.file_stem == stem) {
                            t.title = k.title;
                            t.artist = k.artist;
                            t.subgenre = k.subgenre;
                            t.bpm = k.bpm;
                            t.key = k.key;
                            t.duration_sec = k.dur;
                            t.folder_id = getFolderIdForSubgenre(k.subgenre);
                            matched = true;
                            break;
                        }
                    }

                    if (!matched) {
                        t.title = stem;
                        t.artist = "DJ Local";
                        t.bpm = 128.0;
                        t.key = "8A / Am";
                        t.duration_sec = 240.0;
                        t.subgenre = classifySubgenre(t.artist, t.title, t.bpm);
                        t.folder_id = getFolderIdForSubgenre(t.subgenre);
                    }

                    bool already_has = false;
                    for (auto& ex : tracks) {
                        if (ex.title == t.title && ex.artist == t.artist) {
                            already_has = true;
                            ex.is_downloaded = true;
                            if (ex.local_wav_path.empty()) ex.local_wav_path = t.local_wav_path;
                            if (ex.local_rgba_path.empty()) ex.local_rgba_path = t.local_rgba_path;
                            break;
                        }
                    }

                    if (!already_has) {
                        tracks.push_back(t);
                    }
                }
            }

            // Também varre a raiz de scratch para test_cliff.wav e test_astrix.wav
            std::filesystem::path scratch_root("scratch");
            if (std::filesystem::exists(scratch_root, ec)) {
                for (const auto& entry : std::filesystem::directory_iterator(scratch_root, ec)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".wav") {
                        std::string stem = entry.path().stem().string();
                        std::string full_path = entry.path().string();
                        bool exists = false;
                        for (auto& t : tracks) {
                            if (t.title == "Cliffjumper" && stem.find("cliff") != std::string::npos) { exists = true; break; }
                            if (t.title == "Deep Jungle Walk" && stem.find("astrix") != std::string::npos) { exists = true; break; }
                        }
                        if (exists) continue;

                        for (const auto& k : known_db) {
                            if (k.file_stem == stem) {
                                DJLibraryTrack t;
                                t.id = stem;
                                t.local_wav_path = full_path;
                                t.is_downloaded = true;
                                t.title = k.title;
                                t.artist = k.artist;
                                t.subgenre = k.subgenre;
                                t.bpm = k.bpm;
                                t.key = k.key;
                                t.duration_sec = k.dur;
                                t.folder_id = getFolderIdForSubgenre(k.subgenre);
                                tracks.push_back(t);
                                break;
                            }
                        }
                    }
                }
            }

            updateFolderCounts();
            saveToFile();
        }

        void updateFolderCounts() {
            for (auto& f : folders) {
                f.track_count = 0;
            }

            for (const auto& t : tracks) {
                // Pasta "all"
                if (t.is_downloaded) {
                    for (auto& f : folders) {
                        if (f.id == "all") f.track_count++;
                    }
                }
                // Pasta correspondente
                for (auto& f : folders) {
                    if (f.id == t.folder_id) {
                        f.track_count++;
                    }
                }
            }
        }

        // Retorna faixas filtradas para a pasta ativa
        std::vector<DJLibraryTrack> getTracksForFolder(const std::string& folder_id) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            std::vector<DJLibraryTrack> res;
            for (const auto& t : tracks) {
                if (folder_id == "all") {
                    if (t.is_downloaded) res.push_back(t);
                } else if (folder_id == "cloud") {
                    if (!t.is_downloaded) res.push_back(t);
                } else if (t.folder_id == folder_id) {
                    res.push_back(t);
                }
            }
            return res;
        }

        void addOrUpdateTrack(const DJLibraryTrack& track) {
            std::lock_guard<std::mutex> lock(lib_mutex);
            bool found = false;
            for (auto& t : tracks) {
                if (t.id == track.id || (t.title == track.title && t.artist == track.artist)) {
                    t = track;
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

        std::vector<DJCrateFolder> getFolders() {
            std::lock_guard<std::mutex> lock(lib_mutex);
            return folders;
        }

        std::string getActiveFolderId() const { return active_folder_id; }
        void setActiveFolderId(const std::string& fid) { active_folder_id = fid; }

        void saveToFile() {
            try {
                std::filesystem::path p(db_path);
                std::filesystem::create_directories(p.parent_path());
                std::ofstream f(p);
                if (!f.is_open()) return;

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
                      << ", \"local_wav\": \"" << escapeJson(t.local_wav_path) 
                      << "\", \"is_downloaded\": " << (t.is_downloaded ? "true" : "false") << " }"
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
