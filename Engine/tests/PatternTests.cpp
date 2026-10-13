#include "Pattern.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

using namespace alors;

namespace
{
    std::vector<Event> onsets (const Pattern& pattern, TimeSpan span)
    {
        auto events = pattern.query (span);
        std::erase_if (events, [] (const Event& e) { return ! e.hasOnset(); });
        std::ranges::sort (events, {}, [] (const Event& e) { return e.part.begin; });
        return events;
    }

    std::vector<std::string> names (const std::vector<Event>& events)
    {
        std::vector<std::string> result;

        for (auto& e : events)
            result.push_back (getString (e.value, "s"));

        return result;
    }

    std::vector<Fraction> starts (const std::vector<Event>& events)
    {
        std::vector<Fraction> result;

        for (auto& e : events)
            result.push_back (e.whole->begin);

        return result;
    }

    const TimeSpan firstCycle { 0, 1 };
}

TEST_CASE ("Fraction normalises and compares exactly")
{
    CHECK (Fraction (2, 4) == Fraction (1, 2));
    CHECK (Fraction (1, -3) == Fraction (-1, 3));
    CHECK (Fraction (1, 3) + Fraction (1, 6) == Fraction (1, 2));
    CHECK (Fraction (1, 3) < Fraction (1, 2));
    CHECK (Fraction (7, 2).floor() == Fraction (3));
    CHECK (Fraction (-1, 2).floor() == Fraction (-1));
    CHECK (Fraction (7, 2).ceil() == Fraction (4));
    CHECK (Fraction (7, 2).cyclePosition() == Fraction (1, 2));
}

TEST_CASE ("TimeSpan splits at cycle boundaries")
{
    auto spans = TimeSpan { Fraction (1, 2), Fraction (5, 2) }.splitIntoCycles();

    REQUIRE (spans.size() == 3);
    CHECK (spans[0] == TimeSpan { Fraction (1, 2), 1 });
    CHECK (spans[1] == TimeSpan { 1, 2 });
    CHECK (spans[2] == TimeSpan { 2, Fraction (5, 2) });
}

TEST_CASE ("pure repeats once per cycle")
{
    auto events = onsets (Pattern::sound ("bd"), { 0, 3 });

    CHECK (names (events) == std::vector<std::string> { "bd", "bd", "bd" });
    CHECK (starts (events) == std::vector<Fraction> { 0, 1, 2 });
}

TEST_CASE ("sequence subdivides the cycle evenly")
{
    auto pattern = Pattern::sequence ({ Pattern::sound ("bd"), Pattern::sound ("hh"), Pattern::sound ("sd"), Pattern::sound ("hh") });
    auto events = onsets (pattern, firstCycle);

    CHECK (names (events) == std::vector<std::string> { "bd", "hh", "sd", "hh" });
    CHECK (starts (events) == std::vector<Fraction> { 0, Fraction (1, 4), Fraction (1, 2), Fraction (3, 4) });
    CHECK (events[1].whole->duration() == Fraction (1, 4));
}

TEST_CASE ("nested sequences subdivide their slot")
{
    auto pattern = Pattern::sequence ({ Pattern::sound ("bd"),
                                        Pattern::sequence ({ Pattern::sound ("hh"), Pattern::sound ("hh") }) });

    CHECK (starts (onsets (pattern, firstCycle)) == std::vector<Fraction> { 0, Fraction (1, 2), Fraction (3, 4) });
}

TEST_CASE ("silence in a sequence leaves a rest")
{
    auto pattern = Pattern::sequence ({ Pattern::sound ("bd"), Pattern::silence(), Pattern::sound ("sd") });

    CHECK (starts (onsets (pattern, firstCycle)) == std::vector<Fraction> { 0, Fraction (2, 3) });
}

TEST_CASE ("stack plays layers together")
{
    auto pattern = Pattern::stack ({ Pattern::sound ("bd"), Pattern::sound ("hh").fast (2) });
    auto events = onsets (pattern, firstCycle);

    CHECK (events.size() == 3);
}

TEST_CASE ("fast and slow scale time")
{
    auto pair = Pattern::sequence ({ Pattern::sound ("a"), Pattern::sound ("b") });

    CHECK (names (onsets (pair.fast (2), firstCycle)) == std::vector<std::string> { "a", "b", "a", "b" });

    auto slowed = Pattern::sound ("a").slow (2);
    auto firstHalf = slowed.query ({ 0, 1 });
    auto secondHalf = slowed.query ({ 1, 2 });

    REQUIRE (firstHalf.size() == 1);
    REQUIRE (secondHalf.size() == 1);
    CHECK (firstHalf[0].whole == TimeSpan { 0, 2 });
    CHECK (firstHalf[0].hasOnset());
    CHECK_FALSE (secondHalf[0].hasOnset());
}

TEST_CASE ("partial queries return fragments with correct onsets")
{
    auto pattern = Pattern::sequence ({ Pattern::sound ("a"), Pattern::sound ("b"), Pattern::sound ("c"), Pattern::sound ("d") });

    CHECK (names (onsets (pattern, { Fraction (1, 4), Fraction (3, 4) })) == std::vector<std::string> { "b", "c" });

    auto straddling = pattern.query ({ Fraction (1, 8), Fraction (3, 8) });
    REQUIRE (straddling.size() == 2);
    CHECK (std::ranges::count_if (straddling, [] (const Event& e) { return e.hasOnset(); }) == 1);
}

TEST_CASE ("alternate picks one child per cycle")
{
    auto pattern = Pattern::alternate ({ Pattern::sound ("a"), Pattern::sound ("b"), Pattern::sound ("c") });

    CHECK (names (onsets (pattern, { 0, 4 })) == std::vector<std::string> { "a", "b", "c", "a" });
}

TEST_CASE ("alternate advances each child at its own pace")
{
    auto inner = Pattern::alternate ({ Pattern::sound ("x"), Pattern::sound ("y") });
    auto pattern = Pattern::alternate ({ inner, Pattern::sound ("b") });

    CHECK (names (onsets (pattern, { 0, 4 })) == std::vector<std::string> { "x", "b", "y", "b" });
}

TEST_CASE ("late shifts events in time")
{
    auto events = onsets (Pattern::sound ("bd").late (Fraction (1, 4)), firstCycle);

    CHECK (starts (events) == std::vector<Fraction> { Fraction (1, 4) });
}

TEST_CASE ("with merges controls into every event")
{
    auto events = onsets (Pattern::sound ("bd").with ({ { "gain", 0.5 } }), firstCycle);

    REQUIRE (events.size() == 1);
    CHECK (getNumber (events[0].value, "gain", 1.0) == 0.5);
    CHECK (getString (events[0].value, "s") == "bd");
}

TEST_CASE ("bjorklund matches the canonical euclidean rhythms")
{
    auto render = [] (int k, int n)
    {
        std::string s;
        for (auto hit : bjorklund (k, n))
            s += hit ? 'x' : '.';
        return s;
    };

    CHECK (render (3, 8) == "x..x..x.");
    CHECK (render (5, 8) == "x.xx.xx.");
    CHECK (render (4, 6) == "x.xx.x");
    CHECK (render (1, 4) == "x...");
    CHECK (render (0, 4) == "....");
    CHECK (render (4, 4) == "xxxx");
}

TEST_CASE ("euclid places hits on the right steps")
{
    auto events = onsets (Pattern::sound ("bd").euclid (3, 8), firstCycle);

    CHECK (starts (events) == std::vector<Fraction> { 0, Fraction (3, 8), Fraction (6, 8) });

    auto rotated = onsets (Pattern::sound ("bd").euclid (3, 8, 1), firstCycle);
    CHECK (starts (rotated) == std::vector<Fraction> { Fraction (2, 8), Fraction (5, 8), Fraction (7, 8) });
}
