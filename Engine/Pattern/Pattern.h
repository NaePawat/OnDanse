#pragma once

#include "Event.h"

#include <memory>
#include <variant>
#include <vector>

namespace alors
{

struct PatternNode;

class Pattern
{
public:
    Pattern();

    static Pattern silence();
    static Pattern pure (ControlMap value);
    static Pattern sound (std::string name, int variant = 0);
    static Pattern sequence (std::vector<Pattern> steps);
    static Pattern alternate (std::vector<Pattern> cycles);
    static Pattern stack (std::vector<Pattern> layers);

    Pattern fast (Fraction factor) const;
    Pattern slow (Fraction factor) const;
    Pattern late (Fraction offset) const;
    Pattern early (Fraction offset) const;
    Pattern with (ControlMap controls) const;
    Pattern euclid (int pulses, int steps, int rotation = 0) const;

    std::vector<Event> query (TimeSpan span) const;

    const PatternNode& node() const { return *root; }

private:
    explicit Pattern (std::shared_ptr<const PatternNode> node);

    std::shared_ptr<const PatternNode> root;
};

namespace ir
{
    struct Silence {};
    struct Pure      { ControlMap value; };
    struct Sequence  { std::vector<Pattern> steps; };
    struct Alternate { std::vector<Pattern> cycles; };
    struct Stack     { std::vector<Pattern> layers; };
    struct Fast      { Fraction factor; Pattern source; };
    struct Shift     { Fraction offset; Pattern source; };
    struct Merge     { ControlMap controls; Pattern source; };
}

struct PatternNode
{
    std::variant<ir::Silence, ir::Pure, ir::Sequence, ir::Alternate, ir::Stack, ir::Fast, ir::Shift, ir::Merge> op;
};

std::vector<bool> bjorklund (int pulses, int steps);

}