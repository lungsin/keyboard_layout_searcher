#include "raw_corpus_stats.h"

#include <fstream>

RawCorpusStats::RawCorpusStats(std::istream& text_stream) {
  for (std::string word; text_stream >> word;) {
    addWord(word);
  }
}

void RawCorpusStats::addWord(const CorpusString& word) {
  const int n = word.size();
  for (const char& c : word) {
    characters[c]++;
  }
  for (int i = 1; i < n; i++) {
    bigrams[{word[i - 1], word[i]}]++;
  }
  for (int i = 2; i < n; i++) {
    trigrams[{word[i - 2], word[i - 1], word[i]}];
    skipgrams[{word[i - 2], word[i]}]++;
  }
  for (int i = 3; i < n; i++) {
    skipgrams2[{word[i - 3], word[i]}]++;
  }
  for (int i = 4; i < n; i++) {
    skipgrams3[{word[i - 4], word[i]}]++;
  }

  total_chars += n;
  total_bigrams += n - 1;
  total_trigrams += n - 2;
  total_skipgrams += n - 2;
  total_skipgrams2 += n - 3;
  total_skipgrams3 += n - 4;
}

nlohmann::json RawCorpusStats::as_json() const {
  nlohmann::json j;
  j["total_chars"] = total_chars;
  j["characters"] = characters;
  j["total_bigrams"] = total_bigrams;
  j["total_skipgrams"] = total_skipgrams;
  j["total_skipgrams2"] = total_skipgrams2;
  j["total_skipgrams3"] = total_skipgrams3;
  j["total_trigrams"] = total_trigrams;
  j["bigrams"] = bigrams;
  j["skipgrams"] = skipgrams;
  j["skipgrams2"] = skipgrams2;
  j["skipgrams3"] = skipgrams3;
  j["trigrams"] = trigrams;
  return j;
}

void RawCorpusStats::save_as_json(std::ostream& out_stream) const {
  out_stream << as_json() << std::endl;
}

void RawCorpusStats::save_as_json(std::string file_name) const {
  std::ofstream out_stream(file_name);
  // save_as_json(out_stream);
  out_stream << as_json() << std::endl;
  out_stream.close();
}
