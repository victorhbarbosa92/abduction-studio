#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include "Utils/Constants.h"

namespace nova
{

class DebugWindow : public juce::DocumentWindow,
                    public juce::Logger
{
public:
    DebugWindow();
    ~DebugWindow() override;

    void closeButtonPressed() override;

protected:
    void logMessage(const juce::String& message) override;

private:
    juce::TextEditor textEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugWindow)
};

} // namespace nova
