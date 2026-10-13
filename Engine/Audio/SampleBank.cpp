#include "SampleBank.h"

#include "BuiltInKit.h"

namespace alors
{

namespace
{
    std::optional<Sample> readSample (juce::AudioFormatManager& formats, const juce::File& file)
    {
        std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));

        if (reader == nullptr || reader->lengthInSamples <= 0)
            return std::nullopt;

        Sample sample;
        sample.sampleRate = reader->sampleRate;
        sample.buffer.setSize (static_cast<int> (juce::jmin (reader->numChannels, 2u)),
                               static_cast<int> (reader->lengthInSamples));
        reader->read (&sample.buffer, 0, sample.buffer.getNumSamples(), 0, true, sample.buffer.getNumChannels() > 1);
        return sample;
    }

    juce::Array<juce::File> sortedAudioFiles (const juce::File& folder, const juce::AudioFormatManager& formats)
    {
        auto files = folder.findChildFiles (juce::File::findFiles, false, formats.getWildcardForAllFormats());
        files.sort();
        return files;
    }
}

void SampleBank::addSound (const std::string& name, const std::string& category, Sample sample)
{
    auto& entry = entries[name];

    if (entry.sampleIndices.empty())
        entry.category = category;

    entry.sampleIndices.push_back (static_cast<int> (samples.size()));
    samples.push_back (std::move (sample));
}

void SampleBank::addBuiltInKit()
{
    for (auto& sound : renderBuiltInKit())
        addSound (sound.name, "built-in", std::move (sound.sample));
}

int SampleBank::loadFolder (const juce::File& folder)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();

    auto loaded = 0;
    const auto category = folder.getFileName().toStdString();

    for (auto& file : sortedAudioFiles (folder, formats))
    {
        if (auto sample = readSample (formats, file))
        {
            addSound (file.getFileNameWithoutExtension().toLowerCase().toStdString(), category, std::move (*sample));
            ++loaded;
        }
    }

    for (auto& subfolder : folder.findChildFiles (juce::File::findDirectories, false))
    {
        const auto name = subfolder.getFileName().toLowerCase().toStdString();

        for (auto& file : sortedAudioFiles (subfolder, formats))
        {
            if (auto sample = readSample (formats, file))
            {
                addSound (name, category, std::move (*sample));
                ++loaded;
            }
        }
    }

    return loaded;
}

std::optional<int> SampleBank::find (std::string_view name, int variant) const
{
    auto it = entries.find (name);

    if (it == entries.end() || it->second.sampleIndices.empty())
        return std::nullopt;

    const auto& indices = it->second.sampleIndices;
    const auto count = static_cast<int> (indices.size());
    return indices[static_cast<size_t> (((variant % count) + count) % count)];
}

std::vector<SoundInfo> SampleBank::sounds() const
{
    std::vector<SoundInfo> result;

    for (auto& [name, entry] : entries)
        result.push_back ({ name, entry.category, static_cast<int> (entry.sampleIndices.size()) });

    return result;
}

}
