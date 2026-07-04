#pragma once

// ─── EditManager ────────────────────────────────────────────────────────────
// Manages a te::Edit, which represents a DAW project/session.
// Handles track creation, clip management, and project I/O.

#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>
#include "EngineManager.h"

namespace te = tracktion;

namespace nova
{

class EditManager
{
public:
    explicit EditManager(EngineManager& engineManager);
    ~EditManager();

    /// Create a new empty Edit (new project)
    void createNewEdit();

    /// Load an Edit from a file
    bool loadEdit(const juce::File& file);

    /// Save the current Edit
    bool saveEdit();

    /// Save the current Edit to a specific file
    bool saveEditAs(const juce::File& file);

    /// Get the current Edit (may be nullptr if not loaded)
    te::Edit* getEdit() { return edit.get(); }

    /// Get the transport control for playback
    te::TransportControl* getTransport();

    // ─── Track Management ───────────────────────────────────────────────

    /// Add a new audio track to the edit
    te::AudioTrack* addAudioTrack(const juce::String& name = {});

    /// Delete a track by index
    void deleteTrack(int index);

    /// Get number of audio tracks
    int getNumAudioTracks() const;

    // ─── Clip Management ────────────────────────────────────────────────

    /// Add an audio clip from a file to a specific track at a given time
    te::Clip* addAudioClip(te::AudioTrack& track,
                           const juce::File& audioFile,
                           double startTimeSeconds,
                           double lengthSeconds = -1.0);

    /// Get the length of the edit in seconds
    double getEditLength() const;

    // ─── Project File ───────────────────────────────────────────────────

    /// Get the current project file
    juce::File getProjectFile() const { return projectFile; }

    /// Check if the edit has unsaved changes
    bool hasUnsavedChanges() const;

private:
    EngineManager& engineMgr;
    std::unique_ptr<te::Edit> edit;
    juce::File projectFile;

    /// Creates the default Edit with a few empty tracks
    void setupDefaultEdit();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditManager)
};

} // namespace nova
