#pragma once

#include <istream>
#include <unordered_map>

using namespace std;

struct RawCorpusStats {
  long long totalChars;
  unordered_map<char, long long> characters;

  long long totalBigrams, totalSkipgrams, totalSkipgrams2, totalSkipgrams3,
      totalTrigrams;
  unordered_map<string, long long> bigrams, skipgrams, skipgrams2, skipgrams3,
      trigrams;

  RawCorpusStats(istream& textStream);

  void addWord(const string& word);
};