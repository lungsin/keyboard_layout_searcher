#pragma once

#include <istream>
#include <unordered_map>

struct RawCorpusStats {
  long long totalChars;
  std::unordered_map<char, long long> characters;

  long long totalBigrams, totalSkipgrams, totalSkipgrams2, totalSkipgrams3,
      totalTrigrams;
  std::unordered_map<std::string, long long> bigrams, skipgrams, skipgrams2,
      skipgrams3, trigrams;

  RawCorpusStats(std::istream& textStream);

  void addWord(const std::string& word);
};