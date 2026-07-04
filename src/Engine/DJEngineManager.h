#pragma once

#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>
#include "EngineManager.h"

namespace te = tracktion;

namespace nova
{

class DJEngineManager
{
public:
    explicit DJEngineManager(EngineManager& engineManager);
    ~DJEngineManager();

    /// Load an audio file into a specific deck (0 = Deck A, 1 = Deck B)
    bool loadTrackToDeck(int deckIndex, const juce::File& audioFile);

    /// Transport controls for individual decks
    void playDeck(int deckIndex);
    void pauseDeck(int deckIndex);
    void stopDeck(int deckIndex);

    /// Set volume for a deck (0.0 to 1.0)
    void setDeckVolume(int deckIndex, float volume);

    /// Set pitch/speed for a deck (-1.0 to 1.0, where 0 is original speed)
    void setDeckPitch(int deckIndex, float pitchOffset);

    /// Set crossfader position (-1.0 = Deck A only, 1.0 = Deck B only)
    void setCrossfader(float position);

    te::Edit* getDeckEdit(int deckIndex) { return deckIndex == 0 ? editA.get() : editB.get(); }

private:
    EngineManager& engineMgr;

    std::unique_ptr<te::Edit> editA;
    std::unique_ptr<te::Edit> editB;

    // We keep track of the base volume to apply crossfader math
    float volA = 0.8f;
    float volB = 0.8f;
    float crossfaderPos = 0.0f;

    void updateVolumes();
    std::unique_ptr<te::Edit> createDeckEdit();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DJEngineManager)
};

} // namespace nova
