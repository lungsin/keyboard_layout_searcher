#include "fast_read_corpus_stats.h"

WideString sortKeysetByOccurance(WideString const& keyset,
                                 RawCorpusStats const& stats) {
  WideString sorted_keyset(keyset);
  std::sort(sorted_keyset.begin(), sorted_keyset.end(),
            [&](WideChar const& c1, WideChar const& c2) {
              return stats.getCharFreq(c1) > stats.getCharFreq(c2);
            });
  return sorted_keyset;
}

FastReadCorpusStats::FastReadCorpusStats(RawCorpusStats const& raw_corpus_stats,
                                         WideString const& keyset)
    : keyset_(sortKeysetByOccurance(keyset, raw_corpus_stats)),
      char_id_map_(keyset_),
      total_chars_(raw_corpus_stats.getTotalChars()),
      total_bigrams_(raw_corpus_stats.getTotalBigrams()),
      total_skipgrams_(raw_corpus_stats.getTotalSkipgrams()),
      total_skipgrams2_(raw_corpus_stats.getTotalSkipgrams2()),
      total_skipgrams3_(raw_corpus_stats.getTotalSkipgrams3()),
      total_trigrams_(raw_corpus_stats.getTotalTrigrams()),
      char_freqs_(raw_corpus_stats.getCharFreqPairs(), char_id_map_),
      bigram_freqs_(raw_corpus_stats.getBigramFreqPairs(), char_id_map_),
      skipgram_freqs_(raw_corpus_stats.getSkipgramFreqPairs(), char_id_map_),
      skipgram2_freqs_(raw_corpus_stats.getSkipgram2FreqPairs(), char_id_map_),
      skipgram3_freqs_(raw_corpus_stats.getSkipgram3FreqPairs(), char_id_map_),
      trigram_freqs_(raw_corpus_stats.getTrigramFreqPairs(), char_id_map_) {}

std::optional<int> FastReadCorpusStats::getCharId(WideChar const& c) const {
  if (!char_id_map_.contains(c)) return std::nullopt;
  return char_id_map_.at(c);
}

WideChar FastReadCorpusStats::getCharFromId(CharId const& id) const {
  return keyset_[id];
}

WideString FastReadCorpusStats::getKeyset() const { return keyset_; }

size_t FastReadCorpusStats::getKeysetSize() const {
  return char_id_map_.size();
}

long long FastReadCorpusStats::getTotalChars() const { return total_chars_; }
long long FastReadCorpusStats::getTotalBigrams() const {
  return total_bigrams_;
}
long long FastReadCorpusStats::getTotalSkipgrams() const {
  return total_skipgrams_;
}
long long FastReadCorpusStats::getTotalSkipgrams2() const {
  return total_skipgrams2_;
}
long long FastReadCorpusStats::getTotalSkipgrams3() const {
  return total_skipgrams3_;
}
long long FastReadCorpusStats::getTotalTrigrams() const {
  return total_trigrams_;
}

long long FastReadCorpusStats::getCharFreq(const CharId& c) const {
  return char_freqs_[c];
}
long long FastReadCorpusStats::getBigramFreq(const CharId& c1,
                                             const CharId& c2) const {
  return bigram_freqs_[c1][c2];
}
long long FastReadCorpusStats::getSkipgramFreq(const CharId& c1,
                                               const CharId& c2) const {
  return skipgram_freqs_[c1][c2];
}
long long FastReadCorpusStats::getSkipgram2Freq(const CharId& c1,
                                                const CharId& c2) const {
  return skipgram2_freqs_[c1][c2];
}
long long FastReadCorpusStats::getSkipgram3Freq(const CharId& c1,
                                                const CharId& c2) const {
  return skipgram3_freqs_[c1][c2];
}
long long FastReadCorpusStats::getTrigramFreq(const CharId& c1,
                                              const CharId& c2,
                                              const CharId& c3) const {
  return trigram_freqs_[c1][c2][c3];
}

FastReadCorpusStats::CharIdMap::CharIdMap(WideString const& keyset) {
  for (size_t i = 0; i < keyset.size(); i++) {
    this->insert_or_assign(keyset[i], i);
  }
}

FastReadCorpusStats::CharFrequencyTable::CharFrequencyTable(
    const FrequencyMap<WideChar>& freq_map, const CharIdMap& char_id_map)
    : std::vector<long long>(char_id_map.size(), 0LL) {
  for (const auto& [c, freq] : freq_map) {
    (*this)[char_id_map.at(c)] += freq;
  }
}

FastReadCorpusStats::BigramFrequencyTable::BigramFrequencyTable(
    const WideNgramFrequencyMap& freq_map, const CharIdMap& char_id_map)
    : boost::multi_array<long long, 2>(
          boost::extents[char_id_map.size()][char_id_map.size()]) {
  for (const auto& [s, freq] : freq_map) {
    const CharId id1 = char_id_map.at(s[0]), id2 = char_id_map.at(s[1]);
    (*this)[id1][id2] += freq;
  }
}

FastReadCorpusStats::TrigramFrequencyTable::TrigramFrequencyTable(
    const WideNgramFrequencyMap& freq_map, const CharIdMap& char_id_map)
    : boost::multi_array<long long, 3>(
          boost::extents[char_id_map.size()][char_id_map.size()]
                        [char_id_map.size()]) {
  for (const auto& [s, freq] : freq_map) {
    const CharId id1 = char_id_map.at(s[0]), id2 = char_id_map.at(s[1]),
                 id3 = char_id_map.at(s[2]);
    (*this)[id1][id2][id3] += freq;
  }
}
