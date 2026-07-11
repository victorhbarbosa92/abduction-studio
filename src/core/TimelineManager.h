#pragma once
#include <atomic>
#include <vector>
#include <mutex>
#include <tuple>
#include "KuroConfig.h"

namespace KuroDSP {

    struct MidiNote {
        int pitch;          // MIDI pitch (0-127)
        float start_time;   // Start time in seconds
        float duration;     // Duration in seconds
        float velocity;     // 0.0 to 1.0
        bool is_playing;
        
        MidiNote() : pitch(60), start_time(0.0f), duration(1.0f), velocity(0.8f), is_playing(false) {}
        MidiNote(int p, float st, float dur, float vel = 0.8f) 
            : pitch(p), start_time(st), duration(dur), velocity(vel), is_playing(false) {}
    };

    class TimelineManager {
    private:
        float bpm = 140.0f;
        unsigned int sample_rate = 44100;
        
        std::atomic<uint64_t> master_frame = {0};
        bool is_playing = false;

    public:
        float getBPM() const { return bpm; }
        void setBPM(float b) { bpm = b; }
        
        void setMasterFrame(uint64_t frame) { master_frame.store(frame, std::memory_order_relaxed); }
        // 16 passos (1 Compass = 4 tempos = 16 semicolcheias)
        std::atomic<bool> seq_grid[16];
        std::atomic<int> current_step = {0};
        
        // Piano Roll Notes (Uma lista por canal)
        std::vector<MidiNote> track_notes[MAX_TRACKS];
        std::mutex timeline_mutex;
        
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
        
        void addNote(int track_index, int pitch, float start, float duration, float velocity = 0.8f) {
            if (track_index < 0 || track_index >= 8) return;
            std::lock_guard<std::mutex> lock(timeline_mutex);
            track_notes[track_index].push_back(MidiNote(pitch, start, duration, velocity));
        }

        void clearNotes(int track_index) {
            if (track_index < 0 || track_index >= 8) return;
            std::lock_guard<std::mutex> lock(timeline_mutex);
            track_notes[track_index].clear();
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
            for (int i = 0; i < MAX_TRACKS; i++) {
                for (auto& note : track_notes[i]) {
                    if (!note.is_playing && t_end >= note.start_time && t_start < note.start_time + note.duration) {
                        note.is_playing = true;
                        fired_events.push_back({i, note.pitch, note.duration, note.velocity});
                    }
                    else if (note.is_playing && t_end >= note.start_time + note.duration) {
                        note.is_playing = false;
                        // Não precisamos enviar Note Off, pois a synth engine lida com a duração internamente.
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
