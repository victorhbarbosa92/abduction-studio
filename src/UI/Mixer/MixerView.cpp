#include "MixerView.h"
#include "UI/LookAndFeel/NovaLookAndFeel.h"
#include "Utils/Helpers.h"

namespace nova
{

// ═══════════════════════════════════════════════════════════════════════════
//  ChannelStrip Implementation
// ═══════════════════════════════════════════════════════════════════════════

ChannelStrip::ChannelStrip(te::AudioTrack& t, juce::Colour trackColour)
    : track(t), colour(trackColour)
{
    setupComponents();
    startTimerHz(constants::TIMER_HZ_METERS);
}

ChannelStrip::~ChannelStrip()
{
    stopTimer();
}

void ChannelStrip::setupComponents()
{
    // ─── Track Name ─────────────────────────────────────────────────────
    addAndMakeVisible(nameLabel);
    nameLabel.setText(track.getName(), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(juce::FontOptions().withHeight(11.0f)));
    nameLabel.setColour(juce::Label::textColourId, colors::TEXT_PRIMARY);
    nameLabel.setJustificationType(juce::Justification::centred);

    // ─── Mute ───────────────────────────────────────────────────────────
    addAndMakeVisible(muteBtn);
    muteBtn.setClickingTogglesState(true);
    muteBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHTER);
    muteBtn.setColour(juce::TextButton::buttonOnColourId, colors::LOOP_YELLOW.withAlpha(0.5f));
    muteBtn.onClick = [this] { track.setMute(muteBtn.getToggleState()); };

    // ─── Solo ───────────────────────────────────────────────────────────
    addAndMakeVisible(soloBtn);
    soloBtn.setClickingTogglesState(true);
    soloBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHTER);
    soloBtn.setColour(juce::TextButton::buttonOnColourId, colors::ACCENT_SECONDARY.withAlpha(0.5f));
    soloBtn.onClick = [this] { track.setSolo(soloBtn.getToggleState()); };

    // ─── Fader (vertical slider) ────────────────────────────────────────
    addAndMakeVisible(faderSlider);
    faderSlider.setSliderStyle(juce::Slider::LinearVertical);
    faderSlider.setRange(0.0, 1.0, 0.001);
    faderSlider.setValue(0.75, juce::dontSendNotification); // ~-2.5 dB
    faderSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    faderSlider.onValueChange = [this] {
        float dB = helpers::sliderToDecibels(static_cast<float>(faderSlider.getValue()));
        track.getVolumePlugin()->volParam->setParameter(
            helpers::decibelsToGain(dB), juce::dontSendNotification);
    };

    // ─── Pan Knob ───────────────────────────────────────────────────────
    addAndMakeVisible(panKnob);
    panKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    panKnob.setRange(-1.0, 1.0, 0.01);
    panKnob.setValue(0.0, juce::dontSendNotification);
    panKnob.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    panKnob.onValueChange = [this] {
        track.getVolumePlugin()->panParam->setParameter(
            static_cast<float>((panKnob.getValue() + 1.0) * 0.5), juce::dontSendNotification);
    };

    // ─── dB Label ───────────────────────────────────────────────────────
    addAndMakeVisible(dbLabel);
    dbLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
    dbLabel.setColour(juce::Label::textColourId, colors::TEXT_MUTED);
    dbLabel.setJustificationType(juce::Justification::centred);
    dbLabel.setText("0.0 dB", juce::dontSendNotification);
}

void ChannelStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    g.setColour(colors::SURFACE);
    g.fillRoundedRectangle(bounds.toFloat().reduced(1.0f), 4.0f);

    // Top color bar
    g.setColour(colour);
    g.fillRoundedRectangle(bounds.toFloat().removeFromTop(3.0f).reduced(4.0f, 0.0f), 1.5f);

    // VU Meter bars (drawn to the right of the fader)
    auto faderBounds = faderSlider.getBounds();
    int meterX = faderBounds.getRight() + 4;
    int meterW = 6;
    int meterH = faderBounds.getHeight();
    int meterY = faderBounds.getY();

    // Left channel
    NovaLookAndFeel::drawMeterBar(g,
        juce::Rectangle<int>(meterX, meterY, meterW, meterH),
        currentLevel, true);

    // Right channel
    NovaLookAndFeel::drawMeterBar(g,
        juce::Rectangle<int>(meterX + meterW + 2, meterY, meterW, meterH),
        currentLevel * 0.95f, true); // Slight offset for visual interest

    // Peak indicator
    if (peakLevel > 0.95f)
    {
        g.setColour(colors::METER_RED);
        g.fillRect(meterX, meterY, meterW * 2 + 2, 2);
    }
}

void ChannelStrip::resized()
{
    auto area = getLocalBounds().reduced(4, 6);
    area.removeFromTop(3); // Color bar

    // Track name at top
    nameLabel.setBounds(area.removeFromTop(16));
    area.removeFromTop(4);

    // Mute/Solo buttons
    auto btnRow = area.removeFromTop(20);
    int btnW = (btnRow.getWidth() - 4) / 2;
    muteBtn.setBounds(btnRow.removeFromLeft(btnW));
    btnRow.removeFromLeft(4);
    soloBtn.setBounds(btnRow.removeFromLeft(btnW));

    area.removeFromTop(4);

    // Pan knob at top of the main area
    int knobSize = juce::jmin(40, area.getWidth() - 8);
    panKnob.setBounds(area.removeFromTop(knobSize)
                          .withSizeKeepingCentre(knobSize, knobSize));

    area.removeFromTop(4);

    // dB label at bottom
    dbLabel.setBounds(area.removeFromBottom(14));
    area.removeFromBottom(2);

    // Fader takes remaining space (minus room for meters)
    faderSlider.setBounds(area.withTrimmedRight(20));
}

void ChannelStrip::timerCallback()
{
    // Get level from the track's volume plugin output
    // In a real implementation, you'd read from the track's level meter.
    // For now, we simulate with a simple decay animation.
    if (track.edit.getTransport().isPlaying())
    {
        // Simulate meter activity
        float target = 0.5f + 0.3f * std::sin(static_cast<float>(
            juce::Time::getMillisecondCounter()) * 0.003f + track.getIndexInEditTrackList());
        currentLevel = currentLevel * 0.85f + target * 0.15f;
        peakLevel = juce::jmax(peakLevel * 0.999f, currentLevel);
    }
    else
    {
        currentLevel *= 0.92f;
        peakLevel *= 0.995f;
    }

    // Update dB readout
    float dB = helpers::sliderToDecibels(static_cast<float>(faderSlider.getValue()));
    dbLabel.setText(juce::String(dB, 1) + " dB", juce::dontSendNotification);

    repaint();
}

// ═══════════════════════════════════════════════════════════════════════════
//  MixerView Implementation
// ═══════════════════════════════════════════════════════════════════════════

MixerView::MixerView(te::Edit& e)
    : edit(e)
{
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&stripContainer, false);
    viewport.setScrollBarsShown(false, true);  // Horizontal scrollbar only

    refreshStrips();
}

void MixerView::paint(juce::Graphics& g)
{
    g.fillAll(colors::BG_DARK);

    // Top border
    g.setColour(colors::BORDER);
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));
}

void MixerView::resized()
{
    viewport.setBounds(getLocalBounds().reduced(2));

    // Size the strip container
    int totalWidth = strips.size() * constants::MIXER_STRIP_WIDTH;
    totalWidth = juce::jmax(totalWidth, viewport.getWidth());
    stripContainer.setSize(totalWidth, viewport.getHeight());

    // Layout strips horizontally
    int x = 4;
    for (auto* strip : strips)
    {
        strip->setBounds(x, 4, constants::MIXER_STRIP_WIDTH - 8,
                         stripContainer.getHeight() - 8);
        x += constants::MIXER_STRIP_WIDTH;
    }
}

void MixerView::refreshStrips()
{
    strips.clear();

    auto tracks = te::getAudioTracks(edit);
    for (int i = 0; i < tracks.size(); ++i)
    {
        auto colour = colors::TRACK_COLORS[i % colors::NUM_TRACK_COLORS];
        auto* strip = new ChannelStrip(*tracks[i], colour);
        stripContainer.addAndMakeVisible(strip);
        strips.add(strip);
    }

    resized();
}

} // namespace nova
