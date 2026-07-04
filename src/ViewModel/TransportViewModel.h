#pragma once

// ─── TransportViewModel ─────────────────────────────────────────────────────
// ViewModel for transport controls — bridges the UI transport bar with
// the Tracktion Engine's TransportControl.

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

namespace nova
{

class TransportViewModel : public juce::Timer
{
public:
    explicit TransportViewModel(te::Edit& edit);
    ~TransportViewModel() override;

    // ─── Transport Actions ──────────────────────────────────────────────
    void play();
    void stop();
    void record();
    void togglePlayStop();
    void returnToStart();

    // ─── Loop ───────────────────────────────────────────────────────────
    void toggleLoop();
    bool isLooping() const;

    // ─── State Queries ──────────────────────────────────────────────────
    bool isPlaying() const;
    bool isRecording() const;
    bool isStopped() const;

    // ─── Position ───────────────────────────────────────────────────────
    double getCurrentPositionSeconds() const;
    void setCurrentPositionSeconds(double seconds);

    // ─── Tempo ──────────────────────────────────────────────────────────
    double getBPM() const;
    void setBPM(double bpm);

    // ─── Time Signature ─────────────────────────────────────────────────
    int getTimeSigNumerator() const;
    int getTimeSigDenominator() const;

    // ─── Listener ───────────────────────────────────────────────────────
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void transportStateChanged() = 0;
        virtual void transportPositionChanged(double positionSeconds) = 0;
    };

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    void timerCallback() override;

    te::Edit& edit;
    te::TransportControl& transport;
    juce::ListenerList<Listener> listeners;

    bool lastPlayState = false;
    double lastPosition = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportViewModel)
};

} // namespace nova
