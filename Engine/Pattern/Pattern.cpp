#include "Pattern.h"

#include <algorithm>

namespace alors
{

namespace
{
    template <typename... Ts>
    struct Overloaded : Ts... { using Ts::operator()...; };

    template <typename Fn>
    std::vector<Event> mapEventTimes (std::vector<Event> events, Fn&& fn)
    {
        for (auto& e : events)
        {
            if (e.whole)
                e.whole = e.whole->withTime (fn);

            e.part = e.part.withTime (fn);
        }

        return events;
    }

    void append (std::vector<Event>& into, std::vector<Event>&& from)
    {
        into.insert (into.end(), std::make_move_iterator (from.begin()), std::make_move_iterator (from.end()));
    }

    template <typename QueryFn>
    std::vector<Event> queryScaled (QueryFn&& queryFn, TimeSpan span, Fraction factor)
    {
        if (factor <= Fraction (0))
            return {};

        return mapEventTimes (queryFn (span.withTime ([&] (Fraction t) { return t * factor; })),
                              [&] (Fraction t) { return t / factor; });
    }

    std::vector<Event> queryAlternate (const std::vector<Pattern>& cycles, TimeSpan span)
    {
        std::vector<Event> events;

        if (cycles.empty())
            return events;

        const auto count = static_cast<std::int64_t> (cycles.size());

        for (auto& cycleSpan : span.splitIntoCycles())
        {
            const auto cycle = cycleSpan.begin.floor();
            const auto index = ((cycle.numerator() % count) + count) % count;

            // Each child only advances one cycle per full rotation, so it is queried at its own local cycle.
            const auto offset = cycle - (cycleSpan.begin / Fraction (count)).floor();

            auto childEvents = cycles[static_cast<size_t> (index)].query (
                cycleSpan.withTime ([&] (Fraction t) { return t - offset; }));

            append (events, mapEventTimes (std::move (childEvents), [&] (Fraction t) { return t + offset; }));
        }

        return events;
    }
}

Pattern::Pattern() : Pattern (std::make_shared<PatternNode> (PatternNode { ir::Silence {} })) {}

Pattern::Pattern (std::shared_ptr<const PatternNode> node) : root (std::move (node)) {}

Pattern Pattern::silence() { return {}; }

Pattern Pattern::pure (ControlMap value)
{
    return Pattern (std::make_shared<PatternNode> (PatternNode { ir::Pure { std::move (value) } }));
}

Pattern Pattern::sound (std::string name, int variant)
{
    return pure ({ { "s", std::move (name) }, { "n", static_cast<double> (variant) } });
}

Pattern Pattern::sequence (std::vector<Pattern> steps)
{
    return Pattern (std::make_shared<PatternNode> (PatternNode { ir::Sequence { std::move (steps) } }));
}

Pattern Pattern::alternate (std::vector<Pattern> cycles)
{
    return Pattern (std::make_shared<PatternNode> (PatternNode { ir::Alternate { std::move (cycles) } }));
}

Pattern Pattern::stack (std::vector<Pattern> layers)
{
    return Pattern (std::make_shared<PatternNode> (PatternNode { ir::Stack { std::move (layers) } }));
}

Pattern Pattern::fast (Fraction factor) const
{
    return Pattern (std::make_shared<PatternNode> (PatternNode { ir::Fast { factor, *this } }));
}

Pattern Pattern::slow (Fraction factor) const
{
    if (factor == Fraction (0))
        return silence();

    return fast (Fraction (1) / factor);
}

Pattern Pattern::late (Fraction offset) const
{
    return Pattern (std::make_shared<PatternNode> (PatternNode { ir::Shift { offset, *this } }));
}

Pattern Pattern::early (Fraction offset) const
{
    return late (-offset);
}

Pattern Pattern::with (ControlMap controls) const
{
    return Pattern (std::make_shared<PatternNode> (PatternNode { ir::Merge { std::move (controls), *this } }));
}

Pattern Pattern::euclid (int pulses, int steps, int rotation) const
{
    if (steps <= 0)
        return silence();

    auto hits = bjorklund (pulses, steps);
    const auto shift = ((rotation % steps) + steps) % steps;
    std::rotate (hits.begin(), hits.begin() + shift, hits.end());

    std::vector<Pattern> slots;
    slots.reserve (hits.size());

    for (auto hit : hits)
        slots.push_back (hit ? *this : silence());

    return sequence (std::move (slots));
}

std::vector<Event> Pattern::query (TimeSpan span) const
{
    return std::visit (Overloaded {
        [] (const ir::Silence&) { return std::vector<Event> {}; },

        [&] (const ir::Pure& op)
        {
            std::vector<Event> events;

            for (auto& cycleSpan : span.splitIntoCycles())
            {
                auto start = cycleSpan.begin.floor();
                events.push_back ({ TimeSpan { start, start + 1 }, cycleSpan, op.value });
            }

            return events;
        },

        [&] (const ir::Sequence& op)
        {
            return queryScaled ([&] (TimeSpan s) { return queryAlternate (op.steps, s); },
                                span,
                                Fraction (static_cast<std::int64_t> (op.steps.size())));
        },

        [&] (const ir::Alternate& op) { return queryAlternate (op.cycles, span); },

        [&] (const ir::Stack& op)
        {
            std::vector<Event> events;

            for (auto& layer : op.layers)
                append (events, layer.query (span));

            return events;
        },

        [&] (const ir::Fast& op)
        {
            return queryScaled ([&] (TimeSpan s) { return op.source.query (s); }, span, op.factor);
        },

        [&] (const ir::Shift& op)
        {
            return mapEventTimes (op.source.query (span.withTime ([&] (Fraction t) { return t - op.offset; })),
                                  [&] (Fraction t) { return t + op.offset; });
        },

        [&] (const ir::Merge& op)
        {
            auto events = op.source.query (span);

            for (auto& e : events)
                for (auto& [key, value] : op.controls)
                    e.value.insert_or_assign (key, value);

            return events;
        },
    }, root->op);
}

std::vector<bool> bjorklund (int pulses, int steps)
{
    if (steps <= 0)
        return {};

    if (pulses <= 0)
        return std::vector<bool> (static_cast<size_t> (steps), false);

    if (pulses >= steps)
        return std::vector<bool> (static_cast<size_t> (steps), true);

    using Group = std::vector<bool>;
    std::vector<Group> front (static_cast<size_t> (pulses), Group { true });
    std::vector<Group> back (static_cast<size_t> (steps - pulses), Group { false });

    while (back.size() > 1)
    {
        const auto paired = std::min (front.size(), back.size());
        std::vector<Group> merged;
        merged.reserve (paired);

        for (size_t i = 0; i < paired; ++i)
        {
            auto group = front[i];
            group.insert (group.end(), back[i].begin(), back[i].end());
            merged.push_back (std::move (group));
        }

        std::vector<Group> remainder = front.size() > paired
                                         ? std::vector<Group> (front.begin() + static_cast<std::ptrdiff_t> (paired), front.end())
                                         : std::vector<Group> (back.begin() + static_cast<std::ptrdiff_t> (paired), back.end());

        front = std::move (merged);
        back = std::move (remainder);
    }

    std::vector<bool> result;

    for (auto* groups : { &front, &back })
        for (auto& group : *groups)
            result.insert (result.end(), group.begin(), group.end());

    return result;
}

}
