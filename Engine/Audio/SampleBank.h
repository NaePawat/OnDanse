#pragma once

#include <juce_audio_formats/juce_audio_formats.h>

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace alors
{

struct Sample
{
    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;
};

struct SoundInfo
{
    std::string name;
    std::string category;
    int variantCount = 0;
};

class SampleBank
{
public:
    void addSound (const std::string& name, const std::string& category, Sample sample);
    void addBuiltInKit();
    int loadFolder (const juce::File& folder);

    std::optional<int> find (std::string_view name, int variant) const;
    const Sample& sample (int index) const { return samples[static_cast<size_t> (index)]; }
    int sampleCount() const { return static_cast<int> (samples.size()); }

    std::vector<SoundInfo> sounds() const;

private:
    struct Entry
    {
        std::string category;
        std::vector<int> sampleIndices;
    };

    std::vector<Sample> samples;
    std::map<std::string, Entry, std::less<>> entries;
};

}
