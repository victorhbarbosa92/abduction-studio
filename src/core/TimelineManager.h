#pragma once
#include <atomic>
#include <vector>
#include <mutex>
#include <tuple>
#include <cstdlib>
#include <cmath>
#include "KuroConfig.h"
#include "MidiNote.h"
#include "ClipManager.h"

extern int channel_tracks[8];

namespace KuroDSP {

    class TimelineManager {
    private:
        float bpm = 140.0f;
        unsigned int sample_rate = 44100;
        
        std::atomic<uint64_t> master_frame = {0};
        bool is_playing = false;

    public:
        std::mutex timeline_mutex;
        int track_steps_limit[MAX_TRACKS] = {16, 16, 16, 16, 16, 16, 16, 16};
        
        float getBPM() const { return bpm; }
        void setBPM(float b) { bpm = b; }
        
        void setMasterFrame(uint64_t frame) { master_frame.store(frame, std::memory_order_relaxed); }
        // 16 passos (1 Compass = 4 tempos = 16 semicolcheias)
        std::atomic<bool> seq_grid[16];
        std::atomic<int> current_step = {0};
        
        // FASE 10: AUTOMATION LANES
        struct AutoPoint {
            float time_sec;
            float value;
        };
        struct AutomationLane {
            std::string target_node_id;
            int param_index;
            std::vector<AutoPoint> points; // Mantido em ordem cronológica
            
            float getValueAtTime(float t) const {
                if (points.empty()) return 0.0f;
                if (t <= points.front().time_sec) return points.front().value;
                if (t >= points.back().time_sec) return points.back().value;
                
                for (size_t i = 0; i < points.size() - 1; ++i) {
                    if (t >= points[i].time_sec && t < points[i+1].time_sec) {
                        float range = points[i+1].time_sec - points[i].time_sec;
                        float progress = (t - points[i].time_sec) / range;
                        return points[i].value + (points[i+1].value - points[i].value) * progress; // LERP linear
                    }
                }
                return points.back().value;
            }
        };
        std::vector<AutomationLane> automation_lanes;
        
        void addAutomationPoint(const std::string& node_id, int param_index, float time_sec, float value) {
            std::lock_guard<std::mutex> lock(timeline_mutex);
            for (auto& lane : automation_lanes) {
                if (lane.target_node_id == node_id && lane.param_index == param_index) {
                    for (auto it = lane.points.begin(); it != lane.points.end(); ++it) {
                        if (std::abs(it->time_sec - time_sec) < 0.05f) { // If close enough, overwrite
                            it->value = value;
                            it->time_sec = time_sec; // Update time precisely
                            return;
                        }
                        if (it->time_sec > time_sec) {
                            lane.points.insert(it, {time_sec, value});
                            return;
                        }
                    }
                    lane.points.push_back({time_sec, value});
                    return;
                }
            }
            AutomationLane new_lane;
            new_lane.target_node_id = node_id;
            new_lane.param_index = param_index;
            new_lane.points.push_back({time_sec, value});
            automation_lanes.push_back(new_lane);
        }
        
        void removeAutomationPoint(const std::string& node_id, int param_index, float time_sec) {
            std::lock_guard<std::mutex> lock(timeline_mutex);
            for (auto& lane : automation_lanes) {
                if (lane.target_node_id == node_id && lane.param_index == param_index) {
                    for (auto it = lane.points.begin(); it != lane.points.end(); ++it) {
                        if (std::abs(it->time_sec - time_sec) < 0.05f) {
                            lane.points.erase(it);
                            return;
                        }
                    }
                }
            }
        }
        
        ClipManager* clip_manager = nullptr;
        void setClipManager(ClipManager* cm) { clip_manager = cm; }
        
        TimelineManager() {
            for(int i=0; i<16; i++) seq_grid[i] = false;
        }

        void setPlaying(bool play) {
            is_playing = play;
            if (!play) {
                master_frame = 0; // Stop rewinds to 0
                current_step = 0;
                // Envia Note Off para tudo! (Será tratado no main)
            }
        }
        
        bool getPlaying() const { return is_playing; }
        uint64_t getMasterFrame() const { return master_frame; }
        
        void addNote(int track_index, int pitch, float start, float duration, float velocity = 0.8f, float probability = 1.0f) {
            // Deprecated, notes are now in Patterns
        }

        void clearNotes(int track_index) {
            // Deprecated
        }

        struct AutomationEvent {
            std::string target_node_id;
            int param_index;
            float value;
        };

        // Retorna Tuple de (MidiEvents, AutomationEvents)
        // Midi Event: track_idx, pitch, duration, velocity
        std::tuple<std::vector<std::tuple<int, int, float, float>>, std::vector<AutomationEvent>> processBlock(unsigned int frames, unsigned int& out_offset_frames) {
            std::vector<std::tuple<int, int, float, float>> fired_events;
            std::vector<AutomationEvent> auto_events;
            if (!is_playing) return {fired_events, auto_events};
            
            double frames_per_beat = (60.0 / bpm) * sample_rate;
            double frames_per_step = frames_per_beat / 4.0;
            float snap_step = (60.0f / bpm) / 4.0f;
            
            uint64_t frame_start = master_frame;
            uint64_t frame_end = master_frame + frames;
            master_frame += frames;
            
            int step_start = (int)(frame_start / frames_per_step) % 16;
            int step_end = (int)(frame_end / frames_per_step) % 16;
            current_step.store(step_start, std::memory_order_relaxed);
            
            if (step_start != step_end) {
                uint64_t absolute_next_step = (uint64_t)(frame_start / frames_per_step) + 1;
                uint64_t next_step_frame = (uint64_t)(absolute_next_step * frames_per_step);
                out_offset_frames = (unsigned int)(next_step_frame - frame_start);
                
                if (seq_grid[step_end].load(std::memory_order_relaxed)) {
                    // Duração fixa de 0.2s para o step sequencer (Bateria)
                    fired_events.push_back({0, 60, 0.2f, 1.0f});
                }
            }
            if (frame_start == 0 && seq_grid[0].load(std::memory_order_relaxed)) {
                out_offset_frames = 0;
                fired_events.push_back({0, 60, 0.2f, 1.0f});
            }
            
            float t_start = (float)frame_start / sample_rate;
            float t_end = (float)frame_end / sample_rate;
            
            std::lock_guard<std::mutex> lock(timeline_mutex);
            if (clip_manager) {
                std::lock_guard<std::mutex> clip_lock(clip_manager->clip_mutex);
                
                // Count total MIDI clips on the timeline
                size_t total_clips = 0;
                for (int i = 0; i < MAX_TRACKS; i++) {
                    total_clips += clip_manager->track_midi_clips[i].size();
                }
                
                if (total_clips == 0) {
                    // --- PATTERN MODE (Poly-looping: Drums loop per-bar, Piano Roll notes span multiple bars) ---
                    if (clip_manager->current_pattern_idx >= 0 && clip_manager->current_pattern_idx < (int)clip_manager->global_patterns.size()) {
                        auto& pat = clip_manager->global_patterns[clip_manager->current_pattern_idx];
                        float beat_len = (60.0f / bpm);
                        
                        // 1. Process per-channel notes (0..7) with individual channel loop lengths
                        for (int c = 0; c < 8; c++) {
                            auto& ch_notes = pat.getChannelNotes(c);
                            
                            float max_ch_time = track_steps_limit[c] * snap_step;
                            for (const auto& note : ch_notes) {
                                if (note.start_time + note.duration > max_ch_time) {
                                    max_ch_time = note.start_time + note.duration;
                                }
                            }
                            
                            float total_ch_beats = std::ceil(max_ch_time / beat_len);
                            if (total_ch_beats < 4.0f) total_ch_beats = 4.0f;
                            float ch_loop_beats = std::ceil(total_ch_beats / 4.0f) * 4.0f;
                            float ch_loop_len = ch_loop_beats * beat_len;
                            
                            for (auto& note : ch_notes) {
                                if (note.is_muted) continue;
                                
                                float loop_t_start = fmodf(t_start, ch_loop_len);
                                float loop_t_end = loop_t_start + (t_end - t_start);
                                bool wrapped = (loop_t_end > ch_loop_len);
                                
                                bool trigger = false;
                                if (!wrapped) {
                                    if (note.start_time >= loop_t_start && note.start_time < loop_t_end) {
                                        trigger = true;
                                    }
                                } else {
                                    note.is_playing = false;
                                    if ((note.start_time >= loop_t_start && note.start_time < ch_loop_len) ||
                                        (note.start_time >= 0.0f && note.start_time < loop_t_end - ch_loop_len)) {
                                        trigger = true;
                                    }
                                }
                                
                                if (trigger && note.is_playing) {
                                    trigger = false;
                                }
                                
                                if (trigger) {
                                    note.is_playing = true;
                                    float rand_val = (float)rand() / RAND_MAX;
                                    if (rand_val <= note.probability) {
                                        fired_events.push_back({c, note.pitch, note.duration, note.velocity});
                                    }
                                }
                            }
                        }
                        
                        // 2. Process legacy pattern notes
                        if (!pat.notes.empty()) {
                            float max_legacy_time = 0.0f;
                            for (const auto& note : pat.notes) {
                                if (note.start_time + note.duration > max_legacy_time) {
                                    max_legacy_time = note.start_time + note.duration;
                                }
                            }
                            float total_leg_beats = std::ceil(max_legacy_time / beat_len);
                            if (total_leg_beats < 4.0f) total_leg_beats = 4.0f;
                            float pat_loop_len = std::ceil(total_leg_beats / 4.0f) * 4.0f * beat_len;
                            
                            for (auto& note : pat.notes) {
                                if (note.is_muted) continue;
                                
                                float loop_t_start = fmodf(t_start, pat_loop_len);
                                float loop_t_end = loop_t_start + (t_end - t_start);
                                bool wrapped = (loop_t_end > pat_loop_len);
                                
                                bool trigger = false;
                                if (!wrapped) {
                                    if (note.start_time >= loop_t_start && note.start_time < loop_t_end) {
                                        trigger = true;
                                    }
                                } else {
                                    note.is_playing = false;
                                    if ((note.start_time >= loop_t_start && note.start_time < pat_loop_len) ||
                                        (note.start_time >= 0.0f && note.start_time < loop_t_end - pat_loop_len)) {
                                        trigger = true;
                                    }
                                }
                                
                                if (trigger && note.is_playing) {
                                    trigger = false;
                                }
                                
                                if (trigger) {
                                    note.is_playing = true;
                                    int target_ch = 0;
                                    if (note.pitch == 36) target_ch = 0;
                                    else if (note.pitch == 38) target_ch = 1;
                                    else if (note.pitch == 42) target_ch = 2;
                                    else if (note.pitch == 48) target_ch = 3;
                                    else if (note.pitch == 60) target_ch = 4;
                                    else if (note.pitch == 72) target_ch = 5;
                                    else if (note.pitch == 39) target_ch = 6;
                                    else if (note.pitch == 46) target_ch = 7;
                                    else target_ch = 5;
                                    
                                    float rand_val = (float)rand() / RAND_MAX;
                                    if (rand_val <= note.probability) {
                                        fired_events.push_back({target_ch, note.pitch, note.duration, note.velocity});
                                    }
                                }
                            }
                        }
                    }
                } else {
                    // --- SONG MODE (Play from timeline MIDI clips) ---
                    for (int i = 0; i < MAX_TRACKS; i++) {
                        for (auto& clip : clip_manager->track_midi_clips[i]) {
                            float clip_t_start = t_start - clip.start_time_sec;
                            float clip_t_end = t_end - clip.start_time_sec;
                            
                            if (t_end >= clip.start_time_sec && t_start <= clip.start_time_sec + clip.length_sec) {
                                Pattern* p = nullptr;
                                for (auto& pat : clip_manager->global_patterns) {
                                    if (pat.id == clip.pattern_id) { p = &pat; break; }
                                }
                                
                                if (p) {
                                    for (int c = 0; c < 8; c++) {
                                        auto& ch_notes = p->getChannelNotes(c);
                                        float note_loop_len = track_steps_limit[c] * snap_step;
                                        if (note_loop_len <= 0.001f) note_loop_len = clip.length_sec;
                                        
                                        float local_t_start = fmodf(clip_t_start, note_loop_len);
                                        float local_t_end = local_t_start + (t_end - t_start);
                                        bool local_wrapped = (local_t_end > note_loop_len);
                                        
                                        for (auto& note : ch_notes) {
                                            if (note.is_muted) continue;
                                            
                                            bool note_trigger = false;
                                            if (!local_wrapped) {
                                                if (note.start_time >= local_t_start && note.start_time < local_t_end) note_trigger = true;
                                            } else {
                                                note.is_playing = false;
                                                if ((note.start_time >= local_t_start && note.start_time < note_loop_len) ||
                                                    (note.start_time >= 0.0f && note.start_time < local_t_end - note_loop_len)) note_trigger = true;
                                            }
                                            
                                            if (note_trigger && note.is_playing) note_trigger = false;
                                            
                                            if (note_trigger) {
                                                note.is_playing = true;
                                                float rand_val = (float)rand() / RAND_MAX;
                                                if (rand_val <= note.probability) {
                                                    fired_events.push_back({c, note.pitch, note.duration, note.velocity});
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // FASE 10: Processa e gera os eventos de automação
            for (const auto& lane : automation_lanes) {
                if (!lane.points.empty()) {
                    auto_events.push_back({lane.target_node_id, lane.param_index, lane.getValueAtTime(t_start)});
                }
            }
            
            return {fired_events, auto_events};
        }
    };
}
