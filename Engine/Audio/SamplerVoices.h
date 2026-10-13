#pragma once

#include "SampleBank.h"

#include <array>
#include <cstdint>

namespace alors
{

class SamplerVoices
{
public:
    static constexpr int maxVoices = 64;

    void start (const Sample& sample, double playbackRate, float gain, float pan);
    void render (float* left, float* right, int numSamples);
    void stopAll();

private:
    struct Voice
    {
        const Sample* sample = nullptr;
        double position = 0.0;
        double increment = 1.0;
        float gainLeft = 0.0f;
        float gainRight = 0.0f;
        std::uint64_t startOrder = 0;
    };

    Voice& allocateVoice();

    std::array<Voice, maxVoices> voices;
    std::uint64_t nextStartOrder = 0;
};

}
