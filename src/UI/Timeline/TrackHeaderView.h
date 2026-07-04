#pragma once

// ─── TrackHeaderView ────────────────────────────────────────────────────────
// Left-side panel showing track name, mute/solo/record-arm buttons,
// and a small volume/pan control for each track.

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include "Utils/Constants.h"

namespace te = tracktion;

namespace nova
{

class TrackHeaderView : public juce::Component
{
public:
    explicit TrackHeaderView(te::AudioTrack& track, juce::Colour trackColour);
    ~TrackHeaderView() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    te::AudioTrack& getTrack() { return track; }

private:
    te::AudioTrack& track;
    juce::Colour colour;

    juce::Label nameLabel;
    juce::TextButton muteBtn  { "M" };
    juce::TextButton soloBtn  { "S" };
    juce::TextButton armBtn   { "R" };
    juce::Slider volumeSlider;
    juce::Slider panSlider;

    void setupComponents();
    void updateMuteSoloState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderView)
};

} // namespace nova
