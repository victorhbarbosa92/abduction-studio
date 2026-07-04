#include "EngineManager.h"

namespace nova
{

EngineManager::EngineManager()
{
    // Initialize the Tracktion Engine with our app name.
    // The Engine manages audio devices, plugin scanning, and the audio graph.
    engine = std::make_unique<te::Engine>("NovaDAW");

    // Log successful initialization
    DBG("NovaDAW: Tracktion Engine initialized successfully.");
    DBG("NovaDAW: Engine version = " + engine->getVersion().toString());
}

EngineManager::~EngineManager()
{
    DBG("NovaDAW: Shutting down Tracktion Engine.");
    engine.reset();
}

te::DeviceManager& EngineManager::getDeviceManager()
{
    return engine->getDeviceManager();
}

void EngineManager::showAudioSettings(juce::Component* parent)
{
    // Create a dialog showing audio/MIDI device settings
    auto& deviceManager = engine->getDeviceManager();

    // Use JUCE's built-in AudioDeviceSelectorComponent
    auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        deviceManager.deviceManager,
        0, 512,    // min/max input channels
        0, 512,    // min/max output channels
        true,      // show MIDI inputs
        true,      // show MIDI outputs
        true,      // show channels as stereo pairs
        false      // hide advanced options
    );

    selector->setSize(500, 550);

    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle          = "Audio & MIDI Settings";
    options.dialogBackgroundColour = juce::Colour(0xFF1A1A28);
    options.content.setOwned(selector.release());
    options.componentToCentreAround = parent;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar    = true;
    options.resizable            = false;

    options.launchAsync();
}

juce::StringArray EngineManager::getAvailableDeviceTypes() const
{
    juce::StringArray types;
    for (auto* type : engine->getDeviceManager().deviceManager.getAvailableDeviceTypes())
        types.add(type->getTypeName());
    return types;
}

void EngineManager::scanForPlugins()
{
    auto& pluginManager = engine->getPluginManager();
    // Trigger a background scan for VST3 and other plugin formats
    juce::OwnedArray<juce::PluginDescription> newPlugins;
    pluginManager.knownPluginList.scanAndAddDragAndDroppedFiles(
        pluginManager.pluginFormatManager,
        juce::StringArray(),
        newPlugins
    );
    DBG("NovaDAW: Plugin scan initiated.");
}

te::PluginManager& EngineManager::getPluginManager()
{
    return engine->getPluginManager();
}

} // namespace nova
