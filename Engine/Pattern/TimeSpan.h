#pragma once

#include "Fraction.h"

#include <vector>

namespace alors
{

struct TimeSpan
{
    Fraction begin;
    Fraction end;

    Fraction duration() const { return end - begin; }

    std::vector<TimeSpan> splitIntoCycles() const
    {
        std::vector<TimeSpan> spans;

        if (begin == end)
        {
            spans.push_back (*this);
            return spans;
        }

        for (auto start = begin; start < end;)
        {
            auto nextCycle = start.floor() + 1;
            auto stop = min (nextCycle, end);
            spans.push_back ({ start, stop });
            start = stop;
        }

        return spans;
    }

    template <typename Fn>
    TimeSpan withTime (Fn&& fn) const { return { fn (begin), fn (end) }; }

    friend bool operator== (const TimeSpan&, const TimeSpan&) = default;
};

}
