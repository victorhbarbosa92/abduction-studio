#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace nova
{

class DJEngineManager;

class DJModeView : public juce::Component
{
public:
    explicit DJModeView(DJEngineManager* djEngineMgr);
    ~DJModeView() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class DeckView : public juce::Component
    {
    public:
        DeckView(int deckIdx, const juce::String& deckName, juce::Colour deckColor, DJEngineManager* djEngine);
        void paint(juce::Graphics&) override;
        void resized() override;
    private:
        int index;
        juce::String name;
        juce::Colour color;
        DJEngineManager* engine;
        
        juce::TextButton playButton{ "PLAY" };
        juce::TextButton cueButton{ "CUE" };
        juce::TextButton loadButton{ "LOAD" };
        juce::Slider pitchSlider{ juce::Slider::LinearVertical, juce::Slider::TextBoxBelow };
        juce::Slider jogWheel{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox };
        
        juce::Label displayLabel;
        juce::Label pitchLabel{ {}, "TEMPO" };
    };

    class MixerSection : public juce::Component
    {
    public:
        explicit MixerSection(DJEngineManager* djEngine);
        void paint(juce::Graphics&) override;
        void resized() override;
    private:
        juce::Slider volumeA{ juce::Slider::LinearVertical, juce::Slider::TextBoxBelow };
        juce::Slider volumeB{ juce::Slider::LinearVertical, juce::Slider::TextBoxBelow };
        juce::Slider eqHighA{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Slider eqMidA{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Slider eqLowA{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Slider eqHighB{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Slider eqMidB{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Slider eqLowB{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Slider crossfader{ juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
        
        juce::Label lblVolA{ {}, "DECK A" };
        juce::Label lblVolB{ {}, "DECK B" };
        juce::Label lblEqHighA{ {}, "HIGH" };
        juce::Label lblEqMidA{ {}, "MID" };
        juce::Label lblEqLowA{ {}, "LOW" };
        juce::Label lblEqHighB{ {}, "HIGH" };
        juce::Label lblEqMidB{ {}, "MID" };
        juce::Label lblEqLowB{ {}, "LOW" };
        juce::Label lblCrossfader{ {}, "A <---- CROSSFADER ----> B" };
        
        DJEngineManager* engine;
    };

    DJEngineManager* djEngine;

    std::unique_ptr<DeckView> deckA;
    std::unique_ptr<DeckView> deckB;
    std::unique_ptr<MixerSection> mixer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DJModeView)
};

} // namespace nova
