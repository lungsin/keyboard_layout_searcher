#pragma once

#include <array>
#include <optional>
#include <vector>

#include "raw_corpus_stats.h"
#include "types.h"

class CorpusStats {
 public:
  CorpusStats(Keyset const& keyset, RawCorpusStats const& raw_corpuses_stats);

  double getBigramPercentage(char c1, char c2) const;

  double getSkipgramPercentage(char c1, char c2) const;

  long long getBigramOccurance(char c1, char c2) const;

  long long getSkipgramOccurance(char c1, char c2) const;

  const long long total_bigrams_, total_skipgrams_;

 private:
  using CharIdMapper = std::array<std::optional<int>, 256>;
  using BigramOccurance = std::vector<std::vector<long long>>;

  // Constructor helper
  static CharIdMapper const createCharIdMapper(Keyset const& keyset);
  static BigramOccurance const createBigramOccurance(
      size_t keyset_size, std::unordered_map<std::string, long long> bigrams,
      CharIdMapper char_id);

  // Private field
  const size_t keyset_size_;
  const Keyset keyset_;

  const CharIdMapper char_id_;
  const BigramOccurance bigrams_, skipgrams_;
};