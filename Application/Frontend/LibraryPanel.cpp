#include "LibraryPanel.h"

#include "Theme.h"

namespace alors
{

LibraryPanel::LibraryPanel (PlaybackController& pc)
    : playback (pc),
      sounds (pc.getSounds())
{
    title.setFont (theme::uiFont (12.0f, true));
    title.setColour (juce::Label::textColourId, theme::mutedText);
    addAndMakeVisible (title);

    hint.setFont (theme::uiFont (12.0f));
    hint.setColour (juce::Label::textColourId, theme::mutedText);
    addAndMakeVisible (hint);

    list.setModel (this);
    list.setRowHeight (theme::rowHeight);
    addAndMakeVisible (list);
}

void LibraryPanel::paint (juce::Graphics& g)
{
    g.fillAll (theme::panelBackground);
}

void LibraryPanel::resized()
{
    auto area = getLocalBounds();
    title.setBounds (area.removeFromTop (theme::headerHeight).reduced (8, 5));
    hint.setBounds (area.removeFromTop (20).reduced (8, 0));
    list.setBounds (area);
}

int LibraryPanel::getNumRows()
{
    return static_cast<int> (sounds.size());
}

void LibraryPanel::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, static_cast<int> (sounds.size())))
        return;

    const auto& sound = sounds[static_cast<size_t> (row)];

    if (selected)
        g.fillAll (theme::selection);

    g.setFont (theme::monospaceFont (13.0f));
    g.setColour (theme::text);
    g.drawText (sound.name, 16, 0, width / 2, height, juce::Justification::centredLeft, true);

    auto detail = juce::String (sound.category);
    if (sound.variantCount > 1)
        detail << "  x" << sound.variantCount;

    g.setFont (theme::uiFont (12.0f));
    g.setColour (theme::mutedText);
    g.drawText (detail, width / 2, 0, width / 2 - 10, height, juce::Justification::centredRight, true);
}

void LibraryPanel::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (juce::isPositiveAndBelow (row, static_cast<int> (sounds.size())))
        playback.preview (sounds[static_cast<size_t> (row)].name);
}

}
