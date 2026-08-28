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

extern int channel_tracks[MAX_TRACKS];

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
        
        // FASE 10 & V4: ADVANCED BEZIER AUTOMATION LANES
        struct AutoPoint {
            float time_sec;
            float value;
            float tension = 0.0f; // -1.0f (côncavo/rápido) a +1.0f (convexo/lento), 0.0 = linear/suave
        };
        struct AutomationLane {
            std::string target_node_id;
            int param_index;
            std::vector<AutoPoint> points; // Mantido em ordem cronológica
            
            // Avaliação de curva de Bézier / Tensão no estilo FL Studio
            float getValueAtTime(float t) const {
                if (points.empty()) return 0.0f;
                if (t <= points.front().time_sec) return points.front().value;
                if (t >= points.back().time_sec) return points.back().value;
                
                for (size_t i = 0; i < points.size() - 1; ++i) {
                    if (t >= points[i].time_sec && t < points[i+1].time_sec) {
                        float range = points[i+1].time_sec - points[i].time_sec;
                        if (range <= 0.00001f) return points[i].value;
                        float progress = (t - points[i].time_sec) / range;
                        
                        // Curvatura com base na Tensão (Tension parameter)
                        float tension = points[i].tension;
                        float curved_progress = progress;
                        if (std::abs(tension) > 0.001f) {
                            if (tension > 0.0f) {
                                // Convexo (sobe devagar no início e acelera no final)
                                float p = 1.0f + tension * 4.0f;
                                curved_progress = std::pow(progress, p);
                            } else {
                                // Côncavo (sobe rápido no início e desacelera)
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
        
        struct SectionMarker {
            float time_sec;
            std::string name;
            uint32_t color = 0xFF00E5FF;
        };
        std::vector<SectionMarker> section_markers;

        bool loop_enabled = false;
        float loop_start_sec = 0.0f;
        float loop_end_sec = 32.0f;
        bool is_pattern_mode = true;

        void addSectionMarker(float time_sec, const std::string& name, uint32_t color = 0xFF00E5FF) {
            std::lock_guard<std::mutex> lock(timeline_mutex);
            section_markers.push_back({time_sec, name, color});
            std::sort(section_markers.begin(), section_markers.end(), [](const SectionMarker& a, const SectionMarker& b) {
                return a.time_sec < b.time_sec;
            });
        }

        void removeSectionMarker(size_t idx) {
            std::lock_guard<std::mutex> lock(timeline_mutex);
            if (idx < section_markers.size()) section_markers.erase(section_markers.begin() + idx);
        }

        void setLoopRegion(bool enabled, float start_sec, float end_sec) {
            std::lock_guard<std::mutex> lock(timeline_mutex);
            loop_enabled = enabled;
            loop_start_sec = (std::max)(0.0f, start_sec);
            loop_end_sec = (std::max)(loop_start_sec + 1.0f, end_sec);
        }
        
        ClipManager* clip_manager = nullptr;
        void setClipManager(ClipManager* cm) { clip_manager = cm; }
        
        TimelineManager() {
            for(int i=0; i<16; i++) seq_grid[i] = false;
            // Marcadores de seção padrão para estrutura musical de referência (128 BPM)
            section_markers.push_back({0.0f, "INTRO", 0xFF00E5FF});
            section_markers.push_back({15.0f, "BUILD-UP 1", 0xFFFF9900});
            section_markers.push_back({30.0f, "DROP 1", 0xFFFF007F});
            section_markers.push_back({60.0f, "BREAKDOWN", 0xFF9900FF});
            section_markers.push_back({75.0f, "BUILD-UP 2", 0xFFFFFF00});
            section_markers.push_back({90.0f, "DROP 2 (CLIMAX)", 0xFFFF0055});
            section_markers.push_back({120.0f, "OUTRO", 0xFF00CC88});
        }

        void setPlaying(bool play) {
            is_playing = play;
            if (!play) {
                master_frame = 0; // Stop rewinds to 0
                current_step = 0;
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
            
            // Suporte a Loop Region
            if (loop_enabled && loop_end_sec > loop_start_sec) {
                uint64_t loop_end_frame = (uint64_t)(loop_end_sec * sample_rate);
                uint64_t loop_start_frame = (uint64_t)(loop_start_sec * sample_rate);
                if (master_frame >= loop_end_frame) {
                    master_frame = loop_start_frame;
                }
            }

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
            }
            if (frame_start == 0) {
                out_offset_frames = 0;
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
                
                if (is_pattern_mode || total_clips == 0) {
                    // --- PATTERN MODE (Poly-looping: Drums loop per-bar, Piano Roll notes span multiple bars) ---
                    if (clip_manager->current_pattern_idx >= 0 && clip_manager->current_pattern_idx < (int)clip_manager->global_patterns.size()) {
                        auto& pat = clip_manager->global_patterns[clip_manager->current_pattern_idx];
                        float beat_len = (60.0f / bpm);
                        
                        extern bool track_mutes[MAX_TRACKS];
                        extern bool track_solos[MAX_TRACKS];
                        bool any_solo = false;
                        for (int s = 0; s < MAX_TRACKS; s++) {
                            if (track_solos[s]) { any_solo = true; break; }
                        }

                        // 1. Process per-channel notes (0..7) with individual channel loop lengths
                        for (int c = 0; c < MAX_TRACKS; c++) {
                            if (track_mutes[c] || (any_solo && !track_solos[c])) continue;
                            auto& ch_notes = pat.getChannelNotes(c);
                            
                            float max_ch_time = track_steps_limit[c] * snap_step;
                            for (const auto& note : ch_notes) {
                                if (note.start_time + note.duration > max_ch_time) {
                                    max_ch_time = note.start_time + note.duration;
                                }
                            }
                            
                            if (max_ch_time > 15.0f) {
                                // Linear Full Song Timeline Mode: Disparo direto e 100% sincronizado com o áudio
                                for (auto& note : ch_notes) {
                                    if (note.is_muted) continue;
                                    if (note.start_time >= t_start && note.start_time < t_end) {
                                        float rand_val = (float)rand() / RAND_MAX;
                                        if (rand_val <= note.probability) {
                                            fired_events.push_back({c, note.pitch, note.duration, note.velocity});
                                        }
                                    }
                                }
                            } else {
                                // Pattern Loop Mode (1-4 bars Step Sequencer)
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
                        }
                    }
                } else {
                    // --- SONG MODE (Play from timeline MIDI clips) ---
                    extern bool track_mutes[MAX_TRACKS];
                    extern bool track_solos[MAX_TRACKS];
                    bool any_solo = false;
                    for (int s = 0; s < MAX_TRACKS; s++) {
                        if (track_solos[s]) { any_solo = true; break; }
                    }

                    for (int i = 0; i < MAX_TRACKS; i++) {
                        if (track_mutes[i] || (any_solo && !track_solos[i])) continue;

                        for (auto& clip : clip_manager->track_midi_clips[i]) {
                            if (clip.is_muted) continue;
                            float clip_t_start = t_start - clip.start_time_sec;
                            float clip_t_end = t_end - clip.start_time_sec;
                            
                            if (t_end >= clip.start_time_sec && t_start <= clip.start_time_sec + clip.length_sec) {
                                Pattern* p = nullptr;
                                for (auto& pat : clip_manager->global_patterns) {
                                    if (pat.id == clip.pattern_id) { p = &pat; break; }
                                }
                                
                                if (p) {
                                    int c = i;
                                    if (p->getChannelNotes(c).empty() && (i % 2 == 1)) {
                                        c = i / 2; // Mapeia trilha MIDI ímpar (1, 3, 5, 7) para o canal de synth correspondente
                                    }
                                    if (c >= 0 && c < MAX_TRACKS) {
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
