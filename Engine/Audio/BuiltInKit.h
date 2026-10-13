#pragma once

#include "SampleBank.h"

namespace alors
{

struct NamedSample
{
    std::string name;
    Sample sample;
};

std::vector<NamedSample> renderBuiltInKit();

}
