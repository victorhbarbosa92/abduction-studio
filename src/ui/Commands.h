#pragma once
#include "CommandManager.h"
#include "../core/TimelineManager.h"

namespace KuroUI {

    class AddNoteCommand : public ICommand {
    private:
        KuroDSP::TimelineManager* timeline;
        int track_index;
        KuroDSP::MidiNote note;

    public:
        AddNoteCommand(KuroDSP::TimelineManager* t, int trk_idx, KuroDSP::MidiNote n)
            : timeline(t), track_index(trk_idx), note(n) {}

        void execute() override {
            timeline->track_notes[track_index].push_back(note);
            std::cout << "[AddNoteCommand] Nota " << note.pitch << " adicionada.\n";
        }

        void undo() override {
            auto& notes = timeline->track_notes[track_index];
            // Remove a última nota que seja idêntica a essa
            for (auto it = notes.rbegin(); it != notes.rend(); ++it) {
                if (it->pitch == note.pitch && it->start_time == note.start_time) {
                    notes.erase(std::next(it).base());
                    std::cout << "[AddNoteCommand] Nota " << note.pitch << " desfeita.\n";
                    break;
                }
            }
        }
    };

}
