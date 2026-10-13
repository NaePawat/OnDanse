#include "FileBrowserPanel.h"

#include "Theme.h"

namespace alors
{

FileBrowserPanel::FileBrowserPanel (Workspace& ws) : workspace (ws)
{
    title.setFont (theme::uiFont (12.0f, true));
    title.setColour (juce::Label::textColourId, theme::mutedText);
    addAndMakeVisible (title);

    folderLabel.setFont (theme::uiFont (12.0f));
    folderLabel.setColour (juce::Label::textColourId, theme::mutedText);
    folderLabel.setMinimumHorizontalScale (0.6f);
    addAndMakeVisible (folderLabel);

    openFolderButton.setTooltip ("Choose the folder containing your .alors files");
    openFolderButton.onClick = [this] { chooseFolder(); };
    addAndMakeVisible (openFolderButton);

    refreshButton.onClick = [this] { refresh(); };
    addAndMakeVisible (refreshButton);

    list.setModel (this);
    list.setRowHeight (theme::rowHeight);
    addAndMakeVisible (list);

    refresh();
}

void FileBrowserPanel::refresh()
{
    files = workspace.listFiles();
    folderLabel.setText (workspace.getRoot().getFullPathName(), juce::dontSendNotification);
    folderLabel.setTooltip (workspace.getRoot().getFullPathName());
    list.updateContent();
    list.repaint();
}

void FileBrowserPanel::paint (juce::Graphics& g)
{
    g.fillAll (theme::panelBackground);
}

void FileBrowserPanel::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop (theme::headerHeight).reduced (8, 5);
    refreshButton.setBounds (header.removeFromRight (64));
    header.removeFromRight (4);
    openFolderButton.setBounds (header.removeFromRight (64));
    title.setBounds (header);

    folderLabel.setBounds (area.removeFromTop (20).reduced (8, 0));
    list.setBounds (area);
}

int FileBrowserPanel::getNumRows()
{
    return files.size();
}

void FileBrowserPanel::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, files.size()))
        return;

    if (selected)
        g.fillAll (theme::selection);

    g.setColour (theme::text);
    g.setFont (theme::uiFont (13.0f));
    g.drawText (files[row].getRelativePathFrom (workspace.getRoot()), 16, 0, width - 20, height,
                juce::Justification::centredLeft, true);
}

void FileBrowserPanel::selectedRowsChanged (int lastRowSelected)
{
    if (juce::isPositiveAndBelow (lastRowSelected, files.size()) && onFileChosen)
        onFileChosen (files[lastRowSelected]);
}

void FileBrowserPanel::chooseFolder()
{
    chooser = std::make_unique<juce::FileChooser> ("Choose workspace folder", workspace.getRoot());
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                          [this] (const juce::FileChooser& fc)
                          {
                              if (auto folder = fc.getResult(); folder.isDirectory())
                              {
                                  workspace.setRoot (folder);
                                  refresh();
                              }
                          });
}

}
