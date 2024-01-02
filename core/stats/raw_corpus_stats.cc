#include "raw_corpus_stats.h"

#include <fstream>
#include <unordered_set>

#include "core/unicode/unicode.h"
#include "utils.h"

RawCorpusStats::RawCorpusStats(std::istream& text_stream) {
  for (std::string word; text_stream >> word;) {
    addWord(unicode::toWide(word));
  }
}

RawCorpusStats::RawCorpusStats(const json& j) {
  auto process_json_ngram_freqs = [&](std::string const& json_key,
                                      WideNgramFrequencyMap& result) {
    FrequencyMap<std::string> narrow_map;
    j.at(json_key).get_to(narrow_map);
    result = unicode::toWide(narrow_map);
  };

  auto process_json_char_freqs = [&](std::string const& json_key,
                                     FrequencyMap<WideChar>& result) {
    WideNgramFrequencyMap wide_map;
    process_json_ngram_freqs(json_key, wide_map);
    for (auto const& [ngram, freq] : wide_map) {
      if (ngram.size() != 1) continue;
      result[ngram[0]] += freq;
    }
  };

  process_json_char_freqs("char_freqs", char_freqs_);
  process_json_ngram_freqs("bigram_freqs", bigram_freqs_);
  process_json_ngram_freqs("skipgram_freqs", skipgram_freqs_);
  process_json_ngram_freqs("skipgram2_freqs", skipgram2_freqs_);
  process_json_ngram_freqs("skipgram3_freqs", skipgram3_freqs_);
  process_json_ngram_freqs("trigram_freqs", trigram_freqs_);

  total_chars_ = getTotalFreqs(char_freqs_);
  total_bigrams_ = getTotalFreqs(bigram_freqs_);
  total_skipgrams_ = getTotalFreqs(skipgram_freqs_);
  total_skipgrams2_ = getTotalFreqs(skipgram2_freqs_);
  total_skipgrams3_ = getTotalFreqs(skipgram3_freqs_);
  total_trigrams_ = getTotalFreqs(trigram_freqs_);
}

RawCorpusStats RawCorpusStats::fromTextStream(std::istream& text_stream) {
  return RawCorpusStats(text_stream);
}
RawCorpusStats RawCorpusStats::fromTextFile(std::string const& text_path) {
  std::ifstream text_stream(text_path);
  return fromTextStream(text_stream);
}
RawCorpusStats RawCorpusStats::fromJsonStream(std::istream& json_stream) {
  json j;
  json_stream >> j;
  return RawCorpusStats(j);
}
RawCorpusStats RawCorpusStats::fromJsonFile(std::string const& json_path) {
  std::ifstream json_stream(json_path);
  return fromJsonStream(json_stream);
}

void RawCorpusStats::addWord(const WideString& word) {
  const int n = word.size();
  for (const WideChar& c : word) {
    ++char_freqs_[c];
  }
  for (int i = 1; i < n; ++i) {
    ++bigram_freqs_[{word[i - 1], word[i]}];
  }
  for (int i = 2; i < n; ++i) {
    ++trigram_freqs_[{word[i - 2], word[i - 1], word[i]}];
    ++skipgram_freqs_[{word[i - 2], word[i]}];
  }
  for (int i = 3; i < n; ++i) {
    ++skipgram2_freqs_[{word[i - 3], word[i]}];
  }
  for (int i = 4; i < n; ++i) {
    ++skipgram3_freqs_[{word[i - 4], word[i]}];
  }

  total_chars_ += n;
  total_bigrams_ += n - 1;
  total_trigrams_ += n - 2;
  total_skipgrams_ += n - 2;
  total_skipgrams2_ += n - 3;
  total_skipgrams3_ += n - 4;
}

void RawCorpusStats::addWord(const std::string& word) {
  addWord(unicode::toWide(word));
}

void RawCorpusStats::save_as_json(std::ostream& out_stream) const {
  out_stream << std::setw(4) << json(*this) << std::endl;
}

void RawCorpusStats::save_as_json(const std::string& file_name) const {
  std::ofstream out_stream(file_name);
  save_as_json(out_stream);
}

long long RawCorpusStats::getTotalChars() const { return total_chars_; }
long long RawCorpusStats::getTotalBigrams() const { return total_bigrams_; }
long long RawCorpusStats::getTotalSkipgrams() const { return total_skipgrams_; }
long long RawCorpusStats::getTotalSkipgrams2() const {
  return total_skipgrams2_;
}

long long RawCorpusStats::getTotalSkipgrams3() const {
  return total_skipgrams3_;
}

long long RawCorpusStats::getTotalTrigrams() const { return total_trigrams_; }

long long RawCorpusStats::getCharFreq(const WideChar& c) const {
  if (!char_freqs_.contains(c)) return 0LL;
  return char_freqs_.at(c);
}

long long RawCorpusStats::getBigramFreq(const WideChar& c1,
                                        const WideChar& c2) const {
  const WideString s = {c1, c2};
  if (!bigram_freqs_.contains(s)) return 0LL;
  return bigram_freqs_.at(s);
}

long long RawCorpusStats::getSkipgramFreq(const WideChar& c1,
                                          const WideChar& c2) const {
  const WideString s = {c1, c2};
  if (!skipgram_freqs_.contains(s)) return 0LL;
  return skipgram_freqs_.at(s);
}

long long RawCorpusStats::getSkipgram2Freq(const WideChar& c1,
                                           const WideChar& c2) const {
  const WideString s = {c1, c2};
  if (!skipgram2_freqs_.contains(s)) return 0LL;
  return skipgram2_freqs_.at(s);
}

long long RawCorpusStats::getSkipgram3Freq(const WideChar& c1,
                                           const WideChar& c2) const {
  const WideString s = {c1, c2};
  if (!skipgram3_freqs_.contains(s)) return 0LL;
  return skipgram3_freqs_.at(s);
}

long long RawCorpusStats::getTrigramFreq(const WideChar& c1, const WideChar& c2,
                                         const WideChar& c3) const {
  const WideString s = {c1, c2, c3};
  if (!trigram_freqs_.contains(s)) return 0LL;
  return trigram_freqs_.at(s);
}

FrequencyMap<WideChar> RawCorpusStats::getCharFreqPairs() const {
  return char_freqs_;
}

WideNgramFrequencyMap RawCorpusStats::getBigramFreqPairs() const {
  return bigram_freqs_;
}

WideNgramFrequencyMap RawCorpusStats::getSkipgramFreqPairs() const {
  return skipgram_freqs_;
}

WideNgramFrequencyMap RawCorpusStats::getSkipgram2FreqPairs() const {
  return skipgram2_freqs_;
}

WideNgramFrequencyMap RawCorpusStats::getSkipgram3FreqPairs() const {
  return skipgram3_freqs_;
}

WideNgramFrequencyMap RawCorpusStats::getTrigramFreqPairs() const {
  return trigram_freqs_;
}

void RawCorpusStats::replaceChar(const WideChar& from, const WideChar& to) {
  char_freqs_ = ::replaceChar(char_freqs_, from, to);
  bigram_freqs_ = ::replaceChar(bigram_freqs_, from, to);
  skipgram_freqs_ = ::replaceChar(skipgram_freqs_, from, to);
  skipgram2_freqs_ = ::replaceChar(skipgram2_freqs_, from, to);
  skipgram3_freqs_ = ::replaceChar(skipgram3_freqs_, from, to);
  trigram_freqs_ = ::replaceChar(trigram_freqs_, from, to);

  total_chars_ = getTotalFreqs(char_freqs_);
  total_bigrams_ = getTotalFreqs(bigram_freqs_);
  total_skipgrams_ = getTotalFreqs(skipgram_freqs_);
  total_skipgrams2_ = getTotalFreqs(skipgram2_freqs_);
  total_skipgrams3_ = getTotalFreqs(skipgram3_freqs_);
  total_trigrams_ = getTotalFreqs(trigram_freqs_);
}

void RawCorpusStats::removeChar(const WideChar& c) {
  char_freqs_ = ::removeChar(char_freqs_, c);
  bigram_freqs_ = ::removeChar(bigram_freqs_, c);
  skipgram_freqs_ = ::removeChar(skipgram_freqs_, c);
  skipgram2_freqs_ = ::removeChar(skipgram2_freqs_, c);
  skipgram3_freqs_ = ::removeChar(skipgram3_freqs_, c);
  trigram_freqs_ = ::removeChar(trigram_freqs_, c);

  total_chars_ = getTotalFreqs(char_freqs_);
  total_bigrams_ = getTotalFreqs(bigram_freqs_);
  total_skipgrams_ = getTotalFreqs(skipgram_freqs_);
  total_skipgrams2_ = getTotalFreqs(skipgram2_freqs_);
  total_skipgrams3_ = getTotalFreqs(skipgram3_freqs_);
  total_trigrams_ = getTotalFreqs(trigram_freqs_);
}

void RawCorpusStats::whitelistChars(const WideString& whitelist) {
  std::unordered_set<WideChar> whitelist_set(whitelist.begin(),
                                             whitelist.end());
  for (const WideChar& c : char_freqs_ | std::views::keys) {
    if (!whitelist_set.contains(c)) removeChar(c);
  }
}

void to_json(json& j, const RawCorpusStats& obj) {
  j = json{
      {"total_chars", obj.getTotalChars()},
      {"total_bigrams", obj.getTotalBigrams()},
      {"total_skipgrams", obj.getTotalSkipgrams()},
      {"total_skipgrams2", obj.getTotalSkipgrams2()},
      {"total_skipgrams3", obj.getTotalSkipgrams3()},
      {"total_trigrams", obj.getTotalTrigrams()},
      {"char_freqs", unicode::toNarrow(obj.getCharFreqPairs())},
      {"bigram_freqs", unicode::toNarrow(obj.getBigramFreqPairs())},
      {"skipgram_freqs", unicode::toNarrow(obj.getSkipgramFreqPairs())},
      {"skipgram2_freqs", unicode::toNarrow(obj.getSkipgram2FreqPairs())},
      {"skipgram3_freqs", unicode::toNarrow(obj.getSkipgram3FreqPairs())},
      {"trigram_freqs", unicode::toNarrow(obj.getTrigramFreqPairs())},
  };
}

void from_json(const json& j, RawCorpusStats& obj) { obj = RawCorpusStats(j); }