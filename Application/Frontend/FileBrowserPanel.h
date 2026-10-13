#pragma once

#include "Workspace.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace alors
{

class FileBrowserPanel : public juce::Component,
                         private juce::ListBoxModel
{
public:
    explicit FileBrowserPanel (Workspace& workspace);

    std::function<void (const juce::File&)> onFileChosen;

    void refresh();
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool selected) override;
    void selectedRowsChanged (int lastRowSelected) override;
    void chooseFolder();

    Workspace& workspace;
    juce::Array<juce::File> files;
    juce::Label title { {}, "EXPLORER" };
    juce::TextButton openFolderButton { "Open..." };
    juce::TextButton refreshButton { "Refresh" };
    juce::Label folderLabel;
    juce::ListBox list;
    std::unique_ptr<juce::FileChooser> chooser;
};

}
