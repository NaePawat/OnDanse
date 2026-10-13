#pragma once

#include "AudioEngine.h"
#include "Pattern.h"

#include <juce_core/juce_core.h>

#include <mutex>
#include <optional>

namespace alors
{

class Scheduler : private juce::Thread
{
public:
    explicit Scheduler (AudioEngine& audioEngine);
    ~Scheduler() override;

    void play (Pattern pattern);
    void stop();

    void setCyclesPerSecond (double newCps);
    double getCyclesPerSecond() const;

    bool isPlaying() const;
    double getCurrentCycle() const;

private:
    void run() override;
    void tick();
    void scheduleSpan (const Pattern& pattern, TimeSpan span, std::vector<ScheduledEvent>& out) const;
    double sampleAt (Fraction cycle) const;
    double cycleAt (double samplePosition) const;

    AudioEngine& engine;

    mutable std::mutex mutex;
    std::optional<Pattern> current;
    std::optional<Pattern> pending;
    bool playing = false;
    double cps = 0.5;
    Fraction anchorCycle;
    double anchorSample = 0.0;
    Fraction scheduledUntil;
};

}
