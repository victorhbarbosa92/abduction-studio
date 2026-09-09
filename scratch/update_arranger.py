import os
import sys

print("Building enhanced KuroPlaylistUI.h and PsySongArranger.h...")

# 1. Update PsySongArranger.h to create real AudioClips alongside MidiClips
with open("src/core/PsySongArranger.h", "r", encoding="utf-8") as f:
    arranger_code = f.read()

# Let's inspect where clips are added in PsySongArranger.h
old_add_clip = """            auto addClip = [&](int track, int pat_id, float start_sec, float len_sec) {
                MidiClip mc;
                mc.id = cm.next_id++;
                mc.start_time_sec = start_sec;
                mc.length_sec = len_sec;
                mc.pattern_id = pat_id;
                mc.is_selected = false;
                mc.is_muted = false;
                cm.track_midi_clips[track].push_back(mc);
            };"""

new_add_clip = """            auto addClip = [&](int track, int pat_id, float start_sec, float len_sec) {
                MidiClip mc;
                mc.id = cm.next_id++;
                mc.start_time_sec = start_sec;
                mc.length_sec = len_sec;
                mc.pattern_id = pat_id;
                mc.is_selected = false;
                mc.is_muted = false;
                for (const auto& pat : cm.global_patterns) {
                    if (pat.id == pat_id) { mc.name = pat.name; break; }
                }
                if (mc.name.empty()) mc.name = "Pattern " + std::to_string(pat_id);
                cm.track_midi_clips[track].push_back(mc);
            };

            auto addAudioClip = [&](int track, const std::string& name, float start_sec, float len_sec, unsigned int col) {
                AudioClip ac;
                ac.id = cm.next_id++;
                ac.name = name;
                ac.start_time_sec = start_sec;
                ac.length_sec = len_sec;
                ac.source_offset_sec = 0.0f;
                ac.color = col;
                ac.is_selected = false;
                ac.is_muted = false;
                cm.track_clips[track].push_back(ac);
            };"""

assert old_add_clip in arranger_code, "old_add_clip not found in PsySongArranger.h"
arranger_code = arranger_code.replace(old_add_clip, new_add_clip)

# Now add AudioClips in Intro, Build-up, and Drop
old_intro_clips = """                    addClip(6, 6, bt, block_dur);   // Tribal Percussion
                    addClip(11, 11, bt, block_dur); // Dark Sub Drone
                    addClip(12, 12, bt, block_dur); // Mystic Strings
                    addClip(14, 14, bt, block_dur); // Vocal Mantra"""

new_intro_clips = """                    addClip(6, 6, bt, block_dur);   // Tribal Percussion
                    addClip(11, 11, bt, block_dur); // Dark Sub Drone
                    addClip(12, 12, bt, block_dur); // Mystic Strings
                    addClip(14, 14, bt, block_dur); // Vocal Mantra
                    addAudioClip(14, "PRYZMA_Vocal_Chant_142.wav", bt, block_dur, 0xFFFF5078);
                    addAudioClip(15, "PRYZMA_FX_Atmo_Sweep.wav", bt, block_dur, 0xFFFF3232);"""

assert old_intro_clips in arranger_code, "old_intro_clips not found"
arranger_code = arranger_code.replace(old_intro_clips, new_intro_clips)

# Also in Drop 1: add audio kick sample or audio preview
old_drop1_kick = """                    addClip(0, 1, bt, block_dur); // Psy Kick"""
new_drop1_kick = """                    addClip(0, 1, bt, block_dur); // Psy Kick
                    addAudioClip(0, "PRYZMA_Kick01_144BPM.wav", bt, block_dur, 0xFF00E5FF);"""
assert old_drop1_kick in arranger_code, "old_drop1_kick not found"
arranger_code = arranger_code.replace(old_drop1_kick, new_drop1_kick)

with open("src/core/PsySongArranger.h", "w", encoding="utf-8") as f:
    f.write(arranger_code)

print("PsySongArranger.h updated with AudioClips and Pattern names!")
