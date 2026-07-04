cmake_minimum_required(VERSION 3.25)

project(NovaDAW
    VERSION 0.1.0
    DESCRIPTION "Abduction Studio — A Modern Digital Audio Workstation"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─── JUCE ────────────────────────────────────────────────────────────────────
# JUCE must be added BEFORE Tracktion Engine so TE detects it and skips
# its own bundled JUCE copy (via `if(NOT TARGET juce::juce_core)`).
add_subdirectory(libs/JUCE)

# ─── Tracktion Engine ────────────────────────────────────────────────────────
# Tracktion Engine is a set of JUCE modules with its own CMakeLists.txt.
# It uses juce_add_modules() internally. We disable examples to speed up build.
set(TE_ADD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/tracktion_engine)

# ─── MSVC Fixes ─────────────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(/utf-8)
endif()

# ─── NovaDAW Application ────────────────────────────────────────────────────
juce_add_gui_app(NovaDAW
    PRODUCT_NAME "Abduction Studio"
    COMPANY_NAME "Abduction Studio"
    BUNDLE_ID "com.abductionstudio.app"
    VERSION "${PROJECT_VERSION}"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png"
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    NEEDS_CURL FALSE
    NEEDS_WEB_BROWSER FALSE
)

# ─── Source Files ────────────────────────────────────────────────────────────
target_sources(NovaDAW PRIVATE
    # Entry Point
    src/Main.cpp

    # Application
    src/App/NovaDAWApplication.h
    src/App/NovaDAWApplication.cpp

    # Engine Layer
    src/Engine/EngineManager.h
    src/Engine/EngineManager.cpp
    src/Engine/EditManager.h
    src/Engine/EditManager.cpp
    src/Engine/DJEngineManager.h
    src/Engine/DJEngineManager.cpp

    # ViewModel Layer
    src/ViewModel/TransportViewModel.h
    src/ViewModel/TransportViewModel.cpp

    # UI Layer
    src/UI/MainWindow.h
    src/UI/MainWindow.cpp
    src/UI/MainComponent.h
    src/UI/MainComponent.cpp
    src/UI/LookAndFeel/NovaLookAndFeel.h
    src/UI/LookAndFeel/NovaLookAndFeel.cpp
    src/UI/Transport/TransportBar.h
    src/UI/Transport/TransportBar.cpp
    src/UI/Timeline/TimelineView.h
    src/UI/Timeline/TimelineView.cpp
    src/UI/Timeline/TrackHeaderView.h
    src/UI/Timeline/TrackHeaderView.cpp
    src/UI/Mixer/MixerView.h
    src/UI/Mixer/MixerView.cpp
    src/UI/DJMode/DJModeView.cpp
    src/UI/DebugWindow.h
    src/UI/DebugWindow.cpp

    # Utils
    src/Utils/Constants.h
    src/Utils/Helpers.h
)

# ─── Preprocessor Definitions ───────────────────────────────────────────────
target_compile_definitions(NovaDAW PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:NovaDAW,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:NovaDAW,JUCE_VERSION>"
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_AU=1
    JUCE_PLUGINHOST_LADSPA=0
    TRACKTION_ENABLE_TIMESTRETCH_SOUNDTOUCH=1
)

# ─── Include Directories ────────────────────────────────────────────────────
target_include_directories(NovaDAW PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ─── Link Libraries ─────────────────────────────────────────────────────────
# Link against JUCE modules and Tracktion Engine modules.
# Tracktion modules use the tracktion:: namespace alias (from juce_add_modules ALIAS_NAMESPACE).
target_link_libraries(NovaDAW PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    tracktion::tracktion_core
    tracktion::tracktion_engine
    tracktion::tracktion_graph
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
