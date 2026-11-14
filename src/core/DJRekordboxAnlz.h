#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <sstream>
#include "../utils/Logger.h"

namespace KuroAudio {

    struct RekordboxBeat {
        int bar_beat = 1;         // 1 = Downbeat vermelho (cabeça do compasso), 2, 3, 4 = batidas brancas
        double tempo_bpm = 120.0; // BPM instantâneo desta batida
        double time_ms = 0.0;     // Timestamp absoluto em milissegundos
        double time_sec = 0.0;    // time_ms / 1000.0
        int64_t frame = 0;        // Frame absoluto conforme sample_rate
    };

    struct RekordboxAnalysis {
        bool is_valid = false;
        std::string raw_path;
        std::string title;
        double main_bpm = 120.0;
        double first_beat_ms = 0.0;
        double first_beat_sec = 0.0;
        int64_t first_beat_frame = 0;
        int total_beats = 0;
        std::vector<RekordboxBeat> beats;
        std::string source_anlz_path;
    };

    class DJRekordboxEngine {
    private:
        std::unordered_map<std::string, std::string> anlz_cache; // chave normalizada -> caminho do ANLZ0000.DAT
        bool index_built = false;
        std::mutex index_mutex;

        inline uint16_t readU16BE(const uint8_t* p) const {
            return (uint16_t)((p[0] << 8) | p[1]);
        }

        inline uint32_t readU32BE(const uint8_t* p) const {
            return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
        }

        inline std::string utf16BEtoUTF8(const uint8_t* p, size_t len_bytes) const {
            std::string out;
            for (size_t i = 0; i + 1 < len_bytes; i += 2) {
                uint16_t cp = (uint16_t)((p[i] << 8) | p[i + 1]);
                if (cp == 0) break; // null terminator
                if (cp < 0x80) {
                    out.push_back((char)cp);
                } else if (cp < 0x800) {
                    out.push_back((char)(0xC0 | (cp >> 6)));
                    out.push_back((char)(0x80 | (cp & 0x3F)));
                } else {
                    out.push_back((char)(0xE0 | (cp >> 12)));
                    out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
                    out.push_back((char)(0x80 | (cp & 0x3F)));
                }
            }
            return out;
        }

    public:
        DJRekordboxEngine() = default;

        static DJRekordboxEngine& getInstance() {
            static DJRekordboxEngine instance;
            return instance;
        }

        // Normalização agressiva de título para correspondência imune a pontuações, extensões e espaços
        static std::string normalizeTitle(const std::string& s) {
            std::filesystem::path p(s);
            std::string base = p.stem().string();
            if (base.rfind("?/", 0) == 0 || base.rfind("?\\", 0) == 0) {
                base = base.substr(2);
            }

            std::string norm;
            for (char c : base) {
                unsigned char uc = (unsigned char)c;
                if (std::isalnum(uc)) {
                    norm.push_back((char)std::tolower(uc));
                } else if (c == ' ' || c == '_' || c == '-' || c == '.') {
                    if (!norm.empty() && norm.back() != ' ') {
                        norm.push_back(' ');
                    }
                }
            }
            while (!norm.empty() && norm.back() == ' ') norm.pop_back();
            return norm;
        }

        // Constrói o índice de arquivos ANLZ do Rekordbox em cache (tempo típico ~50-60ms)
        void buildIndex(bool force_rebuild = false) {
            std::lock_guard<std::mutex> lock(index_mutex);
            if (index_built && !force_rebuild) return;

            anlz_cache.clear();

            std::vector<std::string> search_dirs;

            // 1. Diretório padrão do Rekordbox no Windows (%APPDATA%\Pioneer\rekordbox\share\PIONEER\USBANLZ)
            const char* appdata = std::getenv("APPDATA");
            if (appdata) {
                std::filesystem::path rb_dir = std::filesystem::path(appdata) / "Pioneer" / "rekordbox" / "share" / "PIONEER" / "USBANLZ";
                if (std::filesystem::exists(rb_dir)) {
                    search_dirs.push_back(rb_dir.string());
                }
            }

            // Fallback explícito caso APPDATA aponte para outro caminho
            std::string explicit_path = "C:\\Users\\o_vic\\AppData\\Roaming\\Pioneer\\rekordbox\\share\\PIONEER\\USBANLZ";
            if (std::filesystem::exists(explicit_path)) {
                bool already = false;
                for (const auto& d : search_dirs) if (d == explicit_path) already = true;
                if (!already) search_dirs.push_back(explicit_path);
            }

            // 2. Procura em unidades montadas (pendrives DJ)
            const char drive_letters[] = { 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'U' };
            for (char drv : drive_letters) {
                std::string usb_anlz = std::string(1, drv) + ":\\PIONEER\\USBANLZ";
                if (std::filesystem::exists(usb_anlz)) {
                    search_dirs.push_back(usb_anlz);
                }
            }

            for (const auto& dir : search_dirs) {
                try {
                    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir, std::filesystem::directory_options::skip_permission_denied)) {
                        if (entry.is_regular_file()) {
                            std::string filename = entry.path().filename().string();
                            if (filename == "ANLZ0000.DAT") {
                                std::string dat_path = entry.path().string();
                                
                                // Lê cabeçalho rápido para extrair a tag PPTH
                                std::ifstream f(dat_path, std::ios::binary);
                                if (f.is_open()) {
                                    std::vector<uint8_t> buf(4096, 0);
                                    f.read(reinterpret_cast<char*>(buf.data()), buf.size());
                                    size_t bytes_read = (size_t)f.gcount();
                                    
                                    // Procura FourCC 'PPTH'
                                    for (size_t i = 0; i + 12 <= bytes_read; i++) {
                                        if (buf[i] == 'P' && buf[i+1] == 'P' && buf[i+2] == 'T' && buf[i+3] == 'H') {
                                            uint32_t hlen = readU32BE(&buf[i + 4]);
                                            uint32_t tlen = readU32BE(&buf[i + 8]);
                                            if (tlen > hlen) {
                                                std::string title;
                                                if (i + tlen <= bytes_read) {
                                                    title = utf16BEtoUTF8(&buf[i + hlen], tlen - hlen);
                                                } else {
                                                    f.seekg(i + hlen, std::ios::beg);
                                                    std::vector<uint8_t> pbuf(tlen - hlen);
                                                    f.read(reinterpret_cast<char*>(pbuf.data()), pbuf.size());
                                                    title = utf16BEtoUTF8(pbuf.data(), pbuf.size());
                                                }
                                                std::string norm = normalizeTitle(title);
                                                if (!norm.empty()) {
                                                    anlz_cache[norm] = dat_path;
                                                }
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                } catch (...) {}
            }

            index_built = true;
            KuroUtils::Log("[Rekordbox Engine] Indice nativo construído com sucesso: " + std::to_string(anlz_cache.size()) + " faixas indexadas no cache USBANLZ");
        }

        // Localiza o arquivo ANLZ para uma faixa específica (por correspondência exata ou fuzzy)
        std::string findAnlzForTrack(const std::string& track_path) {
            buildIndex();
            std::lock_guard<std::mutex> lock(index_mutex);

            std::string norm_target = normalizeTitle(track_path);
            if (norm_target.empty()) return "";

            // 1. Match exato normalizado
            auto it = anlz_cache.find(norm_target);
            if (it != anlz_cache.end()) {
                return it->second;
            }

            // 2. Match parcial / token overlap (caso haja remix ou parênteses divergentes)
            std::vector<std::string> target_words;
            std::stringstream ss(norm_target);
            std::string w;
            while (ss >> w) {
                if (w.length() >= 3) target_words.push_back(w);
            }

            if (!target_words.empty()) {
                std::string best_match;
                int max_score = 0;

                for (const auto& [key, anlz_path] : anlz_cache) {
                    int match_count = 0;
                    for (const auto& tw : target_words) {
                        if (key.find(tw) != std::string::npos) {
                            match_count++;
                        }
                    }
                    if (match_count > max_score) {
                        max_score = match_count;
                        best_match = anlz_path;
                    }
                }

                // Se coincidiu com mais de 60% das palavras chave da faixa
                if (max_score >= (int)std::ceil(target_words.size() * 0.60) && max_score >= 2) {
                    return best_match;
                }
            }

            return "";
        }

        size_t getCacheSize() const {
            return anlz_cache.size();
        }

        const std::unordered_map<std::string, std::string>& getCache() const {
            return anlz_cache;
        }

        // Faz o parsing completo do arquivo binário ANLZ0000.DAT do Pioneer Rekordbox
        bool parseAnlzDAT(const std::string& dat_path, RekordboxAnalysis& out, double sample_rate = 44100.0) {
            std::ifstream f(dat_path, std::ios::binary);
            if (!f.is_open()) return false;

            f.seekg(0, std::ios::end);
            size_t file_size = (size_t)f.tellg();
            f.seekg(0, std::ios::beg);

            if (file_size < 32) return false;

            std::vector<uint8_t> data(file_size);
            f.read(reinterpret_cast<char*>(data.data()), file_size);

            out = RekordboxAnalysis();
            out.source_anlz_path = dat_path;

            size_t pos = 0;
            while (pos + 12 <= file_size) {
                char tag[5] = { (char)data[pos], (char)data[pos+1], (char)data[pos+2], (char)data[pos+3], 0 };
                uint32_t hlen = readU32BE(&data[pos + 4]);
                uint32_t tlen = readU32BE(&data[pos + 8]);

                if (std::string(tag) == "PMAI") {
                    pos += hlen;
                    continue;
                }
                if (tlen == 0 || pos + tlen > file_size) break;

                if (std::string(tag) == "PPTH") {
                    if (tlen > hlen) {
                        out.title = utf16BEtoUTF8(&data[pos + hlen], tlen - hlen);
                        out.raw_path = out.title;
                    }
                } else if (std::string(tag) == "PQTZ") {
                    // Tag PQTZ: Pioneer Quantize Beat Grid
                    if (tlen > hlen) {
                        const uint8_t* payload = &data[pos + hlen];
                        size_t payload_len = tlen - hlen;
                        size_t num_beats = payload_len / 8;

                        out.beats.reserve(num_beats);
                        double sum_bpm = 0.0;
                        int first_downbeat_idx = -1;

                        for (size_t b = 0; b < num_beats; b++) {
                            const uint8_t* ep = payload + (b * 8);
                            RekordboxBeat beat;
                            beat.bar_beat = (int)readU16BE(ep);
                            beat.tempo_bpm = (double)readU16BE(ep + 2) / 100.0;
                            beat.time_ms = (double)readU32BE(ep + 4);
                            beat.time_sec = beat.time_ms * 0.001;
                            beat.frame = (int64_t)(beat.time_sec * sample_rate);

                            if (beat.bar_beat == 1 && first_downbeat_idx < 0) {
                                first_downbeat_idx = (int)b;
                            }
                            sum_bpm += beat.tempo_bpm;
                            out.beats.push_back(beat);
                        }

                        if (!out.beats.empty()) {
                            out.total_beats = (int)out.beats.size();
                            out.main_bpm = (first_downbeat_idx >= 0) ? out.beats[first_downbeat_idx].tempo_bpm : (sum_bpm / out.total_beats);
                            
                            // Define o verdadeiro Downbeat 1.1 da música
                            if (first_downbeat_idx >= 0) {
                                out.first_beat_ms = out.beats[first_downbeat_idx].time_ms;
                            } else {
                                out.first_beat_ms = out.beats[0].time_ms;
                            }
                            out.first_beat_sec = out.first_beat_ms * 0.001;
                            out.first_beat_frame = (int64_t)(out.first_beat_sec * sample_rate);
                            out.is_valid = true;
                        }
                    }
                }

                pos += tlen;
            }

            if (out.is_valid) {
                KuroUtils::Log("[Rekordbox Engine] PQTZ Carregado com Sucesso: " + std::to_string(out.total_beats) + 
                               " batidas, BPM=" + std::to_string(out.main_bpm) + 
                               ", Downbeat 1.1 em " + std::to_string(out.first_beat_sec) + "s (" + 
                               std::to_string(out.first_beat_ms) + "ms) - Arquivo: " + dat_path);
            }

            return out.is_valid;
        }

        // Tenta carregar a grade do Rekordbox para uma faixa de áudio
        bool loadTrackRekordboxGrid(const std::string& track_path, RekordboxAnalysis& out, double sample_rate = 44100.0) {
            // 1. Procura arquivo ANLZ0000.DAT correspondente
            std::string dat_path = findAnlzForTrack(track_path);
            if (!dat_path.empty()) {
                if (parseAnlzDAT(dat_path, out, sample_rate)) {
                    saveToMeta(track_path, out);
                    return true;
                }
            }

            // 2. Se não encontrou ANLZ, tenta ler cache .meta existente gerado por rekordbox
            std::filesystem::path meta_path(track_path);
            meta_path.replace_extension(".meta");
            if (std::filesystem::exists(meta_path)) {
                std::ifstream mf(meta_path);
                if (mf.is_open()) {
                    std::string line;
                    bool is_rb = false;
                    double mbpm = 0.0;
                    double fsec = -1.0;
                    int tbeats = 0;

                    while (std::getline(mf, line)) {
                        if (line.find("\"analyzer\": \"rekordbox_anlz\"") != std::string::npos ||
                            line.find("\"analyzer\":\"rekordbox_anlz\"") != std::string::npos) {
                            is_rb = true;
                        }
                        size_t bpos = line.find("\"bpm\":");
                        if (bpos != std::string::npos) {
                            try { mbpm = std::stod(line.substr(bpos + 6)); } catch (...) {}
                        }
                        size_t spos = line.find("\"first_beat_sec\":");
                        if (spos != std::string::npos) {
                            try { fsec = std::stod(line.substr(spos + 17)); } catch (...) {}
                        }
                        size_t tbpos = line.find("\"total_beats\":");
                        if (tbpos != std::string::npos) {
                            try { tbeats = std::stoi(line.substr(tbpos + 14)); } catch (...) {}
                        }
                    }

                    if (is_rb && mbpm > 20.0 && fsec >= 0.0) {
                        out.is_valid = true;
                        out.main_bpm = mbpm;
                        out.first_beat_sec = fsec;
                        out.first_beat_ms = fsec * 1000.0;
                        out.first_beat_frame = (int64_t)(fsec * sample_rate);
                        out.total_beats = tbeats;
                        KuroUtils::Log("[Rekordbox Engine] Metadado Rekordbox carregado do disco: BPM=" + 
                                       std::to_string(mbpm) + ", Downbeat=" + std::to_string(fsec) + "s");
                        return true;
                    }
                }
            }

            return false;
        }

        // Salva metadado da grade do Rekordbox no arquivo .meta ao lado da música
        bool saveToMeta(const std::string& track_path, const RekordboxAnalysis& anlz) {
            if (!anlz.is_valid) return false;

            std::filesystem::path meta_path(track_path);
            meta_path.replace_extension(".meta");

            std::ofstream mf(meta_path);
            if (!mf.is_open()) return false;

            mf << "{\n";
            mf << "  \"analyzer\": \"rekordbox_anlz\",\n";
            mf << "  \"source_anlz\": \"" << anlz.source_anlz_path << "\",\n";
            mf << "  \"bpm\": " << anlz.main_bpm << ",\n";
            mf << "  \"first_beat_sec\": " << anlz.first_beat_sec << ",\n";
            mf << "  \"first_beat_ms\": " << anlz.first_beat_ms << ",\n";
            mf << "  \"first_beat_frame\": " << anlz.first_beat_frame << ",\n";
            mf << "  \"total_beats\": " << anlz.total_beats << ",\n";
            mf << "  \"is_downbeat_exact\": true\n";
            mf << "}\n";

            return true;
        }
    };

} // namespace KuroAudio
