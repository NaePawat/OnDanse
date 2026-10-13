#include "Scheduler.h"

#include <algorithm>
#include <cmath>

namespace alors
{

namespace
{
    constexpr double lookaheadSeconds = 0.15;
    constexpr double startDelaySeconds = 0.1;
    constexpr int tickIntervalMs = 10;

    // Query windows are snapped to a fixed grid so window boundaries keep small denominators.
    constexpr std::int64_t windowGrid = 10080;
}

Scheduler::Scheduler (AudioEngine& audioEngine)
    : juce::Thread ("OnDanse Scheduler"),
      engine (audioEngine)
{
    startThread (juce::Thread::Priority::high);
}

Scheduler::~Scheduler()
{
    stopThread (1000);
}

void Scheduler::play (Pattern pattern)
{
    std::scoped_lock lock (mutex);

    if (playing)
    {
        pending = std::move (pattern);
        return;
    }

    current = std::move (pattern);
    pending.reset();
    anchorCycle = 0;
    scheduledUntil = 0;
    anchorSample = static_cast<double> (engine.getSamplePosition()) + startDelaySeconds * engine.getSampleRate();
    playing = true;
    notify();
}

void Scheduler::stop()
{
    {
        std::scoped_lock lock (mutex);
        playing = false;
        current.reset();
        pending.reset();
    }

    engine.silence();
}

void Scheduler::setCyclesPerSecond (double newCps)
{
    if (newCps <= 0.0)
        return;

    std::scoped_lock lock (mutex);

    if (playing)
    {
        anchorSample = sampleAt (scheduledUntil);
        anchorCycle = scheduledUntil;
    }

    cps = newCps;
}

double Scheduler::getCyclesPerSecond() const
{
    std::scoped_lock lock (mutex);
    return cps;
}

bool Scheduler::isPlaying() const
{
    std::scoped_lock lock (mutex);
    return playing;
}

double Scheduler::getCurrentCycle() const
{
    std::scoped_lock lock (mutex);

    if (! playing)
        return 0.0;

    return std::max (0.0, cycleAt (static_cast<double> (engine.getSamplePosition())));
}

void Scheduler::run()
{
    while (! threadShouldExit())
    {
        tick();
        wait (tickIntervalMs);
    }
}

void Scheduler::tick()
{
    std::vector<ScheduledEvent> events;

    {
        std::scoped_lock lock (mutex);

        if (! playing || ! current)
            return;

        const auto horizonSample = static_cast<double> (engine.getSamplePosition()) + lookaheadSeconds * engine.getSampleRate();
        const auto horizon = Fraction (static_cast<std::int64_t> (std::floor (cycleAt (horizonSample) * windowGrid)), windowGrid);

        if (horizon <= scheduledUntil)
            return;

        auto from = scheduledUntil;

        if (pending)
        {
            const auto boundary = from.ceil();

            if (boundary < horizon)
            {
                scheduleSpan (*current, { from, boundary }, events);
                current = std::move (pending);
                pending.reset();
                from = boundary;
            }
        }

        scheduleSpan (*current, { from, horizon }, events);
        scheduledUntil = horizon;
    }

    std::ranges::sort (events, {}, &ScheduledEvent::samplePosition);

    for (auto& event : events)
        engine.schedule (event);
}

void Scheduler::scheduleSpan (const Pattern& pattern, TimeSpan span, std::vector<ScheduledEvent>& out) const
{
    if (span.begin >= span.end)
        return;

    const auto& bank = engine.getSampleBank();

    for (auto& event : pattern.query (span))
    {
        if (! event.hasOnset())
            continue;

        const auto name = getString (event.value, "s");
        const auto variant = static_cast<int> (getNumber (event.value, "n", 0.0));
        const auto sampleIndex = bank.find (name, variant);

        if (! sampleIndex)
            continue;

        out.push_back ({ static_cast<std::int64_t> (std::llround (sampleAt (event.whole->begin))),
                         *sampleIndex,
                         static_cast<float> (getNumber (event.value, "gain", 1.0)),
                         static_cast<float> (getNumber (event.value, "pan", 0.5)),
                         static_cast<float> (getNumber (event.value, "speed", 1.0)) });
    }
}

double Scheduler::sampleAt (Fraction cycle) const
{
    return anchorSample + (cycle - anchorCycle).toDouble() / cps * engine.getSampleRate();
}

double Scheduler::cycleAt (double samplePosition) const
{
    return anchorCycle.toDouble() + (samplePosition - anchorSample) / engine.getSampleRate() * cps;
}

}
