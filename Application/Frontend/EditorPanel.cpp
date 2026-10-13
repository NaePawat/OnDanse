#include "EditorPanel.h"

#include "DemoPatterns.h"
#include "Shortcuts.h"
#include "Theme.h"

namespace alors
{

namespace
{
    constexpr int statusHeight = 24;
}

bool EditorPanel::CodeEditor::keyPressed (const juce::KeyPress& key)
{
    if (shortcuts::isGlobal (key))
        return false;

    return juce::CodeEditorComponent::keyPressed (key);
}

EditorPanel::EditorPanel (PlaybackController& pc) : playback (pc)
{
    editor.setFont (theme::monospaceFont (15.0f));
    editor.setTabSize (2, true);
    editor.setLineNumbersShown (true);
    document.addListener (this);
    addAndMakeVisible (editor);

    fileLabel.setFont (theme::uiFont (13.0f));
    addAndMakeVisible (fileLabel);

    cycleLabel.setFont (theme::monospaceFont (13.0f));
    cycleLabel.setColour (juce::Label::textColourId, theme::mutedText);
    cycleLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (cycleLabel);

    bpmLabel.setFont (theme::uiFont (12.0f, true));
    bpmLabel.setColour (juce::Label::textColourId, theme::mutedText);
    bpmLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (bpmLabel);

    bpmSlider.setRange (40.0, 240.0, 1.0);
    bpmSlider.setValue (playback.getBpm(), juce::dontSendNotification);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 44, 24);
    bpmSlider.onValueChange = [this] { playback.setBpm (bpmSlider.getValue()); };
    addAndMakeVisible (bpmSlider);

    compileButton.setEnabled (false);
    compileButton.setTooltip ("The .alors compiler arrives in milestone 4");
    addAndMakeVisible (compileButton);

    playButton.setTooltip ("Play (Ctrl+Enter)");
    playButton.setColour (juce::TextButton::buttonColourId, theme::accent);
    playButton.onClick = [this] { play(); };
    addAndMakeVisible (playButton);

    stopButton.setTooltip ("Stop (Ctrl+.)");
    stopButton.onClick = [this] { stop(); };
    addAndMakeVisible (stopButton);

    statusLabel.setFont (theme::uiFont (12.0f));
    addAndMakeVisible (statusLabel);

    if (auto error = playback.getDeviceError(); error.isNotEmpty())
        setStatus ("Audio device error: " + error, theme::error);
    else
        setStatus ("Ready. Play runs the built-in demo beat until the compiler lands.", theme::mutedText);

    updateFileLabel();
    startTimerHz (30);
}

EditorPanel::~EditorPanel()
{
    document.removeListener (this);
}

void EditorPanel::openFile (const juce::File& file)
{
    if (file == currentFile)
        return;

    if (currentFile.existsAsFile() && document.hasChangedSinceSavePoint())
        save();

    currentFile = file;
    document.replaceAllContent (file.loadFileAsString());
    document.clearUndoHistory();
    document.setSavePoint();
    editor.scrollToLine (0);
    updateFileLabel();
}

bool EditorPanel::save()
{
    if (! currentFile.existsAsFile())
        return false;

    if (! currentFile.replaceWithText (document.getAllContent()))
    {
        setStatus ("Could not save " + currentFile.getFileName(), theme::error);
        return false;
    }

    document.setSavePoint();
    updateFileLabel();
    setStatus ("Saved " + currentFile.getFileName(), theme::mutedText);
    return true;
}

void EditorPanel::play()
{
    playback.play (demoBeat());
    setStatus ("Playing the built-in demo beat. The .alors compiler is not implemented yet.", theme::playing);
}

void EditorPanel::stop()
{
    playback.stop();
    setStatus ("Stopped.", theme::mutedText);
}

void EditorPanel::paint (juce::Graphics& g)
{
    g.fillAll (theme::editorBackground);

    g.setColour (theme::headerBackground);
    g.fillRect (getLocalBounds().removeFromTop (theme::headerHeight));
    g.fillRect (getLocalBounds().removeFromBottom (statusHeight));

    g.setColour (theme::divider);
    g.drawHorizontalLine (theme::headerHeight - 1, 0.0f, static_cast<float> (getWidth()));
}

void EditorPanel::resized()
{
    auto area = getLocalBounds();

    auto header = area.removeFromTop (theme::headerHeight).reduced (8, 5);
    stopButton.setBounds (header.removeFromRight (60));
    header.removeFromRight (6);
    playButton.setBounds (header.removeFromRight (60));
    header.removeFromRight (6);
    compileButton.setBounds (header.removeFromRight (76));
    header.removeFromRight (12);
    bpmSlider.setBounds (header.removeFromRight (110));
    bpmLabel.setBounds (header.removeFromRight (40));
    cycleLabel.setBounds (header.removeFromRight (110));
    fileLabel.setBounds (header);

    statusLabel.setBounds (area.removeFromBottom (statusHeight).reduced (8, 0));
    editor.setBounds (area);
}

void EditorPanel::timerCallback()
{
    const auto isPlaying = playback.isPlaying();
    cycleLabel.setText (isPlaying ? "cycle " + juce::String (playback.getCurrentCycle(), 2) : juce::String(),
                        juce::dontSendNotification);
    cycleLabel.setColour (juce::Label::textColourId, isPlaying ? theme::playing : theme::mutedText);
}

void EditorPanel::updateFileLabel()
{
    if (! currentFile.existsAsFile())
    {
        fileLabel.setText ("Untitled (open a .alors file from the left)", juce::dontSendNotification);
        return;
    }

    auto name = currentFile.getFileName();
    if (document.hasChangedSinceSavePoint())
        name << "  *";

    fileLabel.setText (name, juce::dontSendNotification);
}

void EditorPanel::setStatus (const juce::String& message, juce::Colour colour)
{
    statusLabel.setText (message, juce::dontSendNotification);
    statusLabel.setColour (juce::Label::textColourId, colour);
}

}
