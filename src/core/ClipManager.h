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
        // Inicializa com 1 pattern vazio
        Pattern p;
        p.id = next_id++;
        p.name = "Pattern 1";
        p.color = 0xFF5050AA; // Laranja/Vermelho base
        global_patterns.push_back(p);
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
};
