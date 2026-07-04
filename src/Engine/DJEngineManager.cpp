#include "DJEngineManager.h"

namespace nova
{

DJEngineManager::DJEngineManager(EngineManager& engineManager)
    : engineMgr(engineManager)
{
    editA = createDeckEdit();
    editB = createDeckEdit();
}

DJEngineManager::~DJEngineManager()
{
    if (editA) editA->getTempDirectory(false).deleteRecursively();
    if (editB) editB->getTempDirectory(false).deleteRecursively();
}

std::unique_ptr<te::Edit> DJEngineManager::createDeckEdit()
{
    te::Edit::Options options =
    {
        engineMgr.getEngine(),
        te::createEmptyEdit(engineMgr.getEngine()),
        te::ProjectItemID::createNewID(0)
    };

    auto edit = std::make_unique<te::Edit>(options);
    edit->playInStopEnabled = true;
    edit->ensureNumberOfAudioTracks(1);
    
    // Disable syncing to other transports
    edit->getTransport().position = te::TimePosition::fromSeconds(0.0);
    
    return edit;
}

bool DJEngineManager::loadTrackToDeck(int deckIndex, const juce::File& audioFile)
{
    auto* edit = getDeckEdit(deckIndex);
    if (!edit || !audioFile.existsAsFile()) return false;

    auto tracks = te::getAudioTracks(*edit);
    if (tracks.isEmpty()) return false;
    
    auto* track = tracks.getFirst();
    
    // Clear existing clips
    auto clips = track->getClips();
    for (int i = clips.size() - 1; i >= 0; --i)
    {
        clips[i]->removeFromParent();
    }

    // Load new clip
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    double lengthSeconds = 0.0;
    if (auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(audioFile)))
    {
        lengthSeconds = reader->lengthInSamples / reader->sampleRate;
    }
    else
    {
        return false;
    }

    auto clipRange = te::TimeRange(te::TimePosition::fromSeconds(0.0), te::TimePosition::fromSeconds(lengthSeconds));
    auto clip = track->insertWaveClip("Track", audioFile, {{ clipRange }}, false);
    
    if (clip)
    {
        // Setup initial volume
        updateVolumes();
        
        // Ensure transport is stopped and at beginning
        edit->getTransport().stop(false, false);
        edit->getTransport().position = te::TimePosition::fromSeconds(0.0);
        return true;
    }

    return false;
}

void DJEngineManager::playDeck(int deckIndex)
{
    if (auto* edit = getDeckEdit(deckIndex))
        edit->getTransport().play(false);
}

void DJEngineManager::pauseDeck(int deckIndex)
{
    if (auto* edit = getDeckEdit(deckIndex))
        edit->getTransport().stop(false, false);
}

void DJEngineManager::stopDeck(int deckIndex)
{
    if (auto* edit = getDeckEdit(deckIndex))
    {
        edit->getTransport().stop(false, false);
        edit->getTransport().position = te::TimePosition::fromSeconds(0.0);
    }
}

void DJEngineManager::setDeckVolume(int deckIndex, float volume)
{
    if (deckIndex == 0) volA = volume;
    else volB = volume;
    
    updateVolumes();
}

void DJEngineManager::setCrossfader(float position)
{
    crossfaderPos = juce::jlimit(-1.0f, 1.0f, position);
    updateVolumes();
}

void DJEngineManager::updateVolumes()
{
    // Crossfader logic (simple constant power or linear)
    // -1.0 -> Deck A 100%, Deck B 0%
    //  0.0 -> Deck A 100%, Deck B 100%
    //  1.0 -> Deck A 0%, Deck B 100%
    
    float crossA = 1.0f;
    float crossB = 1.0f;
    
    if (crossfaderPos > 0.0f)
        crossA = 1.0f - crossfaderPos;
    else if (crossfaderPos < 0.0f)
        crossB = 1.0f + crossfaderPos;

    if (editA && !te::getAudioTracks(*editA).isEmpty())
        te::getAudioTracks(*editA).getFirst()->getVolumePlugin()->setVolumeDb(juce::Decibels::gainToDecibels(volA * crossA));
        
    if (editB && !te::getAudioTracks(*editB).isEmpty())
        te::getAudioTracks(*editB).getFirst()->getVolumePlugin()->setVolumeDb(juce::Decibels::gainToDecibels(volB * crossB));
}

void DJEngineManager::setDeckPitch(int deckIndex, float pitchOffset)
{
    // pitchOffset from -1.0 to 1.0
    // Maps to speed ratio. Let's say -100% to +100% (or realistic -16% to +16%)
    // Let's do a realistic DJ pitch: +/- 16%
    float pitchPercent = pitchOffset * 0.16f; 
    double speedRatio = 1.0 + pitchPercent;

    if (auto* edit = getDeckEdit(deckIndex))
    {
        auto tracks = te::getAudioTracks(*edit);
        if (!tracks.isEmpty())
        {
            for (auto* clip : tracks.getFirst()->getClips())
            {
                if (auto* waveClip = dynamic_cast<te::WaveAudioClip*>(clip))
                {
                    waveClip->setSpeedRatio(speedRatio);
                }
            }
        }
    }
}

} // namespace nova
