#include "AudioEngine.h"

#include <cmath>

namespace alors
{

namespace
{
    constexpr int maxChunkSize = 4096;
    constexpr float masterGain = 0.8f;
}

AudioEngine::AudioEngine (std::shared_ptr<const SampleBank> sampleBank)
    : bank (std::move (sampleBank)),
      mixBuffer (2, maxChunkSize)
{
}

bool AudioEngine::schedule (const ScheduledEvent& event)
{
    return scheduled.push (event);
}

bool AudioEngine::preview (int sampleIndex, float gain)
{
    return previews.push ({ 0, sampleIndex, gain, 0.5f, 1.0f });
}

void AudioEngine::silence()
{
    silenceRequested.store (true, std::memory_order_release);
}

void AudioEngine::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    sampleRate.store (device->getCurrentSampleRate(), std::memory_order_release);
}

void AudioEngine::audioDeviceStopped()
{
    voices.stopAll();
}

void AudioEngine::audioDeviceIOCallbackWithContext (const float* const*,
                                                    int,
                                                    float* const* outputChannelData,
                                                    int numOutputChannels,
                                                    int numSamples,
                                                    const juce::AudioIODeviceCallbackContext&)
{
    juce::ScopedNoDenormals noDenormals;

    if (silenceRequested.exchange (false, std::memory_order_acq_rel))
    {
        voices.stopAll();
        scheduled.clear();
    }

    while (auto* event = previews.front())
    {
        startEvent (*event);
        previews.pop();
    }

    auto position = samplePosition.load (std::memory_order_relaxed);

    for (int offset = 0; offset < numSamples; offset += maxChunkSize)
    {
        const auto chunk = juce::jmin (maxChunkSize, numSamples - offset);
        renderChunk (position, chunk);

        const auto* left = mixBuffer.getReadPointer (0);
        const auto* right = mixBuffer.getReadPointer (1);

        for (int channel = 0; channel < numOutputChannels; ++channel)
        {
            auto* out = outputChannelData[channel];

            if (out == nullptr)
                continue;

            for (int i = 0; i < chunk; ++i)
            {
                const auto value = numOutputChannels == 1 ? 0.5f * (left[i] + right[i])
                                 : channel == 0           ? left[i]
                                 : channel == 1           ? right[i]
                                                          : 0.0f;
                out[offset + i] = std::tanh (value * masterGain);
            }
        }

        position += chunk;
    }

    samplePosition.store (position, std::memory_order_release);
}

void AudioEngine::renderChunk (std::int64_t chunkStart, int numSamples)
{
    mixBuffer.clear (0, numSamples);
    auto* left = mixBuffer.getWritePointer (0);
    auto* right = mixBuffer.getWritePointer (1);

    int cursor = 0;

    while (auto* event = scheduled.front())
    {
        const auto eventOffset = event->samplePosition - chunkStart;

        if (eventOffset >= numSamples)
            break;

        if (eventOffset > cursor)
        {
            voices.render (left + cursor, right + cursor, static_cast<int> (eventOffset) - cursor);
            cursor = static_cast<int> (eventOffset);
        }

        startEvent (*event);
        scheduled.pop();
    }

    voices.render (left + cursor, right + cursor, numSamples - cursor);
}

void AudioEngine::startEvent (const ScheduledEvent& event)
{
    if (event.sampleIndex < 0 || event.sampleIndex >= bank->sampleCount())
        return;

    const auto& sample = bank->sample (event.sampleIndex);
    const auto rate = sample.sampleRate / sampleRate.load (std::memory_order_relaxed) * event.speed;
    voices.start (sample, rate, event.gain, event.pan);
}

}
