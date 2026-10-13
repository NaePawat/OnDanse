#include "AudioEngine.h"
#include "Scheduler.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cmath>
#include <thread>

using namespace alors;

namespace
{
    constexpr int blockSize = 480;
    constexpr double testSampleRate = 48000.0;

    std::vector<float> renderRealtime (AudioEngine& engine, double seconds)
    {
        std::vector<float> left;
        std::vector<float> l (blockSize), r (blockSize);
        float* outputs[] = { l.data(), r.data() };
        const auto blocks = static_cast<int> (seconds * testSampleRate / blockSize);

        for (int i = 0; i < blocks; ++i)
        {
            engine.audioDeviceIOCallbackWithContext (nullptr, 0, outputs, 2, blockSize, {});
            left.insert (left.end(), l.begin(), l.end());
            std::this_thread::sleep_for (std::chrono::milliseconds (10));
        }

        return left;
    }

    std::vector<std::int64_t> onsets (const std::vector<float>& signal)
    {
        std::vector<std::int64_t> result;
        int quietRun = 1000;

        for (size_t i = 0; i < signal.size(); ++i)
        {
            const auto level = std::abs (signal[i]);

            if (level > 1.0e-5f && quietRun >= 1000)
                result.push_back (static_cast<std::int64_t> (i));

            quietRun = level < 1.0e-7f ? quietRun + 1 : 0;
        }

        return result;
    }
}

TEST_CASE ("scheduled events land on exact sample positions")
{
    auto bank = std::make_shared<SampleBank>();
    bank->addBuiltInKit();

    AudioEngine engine (bank);
    Scheduler scheduler (engine);
    scheduler.setCyclesPerSecond (0.5);
    scheduler.play (Pattern::sound ("rim").fast (4));

    auto hits = onsets (renderRealtime (engine, 3.0));
    scheduler.stop();

    REQUIRE (hits.size() >= 4);

    const auto samplesPerBeat = static_cast<std::int64_t> (testSampleRate / 0.5 / 4);

    for (size_t i = 1; i < hits.size(); ++i)
        CHECK (hits[i] - hits[i - 1] == samplesPerBeat);
}

TEST_CASE ("stop silences playback")
{
    auto bank = std::make_shared<SampleBank>();
    bank->addBuiltInKit();

    AudioEngine engine (bank);
    Scheduler scheduler (engine);
    scheduler.play (Pattern::sound ("bd").fast (8));

    renderRealtime (engine, 0.5);
    scheduler.stop();
    auto afterStop = renderRealtime (engine, 0.5);

    CHECK (onsets (afterStop).empty());
    CHECK (std::ranges::all_of (afterStop, [] (float s) { return s == 0.0f; }));
}

TEST_CASE ("a new pattern takes over at the next cycle boundary")
{
    auto bank = std::make_shared<SampleBank>();
    bank->addBuiltInKit();

    AudioEngine engine (bank);
    Scheduler scheduler (engine);
    scheduler.setCyclesPerSecond (1.0);
    scheduler.play (Pattern::sound ("rim"));

    auto before = onsets (renderRealtime (engine, 1.3));
    scheduler.play (Pattern::sound ("rim").fast (4));
    auto after = onsets (renderRealtime (engine, 2.0));

    REQUIRE (before.size() == 2);
    REQUIRE (after.size() >= 5);

    const auto cycleLength = static_cast<std::int64_t> (testSampleRate);
    const auto offset = static_cast<std::int64_t> (1.3 * testSampleRate / blockSize) * blockSize;
    const auto firstAfterSwap = after[0] + offset;

    CHECK (firstAfterSwap - before[0] == 2 * cycleLength);
    CHECK (after[1] - after[0] == cycleLength / 4);
}
