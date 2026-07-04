#include "DebugWindow.h"

namespace nova
{

DebugWindow::DebugWindow()
    : DocumentWindow("Console de Debug",
                     colors::BG_DARK,
                     DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    
    textEditor.setMultiLine(true);
    textEditor.setReadOnly(true);
    textEditor.setScrollbarsShown(true);
    textEditor.setCaretVisible(false);
    textEditor.setPopupMenuEnabled(true);
    textEditor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    textEditor.setColour(juce::TextEditor::backgroundColourId, colors::BG_DARK);
    textEditor.setColour(juce::TextEditor::textColourId, colors::TEXT_PRIMARY);

    setContentNonOwned(&textEditor, true);
    
    setResizable(true, true);
    setResizeLimits(400, 300, 2000, 2000);
    centreWithSize(600, 400);

    // Registra este logger como o atual.
    juce::Logger::setCurrentLogger(this);
}

DebugWindow::~DebugWindow()
{
    // Se ainda formos o logger atual, limpa
    if (juce::Logger::getCurrentLogger() == this)
        juce::Logger::setCurrentLogger(nullptr);
}

void DebugWindow::closeButtonPressed()
{
    // Just hide the window instead of destroying it, since the application manages its lifetime
    setVisible(false);
}

void DebugWindow::logMessage(const juce::String& message)
{
    // We must ensure the UI update happens on the message thread
    juce::MessageManager::callAsync([this, msg = message]()
    {
        // Limita o tamanho do log se ficar muito grande (ex: 50.000 caracteres)
        if (textEditor.getTotalNumChars() > 50000)
            textEditor.clear();
            
        textEditor.moveCaretToEnd();
        textEditor.insertTextAtCaret(msg + "\n");
    });
}

} // namespace nova
