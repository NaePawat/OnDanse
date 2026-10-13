#include "MainComponent.h"

#include "Shortcuts.h"

namespace alors
{

MainComponent::MainComponent()
    : workspace (Workspace::defaultRoot()),
      playback (Workspace::defaultLibraryFolder())
{
    fileBrowser.onFileChosen = [this] (const juce::File& file) { editorPanel.openFile (file); };

    layout.setItemLayout (0, 160, 480, -0.18);
    layout.setItemLayout (1, 4, 4, 4);
    layout.setItemLayout (2, 320, -1.0, -0.60);
    layout.setItemLayout (3, 4, 4, 4);
    layout.setItemLayout (4, 160, 480, -0.22);

    for (auto* child : std::initializer_list<juce::Component*> { &fileBrowser, &leftDivider, &editorPanel, &rightDivider, &libraryPanel })
        addAndMakeVisible (child);

    setWantsKeyboardFocus (true);
    setSize (1400, 860);
}

void MainComponent::resized()
{
    juce::Component* columns[] = { &fileBrowser, &leftDivider, &editorPanel, &rightDivider, &libraryPanel };
    layout.layOutComponents (columns, 5, 0, 0, getWidth(), getHeight(), false, true);
}

bool MainComponent::keyPressed (const juce::KeyPress& key)
{
    if (key == shortcuts::play) { editorPanel.play(); return true; }
    if (key == shortcuts::stop) { editorPanel.stop(); return true; }
    if (key == shortcuts::save) { editorPanel.save(); return true; }
    return false;
}

}
