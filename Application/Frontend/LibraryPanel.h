#pragma once

#include "PlaybackController.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace alors
{

class LibraryPanel : public juce::Component,
                     private juce::ListBoxModel
{
public:
    explicit LibraryPanel (PlaybackController& playback);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool selected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;

    PlaybackController& playback;
    std::vector<SoundInfo> sounds;
    juce::Label title { {}, "SOUNDS" };
    juce::Label hint { {}, "Click a sound to preview it" };
    juce::ListBox list;
};

}
