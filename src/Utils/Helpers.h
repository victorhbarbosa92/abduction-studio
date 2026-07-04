#pragma once

// ─── NovaDAW Helpers ─────────────────────────────────────────────────────────
// Utility functions used throughout the application.

#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

namespace nova
{
namespace helpers
{

// ─── Time Formatting ────────────────────────────────────────────────────────

/// Formats seconds into MM:SS.ms display (e.g., "02:35.420")
inline juce::String formatTimeMMSS(double seconds)
{
    int mins = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    int ms   = static_cast<int>((seconds - std::floor(seconds)) * 1000.0);
    return juce::String::formatted("%02d:%02d.%03d", mins, secs, ms);
}

/// Formats seconds into Bars:Beats:Ticks based on tempo
inline juce::String formatTimeBarsBeats(double seconds, double bpm, int timeSigNum = 4, int timeSigDenom = 4)
{
    double beatsPerSecond = bpm / 60.0;
    double totalBeats = seconds * beatsPerSecond;
    int bars  = static_cast<int>(totalBeats / timeSigNum) + 1;
    int beats = static_cast<int>(std::fmod(totalBeats, timeSigNum)) + 1;
    int ticks = static_cast<int>(std::fmod(totalBeats * 960.0, 960.0));
    return juce::String::formatted("%d.%d.%03d", bars, beats, ticks);
}

// ─── Track Utilities ────────────────────────────────────────────────────────

/// Get all audio tracks from an Edit
inline juce::Array<te::AudioTrack*> getAudioTracks(te::Edit& edit)
{
    juce::Array<te::AudioTrack*> result;
    for (auto track : te::getAudioTracks(edit))
        result.add(track);
    return result;
}

/// Create a unique track name
inline juce::String getNextTrackName(te::Edit& edit, const juce::String& prefix = "Faixa")
{
    int index = te::getAudioTracks(edit).size() + 1;
    return prefix + " " + juce::String(index);
}

// ─── dB / Gain Conversion ───────────────────────────────────────────────────

inline float gainToDecibels(float gain)
{
    return juce::Decibels::gainToDecibels(gain, -100.0f);
}

inline float decibelsToGain(float dB)
{
    return juce::Decibels::decibelsToGain(dB, -100.0f);
}

/// Maps a 0..1 slider value to a dB value with a nice log curve
inline float sliderToDecibels(float sliderValue)
{
    if (sliderValue <= 0.0f) return -100.0f;
    // Use a power curve for more natural fader response
    float dB = 20.0f * std::log10(sliderValue);
    return juce::jlimit(-100.0f, 12.0f, dB);
}

inline float decibelsToSlider(float dB)
{
    if (dB <= -100.0f) return 0.0f;
    return std::pow(10.0f, dB / 20.0f);
}

// ─── File Utilities ─────────────────────────────────────────────────────────

/// Check if a file is a supported audio format
inline bool isSupportedAudioFile(const juce::File& file)
{
    auto ext = file.getFileExtension().toLowerCase();
    return ext == ".wav" || ext == ".aiff" || ext == ".aif"
        || ext == ".flac" || ext == ".ogg" || ext == ".mp3"
        || ext == ".wma" || ext == ".m4a";
}

} // namespace helpers
} // namespace nova
