#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <mutex>
#include <algorithm>
#include <filesystem>
#include "miniz.h"
#include "../plugin_manager/NativePlugins.h"
#include "../plugin_manager/DAG.h"
#include "../audio/SynthEngine.h"
#include "../audio/KuroWave.h"
#include "../core/TimelineManager.h"
#include "ClipManager.h"
#include "../utils/JSONHelper.h"

extern float track_volumes[MAX_TRACKS];
extern bool track_mutes[MAX_TRACKS];
extern bool track_solos[MAX_TRACKS];
extern KuroDSP::TimelineManager timeline;
extern KuroAudio::KuroWave g_kurowave;
extern KuroAudio::SynthEngine g_piano_synth;
extern ClipManager g_clip_manager;

namespace KuroUI {
    // track_fx_chain is static in StudioUI.h
}

class ProjectManager {
public:
    using JSON = KuroUI::JSON;

    static void SaveProject(const std::string& filepath) {
        std::stringstream ss;
        ss << "{\n";
        ss << "  " << JSON::format("bpm", ::timeline.getBPM()) << ",\n";
        ss << "  " << JSON::format("swing", ::timeline.getSwing()) << ",\n";
        
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

        // Channel Samples and Sampler Settings
        ss << "  \"channel_samples\": [\n";
        for (int c = 0; c < MAX_TRACKS; c++) {
            auto& ds = ::g_piano_synth.getDrumSample(c);
            auto& ss_ch = ::g_piano_synth.sampler_settings[c];
            std::string clean_path = ds.loaded ? ds.filepath : "";
            std::replace(clean_path.begin(), clean_path.end(), '\\', '/');

            ss << "    {\n";
            ss << "      " << JSON::format("channel", c) << ",\n";
            ss << "      " << JSON::format("filepath", clean_path) << ",\n";
            ss << "      " << JSON::format("root_note", ss_ch.root_note) << ",\n";
            ss << "      " << JSON::format("pitch", ss_ch.pitch) << ",\n";
            ss << "      " << JSON::format("vol", ss_ch.vol) << ",\n";
            ss << "      " << JSON::format("pan", ss_ch.pan) << "\n";
            ss << "    }" << (c < MAX_TRACKS - 1 ? ",\n" : "\n");
        }
        ss << "  ],\n";

        // Global Patterns & Channel Notes
        {
            std::lock_guard<std::mutex> lock(::g_clip_manager.clip_mutex);
            ss << "  " << JSON::format("current_pattern_idx", ::g_clip_manager.current_pattern_idx) << ",\n";
            ss << "  \"patterns\": [\n";
            for (size_t p = 0; p < ::g_clip_manager.global_patterns.size(); p++) {
                const auto& pat = ::g_clip_manager.global_patterns[p];
                ss << "    {\n";
                ss << "      " << JSON::format("id", pat.id) << ",\n";
                ss << "      " << JSON::format("name", pat.name) << ",\n";
                ss << "      " << JSON::format("color", (int)pat.color) << ",\n";
                ss << "      " << JSON::format("length", pat.default_length_sec) << ",\n";
                ss << "      \"channel_notes\": [\n";
                for (int c = 0; c < MAX_TRACKS; c++) {
                    const auto& cnotes = pat.channel_notes[c];
                    ss << "        [";
                    for (size_t n = 0; n < cnotes.size(); n++) {
                        const auto& note = cnotes[n];
                        ss << "{"
                           << JSON::format("p", note.pitch) << ","
                           << JSON::format("st", note.start_time) << ","
                           << JSON::format("d", note.duration) << ","
                           << JSON::format("v", note.velocity) << ","
                           << JSON::format("pan", note.pan) << ","
                           << JSON::format("poff", note.pitch_offset)
                           << "}" << (n < cnotes.size() - 1 ? ", " : "");
                    }
                    ss << "]" << (c < MAX_TRACKS - 1 ? ",\n" : "\n");
                }
                ss << "      ]\n";
                ss << "    }" << (p < ::g_clip_manager.global_patterns.size() - 1 ? ",\n" : "\n");
            }
            ss << "  ],\n";

            // Playlist MIDI clips
            ss << "  \"playlist_midi_clips\": [\n";
            for (int t = 0; t < MAX_TRACKS; t++) {
                const auto& clips = ::g_clip_manager.track_midi_clips[t];
                ss << "    [";
                for (size_t k = 0; k < clips.size(); k++) {
                    const auto& c = clips[k];
                    ss << "{"
                       << JSON::format("id", c.id) << ","
                       << JSON::format("st", c.start_time_sec) << ","
                       << JSON::format("len", c.length_sec) << ","
                       << JSON::format("pat_id", c.pattern_id) << ","
                       << JSON::format("name", c.name) << ","
                       << JSON::format("muted", c.is_muted)
                       << "}" << (k < clips.size() - 1 ? ", " : "");
                }
                ss << "]" << (t < MAX_TRACKS - 1 ? ",\n" : "\n");
            }
            ss << "  ],\n";

            // Playlist Audio clips
            ss << "  \"playlist_audio_clips\": [\n";
            for (int t = 0; t < MAX_TRACKS; t++) {
                const auto& aclips = ::g_clip_manager.track_clips[t];
                ss << "    [";
                for (size_t k = 0; k < aclips.size(); k++) {
                    const auto& c = aclips[k];
                    std::string clean_cpath = c.file_path;
                    std::replace(clean_cpath.begin(), clean_cpath.end(), '\\', '/');

                    ss << "{"
                       << JSON::format("id", c.id) << ","
                       << JSON::format("st", c.start_time_sec) << ","
                       << JSON::format("len", c.length_sec) << ","
                       << JSON::format("offset", c.source_offset_sec) << ","
                       << JSON::format("name", c.name) << ","
                       << JSON::format("path", clean_cpath) << ","
                       << JSON::format("pitch", c.pitch_shift_semitones) << ","
                       << JSON::format("stretch", c.time_stretch_ratio) << ","
                       << JSON::format("muted", c.is_muted)
                       << "}" << (k < aclips.size() - 1 ? ", " : "");
                }
                ss << "]" << (t < MAX_TRACKS - 1 ? ",\n" : "\n");
            }
            ss << "  ],\n";

            // Playlist Automation clips
            ss << "  \"playlist_auto_clips\": [\n";
            for (int t = 0; t < MAX_TRACKS; t++) {
                const auto& aclips = ::g_clip_manager.track_auto_clips[t];
                ss << "    [";
                for (size_t k = 0; k < aclips.size(); k++) {
                    const auto& c = aclips[k];
                    ss << "{"
                       << JSON::format("id", c.id) << ","
                       << JSON::format("st", c.start_time_sec) << ","
                       << JSON::format("len", c.length_sec) << ","
                       << JSON::format("name", c.name) << ","
                       << JSON::format("color", (int)c.color) << ","
                       << JSON::format("target", c.target_node_id) << ","
                       << JSON::format("param", c.param_index) << ","
                       << "\"points\": [";
                    for (size_t pt_i = 0; pt_i < c.points.size(); pt_i++) {
                        const auto& pt = c.points[pt_i];
                        ss << "{"
                           << JSON::format("t", pt.time_rel_sec) << ","
                           << JSON::format("v", pt.value) << ","
                           << JSON::format("tension", pt.tension)
                           << "}" << (pt_i < c.points.size() - 1 ? "," : "");
                    }
                    ss << "]}";
                    if (k < aclips.size() - 1) ss << ", ";
                }
                ss << "]" << (t < MAX_TRACKS - 1 ? ",\n" : "\n");
            }
            ss << "  ],\n";
        }

        // Timeline Section Markers
        ss << "  \"timeline_markers\": [\n";
        for (size_t m = 0; m < ::timeline.section_markers.size(); m++) {
            const auto& mk = ::timeline.section_markers[m];
            ss << "    {"
               << JSON::format("t", mk.time_sec) << ","
               << JSON::format("name", mk.name) << ","
               << JSON::format("color", (int)mk.color)
               << "}" << (m < ::timeline.section_markers.size() - 1 ? ",\n" : "\n");
        }
        ss << "  ],\n";

        // Backwards compatibility legacy field
        ss << "  \"track_midi_notes\": [],\n";

        // KuroWave synth settings
        ss << "  \"kurowave\": {\n";
        ss << "    " << JSON::format("wt_position", ::g_kurowave.param_wt_position) << ",\n";
        ss << "    " << JSON::format("unison_voices", ::g_kurowave.param_unison_voices) << ",\n";
        ss << "    " << JSON::format("unison_detune", ::g_kurowave.param_unison_detune) << ",\n";
        ss << "    " << JSON::format("cutoff", ::g_kurowave.param_cutoff) << ",\n";
        ss << "    " << JSON::format("resonance", ::g_kurowave.param_resonance) << ",\n";
        ss << "    " << JSON::format("lfo_wt_mod", ::g_kurowave.param_lfo_wt_mod) << "\n";
        ss << "  },\n";

        // Piano & Synth settings
        ss << "  \"piano_synth\": {\n";
        ss << "    " << JSON::format("instrument", (int)::g_piano_synth.current_instrument) << ",\n";
        ss << "    \"flex_instruments\": [";
        for (int i = 0; i < MAX_TRACKS; i++) {
            ss << (int)::g_piano_synth.flex_channel_instrument[i] << (i < MAX_TRACKS - 1 ? ", " : "");
        }
        ss << "],\n";
        ss << "    \"flex_active\": [";
        for (int i = 0; i < MAX_TRACKS; i++) {
            ss << (::g_piano_synth.flex_active[i] ? "true" : "false") << (i < MAX_TRACKS - 1 ? ", " : "");
        }
        ss << "]\n";
        ss << "  }\n";

        ss << "}\n";

        std::string configData = ss.str();
        std::ofstream out(filepath);
        if (out.is_open()) {
            out << configData;
            out.close();
            std::cout << "[ProjectManager] Projeto salvo com sucesso: " << filepath << "\n";
        } else {
            std::cerr << "[ProjectManager] Erro ao salvar o projeto em: " << filepath << "\n";
        }
    }

    static void LoadProject(const std::string& filepath) {
        std::ifstream in(filepath);
        if (!in.is_open()) {
            std::cerr << "[ProjectManager] Arquivo .kuro nao encontrado ou invalido: " << filepath << "\n";
            return;
        }
        std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();
        
        // 1. BPM & Swing
        float bpm = JSON::parseFloat(json, "bpm", 140.0f);
        ::timeline.setBPM(bpm);
        float swing = JSON::parseFloat(json, "swing", 0.0f);
        ::timeline.setSwing(swing);

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

        // 5. Channel Samples and Sampler Settings
        std::string samples_arr = JSON::getSubObject(json, "channel_samples");
        if (!samples_arr.empty()) {
            std::vector<std::string> sample_objs = JSON::splitArrayObjects(samples_arr);
            for (const auto& sobj : sample_objs) {
                int ch = JSON::parseInt(sobj, "channel", -1);
                if (ch >= 0 && ch < MAX_TRACKS) {
                    std::string path = JSON::parseString(sobj, "filepath");
                    if (!path.empty()) {
                        std::error_code ec;
                        if (std::filesystem::exists(path, ec)) {
                            ::g_piano_synth.getDrumSample(ch).load(path);
                        }
                    }
                    if (sobj.find("\"root_note\"") != std::string::npos) {
                        ::g_piano_synth.sampler_settings[ch].root_note = JSON::parseInt(sobj, "root_note", 60);
                    }
                    if (sobj.find("\"pitch\"") != std::string::npos) {
                        ::g_piano_synth.sampler_settings[ch].pitch = JSON::parseFloat(sobj, "pitch", 0.0f);
                    }
                    if (sobj.find("\"vol\"") != std::string::npos) {
                        ::g_piano_synth.sampler_settings[ch].vol = JSON::parseFloat(sobj, "vol", 0.8f);
                    }
                    if (sobj.find("\"pan\"") != std::string::npos) {
                        ::g_piano_synth.sampler_settings[ch].pan = JSON::parseFloat(sobj, "pan", 0.0f);
                    }
                }
            }
        }

        // 6. Global Patterns & Channel Notes
        std::string patterns_arr = JSON::getSubObject(json, "patterns");
        if (!patterns_arr.empty()) {
            std::vector<std::string> pat_objs = JSON::splitArrayObjects(patterns_arr);
            if (!pat_objs.empty()) {
                std::lock_guard<std::mutex> lock(::g_clip_manager.clip_mutex);
                ::g_clip_manager.global_patterns.clear();
                for (const auto& pobj : pat_objs) {
                    Pattern pat;
                    pat.id = JSON::parseInt(pobj, "id", (int)::g_clip_manager.global_patterns.size() + 1);
                    pat.name = JSON::parseString(pobj, "name");
                    if (pat.name.empty()) pat.name = "Pattern " + std::to_string(pat.id);
                    pat.color = (unsigned int)JSON::parseInt(pobj, "color", 0xFF00FF7F);
                    pat.default_length_sec = JSON::parseFloat(pobj, "length", 4.0f);

                    std::string cn_str = JSON::getSubObject(pobj, "channel_notes");
                    if (!cn_str.empty()) {
                        size_t open_br = cn_str.find('[');
                        size_t cursor = (open_br != std::string::npos) ? open_br + 1 : 0;
                        for (int ch = 0; ch < MAX_TRACKS; ch++) {
                            size_t start = cn_str.find('[', cursor);
                            if (start == std::string::npos) break;
                            size_t end = cn_str.find(']', start);
                            if (end == std::string::npos) break;
                            std::string list_str = cn_str.substr(start, end - start + 1);
                            std::vector<std::string> note_objs = JSON::splitArrayObjects(list_str);
                            for (const auto& nobj : note_objs) {
                                int pitch = JSON::parseInt(nobj, "p", 60);
                                float st = JSON::parseFloat(nobj, "st", 0.0f);
                                float dur = JSON::parseFloat(nobj, "d", 0.25f);
                                float vel = JSON::parseFloat(nobj, "v", 0.8f);
                                float pan = JSON::parseFloat(nobj, "pan", 0.5f);
                                float poff = JSON::parseFloat(nobj, "poff", 0.0f);
                                pat.channel_notes[ch].emplace_back(pitch, st, dur, vel, 1.0f, ch, pan, poff);
                            }
                            cursor = end + 1;
                        }
                    }
                    ::g_clip_manager.global_patterns.push_back(pat);
                }
                int cur_pat = JSON::parseInt(json, "current_pattern_idx", 0);
                if (cur_pat >= 0 && cur_pat < (int)::g_clip_manager.global_patterns.size()) {
                    ::g_clip_manager.current_pattern_idx = cur_pat;
                } else {
                    ::g_clip_manager.current_pattern_idx = 0;
                }
            }
        }

        // 7. Playlist MIDI Clips
        std::string pmc_str = JSON::getSubObject(json, "playlist_midi_clips");
        if (!pmc_str.empty()) {
            size_t open_br = pmc_str.find('[');
            size_t cursor = (open_br != std::string::npos) ? open_br + 1 : 0;
            for (int t = 0; t < MAX_TRACKS; t++) {
                size_t start = pmc_str.find('[', cursor);
                if (start == std::string::npos) break;
                size_t end = pmc_str.find(']', start);
                if (end == std::string::npos) break;
                std::string list_str = pmc_str.substr(start, end - start + 1);
                std::vector<std::string> clip_objs = JSON::splitArrayObjects(list_str);
                ::g_clip_manager.track_midi_clips[t].clear();
                for (const auto& cobj : clip_objs) {
                    MidiClip mc;
                    mc.id = JSON::parseInt(cobj, "id", 1);
                    mc.start_time_sec = JSON::parseFloat(cobj, "st", 0.0f);
                    mc.length_sec = JSON::parseFloat(cobj, "len", 4.0f);
                    mc.pattern_id = JSON::parseInt(cobj, "pat_id", 1);
                    mc.name = JSON::parseString(cobj, "name");
                    mc.is_muted = JSON::parseBool(cobj, "muted", false);
                    ::g_clip_manager.track_midi_clips[t].push_back(mc);
                }
                cursor = end + 1;
            }
        }

        // 8. Playlist Audio Clips
        std::string pac_str = JSON::getSubObject(json, "playlist_audio_clips");
        if (!pac_str.empty()) {
            size_t open_br = pac_str.find('[');
            size_t cursor = (open_br != std::string::npos) ? open_br + 1 : 0;
            for (int t = 0; t < MAX_TRACKS; t++) {
                size_t start = pac_str.find('[', cursor);
                if (start == std::string::npos) break;
                size_t end = pac_str.find(']', start);
                if (end == std::string::npos) break;
                std::string list_str = pac_str.substr(start, end - start + 1);
                std::vector<std::string> clip_objs = JSON::splitArrayObjects(list_str);
                ::g_clip_manager.track_clips[t].clear();
                for (const auto& cobj : clip_objs) {
                    AudioClip ac;
                    ac.id = JSON::parseInt(cobj, "id", 1);
                    ac.start_time_sec = JSON::parseFloat(cobj, "st", 0.0f);
                    ac.length_sec = JSON::parseFloat(cobj, "len", 4.0f);
                    ac.source_offset_sec = JSON::parseFloat(cobj, "offset", 0.0f);
                    ac.name = JSON::parseString(cobj, "name");
                    ac.file_path = JSON::parseString(cobj, "path");
                    ac.pitch_shift_semitones = JSON::parseFloat(cobj, "pitch", 0.0f);
                    ac.time_stretch_ratio = JSON::parseFloat(cobj, "stretch", 1.0f);
                    ac.is_muted = JSON::parseBool(cobj, "muted", false);
                    ::g_clip_manager.track_clips[t].push_back(ac);
                }
                cursor = end + 1;
            }
        }

        // 9. Playlist Automation Clips
        std::string auto_str = JSON::getSubObject(json, "playlist_auto_clips");
        if (!auto_str.empty()) {
            size_t open_br = auto_str.find('[');
            size_t cursor = (open_br != std::string::npos) ? open_br + 1 : 0;
            for (int t = 0; t < MAX_TRACKS; t++) {
                size_t start = auto_str.find('[', cursor);
                if (start == std::string::npos) break;
                size_t end = auto_str.find(']', start);
                if (end == std::string::npos) break;
                std::string list_str = auto_str.substr(start, end - start + 1);
                std::vector<std::string> clip_objs = JSON::splitArrayObjects(list_str);
                ::g_clip_manager.track_auto_clips[t].clear();
                for (const auto& cobj : clip_objs) {
                    AutomationClip ac;
                    ac.id = JSON::parseInt(cobj, "id", 1);
                    ac.start_time_sec = JSON::parseFloat(cobj, "st", 0.0f);
                    ac.length_sec = JSON::parseFloat(cobj, "len", 16.0f);
                    ac.name = JSON::parseString(cobj, "name");
                    ac.color = (unsigned int)JSON::parseInt(cobj, "color", 0xFF00E5FF);
                    ac.target_node_id = JSON::parseString(cobj, "target");
                    ac.param_index = JSON::parseInt(cobj, "param", 0);

                    std::string pts_str = JSON::getSubObject(cobj, "points");
                    if (!pts_str.empty()) {
                        ac.points.clear();
                        std::vector<std::string> pt_objs = JSON::splitArrayObjects(pts_str);
                        for (const auto& pto : pt_objs) {
                            float pt_t = JSON::parseFloat(pto, "t", 0.0f);
                            float pt_v = JSON::parseFloat(pto, "v", 0.5f);
                            float pt_ten = JSON::parseFloat(pto, "tension", 0.0f);
                            ac.points.push_back({pt_t, pt_v, pt_ten});
                        }
                    }
                    ::g_clip_manager.track_auto_clips[t].push_back(ac);
                }
                cursor = end + 1;
            }
        }

        // 10. Timeline Section Markers
        std::string tm_str = JSON::getSubObject(json, "timeline_markers");
        if (!tm_str.empty()) {
            std::vector<std::string> mobjs = JSON::splitArrayObjects(tm_str);
            if (!mobjs.empty()) {
                ::timeline.clearSectionMarkers();
                for (const auto& mo : mobjs) {
                    float t_sec = JSON::parseFloat(mo, "t", 0.0f);
                    std::string name = JSON::parseString(mo, "name");
                    uint32_t col = (uint32_t)JSON::parseInt(mo, "color", 0xFF00E5FF);
                    ::timeline.addSectionMarker(t_sec, name, col);
                }
            }
        }

        // 11. KuroWave settings
        std::string kurowave_obj = JSON::getSubObject(json, "kurowave");
        if (!kurowave_obj.empty()) {
            ::g_kurowave.param_wt_position = JSON::parseFloat(kurowave_obj, "wt_position", 2.0f);
            ::g_kurowave.param_unison_voices = JSON::parseInt(kurowave_obj, "unison_voices", 7);
            ::g_kurowave.param_unison_detune = JSON::parseFloat(kurowave_obj, "unison_detune", 0.15f);
            ::g_kurowave.param_cutoff = JSON::parseFloat(kurowave_obj, "cutoff", 2000.0f);
            ::g_kurowave.param_resonance = JSON::parseFloat(kurowave_obj, "resonance", 1.5f);
            ::g_kurowave.param_lfo_wt_mod = JSON::parseFloat(kurowave_obj, "lfo_wt_mod", 0.5f);
        }

        // 10. Piano synth settings
        std::string piano_obj = JSON::getSubObject(json, "piano_synth");
        if (!piano_obj.empty()) {
            int inst = JSON::parseInt(piano_obj, "instrument", 0);
            ::g_piano_synth.setInstrument((KuroAudio::MidiInstrument)inst);

            std::vector<int> flex_insts = JSON::parseIntArray(piano_obj, "flex_instruments");
            for (size_t i = 0; i < flex_insts.size() && i < MAX_TRACKS; i++) {
                ::g_piano_synth.flex_channel_instrument[i] = (KuroAudio::MidiInstrument)flex_insts[i];
            }

            size_t fa_pos = piano_obj.find("\"flex_active\"");
            if (fa_pos != std::string::npos) {
                size_t start = piano_obj.find("[", fa_pos);
                size_t end = piano_obj.find("]", start);
                if (start != std::string::npos && end != std::string::npos) {
                    std::string sub = piano_obj.substr(start + 1, end - start - 1);
                    std::replace(sub.begin(), sub.end(), ',', ' ');
                    std::stringstream ss(sub);
                    std::string word;
                    int idx = 0;
                    while (ss >> word && idx < MAX_TRACKS) {
                        ::g_piano_synth.flex_active[idx++] = (word == "true");
                    }
                }
            }
        }

        std::cout << "[ProjectManager] Projeto Portátil (JSON) carregado: " << filepath << "\n";
    }
};
