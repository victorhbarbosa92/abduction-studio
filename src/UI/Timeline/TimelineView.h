#pragma once

// ─── TimelineView ───────────────────────────────────────────────────────────
// The main timeline area where audio/MIDI clips are displayed as waveforms
// on tracks. Includes playhead, grid lines, ruler, and drag-drop support.

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <tracktion_engine/tracktion_engine.h>
#include "UI/Timeline/TrackHeaderView.h"
#include "ViewModel/TransportViewModel.h"
#include "Utils/Constants.h"

namespace te = tracktion;

namespace nova
{

class TimelineView : public juce::Component,
                     public TransportViewModel::Listener,
                     public juce::FileDragAndDropTarget,
                     public juce::Timer
{
public:
    TimelineView(te::Edit& edit, TransportViewModel& transportVM);
    ~TimelineView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e,
                        const juce::MouseWheelDetails& wheel) override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // TransportViewModel::Listener
    void transportStateChanged() override;
    void transportPositionChanged(double positionSeconds) override;

    // Timer (for playhead animation)
    void timerCallback() override;

    /// Rebuild track headers and layout when tracks change
    void refreshTracks();

    /// Get/set zoom level (pixels per beat)
    double getPixelsPerBeat() const { return pixelsPerBeat; }
    void setPixelsPerBeat(double ppb);

    /// Get/set scroll offset in seconds
    double getScrollOffsetSeconds() const { return scrollOffsetSeconds; }
    void setScrollOffsetSeconds(double offset);

private:
    te::Edit& edit;
    TransportViewModel& transportVM;

    // ─── State ──────────────────────────────────────────────────────────
    double pixelsPerBeat      = constants::DEFAULT_PIXELS_PER_BEAT;
    double scrollOffsetSeconds = 0.0;
    double playheadPosition    = 0.0;  // in seconds

    // ─── Track Headers ──────────────────────────────────────────────────
    juce::OwnedArray<TrackHeaderView> trackHeaders;

    // ─── Thumbnail Cache ────────────────────────────────────────────────
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache { 32 };

    // ─── Drawing Helpers ────────────────────────────────────────────────
    void drawRuler(juce::Graphics& g, juce::Rectangle<int> rulerArea);
    void drawGridLines(juce::Graphics& g, juce::Rectangle<int> trackArea);
    void drawClips(juce::Graphics& g, juce::Rectangle<int> trackArea);
    void drawWaveformForClip(juce::Graphics& g, te::WaveAudioClip& clip,
                             juce::Rectangle<int> clipBounds);
    void drawPlayhead(juce::Graphics& g, juce::Rectangle<int> area);

    // ─── Coordinate Conversion ──────────────────────────────────────────
    double pixelsToSeconds(double pixels) const;
    double secondsToPixels(double seconds) const;
    int getTrackYPosition(int trackIndex) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineView)
};

} // namespace nova
