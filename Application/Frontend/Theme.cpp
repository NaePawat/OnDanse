#include "Theme.h"

namespace alors::theme
{

LookAndFeel::LookAndFeel()
{
    setColourScheme ({ panelBackground, editorBackground, headerBackground, divider, text,
                       accent, juce::Colours::white, selection, text });

    setColour (juce::ResizableWindow::backgroundColourId, editorBackground);
    setColour (juce::ListBox::backgroundColourId, panelBackground);
    setColour (juce::TextButton::buttonColourId, headerBackground);
    setColour (juce::TextButton::buttonOnColourId, accent);
    setColour (juce::TextButton::textColourOffId, text);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::ComboBox::outlineColourId, divider);
    setColour (juce::Slider::textBoxBackgroundColourId, editorBackground);
    setColour (juce::Slider::textBoxOutlineColourId, divider);
    setColour (juce::Slider::textBoxTextColourId, text);
    setColour (juce::Label::textColourId, text);
    setColour (juce::CodeEditorComponent::backgroundColourId, editorBackground);
    setColour (juce::CodeEditorComponent::defaultTextColourId, juce::Colour (0xffd4d4d4));
    setColour (juce::CodeEditorComponent::lineNumberBackgroundId, editorBackground);
    setColour (juce::CodeEditorComponent::lineNumberTextId, juce::Colour (0xff858585));
    setColour (juce::CodeEditorComponent::highlightColourId, juce::Colour (0xff264f78));
    setColour (juce::CaretComponent::caretColourId, juce::Colour (0xffaeafad));
    setColour (juce::ScrollBar::thumbColourId, juce::Colour (0x66797979));
}

juce::Font LookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return uiFont (juce::jmin (14.0f, buttonHeight * 0.55f));
}

juce::Font monospaceFont (float height)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain));
}

juce::Font uiFont (float height, bool bold)
{
    return juce::Font (juce::FontOptions (height, bold ? juce::Font::bold : juce::Font::plain));
}

}
