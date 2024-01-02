#pragma once

#include <boost/multi_array.hpp>
#include <optional>
#include <unordered_map>
#include <vector>

#include "core/types.h"
#include "raw_corpus_stats.h"

WideString sortKeysetByOccurance(WideString const& keyset,
                                 RawCorpusStats const& stats);

class FastReadCorpusStats {
 public:
  using CharId = int;

  FastReadCorpusStats(RawCorpusStats const& raw_corpus_stats,
                      WideString const& keyset);

  std::optional<CharId> getCharId(WideChar const& c) const;
  WideChar getCharFromId(CharId const& id) const;
  WideString getKeyset() const;
  size_t getKeysetSize() const;

  long long getTotalChars() const;
  long long getTotalBigrams() const;
  long long getTotalSkipgrams() const;
  long long getTotalSkipgrams2() const;
  long long getTotalSkipgrams3() const;
  long long getTotalTrigrams() const;

  long long getCharFreq(const CharId& c) const;
  long long getBigramFreq(const CharId& c1, const CharId& c2) const;
  long long getSkipgramFreq(const CharId& c1, const CharId& c2) const;
  long long getSkipgram2Freq(const CharId& c1, const CharId& c2) const;
  long long getSkipgram3Freq(const CharId& c1, const CharId& c2) const;
  long long getTrigramFreq(const CharId& c1, const CharId& c2,
                           const CharId& c3) const;

 private:
  class CharIdMap : public std::unordered_map<WideChar, CharId> {
   public:
    CharIdMap(WideString const& keyset);
  };

  class BigramFrequencyTable : public boost::multi_array<long long, 2> {
   public:
    BigramFrequencyTable(const WideNgramFrequencyMap& freq_map,
                         const CharIdMap& char_id_map);
  };

  class TrigramFrequencyTable : public boost::multi_array<long long, 3> {
   public:
    TrigramFrequencyTable(const WideNgramFrequencyMap& freq_map,
                          const CharIdMap& char_id_map);
  };

  class CharFrequencyTable : public std::vector<long long> {
   public:
    CharFrequencyTable(const FrequencyMap<WideChar>& freq_map,
                       const CharIdMap& char_id_map);
  };

  WideString keyset_;
  CharIdMap char_id_map_;

  long long total_chars_ = 0;
  long long total_bigrams_ = 0;
  long long total_skipgrams_ = 0;
  long long total_skipgrams2_ = 0;
  long long total_skipgrams3_ = 0;
  long long total_trigrams_ = 0;

  CharFrequencyTable char_freqs_;
  BigramFrequencyTable bigram_freqs_;
  BigramFrequencyTable skipgram_freqs_;
  BigramFrequencyTable skipgram2_freqs_;
  BigramFrequencyTable skipgram3_freqs_;
  TrigramFrequencyTable trigram_freqs_;
};
