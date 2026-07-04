#include "TimelineView.h"
#include "Utils/Helpers.h"

namespace nova
{

TimelineView::TimelineView(te::Edit& e, TransportViewModel& tvm)
    : edit(e), transportVM(tvm)
{
    formatManager.registerBasicFormats();
    transportVM.addListener(this);
    refreshTracks();
    startTimerHz(constants::TIMER_HZ_UI);
}

TimelineView::~TimelineView()
{
    transportVM.removeListener(this);
    stopTimer();
}

// ─── Paint ──────────────────────────────────────────────────────────────────

void TimelineView::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Overall background
    g.fillAll(colors::BG_DARKEST);

    // Ruler area at top (24px height)
    auto rulerArea = bounds.removeFromTop(24);
    drawRuler(g, rulerArea);

    // Track area (everything else, offset for headers)
    auto headerWidth = constants::TRACK_HEADER_WIDTH;
    auto trackArea = bounds.withTrimmedLeft(headerWidth);

    drawGridLines(g, trackArea);
    drawClips(g, trackArea);
    drawPlayhead(g, bounds);  // Full height playhead including headers
}

void TimelineView::resized()
{
    // Position track headers on the left side
    int y = 24; // Below ruler
    for (auto* header : trackHeaders)
    {
        header->setBounds(0, y, constants::TRACK_HEADER_WIDTH, constants::TRACK_HEIGHT);
        y += constants::TRACK_HEIGHT;
    }
}

// ─── Mouse Handling ─────────────────────────────────────────────────────────

void TimelineView::mouseDown(const juce::MouseEvent& e)
{
    // Click on timeline to set playhead position
    if (e.x > constants::TRACK_HEADER_WIDTH)
    {
        double clickSeconds = pixelsToSeconds(e.x - constants::TRACK_HEADER_WIDTH) + scrollOffsetSeconds;
        transportVM.setCurrentPositionSeconds(juce::jmax(0.0, clickSeconds));
    }
}

void TimelineView::mouseDrag(const juce::MouseEvent& e)
{
    // Drag to scrub playhead
    if (e.x > constants::TRACK_HEADER_WIDTH)
    {
        double clickSeconds = pixelsToSeconds(e.x - constants::TRACK_HEADER_WIDTH) + scrollOffsetSeconds;
        transportVM.setCurrentPositionSeconds(juce::jmax(0.0, clickSeconds));
    }
}

void TimelineView::mouseWheelMove(const juce::MouseEvent& e,
                                   const juce::MouseWheelDetails& wheel)
{
    if (e.mods.isCtrlDown())
    {
        // Ctrl + scroll = zoom
        double zoomFactor = 1.0 + wheel.deltaY * 0.3;
        setPixelsPerBeat(pixelsPerBeat * zoomFactor);
    }
    else
    {
        // Normal scroll = horizontal scroll
        double scrollDelta = wheel.deltaY * 50.0;
        setScrollOffsetSeconds(scrollOffsetSeconds - pixelsToSeconds(scrollDelta));
    }
}

// ─── Drag & Drop ────────────────────────────────────────────────────────────

bool TimelineView::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& file : files)
    {
        if (helpers::isSupportedAudioFile(juce::File(file)))
            return true;
    }
    return false;
}

void TimelineView::filesDropped(const juce::StringArray& files, int x, int y)
{
    // Determine which track was targeted
    int trackIndex = (y - 24) / constants::TRACK_HEIGHT;
    auto tracks = te::getAudioTracks(edit);

    if (trackIndex < 0 || trackIndex >= tracks.size())
        return;

    auto* track = tracks[trackIndex];
    double dropTime = pixelsToSeconds(x - constants::TRACK_HEADER_WIDTH) + scrollOffsetSeconds;
    dropTime = juce::jmax(0.0, dropTime);

    for (auto& filePath : files)
    {
        juce::File audioFile(filePath);
        if (helpers::isSupportedAudioFile(audioFile))
        {
            // Read file to get its length
            if (auto reader = std::unique_ptr<juce::AudioFormatReader>(
                    formatManager.createReaderFor(audioFile)))
            {
                double length = reader->lengthInSamples / reader->sampleRate;

                auto clipRange = te::TimeRange(
                    te::TimePosition::fromSeconds(dropTime),
                    te::TimePosition::fromSeconds(dropTime + length)
                );

                track->insertWaveClip(audioFile.getFileNameWithoutExtension(),
                                      audioFile, {{ clipRange }}, false);

                dropTime += length; // Stack clips if multiple files
            }
        }
    }

    repaint();
}

// ─── Transport Listener ─────────────────────────────────────────────────────

void TimelineView::transportStateChanged()
{
    repaint();
}

void TimelineView::transportPositionChanged(double positionSeconds)
{
    playheadPosition = positionSeconds;

    // Auto-scroll if playhead goes off-screen
    double playheadPixelX = secondsToPixels(playheadPosition - scrollOffsetSeconds);
    double viewWidth = getWidth() - constants::TRACK_HEADER_WIDTH;

    if (playheadPixelX > viewWidth * 0.9)
    {
        scrollOffsetSeconds = playheadPosition - pixelsToSeconds(viewWidth * 0.2);
    }
}

void TimelineView::timerCallback()
{
    if (transportVM.isPlaying() || transportVM.isRecording())
        repaint();
}

// ─── Track Management ───────────────────────────────────────────────────────

void TimelineView::refreshTracks()
{
    trackHeaders.clear();

    auto tracks = te::getAudioTracks(edit);
    for (int i = 0; i < tracks.size(); ++i)
    {
        auto colour = colors::TRACK_COLORS[i % colors::NUM_TRACK_COLORS];
        auto* header = new TrackHeaderView(*tracks[i], colour);
        addAndMakeVisible(header);
        trackHeaders.add(header);
    }

    resized();
    repaint();
}

// ─── Zoom & Scroll ──────────────────────────────────────────────────────────

void TimelineView::setPixelsPerBeat(double ppb)
{
    pixelsPerBeat = juce::jlimit(constants::MIN_PIXELS_PER_BEAT,
                                  constants::MAX_PIXELS_PER_BEAT, ppb);
    repaint();
}

void TimelineView::setScrollOffsetSeconds(double offset)
{
    scrollOffsetSeconds = juce::jmax(0.0, offset);
    repaint();
}

// ─── Drawing: Ruler ─────────────────────────────────────────────────────────

void TimelineView::drawRuler(juce::Graphics& g, juce::Rectangle<int> rulerArea)
{
    g.setColour(colors::BG_DARK);
    g.fillRect(rulerArea);

    g.setColour(colors::BORDER);
    g.drawHorizontalLine(rulerArea.getBottom() - 1, 0.0f,
                          static_cast<float>(rulerArea.getWidth()));

    // Draw bar numbers and beat markers
    auto area = rulerArea.withTrimmedLeft(constants::TRACK_HEADER_WIDTH);
    double bpm = transportVM.getBPM();
    double secondsPerBeat = 60.0 / bpm;
    int beatsPerBar = transportVM.getTimeSigNumerator();

    // Find the first visible beat
    double firstBeat = scrollOffsetSeconds / secondsPerBeat;
    int startBeat = static_cast<int>(std::floor(firstBeat));

    g.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));

    for (int beat = startBeat; ; ++beat)
    {
        double beatSeconds = beat * secondsPerBeat;
        double px = secondsToPixels(beatSeconds - scrollOffsetSeconds);

        if (px > area.getWidth()) break;
        if (px < 0) continue;

        float xPos = area.getX() + static_cast<float>(px);

        bool isBarLine = (beat % beatsPerBar) == 0;

        if (isBarLine)
        {
            // Bar number
            int bar = (beat / beatsPerBar) + 1;
            g.setColour(colors::TEXT_SECONDARY);
            g.drawText(juce::String(bar),
                       static_cast<int>(xPos) + 4, rulerArea.getY(),
                       40, rulerArea.getHeight(),
                       juce::Justification::centredLeft);

            // Tick mark
            g.setColour(colors::TEXT_MUTED);
            g.drawVerticalLine(static_cast<int>(xPos), 
                               static_cast<float>(rulerArea.getY() + rulerArea.getHeight() / 2),
                               static_cast<float>(rulerArea.getBottom()));
        }
        else
        {
            // Beat tick
            g.setColour(colors::BG_LIGHTER);
            g.drawVerticalLine(static_cast<int>(xPos),
                               static_cast<float>(rulerArea.getBottom() - 6),
                               static_cast<float>(rulerArea.getBottom()));
        }
    }
}

// ─── Drawing: Grid Lines ────────────────────────────────────────────────────

void TimelineView::drawGridLines(juce::Graphics& g, juce::Rectangle<int> trackArea)
{
    double bpm = transportVM.getBPM();
    double secondsPerBeat = 60.0 / bpm;
    int beatsPerBar = transportVM.getTimeSigNumerator();

    double firstBeat = scrollOffsetSeconds / secondsPerBeat;
    int startBeat = static_cast<int>(std::floor(firstBeat));

    for (int beat = startBeat; ; ++beat)
    {
        double beatSeconds = beat * secondsPerBeat;
        double px = secondsToPixels(beatSeconds - scrollOffsetSeconds);

        if (px > trackArea.getWidth()) break;
        if (px < 0) continue;

        float xPos = trackArea.getX() + static_cast<float>(px);
        bool isBarLine = (beat % beatsPerBar) == 0;

        g.setColour(isBarLine ? colors::BORDER : colors::BORDER.withAlpha(0.3f));
        g.drawVerticalLine(static_cast<int>(xPos),
                           static_cast<float>(trackArea.getY()),
                           static_cast<float>(trackArea.getBottom()));
    }

    // Horizontal track separators
    auto tracks = te::getAudioTracks(edit);
    for (int i = 0; i <= tracks.size(); ++i)
    {
        int y = trackArea.getY() + i * constants::TRACK_HEIGHT;
        g.setColour(colors::BORDER);
        g.drawHorizontalLine(y, static_cast<float>(trackArea.getX()),
                              static_cast<float>(trackArea.getRight()));
    }
}

// ─── Drawing: Clips ─────────────────────────────────────────────────────────

void TimelineView::drawClips(juce::Graphics& g, juce::Rectangle<int> trackArea)
{
    auto tracks = te::getAudioTracks(edit);

    for (int trackIdx = 0; trackIdx < tracks.size(); ++trackIdx)
    {
        auto* track = tracks[trackIdx];
        int trackY = trackArea.getY() + trackIdx * constants::TRACK_HEIGHT;
        auto trackColour = colors::TRACK_COLORS[trackIdx % colors::NUM_TRACK_COLORS];

        for (auto clip : track->getClips())
        {
            double clipStart = clip->getPosition().getStart().inSeconds();
            double clipEnd   = clip->getPosition().getEnd().inSeconds();

            // Convert to pixel coordinates
            double startPx = secondsToPixels(clipStart - scrollOffsetSeconds) + trackArea.getX();
            double endPx   = secondsToPixels(clipEnd - scrollOffsetSeconds) + trackArea.getX();

            // Skip clips outside visible area
            if (endPx < trackArea.getX() || startPx > trackArea.getRight())
                continue;

            auto clipBounds = juce::Rectangle<int>(
                static_cast<int>(startPx), trackY + 2,
                static_cast<int>(endPx - startPx), constants::TRACK_HEIGHT - 4);

            // Clip background
            g.setColour(trackColour.withAlpha(0.2f));
            g.fillRoundedRectangle(clipBounds.toFloat(), 4.0f);

            // Clip border
            g.setColour(trackColour.withAlpha(0.6f));
            g.drawRoundedRectangle(clipBounds.toFloat(), 4.0f, 1.0f);

            // Clip name
            g.setColour(colors::TEXT_PRIMARY.withAlpha(0.8f));
            g.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
            g.drawText(clip->getName(), clipBounds.reduced(6, 2),
                       juce::Justification::topLeft);

            // Draw waveform if it's a wave clip
            if (auto* waveClip = dynamic_cast<te::WaveAudioClip*>(clip))
            {
                drawWaveformForClip(g, *waveClip, clipBounds.reduced(2, 14));
            }
        }
    }
}

void TimelineView::drawWaveformForClip(juce::Graphics& g, te::WaveAudioClip& clip,
                                        juce::Rectangle<int> clipBounds)
{
    auto file = clip.getSourceFileReference().getFile();
    if (!file.existsAsFile()) return;

    // Create a thumbnail for this clip
    juce::AudioThumbnail thumbnail(512, formatManager, thumbnailCache);
    thumbnail.setSource(new juce::FileInputSource(file));

    if (thumbnail.getTotalLength() <= 0.0) return;

    auto trackColour = colors::WAVEFORM_FILL;

    // Draw the waveform
    g.setColour(trackColour.withAlpha(0.6f));
    thumbnail.drawChannels(g, clipBounds, 0.0, thumbnail.getTotalLength(), 1.0f);
}

// ─── Drawing: Playhead ──────────────────────────────────────────────────────

void TimelineView::drawPlayhead(juce::Graphics& g, juce::Rectangle<int> area)
{
    double playheadPx = secondsToPixels(playheadPosition - scrollOffsetSeconds)
                        + constants::TRACK_HEADER_WIDTH;

    if (playheadPx < constants::TRACK_HEADER_WIDTH || playheadPx > area.getRight())
        return;

    float x = static_cast<float>(playheadPx);

    // Playhead line
    g.setColour(colors::PLAYHEAD);
    g.drawVerticalLine(static_cast<int>(x), 0.0f, static_cast<float>(area.getBottom()));

    // Playhead triangle at top
    juce::Path triangle;
    triangle.addTriangle(x - 5.0f, 0.0f, x + 5.0f, 0.0f, x, 8.0f);
    g.setColour(colors::PLAYHEAD);
    g.fillPath(triangle);
}

// ─── Coordinate Conversion ──────────────────────────────────────────────────

double TimelineView::pixelsToSeconds(double pixels) const
{
    double bpm = transportVM.getBPM();
    double secondsPerBeat = 60.0 / bpm;
    return (pixels / pixelsPerBeat) * secondsPerBeat;
}

double TimelineView::secondsToPixels(double seconds) const
{
    double bpm = transportVM.getBPM();
    double beatsPerSecond = bpm / 60.0;
    return seconds * beatsPerSecond * pixelsPerBeat;
}

int TimelineView::getTrackYPosition(int trackIndex) const
{
    return 24 + trackIndex * constants::TRACK_HEIGHT; // 24 = ruler height
}

} // namespace nova
