#include "EditManager.h"
#include "Utils/Helpers.h"

namespace nova
{

EditManager::EditManager(EngineManager& engineManager)
    : engineMgr(engineManager)
{
    createNewEdit();
}

EditManager::~EditManager()
{
    if (edit)
    {
        // Flush pending changes before destruction
        te::EditFileOperations(*edit).save(true, true, false);
        edit->getTempDirectory(false).deleteRecursively();
    }
}

void EditManager::createNewEdit()
{
    auto& engine = engineMgr.getEngine();

    // Create a temporary file for the new edit
    auto editFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                        .getChildFile("NovaDAW")
                        .getChildFile("Untitled.tracktionedit");
    editFile.getParentDirectory().createDirectory();

    // Create the Edit using Tracktion Engine
    te::Edit::Options options =
    {
        engine,
        te::createEmptyEdit(engine),  // Empty ValueTree for a new edit
        te::ProjectItemID::createNewID(0)
    };

    edit = std::make_unique<te::Edit>(options);
    edit->playInStopEnabled = true;

    projectFile = juce::File();  // No file yet (unsaved)

    setupDefaultEdit();

    DBG("NovaDAW: New edit created.");
}

bool EditManager::loadEdit(const juce::File& file)
{
    if (!file.existsAsFile())
    {
        DBG("NovaDAW: File not found: " + file.getFullPathName());
        return false;
    }

    auto& engine = engineMgr.getEngine();

    auto xml = juce::XmlDocument::parse(file);
    if (!xml)
    {
        DBG("NovaDAW: Failed to parse edit file.");
        return false;
    }

    auto valueTree = juce::ValueTree::fromXml(*xml);

    te::Edit::Options options =
    {
        engine,
        valueTree,
        te::ProjectItemID::createNewID(0)
    };

    edit = std::make_unique<te::Edit>(options);
    edit->playInStopEnabled = true;
    projectFile = file;

    DBG("NovaDAW: Edit loaded from " + file.getFullPathName());
    return true;
}

bool EditManager::saveEdit()
{
    if (!edit)
        return false;

    if (projectFile == juce::File())
    {
        // No file set yet — prompt user for file location
        juce::FileChooser chooser("Salvar Projeto",
                                   juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                                   "*.tracktionedit");

        if (chooser.browseForFileToSave(true))
        {
            projectFile = chooser.getResult();
        }
        else
        {
            return false;
        }
    }

    return saveEditAs(projectFile);
}

bool EditManager::saveEditAs(const juce::File& file)
{
    if (!edit)
        return false;

    // Use Tracktion's built-in save mechanism
    auto result = te::EditFileOperations(*edit).save(true, true, false);
    
    // Also write the edit state to our file
    auto state = edit->state;
    if (auto xml = state.createXml())
    {
        if (xml->writeTo(file))
        {
            projectFile = file;
            DBG("NovaDAW: Edit saved to " + file.getFullPathName());
            return true;
        }
    }

    DBG("NovaDAW: Failed to save edit.");
    return false;
}

te::TransportControl* EditManager::getTransport()
{
    return edit ? &edit->getTransport() : nullptr;
}

te::AudioTrack* EditManager::addAudioTrack(const juce::String& name)
{
    if (!edit)
        return nullptr;

    // Insert a new audio track
    edit->ensureNumberOfAudioTracks(getNumAudioTracks() + 1);

    auto tracks = te::getAudioTracks(*edit);
    if (auto* newTrack = tracks.getLast())
    {
        juce::String trackName = name.isNotEmpty()
                                     ? name
                                     : helpers::getNextTrackName(*edit);
        newTrack->setName(trackName);

        DBG("NovaDAW: Added track '" + trackName + "'");
        return newTrack;
    }

    return nullptr;
}

void EditManager::deleteTrack(int index)
{
    if (!edit)
        return;

    auto tracks = te::getAudioTracks(*edit);
    if (index >= 0 && index < tracks.size())
    {
        auto* track = tracks[index];
        edit->deleteTrack(track);
        DBG("NovaDAW: Deleted track at index " + juce::String(index));
    }
}

int EditManager::getNumAudioTracks() const
{
    if (!edit)
        return 0;
    return te::getAudioTracks(*edit).size();
}

te::Clip* EditManager::addAudioClip(te::AudioTrack& track,
                                     const juce::File& audioFile,
                                     double startTimeSeconds,
                                     double lengthSeconds)
{
    if (!edit)
        return nullptr;

    // Determine the clip length from the audio file if not specified
    if (lengthSeconds <= 0.0)
    {
        // Use the full file length
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        if (auto reader = std::unique_ptr<juce::AudioFormatReader>(
                formatManager.createReaderFor(audioFile)))
        {
            lengthSeconds = reader->lengthInSamples / reader->sampleRate;
        }
        else
        {
            DBG("NovaDAW: Cannot read audio file: " + audioFile.getFullPathName());
            return nullptr;
        }
    }

    // Create a time range for the clip
    auto clipRange = te::TimeRange(
        te::TimePosition::fromSeconds(startTimeSeconds),
        te::TimePosition::fromSeconds(startTimeSeconds + lengthSeconds)
    );

    // Insert the clip
    auto clip = track.insertWaveClip(
        audioFile.getFileNameWithoutExtension(),  // clip name
        audioFile,                                 // source file
        {{ clipRange }},                           // time range
        false                                      // don't delete the source file
    );

    if (clip)
    {
        DBG("NovaDAW: Added clip '" + audioFile.getFileName()
            + "' at " + juce::String(startTimeSeconds, 2) + "s");
    }

    return clip.get();
}

double EditManager::getEditLength() const
{
    if (!edit)
        return 0.0;
    return edit->getLength().inSeconds();
}

bool EditManager::hasUnsavedChanges() const
{
    if (!edit)
        return false;
        
    return edit->hasChangedSinceSaved();
}

void EditManager::setupDefaultEdit()
{
    if (!edit)
        return;

    // Set default tempo
    edit->tempoSequence.getTempo(0)->setBpm(120.0);

    // Create 4 default audio tracks
    for (int i = 0; i < 4; ++i)
    {
        addAudioTrack("Faixa " + juce::String(i + 1));
    }

    DBG("Abduction Studio: Default edit setup complete (4 tracks, 120 BPM).");
}

} // namespace nova
