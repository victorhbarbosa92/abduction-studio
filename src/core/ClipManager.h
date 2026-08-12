#pragma once
#include <vector>
#include <mutex>
#include "MidiNote.h"
#include "KuroConfig.h"

struct AudioClip {
    int id;
    float start_time_sec;      // Posição no tempo do projeto
    float length_sec;          // Duração do clipe
    float source_offset_sec;   // De qual parte do áudio original esse clipe começa
    bool is_selected;
    float pitch_shift_semitones = 0.0f; // Shift de tom (-12 a +12 semitones)
    float time_stretch_ratio = 1.0f;    // Fator de tempo (0.5x a 2.0x)
    bool enable_stretch = false;
};

struct Pattern {
    int id;
    std::string name;
    unsigned int color;
    std::vector<KuroDSP::MidiNote> notes; // General / legacy notes
    std::vector<KuroDSP::MidiNote> channel_notes[8]; // Per-channel notes (0..7)
    float default_length_sec = 4.0f;

    std::vector<KuroDSP::MidiNote>& getChannelNotes(int channel_idx) {
        int idx = std::clamp(channel_idx, 0, 7);
        return channel_notes[idx];
    }

    const std::vector<KuroDSP::MidiNote>& getChannelNotes(int channel_idx) const {
        int idx = std::clamp(channel_idx, 0, 7);
        return channel_notes[idx];
    }
};

struct MidiClip {
    int id;
    float start_time_sec;
    float length_sec;
    int pattern_id; // Refers to the Pattern.id
    bool is_selected;
};

class ClipManager {
public:
    std::vector<AudioClip> track_clips[MAX_TRACKS];
    std::vector<MidiClip> track_midi_clips[MAX_TRACKS];
    std::vector<Pattern> global_patterns;
    int current_pattern_idx = 0;
    std::mutex clip_mutex;
    int next_id = 1;

    ClipManager() {
        // Inicializa com 1 pattern base
        Pattern p;
        p.id = next_id++;
        p.name = "Pattern 1";
        p.color = 0xFF00E5FF;
        p.notes.clear();
        // Preset de Bassline Psytrance 16th Note Offbeat (K - B - B - B)
        for (int b = 0; b < 16; ++b) {
            if (b % 4 == 0) continue; // Folga do Kick no primeiro tempo de cada tempo
            KuroDSP::MidiNote n;
            n.pitch = (b >= 12) ? 38 : 36; // C2 (36) com transição para D2 (38) na virada
            n.start_time = b * 0.125f;     // Sincronismo de 16th notes
            n.duration = 0.10f;
            n.velocity = 0.90f;
            p.notes.push_back(n);
        }
        global_patterns.push_back(p);
        // Por padrão, a Playlist da DAW inicia 100% LIMPA e VAZIA como no FL Studio!
        // Apenas Pattern 1 fica disponível no Channel Rack e no Picker Panel à esquerda.
    }

    Pattern& getCurrentPattern() {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (global_patterns.empty()) {
            Pattern p;
            p.id = next_id++;
            p.name = "Pattern 1";
            p.color = 0xFF5050AA;
            global_patterns.push_back(p);
        }
        int idx = std::clamp(current_pattern_idx, 0, (int)global_patterns.size() - 1);
        current_pattern_idx = idx;
        return global_patterns[idx];
    }

    void initTrack(int track_index, float total_length_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        
        float start_t = 0.0f;
        if (!track_clips[track_index].empty()) {
            start_t = track_clips[track_index].back().start_time_sec + track_clips[track_index].back().length_sec;
        }
        
        AudioClip clip;
        clip.id = next_id++;
        clip.start_time_sec = start_t;
        clip.length_sec = total_length_sec;
        clip.source_offset_sec = 0.0f;
        clip.is_selected = false;
        track_clips[track_index].push_back(clip);
    }

    void reset() {
        std::lock_guard<std::mutex> lock(clip_mutex);
        for(int i=0; i<MAX_TRACKS; i++) {
            track_clips[i].clear();
            track_midi_clips[i].clear();
        }
        next_id = 1;
    }
    
    void splitClip(int track_index, int clip_index, float split_time_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= track_clips[track_index].size()) return;
        
        AudioClip& c = track_clips[track_index][clip_index];
        if (split_time_sec <= c.start_time_sec || split_time_sec >= c.start_time_sec + c.length_sec) return;
        
        float relative_split = split_time_sec - c.start_time_sec;
        
        AudioClip new_clip;
        new_clip.id = next_id++;
        new_clip.start_time_sec = split_time_sec;
        new_clip.length_sec = c.length_sec - relative_split;
        new_clip.source_offset_sec = c.source_offset_sec + relative_split;
        new_clip.is_selected = false;
        
        c.length_sec = relative_split;
        
        track_clips[track_index].insert(track_clips[track_index].begin() + clip_index + 1, new_clip);
    }
    
    void updateClipStart(int track_index, int clip_index, float new_start_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= track_clips[track_index].size()) return;
        track_clips[track_index][clip_index].start_time_sec = new_start_sec;
    }

    void deleteClip(int track_index, int clip_index) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= track_clips[track_index].size()) return;
        track_clips[track_index].erase(track_clips[track_index].begin() + clip_index);
    }

    std::vector<AudioClip> getClips(int track_index) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return std::vector<AudioClip>();
        return track_clips[track_index];
    }
    
    void addPatternClip(int track_index, float start_time_sec, float length_sec, int pattern_id) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        MidiClip clip;
        clip.id = next_id++;
        clip.start_time_sec = start_time_sec;
        clip.length_sec = length_sec;
        clip.pattern_id = pattern_id;
        clip.is_selected = false;
        track_midi_clips[track_index].push_back(clip);
    }
    
    std::vector<MidiClip> getMidiClips(int track_index) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return std::vector<MidiClip>();
        return track_midi_clips[track_index];
    }

    void removeMidiClipAt(int track_index, float time_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        auto& clips = track_midi_clips[track_index];
        for (auto it = clips.begin(); it != clips.end(); ++it) {
            if (time_sec >= it->start_time_sec && time_sec <= it->start_time_sec + it->length_sec) {
                clips.erase(it);
                break;
            }
        }
    }

    void loadTrackTemplate(int template_id, float& bpm_out) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        global_patterns.clear();
        for (int t = 0; t < MAX_TRACKS; ++t) {
            track_clips[t].clear();
            track_midi_clips[t].clear();
        }

        if (template_id == 0) { // 👽 Full Track 01: "Alien Abduction" - Psytrance Trance (142 BPM, F# Phrygian, 3:45m)
            bpm_out = 142.0f;
            
            // Pattern 1: Atmospheric Intro Pad
            Pattern p_intro; p_intro.id = 1; p_intro.name = "01. Intro Atmosphere"; p_intro.color = 0xFF00E5FF;
            p_intro.getChannelNotes(4).push_back(KuroDSP::MidiNote(42, 0.0f, 4.0f, 0.70f, 1.0f, 4));
            p_intro.getChannelNotes(4).push_back(KuroDSP::MidiNote(45, 0.0f, 4.0f, 0.70f, 1.0f, 4));
            p_intro.getChannelNotes(4).push_back(KuroDSP::MidiNote(49, 0.0f, 4.0f, 0.70f, 1.0f, 4));
            global_patterns.push_back(p_intro);

            // Pattern 2: Kick & Build-Up Snare Roll
            Pattern p_build; p_build.id = 2; p_build.name = "02. Build-Up Snare Roll"; p_build.color = 0xFFFF9900;
            for (int b = 0; b < 16; b += 4) p_build.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, b * 0.125f, 0.10f, 1.0f, 1.0f, 0));
            for (int b = 0; b < 16; b += 2) p_build.getChannelNotes(1).push_back(KuroDSP::MidiNote(38, b * 0.125f, 0.08f, 0.85f, 1.0f, 1));
            global_patterns.push_back(p_build);

            // Pattern 3: 16th Psy Rolling Bassline (Drop 1 Energy)
            Pattern p_bass; p_bass.id = 3; p_bass.name = "03. 16th Psy Rolling Bass"; p_bass.color = 0xFF00E5FF;
            for (int b = 0; b < 16; b += 4) p_bass.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, b * 0.125f, 0.10f, 1.0f, 1.0f, 0));
            for (int b = 0; b < 16; ++b) {
                if (b % 4 == 0) continue;
                p_bass.getChannelNotes(3).push_back(KuroDSP::MidiNote(30, b * 0.125f, 0.08f, 0.90f, 1.0f, 3));
            }
            for (int b = 2; b < 16; b += 4) p_bass.getChannelNotes(2).push_back(KuroDSP::MidiNote(42, b * 0.125f, 0.05f, 0.80f, 1.0f, 2));
            global_patterns.push_back(p_bass);

            // Pattern 4: Acid Resonant Lead Melody
            Pattern p_lead; p_lead.id = 4; p_lead.name = "04. Acid Resonant Lead"; p_lead.color = 0xFFFF007F;
            int acid_pitches[16] = {54, 55, 58, 61, 58, 55, 54, 61, 54, 55, 58, 66, 61, 58, 55, 54};
            for (int i = 0; i < 16; ++i) p_lead.getChannelNotes(5).push_back(KuroDSP::MidiNote(acid_pitches[i], i * 0.125f, 0.10f, 0.88f, 1.0f, 5));
            global_patterns.push_back(p_lead);

            // Montar Arranjo Completo de Música de 3m45s na Playlist (Tracks 0 a 7)
            // 0:00 - 0:30 (Intro Audio & Atmosphere Pads)
            for (int b = 0; b < 2; ++b) {
                MidiClip mc; mc.id = next_id++; mc.start_time_sec = b * 16.0f; mc.length_sec = 16.0f; mc.pattern_id = 1; mc.is_selected = false;
                track_midi_clips[6].push_back(mc);
                AudioClip ac; ac.id = next_id++; ac.start_time_sec = b * 16.0f; ac.length_sec = 16.0f; ac.source_offset_sec = 0.0f; ac.is_selected = false;
                track_clips[7].push_back(ac);
            }
            // 0:30 - 1:00 (Build-Up Kick & Snare Drums)
            for (int b = 2; b < 4; ++b) {
                MidiClip mc; mc.id = next_id++; mc.start_time_sec = b * 16.0f; mc.length_sec = 16.0f; mc.pattern_id = 2; mc.is_selected = false;
                track_midi_clips[0].push_back(mc);
                AudioClip ac_kick; ac_kick.id = next_id++; ac_kick.start_time_sec = b * 16.0f; ac_kick.length_sec = 16.0f; ac_kick.source_offset_sec = 0.0f; ac_kick.is_selected = false;
                AudioClip ac_snare; ac_snare.id = next_id++; ac_snare.start_time_sec = b * 16.0f; ac_snare.length_sec = 16.0f; ac_snare.source_offset_sec = 0.0f; ac_snare.is_selected = false;
                track_clips[0].push_back(ac_kick); track_clips[1].push_back(ac_snare);
            }
            // 1:00 - 2:00 (Drop 1 Main Bassline & Drums)
            for (int b = 4; b < 8; ++b) {
                MidiClip mc; mc.id = next_id++; mc.start_time_sec = b * 16.0f; mc.length_sec = 16.0f; mc.pattern_id = 3; mc.is_selected = false;
                track_midi_clips[1].push_back(mc);
                AudioClip ac_kick; ac_kick.id = next_id++; ac_kick.start_time_sec = b * 16.0f; ac_kick.length_sec = 16.0f; ac_kick.source_offset_sec = 0.0f; ac_kick.is_selected = false;
                AudioClip ac_vox; ac_vox.id = next_id++; ac_vox.start_time_sec = b * 16.0f; ac_vox.length_sec = 16.0f; ac_vox.source_offset_sec = 0.0f; ac_vox.is_selected = false;
                track_clips[0].push_back(ac_kick); track_clips[3].push_back(ac_vox);
            }
            // 2:00 - 3:00 (Drop 2 Climax Lead & Bassline)
            for (int b = 8; b < 12; ++b) {
                MidiClip mc1; mc1.id = next_id++; mc1.start_time_sec = b * 16.0f; mc1.length_sec = 16.0f; mc1.pattern_id = 3; mc1.is_selected = false;
                MidiClip mc2; mc2.id = next_id++; mc2.start_time_sec = b * 16.0f; mc2.length_sec = 16.0f; mc2.pattern_id = 4; mc2.is_selected = false;
                track_midi_clips[1].push_back(mc1); track_midi_clips[2].push_back(mc2);
                AudioClip ac_kick; ac_kick.id = next_id++; ac_kick.start_time_sec = b * 16.0f; ac_kick.length_sec = 16.0f; ac_kick.source_offset_sec = 0.0f; ac_kick.is_selected = false;
                track_clips[0].push_back(ac_kick);
            }
            // 3:00 - 3:45 (Outro Fading Atmosphere)
            for (int b = 12; b < 14; ++b) {
                MidiClip mc; mc.id = next_id++; mc.start_time_sec = b * 16.0f; mc.length_sec = 16.0f; mc.pattern_id = 1; mc.is_selected = false;
                track_midi_clips[6].push_back(mc);
                AudioClip ac; ac.id = next_id++; ac.start_time_sec = b * 16.0f; ac.length_sec = 16.0f; ac.source_offset_sec = 0.0f; ac.is_selected = false;
                track_clips[7].push_back(ac);
            }
        }
        else if (template_id == 1) { // 🚀 Full Track 02: "Cyberpunk 2088" - Synthwave (118 BPM, D Minor, 3:30m)
            bpm_out = 118.0f;
            Pattern p1; p1.id = 1; p1.name = "01. Synthwave Intro Pad"; p1.color = 0xFFFF007F;
            p1.getChannelNotes(4).push_back(KuroDSP::MidiNote(38, 0.0f, 4.0f, 0.85f, 1.0f, 4));
            p1.getChannelNotes(4).push_back(KuroDSP::MidiNote(41, 0.0f, 4.0f, 0.85f, 1.0f, 4));
            p1.getChannelNotes(4).push_back(KuroDSP::MidiNote(45, 0.0f, 4.0f, 0.85f, 1.0f, 4));
            global_patterns.push_back(p1);

            Pattern p2; p2.id = 2; p2.name = "02. Darksynth Main Beat & Bass"; p2.color = 0xFF00E5FF;
            for (int b = 0; b < 16; b += 4) p2.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, b * 0.125f, 0.12f, 1.0f, 1.0f, 0));
            p2.getChannelNotes(1).push_back(KuroDSP::MidiNote(38, 0.5f, 0.15f, 0.95f, 1.0f, 1));
            p2.getChannelNotes(1).push_back(KuroDSP::MidiNote(38, 1.5f, 0.15f, 0.95f, 1.0f, 1));
            int sw_pitches[8] = {38, 50, 38, 50, 34, 46, 36, 48};
            for (int i = 0; i < 8; ++i) p2.getChannelNotes(3).push_back(KuroDSP::MidiNote(sw_pitches[i], i * 0.25f, 0.20f, 0.90f, 1.0f, 3));
            global_patterns.push_back(p2);

            for (int b = 0; b < 12; ++b) {
                MidiClip mc; mc.id = next_id++; mc.start_time_sec = b * 16.0f; mc.length_sec = 16.0f; mc.pattern_id = (b < 3) ? 1 : 2; mc.is_selected = false;
                track_midi_clips[1].push_back(mc);
                AudioClip ac; ac.id = next_id++; ac.start_time_sec = b * 16.0f; ac.length_sec = 16.0f; ac.source_offset_sec = 0.0f; ac.is_selected = false;
                track_clips[0].push_back(ac);
            }
        }
        else if (template_id == 2) { // 🎹 Full Track 03: "Afterlife Journey" - Melodic Techno (124 BPM, C Minor, 3:40m)
            bpm_out = 124.0f;
            Pattern p1; p1.id = 1; p1.name = "01. Melodic Techno Beat & Pluck"; p1.color = 0xFF00E5FF;
            for (int b = 0; b < 16; b += 4) p1.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, b * 0.125f, 0.12f, 1.0f, 1.0f, 0));
            for (int b = 2; b < 16; b += 4) p1.getChannelNotes(2).push_back(KuroDSP::MidiNote(42, b * 0.125f, 0.05f, 0.80f, 1.0f, 2));
            p1.getChannelNotes(4).push_back(KuroDSP::MidiNote(48, 0.0f, 0.8f, 0.85f, 1.0f, 4));
            p1.getChannelNotes(4).push_back(KuroDSP::MidiNote(51, 0.0f, 0.8f, 0.85f, 1.0f, 4));
            p1.getChannelNotes(4).push_back(KuroDSP::MidiNote(55, 0.0f, 0.8f, 0.85f, 1.0f, 4));
            global_patterns.push_back(p1);

            for (int b = 0; b < 12; ++b) {
                MidiClip mc; mc.id = next_id++; mc.start_time_sec = b * 16.0f; mc.length_sec = 16.0f; mc.pattern_id = 1; mc.is_selected = false;
                track_midi_clips[0].push_back(mc); track_midi_clips[2].push_back(mc);
            }
        }
        else if (template_id == 3) { // 🔥 Full Track 04: "Losing Control" - Tech House (126 BPM, F Minor, 3:30m)
            bpm_out = 126.0f;
            Pattern p1; p1.id = 1; p1.name = "01. Tech House Bouncy Pump"; p1.color = 0xFFFF9900;
            for (int b = 0; b < 16; b += 4) p1.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, b * 0.125f, 0.10f, 1.0f, 1.0f, 0));
            p1.getChannelNotes(6).push_back(KuroDSP::MidiNote(39, 0.5f, 0.10f, 0.95f, 1.0f, 6));
            p1.getChannelNotes(6).push_back(KuroDSP::MidiNote(39, 1.5f, 0.10f, 0.95f, 1.0f, 6));
            p1.getChannelNotes(3).push_back(KuroDSP::MidiNote(29, 0.25f, 0.15f, 0.95f, 1.0f, 3));
            p1.getChannelNotes(3).push_back(KuroDSP::MidiNote(29, 0.75f, 0.15f, 0.95f, 1.0f, 3));
            p1.getChannelNotes(3).push_back(KuroDSP::MidiNote(32, 1.25f, 0.15f, 0.95f, 1.0f, 3));
            p1.getChannelNotes(3).push_back(KuroDSP::MidiNote(34, 1.75f, 0.15f, 0.95f, 1.0f, 3));
            global_patterns.push_back(p1);

            for (int b = 0; b < 12; ++b) {
            }
        }
        current_pattern_idx = 0;
    }
};
