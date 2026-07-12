#pragma once
#include <vector>
#include <mutex>
#include "MidiNote.h"

struct AudioClip {
    int id;
    float start_time_sec;      // Posição no tempo do projeto
    float length_sec;          // Duração do clipe
    float source_offset_sec;   // De qual parte do áudio original esse clipe começa
    bool is_selected;
};

struct MidiClip {
    int id;
    float start_time_sec;
    float length_sec;
    std::vector<KuroDSP::MidiNote> notes;
    bool is_selected;
};

class ClipManager {
public:
    std::vector<AudioClip> track_clips[8];
    std::vector<MidiClip> track_midi_clips[8];
    std::mutex clip_mutex;
    int next_id = 1;

    ClipManager() {}

    void initTrack(int track_index, float total_length_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= 8) return;
        track_clips[track_index].clear();
        AudioClip clip;
        clip.id = next_id++;
        clip.start_time_sec = 0.0f;
        clip.length_sec = total_length_sec;
        clip.source_offset_sec = 0.0f;
        clip.is_selected = false;
        track_clips[track_index].push_back(clip);
    }

    void reset() {
        std::lock_guard<std::mutex> lock(clip_mutex);
        for(int i=0; i<8; i++) {
            track_clips[i].clear();
            track_midi_clips[i].clear();
        }
        next_id = 1;
    }
    
    void splitClip(int track_index, int clip_index, float split_time_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= 8) return;
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
        if (track_index < 0 || track_index >= 8) return;
        if (clip_index < 0 || clip_index >= track_clips[track_index].size()) return;
        track_clips[track_index][clip_index].start_time_sec = new_start_sec;
    }

    std::vector<AudioClip> getClips(int track_index) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= 8) return std::vector<AudioClip>();
        return track_clips[track_index];
    }
    
    void addMidiClip(int track_index, float start_time_sec, float length_sec, const std::vector<KuroDSP::MidiNote>& notes) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= 8) return;
        MidiClip clip;
        clip.id = next_id++;
        clip.start_time_sec = start_time_sec;
        clip.length_sec = length_sec;
        clip.notes = notes;
        clip.is_selected = false;
        track_midi_clips[track_index].push_back(clip);
    }
    
    std::vector<MidiClip> getMidiClips(int track_index) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= 8) return std::vector<MidiClip>();
        return track_midi_clips[track_index];
    }
};
