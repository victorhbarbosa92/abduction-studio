#include "NovaDAWApplication.h"

namespace nova
{

void NovaDAWApplication::initialise(const juce::String& /*commandLine*/)
{
    debugWindow = std::make_unique<DebugWindow>();
    debugWindow->setVisible(true);
    DBG("═══════════════════════════════════════════════════");
    DBG("  Abduction Studio v" + juce::String(constants::APP_VERSION));
    DBG("  Uma Estação de Trabalho de Áudio Digital Moderna");
    DBG("═══════════════════════════════════════════════════");

    // 1. Initialize the audio engine
    engineManager = std::make_unique<EngineManager>();

    // 2. Create a new edit (project)
    editManager = std::make_unique<EditManager>(*engineManager);

    // 3. Create the main UI component
    mainComponent = std::make_unique<MainComponent>(*engineManager, *editManager);

    // 4. Create the main window
    mainWindow = std::make_unique<MainWindow>(
        constants::APP_NAME + juce::String(" v") + constants::APP_VERSION,
        mainComponent.get()
    );

    DBG("Abduction Studio: Application started successfully.");
}

void NovaDAWApplication::shutdown()
{
    DBG("Abduction Studio: Shutting down...");

    // Destroy in reverse order
    mainWindow.reset();
    mainComponent.reset();
    editManager.reset();
    engineManager.reset();
    debugWindow.reset();

    DBG("Abduction Studio: Shutdown complete.");
}

void NovaDAWApplication::systemRequestedQuit()
{
    if (editManager && editManager->hasUnsavedChanges())
    {
        juce::MessageBoxOptions options = juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::QuestionIcon)
            .withTitle("Alterações Não Salvas")
            .withMessage("Deseja salvar as alterações do seu projeto antes de sair?")
            .withButton("Salvar")
            .withButton("Não Salvar")
            .withButton("Cancelar");

        juce::AlertWindow::showAsync(options, [this](int result) {
            if (result == 1) // Save
            {
                if (editManager->saveEdit())
                    quit();
            }
            else if (result == 2) // Don't Save
            {
                quit();
            }
        });
    }
    else
    {
        quit();
    }
}

void NovaDAWApplication::anotherInstanceStarted(const juce::String& /*commandLine*/)
{
    // Bring existing window to front
    if (mainWindow)
        mainWindow->toFront(true);
}

} // namespace nova
