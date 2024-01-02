#pragma once

#include <iostream>
#include <nlohmann/json.hpp>

#include "core/types.h"

using json = nlohmann::json;

class RawCorpusStats {
 public:
  RawCorpusStats() = default;
  RawCorpusStats(std::istream& text_stream);
  RawCorpusStats(const json& j);

  static RawCorpusStats fromTextStream(std::istream& text_stream);
  static RawCorpusStats fromTextFile(std::string const& text_path);
  static RawCorpusStats fromJsonStream(std::istream& json_stream);
  static RawCorpusStats fromJsonFile(std::string const& json_path);

  void addWord(const WideString& word);
  void addWord(const std::string& word);

  void save_as_json(std::ostream& out_stream) const;

  void save_as_json(const std::string& file_name) const;

  long long getTotalChars() const;
  long long getTotalBigrams() const;
  long long getTotalSkipgrams() const;
  long long getTotalSkipgrams2() const;
  long long getTotalSkipgrams3() const;
  long long getTotalTrigrams() const;

  long long getCharFreq(const WideChar& c) const;
  long long getBigramFreq(const WideChar& c1, const WideChar& c2) const;
  long long getSkipgramFreq(const WideChar& c1, const WideChar& c2) const;
  long long getSkipgram2Freq(const WideChar& c1, const WideChar& c2) const;
  long long getSkipgram3Freq(const WideChar& c1, const WideChar& c2) const;
  long long getTrigramFreq(const WideChar& c1, const WideChar& c2,
                           const WideChar& c3) const;

  FrequencyMap<WideChar> getCharFreqPairs() const;
  WideNgramFrequencyMap getBigramFreqPairs() const;
  WideNgramFrequencyMap getSkipgramFreqPairs() const;
  WideNgramFrequencyMap getSkipgram2FreqPairs() const;
  WideNgramFrequencyMap getSkipgram3FreqPairs() const;
  WideNgramFrequencyMap getTrigramFreqPairs() const;

  void replaceChar(const WideChar& from, const WideChar& to);
  void removeChar(const WideChar& c);
  void whitelistChars(const WideString& whitelist);

 private:
  long long total_chars_ = 0;
  FrequencyMap<WideChar> char_freqs_;

  long long total_bigrams_ = 0;
  long long total_skipgrams_ = 0;
  long long total_skipgrams2_ = 0;
  long long total_skipgrams3_ = 0;
  long long total_trigrams_ = 0;

  WideNgramFrequencyMap bigram_freqs_;
  WideNgramFrequencyMap skipgram_freqs_;
  WideNgramFrequencyMap skipgram2_freqs_;
  WideNgramFrequencyMap skipgram3_freqs_;
  WideNgramFrequencyMap trigram_freqs_;
};

void to_json(json& j, const RawCorpusStats& s);

void from_json(const json& j, RawCorpusStats& s);