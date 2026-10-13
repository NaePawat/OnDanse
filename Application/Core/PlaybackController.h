#pragma once

#include "AudioEngine.h"
#include "Scheduler.h"

#include <juce_audio_devices/juce_audio_devices.h>

namespace alors
{

class PlaybackController
{
public:
    explicit PlaybackController (const juce::File& userLibraryFolder);
    ~PlaybackController();

    void play (Pattern pattern);
    void stop();
    bool preview (const std::string& soundName, int variant = 0);

    void setBpm (double bpm);
    double getBpm() const;

    bool isPlaying() const;
    double getCurrentCycle() const;

    std::vector<SoundInfo> getSounds() const;
    juce::String getDeviceError() const { return deviceError; }

private:
    static std::shared_ptr<const SampleBank> buildSampleBank (const juce::File& userLibraryFolder);

    juce::AudioDeviceManager deviceManager;
    AudioEngine engine;
    Scheduler scheduler;
    juce::String deviceError;
};

}
