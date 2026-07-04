#include "TransportBar.h"

namespace nova
{

TransportBar::TransportBar(TransportViewModel& transportVM)
    : vm(transportVM)
{
    setupButtons();
    vm.addListener(this);
    updateTransportState();
    updatePositionDisplay(0.0);
}

TransportBar::~TransportBar()
{
    vm.removeListener(this);
}

void TransportBar::setupButtons()
{
    // ─── Return to Start ────────────────────────────────────────────────
    addAndMakeVisible(returnToStartBtn);
    returnToStartBtn.onClick = [this] { vm.returnToStart(); };
    returnToStartBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHT);

    // ─── Stop ───────────────────────────────────────────────────────────
    addAndMakeVisible(stopBtn);
    stopBtn.onClick = [this] { vm.stop(); };
    stopBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHT);

    // ─── Play ───────────────────────────────────────────────────────────
    addAndMakeVisible(playBtn);
    playBtn.onClick = [this] { vm.togglePlayStop(); };
    playBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHT);

    // ─── Record ─────────────────────────────────────────────────────────
    addAndMakeVisible(recordBtn);
    recordBtn.onClick = [this] { vm.record(); };
    recordBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHT);

    // ─── Loop ───────────────────────────────────────────────────────────
    addAndMakeVisible(loopBtn);
    loopBtn.setClickingTogglesState(true);
    loopBtn.onClick = [this] { vm.toggleLoop(); };
    loopBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHT);
    loopBtn.setColour(juce::TextButton::buttonOnColourId, colors::LOOP_YELLOW.withAlpha(0.3f));

    // ─── DJ Mode ─────────────────────────────────────────────────────────
    addAndMakeVisible(djModeBtn);
    djModeBtn.setClickingTogglesState(true);
    djModeBtn.onClick = [this] { if (onDJModeToggled) onDJModeToggled(); };
    djModeBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHT);
    djModeBtn.setColour(juce::TextButton::buttonOnColourId, colors::ACCENT_SECONDARY.withAlpha(0.3f));
    djModeBtn.setColour(juce::TextButton::textColourOffId, colors::ACCENT_SECONDARY);
    djModeBtn.setColour(juce::TextButton::textColourOnId, colors::ACCENT_SECONDARY);

    // ─── Position Labels ────────────────────────────────────────────────
    addAndMakeVisible(positionLabel);
    positionLabel.setFont(juce::Font(juce::FontOptions().withHeight(18.0f)));
    positionLabel.setColour(juce::Label::textColourId, colors::TEXT_PRIMARY);
    positionLabel.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(barsBeatsLabel);
    barsBeatsLabel.setFont(juce::Font(juce::FontOptions().withHeight(12.0f)));
    barsBeatsLabel.setColour(juce::Label::textColourId, colors::TEXT_SECONDARY);
    barsBeatsLabel.setJustificationType(juce::Justification::centredRight);

    // ─── BPM ────────────────────────────────────────────────────────────
    addAndMakeVisible(bpmLabel);
    bpmLabel.setText("BPM", juce::dontSendNotification);
    bpmLabel.setFont(juce::Font(juce::FontOptions().withHeight(11.0f)));
    bpmLabel.setColour(juce::Label::textColourId, colors::TEXT_MUTED);
    bpmLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(bpmSlider);
    bpmSlider.setSliderStyle(juce::Slider::LinearBar);
    bpmSlider.setRange(20.0, 999.0, 0.1);
    bpmSlider.setValue(vm.getBPM(), juce::dontSendNotification);
    bpmSlider.setTextValueSuffix("");
    bpmSlider.setColour(juce::Slider::backgroundColourId, colors::BG_DARK);
    bpmSlider.setColour(juce::Slider::trackColourId, colors::ACCENT_PRIMARY.withAlpha(0.3f));
    bpmSlider.setColour(juce::Slider::textBoxTextColourId, colors::ACCENT_SECONDARY);
    bpmSlider.onValueChange = [this] { vm.setBPM(bpmSlider.getValue()); };

    // ─── Time Signature ─────────────────────────────────────────────────
    addAndMakeVisible(timeSigLabel);
    timeSigLabel.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));
    timeSigLabel.setColour(juce::Label::textColourId, colors::TEXT_SECONDARY);
    timeSigLabel.setJustificationType(juce::Justification::centred);
    timeSigLabel.setText(juce::String(vm.getTimeSigNumerator()) + "/" + juce::String(vm.getTimeSigDenominator()),
                         juce::dontSendNotification);
}

void TransportBar::paint(juce::Graphics& g)
{
    // Background with subtle gradient
    g.setGradientFill(juce::ColourGradient(
        colors::BG_DARK.brighter(0.03f), 0.0f, 0.0f,
        colors::BG_DARK,                  0.0f, static_cast<float>(getHeight()),
        false));
    g.fillRect(getLocalBounds());

    // Bottom border line
    g.setColour(colors::BORDER);
    g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));
}

void TransportBar::resized()
{
    auto area = getLocalBounds().reduced(8, 6);

    int btnSize = 42;
    int btnGap  = 4;

    // ─── Left: Transport Buttons ────────────────────────────────────────
    auto leftArea = area.removeFromLeft(6 * (btnSize + btnGap) + 30);

    returnToStartBtn.setBounds(leftArea.removeFromLeft(btnSize).reduced(0, 2));
    leftArea.removeFromLeft(btnGap);

    stopBtn.setBounds(leftArea.removeFromLeft(btnSize).reduced(0, 2));
    leftArea.removeFromLeft(btnGap);

    playBtn.setBounds(leftArea.removeFromLeft(btnSize + 10).reduced(0, 2));
    leftArea.removeFromLeft(btnGap);

    recordBtn.setBounds(leftArea.removeFromLeft(btnSize).reduced(0, 2));
    leftArea.removeFromLeft(btnGap);

    loopBtn.setBounds(leftArea.removeFromLeft(btnSize).reduced(0, 2));
    leftArea.removeFromLeft(btnGap);
    
    djModeBtn.setBounds(leftArea.removeFromLeft(btnSize + 20).reduced(0, 2));

    // ─── Right: BPM + Time Signature ────────────────────────────────────
    auto rightArea = area.removeFromRight(180);

    auto bpmArea = rightArea.removeFromLeft(90);
    bpmLabel.setBounds(bpmArea.removeFromTop(14));
    bpmSlider.setBounds(bpmArea.reduced(2, 4));

    rightArea.removeFromLeft(10);
    timeSigLabel.setBounds(rightArea);

    // ─── Center: Position Display ───────────────────────────────────────
    area.removeFromLeft(20);
    area.removeFromRight(20);

    auto posArea = area.withSizeKeepingCentre(200, area.getHeight());
    positionLabel.setBounds(posArea.removeFromTop(posArea.getHeight() / 2 + 4));
    barsBeatsLabel.setBounds(posArea);
}

// ─── Listener Callbacks ─────────────────────────────────────────────────────

void TransportBar::transportStateChanged()
{
    updateTransportState();
}

void TransportBar::transportPositionChanged(double positionSeconds)
{
    updatePositionDisplay(positionSeconds);
}

void TransportBar::updatePositionDisplay(double seconds)
{
    positionLabel.setText(helpers::formatTimeMMSS(seconds), juce::dontSendNotification);
    barsBeatsLabel.setText(helpers::formatTimeBarsBeats(seconds, vm.getBPM(),
                                                        vm.getTimeSigNumerator(),
                                                        vm.getTimeSigDenominator()),
                           juce::dontSendNotification);
}

void TransportBar::updateTransportState()
{
    // Update button colors based on state
    if (vm.isPlaying())
    {
        playBtn.setColour(juce::TextButton::buttonColourId, colors::PLAY_GREEN.withAlpha(0.3f));
        playBtn.setButtonText("||");  // Pause icon
    }
    else
    {
        playBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHT);
        playBtn.setButtonText("TOCAR");
    }

    if (vm.isRecording())
    {
        recordBtn.setColour(juce::TextButton::buttonColourId, colors::RECORD_RED.withAlpha(0.3f));
    }
    else
    {
        recordBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHT);
    }

    loopBtn.setToggleState(vm.isLooping(), juce::dontSendNotification);

    repaint();
}

} // namespace nova
