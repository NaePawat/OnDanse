#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace alors::shortcuts
{

inline const juce::KeyPress play { juce::KeyPress::returnKey, juce::ModifierKeys::commandModifier, 0 };
inline const juce::KeyPress stop { '.', juce::ModifierKeys::commandModifier, 0 };
inline const juce::KeyPress save { 's', juce::ModifierKeys::commandModifier, 0 };

inline bool isGlobal (const juce::KeyPress& key)
{
    return key == play || key == stop || key == save;
}

}
