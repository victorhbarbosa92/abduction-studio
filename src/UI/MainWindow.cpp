#include "MainWindow.h"

namespace nova
{

MainWindow::MainWindow(const juce::String& name, juce::Component* contentComponent)
    : DocumentWindow(name,
                     juce::Colour(colors::BG_DARKEST),
                     DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setContentNonOwned(contentComponent, true);
    setResizable(true, true);

    auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    if (display != nullptr)
    {
        auto displayArea = display->userArea;
        
        int targetWidth = constants::DEFAULT_WINDOW_WIDTH;
        int targetHeight = constants::DEFAULT_WINDOW_HEIGHT;
        
        int minW = juce::jmin((int)constants::MIN_WINDOW_WIDTH, displayArea.getWidth());
        int minH = juce::jmin((int)constants::MIN_WINDOW_HEIGHT, displayArea.getHeight());
        
        setResizeLimits(minW, minH, 4096, 2160);
        
        // Adapt for smaller screens by maximizing to user area
        if (displayArea.getWidth() <= 1280 || displayArea.getHeight() <= 768)
        {
            setBounds(displayArea);
        }
        else
        {
            targetWidth = juce::jmin(targetWidth, displayArea.getWidth() - 100);
            targetHeight = juce::jmin(targetHeight, displayArea.getHeight() - 100);
            centreWithSize(targetWidth, targetHeight);
        }
    }
    else
    {
        setResizeLimits(constants::MIN_WINDOW_WIDTH,
                        constants::MIN_WINDOW_HEIGHT,
                        4096, 2160);
        centreWithSize(constants::DEFAULT_WINDOW_WIDTH,
                       constants::DEFAULT_WINDOW_HEIGHT);
    }

    setVisible(true);
}

void MainWindow::closeButtonPressed()
{
    // Ask the application to quit
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

} // namespace nova
