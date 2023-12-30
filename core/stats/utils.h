#pragma once

#include <numeric>
#include <ranges>

#include "core/types.h"

WideNgramFrequencyMap replaceChar(const WideNgramFrequencyMap& freq_map,
                                  const WideChar& from, const WideChar& to);

FrequencyMap<WideChar> replaceChar(const FrequencyMap<WideChar>& freq_map,
                                   const WideChar& from, const WideChar& to);

WideNgramFrequencyMap removeChar(const WideNgramFrequencyMap& freq_map,
                                 const WideChar& remove_char);

FrequencyMap<WideChar> removeChar(const FrequencyMap<WideChar>& freq_map,
                                  const WideChar& remove_char);

template <typename T>
long long getTotalFreqs(const FrequencyMap<T>& freq_map) {
  auto freq_r = freq_map | std::views::values;
  return std::accumulate(freq_r.begin(), freq_r.end(), 0LL);
}
