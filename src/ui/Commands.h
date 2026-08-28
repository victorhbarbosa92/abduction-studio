#pragma once
#include "CommandManager.h"
#include "../core/TimelineManager.h"
#include "../core/ClipManager.h"

// ─── Externals shared between audio thread and UI thread ────────────────────
extern bool is_playing;
extern unsigned long long global_sample_count;
extern ClipManager g_clip_manager;
extern void clear_all_synths();
#include "../plugin_manager/KuroSamplerNode.h"
extern std::shared_ptr<KuroDSP::KuroSamplerNode> g_global_sampler;

namespace KuroUI {

    class AddNoteCommand : public ICommand {
    private:
        std::vector<KuroDSP::MidiNote>* notes_ptr;
        KuroDSP::MidiNote note;

    public:
        AddNoteCommand(std::vector<KuroDSP::MidiNote>* n_ptr, KuroDSP::MidiNote n)
            : notes_ptr(n_ptr), note(n) {}

        void execute() override {
            notes_ptr->push_back(note);
        }

        void undo() override {
            for (auto it = notes_ptr->rbegin(); it != notes_ptr->rend(); ++it) {
                if (it->pitch == note.pitch && it->start_time == note.start_time) {
                    notes_ptr->erase(std::next(it).base());
                    break;
                }
            }
        }
    };

    // ── MODULAR TRANSPORT CONTROLLER ─────────────────────────────────────────
    // Single source of truth for play/stop/pause.
    // Used by: top transport bar, keyboard shortcut, menu, and MIDI clock.
    // ─────────────────────────────────────────────────────────────────────────
    class TransportController {
    public:
        static void Play(KuroDSP::TimelineManager& timeline) {
            if (!is_playing) {
                is_playing = true;
                timeline.setPlaying(true);
            }
        }

        static void Stop(KuroDSP::TimelineManager& timeline) {
            is_playing = false;
            timeline.setPlaying(false);
            clear_all_synths();
            if (g_global_sampler) {
                g_global_sampler->stop();
            }
            // Reset playhead to beat 1
            timeline.setMasterFrame(0);
            global_sample_count = 0;
            // Reset note.is_playing flags so pattern re-triggers on next Play
            {
                std::lock_guard<std::mutex> lock(g_clip_manager.clip_mutex);
                for (auto& pat : g_clip_manager.global_patterns) {
                    for (int c = 0; c < 8; c++) {
                        for (auto& n : pat.getChannelNotes(c)) {
                            n.is_playing = false;
                        }
                    }
                }
            }
        }

        static void TogglePlayPause(KuroDSP::TimelineManager& timeline) {
            if (is_playing) {
                // Pause (keep playhead position)
                is_playing = false;
                timeline.setPlaying(false);
                clear_all_synths();
                if (g_global_sampler) {
                    g_global_sampler->pause();
                }
            } else {
                Play(timeline);
            }
        }

        static void Restart(KuroDSP::TimelineManager& timeline) {
            Stop(timeline);
            Play(timeline);
        }
    };

}
