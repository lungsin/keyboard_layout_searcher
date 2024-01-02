#pragma once

#include <string>
#include <unordered_map>

using WideChar = char32_t;
using WideString = std::u32string;

template <class T>
using FrequencyMap = std::unordered_map<T, long long>;

using WideNgramFrequencyMap = FrequencyMap<WideString>;
