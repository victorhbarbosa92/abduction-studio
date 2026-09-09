#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include "DJEngine.h"
#include "SpotifyDJService.h"
#include "../utils/Logger.h"

namespace KuroAudio {

    struct USBDeviceDrive {
        std::string path;
        std::string label;
        bool is_removable = true;
    };

    class DJExportService {
    public:
        static std::vector<USBDeviceDrive> scanDrives() {
            std::vector<USBDeviceDrive> result;
            DWORD driveMask = GetLogicalDrives();
            
            for (char letter = 'D'; letter <= 'Z'; letter++) {
                if (driveMask & (1 << (letter - 'A'))) {
                    std::string root = std::string(1, letter) + ":\\";
                    UINT type = GetDriveTypeA(root.c_str());
                    
                    if (type == DRIVE_REMOVABLE || type == DRIVE_FIXED) {
                        char volumeName[MAX_PATH + 1] = { 0 };
                        GetVolumeInformationA(root.c_str(), volumeName, sizeof(volumeName), NULL, NULL, NULL, NULL, 0);
                        
                        USBDeviceDrive d;
                        d.path = root;
                        std::string vol = volumeName;
                        d.label = root + " " + (vol.empty() ? (type == DRIVE_REMOVABLE ? "[Pen Drive USB]" : "[Disco Local]") : "[" + vol + "]");
                        d.is_removable = (type == DRIVE_REMOVABLE);
                        result.push_back(d);
                    }
                }
            }
            return result;
        }

        static bool exportRekordboxPackage(const std::string& target_drive, const std::vector<SpotifyDJTrack>& tracks, const DJDeck decks[4], std::string& out_status) {
            try {
                std::filesystem::path root(target_drive);
                if (!std::filesystem::exists(root)) {
                    out_status = "Unidade selecionada não encontrada: " + target_drive;
                    return false;
                }

                std::filesystem::path pioneer_dir = root / "PIONEER" / "rekordbox";
                std::filesystem::path contents_dir = root / "Contents";
                std::filesystem::create_directories(pioneer_dir);
                std::filesystem::create_directories(contents_dir);

                std::stringstream xml;
                xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
                xml << "<DJ_PLAYLISTS Version=\"1.0.0\">\n";
                xml << "  <PRODUCT Name=\"rekordbox\" Version=\"6.0.0\" Company=\"Pioneer DJ\" />\n";
                xml << "  <COLLECTION Entries=\"" << tracks.size() << "\">\n";

                int track_id = 1;
                for (const auto& t : tracks) {
                    if (t.local_wav_path.empty() || !std::filesystem::exists(t.local_wav_path)) continue;

                    std::filesystem::path src_path(t.local_wav_path);
                    std::filesystem::path artist_dir = contents_dir / (t.artist.empty() ? "Various Artists" : t.artist);
                    std::filesystem::create_directories(artist_dir);

                    std::string safe_title = src_path.filename().string();
                    std::filesystem::path dest_file = artist_dir / safe_title;

                    std::error_code ec;
                    if (!std::filesystem::exists(dest_file, ec)) {
                        std::filesystem::copy_file(src_path, dest_file, std::filesystem::copy_options::overwrite_existing, ec);
                    }

                    std::string url_path = "file://localhost/" + dest_file.generic_string();

                    xml << "    <TRACK TrackID=\"" << track_id << "\" Name=\"" << escapeXml(t.title) << "\" Artist=\"" << escapeXml(t.artist) 
                        << "\" TotalTime=\"" << (int)t.duration_sec << "\" AverageBpm=\"" << std::fixed << std::setprecision(2) << t.bpm 
                        << "\" Tonality=\"" << escapeXml(t.key) << "\" Location=\"" << escapeXml(url_path) << "\">\n";
                    
                    xml << "      <TEMPO Inizio=\"0.000\" Bpm=\"" << std::fixed << std::setprecision(2) << t.bpm << "\" Metro=\"4/4\" Battito=\"1\" />\n";

                    // Hot Cues do Deck correspondente caso a música esteja carregada
                    for (int d = 0; d < 4; d++) {
                        if (decks[d].track_path == t.local_wav_path) {
                            for (int c = 0; c < 8; c++) {
                                if (decks[d].hot_cues[c].active) {
                                    uint32_t col = decks[d].hot_cues[c].color_rgba;
                                    int r = (col) & 0xFF;
                                    int g = (col >> 8) & 0xFF;
                                    int b = (col >> 16) & 0xFF;
                                    xml << "      <POSITION_MARK Name=\"CUE " << (c + 1) << "\" Type=\"0\" Start=\"" 
                                        << std::fixed << std::setprecision(3) << decks[d].hot_cues[c].time_sec 
                                        << "\" Num=\"" << c << "\" Red=\"" << r << "\" Green=\"" << g << "\" Blue=\"" << b << "\" />\n";
                                }
                            }
                            break;
                        }
                    }

                    xml << "    </TRACK>\n";
                    track_id++;
                }

                xml << "  </COLLECTION>\n";
                xml << "  <PLAYLISTS>\n";
                xml << "    <NODE Type=\"0\" Name=\"ROOT\">\n";
                xml << "      <NODE Name=\"Abduction DJ Suite Set\" Type=\"1\" KeyType=\"0\" Entries=\"" << (track_id - 1) << "\">\n";
                for (int i = 1; i < track_id; i++) {
                    xml << "        <TRACK Key=\"" << i << "\" />\n";
                }
                xml << "      </NODE>\n";
                xml << "    </NODE>\n";
                xml << "  </PLAYLISTS>\n";
                xml << "</DJ_PLAYLISTS>\n";

                std::ofstream out(pioneer_dir / "rekordbox.xml", std::ios::trunc);
                if (out.is_open()) {
                    out << xml.str();
                    out.close();
                    out_status = "Sucesso: Exportado no padrão Pioneer Rekordbox para " + target_drive;
                    return true;
                } else {
                    out_status = "Erro ao escrever rekordbox.xml no Pen Drive.";
                    return false;
                }
            } catch (const std::exception& e) {
                out_status = std::string("Erro na exportação: ") + e.what();
                return false;
            }
        }

        static bool exportUniversalPackage(const std::string& target_drive, const std::vector<SpotifyDJTrack>& tracks, const DJDeck decks[4], std::string& out_status) {
            try {
                std::filesystem::path root(target_drive);
                if (!std::filesystem::exists(root)) {
                    out_status = "Unidade selecionada não encontrada: " + target_drive;
                    return false;
                }

                std::filesystem::path music_dir = root / "Music";
                std::filesystem::create_directories(music_dir);

                std::stringstream m3u;
                m3u << "#EXTM3U\n";
                m3u << "#PLAYLIST:Abduction DJ Suite Live Set\n\n";

                std::stringstream json;
                json << "{\n  \"export_timestamp\": " << (long long)time(NULL) << ",\n  \"tracks\": [\n";

                bool first_json = true;
                for (const auto& t : tracks) {
                    if (t.local_wav_path.empty() || !std::filesystem::exists(t.local_wav_path)) continue;

                    std::filesystem::path src_path(t.local_wav_path);
                    std::filesystem::path artist_dir = music_dir / (t.artist.empty() ? "Various Artists" : t.artist);
                    std::filesystem::create_directories(artist_dir);

                    std::string safe_name = src_path.filename().string();
                    std::filesystem::path dest_file = artist_dir / safe_name;

                    std::error_code ec;
                    if (!std::filesystem::exists(dest_file, ec)) {
                        std::filesystem::copy_file(src_path, dest_file, std::filesystem::copy_options::overwrite_existing, ec);
                    }

                    // M3U Entry
                    m3u << "#EXTINF:" << (int)t.duration_sec << "," << t.artist << " - " << t.title << "\n";
                    m3u << "#EXT-X-BPM:" << std::fixed << std::setprecision(2) << t.bpm << "\n";
                    m3u << "#EXT-X-KEY:" << t.key << "\n";
                    m3u << dest_file.generic_string() << "\n\n";

                    // JSON metadata entry
                    if (!first_json) json << ",\n";
                    first_json = false;

                    json << "    {\n";
                    json << "      \"title\": \"" << escapeJson(t.title) << "\",\n";
                    json << "      \"artist\": \"" << escapeJson(t.artist) << "\",\n";
                    json << "      \"bpm\": " << t.bpm << ",\n";
                    json << "      \"key\": \"" << escapeJson(t.key) << "\",\n";
                    json << "      \"duration_sec\": " << t.duration_sec << ",\n";
                    json << "      \"file\": \"" << escapeJson(dest_file.filename().string()) << "\",\n";
                    json << "      \"hot_cues\": [";

                    bool first_cue = true;
                    for (int d = 0; d < 4; d++) {
                        if (decks[d].track_path == t.local_wav_path) {
                            for (int c = 0; c < 8; c++) {
                                if (decks[d].hot_cues[c].active) {
                                    if (!first_cue) json << ", ";
                                    first_cue = false;
                                    json << "{\"index\": " << (c + 1) << ", \"time\": " << std::fixed << std::setprecision(3) << decks[d].hot_cues[c].time_sec << "}";
                                }
                            }
                            break;
                        }
                    }
                    json << "]\n    }";
                }

                json << "\n  ]\n}\n";

                // Salva M3U8
                std::ofstream m3u_out(music_dir / "Abduction_Live_Set.m3u8", std::ios::trunc);
                if (m3u_out.is_open()) {
                    m3u_out << m3u.str();
                    m3u_out.close();
                }

                // Salva JSON Cues
                std::ofstream json_out(music_dir / "cues_metadata.json", std::ios::trunc);
                if (json_out.is_open()) {
                    json_out << json.str();
                    json_out.close();
                }

                out_status = "Sucesso: Músicas, playlist M3U8 e Cues salvos em " + target_drive;
                return true;
            } catch (const std::exception& e) {
                out_status = std::string("Erro na exportação universal: ") + e.what();
                return false;
            }
        }

    private:
        static std::string escapeXml(const std::string& input) {
            std::string res;
            for (char c : input) {
                if (c == '&') res += "&amp;";
                else if (c == '<') res += "&lt;";
                else if (c == '>') res += "&gt;";
                else if (c == '\"') res += "&quot;";
                else if (c == '\'') res += "&apos;";
                else res += c;
            }
            return res;
        }

        static std::string escapeJson(const std::string& input) {
            std::string res;
            for (char c : input) {
                if (c == '\\') res += "\\\\";
                else if (c == '\"') res += "\\\"";
                else if (c == '\n') res += "\\n";
                else if (c == '\r') res += "\\r";
                else res += c;
            }
            return res;
        }
    };

} // namespace KuroAudio
