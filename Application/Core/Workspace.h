#pragma once

#include <juce_core/juce_core.h>

namespace alors
{

class Workspace
{
public:
    static constexpr const char* fileExtension = ".alors";

    static juce::File defaultRoot();
    static juce::File defaultLibraryFolder();

    explicit Workspace (juce::File rootFolder);

    const juce::File& getRoot() const { return root; }
    void setRoot (juce::File newRoot);

    juce::Array<juce::File> listFiles() const;

private:
    juce::File root;
};

}
