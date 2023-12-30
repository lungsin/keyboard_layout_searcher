#pragma once

#include "core/types.h"

namespace unicode {

WideString toWide(const std::string &s);

std::string toNarrow(const WideString &ws);

WideNgramFrequencyMap toWide(const FrequencyMap<std::string> &m);

FrequencyMap<std::string> toNarrow(const WideNgramFrequencyMap &wm);

WideString toLowercase(const WideChar &c);

}  // namespace unicode