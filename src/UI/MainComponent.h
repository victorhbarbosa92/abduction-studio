#pragma once

// ─── MainComponent ──────────────────────────────────────────────────────────
// Root component that assembles the full DAW layout:
//   ┌──────────────────────────────────────┐
//   │          TransportBar (top)          │
//   ├──────────────────────────────────────┤
//   │                                      │
//   │          TimelineView (center)       │
//   │                                      │
//   ├──────────────────────────────────────┤
//   │          MixerView (bottom)          │
//   └──────────────────────────────────────┘

#include <juce_gui_basics/juce_gui_basics.h>
#include "Engine/EngineManager.h"
#include "Engine/EditManager.h"
#include "Engine/DJEngineManager.h"
#include "ViewModel/TransportViewModel.h"
#include "UI/Transport/TransportBar.h"
#include "UI/Timeline/TimelineView.h"
#include "UI/Mixer/MixerView.h"
#include "UI/DJMode/DJModeView.h"
#include "UI/LookAndFeel/NovaLookAndFeel.h"
#include "Utils/Constants.h"

namespace nova
{

class MainComponent : public juce::Component,
                      public juce::MenuBarModel
{
public:
    MainComponent(EngineManager& engineMgr, EditManager& editMgr);
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void toggleDJMode();

    // ─── MenuBarModel ───────────────────────────────────────────────────
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

private:
    EngineManager& engineMgr;
    EditManager& editMgr;

    // ─── LookAndFeel ────────────────────────────────────────────────────
    NovaLookAndFeel lookAndFeel;

    // ─── ViewModel ──────────────────────────────────────────────────────
    std::unique_ptr<TransportViewModel> transportVM;
    
    // ─── DJ Engine ──────────────────────────────────────────────────────
    std::unique_ptr<DJEngineManager> djEngineMgr;

    // ─── UI Components ──────────────────────────────────────────────────
    std::unique_ptr<juce::MenuBarComponent> menuBar;
    std::unique_ptr<TransportBar> transportBar;
    std::unique_ptr<TimelineView> timelineView;
    std::unique_ptr<MixerView> mixerView;
    std::unique_ptr<DJModeView> djModeView;

    bool isDJMode = false;

    // ─── Resizable divider between timeline and mixer ───────────────────
    int mixerHeight = constants::MIXER_DEFAULT_HEIGHT;

    // ─── Menu IDs ───────────────────────────────────────────────────────
    enum MenuIDs
    {
        menuNewProject     = 1000,
        menuOpenProject    = 1001,
        menuSaveProject    = 1002,
        menuSaveAs         = 1003,
        menuQuit           = 1004,

        menuUndo           = 2000,
        menuRedo           = 2001,

        menuAddTrack       = 3000,
        menuDeleteTrack    = 3001,

        menuAudioSettings  = 4000,

        menuAbout          = 5000,
    };

    void createNewProject();
    void openProject();
    void saveProject();

    void performCreateNewProject();
    void performOpenProject(const juce::File& file);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace nova
