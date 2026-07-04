#pragma once

// ─── MainWindow ─────────────────────────────────────────────────────────────
// The main application window. Contains the MainComponent.

#include <juce_gui_basics/juce_gui_basics.h>
#include "Utils/Constants.h"

namespace nova
{

class MainWindow : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name, juce::Component* contentComponent);
    ~MainWindow() override = default;

    void closeButtonPressed() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace nova
