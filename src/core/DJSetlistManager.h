#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <mutex>
#include <cmath>
#include <filesystem>
#include "DJHarmonicEngine.h"
#include "DJLibraryManager.h"

namespace KuroAudio {

    // 4 Fases Estratégicas de um DJ Set Perfeito
    enum class DJSetPhase {
        OPENER_HOOK = 0,    // 0% a 20%: Abertura, Hook Marcante, Prender a Pista
        BUILD_UP = 1,       // 20% a 60%: Construção Harmônica, Subida de Energia (+1 Camelot)
        PEAK_TIME = 2,      // 60% a 85%: Clímax, Máxima Intensidade (Energy Boost +2)
        OUTRO_RESOLVE = 3   // 85% a 100%: Desfecho Épico, Resolução Melódica / Major Mode
    };

    inline const char* getPhaseName(int phase) {
        switch (phase) {
            case 0: return "1. Hook & Abertura (0-20%)";
            case 1: return "2. Construção & Tensão (20-60%)";
            case 2: return "3. Peak Time / Clímax (60-85%)";
            case 3: return "4. Desfecho Épico (85-100%)";
            default: return "Geral";
        }
    }

    inline const char* getPhaseShortName(int phase) {
        switch (phase) {
            case 0: return "Abertura";
            case 1: return "Build-Up";
            case 2: return "Peak Time";
            case 3: return "Desfecho";
            default: return "Set";
        }
    }

    inline unsigned int getPhaseColor(int phase) {
        switch (phase) {
            case 0: return IM_COL32(0, 220, 255, 255);    // Ciano Neon
            case 1: return IM_COL32(255, 185, 30, 255);   // Âmbar / Ouro
            case 2: return IM_COL32(255, 45, 120, 255);   // Magenta Neon
            case 3: return IM_COL32(110, 240, 140, 255);  // Verde Esmeralda
            default: return IM_COL32(180, 180, 180, 255);
        }
    }

    inline ImVec4 getPhaseColorVec4(int phase) {
        switch (phase) {
            case 0: return ImVec4(0.0f, 0.86f, 1.0f, 1.0f);   // Ciano Neon
            case 1: return ImVec4(1.0f, 0.72f, 0.12f, 1.0f);  // Âmbar / Ouro
            case 2: return ImVec4(1.0f, 0.18f, 0.47f, 1.0f);  // Magenta Neon
            case 3: return ImVec4(0.43f, 0.94f, 0.55f, 1.0f); // Verde Esmeralda
            default: return ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
        }
    }

    struct DJSetlistItem {
        std::string track_id;
        std::string title;
        std::string artist;
        double bpm = 128.0;
        std::string key = "8A / Am";
        std::string subgenre = "Progressive Psytrance";
        double duration_sec = 240.0;
        int phase = 0; // 0=Abertura, 1=Build, 2=Peak, 3=Outro
        float energy = 7.0f; // 1.0 a 10.0
        std::string transition_note;
    };

    struct DJSetlist {
        std::string id;
        std::string name;
        std::string description;
        std::vector<DJSetlistItem> items;
    };

    struct NextTrackSuggestion {
        DJLibraryTrack track;
        int match_score = 0; // 0 a 100
        HarmonicCompatibility harmonic_relation;
        double bpm_diff = 0.0;
        std::string camelot_badge;
        std::string mixing_advice;
        int suggested_phase = 0;
    };

    class DJSetlistManager {
    private:
        std::vector<DJSetlist> setlists;
        std::string active_setlist_id = "default_set";
        std::mutex setlist_mutex;
        std::string db_path = "scratch/tracks/dj_setlists.json";

    public:
        DJSetlistManager() {
            initDefaultSetlists();
            loadFromFile();
        }

        static DJSetlistManager& getInstance() {
            static DJSetlistManager instance;
            return instance;
        }

        void initDefaultSetlists() {
            setlists.clear();
            DJSetlist def;
            def.id = "default_set";
            def.name = "Meu Set Psytrance Flagship";
            def.description = "Setlist estruturado por radar harmônico para retenção máxima de pista.";
            setlists.push_back(def);
            active_setlist_id = "default_set";
        }

        std::vector<DJSetlist> getSetlists() {
            std::lock_guard<std::mutex> lock(setlist_mutex);
            return setlists;
        }

        std::vector<DJSetlist> getAllSetlists() {
            return getSetlists();
        }

        std::string getActiveSetlistId() const { return active_setlist_id; }
        void setActiveSetlistId(const std::string& id) { active_setlist_id = id; }

        DJSetlist* getActiveSetlist() {
            for (auto& s : setlists) {
                if (s.id == active_setlist_id) return &s;
            }
            if (!setlists.empty()) return &setlists[0];
            return nullptr;
        }

        bool createSetlist(const std::string& name, const std::string& desc = "") {
            if (name.empty()) return false;
            std::lock_guard<std::mutex> lock(setlist_mutex);
            std::string id = "set_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count() % 1000000);
            setlists.push_back({ id, name, desc, {} });
            active_setlist_id = id;
            saveToFile();
            return true;
        }

        bool renameSetlist(const std::string& id, const std::string& new_name) {
            if (new_name.empty()) return false;
            std::lock_guard<std::mutex> lock(setlist_mutex);
            for (auto& s : setlists) {
                if (s.id == id) {
                    s.name = new_name;
                    saveToFile();
                    return true;
                }
            }
            return false;
        }

        bool deleteSetlist(const std::string& id) {
            std::lock_guard<std::mutex> lock(setlist_mutex);
            auto it = std::remove_if(setlists.begin(), setlists.end(), [&](const DJSetlist& s) {
                return s.id == id;
            });
            if (it != setlists.end()) {
                setlists.erase(it, setlists.end());
                if (setlists.empty()) {
                    initDefaultSetlists();
                } else if (active_setlist_id == id) {
                    active_setlist_id = setlists[0].id;
                }
                saveToFile();
                return true;
            }
            return false;
        }

        bool addTrackToSetlist(const std::string& setlist_id, const DJLibraryTrack& trk, int phase = -1, const std::string& note = "") {
            std::lock_guard<std::mutex> lock(setlist_mutex);
            DJSetlist* s = nullptr;
            for (auto& set : setlists) {
                if (set.id == setlist_id) { s = &set; break; }
            }
            if (!s) {
                for (auto& set : setlists) {
                    if (set.id == active_setlist_id) { s = &set; break; }
                }
            }
            if (!s && !setlists.empty()) s = &setlists[0];
            if (!s) return false;

            int target_phase = phase;
            if (target_phase < 0) {
                size_t count = s->items.size();
                if (count == 0) target_phase = 0;
                else if (count <= 3) target_phase = 1;
                else if (count <= 6) target_phase = 2;
                else target_phase = 3;
            }

            DJSetlistItem item;
            item.track_id = trk.id;
            item.title = trk.title;
            item.artist = trk.artist;
            item.bpm = trk.bpm;
            item.key = trk.key;
            item.subgenre = trk.subgenre;
            item.duration_sec = trk.duration_sec;
            item.phase = target_phase;
            item.energy = (target_phase == 2) ? 9.0f : ((target_phase == 1) ? 7.5f : 6.0f);

            if (!s->items.empty()) {
                const auto& prev = s->items.back();
                item.transition_note = generateTransitionAdvice(prev.key, item.key, prev.bpm, item.bpm);
            } else {
                item.transition_note = "Faixa de Abertura: Estabelece a identidade e energia inicial da pista.";
            }

            if (!note.empty()) item.transition_note = note;

            s->items.push_back(item);
            saveToFile();
            return true;
        }

        bool addTrackToActiveSetlist(const DJLibraryTrack& trk, int phase = -1, const std::string& note = "") {
            return addTrackToSetlist(active_setlist_id, trk, phase, note);
        }

        bool removeTrackFromActiveSetlist(size_t index) {
            std::lock_guard<std::mutex> lock(setlist_mutex);
            DJSetlist* s = nullptr;
            for (auto& set : setlists) {
                if (set.id == active_setlist_id) { s = &set; break; }
            }
            if (!s || index >= s->items.size()) return false;
            s->items.erase(s->items.begin() + index);
            saveToFile();
            return true;
        }

        bool removeTrackFromSetlist(const std::string& setlist_id, size_t index) {
            std::lock_guard<std::mutex> lock(setlist_mutex);
            DJSetlist* s = nullptr;
            for (auto& set : setlists) {
                if (set.id == setlist_id) { s = &set; break; }
            }
            if (!s || index >= s->items.size()) return false;
            s->items.erase(s->items.begin() + index);
            saveToFile();
            return true;
        }

        bool moveItem(size_t from_idx, size_t to_idx) {
            std::lock_guard<std::mutex> lock(setlist_mutex);
            DJSetlist* s = nullptr;
            for (auto& set : setlists) {
                if (set.id == active_setlist_id) { s = &set; break; }
            }
            if (!s || from_idx >= s->items.size() || to_idx >= s->items.size() || from_idx == to_idx) return false;
            auto item = s->items[from_idx];
            s->items.erase(s->items.begin() + from_idx);
            s->items.insert(s->items.begin() + to_idx, item);

            // Recalcula notas de transição relativas
            for (size_t i = 1; i < s->items.size(); i++) {
                s->items[i].transition_note = generateTransitionAdvice(
                    s->items[i - 1].key, s->items[i].key, s->items[i - 1].bpm, s->items[i].bpm
                );
            }

            saveToFile();
            return true;
        }

        // Radar Inteligente de Próxima Música (Next Track Radar Engine)
        std::vector<NextTrackSuggestion> getRadarSuggestions(const std::string& ref_track_id, const std::string& ref_key, double ref_bpm, int target_phase = -1, size_t limit = 8) {
            std::vector<NextTrackSuggestion> results;
            auto all_tracks = DJLibraryManager::getInstance().getTracksForFolder("all");

            for (const auto& trk : all_tracks) {
                if (trk.id == ref_track_id) continue;

                NextTrackSuggestion sug;
                sug.track = trk;
                sug.harmonic_relation = DJHarmonicEngine::compareKeys(ref_key, trk.key);
                int harm_score = DJHarmonicEngine::getCompatibilityScore(ref_key, trk.key);

                sug.bpm_diff = std::abs(ref_bpm - trk.bpm);
                double bpm_score = std::max(0.0, 100.0 - sug.bpm_diff * 12.0);

                // Pontuação Combinada de Curadoria
                double combined = (harm_score * 0.65) + (bpm_score * 0.35);
                sug.match_score = std::clamp((int)combined, 10, 100);

                // Badge Camelot
                CamelotKey ck = DJHarmonicEngine::parseKey(trk.key);
                sug.camelot_badge = ck.getCamelotCode();

                // Dica Profissional de Mixagem
                sug.mixing_advice = generateTransitionAdvice(ref_key, trk.key, ref_bpm, trk.bpm);

                // Sugestão de Fase
                if (sug.harmonic_relation == HarmonicCompatibility::ENERGY_BOOST_2) sug.suggested_phase = 2; // Peak
                else if (sug.harmonic_relation == HarmonicCompatibility::ENERGY_PLUS_1) sug.suggested_phase = 1; // Build
                else if (sug.harmonic_relation == HarmonicCompatibility::PERFECT) sug.suggested_phase = 1;
                else sug.suggested_phase = 0;

                if (target_phase >= 0 && sug.suggested_phase != target_phase && sug.match_score < 75) {
                    continue;
                }

                results.push_back(sug);
            }

            // Ordena pelo maior score de compatibilidade
            std::sort(results.begin(), results.end(), [](const NextTrackSuggestion& a, const NextTrackSuggestion& b) {
                return a.match_score > b.match_score;
            });

            if (results.size() > limit) results.resize(limit);
            return results;
        }

        static std::string generateTransitionAdvice(const std::string& keyA, const std::string& keyB, double bpmA, double bpmB) {
            auto comp = DJHarmonicEngine::compareKeys(keyA, keyB);
            CamelotKey cA = DJHarmonicEngine::parseKey(keyA);
            CamelotKey cB = DJHarmonicEngine::parseKey(keyB);
            double bpm_diff = bpmB - bpmA;

            std::stringstream ss;
            switch (comp) {
                case HarmonicCompatibility::PERFECT:
                    ss << "★ TRANSIÇÃO PERFEITA (" << cA.getCamelotCode() << "): Mesmo tom. Mixagem longa e imperceptível sem choque de baixos.";
                    break;
                case HarmonicCompatibility::HARMONIC_RELATIVE:
                    ss << "◆ RELATIVO MAIOR/MENOR (" << cA.getCamelotCode() << " ➔ " << cB.getCamelotCode() << "): Mudança de atmosfera sutil mantendo 100% de consonância.";
                    break;
                case HarmonicCompatibility::ENERGY_PLUS_1:
                    ss << "▲ ELEVAÇÃO DE ENERGIA (+1 Camelot: " << cA.getCamelotCode() << " ➔ " << cB.getCamelotCode() << "): Eleva a tensão da pista de forma natural e vibrante.";
                    break;
                case HarmonicCompatibility::ENERGY_MINUS_1:
                    ss << "▼ TRANSIÇÃO SUAVE / ALÍVIO (-1 Camelot: " << cA.getCamelotCode() << " ➔ " << cB.getCamelotCode() << "): Ideal para respiro ou preparar a subida seguinte.";
                    break;
                case HarmonicCompatibility::ENERGY_BOOST_2:
                    ss << "⚡ ENERGY BOOST EXPLOSIVO (+2 Camelot: " << cA.getCamelotCode() << " ➔ " << cB.getCamelotCode() << "): Salto de 2 tons para o clímax do set / drop principal!";
                    break;
                case HarmonicCompatibility::DIAGONAL_PLUS_1:
                    ss << "↗ DIAGONAL (+1 & Modo): Modulação melódica elegante para criar surpresa na pista.";
                    break;
                default:
                    ss << "⚠ MODULAÇÃO LIVRE (" << cA.getCamelotCode() << " ➔ " << cB.getCamelotCode() << "): Recomendado transição por corte seco (Drop Cut) ou filtro de reverb.";
                    break;
            }

            if (std::abs(bpm_diff) > 2.5) {
                char buf[64];
                snprintf(buf, sizeof(buf), " [BPM: %+.1f - ajuste o pitch antes do drop]", bpm_diff);
                ss << buf;
            } else if (std::abs(bpm_diff) <= 0.5) {
                ss << " [BPM em sincronia cirúrgica]";
            }

            return ss.str();
        }

        void saveToFile() {
            try {
                std::filesystem::path p(db_path);
                std::filesystem::create_directories(p.parent_path());
                std::ofstream f(p);
                if (!f.is_open()) return;

                f << "{\n  \"active_setlist\": \"" << active_setlist_id << "\",\n  \"setlists\": [\n";
                for (size_t i = 0; i < setlists.size(); i++) {
                    const auto& s = setlists[i];
                    f << "    {\n      \"id\": \"" << s.id 
                      << "\",\n      \"name\": \"" << escapeJson(s.name)
                      << "\",\n      \"description\": \"" << escapeJson(s.description)
                      << "\",\n      \"items\": [\n";
                    for (size_t j = 0; j < s.items.size(); j++) {
                        const auto& item = s.items[j];
                        f << "        { \"track_id\": \"" << item.track_id 
                          << "\", \"title\": \"" << escapeJson(item.title)
                          << "\", \"artist\": \"" << escapeJson(item.artist)
                          << "\", \"bpm\": " << item.bpm
                          << ", \"key\": \"" << item.key
                          << "\", \"subgenre\": \"" << escapeJson(item.subgenre)
                          << "\", \"duration_sec\": " << item.duration_sec
                          << ", \"phase\": " << item.phase
                          << ", \"energy\": " << item.energy
                          << ", \"transition_note\": \"" << escapeJson(item.transition_note)
                          << "\" }" << (j + 1 < s.items.size() ? ",\n" : "\n");
                    }
                    f << "      ]\n    }" << (i + 1 < setlists.size() ? ",\n" : "\n");
                }
                f << "  ]\n}\n";
            } catch (...) {}
        }

        void loadFromFile() {
            try {
                std::filesystem::path p(db_path);
                if (!std::filesystem::exists(p)) return;
                std::ifstream f(p);
                if (!f.is_open()) return;
                std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                if (content.empty()) return;

                auto extractJsonField = [](const std::string& obj, const std::string& key) -> std::string {
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

                std::string act_id = extractJsonField(content, "active_setlist");

                size_t s_pos = content.find("\"setlists\":");
                if (s_pos == std::string::npos) return;
                size_t start_arr = content.find('[', s_pos);
                size_t end_arr = content.rfind(']');
                if (start_arr == std::string::npos || end_arr == std::string::npos || end_arr <= start_arr) return;

                std::vector<DJSetlist> loaded_sets;
                std::string sets_str = content.substr(start_arr + 1, end_arr - start_arr - 1);

                size_t pos = 0;
                while (pos < sets_str.length()) {
                    size_t cur_set_start = sets_str.find('{', pos);
                    if (cur_set_start == std::string::npos) break;

                    size_t items_pos = sets_str.find("\"items\":", cur_set_start);
                    if (items_pos == std::string::npos) break;
                    size_t items_arr_start = sets_str.find('[', items_pos);
                    if (items_arr_start == std::string::npos) break;
                    size_t items_arr_end = sets_str.find(']', items_arr_start);
                    if (items_arr_end == std::string::npos) break;
                    size_t cur_set_end = sets_str.find('}', items_arr_end);
                    if (cur_set_end == std::string::npos) break;

                    std::string set_header = sets_str.substr(cur_set_start, items_pos - cur_set_start);
                    DJSetlist new_set;
                    new_set.id = extractJsonField(set_header, "id");
                    new_set.name = extractJsonField(set_header, "name");
                    new_set.description = extractJsonField(set_header, "description");
                    if (new_set.id.empty()) new_set.id = "set_" + std::to_string(loaded_sets.size());

                    std::string items_substr = sets_str.substr(items_arr_start + 1, items_arr_end - items_arr_start - 1);
                    size_t it_start = 0;
                    while ((it_start = items_substr.find('{', it_start)) != std::string::npos) {
                        size_t it_end = items_substr.find('}', it_start);
                        if (it_end == std::string::npos) break;
                        std::string it_obj = items_substr.substr(it_start, it_end - it_start + 1);

                        DJSetlistItem item;
                        item.track_id = extractJsonField(it_obj, "track_id");
                        item.title = extractJsonField(it_obj, "title");
                        item.artist = extractJsonField(it_obj, "artist");
                        std::string bpm_str = extractJsonField(it_obj, "bpm");
                        if (!bpm_str.empty()) try { item.bpm = std::stod(bpm_str); } catch (...) {}
                        item.key = extractJsonField(it_obj, "key");
                        item.subgenre = extractJsonField(it_obj, "subgenre");
                        std::string dur_str = extractJsonField(it_obj, "duration_sec");
                        if (!dur_str.empty()) try { item.duration_sec = std::stod(dur_str); } catch (...) {}
                        std::string ph_str = extractJsonField(it_obj, "phase");
                        if (!ph_str.empty()) try { item.phase = std::stoi(ph_str); } catch (...) {}
                        std::string en_str = extractJsonField(it_obj, "energy");
                        if (!en_str.empty()) try { item.energy = std::stof(en_str); } catch (...) {}
                        item.transition_note = extractJsonField(it_obj, "transition_note");

                        new_set.items.push_back(item);
                        it_start = it_end + 1;
                    }

                    loaded_sets.push_back(new_set);
                    pos = cur_set_end + 1;
                }

                if (!loaded_sets.empty()) {
                    std::lock_guard<std::mutex> lock(setlist_mutex);
                    setlists = loaded_sets;
                    if (!act_id.empty()) active_setlist_id = act_id;
                    else active_setlist_id = setlists[0].id;
                }
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
