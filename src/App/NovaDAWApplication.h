#pragma once

// ─── NovaDAWApplication ─────────────────────────────────────────────────────
// Main application class — manages the lifecycle of the app,
// initializes the engine, and creates the main window.

#include <juce_gui_basics/juce_gui_basics.h>
#include "Engine/EngineManager.h"
#include "Engine/EditManager.h"
#include "UI/MainWindow.h"
#include "UI/MainComponent.h"
#include "UI/DebugWindow.h"
#include "Utils/Constants.h"

namespace nova
{

class NovaDAWApplication : public juce::JUCEApplication
{
public:
    NovaDAWApplication() = default;

    // ─── JUCEApplication overrides ──────────────────────────────────────
    const juce::String getApplicationName() override    { return constants::APP_NAME; }
    const juce::String getApplicationVersion() override { return constants::APP_VERSION; }
    bool moreThanOneInstanceAllowed() override           { return false; }

    void initialise(const juce::String& commandLine) override;
    void shutdown() override;

    void systemRequestedQuit() override;
    void anotherInstanceStarted(const juce::String& commandLine) override;

private:
    std::unique_ptr<EngineManager> engineManager;
    std::unique_ptr<EditManager> editManager;
    std::unique_ptr<MainComponent> mainComponent;
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<DebugWindow> debugWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NovaDAWApplication)
};

} // namespace nova
