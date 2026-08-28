#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <vector>
#include <mutex>
#include <algorithm>
#include <string>
#include "MidiNote.h"
#include "KuroConfig.h"
#include "../audio/SynthEngine.h"

struct AudioClip {
    int id;
    float start_time_sec;      // Posição no tempo do projeto
    float length_sec;          // Duração do clipe
    float source_offset_sec;   // De qual parte do áudio original esse clipe começa
    bool is_selected = false;
    bool is_muted = false;
    std::string name = "";
    std::string file_path = "";
    float pitch_shift_semitones = 0.0f; // Shift de tom (-12 a +12 semitones)
    float time_stretch_ratio = 1.0f;    // Fator de tempo (0.5x a 2.0x)
    bool enable_stretch = false;
    bool is_reversed = false;
    float fade_in_sec = 0.0f;
    float fade_out_sec = 0.0f;
    float gain_db = 0.0f;
    bool is_normalized = false;
};

struct AutoPoint {
    float time_rel_sec; // Tempo relativo ao início do clipe
    float value;        // 0.0f a 1.0f
    float tension = 0.0f; // -1.0f a +1.0f (Bézier tension)
};

struct AutomationClip {
    int id;
    std::string name;
    unsigned int color = 0xFF00E5FF;
    std::string target_node_id = "";
    int param_index = 0;
    float start_time_sec = 0.0f;
    float length_sec = 16.0f;
    bool is_selected = false;
    bool is_muted = false;
    std::vector<AutoPoint> points;

    AutomationClip() {
        points.push_back({0.0f, 0.2f, 0.0f});
        points.push_back({16.0f, 0.8f, 0.0f});
    }

    float getValueAtTime(float rel_t) const {
        if (points.empty()) return 0.5f;
        if (rel_t <= points.front().time_rel_sec) return points.front().value;
        if (rel_t >= points.back().time_rel_sec) return points.back().value;

        for (size_t i = 0; i < points.size() - 1; ++i) {
            if (rel_t >= points[i].time_rel_sec && rel_t < points[i+1].time_rel_sec) {
                float range = points[i+1].time_rel_sec - points[i].time_rel_sec;
                if (range <= 0.0001f) return points[i].value;
                float progress = (rel_t - points[i].time_rel_sec) / range;
                float tension = points[i].tension;
                float curved_progress = progress;
                if (std::abs(tension) > 0.001f) {
                    if (tension > 0.0f) {
                        float p = 1.0f + tension * 4.0f;
                        curved_progress = std::pow(progress, p);
                    } else {
                        float p = 1.0f + std::abs(tension) * 4.0f;
                        curved_progress = 1.0f - std::pow(1.0f - progress, p);
                    }
                }
                return points[i].value + (points[i+1].value - points[i].value) * curved_progress;
            }
        }
        return points.back().value;
    }
};

struct Pattern {
    int id;
    std::string name;
    unsigned int color;
    std::vector<KuroDSP::MidiNote> notes; // General / legacy notes
    std::vector<KuroDSP::MidiNote> channel_notes[MAX_TRACKS]; // Per-channel notes (0..MAX_TRACKS-1)
    float default_length_sec = 4.0f;

    std::vector<KuroDSP::MidiNote>& getChannelNotes(int channel_idx) {
        int idx = (std::clamp)(channel_idx, 0, MAX_TRACKS - 1);
        return channel_notes[idx];
    }

    const std::vector<KuroDSP::MidiNote>& getChannelNotes(int channel_idx) const {
        int idx = (std::clamp)(channel_idx, 0, MAX_TRACKS - 1);
        return channel_notes[idx];
    }
};

struct MidiClip {
    int id;
    float start_time_sec;
    float length_sec;
    int pattern_id; // Refers to the Pattern.id
    bool is_selected = false;
    bool is_muted = false;
    std::string name = "";
};

class ClipManager {
public:
    std::vector<AudioClip> track_clips[MAX_TRACKS];
    std::vector<MidiClip> track_midi_clips[MAX_TRACKS];
    std::vector<AutomationClip> track_auto_clips[MAX_TRACKS];
    std::vector<Pattern> global_patterns;
    int current_pattern_idx = 0;
    std::mutex clip_mutex;
    int next_id = 1;

    struct ProjectSnapshot {
        std::vector<AudioClip> track_clips[MAX_TRACKS];
        std::vector<MidiClip> track_midi_clips[MAX_TRACKS];
        std::vector<AutomationClip> track_auto_clips[MAX_TRACKS];
        std::vector<Pattern> global_patterns;
        int current_pattern_idx = 0;
    };
    
    std::vector<ProjectSnapshot> undo_stack;
    std::vector<ProjectSnapshot> redo_stack;
    
    // Clipboard
    std::vector<AudioClip> clip_board_audio;
    std::vector<MidiClip> clip_board_midi;
    std::vector<KuroDSP::MidiNote> clip_board_notes;
    int clip_board_source_track = 0;

    void pushUndo() {
        std::lock_guard<std::mutex> lock(clip_mutex);
        ProjectSnapshot snap;
        for (int i = 0; i < MAX_TRACKS; i++) {
            snap.track_clips[i] = track_clips[i];
            snap.track_midi_clips[i] = track_midi_clips[i];
            snap.track_auto_clips[i] = track_auto_clips[i];
        }
        snap.global_patterns = global_patterns;
        snap.current_pattern_idx = current_pattern_idx;
        undo_stack.push_back(snap);
        if (undo_stack.size() > 50) undo_stack.erase(undo_stack.begin());
        redo_stack.clear();
    }

    void undo() {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (undo_stack.empty()) return;
        
        ProjectSnapshot current_snap;
        for (int i = 0; i < MAX_TRACKS; i++) {
            current_snap.track_clips[i] = track_clips[i];
            current_snap.track_midi_clips[i] = track_midi_clips[i];
            current_snap.track_auto_clips[i] = track_auto_clips[i];
        }
        current_snap.global_patterns = global_patterns;
        current_snap.current_pattern_idx = current_pattern_idx;
        redo_stack.push_back(current_snap);
        
        ProjectSnapshot prev = undo_stack.back();
        undo_stack.pop_back();
        
        for (int i = 0; i < MAX_TRACKS; i++) {
            track_clips[i] = prev.track_clips[i];
            track_midi_clips[i] = prev.track_midi_clips[i];
            track_auto_clips[i] = prev.track_auto_clips[i];
        }
        global_patterns = prev.global_patterns;
        current_pattern_idx = prev.current_pattern_idx;
    }

    void redo() {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (redo_stack.empty()) return;
        
        ProjectSnapshot current_snap;
        for (int i = 0; i < MAX_TRACKS; i++) {
            current_snap.track_clips[i] = track_clips[i];
            current_snap.track_midi_clips[i] = track_midi_clips[i];
            current_snap.track_auto_clips[i] = track_auto_clips[i];
        }
        current_snap.global_patterns = global_patterns;
        current_snap.current_pattern_idx = current_pattern_idx;
        undo_stack.push_back(current_snap);
        
        ProjectSnapshot next_s = redo_stack.back();
        redo_stack.pop_back();
        
        for (int i = 0; i < MAX_TRACKS; i++) {
            track_clips[i] = next_s.track_clips[i];
            track_midi_clips[i] = next_s.track_midi_clips[i];
            track_auto_clips[i] = next_s.track_auto_clips[i];
        }
        global_patterns = next_s.global_patterns;
        current_pattern_idx = next_s.current_pattern_idx;
    }

    void copySelection(int selected_track = 0) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        clip_board_audio.clear();
        clip_board_midi.clear();
        clip_board_notes.clear();
        clip_board_source_track = selected_track;
        
        for (int t = 0; t < MAX_TRACKS; t++) {
            for (const auto& ac : track_clips[t]) {
                if (ac.is_selected) clip_board_audio.push_back(ac);
            }
            for (const auto& mc : track_midi_clips[t]) {
                if (mc.is_selected) clip_board_midi.push_back(mc);
            }
        }
        if (!global_patterns.empty()) {
            int p_idx = (std::clamp)(current_pattern_idx, 0, (int)global_patterns.size() - 1);
            for (const auto& n : global_patterns[p_idx].getChannelNotes(selected_track)) {
                if (n.is_selected) clip_board_notes.push_back(n);
            }
        }
    }

    void cutSelection(int selected_track = 0) {
        pushUndo();
        copySelection(selected_track);
        std::lock_guard<std::mutex> lock(clip_mutex);
        for (int t = 0; t < MAX_TRACKS; t++) {
            auto& acs = track_clips[t];
            acs.erase(std::remove_if(acs.begin(), acs.end(), [](const AudioClip& c){ return c.is_selected; }), acs.end());
            auto& mcs = track_midi_clips[t];
            mcs.erase(std::remove_if(mcs.begin(), mcs.end(), [](const MidiClip& c){ return c.is_selected; }), mcs.end());
        }
        if (!global_patterns.empty()) {
            int p_idx = (std::clamp)(current_pattern_idx, 0, (int)global_patterns.size() - 1);
            auto& notes = global_patterns[p_idx].getChannelNotes(selected_track);
            notes.erase(std::remove_if(notes.begin(), notes.end(), [](const KuroDSP::MidiNote& n){ return n.is_selected; }), notes.end());
        }
    }

    void pasteClipboard(int target_track = 0, float paste_time_sec = 0.0f) {
        if (clip_board_audio.empty() && clip_board_midi.empty() && clip_board_notes.empty()) return;
        pushUndo();
        std::lock_guard<std::mutex> lock(clip_mutex);
        
        float min_time = 1e9f;
        for (const auto& ac : clip_board_audio) if (ac.start_time_sec < min_time) min_time = ac.start_time_sec;
        for (const auto& mc : clip_board_midi) if (mc.start_time_sec < min_time) min_time = mc.start_time_sec;
        for (const auto& n : clip_board_notes) if (n.start_time < min_time) min_time = n.start_time;
        if (min_time >= 1e8f) min_time = 0.0f;
        
        for (const auto& ac : clip_board_audio) {
            int t = std::clamp(target_track, 0, MAX_TRACKS - 1);
            AudioClip copy_ac = ac;
            copy_ac.id = next_id++;
            copy_ac.start_time_sec = paste_time_sec + (ac.start_time_sec - min_time);
            copy_ac.is_selected = true;
            track_clips[t].push_back(copy_ac);
        }
        for (const auto& mc : clip_board_midi) {
            int t = std::clamp(target_track, 0, MAX_TRACKS - 1);
            MidiClip copy_mc = mc;
            copy_mc.id = next_id++;
            copy_mc.start_time_sec = paste_time_sec + (mc.start_time_sec - min_time);
            copy_mc.is_selected = true;
            track_midi_clips[t].push_back(copy_mc);
        }
        if (!clip_board_notes.empty() && !global_patterns.empty()) {
            int p_idx = (std::clamp)(current_pattern_idx, 0, (int)global_patterns.size() - 1);
            auto& notes = global_patterns[p_idx].getChannelNotes(target_track);
            for (const auto& n : clip_board_notes) {
                KuroDSP::MidiNote copy_n = n;
                copy_n.start_time = paste_time_sec + (n.start_time - min_time);
                copy_n.is_selected = true;
                notes.push_back(copy_n);
            }
        }
    }

    ClipManager() {
        // Inicializa com 1 pattern base 100% limpo e vazio (FL Studio Default)
        Pattern p;
        p.id = next_id++;
        p.name = "Pattern 1";
        p.color = 0xFF00E5FF;
        p.notes.clear();
        for (int c = 0; c < MAX_TRACKS; c++) {
            p.channel_notes[c].clear();
        }
        global_patterns.push_back(p);
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
        int idx = (std::clamp)(current_pattern_idx, 0, (int)global_patterns.size() - 1);
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
        clip.is_muted = false;
        clip.name = "Audio Stem";
        track_clips[track_index].push_back(clip);
    }

    void reset() {
        std::lock_guard<std::mutex> lock(clip_mutex);
        for(int i=0; i<MAX_TRACKS; i++) {
            track_clips[i].clear();
            track_midi_clips[i].clear();
        }
        global_patterns.clear();
        Pattern p;
        p.id = 1;
        p.name = "Pattern 1";
        p.color = 0xFF00E5FF;
        for (int c = 0; c < 8; c++) {
            p.channel_notes[c].clear();
        }
        global_patterns.push_back(p);
        current_pattern_idx = 0;
        next_id = 2;
    }
    
    void splitClip(int track_index, int clip_index, float split_time_sec) {
        pushUndo();
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_clips[track_index].size()) return;
        
        AudioClip& c = track_clips[track_index][clip_index];
        if (split_time_sec <= c.start_time_sec || split_time_sec >= c.start_time_sec + c.length_sec) return;
        
        float relative_split = split_time_sec - c.start_time_sec;
        
        AudioClip new_clip = c;
        new_clip.id = next_id++;
        new_clip.start_time_sec = split_time_sec;
        new_clip.length_sec = c.length_sec - relative_split;
        new_clip.source_offset_sec = c.source_offset_sec + relative_split;
        new_clip.is_selected = false;
        
        c.length_sec = relative_split;
        
        track_clips[track_index].insert(track_clips[track_index].begin() + clip_index + 1, new_clip);
    }

    void splitMidiClip(int track_index, int clip_index, float split_time_sec) {
        pushUndo();
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_midi_clips[track_index].size()) return;
        
        MidiClip& c = track_midi_clips[track_index][clip_index];
        if (split_time_sec <= c.start_time_sec || split_time_sec >= c.start_time_sec + c.length_sec) return;
        
        float relative_split = split_time_sec - c.start_time_sec;
        
        MidiClip new_clip = c;
        new_clip.id = next_id++;
        new_clip.start_time_sec = split_time_sec;
        new_clip.length_sec = c.length_sec - relative_split;
        new_clip.is_selected = false;
        
        c.length_sec = relative_split;
        
        track_midi_clips[track_index].insert(track_midi_clips[track_index].begin() + clip_index + 1, new_clip);
    }
    
    void updateClipStart(int track_index, int clip_index, float new_start_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_clips[track_index].size()) return;
        track_clips[track_index][clip_index].start_time_sec = (std::max)(0.0f, new_start_sec);
    }

    void updateMidiClipStart(int track_index, int clip_index, float new_start_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_midi_clips[track_index].size()) return;
        track_midi_clips[track_index][clip_index].start_time_sec = (std::max)(0.0f, new_start_sec);
    }

    void moveAudioClip(int src_track, int clip_index, int dst_track, float new_start_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (src_track < 0 || src_track >= MAX_TRACKS || dst_track < 0 || dst_track >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_clips[src_track].size()) return;

        AudioClip clip = track_clips[src_track][clip_index];
        clip.start_time_sec = (std::max)(0.0f, new_start_sec);
        track_clips[src_track].erase(track_clips[src_track].begin() + clip_index);
        track_clips[dst_track].push_back(clip);
    }

    void moveMidiClip(int src_track, int clip_index, int dst_track, float new_start_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (src_track < 0 || src_track >= MAX_TRACKS || dst_track < 0 || dst_track >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_midi_clips[src_track].size()) return;

        MidiClip clip = track_midi_clips[src_track][clip_index];
        clip.start_time_sec = (std::max)(0.0f, new_start_sec);
        track_midi_clips[src_track].erase(track_midi_clips[src_track].begin() + clip_index);
        track_midi_clips[dst_track].push_back(clip);
    }

    void resizeAudioClip(int track_index, int clip_index, float new_length_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_clips[track_index].size()) return;
        track_clips[track_index][clip_index].length_sec = (std::max)(0.25f, new_length_sec);
    }

    void resizeMidiClip(int track_index, int clip_index, float new_length_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_midi_clips[track_index].size()) return;
        track_midi_clips[track_index][clip_index].length_sec = (std::max)(0.25f, new_length_sec);
    }

    void duplicateAudioClip(int track_index, int clip_index, float offset_sec) {
        pushUndo();
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_clips[track_index].size()) return;
        AudioClip c = track_clips[track_index][clip_index];
        c.id = next_id++;
        c.start_time_sec += offset_sec > 0.0f ? offset_sec : c.length_sec;
        track_clips[track_index].push_back(c);
    }

    void duplicateMidiClip(int track_index, int clip_index, float offset_sec) {
        pushUndo();
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_midi_clips[track_index].size()) return;
        MidiClip c = track_midi_clips[track_index][clip_index];
        c.id = next_id++;
        c.start_time_sec += offset_sec > 0.0f ? offset_sec : c.length_sec;
        track_midi_clips[track_index].push_back(c);
    }

    void toggleMuteAudioClip(int track_index, int clip_index) {
        pushUndo();
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_clips[track_index].size()) return;
        track_clips[track_index][clip_index].is_muted = !track_clips[track_index][clip_index].is_muted;
    }

    void toggleMuteMidiClip(int track_index, int clip_index) {
        pushUndo();
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_midi_clips[track_index].size()) return;
        track_midi_clips[track_index][clip_index].is_muted = !track_midi_clips[track_index][clip_index].is_muted;
    }

    void deleteClip(int track_index, int clip_index) {
        pushUndo();
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_clips[track_index].size()) return;
        track_clips[track_index].erase(track_clips[track_index].begin() + clip_index);
    }

    void deleteMidiClip(int track_index, int clip_index) {
        pushUndo();
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_midi_clips[track_index].size()) return;
        track_midi_clips[track_index].erase(track_midi_clips[track_index].begin() + clip_index);
    }

    int makeClipUnique(int track_index, int clip_index, bool is_midi) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return -1;
        if (is_midi) {
            if (clip_index < 0 || clip_index >= (int)track_midi_clips[track_index].size()) return -1;
            MidiClip& mc = track_midi_clips[track_index][clip_index];
            // Encontra o pattern atual e clona
            Pattern* source_pat = nullptr;
            for (auto& p : global_patterns) {
                if (p.id == mc.pattern_id) { source_pat = &p; break; }
            }
            if (source_pat) {
                Pattern new_p = *source_pat;
                new_p.id = next_id++;
                new_p.name = source_pat->name + " #2";
                global_patterns.push_back(new_p);
                mc.pattern_id = new_p.id;
                return new_p.id;
            }
        } else {
            if (clip_index < 0 || clip_index >= (int)track_clips[track_index].size()) return -1;
            AudioClip& ac = track_clips[track_index][clip_index];
            ac.id = next_id++;
            ac.name = ac.name + " (Unique)";
            return ac.id;
        }
        return -1;
    }

    void addAutomationClip(int track_index, float start_sec, float length_sec, const std::string& name, const std::string& target_node = "", int param_idx = 0) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        AutomationClip ac;
        ac.id = next_id++;
        ac.name = name;
        ac.start_time_sec = start_sec;
        ac.length_sec = length_sec;
        ac.target_node_id = target_node;
        ac.param_index = param_idx;
        ac.points.clear();
        ac.points.push_back({0.0f, 0.25f, 0.0f});
        ac.points.push_back({length_sec * 0.5f, 0.75f, 0.3f});
        ac.points.push_back({length_sec, 0.95f, 0.0f});
        track_auto_clips[track_index].push_back(ac);
    }

    std::vector<AutomationClip> getAutomationClips(int track_index) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return std::vector<AutomationClip>();
        return track_auto_clips[track_index];
    }

    void deleteAutomationClip(int track_index, int clip_index) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_auto_clips[track_index].size()) return;
        track_auto_clips[track_index].erase(track_auto_clips[track_index].begin() + clip_index);
    }

    void updateAutomationClipStart(int track_index, int clip_index, float new_start_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_auto_clips[track_index].size()) return;
        track_auto_clips[track_index][clip_index].start_time_sec = (std::max)(0.0f, new_start_sec);
    }

    void resizeAutomationClip(int track_index, int clip_index, float new_length_sec) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return;
        if (clip_index < 0 || clip_index >= (int)track_auto_clips[track_index].size()) return;
        track_auto_clips[track_index][clip_index].length_sec = (std::max)(0.5f, new_length_sec);
    }

    std::vector<AudioClip> getClips(int track_index) {
        std::lock_guard<std::mutex> lock(clip_mutex);
        if (track_index < 0 || track_index >= MAX_TRACKS) return std::vector<AudioClip>();
        return track_clips[track_index];
    }
    
    void addPatternClip(int track_index, float start_time_sec, float length_sec, int pattern_id) {
        pushUndo();
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

        if (template_id == 0) { // 🌀 Full Track 01: "Perception - The Astral Portal" (Brazilian Progressive Psytrance 140 BPM, F# Minor, 16 Tracks, 96 Bars / 2:44 min)
            bpm_out = 140.0f;
            float beat = 60.0f / 140.0f;        // 0.42857s
            float step = beat / 4.0f;           // 0.10714s (16th note)
            float bar = beat * 4.0f;            // 1.71428s
            float block_len = bar * 8.0f;       // 13.714s (8 bars complete harmonic block)

            extern KuroAudio::SynthEngine g_piano_synth;
            // Configura os 16 canais no Sampler / Synths
            g_piano_synth.flex_active[0]  = false; // Track 0: Psy Kick (Punch & Click Transient)
            g_piano_synth.flex_active[1]  = true;  // Track 1: Rolling Sub Bass (46Hz Sub)
            g_piano_synth.flex_active[2]  = true;  // Track 2: Rolling Mid Saw Bass (92Hz Growl)
            g_piano_synth.flex_active[3]  = false; // Track 3: Snare & Smash Clap
            g_piano_synth.flex_active[4]  = false; // Track 4: Offbeat Open Hi-Hat
            g_piano_synth.flex_active[5]  = false; // Track 5: Closed Hats & Shakers
            g_piano_synth.flex_active[6]  = true;  // Track 6: Brazilian Tribal Percussion
            g_piano_synth.flex_active[7]  = true;  // Track 7: FM Squelch & Modular Zaps
            g_piano_synth.flex_active[8]  = true;  // Track 8: Main Acid Lead Hook
            g_piano_synth.flex_active[9]  = true;  // Track 9: Counter-Arp Pluck Matrix
            g_piano_synth.flex_active[10] = true;  // Track 10: SuperSaw Polyphonic Lead
            g_piano_synth.flex_active[11] = true;  // Track 11: Dark Sub Drone
            g_piano_synth.flex_active[12] = true;  // Track 12: Symphonic Strings & Mystic Pads
            g_piano_synth.flex_active[13] = true;  // Track 13: Steinway Piano Stabs
            g_piano_synth.flex_active[14] = true;  // Track 14: Vocal Mantra Chants
            g_piano_synth.flex_active[15] = true;  // Track 15: Snare Roll & FX Risers

            g_piano_synth.flex_channel_instrument[1]  = KuroAudio::MidiInstrument::SUB_BASS;
            g_piano_synth.flex_channel_instrument[2]  = KuroAudio::MidiInstrument::SYNTH_SAW;
            g_piano_synth.flex_channel_instrument[6]  = KuroAudio::MidiInstrument::MARIMBA;
            g_piano_synth.flex_channel_instrument[7]  = KuroAudio::MidiInstrument::FM_SYNTH;
            g_piano_synth.flex_channel_instrument[8]  = KuroAudio::MidiInstrument::SYNTH_SAW;
            g_piano_synth.flex_channel_instrument[9]  = KuroAudio::MidiInstrument::TRANCE_LEAD;
            g_piano_synth.flex_channel_instrument[10] = KuroAudio::MidiInstrument::FULLON_LEAD;
            g_piano_synth.flex_channel_instrument[11] = KuroAudio::MidiInstrument::PAD_SYNTH;
            g_piano_synth.flex_channel_instrument[12] = KuroAudio::MidiInstrument::STRING_ENSEMBLE;
            g_piano_synth.flex_channel_instrument[13] = KuroAudio::MidiInstrument::ACOUSTIC_PIANO;
            g_piano_synth.flex_channel_instrument[14] = KuroAudio::MidiInstrument::CHOIR;
            g_piano_synth.flex_channel_instrument[15] = KuroAudio::MidiInstrument::SWEEP_PAD;

            // =========================================================================
            // PATTERN 1: PSY KICK (PUNCH & CLICK TRANSIENT SNAP - 4-ON-THE-FLOOR)
            // =========================================================================
            Pattern p_kick; p_kick.id = 1; p_kick.name = "01. Psy Kick (Estalo & Sub Punch)"; p_kick.color = 0xFF00F0FF;
            for (int b = 0; b < 8; ++b) {
                for (int beat_i = 0; beat_i < 4; ++beat_i) {
                    p_kick.getChannelNotes(0).push_back(KuroDSP::MidiNote(36, b * bar + beat_i * beat, 0.18f, 1.0f, 1.0f, 0));
                }
            }
            global_patterns.push_back(p_kick);

            // =========================================================================
            // PATTERN 2: ROLLING SUB BASS (KB-B-B 16TH PULSE - 46Hz F#1)
            // =========================================================================
            Pattern p_sub_bass; p_sub_bass.id = 2; p_sub_bass.name = "02. Sub Bass (KB-B-B 16th Pulse)"; p_sub_bass.color = 0xFFFF0080;
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                int root_pitch = (b < 2) ? 30 : ((b < 4) ? 33 : ((b < 6) ? 28 : 26)); // F#1, A1, E1, D1
                for (int beat_i = 0; beat_i < 4; ++beat_i) {
                    float b_start = bar_offset + beat_i * beat;
                    // Toca nos 16ths 1, 2, 3 (pula o 0 onde o kick bate!)
                    p_sub_bass.getChannelNotes(1).push_back(KuroDSP::MidiNote(root_pitch, b_start + step * 1.0f, step * 0.88f, 0.92f, 1.0f, 1));
                    p_sub_bass.getChannelNotes(1).push_back(KuroDSP::MidiNote(root_pitch, b_start + step * 2.0f, step * 0.88f, 0.90f, 1.0f, 1));
                    p_sub_bass.getChannelNotes(1).push_back(KuroDSP::MidiNote(root_pitch, b_start + step * 3.0f, step * 0.88f, 0.94f, 1.0f, 1));
                }
            }
            global_patterns.push_back(p_sub_bass);

            // =========================================================================
            // PATTERN 3: ROLLING MID SAW BASS (GROWL & OCTAVE JUMPS)
            // =========================================================================
            Pattern p_mid_bass; p_mid_bass.id = 3; p_mid_bass.name = "03. Mid Saw Bass (Growl & Cutoff Sweep)"; p_mid_bass.color = 0xFFFF3CA0;
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                int root_pitch = (b < 2) ? 42 : ((b < 4) ? 45 : ((b < 6) ? 40 : 38)); // F#2, A2, E2, D2
                for (int s = 0; s < 16; ++s) {
                    if (s % 4 == 0) continue; // Pula os tempos do kick
                    int pitch = root_pitch;
                    if (s == 11 || s == 15) pitch += 12; // Salto de oitava síncopado característico do Perception
                    float vel = (s == 11 || s == 15) ? 0.98f : 0.82f;
                    p_mid_bass.getChannelNotes(2).push_back(KuroDSP::MidiNote(pitch, bar_offset + s * step, step * 0.80f, vel, 1.0f, 2));
                }
            }
            global_patterns.push_back(p_mid_bass);

            // =========================================================================
            // PATTERN 4: SNARE & SMASH CLAP (BEATS 2 & 4)
            // =========================================================================
            Pattern p_snare; p_snare.id = 4; p_snare.name = "04. Snare & Smash Clap (Beats 2 & 4)"; p_snare.color = 0xFFFF8C00;
            for (int b = 0; b < 8; ++b) {
                p_snare.getChannelNotes(3).push_back(KuroDSP::MidiNote(38, b * bar + beat * 1.0f, 0.25f, 0.98f, 1.0f, 3));
                p_snare.getChannelNotes(3).push_back(KuroDSP::MidiNote(38, b * bar + beat * 3.0f, 0.25f, 0.98f, 1.0f, 3));
            }
            global_patterns.push_back(p_snare);

            // =========================================================================
            // PATTERN 5: OFFBEAT OPEN HI-HAT (SIZZLING 8THS)
            // =========================================================================
            Pattern p_open_hat; p_open_hat.id = 5; p_open_hat.name = "05. Offbeat Open Hat (Sizzling 8ths)"; p_open_hat.color = 0xFFFFDC00;
            for (int b = 0; b < 8; ++b) {
                for (int beat_i = 0; beat_i < 4; ++beat_i) {
                    p_open_hat.getChannelNotes(4).push_back(KuroDSP::MidiNote(46, b * bar + beat_i * beat + beat * 0.5f, 0.20f, 0.88f, 1.0f, 4));
                }
            }
            global_patterns.push_back(p_open_hat);

            // =========================================================================
            // PATTERN 6: CLOSED HI-HATS & SHAKERS (16TH DRIVING GROOVE)
            // =========================================================================
            Pattern p_closed_hat; p_closed_hat.id = 6; p_closed_hat.name = "06. Closed Hats & Shakers (16th Groove)"; p_closed_hat.color = 0xFFC8FF00;
            for (int b = 0; b < 8; ++b) {
                for (int s = 0; s < 16; ++s) {
                    float vel = (s % 4 == 2) ? 0.85f : ((s % 2 == 1) ? 0.55f : 0.40f);
                    p_closed_hat.getChannelNotes(5).push_back(KuroDSP::MidiNote(42, b * bar + s * step, step * 0.65f, vel, 1.0f, 5));
                }
            }
            global_patterns.push_back(p_closed_hat);

            // =========================================================================
            // PATTERN 7: BRAZILIAN TRIBAL PERCUSSION (CONGAS / BONGOS / RIM)
            // =========================================================================
            Pattern p_tribal; p_tribal.id = 7; p_tribal.name = "07. Brazilian Tribal Percussion"; p_tribal.color = 0xFF00FF80;
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                p_tribal.getChannelNotes(6).push_back(KuroDSP::MidiNote(65, bar_offset + step * 3.0f, step * 1.2f, 0.82f, 1.0f, 6));
                p_tribal.getChannelNotes(6).push_back(KuroDSP::MidiNote(62, bar_offset + step * 6.0f, step * 1.2f, 0.86f, 1.0f, 6));
                p_tribal.getChannelNotes(6).push_back(KuroDSP::MidiNote(67, bar_offset + step * 10.0f, step * 1.2f, 0.90f, 1.0f, 6));
                p_tribal.getChannelNotes(6).push_back(KuroDSP::MidiNote(69, bar_offset + step * 14.0f, step * 1.2f, 0.88f, 1.0f, 6));
            }
            global_patterns.push_back(p_tribal);

            // =========================================================================
            // PATTERN 8: FM SQUELCH & MODULAR ZAPS (PSY 303 / SERUM)
            // =========================================================================
            Pattern p_fm_squelch; p_fm_squelch.id = 8; p_fm_squelch.name = "08. FM Squelch & Modular Zaps"; p_fm_squelch.color = 0xFF00FFC8;
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                if (b % 2 == 1) { // Turnaround squelches
                    p_fm_squelch.getChannelNotes(7).push_back(KuroDSP::MidiNote(78, bar_offset + step * 12.0f, step * 0.75f, 0.95f, 1.0f, 7));
                    p_fm_squelch.getChannelNotes(7).push_back(KuroDSP::MidiNote(81, bar_offset + step * 13.5f, step * 0.75f, 0.98f, 1.0f, 7));
                    p_fm_squelch.getChannelNotes(7).push_back(KuroDSP::MidiNote(73, bar_offset + step * 15.0f, step * 0.75f, 0.92f, 1.0f, 7));
                }
            }
            global_patterns.push_back(p_fm_squelch);

            // =========================================================================
            // PATTERN 9: MAIN ACID LEAD HOOK (16TH SCREAMING RIFF)
            // =========================================================================
            Pattern p_acid; p_acid.id = 9; p_acid.name = "09. Main Acid Lead Hook (16th Screaming Riff)"; p_acid.color = 0xFF00B4FF;
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                int root = (b < 2) ? 66 : ((b < 4) ? 69 : ((b < 6) ? 64 : 62)); // F#4, A4, E4, D4
                for (int s = 0; s < 16; ++s) {
                    int note = root;
                    if (s == 3 || s == 7) note += 3;
                    else if (s == 8 || s == 12) note += 7;
                    else if (s == 11 || s == 15) note += 12;
                    p_acid.getChannelNotes(8).push_back(KuroDSP::MidiNote(note, bar_offset + s * step, step * 0.85f, 0.92f, 1.0f, 8));
                }
            }
            global_patterns.push_back(p_acid);

            // =========================================================================
            // PATTERN 10: COUNTER-ARP PLUCK MATRIX (HYPNOTIC ARPEGGIOS)
            // =========================================================================
            Pattern p_counter_arp; p_counter_arp.id = 10; p_counter_arp.name = "10. Counter-Arp Pluck Matrix"; p_counter_arp.color = 0xFF5078FF;
            for (int b = 0; b < 8; ++b) {
                float bar_offset = b * bar;
                int notes_seq[8] = { 78, 76, 73, 69, 71, 73, 76, 81 }; // F#5, E5, C#5, A4, B4, C#5, E5, A5
                for (int i = 0; i < 8; ++i) {
                    p_counter_arp.getChannelNotes(9).push_back(KuroDSP::MidiNote(notes_seq[i], bar_offset + i * (step * 2.0f), step * 1.1f, 0.85f, 1.0f, 9));
                }
            }
            global_patterns.push_back(p_counter_arp);

            // =========================================================================
            // PATTERN 11: SUPERSAW POLYPHONIC LEAD (WIDESCREEN STEREO HOOK)
            // =========================================================================
            Pattern p_supersaw; p_supersaw.id = 11; p_supersaw.name = "11. SuperSaw Poly Lead Hook"; p_supersaw.color = 0xFFA050FF;
            // Bars 1-2: F#m (F#3, C#4, F#4, A4)
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(54, 0.0f, bar * 2.0f, 0.92f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(61, 0.0f, bar * 2.0f, 0.92f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(66, 0.0f, bar * 2.0f, 0.95f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(69, 0.0f, bar * 2.0f, 0.95f, 1.0f, 10));
            // Bars 3-4: A (A3, E4, A4, C#5)
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(57, bar * 2.0f, bar * 2.0f, 0.92f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(64, bar * 2.0f, bar * 2.0f, 0.92f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(69, bar * 2.0f, bar * 2.0f, 0.95f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(73, bar * 2.0f, bar * 2.0f, 0.95f, 1.0f, 10));
            // Bars 5-6: E (E3, B3, E4, G#4)
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(52, bar * 4.0f, bar * 2.0f, 0.92f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(59, bar * 4.0f, bar * 2.0f, 0.92f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(64, bar * 4.0f, bar * 2.0f, 0.95f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(68, bar * 4.0f, bar * 2.0f, 0.95f, 1.0f, 10));
            // Bars 7-8: Dmaj7 (D3, A3, D4, F#4, C#5)
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(50, bar * 6.0f, bar * 2.0f, 0.95f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(57, bar * 6.0f, bar * 2.0f, 0.95f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(62, bar * 6.0f, bar * 2.0f, 0.95f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(66, bar * 6.0f, bar * 2.0f, 0.98f, 1.0f, 10));
            p_supersaw.getChannelNotes(10).push_back(KuroDSP::MidiNote(73, bar * 6.0f, bar * 2.0f, 0.98f, 1.0f, 10));
            global_patterns.push_back(p_supersaw);

            // =========================================================================
            // PATTERN 12: DARK ATMOSPHERIC SUB DRONE (DEEP TENSION)
            // =========================================================================
            Pattern p_drone; p_drone.id = 12; p_drone.name = "12. Dark Atmospheric Sub Drone"; p_drone.color = 0xFF6E28DC;
            p_drone.getChannelNotes(11).push_back(KuroDSP::MidiNote(42, 0.0f, block_len, 0.88f, 1.0f, 11)); // F#2
            p_drone.getChannelNotes(11).push_back(KuroDSP::MidiNote(49, 0.0f, block_len, 0.85f, 1.0f, 11)); // C#3
            p_drone.getChannelNotes(11).push_back(KuroDSP::MidiNote(54, 0.0f, block_len, 0.82f, 1.0f, 11)); // F#3
            global_patterns.push_back(p_drone);

            // =========================================================================
            // PATTERN 13: SYMPHONIC STRINGS & MYSTIC PADS
            // =========================================================================
            Pattern p_strings; p_strings.id = 13; p_strings.name = "13. Symphonic Strings & Mystic Pads"; p_strings.color = 0xFF00D2E6;
            // Bars 1-2: F#m9 (F#2, C#3, A3, C#4, G#4)
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(42, 0.0f, bar * 2.0f, 0.88f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(49, 0.0f, bar * 2.0f, 0.88f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(57, 0.0f, bar * 2.0f, 0.85f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(61, 0.0f, bar * 2.0f, 0.85f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(68, 0.0f, bar * 2.0f, 0.82f, 1.0f, 12));
            // Bars 3-4: Aadd9 (A2, E3, A3, C#4, B4)
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(45, bar * 2.0f, bar * 2.0f, 0.88f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(52, bar * 2.0f, bar * 2.0f, 0.88f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(57, bar * 2.0f, bar * 2.0f, 0.85f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(61, bar * 2.0f, bar * 2.0f, 0.85f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(71, bar * 2.0f, bar * 2.0f, 0.82f, 1.0f, 12));
            // Bars 5-6: E (E2, B2, G#3, E4, B4)
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(40, bar * 4.0f, bar * 2.0f, 0.88f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(47, bar * 4.0f, bar * 2.0f, 0.88f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(56, bar * 4.0f, bar * 2.0f, 0.85f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(64, bar * 4.0f, bar * 2.0f, 0.85f, 1.0f, 12));
            // Bars 7-8: Dmaj7(#11) (D2, A2, F#3, C#4, G#4)
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(38, bar * 6.0f, bar * 2.0f, 0.90f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(45, bar * 6.0f, bar * 2.0f, 0.90f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(54, bar * 6.0f, bar * 2.0f, 0.88f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(61, bar * 6.0f, bar * 2.0f, 0.88f, 1.0f, 12));
            p_strings.getChannelNotes(12).push_back(KuroDSP::MidiNote(68, bar * 6.0f, bar * 2.0f, 0.85f, 1.0f, 12));
            global_patterns.push_back(p_strings);

            // =========================================================================
            // PATTERN 14: STEINWAY PIANO EMOTIVE STABS & ARPS
            // =========================================================================
            Pattern p_piano; p_piano.id = 14; p_piano.name = "14. Steinway Piano Stabs & Arps"; p_piano.color = 0xFFFFD700;
            // Chord stabs on downbeats
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(66, 0.0f, bar * 1.5f, 0.95f, 1.0f, 13));
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(69, 0.0f, bar * 1.5f, 0.95f, 1.0f, 13));
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(73, 0.0f, bar * 1.5f, 0.92f, 1.0f, 13));
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(69, bar * 2.0f, bar * 1.5f, 0.95f, 1.0f, 13));
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(73, bar * 2.0f, bar * 1.5f, 0.95f, 1.0f, 13));
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(76, bar * 2.0f, bar * 1.5f, 0.92f, 1.0f, 13));
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(64, bar * 4.0f, bar * 1.5f, 0.95f, 1.0f, 13));
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(68, bar * 4.0f, bar * 1.5f, 0.95f, 1.0f, 13));
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(71, bar * 4.0f, bar * 1.5f, 0.92f, 1.0f, 13));
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(62, bar * 6.0f, bar * 1.5f, 0.98f, 1.0f, 13));
            p_piano.getChannelNotes(13).push_back(KuroDSP::MidiNote(66, bar * 6.0f, bar * 1.5f, 0.98f, 1.0f, 13));
            global_patterns.push_back(p_piano);

            // =========================================================================
            // PATTERN 15: VOCAL MANTRA CHANTS & SPEECH CHOPS
            // =========================================================================
            Pattern p_vocal; p_vocal.id = 15; p_vocal.name = "15. Vocal Mantra Chants & Speech"; p_vocal.color = 0xFFFF5078;
            p_vocal.getChannelNotes(14).push_back(KuroDSP::MidiNote(66, 0.0f, bar * 2.0f, 0.88f, 1.0f, 14));
            p_vocal.getChannelNotes(14).push_back(KuroDSP::MidiNote(69, bar * 2.0f, bar * 2.0f, 0.88f, 1.0f, 14));
            p_vocal.getChannelNotes(14).push_back(KuroDSP::MidiNote(64, bar * 4.0f, bar * 2.0f, 0.88f, 1.0f, 14));
            p_vocal.getChannelNotes(14).push_back(KuroDSP::MidiNote(62, bar * 6.0f, bar * 2.0f, 0.92f, 1.0f, 14));
            global_patterns.push_back(p_vocal);

            // =========================================================================
            // PATTERN 16: SNARE ROLL CRESCENDO & FX RISERS
            // =========================================================================
            Pattern p_riser; p_riser.id = 16; p_riser.name = "16. Snare Roll Crescendo & FX Risers"; p_riser.color = 0xFFFF3232;
            for (int s = 0; s < 32; ++s) {
                float progress = (float)s / 32.0f;
                float vel = 0.40f + 0.60f * progress;
                float note_t = bar * 6.0f + s * (bar * 2.0f / 32.0f);
                p_riser.getChannelNotes(15).push_back(KuroDSP::MidiNote(38, note_t, 0.10f, vel, 1.0f, 15));
            }
            global_patterns.push_back(p_riser);

            // =========================================================================
            // ARRANJO COMPLETO NA PLAYLIST (ESTRUTURA DE 96 COMPASSOS / 2 MINUTOS E 44 SEGUNDOS)
            // =========================================================================
            auto addClip = [&](int track, int pat_id, float start_sec, float len_sec) {
                MidiClip mc;
                mc.id = next_id++;
                mc.start_time_sec = start_sec;
                mc.length_sec = len_sec;
                mc.pattern_id = pat_id;
                mc.is_selected = false;
                track_midi_clips[track].push_back(mc);
            };

            // 1. ACT I: THE TRIBAL MYSTIC AWAKENING (Bars 0-16 / 0.0s - 27.4s)
            for (int b = 0; b < 2; ++b) {
                float t = b * block_len;
                addClip(6, 7, t, block_len);   // Trk 6: Tribal Percussion
                addClip(11, 12, t, block_len); // Trk 11: Dark Sub Drone
                addClip(12, 13, t, block_len); // Trk 12: Symphonic Strings
                addClip(13, 14, t, block_len); // Trk 13: Steinway Piano Stabs
                addClip(14, 15, t, block_len); // Trk 14: Vocal Mantra Chants
            }

            // 2. ACT II: GROOVE LOCK - BASSLINE ARRIVAL (Bars 16-32 / 27.4s - 54.8s)
            for (int b = 2; b < 4; ++b) {
                float t = b * block_len;
                addClip(0, 1, t, block_len);   // Trk 0: Psy Kick (Punch & Click)
                addClip(1, 2, t, block_len);   // Trk 1: Sub Bass (KB-B-B)
                addClip(2, 3, t, block_len);   // Trk 2: Mid Saw Bass (Growl)
                addClip(5, 6, t, block_len);   // Trk 5: Closed Hats & Shakers
                addClip(9, 10, t, block_len);  // Trk 9: Counter-Arp Pluck
                addClip(11, 12, t, block_len); // Trk 11: Dark Sub Drone
            }

            // 3. ACT III: THE STORM & FM SQUELCHES (Bars 32-48 / 54.8s - 82.3s)
            for (int b = 4; b < 6; ++b) {
                float t = b * block_len;
                addClip(0, 1, t, block_len);   // Trk 0: Psy Kick
                addClip(1, 2, t, block_len);   // Trk 1: Sub Bass
                addClip(2, 3, t, block_len);   // Trk 2: Mid Saw Bass
                addClip(6, 7, t, block_len);   // Trk 6: Tribal Percussion
                addClip(7, 8, t, block_len);   // Trk 7: FM Squelch & Zaps
                addClip(8, 9, t, block_len);   // Trk 8: Main Acid Lead Hook
                addClip(14, 15, t, block_len); // Trk 14: Vocal Mantra Chants
                addClip(15, 16, t, block_len); // Trk 15: Snare Roll Crescendo & Risers
            }

            // 4. ACT IV: DROP 1 - FULL POWER PROGRESSIVE PSY CLIMAX (Bars 48-64 / 82.3s - 109.7s)
            for (int b = 6; b < 8; ++b) {
                float t = b * block_len;
                addClip(0, 1, t, block_len);   // Trk 0: Psy Kick
                addClip(1, 2, t, block_len);   // Trk 1: Sub Bass
                addClip(2, 3, t, block_len);   // Trk 2: Mid Saw Bass
                addClip(3, 4, t, block_len);   // Trk 3: Snare & Smash Clap
                addClip(4, 5, t, block_len);   // Trk 4: Offbeat Open Hat
                addClip(5, 6, t, block_len);   // Trk 5: Closed Hats & Shakers
                addClip(6, 7, t, block_len);   // Trk 6: Tribal Percussion
                addClip(7, 8, t, block_len);   // Trk 7: FM Squelch
                addClip(8, 9, t, block_len);   // Trk 8: Main Acid Lead Hook
                addClip(9, 10, t, block_len);  // Trk 9: Counter-Arp Pluck Matrix
                addClip(10, 11, t, block_len); // Trk 10: SuperSaw Poly Lead
                addClip(11, 12, t, block_len); // Trk 11: Dark Sub Drone
                addClip(12, 13, t, block_len); // Trk 12: Symphonic Strings
                addClip(13, 14, t, block_len); // Trk 13: Steinway Piano Stabs
                addClip(14, 15, t, block_len); // Trk 14: Vocal Mantra Chants
            }

            // 5. ACT V: THE PSYCHEDELIC BREAKDOWN (Bars 64-80 / 109.7s - 137.1s)
            for (int b = 8; b < 10; ++b) {
                float t = b * block_len;
                addClip(7, 8, t, block_len);   // Trk 7: FM Squelches
                addClip(8, 9, t, block_len);   // Trk 8: Solo Acid Lead
                addClip(11, 12, t, block_len); // Trk 11: Dark Sub Drone
                addClip(12, 13, t, block_len); // Trk 12: Symphonic Strings
                addClip(13, 14, t, block_len); // Trk 13: Steinway Piano Solo
                addClip(14, 15, t, block_len); // Trk 14: Vocal Mantra Chants
            }

            // 6. ACT VI: DROP 2 - THE FINAL SUPERNOVA (Bars 80-96 / 137.1s - 164.6s)
            for (int b = 10; b < 12; ++b) {
                float t = b * block_len;
                addClip(0, 1, t, block_len);   // Trk 0: Psy Kick
                addClip(1, 2, t, block_len);   // Trk 1: Sub Bass
                addClip(2, 3, t, block_len);   // Trk 2: Mid Saw Bass
                addClip(3, 4, t, block_len);   // Trk 3: Snare & Smash Clap
                addClip(4, 5, t, block_len);   // Trk 4: Offbeat Open Hat
                addClip(5, 6, t, block_len);   // Trk 5: Closed Hats & Shakers
                addClip(6, 7, t, block_len);   // Trk 6: Tribal Percussion
                addClip(7, 8, t, block_len);   // Trk 7: FM Squelch
                addClip(8, 9, t, block_len);   // Trk 8: Main Acid Lead Hook
                addClip(9, 10, t, block_len);  // Trk 9: Counter-Arp Pluck Matrix
                addClip(10, 11, t, block_len); // Trk 10: SuperSaw Poly Lead Hook
                addClip(11, 12, t, block_len); // Trk 11: Dark Sub Drone
                addClip(12, 13, t, block_len); // Trk 12: Symphonic Strings
                addClip(13, 14, t, block_len); // Trk 13: Steinway Piano Lead
                addClip(14, 15, t, block_len); // Trk 14: Vocal Mantra Chants
                addClip(15, 16, t, block_len); // Trk 15: Snare Roll Crescendo & Risers
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
