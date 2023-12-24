#pragma once

#include <array>
#include <optional>
#include <vector>

#include "raw_corpus_stats.h"

using namespace std;

class CorpusStats {
  const int keysetSize;

  vector<vector<long long>> bigrams, skipgrams;
  long long totalBigrams, totalSkipgrams;

  array<optional<int>, 256> charId;

 public:
  CorpusStats(vector<char> const& keyset, RawCorpusStats const& rawCorpusStats);

  double getBigramPercentage(char c1, char c2) const;

  double getSkipgramPercentage(char c1, char c2) const;

  long long getBigramOccurance(char c1, char c2) const;

  long long getSkipgramOccurance(char c1, char c2) const;
};