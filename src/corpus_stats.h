#pragma once

#include <array>
#include <optional>
#include <vector>

#include "raw_corpus_stats.h"

class CorpusStats {
  const int keysetSize;

  std::vector<std::vector<long long>> bigrams, skipgrams;
  long long totalBigrams, totalSkipgrams;

  std::array<std::optional<int>, 256> charId;

 public:
  CorpusStats(std::vector<char> const& keyset,
              RawCorpusStats const& rawCorpusStats);

  double getBigramPercentage(char c1, char c2) const;

  double getSkipgramPercentage(char c1, char c2) const;

  long long getBigramOccurance(char c1, char c2) const;

  long long getSkipgramOccurance(char c1, char c2) const;
};