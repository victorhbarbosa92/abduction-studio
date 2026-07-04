#pragma once

// ─── MixerView ──────────────────────────────────────────────────────────────
// Bottom panel mixer with channel strips for each track.
// Each strip has: fader, pan knob, mute/solo, VU meter.

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include "Utils/Constants.h"

namespace te = tracktion;

namespace nova
{

// ─── Channel Strip (one per track) ──────────────────────────────────────────

class ChannelStrip : public juce::Component,
                     public juce::Timer
{
public:
    ChannelStrip(te::AudioTrack& track, juce::Colour trackColour);
    ~ChannelStrip() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    te::AudioTrack& track;
    juce::Colour colour;

    juce::Label nameLabel;
    juce::TextButton muteBtn { "M" };
    juce::TextButton soloBtn { "S" };
    juce::Slider faderSlider;
    juce::Slider panKnob;
    juce::Label dbLabel;

    float currentLevel = 0.0f;
    float peakLevel    = 0.0f;

    void setupComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStrip)
};

// ─── MixerView (container for all strips) ───────────────────────────────────

class MixerView : public juce::Component
{
public:
    explicit MixerView(te::Edit& edit);
    ~MixerView() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    /// Rebuild channel strips when tracks change
    void refreshStrips();

private:
    te::Edit& edit;
    juce::OwnedArray<ChannelStrip> strips;
    juce::Viewport viewport;
    juce::Component stripContainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerView)
};

} // namespace nova
