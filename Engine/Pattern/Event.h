#pragma once

#include "TimeSpan.h"

#include <map>
#include <optional>
#include <string>
#include <variant>

namespace alors
{

using Control = std::variant<double, std::string>;
using ControlMap = std::map<std::string, Control, std::less<>>;

inline double getNumber (const ControlMap& controls, std::string_view key, double fallback)
{
    if (auto it = controls.find (key); it != controls.end())
        if (auto* value = std::get_if<double> (&it->second))
            return *value;

    return fallback;
}

inline std::string getString (const ControlMap& controls, std::string_view key, std::string fallback = {})
{
    if (auto it = controls.find (key); it != controls.end())
        if (auto* value = std::get_if<std::string> (&it->second))
            return *value;

    return fallback;
}

struct Event
{
    std::optional<TimeSpan> whole;
    TimeSpan part;
    ControlMap value;

    bool hasOnset() const { return whole.has_value() && whole->begin == part.begin; }
};

}
