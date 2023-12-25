#include "corpus_stats.h"

#include <nlohmann/json.hpp>

CorpusStats::CorpusStats(std::vector<char> const& keyset,
                         RawCorpusStats const& raw_corpuses_stats)
    : total_bigrams_(raw_corpuses_stats.totalBigrams),
      total_skipgrams_(raw_corpuses_stats.totalSkipgrams),
      keyset_size_(keyset.size()),
      keyset_(keyset),
      char_id_(createCharIdMapper(keyset)),
      bigrams_(createBigramOccurance(keyset_size_, raw_corpuses_stats.bigrams,
                                     char_id_)),
      skipgrams_(createBigramOccurance(
          keyset_size_, raw_corpuses_stats.skipgrams, char_id_)) {}

double CorpusStats::getBigramPercentage(char c1, char c2) const {
  return (double)getBigramOccurance(c1, c2) / total_bigrams_;
}

double CorpusStats::getSkipgramPercentage(char c1, char c2) const {
  return (double)getSkipgramOccurance(c1, c2) / total_skipgrams_;
}

long long CorpusStats::getBigramOccurance(char c1, char c2) const {
  if (!char_id_[c1].has_value() || !char_id_[c2].has_value()) return 0.0;
  return bigrams_[char_id_[c1].value()][char_id_[c2].value()];
}

long long CorpusStats::getSkipgramOccurance(char c1, char c2) const {
  if (!char_id_[c1].has_value() || !char_id_[c2].has_value()) return 0.0;
  return skipgrams_[char_id_[c1].value()][char_id_[c2].value()];
}

CorpusStats::CharIdMapper const CorpusStats::createCharIdMapper(
    Keyset const& keyset) {
  CharIdMapper char_id;
  for (size_t i = 0; i < keyset.size(); i++) {
    char_id[keyset[i]] = i;
  }
  return char_id;
}

CorpusStats::BigramOccurance const CorpusStats::createBigramOccurance(
    size_t keyset_size, std::unordered_map<std::string, long long> bigrams,
    CharIdMapper char_id) {
  BigramOccurance occ(keyset_size, std::vector(keyset_size, 0LL));
  for (const auto& [bigramStr, occurance] : bigrams) {
    assert(bigramStr.size() == 2);

    const auto id1 = char_id[bigramStr[0]], id2 = char_id[bigramStr[1]];
    if (id1.has_value() && id2.has_value()) {
      occ[id1.value()][id2.value()] += occurance;
      occ[id2.value()][id1.value()] += occurance;
    }
  }
  return occ;
}
