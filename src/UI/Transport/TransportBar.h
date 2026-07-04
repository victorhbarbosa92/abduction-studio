#pragma once

// ─── TransportBar ───────────────────────────────────────────────────────────
// The top bar containing transport controls (play, stop, record),
// position display, BPM control, and loop toggle.

#include <juce_gui_basics/juce_gui_basics.h>
#include "ViewModel/TransportViewModel.h"
#include "Utils/Constants.h"
#include "Utils/Helpers.h"

namespace nova
{

class TransportBar : public juce::Component,
                     public TransportViewModel::Listener
{
public:
    explicit TransportBar(TransportViewModel& transportVM);
    ~TransportBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // TransportViewModel::Listener
    void transportStateChanged() override;
    void transportPositionChanged(double positionSeconds) override;

    std::function<void()> onDJModeToggled;

private:
    TransportViewModel& vm;

    // ─── Buttons ────────────────────────────────────────────────────────
    juce::TextButton returnToStartBtn { "|<" };
    juce::TextButton stopBtn          { "PARAR" };
    juce::TextButton playBtn          { "TOCAR" };
    juce::TextButton recordBtn        { "GRAVAR" };
    juce::TextButton loopBtn          { "LOOP" };
    juce::TextButton djModeBtn        { "MODO DJ" };

    // ─── Position Display ───────────────────────────────────────────────
    juce::Label positionLabel;
    juce::Label barsBeatsLabel;

    // ─── BPM ────────────────────────────────────────────────────────────
    juce::Label bpmLabel;
    juce::Slider bpmSlider;

    // ─── Time Signature Display ─────────────────────────────────────────
    juce::Label timeSigLabel;

    void updatePositionDisplay(double seconds);
    void updateTransportState();
    void setupButtons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};

} // namespace nova
