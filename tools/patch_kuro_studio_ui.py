import os
import sys

HEADER_PATH = r"C:\Users\o_vic\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroDJStudioUI.h"

with open(HEADER_PATH, "r", encoding="utf-8") as f:
    c = f.read()

# 1. Add structs before class KuroDJStudioUI
structs_def = """    struct TapTempoTracker {
        std::vector<double> tap_times;
        double last_tap_time = 0.0;
        float flash_timer = 0.0f;
        
        double registerTap(double current_time_sec) {
            flash_timer = 0.35f;
            if (current_time_sec - last_tap_time > 2.2) {
                tap_times.clear();
            }
            last_tap_time = current_time_sec;
            tap_times.push_back(current_time_sec);
            if (tap_times.size() > 8) {
                tap_times.erase(tap_times.begin());
            }
            if (tap_times.size() < 2) return 0.0;
            
            double total_diff = 0.0;
            for (size_t i = 1; i < tap_times.size(); i++) {
                total_diff += (tap_times[i] - tap_times[i - 1]);
            }
            double avg_diff = total_diff / (double)(tap_times.size() - 1);
            if (avg_diff > 0.05) {
                double calc_bpm = 60.0 / avg_diff;
                return std::clamp(calc_bpm, 40.0, 240.0);
            }
            return 0.0;
        }
    };

    struct CustomPadConfig {
        int fx_type = 6; // 6 = FX_VINYL_BRAKE
        std::string name = "VINYL BRK";
        bool is_toggle = false; // false = Hold, true = Toggle
        bool is_active = false;
        float depth = 0.85f;
        float beat_fraction = 1.0f;
    };

    class KuroDJStudioUI {"""

target_class_kuro = "    class KuroDJStudioUI {"
if target_class_kuro in c:
    c = c.replace(target_class_kuro, structs_def, 1)
    print("1. Added TapTempoTracker and CustomPadConfig structs.")
else:
    print("Error: target_class_kuro not found!")

# 2. Add members to KuroDJStudioUI
target_members = "        bool show_rekordbox_fx_bar = true;"
replacement_members = """        bool show_rekordbox_fx_bar = true;
        TapTempoTracker tap_trackers[2];
        CustomPadConfig custom_pads[4][8];
        std::string selected_usb_drive = "";
        std::vector<KuroAudio::DJLibraryTrack> usb_scanned_tracks;
        float usb_scan_timer = 0.0f;
        std::vector<KuroAudio::USBDeviceDrive> cached_usb_drives;
        bool rekordbox_fx1_hold = false;

        void scanUSBTracks(const std::string& drive_root) {
            usb_scanned_tracks.clear();
            try {
                std::filesystem::path root(drive_root);
                if (!std::filesystem::exists(root)) return;
                std::vector<std::filesystem::path> search_paths = { root, root / "Music", root / "Tracks", root / "PIONEER" / "Contents" };
                for (const auto& sp : search_paths) {
                    if (!std::filesystem::exists(sp)) continue;
                    for (const auto& entry : std::filesystem::directory_iterator(sp)) {
                        if (entry.is_regular_file()) {
                            auto ext = entry.path().extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                            if (ext == ".wav" || ext == ".mp3" || ext == ".flac" || ext == ".aiff" || ext == ".m4a") {
                                KuroAudio::DJLibraryTrack trk;
                                std::string fn = entry.path().stem().string();
                                trk.id = "usb_" + std::to_string(std::hash<std::string>{}(entry.path().string()));
                                size_t sep = fn.find(" - ");
                                if (sep != std::string::npos) {
                                    trk.artist = fn.substr(0, sep);
                                    trk.title = fn.substr(sep + 3);
                                } else {
                                    trk.artist = "Pen Drive USB";
                                    trk.title = fn;
                                }
                                trk.local_wav_path = entry.path().string();
                                trk.bpm = 138.0;
                                trk.key = "8A / Am";
                                trk.subgenre = "USB / Removable";
                                trk.is_downloaded = true;
                                usb_scanned_tracks.push_back(trk);
                                if (usb_scanned_tracks.size() >= 300) break;
                            }
                        }
                    }
                }
            } catch (...) {}
        }"""

if target_members in c:
    c = c.replace(target_members, replacement_members, 1)
    print("2. Added member variables and scanUSBTracks method.")
else:
    print("Error: target_members not found!")

# 3. Initialize custom_pads defaults in constructor
target_ctor = "            // Navegação de playlist pelo jog com shift"
replacement_ctor = """            // Inicializa 8 pads com efeitos customizados para todos os decks
            for (int d = 0; d < 4; d++) {
                custom_pads[d][0] = { KuroAudio::FX_SLIP_ROLL, "SLIP ROLL", false, false, 0.85f, 0.5f };
                custom_pads[d][1] = { KuroAudio::FX_VINYL_BRAKE, "VINYL BRK", false, false, 0.90f, 1.0f };
                custom_pads[d][2] = { KuroAudio::FX_ENIGMA_JET, "ENIGMA JET", false, false, 0.80f, 1.0f };
                custom_pads[d][3] = { KuroAudio::FX_HELIX, "HELIX", false, false, 0.80f, 1.0f };
                custom_pads[d][4] = { KuroAudio::FX_MOBIUS_SAW, "MOBIUS SAW", false, false, 0.75f, 1.0f };
                custom_pads[d][5] = { KuroAudio::FX_PITCH, "PITCH", false, false, 0.75f, 1.0f };
                custom_pads[d][6] = { KuroAudio::FX_REVERB_UP, "REVERB UP", false, false, 0.85f, 1.0f };
                custom_pads[d][7] = { KuroAudio::FX_LOW_CUT_ECHO, "LOW CUT", true, false, 0.85f, 0.75f };
            }

            // Navegação de playlist pelo jog com shift"""

if target_ctor in c:
    c = c.replace(target_ctor, replacement_ctor, 1)
    print("3. Initialized custom_pads in constructor.")
else:
    print("Error: target_ctor not found!")

# 4. Update setPadMode to allow mode 0 to 6
target_set_pad = "            mode = std::clamp(mode, 0, 5);"
replacement_set_pad = "            mode = std::clamp(mode, 0, 6);"
if target_set_pad in c:
    c = c.replace(target_set_pad, replacement_set_pad, 1)
    print("4. Updated setPadMode clamp to 0-6.")

with open(HEADER_PATH, "w", encoding="utf-8") as f:
    f.write(c)

print("Patch part 1 applied.")
