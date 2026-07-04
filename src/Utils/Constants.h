#pragma once

// ─── NovaDAW Constants ───────────────────────────────────────────────────────
// Central location for all application-wide constants, colors, and sizes.

#include <juce_gui_basics/juce_gui_basics.h>

namespace nova
{
namespace constants
{

// ─── Application ────────────────────────────────────────────────────────────
inline constexpr const char* APP_NAME    = "Abduction Studio";
inline constexpr const char* APP_VERSION = "0.1.0";

// ─── Window ─────────────────────────────────────────────────────────────────
inline constexpr int DEFAULT_WINDOW_WIDTH  = 1400;
inline constexpr int DEFAULT_WINDOW_HEIGHT = 850;
inline constexpr int MIN_WINDOW_WIDTH      = 900;
inline constexpr int MIN_WINDOW_HEIGHT     = 600;

// ─── Layout Sizes ───────────────────────────────────────────────────────────
inline constexpr int TRANSPORT_BAR_HEIGHT  = 56;
inline constexpr int TRACK_HEADER_WIDTH    = 200;
inline constexpr int TRACK_HEIGHT          = 80;
inline constexpr int MIXER_DEFAULT_HEIGHT  = 220;
inline constexpr int MIXER_STRIP_WIDTH     = 80;
inline constexpr int SCROLLBAR_WIDTH       = 10;

// ─── Timeline ───────────────────────────────────────────────────────────────
inline constexpr double DEFAULT_BPM              = 120.0;
inline constexpr double DEFAULT_PIXELS_PER_BEAT  = 60.0;
inline constexpr double MIN_PIXELS_PER_BEAT      = 10.0;
inline constexpr double MAX_PIXELS_PER_BEAT      = 300.0;

// ─── Audio ──────────────────────────────────────────────────────────────────
inline constexpr double DEFAULT_SAMPLE_RATE = 44100.0;
inline constexpr int    DEFAULT_BUFFER_SIZE = 512;

// ─── Animation ──────────────────────────────────────────────────────────────
inline constexpr int TIMER_HZ_UI     = 30;  // UI refresh rate
inline constexpr int TIMER_HZ_METERS = 60;  // VU meter refresh rate

} // namespace constants

// ─── Color Palette (Dark Theme) ─────────────────────────────────────────────
// Inspired by Bitwig Studio / Ableton Live dark modes
namespace colors
{

// Background layers (darkest to lightest)
inline const juce::Colour BG_DARKEST     { 0xFF06040A };  // Deep space / Abyss
inline const juce::Colour BG_DARK        { 0xFF0D0A14 };  // Dark gothic panel
inline const juce::Colour BG_MID         { 0xFF161224 };  // Soul plate
inline const juce::Colour BG_LIGHT       { 0xFF221A33 };  // Elevated gothic plate
inline const juce::Colour BG_LIGHTER     { 0xFF2F2445 };  // Highlighted plate

// Surface / Cards
inline const juce::Colour SURFACE        { 0xFF120E1C };
inline const juce::Colour SURFACE_HOVER  { 0xFF1A1429 };
inline const juce::Colour SURFACE_ACTIVE { 0xFF251D3A };

// Borders & Dividers
inline const juce::Colour BORDER         { 0xFF030205 };
inline const juce::Colour BORDER_LIGHT   { 0xFF3D2A54 };

// Text
inline const juce::Colour TEXT_PRIMARY   { 0xFF00FFF0 };  // Cyan Neon (Sci-fi)
inline const juce::Colour TEXT_SECONDARY { 0xFFFF2A5F };  // Crimson Soul
inline const juce::Colour TEXT_MUTED     { 0xFF4A3C6B };  // Purple Gray
inline const juce::Colour TEXT_ON_ACCENT { 0xFF06040A };

// Accent Colors
inline const juce::Colour ACCENT_PRIMARY   { 0xFFFF0055 };  // Neon Crimson (Blood/Soul)
inline const juce::Colour ACCENT_SECONDARY { 0xFFB300FF };  // Deep Magenta / Purple
inline const juce::Colour ACCENT_WARM      { 0xFF00E5FF };  // Cyan (Sci-Fi contrast)

// Transport Colors
inline const juce::Colour PLAY_GREEN    { 0xFF00FFCC };  // Cyan/Turquoise Play
inline const juce::Colour STOP_RED      { 0xFFFF0055 };  // Crimson Stop
inline const juce::Colour RECORD_RED    { 0xFFFF2A5F };
inline const juce::Colour LOOP_YELLOW   { 0xFFB300FF };  // Magenta Loop

// Meter Colors (gradient: cyan → magenta → crimson)
inline const juce::Colour METER_GREEN   { 0xFF00FFF0 };  // Cyan Base
inline const juce::Colour METER_YELLOW  { 0xFFB300FF };  // Magenta Mid
inline const juce::Colour METER_RED     { 0xFFFF0055 };  // Crimson Peak
inline const juce::Colour METER_BG      { 0xFF06040A };

// Track Colors (for assigning to tracks)
inline const juce::Colour TRACK_COLORS[] = {
    juce::Colour(0xFF00E5FF),  // Cyan
    juce::Colour(0xFFFFB300),  // Amber
    juce::Colour(0xFFFF3300),  // Red
    juce::Colour(0xFF39FF14),  // Acid Green
    juce::Colour(0xFF8A2BE2),  // Cosmic Purple
    juce::Colour(0xFF00FFCC),  // Turquoise
    juce::Colour(0xFFFF007F),  // Neon Pink
    juce::Colour(0xFFFECA57),  // Gold
    juce::Colour(0xFF48DBFB),  // Sky Blue
    juce::Colour(0xFFFF9FF3),  // Light Pink
};
inline constexpr int NUM_TRACK_COLORS = 10;

// Waveform
inline const juce::Colour WAVEFORM_FILL   { 0xFFFF0055 };  // Crimson
inline const juce::Colour WAVEFORM_OUTLINE { 0xFFFF6699 };
inline const juce::Colour PLAYHEAD         { 0xFF00FFF0 }; // Cyan Playhead

} // namespace colors
} // namespace nova
