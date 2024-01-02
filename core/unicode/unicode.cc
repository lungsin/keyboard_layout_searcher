#include "unicode.h"

#include <boost/locale.hpp>

namespace unicode {

WideString toWide(const std::string& s) {
  return boost::locale::conv::utf_to_utf<WideChar>(s);
}

std::string toNarrow(const WideString& ws) {
  return boost::locale::conv::utf_to_utf<char>(ws);
}

WideNgramFrequencyMap toWide(const FrequencyMap<std::string>& m) {
  WideNgramFrequencyMap wm;
  for (const auto& [s, freq] : m) {
    wm[toWide(s)] += freq;
  }
  return wm;
}

FrequencyMap<std::string> toNarrow(const FrequencyMap<WideChar>& wm) {
  FrequencyMap<std::string> m;
  for (const auto& [wc, freq] : wm) {
    m[toNarrow(WideString({wc}))] += freq;
  }
  return m;
}

FrequencyMap<std::string> toNarrow(const WideNgramFrequencyMap& wm) {
  FrequencyMap<std::string> m;
  for (const auto& [ws, freq] : wm) {
    m[toNarrow(ws)] += freq;
  }
  return m;
}

WideString toLowercase(const WideChar& c) {
  std::string lowercase = boost::locale::to_lower(toNarrow(WideString({c})));
  return toWide(lowercase);
}

}  // namespace unicode