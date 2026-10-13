#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace alors::theme
{

inline const juce::Colour editorBackground { 0xff1e1e1e };
inline const juce::Colour panelBackground  { 0xff252526 };
inline const juce::Colour headerBackground { 0xff2d2d2d };
inline const juce::Colour divider          { 0xff3c3c3c };
inline const juce::Colour text             { 0xffcccccc };
inline const juce::Colour mutedText        { 0xff8b8b8b };
inline const juce::Colour accent           { 0xff0e639c };
inline const juce::Colour selection        { 0xff094771 };
inline const juce::Colour playing          { 0xff89d185 };
inline const juce::Colour error            { 0xfff48771 };

constexpr int headerHeight = 34;
constexpr int rowHeight = 24;

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
};

juce::Font monospaceFont (float height);
juce::Font uiFont (float height, bool bold = false);

}
