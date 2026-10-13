#pragma once

#include "EditorPanel.h"
#include "FileBrowserPanel.h"
#include "LibraryPanel.h"
#include "PlaybackController.h"
#include "Workspace.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace alors
{

class MainComponent : public juce::Component
{
public:
    MainComponent();

    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;

private:
    Workspace workspace;
    PlaybackController playback;

    FileBrowserPanel fileBrowser { workspace };
    EditorPanel editorPanel { playback };
    LibraryPanel libraryPanel { playback };

    juce::StretchableLayoutManager layout;
    juce::StretchableLayoutResizerBar leftDivider { &layout, 1, true };
    juce::StretchableLayoutResizerBar rightDivider { &layout, 3, true };
    juce::TooltipWindow tooltips { this };
};

}
