#include "BuiltInKit.h"

#include <cmath>

namespace alors
{

namespace
{
    constexpr double kitSampleRate = 48000.0;
    constexpr double twoPi = juce::MathConstants<double>::twoPi;

    template <typename Fn>
    Sample render (double seconds, Fn&& generator)
    {
        Sample sample;
        sample.sampleRate = kitSampleRate;
        sample.buffer.setSize (1, static_cast<int> (seconds * kitSampleRate));

        auto* out = sample.buffer.getWritePointer (0);

        for (int i = 0; i < sample.buffer.getNumSamples(); ++i)
            out[i] = static_cast<float> (generator (i / kitSampleRate));

        return sample;
    }

    Sample kick()
    {
        double phase = 0.0;

        return render (0.6, [phase] (double t) mutable
        {
            const auto frequency = 45.0 + 110.0 * std::exp (-t * 30.0);
            phase += twoPi * frequency / kitSampleRate;
            const auto click = std::exp (-t * 400.0) * 0.4;
            return (std::sin (phase) + click) * std::exp (-t * 6.0) * 0.95;
        });
    }

    Sample snare()
    {
        juce::Random random (1);

        return render (0.3, [random] (double t) mutable
        {
            const auto tone = std::sin (twoPi * 185.0 * t) * std::exp (-t * 35.0) * 0.5;
            const auto noise = (random.nextDouble() * 2.0 - 1.0) * std::exp (-t * 18.0) * 0.6;
            return tone + noise;
        });
    }

    Sample hat (double seconds, double decay, juce::int64 seed)
    {
        juce::Random random (seed);
        double previous = 0.0;

        return render (seconds, [=] (double t) mutable
        {
            const auto white = random.nextDouble() * 2.0 - 1.0;
            const auto highPassed = white - previous;
            previous = white;
            return highPassed * std::exp (-t * decay) * 0.35;
        });
    }

    Sample clap()
    {
        juce::Random random (4);

        return render (0.35, [random] (double t) mutable
        {
            const auto noise = random.nextDouble() * 2.0 - 1.0;
            auto envelope = std::exp (-(t - 0.03) * 22.0) * (t >= 0.03 ? 1.0 : 0.0);

            for (auto burst : { 0.0, 0.01, 0.02 })
                if (t >= burst && t < burst + 0.01)
                    envelope = juce::jmax (envelope, std::exp (-(t - burst) * 250.0));

            return noise * envelope * 0.6;
        });
    }

    Sample tom (double frequency)
    {
        double phase = 0.0;

        return render (0.45, [phase, frequency] (double t) mutable
        {
            phase += twoPi * frequency * (1.0 + 0.6 * std::exp (-t * 25.0)) / kitSampleRate;
            return std::sin (phase) * std::exp (-t * 9.0) * 0.8;
        });
    }

    Sample rim()
    {
        return render (0.08, [] (double t)
        {
            return (std::sin (twoPi * 1700.0 * t) + 0.6 * std::sin (twoPi * 830.0 * t)) * std::exp (-t * 90.0) * 0.5;
        });
    }
}

std::vector<NamedSample> renderBuiltInKit()
{
    std::vector<NamedSample> kit;
    kit.push_back ({ "bd", kick() });
    kit.push_back ({ "sd", snare() });
    kit.push_back ({ "hh", hat (0.08, 70.0, 2) });
    kit.push_back ({ "oh", hat (0.5, 7.0, 3) });
    kit.push_back ({ "cp", clap() });
    kit.push_back ({ "lt", tom (110.0) });
    kit.push_back ({ "ht", tom (180.0) });
    kit.push_back ({ "rim", rim() });
    return kit;
}

}
