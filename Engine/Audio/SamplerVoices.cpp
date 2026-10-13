#include "SamplerVoices.h"

#include <cmath>

namespace alors
{

void SamplerVoices::start (const Sample& sample, double playbackRate, float gain, float pan)
{
    const auto angle = juce::jlimit (0.0f, 1.0f, pan) * juce::MathConstants<float>::halfPi;

    auto& voice = allocateVoice();
    voice.sample = &sample;
    voice.position = 0.0;
    voice.increment = playbackRate;
    voice.gainLeft = gain * std::cos (angle);
    voice.gainRight = gain * std::sin (angle);
    voice.startOrder = nextStartOrder++;
}

void SamplerVoices::render (float* left, float* right, int numSamples)
{
    for (auto& voice : voices)
    {
        if (voice.sample == nullptr)
            continue;

        const auto& buffer = voice.sample->buffer;
        const auto length = buffer.getNumSamples();
        const auto* channelLeft = buffer.getReadPointer (0);
        const auto* channelRight = buffer.getReadPointer (buffer.getNumChannels() > 1 ? 1 : 0);

        for (int i = 0; i < numSamples; ++i)
        {
            const auto index = static_cast<int> (voice.position);

            if (index + 1 >= length)
            {
                voice.sample = nullptr;
                break;
            }

            const auto frac = static_cast<float> (voice.position - index);
            const auto sampleLeft = channelLeft[index] + frac * (channelLeft[index + 1] - channelLeft[index]);
            const auto sampleRight = channelRight[index] + frac * (channelRight[index + 1] - channelRight[index]);

            left[i] += sampleLeft * voice.gainLeft;
            right[i] += sampleRight * voice.gainRight;
            voice.position += voice.increment;
        }
    }
}

void SamplerVoices::stopAll()
{
    for (auto& voice : voices)
        voice.sample = nullptr;
}

SamplerVoices::Voice& SamplerVoices::allocateVoice()
{
    Voice* oldest = &voices[0];

    for (auto& voice : voices)
    {
        if (voice.sample == nullptr)
            return voice;

        if (voice.startOrder < oldest->startOrder)
            oldest = &voice;
    }

    return *oldest;
}

}
