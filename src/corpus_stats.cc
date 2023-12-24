#include "corpus_stats.h"

#include <nlohmann/json.hpp>

using namespace std;

CorpusStats::CorpusStats(vector<char> const& keyset,
                         RawCorpusStats const& rawCorpusStats)
    : keysetSize(keyset.size()),
      bigrams(keysetSize, vector(keysetSize, 0LL)),
      skipgrams(keysetSize, vector(keysetSize, 0LL)),
      totalBigrams(rawCorpusStats.totalBigrams),
      totalSkipgrams(rawCorpusStats.totalSkipgrams) {
  for (int i = 0; i < keysetSize; i++) {
    charId[keyset[i]] = i;
  }

  for (const auto& [bigramStr, occurance] : rawCorpusStats.bigrams) {
    assert(bigramStr.size() == 2);

    const auto id1 = charId[bigramStr[0]], id2 = charId[bigramStr[1]];
    if (id1.has_value() && id2.has_value())
      bigrams[id1.value()][id2.value()] += occurance;
  }

  for (const auto& [skipgramStr, occurance] : rawCorpusStats.skipgrams) {
    assert(skipgramStr.size() == 2);

    const auto id1 = charId[skipgramStr[0]], id2 = charId[skipgramStr[1]];
    if (id1.has_value() && id2.has_value())
      bigrams[id1.value()][id2.value()] += occurance;
  }
}

double CorpusStats::getBigramPercentage(char c1, char c2) const {
  return (double)getBigramOccurance(c1, c2) / totalBigrams;
}

double CorpusStats::getSkipgramPercentage(char c1, char c2) const {
  return (double)getSkipgramOccurance(c1, c2) / totalSkipgrams;
}

long long CorpusStats::getBigramOccurance(char c1, char c2) const {
  if (!charId[c1].has_value() || !charId[c2].has_value()) return 0.0;
  return bigrams[charId[c1].value()][charId[c2].value()];
}

long long CorpusStats::getSkipgramOccurance(char c1, char c2) const {
  if (!charId[c1].has_value() || !charId[c2].has_value()) return 0.0;
  return skipgrams[charId[c1].value()][charId[c2].value()];
}
