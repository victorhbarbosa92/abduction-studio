#pragma once

namespace KuroDSP {
    struct MidiNote {
        int pitch;          // MIDI pitch (0-127)
        float start_time;   // Start time in seconds (relative to clip or absolute for scratchpad)
        float duration;     // Duration in seconds
        float velocity;     // 0.0 to 1.0
        bool is_playing;
        float probability;  // Chance: 0.0 to 1.0
        bool is_muted;
        bool is_selected;
        
        MidiNote() : pitch(60), start_time(0.0f), duration(1.0f), velocity(0.8f), is_playing(false), probability(1.0f), is_muted(false), is_selected(false) {}
        MidiNote(int p, float st, float dur, float vel = 0.8f, float prob = 1.0f) 
            : pitch(p), start_time(st), duration(dur), velocity(vel), is_playing(false), probability(prob), is_muted(false), is_selected(false) {}
    };
}
