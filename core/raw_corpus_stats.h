#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

struct RawCorpusStats {
  long long total_chars = 0;
  std::unordered_map<char, long long> characters;

  long long total_bigrams = 0, total_skipgrams = 0, total_skipgrams2 = 0,
            total_skipgrams3 = 0, total_trigrams = 0;
  std::unordered_map<std::string, long long> bigrams, skipgrams, skipgrams2,
      skipgrams3, trigrams;

  RawCorpusStats() = default;
  RawCorpusStats(std::istream& text_stream);

  void addWord(const std::string& word);

  nlohmann::json as_json() const;

  void save_as_json(std::ostream& out_stream) const;

  void save_as_json(std::string file_name) const;
};