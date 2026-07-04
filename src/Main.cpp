// ─── NovaDAW Entry Point ─────────────────────────────────────────────────────
// This is the main entry point for the NovaDAW application.
// JUCE's START_JUCE_APPLICATION macro creates the platform-specific
// main() / WinMain() function automatically.

#include "App/NovaDAWApplication.h"

// This macro generates the appropriate main() function for each platform
// and creates an instance of NovaDAWApplication.
START_JUCE_APPLICATION(nova::NovaDAWApplication)
