#include "TransportViewModel.h"
#include "Utils/Constants.h"

namespace nova
{

TransportViewModel::TransportViewModel(te::Edit& e)
    : edit(e),
      transport(e.getTransport())
{
    // Start a timer to poll transport state and position for UI updates
    startTimerHz(constants::TIMER_HZ_UI);
}

TransportViewModel::~TransportViewModel()
{
    stopTimer();
}

// ─── Transport Actions ──────────────────────────────────────────────────────

void TransportViewModel::play()
{
    transport.play(false);
}

void TransportViewModel::stop()
{
    transport.stop(false, false);
}

void TransportViewModel::record()
{
    transport.record(false);
}

void TransportViewModel::togglePlayStop()
{
    if (isPlaying())
        stop();
    else
        play();
}

void TransportViewModel::returnToStart()
{
    transport.setPosition(te::TimePosition::fromSeconds(0.0));
}

// ─── Loop ───────────────────────────────────────────────────────────────────

void TransportViewModel::toggleLoop()
{
    transport.looping.setValue(!transport.looping.get(), nullptr);
}

bool TransportViewModel::isLooping() const
{
    return transport.looping.get();
}

// ─── State Queries ──────────────────────────────────────────────────────────

bool TransportViewModel::isPlaying() const
{
    return transport.isPlaying();
}

bool TransportViewModel::isRecording() const
{
    return transport.isRecording();
}

bool TransportViewModel::isStopped() const
{
    return !transport.isPlaying() && !transport.isRecording();
}

// ─── Position ───────────────────────────────────────────────────────────────

double TransportViewModel::getCurrentPositionSeconds() const
{
    return transport.getPosition().inSeconds();
}

void TransportViewModel::setCurrentPositionSeconds(double seconds)
{
    transport.setPosition(te::TimePosition::fromSeconds(seconds));
}

// ─── Tempo ──────────────────────────────────────────────────────────────────

double TransportViewModel::getBPM() const
{
    return edit.tempoSequence.getTempo(0)->getBpm();
}

void TransportViewModel::setBPM(double bpm)
{
    bpm = juce::jlimit(20.0, 999.0, bpm);
    edit.tempoSequence.getTempo(0)->setBpm(bpm);
}

// ─── Time Signature ─────────────────────────────────────────────────────────

int TransportViewModel::getTimeSigNumerator() const
{
    return edit.tempoSequence.getTimeSig(0)->numerator;
}

int TransportViewModel::getTimeSigDenominator() const
{
    return edit.tempoSequence.getTimeSig(0)->denominator;
}

// ─── Listener ───────────────────────────────────────────────────────────────

void TransportViewModel::addListener(Listener* listener)
{
    listeners.add(listener);
}

void TransportViewModel::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

// ─── Timer Callback ─────────────────────────────────────────────────────────

void TransportViewModel::timerCallback()
{
    bool currentPlayState = isPlaying() || isRecording();
    double currentPosition = getCurrentPositionSeconds();

    // Notify listeners of state change
    if (currentPlayState != lastPlayState)
    {
        lastPlayState = currentPlayState;
        listeners.call(&Listener::transportStateChanged);
    }

    // Notify listeners of position change (always during playback, or on seek)
    if (std::abs(currentPosition - lastPosition) > 0.001)
    {
        lastPosition = currentPosition;
        listeners.call(&Listener::transportPositionChanged, currentPosition);
    }
}

} // namespace nova
