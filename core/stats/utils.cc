#include "utils.h"

#include <boost/algorithm/string.hpp>

WideNgramFrequencyMap replaceChar(const WideNgramFrequencyMap& freq_map,
                                  const WideChar& from, const WideChar& to) {
  WideNgramFrequencyMap result_map;
  for (const auto& [ngram, freq] : freq_map) {
    result_map[boost::algorithm::replace_all_copy(ngram, WideString({from}),
                                                  WideString({to}))] += freq;
  }
  return result_map;
}

FrequencyMap<WideChar> replaceChar(const FrequencyMap<WideChar>& freq_map,
                                   const WideChar& from, const WideChar& to) {
  FrequencyMap<WideChar> result_map;
  for (const auto& [c, freq] : freq_map) {
    WideChar relaced_char = c == from ? to : c;
    result_map[relaced_char] += freq;
  }
  return result_map;
}

WideNgramFrequencyMap removeChar(const WideNgramFrequencyMap& freq_map,
                                 const WideChar& remove_char) {
  WideNgramFrequencyMap result_map;
  for (const auto& [ngram, freq] : freq_map) {
    const bool is_contain = ngram.find(remove_char) == std::string::npos;
    if (is_contain) continue;
    result_map[ngram] += freq;
  }
  return result_map;
}

FrequencyMap<WideChar> removeChar(const FrequencyMap<WideChar>& freq_map,
                                  const WideChar& remove_char) {
  FrequencyMap<WideChar> result_map;
  for (const auto& [c, freq] : freq_map) {
    if (c == remove_char) continue;
    result_map[c] += freq;
  }
  return result_map;
}
