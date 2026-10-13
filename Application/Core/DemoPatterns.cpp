#include "DemoPatterns.h"

namespace alors
{

Pattern demoBeat()
{
    const auto rest = Pattern::silence();
    const auto s = [] (const char* name) { return Pattern::sound (name); };

    auto kick = s ("bd").fast (4);
    auto clap = Pattern::sequence ({ rest, s ("cp") }).fast (2).with ({ { "gain", 0.8 } });
    auto openHats = Pattern::sequence ({ rest, s ("oh") }).fast (4).with ({ { "gain", 0.45 } });
    auto ticks = s ("hh").euclid (5, 8).fast (2).with ({ { "gain", 0.35 }, { "pan", 0.3 } });
    auto rim = s ("rim").euclid (3, 8, 2).with ({ { "gain", 0.5 }, { "pan", 0.75 } });

    auto fill = Pattern::sequence ({ rest, rest, rest, Pattern::sequence ({ s ("ht"), s ("ht"), s ("lt"), s ("lt") }) });
    auto toms = Pattern::alternate ({ rest, rest, rest, fill }).with ({ { "gain", 0.7 } });

    return Pattern::stack ({ kick, clap, openHats, ticks, rim, toms });
}

}
