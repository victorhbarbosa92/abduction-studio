#include "NovaLookAndFeel.h"

namespace nova
{

NovaLookAndFeel::NovaLookAndFeel()
{
    // ─── Base Color Scheme ──────────────────────────────────────────────
    setColour(juce::ResizableWindow::backgroundColourId,   colors::BG_DARKEST);
    setColour(juce::DocumentWindow::textColourId,          colors::TEXT_PRIMARY);

    // TextButton
    setColour(juce::TextButton::buttonColourId,            colors::SURFACE);
    setColour(juce::TextButton::buttonOnColourId,          colors::ACCENT_PRIMARY.withAlpha(0.2f));
    setColour(juce::TextButton::textColourOnId,            colors::ACCENT_PRIMARY);
    setColour(juce::TextButton::textColourOffId,           colors::TEXT_MUTED);

    // Label (HUD Style)
    setColour(juce::Label::textColourId,                   colors::TEXT_PRIMARY);
    setColour(juce::Label::backgroundColourId,             colors::BG_DARKEST);

    // Slider
    setColour(juce::Slider::backgroundColourId,            colors::BG_DARKEST);
    setColour(juce::Slider::thumbColourId,                 colors::SURFACE_HOVER);
    setColour(juce::Slider::trackColourId,                 colors::ACCENT_PRIMARY);
    setColour(juce::Slider::rotarySliderFillColourId,      colors::ACCENT_PRIMARY);
    setColour(juce::Slider::rotarySliderOutlineColourId,   colors::BG_DARKEST);
    setColour(juce::Slider::textBoxTextColourId,           colors::TEXT_PRIMARY);
    setColour(juce::Slider::textBoxBackgroundColourId,     colors::BG_DARKEST);
    setColour(juce::Slider::textBoxOutlineColourId,        colors::ACCENT_PRIMARY.withAlpha(0.5f));

    // ScrollBar
    setColour(juce::ScrollBar::thumbColourId,              colors::ACCENT_PRIMARY.withAlpha(0.5f));
    setColour(juce::ScrollBar::backgroundColourId,         colors::BG_DARKEST);

    // ComboBox
    setColour(juce::ComboBox::backgroundColourId,          colors::BG_DARKEST);
    setColour(juce::ComboBox::textColourId,                colors::TEXT_PRIMARY);
    setColour(juce::ComboBox::outlineColourId,             colors::ACCENT_PRIMARY.withAlpha(0.5f));
    setColour(juce::ComboBox::arrowColourId,               colors::ACCENT_PRIMARY);

    // PopupMenu
    setColour(juce::PopupMenu::backgroundColourId,         colors::BG_DARKEST);
    setColour(juce::PopupMenu::textColourId,               colors::TEXT_PRIMARY);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, colors::ACCENT_PRIMARY.withAlpha(0.3f));
    setColour(juce::PopupMenu::highlightedTextColourId,    colors::ACCENT_PRIMARY);

    // TextEditor
    setColour(juce::TextEditor::backgroundColourId,        colors::BG_DARKEST);
    setColour(juce::TextEditor::textColourId,              colors::TEXT_PRIMARY);
    setColour(juce::TextEditor::outlineColourId,           colors::ACCENT_PRIMARY.withAlpha(0.5f));
    setColour(juce::TextEditor::focusedOutlineColourId,    colors::ACCENT_PRIMARY);
    setColour(juce::TextEditor::highlightColourId,         colors::ACCENT_PRIMARY.withAlpha(0.4f));

    // ListBox
    setColour(juce::ListBox::backgroundColourId,           colors::BG_DARKEST);
    setColour(juce::ListBox::textColourId,                 colors::TEXT_PRIMARY);

    // AlertWindow
    setColour(juce::AlertWindow::backgroundColourId,       colors::BG_MID);
    setColour(juce::AlertWindow::textColourId,             colors::TEXT_PRIMARY);
    setColour(juce::AlertWindow::outlineColourId,          colors::ACCENT_PRIMARY);

    // Tooltip
    setColour(juce::TooltipWindow::backgroundColourId,     colors::BG_DARKEST);
    setColour(juce::TooltipWindow::textColourId,           colors::ACCENT_SECONDARY);
    setColour(juce::TooltipWindow::outlineColourId,        colors::ACCENT_SECONDARY);

    // Set default font (Monospaced or bold sans for HUD look)
    defaultFont = juce::Font("Consolas", 15.0f, juce::Font::bold);
    setDefaultSansSerifTypeface(defaultFont.getTypefacePtr());
}

// ─── Button Drawing ─────────────────────────────────────────────────────────

void NovaLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                            juce::Button& button,
                                            const juce::Colour& backgroundColour,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    // Pill shape for spaceship buttons
    float cornerRadius = bounds.getHeight() / 2.0f;
    
    juce::Colour baseColor = button.getToggleState() ? button.findColour(juce::TextButton::buttonOnColourId) : backgroundColour;
    
    if (shouldDrawButtonAsDown)
        baseColor = baseColor.darker(0.3f);
    else if (shouldDrawButtonAsHighlighted)
        baseColor = baseColor.brighter(0.1f);

    // Metallic Gradient for the button
    juce::ColourGradient gradient(baseColor.brighter(0.15f), bounds.getX(), bounds.getY(),
                                  baseColor.darker(0.2f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, cornerRadius);

    if (button.getToggleState() || shouldDrawButtonAsDown)
    {
        // Inner shadow when pressed or active
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, cornerRadius, 2.0f);
        
        // HUD glow
        if (button.getToggleState())
        {
            g.setColour(colors::ACCENT_PRIMARY.withAlpha(0.8f));
            g.drawRoundedRectangle(bounds.expanded(1.0f), cornerRadius + 1.0f, 2.5f);
        }
    }
    else
    {
        // Top highlight for 3D extrusion
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), cornerRadius, 1.0f);
        // Bottom shadow
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.drawRoundedRectangle(bounds, cornerRadius, 1.5f);
    }
}

void NovaLookAndFeel::drawButtonText(juce::Graphics& g,
                                      juce::TextButton& button,
                                      bool /*shouldDrawButtonAsHighlighted*/,
                                      bool /*shouldDrawButtonAsDown*/)
{
    auto font = juce::Font("Consolas", 13.0f, juce::Font::bold);
    g.setFont(font);

    juce::Colour textCol = button.getToggleState()
                    ? findColour(juce::TextButton::textColourOnId)
                    : findColour(juce::TextButton::textColourOffId);
                    
    if (button.getToggleState())
    {
        g.setColour(textCol.withAlpha(0.8f));
        g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(4, 2).translated(0, 1), juce::Justification::centred, 1);
    }
    
    g.setColour(textCol);
    g.drawFittedText(button.getButtonText(),
                     button.getLocalBounds().reduced(4, 2),
                     juce::Justification::centred, 1);
}

// ─── Linear Slider (Fader) ──────────────────────────────────────────────────

void NovaLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPos,
                                        float /*minSliderPos*/, float /*maxSliderPos*/,
                                        juce::Slider::SliderStyle style,
                                        juce::Slider& slider)
{
    bool isVertical = (style == juce::Slider::LinearVertical || style == juce::Slider::LinearBarVertical);
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                          static_cast<float>(width), static_cast<float>(height));

    float trackWidth = 8.0f; // Wider track

    if (isVertical)
    {
        // Groove background (very dark, almost black)
        auto trackBounds = bounds.withSizeKeepingCentre(trackWidth, bounds.getHeight());
        g.setColour(colors::BG_DARKEST);
        g.fillRoundedRectangle(trackBounds, 4.0f);
        
        // Inner groove bevel
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(trackBounds, 4.0f, 1.5f);

        // Fill bar (HUD style)
        auto fillBounds = trackBounds.withTop(sliderPos).withBottom(bounds.getBottom()).reduced(1.5f);
        g.setColour(colors::ACCENT_PRIMARY.withAlpha(0.3f));
        g.fillRoundedRectangle(fillBounds, 2.0f);
        
        // Horizontal scanlines in the fill
        g.setColour(colors::ACCENT_PRIMARY);
        for (float ly = fillBounds.getBottom(); ly > fillBounds.getY(); ly -= 4.0f) {
            g.fillRect(fillBounds.getX(), ly, fillBounds.getWidth(), 1.0f);
        }

        // Space Thruster Fader Cap
        float thumbW = 28.0f;
        float thumbH = 20.0f;
        auto thumbBounds = juce::Rectangle<float>(thumbW, thumbH).withCentre({ bounds.getCentreX(), sliderPos });

        // Metallic cap base
        juce::ColourGradient capGradient(colors::SURFACE.brighter(0.4f), thumbBounds.getX(), thumbBounds.getY(),
                                         colors::SURFACE.darker(0.6f), thumbBounds.getRight(), thumbBounds.getBottom(), false);
        g.setGradientFill(capGradient);
        g.fillRoundedRectangle(thumbBounds, 3.0f);

        // Cap bevel
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(thumbBounds, 3.0f, 1.5f);
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRoundedRectangle(thumbBounds.reduced(1.0f), 2.0f, 1.0f);

        // Thruster grip lines
        g.setColour(colors::BG_DARKEST);
        g.fillRect(thumbBounds.getX() + 4.0f, thumbBounds.getCentreY() - 3.0f, thumbW - 8.0f, 1.5f);
        g.fillRect(thumbBounds.getX() + 4.0f, thumbBounds.getCentreY() + 1.5f, thumbW - 8.0f, 1.5f);
        
        // Indicator LED
        g.setColour(colors::ACCENT_SECONDARY);
        g.fillRect(thumbBounds.getX() - 2.0f, thumbBounds.getCentreY() - 2.0f, 3.0f, 4.0f);
    }
    else
    {
        // Horizontal slider
        auto trackBounds = bounds.withSizeKeepingCentre(bounds.getWidth(), trackWidth);
        g.setColour(colors::BG_DARKEST);
        g.fillRoundedRectangle(trackBounds, 4.0f);
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(trackBounds, 4.0f, 1.5f);

        auto fillBounds = trackBounds.withRight(sliderPos).reduced(1.5f);
        g.setColour(colors::ACCENT_PRIMARY.withAlpha(0.3f));
        g.fillRoundedRectangle(fillBounds, 2.0f);
        g.setColour(colors::ACCENT_PRIMARY);
        for (float lx = fillBounds.getX(); lx < fillBounds.getRight(); lx += 4.0f) {
            g.fillRect(lx, fillBounds.getY(), 1.0f, fillBounds.getHeight());
        }

        float thumbW = 20.0f;
        float thumbH = 28.0f;
        auto thumbBounds = juce::Rectangle<float>(thumbW, thumbH).withCentre({ sliderPos, bounds.getCentreY() });

        juce::ColourGradient capGradient(colors::SURFACE.brighter(0.4f), thumbBounds.getX(), thumbBounds.getY(),
                                         colors::SURFACE.darker(0.6f), thumbBounds.getRight(), thumbBounds.getBottom(), false);
        g.setGradientFill(capGradient);
        g.fillRoundedRectangle(thumbBounds, 3.0f);
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(thumbBounds, 3.0f, 1.5f);
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRoundedRectangle(thumbBounds.reduced(1.0f), 2.0f, 1.0f);

        g.setColour(colors::BG_DARKEST);
        g.fillRect(thumbBounds.getCentreX() - 3.0f, thumbBounds.getY() + 4.0f, 1.5f, thumbH - 8.0f);
        g.fillRect(thumbBounds.getCentreX() + 1.5f, thumbBounds.getY() + 4.0f, 1.5f, thumbH - 8.0f);
    }
}

// ─── Rotary Slider (Knob) ───────────────────────────────────────────────────

void NovaLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPosProportional,
                                        float rotaryStartAngle,
                                        float rotaryEndAngle,
                                        juce::Slider& /*slider*/)
{
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                          static_cast<float>(width), static_cast<float>(height)).reduced(2.0f);
    float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();
    float arcRadius = radius - 4.0f;
    float lineWidth = 5.0f;

    // Outer HUD Ring (Radar style)
    juce::Path bgArc;
    bgArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(colors::BG_DARKEST);
    g.strokePath(bgArc, juce::PathStrokeType(lineWidth + 1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
    g.setColour(colors::TEXT_MUTED.withAlpha(0.5f));
    g.strokePath(bgArc, juce::PathStrokeType(1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));

    // Value Arc
    float endAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, endAngle, true);
    g.setColour(colors::ACCENT_PRIMARY.withAlpha(0.6f));
    g.strokePath(valueArc, juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
    
    // Draw segmented tick marks on the HUD ring
    g.setColour(colors::ACCENT_PRIMARY);
    for (float angle = rotaryStartAngle; angle <= endAngle; angle += 0.2f) {
        float tx = centreX + std::sin(angle) * arcRadius;
        float ty = centreY - std::cos(angle) * arcRadius;
        g.fillEllipse(tx - 1.0f, ty - 1.0f, 2.0f, 2.0f);
    }

    // Heavy Metallic Knob
    float knobRadius = radius - 9.0f;
    juce::Rectangle<float> knobBounds(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
    
    juce::ColourGradient knobGradient(colors::SURFACE.brighter(0.5f), knobBounds.getX(), knobBounds.getY(),
                                      colors::SURFACE.darker(0.8f), knobBounds.getRight(), knobBounds.getBottom(), false);
    g.setGradientFill(knobGradient);
    g.fillEllipse(knobBounds);
    
    // Inner bevel for heavy machinery look
    g.setColour(juce::Colours::black);
    g.drawEllipse(knobBounds, 2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawEllipse(knobBounds.reduced(1.5f), 1.0f);

    // Pointer line
    float pointerLength = knobRadius - 3.0f;
    float pointerThickness = 3.0f;
    juce::Path pointer;
    pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength);
    pointer.applyTransform(juce::AffineTransform::rotation(endAngle).translated(centreX, centreY));
    
    g.setColour(colors::ACCENT_SECONDARY); // Amber pointer
    g.fillPath(pointer);
}

// ─── Scrollbar ──────────────────────────────────────────────────────────────

void NovaLookAndFeel::drawScrollbar(juce::Graphics& g,
                                     juce::ScrollBar& /*scrollbar*/,
                                     int x, int y, int width, int height,
                                     bool isScrollbarVertical,
                                     int thumbStartPosition, int thumbSize,
                                     bool isMouseOver, bool isMouseDown)
{
    juce::Colour thumbColour = colors::BG_LIGHTER;
    if (isMouseDown) thumbColour = colors::ACCENT_PRIMARY;
    else if (isMouseOver) thumbColour = colors::ACCENT_PRIMARY.withAlpha(0.6f);

    auto thumbBounds = isScrollbarVertical
        ? juce::Rectangle<int>(x + 2, thumbStartPosition, width - 4, thumbSize)
        : juce::Rectangle<int>(thumbStartPosition, y + 2, thumbSize, height - 4);

    g.setColour(thumbColour);
    g.fillRoundedRectangle(thumbBounds.toFloat(), 2.0f);
}

// ─── Label ──────────────────────────────────────────────────────────────────

void NovaLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    // Draw HUD screen background for labels
    auto bounds = label.getLocalBounds().toFloat();
    g.setColour(colors::BG_DARKEST);
    g.fillRect(bounds);
    
    g.setColour(colors::ACCENT_PRIMARY.withAlpha(0.3f));
    g.drawRect(bounds, 1.0f); // Thin neon border

    auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
    auto font = getLabelFont(label);

    g.setFont(font);
    g.setColour(label.findColour(juce::Label::textColourId));
    g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                     juce::jmax(1, static_cast<int>(textArea.getHeight() / font.getHeight())),
                     label.getMinimumHorizontalScale());
}

juce::Font NovaLookAndFeel::getLabelFont(juce::Label& /*label*/)
{
    return juce::Font("Consolas", 14.0f, juce::Font::bold);
}

// ─── Custom Drawing Helpers ─────────────────────────────────────────────────

void NovaLookAndFeel::drawPanel(juce::Graphics& g, juce::Rectangle<int> bounds,
                                 float cornerRadius)
{
    // Spaceship Metal Plate
    juce::ColourGradient bgGrad(colors::BG_MID.brighter(0.1f), bounds.getX(), bounds.getY(),
                                colors::BG_MID.darker(0.3f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(bounds.toFloat(), cornerRadius);

    // Beveled edges
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(bounds.toFloat().reduced(1.0f), cornerRadius, 1.0f);
    
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.drawRoundedRectangle(bounds.toFloat(), cornerRadius, 2.0f);
    
    // Draw subtle panel lines (screws/rivets)
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(bounds.getX() + 6, bounds.getY() + 6, 4, 4);
    g.fillEllipse(bounds.getRight() - 10, bounds.getY() + 6, 4, 4);
    g.fillEllipse(bounds.getX() + 6, bounds.getBottom() - 10, 4, 4);
    g.fillEllipse(bounds.getRight() - 10, bounds.getBottom() - 10, 4, 4);
}

void NovaLookAndFeel::drawMeterBar(juce::Graphics& g, juce::Rectangle<int> bounds,
                                    float level, bool isVertical)
{
    level = juce::jlimit(0.0f, 1.0f, level);

    // HUD Meter Background
    g.setColour(colors::METER_BG);
    g.fillRoundedRectangle(bounds.toFloat(), 0.0f);
    
    // Grid overlay
    g.setColour(colors::ACCENT_PRIMARY.withAlpha(0.1f));
    g.drawRect(bounds.toFloat(), 1.0f);

    if (level <= 0.0f) return;

    juce::Rectangle<float> filledBounds;
    if (isVertical) {
        float fillHeight = bounds.getHeight() * level;
        filledBounds = bounds.toFloat().withTop(bounds.getBottom() - fillHeight);
    } else {
        float fillWidth = bounds.getWidth() * level;
        filledBounds = bounds.toFloat().withWidth(fillWidth);
    }

    juce::Colour meterColour;
    if (level < 0.6f) meterColour = colors::METER_GREEN;
    else if (level < 0.85f) meterColour = colors::METER_GREEN.interpolatedWith(colors::METER_YELLOW, (level - 0.6f) / 0.25f);
    else meterColour = colors::METER_YELLOW.interpolatedWith(colors::METER_RED, (level - 0.85f) / 0.15f);

    // Blocky/Segmented HUD style fill
    g.setColour(meterColour);
    if (isVertical) {
        for (float ly = filledBounds.getBottom(); ly > filledBounds.getY(); ly -= 3.0f) {
            g.fillRect(filledBounds.getX(), ly - 2.0f, filledBounds.getWidth(), 2.0f);
        }
    } else {
        for (float lx = filledBounds.getX(); lx < filledBounds.getRight(); lx += 3.0f) {
            g.fillRect(lx, filledBounds.getY(), 2.0f, filledBounds.getHeight());
        }
    }
}

} // namespace nova
