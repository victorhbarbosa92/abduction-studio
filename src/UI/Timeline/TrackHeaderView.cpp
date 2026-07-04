#include "TrackHeaderView.h"
#include "Utils/Helpers.h"

namespace nova
{

TrackHeaderView::TrackHeaderView(te::AudioTrack& t, juce::Colour trackColour)
    : track(t), colour(trackColour)
{
    setupComponents();
}

void TrackHeaderView::setupComponents()
{
    // ─── Track Name ─────────────────────────────────────────────────────
    addAndMakeVisible(nameLabel);
    nameLabel.setText(track.getName(), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(juce::FontOptions().withHeight(13.0f)));
    nameLabel.setColour(juce::Label::textColourId, colors::TEXT_PRIMARY);
    nameLabel.setEditable(true);
    nameLabel.onTextChange = [this] {
        track.setName(nameLabel.getText());
    };

    // ─── Mute Button ────────────────────────────────────────────────────
    addAndMakeVisible(muteBtn);
    muteBtn.setClickingTogglesState(true);
    muteBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHTER);
    muteBtn.setColour(juce::TextButton::buttonOnColourId, colors::LOOP_YELLOW.withAlpha(0.5f));
    muteBtn.onClick = [this] {
        track.setMute(muteBtn.getToggleState());
        updateMuteSoloState();
    };

    // ─── Solo Button ────────────────────────────────────────────────────
    addAndMakeVisible(soloBtn);
    soloBtn.setClickingTogglesState(true);
    soloBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHTER);
    soloBtn.setColour(juce::TextButton::buttonOnColourId, colors::ACCENT_SECONDARY.withAlpha(0.5f));
    soloBtn.onClick = [this] {
        track.setSolo(soloBtn.getToggleState());
        updateMuteSoloState();
    };

    // ─── Record Arm Button ──────────────────────────────────────────────
    addAndMakeVisible(armBtn);
    armBtn.setClickingTogglesState(true);
    armBtn.setColour(juce::TextButton::buttonColourId, colors::BG_LIGHTER);
    armBtn.setColour(juce::TextButton::buttonOnColourId, colors::RECORD_RED.withAlpha(0.5f));
    armBtn.onClick = [this] {
        // Arm/disarm track for recording
        for (auto instance : track.edit.getAllInputDevices())
        {
            if (te::isOnTargetTrack(*instance, track, 0))
            {
                instance->setRecordingEnabled(track.itemID, armBtn.getToggleState());
                break;
            }
        }
    };

    // ─── Volume Slider (horizontal) ─────────────────────────────────────
    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::LinearBar);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(helpers::decibelsToSlider(
        track.getVolumePlugin()->volParam->getCurrentValue()), juce::dontSendNotification);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.setColour(juce::Slider::backgroundColourId, colors::BG_DARK);
    volumeSlider.setColour(juce::Slider::trackColourId, colour.withAlpha(0.4f));
    volumeSlider.onValueChange = [this] {
        float dB = helpers::sliderToDecibels(static_cast<float>(volumeSlider.getValue()));
        track.getVolumePlugin()->volParam->setParameter(
            helpers::decibelsToGain(dB), juce::dontSendNotification);
    };

    // ─── Pan Knob ───────────────────────────────────────────────────────
    addAndMakeVisible(panSlider);
    panSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    panSlider.setRange(-1.0, 1.0, 0.01);
    panSlider.setValue(0.0, juce::dontSendNotification);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    panSlider.onValueChange = [this] {
        track.getVolumePlugin()->panParam->setParameter(
            static_cast<float>((panSlider.getValue() + 1.0) * 0.5), juce::dontSendNotification);
    };
}

void TrackHeaderView::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    g.setColour(colors::BG_MID);
    g.fillRect(bounds);

    // Left color strip (track identity)
    g.setColour(colour);
    g.fillRect(bounds.removeFromLeft(4));

    // Bottom separator
    g.setColour(colors::BORDER);
    g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));
}

void TrackHeaderView::resized()
{
    auto area = getLocalBounds().reduced(6, 4);
    area.removeFromLeft(4); // Account for color strip

    // Top row: name + buttons
    auto topRow = area.removeFromTop(22);
    nameLabel.setBounds(topRow.removeFromLeft(topRow.getWidth() - 78));
    topRow.removeFromLeft(4);

    int btnSize = 22;
    muteBtn.setBounds(topRow.removeFromLeft(btnSize));
    topRow.removeFromLeft(2);
    soloBtn.setBounds(topRow.removeFromLeft(btnSize));
    topRow.removeFromLeft(2);
    armBtn.setBounds(topRow.removeFromLeft(btnSize));

    area.removeFromTop(6);

    // Bottom row: volume slider + pan knob
    auto bottomRow = area;
    panSlider.setBounds(bottomRow.removeFromRight(bottomRow.getHeight()).reduced(2));
    bottomRow.removeFromRight(4);
    volumeSlider.setBounds(bottomRow.reduced(0, 6));
}

void TrackHeaderView::updateMuteSoloState()
{
    float alpha = track.isMuted(true) ? 0.3f : 1.0f;
    nameLabel.setAlpha(alpha);
}

} // namespace nova
