#include "Workspace.h"

namespace alors
{

namespace
{
    juce::File appDocumentsFolder()
    {
        return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("OnDanse");
    }
}

juce::File Workspace::defaultRoot()
{
    return appDocumentsFolder().getChildFile ("Sketches");
}

juce::File Workspace::defaultLibraryFolder()
{
    return appDocumentsFolder().getChildFile ("Library");
}

Workspace::Workspace (juce::File rootFolder)
{
    setRoot (std::move (rootFolder));
}

void Workspace::setRoot (juce::File newRoot)
{
    root = std::move (newRoot);
    root.createDirectory();
}

juce::Array<juce::File> Workspace::listFiles() const
{
    auto files = root.findChildFiles (juce::File::findFiles, true, juce::String ("*") + fileExtension);
    files.sort();
    return files;
}

}
