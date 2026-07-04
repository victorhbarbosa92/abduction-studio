#pragma once

// ─── EngineManager ──────────────────────────────────────────────────────────
// Wrapper around te::Engine that manages the Tracktion Engine lifecycle.
// This is the core of NovaDAW's audio backend.

#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

namespace nova
{

class EngineManager
{
public:
    EngineManager();
    ~EngineManager();

    /// Get the Tracktion Engine instance
    te::Engine& getEngine() { return *engine; }

    /// Get the DeviceManager for audio/MIDI device configuration
    te::DeviceManager& getDeviceManager();

    /// Show the audio/MIDI settings dialog
    void showAudioSettings(juce::Component* parent);

    /// Get available audio device types
    juce::StringArray getAvailableDeviceTypes() const;

    /// Scan for available plugins (VST3, AU, etc.)
    void scanForPlugins();

    /// Get the plugin manager
    te::PluginManager& getPluginManager();

private:
    std::unique_ptr<te::Engine> engine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EngineManager)
};

} // namespace nova
