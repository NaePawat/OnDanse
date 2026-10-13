#pragma once

#include "SampleBank.h"
#include "SamplerVoices.h"
#include "SpscQueue.h"

#include <juce_audio_devices/juce_audio_devices.h>

#include <atomic>
#include <cstdint>
#include <memory>

namespace alors
{

struct ScheduledEvent
{
    std::int64_t samplePosition = 0;
    int sampleIndex = 0;
    float gain = 1.0f;
    float pan = 0.5f;
    float speed = 1.0f;
};

class AudioEngine : public juce::AudioIODeviceCallback
{
public:
    explicit AudioEngine (std::shared_ptr<const SampleBank> sampleBank);

    const SampleBank& getSampleBank() const { return *bank; }

    bool schedule (const ScheduledEvent& event);
    bool preview (int sampleIndex, float gain = 0.8f);
    void silence();

    std::int64_t getSamplePosition() const { return samplePosition.load (std::memory_order_acquire); }
    double getSampleRate() const { return sampleRate.load (std::memory_order_acquire); }

    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                           int numInputChannels,
                                           float* const* outputChannelData,
                                           int numOutputChannels,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

private:
    void startEvent (const ScheduledEvent& event);
    void renderChunk (std::int64_t chunkStart, int numSamples);

    std::shared_ptr<const SampleBank> bank;
    SpscQueue<ScheduledEvent, 8192> scheduled;
    SpscQueue<ScheduledEvent, 256> previews;
    SamplerVoices voices;
    juce::AudioBuffer<float> mixBuffer;

    std::atomic<std::int64_t> samplePosition { 0 };
    std::atomic<double> sampleRate { 48000.0 };
    std::atomic<bool> silenceRequested { false };
};

}
