#include "MainComponent.h"

namespace nova
{

MainComponent::MainComponent(EngineManager& engMgr, EditManager& edMgr)
    : engineMgr(engMgr), editMgr(edMgr)
{
    // Apply our custom LookAndFeel
    setLookAndFeel(&lookAndFeel);
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

    // ─── Create ViewModel ───────────────────────────────────────────────
    auto* edit = editMgr.getEdit();
    jassert(edit != nullptr);

    transportVM = std::make_unique<TransportViewModel>(*edit);

    // ─── Create Menu Bar ────────────────────────────────────────────────
    menuBar = std::make_unique<juce::MenuBarComponent>(this);
    addAndMakeVisible(menuBar.get());

    // ─── Create Transport Bar ───────────────────────────────────────────
    transportBar = std::make_unique<TransportBar>(*transportVM);
    addAndMakeVisible(transportBar.get());

    // ─── Create Timeline View ───────────────────────────────────────────
    timelineView = std::make_unique<TimelineView>(*edit, *transportVM);
    addAndMakeVisible(timelineView.get());

    // ─── Create Mixer View ──────────────────────────────────────────────
    mixerView = std::make_unique<MixerView>(*edit);
    addAndMakeVisible(mixerView.get());

    // ─── Create DJ Mode View ────────────────────────────────────────────
    djEngineMgr = std::make_unique<DJEngineManager>(engineMgr);
    djModeView = std::make_unique<DJModeView>(djEngineMgr.get());
    addChildComponent(djModeView.get()); // Hidden by default

    transportBar->onDJModeToggled = [this] { toggleDJMode(); };

    // ─── Window Size ────────────────────────────────────────────────────
    setSize(constants::DEFAULT_WINDOW_WIDTH, constants::DEFAULT_WINDOW_HEIGHT);

    DBG("NovaDAW: MainComponent initialized.");
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(colors::BG_DARKEST);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // Menu bar at very top
    if (menuBar)
        menuBar->setBounds(area.removeFromTop(24));

    // Transport bar
    if (transportBar)
        transportBar->setBounds(area.removeFromTop(constants::TRANSPORT_BAR_HEIGHT));

    // Mixer at bottom
    if (isDJMode)
    {
        if (djModeView)
            djModeView->setBounds(area);
    }
    else
    {
        if (mixerView)
            mixerView->setBounds(area.removeFromBottom(mixerHeight));

        // Timeline takes the remaining center area
        if (timelineView)
            timelineView->setBounds(area);
    }
}

void MainComponent::toggleDJMode()
{
    isDJMode = !isDJMode;
    if (timelineView) timelineView->setVisible(!isDJMode);
    if (mixerView) mixerView->setVisible(!isDJMode);
    if (djModeView) djModeView->setVisible(isDJMode);
    resized();
}

// ─── Menu Bar Model ─────────────────────────────────────────────────────────

juce::StringArray MainComponent::getMenuBarNames()
{
    return { "Arquivo", "Editar", "Faixa", juce::String::fromUTF8("Op\xC3\xA7\xC3\xB5""es"), "Ajuda" };
}

juce::PopupMenu MainComponent::getMenuForIndex(int topLevelMenuIndex, const juce::String& /*menuName*/)
{
    juce::PopupMenu menu;

    switch (topLevelMenuIndex)
    {
        case 0: // File
            menu.addItem(menuNewProject,  "Novo Projeto", true, false);
            menu.addItem(menuOpenProject, "Abrir Projeto...", true, false);
            menu.addSeparator();
            menu.addItem(menuSaveProject, "Salvar", true, false);
            menu.addItem(menuSaveAs,      "Salvar Como...", true, false);
            menu.addSeparator();
            menu.addItem(menuQuit,        "Sair", true, false);
            break;

        case 1: // Edit
            menu.addItem(menuUndo, "Desfazer", true, false);
            menu.addItem(menuRedo, "Refazer", true, false);
            break;

        case 2: // Track
            menu.addItem(menuAddTrack,    "Adicionar Faixa de Áudio", true, false);
            menu.addItem(menuDeleteTrack, "Excluir Faixa Selecionada", true, false);
            break;

        case 3: // Options
            menu.addItem(menuAudioSettings, "Configurações de Áudio e MIDI...", true, false);
            break;

        case 4: // Help
            menu.addItem(menuAbout, "Sobre o NovaDAW", true, false);
            break;

        default:
            break;
    }

    return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/)
{
    switch (menuItemID)
    {
        case menuNewProject:
            createNewProject();
            break;

        case menuOpenProject:
            openProject();
            break;

        case menuSaveProject:
            saveProject();
            break;

        case menuSaveAs:
            editMgr.saveEdit();
            break;

        case menuQuit:
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
            break;

        case menuUndo:
            if (auto* edit = editMgr.getEdit())
                edit->getUndoManager().undo();
            break;

        case menuRedo:
            if (auto* edit = editMgr.getEdit())
                edit->getUndoManager().redo();
            break;

        case menuAddTrack:
        {
            editMgr.addAudioTrack();
            timelineView->refreshTracks();
            mixerView->refreshStrips();
            break;
        }

        case menuDeleteTrack:
        {
            int numTracks = editMgr.getNumAudioTracks();
            if (numTracks > 0)
            {
                editMgr.deleteTrack(numTracks - 1); // Delete last track for now
                timelineView->refreshTracks();
                mixerView->refreshStrips();
            }
            break;
        }

        case menuAudioSettings:
            engineMgr.showAudioSettings(this);
            break;

        case menuAbout:
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::InfoIcon,
                "Sobre o NovaDAW",
                "NovaDAW v" + juce::String(constants::APP_VERSION)
                    + "\n\nUma estação de trabalho de áudio digital moderna"
                    + "\nDesenvolvido com JUCE & Tracktion Engine"
                    + "\n\n© 2026 NovaDAW",
                "OK");
            break;
        }

        default:
            break;
    }
}

// ─── Menu Actions ───────────────────────────────────────────────────────────

void MainComponent::createNewProject()
{
    if (editMgr.hasUnsavedChanges())
    {
        juce::MessageBoxOptions options = juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::QuestionIcon)
            .withTitle("Alterações Não Salvas")
            .withMessage("Deseja salvar as alterações do seu projeto antes de criar um novo?")
            .withButton("Salvar")
            .withButton("Não Salvar")
            .withButton("Cancelar");
            
        juce::AlertWindow::showAsync(options, [this](int result) {
            if (result == 1) // Save
            {
                if (editMgr.saveEdit())
                    performCreateNewProject();
            }
            else if (result == 2) // Don't save
            {
                performCreateNewProject();
            }
        });
    }
    else
    {
        performCreateNewProject();
    }
}

void MainComponent::performCreateNewProject()
{
    editMgr.createNewEdit();

    auto* edit = editMgr.getEdit();
    transportVM = std::make_unique<TransportViewModel>(*edit);

    // Recreate UI components with new edit
    transportBar = std::make_unique<TransportBar>(*transportVM);
    addAndMakeVisible(transportBar.get());

    timelineView = std::make_unique<TimelineView>(*edit, *transportVM);
    addAndMakeVisible(timelineView.get());

    mixerView = std::make_unique<MixerView>(*edit);
    addAndMakeVisible(mixerView.get());

    resized();
    DBG("NovaDAW: New project created.");
}

void MainComponent::openProject()
{
    juce::FileChooser chooser("Abrir Projeto",
                               juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                               "*.tracktionedit");

    if (chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();
        if (editMgr.hasUnsavedChanges())
        {
            juce::MessageBoxOptions options = juce::MessageBoxOptions()
                .withIconType(juce::MessageBoxIconType::QuestionIcon)
                .withTitle("Alterações Não Salvas")
                .withMessage("Deseja salvar as alterações do seu projeto antes de abrir outro?")
                .withButton("Salvar")
                .withButton("Não Salvar")
                .withButton("Cancelar");
                
            juce::AlertWindow::showAsync(options, [this, file](int result) {
                if (result == 1) // Save
                {
                    if (editMgr.saveEdit())
                        performOpenProject(file);
                }
                else if (result == 2) // Don't save
                {
                    performOpenProject(file);
                }
            });
        }
        else
        {
            performOpenProject(file);
        }
    }
}

void MainComponent::performOpenProject(const juce::File& file)
{
    if (editMgr.loadEdit(file))
    {
        auto* edit = editMgr.getEdit();
        transportVM = std::make_unique<TransportViewModel>(*edit);

        transportBar = std::make_unique<TransportBar>(*transportVM);
        addAndMakeVisible(transportBar.get());

        timelineView = std::make_unique<TimelineView>(*edit, *transportVM);
        addAndMakeVisible(timelineView.get());

        mixerView = std::make_unique<MixerView>(*edit);
        addAndMakeVisible(mixerView.get());

        resized();
        DBG("NovaDAW: Project opened.");
    }
}

void MainComponent::saveProject()
{
    editMgr.saveEdit();
}

} // namespace nova
