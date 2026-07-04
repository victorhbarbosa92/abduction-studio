#include "DJModeView.h"
#include "../../Utils/Constants.h"
#include "../../Engine/DJEngineManager.h"

namespace nova
{

// ─── Utils ─────────────────────────────────────────────────────────────────
static void setupHUDLabel(juce::Label& lbl, juce::Colour color, float fontSize = 14.0f)
{
    lbl.setFont(juce::Font("Consolas", fontSize, juce::Font::bold));
    lbl.setColour(juce::Label::textColourId, color);
    lbl.setJustificationType(juce::Justification::centred);
}

static void setupHUDSlider(juce::Slider& s)
{
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

// ─── DeckView ──────────────────────────────────────────────────────────────

DJModeView::DeckView::DeckView(int deckIdx, const juce::String& deckName, juce::Colour deckColor, DJEngineManager* djEngine)
    : index(deckIdx), name(deckName), color(deckColor), engine(djEngine)
{
    addAndMakeVisible(playButton);
    addAndMakeVisible(cueButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(pitchSlider);
    addAndMakeVisible(jogWheel);
    addAndMakeVisible(displayLabel);
    addAndMakeVisible(pitchLabel);

    setupHUDLabel(displayLabel, color, 16.0f);
    displayLabel.setText(name + " - No Track Loaded", juce::dontSendNotification);

    setupHUDLabel(pitchLabel, colors::TEXT_MUTED, 12.0f);

    playButton.setColour(juce::TextButton::buttonColourId, colors::SURFACE);
    playButton.setColour(juce::TextButton::textColourOffId, color);
    playButton.onClick = [this] { if (engine) engine->playDeck(index); };
    
    cueButton.setColour(juce::TextButton::buttonColourId, colors::SURFACE);
    cueButton.setColour(juce::TextButton::textColourOffId, colors::ACCENT_SECONDARY); // Amber
    cueButton.onClick = [this] { if (engine) engine->pauseDeck(index); };
    
    loadButton.setColour(juce::TextButton::buttonColourId, colors::SURFACE);
    loadButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loadButton.onClick = [this] {
        juce::FileChooser chooser("Select Audio File", juce::File::getSpecialLocation(juce::File::userMusicDirectory), "*.mp3;*.wav;*.aiff;*.flac");
        if (chooser.browseForFileToOpen()) {
            if (engine && engine->loadTrackToDeck(index, chooser.getResult())) {
                displayLabel.setText(chooser.getResult().getFileNameWithoutExtension(), juce::dontSendNotification);
            }
        }
    };

    pitchSlider.setRange(-1.0, 1.0, 0.01);
    pitchSlider.setValue(0.0);
    setupHUDSlider(pitchSlider);
    pitchSlider.setTextValueSuffix("%");
    pitchSlider.setColour(juce::Slider::trackColourId, color.withAlpha(0.5f));
    pitchSlider.onValueChange = [this] { if (engine) engine->setDeckPitch(index, pitchSlider.getValue()); };
    
    jogWheel.setRange(0.0, 1.0, 0.01);
    jogWheel.setValue(0.5);
}

void DJModeView::DeckView::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);
    
    // Panel Background
    juce::ColourGradient grad(colors::BG_DARK.brighter(0.1f), bounds.getX(), bounds.getY(),
                              colors::BG_DARK, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, 8.0f);
    
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 7.0f, 1.0f);
}

void DJModeView::DeckView::resized()
{
    auto area = getLocalBounds().reduced(12);

    auto topArea = area.removeFromTop(40);
    displayLabel.setBounds(topArea.removeFromLeft(topArea.getWidth() - 70));
    loadButton.setBounds(topArea.reduced(5, 5));

    auto rightArea = area.removeFromRight(80);
    pitchLabel.setBounds(rightArea.removeFromTop(20));
    pitchSlider.setBounds(rightArea.reduced(10, 10));

    auto bottomArea = area.removeFromBottom(80);
    playButton.setBounds(bottomArea.removeFromLeft(80).reduced(10));
    cueButton.setBounds(bottomArea.removeFromLeft(80).reduced(10));

    // Remaining area for jog wheel
    jogWheel.setBounds(area.reduced(20));
}

// ─── MixerSection ──────────────────────────────────────────────────────────

DJModeView::MixerSection::MixerSection(DJEngineManager* djEngine)
    : engine(djEngine)
{
    addAndMakeVisible(volumeA); addAndMakeVisible(volumeB);
    addAndMakeVisible(eqHighA); addAndMakeVisible(eqMidA); addAndMakeVisible(eqLowA);
    addAndMakeVisible(eqHighB); addAndMakeVisible(eqMidB); addAndMakeVisible(eqLowB);
    addAndMakeVisible(crossfader);

    addAndMakeVisible(lblVolA); addAndMakeVisible(lblVolB);
    addAndMakeVisible(lblEqHighA); addAndMakeVisible(lblEqMidA); addAndMakeVisible(lblEqLowA);
    addAndMakeVisible(lblEqHighB); addAndMakeVisible(lblEqMidB); addAndMakeVisible(lblEqLowB);
    addAndMakeVisible(lblCrossfader);

    juce::Colour hudCol = colors::TEXT_MUTED;
    setupHUDLabel(lblVolA, hudCol); setupHUDLabel(lblVolB, hudCol);
    setupHUDLabel(lblEqHighA, hudCol); setupHUDLabel(lblEqMidA, hudCol); setupHUDLabel(lblEqLowA, hudCol);
    setupHUDLabel(lblEqHighB, hudCol); setupHUDLabel(lblEqMidB, hudCol); setupHUDLabel(lblEqLowB, hudCol);
    setupHUDLabel(lblCrossfader, hudCol, 12.0f);

    auto initFader = [](juce::Slider& s) { 
        s.setRange(0.0, 1.0, 0.01); s.setValue(0.8); 
        setupHUDSlider(s);
    };
    initFader(volumeA); initFader(volumeB);
    volumeA.onValueChange = [this] { if (engine) engine->setDeckVolume(0, volumeA.getValue()); };
    volumeB.onValueChange = [this] { if (engine) engine->setDeckVolume(1, volumeB.getValue()); };
    
    auto initEQ = [](juce::Slider& s) { 
        s.setRange(-1.0, 1.0, 0.01); s.setValue(0.0);
        setupHUDSlider(s);
    };
    initEQ(eqHighA); initEQ(eqMidA); initEQ(eqLowA);
    initEQ(eqHighB); initEQ(eqMidB); initEQ(eqLowB);

    crossfader.setRange(-1.0, 1.0, 0.01); crossfader.setValue(0.0);
    crossfader.onValueChange = [this] { if (engine) engine->setCrossfader(crossfader.getValue()); };
}

void DJModeView::MixerSection::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);
    
    juce::ColourGradient grad(colors::BG_MID.brighter(0.05f), bounds.getX(), bounds.getY(),
                              colors::BG_MID.darker(0.15f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, 8.0f);
    
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);
    
    // Draw EQ grouping boxes
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    auto eqArea = bounds.reduced(10.0f);
    eqArea.removeFromBottom(100.0f); // Reserve space for crossfader
    
    auto eqAreaA = eqArea.removeFromLeft(eqArea.getWidth() / 2.0f).reduced(10.0f);
    auto eqAreaB = eqArea.reduced(10.0f);
    
    g.fillRoundedRectangle(eqAreaA, 4.0f);
    g.fillRoundedRectangle(eqAreaB, 4.0f);
    
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(eqAreaA, 4.0f, 1.5f);
    g.drawRoundedRectangle(eqAreaB, 4.0f, 1.5f);
}

void DJModeView::MixerSection::resized()
{
    auto area = getLocalBounds().reduced(12);

    auto crossfaderArea = area.removeFromBottom(70);
    lblCrossfader.setBounds(crossfaderArea.removeFromTop(20));
    crossfader.setBounds(crossfaderArea.reduced(30, 10));

    auto leftMixer = area.removeFromLeft(area.getWidth() / 2);
    auto rightMixer = area;
    
    // Left Channel (Deck A) Layout
    auto eqBoxA = leftMixer.reduced(10);
    lblVolA.setBounds(eqBoxA.removeFromBottom(20));
    volumeA.setBounds(eqBoxA.removeFromBottom(120).reduced(10, 0));
    
    int eqHeight = eqBoxA.getHeight() / 3;
    
    auto rowL1 = eqBoxA.removeFromTop(eqHeight);
    lblEqHighA.setBounds(rowL1.removeFromTop(20));
    eqHighA.setBounds(rowL1);
    
    auto rowL2 = eqBoxA.removeFromTop(eqHeight);
    lblEqMidA.setBounds(rowL2.removeFromTop(20));
    eqMidA.setBounds(rowL2);
    
    auto rowL3 = eqBoxA.removeFromTop(eqHeight);
    lblEqLowA.setBounds(rowL3.removeFromTop(20));
    eqLowA.setBounds(rowL3);
    
    // Right Channel (Deck B) Layout
    auto eqBoxB = rightMixer.reduced(10);
    lblVolB.setBounds(eqBoxB.removeFromBottom(20));
    volumeB.setBounds(eqBoxB.removeFromBottom(120).reduced(10, 0));
    
    auto rowR1 = eqBoxB.removeFromTop(eqHeight);
    lblEqHighB.setBounds(rowR1.removeFromTop(20));
    eqHighB.setBounds(rowR1);
    
    auto rowR2 = eqBoxB.removeFromTop(eqHeight);
    lblEqMidB.setBounds(rowR2.removeFromTop(20));
    eqMidB.setBounds(rowR2);
    
    auto rowR3 = eqBoxB.removeFromTop(eqHeight);
    lblEqLowB.setBounds(rowR3.removeFromTop(20));
    eqLowB.setBounds(rowR3);
}

// ─── DJModeView ────────────────────────────────────────────────────────────

DJModeView::DJModeView(DJEngineManager* djEngineMgr)
    : djEngine(djEngineMgr)
{
    deckA = std::make_unique<DeckView>(0, u8"DIMENSÃO A [ALFA]", colors::TEXT_PRIMARY, djEngine);
    deckB = std::make_unique<DeckView>(1, u8"DIMENSÃO B [ÔMEGA]", colors::TEXT_SECONDARY, djEngine);
    mixer = std::make_unique<MixerSection>(djEngine);

    addAndMakeVisible(deckA.get());
    addAndMakeVisible(deckB.get());
    addAndMakeVisible(mixer.get());
}

DJModeView::~DJModeView() {}

void DJModeView::paint(juce::Graphics& g)
{
    g.fillAll(colors::BG_DARKEST);
}

void DJModeView::resized()
{
    auto area = getLocalBounds();
    int deckWidth = area.getWidth() * 0.35;
    
    if (deckA) deckA->setBounds(area.removeFromLeft(deckWidth));
    if (deckB) deckB->setBounds(area.removeFromRight(deckWidth));
    if (mixer) mixer->setBounds(area);
}

} // namespace nova
