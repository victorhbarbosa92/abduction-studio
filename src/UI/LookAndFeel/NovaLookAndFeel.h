#pragma once

// ─── NovaLookAndFeel ────────────────────────────────────────────────────────
// Custom LookAndFeel for NovaDAW — modern dark theme inspired by
// Bitwig Studio and Ableton Live.

#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils/Constants.h"

namespace nova
{

class NovaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    NovaLookAndFeel();
    ~NovaLookAndFeel() override = default;

    // ─── Overrides ──────────────────────────────────────────────────────

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g,
                        juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    void drawLinearSlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style,
                          juce::Slider& slider) override;

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawScrollbar(juce::Graphics& g,
                       juce::ScrollBar& scrollbar,
                       int x, int y, int width, int height,
                       bool isScrollbarVertical,
                       int thumbStartPosition, int thumbSize,
                       bool isMouseOver, bool isMouseDown) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    juce::Font getLabelFont(juce::Label& label) override;

    // ─── Custom Drawing ─────────────────────────────────────────────────

    /// Draw a panel/card background with rounded corners
    static void drawPanel(juce::Graphics& g, juce::Rectangle<int> bounds,
                          float cornerRadius = 6.0f);

    /// Draw a meter bar (for VU meters)
    static void drawMeterBar(juce::Graphics& g, juce::Rectangle<int> bounds,
                             float level, bool isVertical = true);

private:
    juce::Font defaultFont;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NovaLookAndFeel)
};

} // namespace nova
