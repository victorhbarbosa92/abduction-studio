import sys

def patch_spotify_service():
    path = "C:/Users/o_vic/.gemini/antigravity-ide/scratch/abduction_studio_v2/src/core/SpotifyDJService.h"
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()

    target = """            item.target_deck = target_deck;
            enqueueDownloadItem(item);
        }
    };"""

    replacement = """            item.target_deck = target_deck;
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
            size_t ref_pos = content.find("\\"reference\\":");
            if (ref_pos != std::string::npos) {
                size_t robj_start = content.find('{', ref_pos);
                size_t robj_end = content.find('}', robj_start);
                if (robj_start != std::string::npos && robj_end != std::string::npos) {
                    std::string robj = content.substr(robj_start, robj_end - robj_start + 1);
                    auto get_rfield = [&](const std::string& key) -> std::string {
                        std::string needle = "\\"" + key + "\\":";
                        size_t kpos = robj.find(needle);
                        if (kpos == std::string::npos) return "";
                        size_t vstart = robj.find_first_not_of(" \\t\\r\\n", kpos + needle.length());
                        if (vstart == std::string::npos) return "";
                        if (robj[vstart] == '\\"') {
                            size_t vend = robj.find('\\"', vstart + 1);
                            if (vend != std::string::npos) return robj.substr(vstart + 1, vend - vstart - 1);
                        } else {
                            size_t vend = robj.find_first_of(",}\\r\\n", vstart);
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
            size_t res_array = content.find("\\"results\\":");
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
                    std::string needle = "\\"" + key + "\\":";
                    size_t kpos = obj.find(needle);
                    if (kpos == std::string::npos) return "";
                    size_t vstart = obj.find_first_not_of(" \\t\\r\\n", kpos + needle.length());
                    if (vstart == std::string::npos) return "";
                    if (obj[vstart] == '\\"') {
                        size_t vend = obj.find('\\"', vstart + 1);
                        if (vend != std::string::npos) return obj.substr(vstart + 1, vend - vstart - 1);
                    } else {
                        size_t vend = obj.find_first_of(",}\\r\\n", vstart);
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

                std::wstring wcmd = L"\\"" + py_path.wstring() + L"\\" \\"" + worker_path.wstring() + L"\\" dig_deeper --query \\""
                    + std::filesystem::path(query).wstring() + L"\\"";
                if (!artist.empty()) {
                    wcmd += L" --artist \\"" + std::filesystem::path(artist).wstring() + L"\\"";
                }
                if (!key.empty()) {
                    wcmd += L" --key \\"" + std::filesystem::path(key).wstring() + L"\\"";
                }
                if (bpm > 0.0) {
                    wcmd += L" --bpm " + std::to_wstring(bpm);
                }
                wcmd += L" --out_json \\"" + out_json.wstring() + L"\\"";

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
                std::wstring wcmd = L"\\"" + py_path.wstring() + L"\\" \\"" + worker_path.wstring() + L"\\" dig_feedback --ref \\""
                    + std::filesystem::path(ref_id).wstring() + L"\\" --target \\""
                    + std::filesystem::path(target_id).wstring() + L"\\" --feedback_action \\""
                    + std::filesystem::path(action).wstring() + L"\\" --deck " + std::to_wstring(deck);
                RunHiddenProcess(wcmd);
            }).detach();
        }
    };"""

    if target not in content:
        print("Target not found!")
        sys.exit(1)

    new_content = content.replace(target, replacement, 1)
    with open(path, "w", encoding="utf-8") as f:
        f.write(new_content)
    print("Patched SpotifyDJService.h successfully!")

if __name__ == "__main__":
    patch_spotify_service()
