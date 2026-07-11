#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <mutex>
#include "miniz.h"
#include "../plugin_manager/NativePlugins.h"
#include "../plugin_manager/DAG.h"
#include "../audio/SynthEngine.h"
#include "../audio/KuroWave.h"
#include "../core/TimelineManager.h"
#include "../utils/JSONHelper.h"

extern float track_volumes[MAX_TRACKS];
extern bool track_mutes[MAX_TRACKS];
extern bool track_solos[MAX_TRACKS];
extern KuroDSP::TimelineManager timeline;
extern KuroAudio::KuroWave g_kurowave;
extern KuroAudio::SynthEngine g_piano_synth;

namespace KuroUI {
    extern std::vector<int> track_fx_chain[MAX_TRACKS];
}

class ProjectManager {
public:
    using JSON = KuroUI::JSON;
    static void SaveProject(const std::string& filepath) {
        std::stringstream ss;
        ss << "{\n";
        ss << "  " << JSON::format("bpm", ::timeline.getBPM()) << ",\n";
        
        // Track volumes
        ss << "  \"track_volumes\": [";
        for (int i = 0; i < MAX_TRACKS; i++) {
            ss << ::track_volumes[i] << (i < MAX_TRACKS - 1 ? ", " : "");
        }
        ss << "],\n";
        
        // Track mutes
        ss << "  \"track_mutes\": [";
        for (int i = 0; i < MAX_TRACKS; i++) {
            ss << (::track_mutes[i] ? "true" : "false") << (i < MAX_TRACKS - 1 ? ", " : "");
        }
        ss << "],\n";
        
        // Track solos
        ss << "  \"track_solos\": [";
        for (int i = 0; i < MAX_TRACKS; i++) {
            ss << (::track_solos[i] ? "true" : "false") << (i < MAX_TRACKS - 1 ? ", " : "");
        }
        ss << "],\n";

        // Track FX chains
        ss << "  \"track_fx_chains\": [\n";
        for (int i = 0; i < MAX_TRACKS; i++) {
            ss << "    [";
            const auto& chain = KuroUI::track_fx_chain[i];
            for (size_t j = 0; j < chain.size(); j++) {
                ss << chain[j] << (j < chain.size() - 1 ? ", " : "");
            }
            ss << "]" << (i < MAX_TRACKS - 1 ? ",\n" : "\n");
        }
        ss << "  ],\n";

        // MIDI Notes per track
        ss << "  \"track_midi_notes\": [\n";
        for (int i = 0; i < MAX_TRACKS; i++) {
            ss << "    [\n";
            std::lock_guard<std::mutex> lock(::timeline.timeline_mutex);
            const auto& notes = ::timeline.track_notes[i];
            for (size_t j = 0; j < notes.size(); j++) {
                const auto& n = notes[j];
                ss << "      {\n";
                ss << "        " << JSON::format("pitch", n.pitch) << ",\n";
                ss << "        " << JSON::format("start_time", n.start_time) << ",\n";
                ss << "        " << JSON::format("duration", n.duration) << ",\n";
                ss << "        " << JSON::format("velocity", n.velocity) << "\n";
                ss << "      }" << (j < notes.size() - 1 ? ",\n" : "\n");
            }
            ss << "    ]" << (i < MAX_TRACKS - 1 ? ",\n" : "\n");
        }
        ss << "  ],\n";

        // KuroWave synth settings
        ss << "  \"kurowave\": {\n";
        ss << "    " << JSON::format("wt_position", ::g_kurowave.param_wt_position) << ",\n";
        ss << "    " << JSON::format("unison_voices", ::g_kurowave.param_unison_voices) << ",\n";
        ss << "    " << JSON::format("unison_detune", ::g_kurowave.param_unison_detune) << ",\n";
        ss << "    " << JSON::format("cutoff", ::g_kurowave.param_cutoff) << ",\n";
        ss << "    " << JSON::format("resonance", ::g_kurowave.param_resonance) << ",\n";
        ss << "    " << JSON::format("lfo_wt_mod", ::g_kurowave.param_lfo_wt_mod) << "\n";
        ss << "  },\n";

        // Piano synth settings
        ss << "  \"piano_synth\": {\n";
        ss << "    " << JSON::format("instrument", (int)::g_piano_synth.current_instrument) << "\n";
        ss << "  }\n";

        ss << "}\n";

        std::string configData = ss.str();
        remove(filepath.c_str());
        mz_bool status = mz_zip_add_mem_to_archive_file_in_place(filepath.c_str(), "project.json", configData.c_str(), configData.size(), "", 0, MZ_BEST_COMPRESSION);
        
        if (!status) {
            std::cerr << "[ProjectManager] Erro fatal ao zipar o projeto .kuro usando miniz!\n";
        } else {
            std::cout << "[ProjectManager] Projeto Portátil salvo com sucesso em formato ZIP (JSON): " << filepath << "\n";
        }
    }

    static void LoadProject(const std::string& filepath) {
        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));
        
        if (!mz_zip_reader_init_file(&zip_archive, filepath.c_str(), 0)) {
            std::cerr << "[ProjectManager] Arquivo .kuro nao encontrado ou invalido.\n";
            return;
        }
        
        int file_index = mz_zip_reader_locate_file(&zip_archive, "project.json", NULL, 0);
        if (file_index < 0) {
            std::cerr << "[ProjectManager] project.json ausente no arquivo .kuro!\n";
            mz_zip_reader_end(&zip_archive);
            return;
        }
        
        size_t uncomp_size;
        void* p = mz_zip_reader_extract_to_heap(&zip_archive, file_index, &uncomp_size, 0);
        if (!p) {
            std::cerr << "[ProjectManager] Erro ao extrair project.json\n";
            mz_zip_reader_end(&zip_archive);
            return;
        }
        
        std::string json(static_cast<const char*>(p), uncomp_size);
        mz_free(p);
        mz_zip_reader_end(&zip_archive);
        
        // 1. BPM
        float bpm = JSON::parseFloat(json, "bpm", 140.0f);
        ::timeline.setBPM(bpm);

        // 2. Track volumes
        std::vector<float> volumes = JSON::parseFloatArray(json, "track_volumes");
        for (size_t i = 0; i < volumes.size() && i < MAX_TRACKS; i++) {
            ::track_volumes[i] = volumes[i];
        }

        // 3. Track mutes
        size_t mutes_pos = json.find("\"track_mutes\"");
        if (mutes_pos != std::string::npos) {
            size_t start = json.find("[", mutes_pos);
            size_t end = json.find("]", start);
            if (start != std::string::npos && end != std::string::npos) {
                std::string sub = json.substr(start + 1, end - start - 1);
                std::replace(sub.begin(), sub.end(), ',', ' ');
                std::stringstream ss(sub);
                std::string word;
                int idx = 0;
                while (ss >> word && idx < MAX_TRACKS) {
                    ::track_mutes[idx++] = (word == "true");
                }
            }
        }

        // 4. Track solos
        size_t solos_pos = json.find("\"track_solos\"");
        if (solos_pos != std::string::npos) {
            size_t start = json.find("[", solos_pos);
            size_t end = json.find("]", start);
            if (start != std::string::npos && end != std::string::npos) {
                std::string sub = json.substr(start + 1, end - start - 1);
                std::replace(sub.begin(), sub.end(), ',', ' ');
                std::stringstream ss(sub);
                std::string word;
                int idx = 0;
                while (ss >> word && idx < MAX_TRACKS) {
                    ::track_solos[idx++] = (word == "true");
                }
            }
        }

        // 5. Track FX chains
        std::string fx_chains_obj = JSON::getSubObject(json, "track_fx_chains");
        if (!fx_chains_obj.empty()) {
            size_t start = fx_chains_obj.find("[");
            size_t end = fx_chains_obj.rfind("]");
            if (start != std::string::npos && end != std::string::npos && start < end) {
                std::string inner = fx_chains_obj.substr(start + 1, end - start - 1);
                int idx = 0;
                size_t sub_start = 0;
                while (idx < MAX_TRACKS) {
                    size_t sub_open = inner.find("[", sub_start);
                    if (sub_open == std::string::npos) break;
                    size_t sub_close = inner.find("]", sub_open);
                    if (sub_close == std::string::npos) break;
                    
                    std::string chain_str = inner.substr(sub_open + 1, sub_close - sub_open - 1);
                    std::replace(chain_str.begin(), chain_str.end(), ',', ' ');
                    std::stringstream ss(chain_str);
                    KuroUI::track_fx_chain[idx].clear();
                    int fx_id;
                    while (ss >> fx_id) {
                        KuroUI::track_fx_chain[idx].push_back(fx_id);
                    }
                    idx++;
                    sub_start = sub_close + 1;
                }
            }
        }

        // 6. MIDI Notes per track
        std::string notes_obj = JSON::getSubObject(json, "track_midi_notes");
        if (!notes_obj.empty()) {
            size_t start = notes_obj.find("[");
            size_t end = notes_obj.rfind("]");
            if (start != std::string::npos && end != std::string::npos && start < end) {
                std::string inner = notes_obj.substr(start + 1, end - start - 1);
                int idx = 0;
                size_t sub_start = 0;
                while (idx < MAX_TRACKS) {
                    size_t sub_open = inner.find("[", sub_start);
                    if (sub_open == std::string::npos) break;
                    
                    int bracket_count = 1;
                    size_t sub_close = sub_open + 1;
                    for (; sub_close < inner.size(); sub_close++) {
                        if (inner[sub_close] == '[') bracket_count++;
                        else if (inner[sub_close] == ']') {
                            bracket_count--;
                            if (bracket_count == 0) break;
                        }
                    }
                    if (bracket_count != 0) break;
                    
                    std::string list_str = inner.substr(sub_open, sub_close - sub_open + 1);
                    std::vector<std::string> objs = JSON::splitArrayObjects(list_str);
                    
                    std::lock_guard<std::mutex> lock(::timeline.timeline_mutex);
                    ::timeline.track_notes[idx].clear();
                    for (const auto& obj : objs) {
                        int pitch = JSON::parseInt(obj, "pitch");
                        float start_time = JSON::parseFloat(obj, "start_time");
                        float duration = JSON::parseFloat(obj, "duration");
                        float velocity = JSON::parseFloat(obj, "velocity");
                        ::timeline.track_notes[idx].push_back(KuroDSP::MidiNote(pitch, start_time, duration, velocity));
                    }
                    idx++;
                    sub_start = sub_close + 1;
                }
            }
        }

        // 7. KuroWave settings
        std::string kurowave_obj = JSON::getSubObject(json, "kurowave");
        if (!kurowave_obj.empty()) {
            ::g_kurowave.param_wt_position = JSON::parseFloat(kurowave_obj, "wt_position", 2.0f);
            ::g_kurowave.param_unison_voices = JSON::parseInt(kurowave_obj, "unison_voices", 7);
            ::g_kurowave.param_unison_detune = JSON::parseFloat(kurowave_obj, "unison_detune", 0.15f);
            ::g_kurowave.param_cutoff = JSON::parseFloat(kurowave_obj, "cutoff", 2000.0f);
            ::g_kurowave.param_resonance = JSON::parseFloat(kurowave_obj, "resonance", 1.5f);
            ::g_kurowave.param_lfo_wt_mod = JSON::parseFloat(kurowave_obj, "lfo_wt_mod", 0.5f);
        }

        // 8. Piano synth settings
        std::string piano_obj = JSON::getSubObject(json, "piano_synth");
        if (!piano_obj.empty()) {
            int inst = JSON::parseInt(piano_obj, "instrument", 0);
            ::g_piano_synth.setInstrument((KuroAudio::MidiInstrument)inst);
        }

        std::cout << "[ProjectManager] Projeto Portátil (JSON) carregado: " << filepath << "\n";
    }
};
