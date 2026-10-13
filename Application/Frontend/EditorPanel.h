#pragma once

#include "PlaybackController.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace alors
{

class EditorPanel : public juce::Component,
                    private juce::CodeDocument::Listener,
                    private juce::Timer
{
public:
    explicit EditorPanel (PlaybackController& playback);
    ~EditorPanel() override;

    void openFile (const juce::File& file);
    bool save();
    void play();
    void stop();

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class CodeEditor : public juce::CodeEditorComponent
    {
    public:
        using juce::CodeEditorComponent::CodeEditorComponent;
        bool keyPressed (const juce::KeyPress& key) override;
    };

    void codeDocumentTextInserted (const juce::String&, int) override { updateFileLabel(); }
    void codeDocumentTextDeleted (int, int) override { updateFileLabel(); }
    void timerCallback() override;

    void updateFileLabel();
    void setStatus (const juce::String& message, juce::Colour colour);

    PlaybackController& playback;
    juce::File currentFile;
    juce::CodeDocument document;
    CodeEditor editor { document, nullptr };

    juce::Label fileLabel;
    juce::Label cycleLabel;
    juce::Label bpmLabel { {}, "BPM" };
    juce::Slider bpmSlider { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
    juce::TextButton compileButton { "Compile" };
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::Label statusLabel;
};

}
