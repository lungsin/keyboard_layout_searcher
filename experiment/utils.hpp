#include <bits/stdc++.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

map<string, double> add(map<string, double> const& l,
                        map<string, double> const& r) {
  map<string, double> ans = l;
  for (auto [s, p] : r) ans[s] += p;
  return ans;
}

map<string, double> mul(map<string, double> const& l, double const& v) {
  map<string, double> ans;
  for (auto [s, p] : l) ans[s] += p * v;
  return ans;
}

struct Stats {
  map<string, double> characters, bigrams, skipgrams, trigrams;

  Stats operator+(Stats s) const {
    return {
        add(characters, s.characters),
        add(bigrams, s.bigrams),
        add(skipgrams, s.skipgrams),
        add(trigrams, s.trigrams),
    };
  }
  Stats operator*(double v) const {
    return {
        mul(characters, v),
        mul(bigrams, v),
        mul(skipgrams, v),
        mul(trigrams, v),
    };
  }
  Stats operator/(double v) const { return (*this) * (1.0 / v); }
};

void to_json(json& j, const Stats& s) {
  j = json{
      {"characters", s.characters},
      {"bigrams", s.bigrams},
      {"skipgrams", s.skipgrams},
      {"trigrams", s.trigrams},
  };
}

void from_json(const json& j, Stats& s) {
  j.at("characters").get_to(s.characters);
  j.at("bigrams").get_to(s.bigrams);
  j.at("skipgrams").get_to(s.skipgrams);
  j.at("trigrams").get_to(s.trigrams);
}

Stats calculateStatsFromText(filesystem::path f_name,
                             map<char, char> const& mapper) {
  array<char, 256> mapper_arr = {{}};
  for (auto const [from, to] : mapper) mapper_arr[from + 128] = to;

  long long total_characters = 0, total_bigrams = 0, total_skipgrams = 0,
            total_trigrams = 0;
  map<string, long long> characters, bigrams, skipgrams, trigrams;

  ifstream in(f_name);
  string word;
  while (in >> word) {
    for (size_t i = 0, j = -1; i < word.size(); ++i) {
      word[i] = mapper_arr[word[i] + 128];
      if (!word[i]) {
        j = i;
        continue;
      }

      ++total_characters;
      ++characters[{word[i]}];

      if (i - j >= 2) {
        ++total_bigrams;
        ++bigrams[word.substr(j + 1, 2)];
      }

      if (i - j >= 3) {
        ++total_trigrams;
        ++trigrams[word.substr(j + 1, 3)];
        ++total_skipgrams;
        ++skipgrams[{word[i - 2], word[i]}];
      }
    }
  }

  Stats result;
  for (auto [s, freq] : characters)
    result.characters[s] = (double)freq / total_characters;
  for (auto [s, freq] : bigrams)
    result.bigrams[s] = (double)freq / total_bigrams;
  for (auto [s, freq] : skipgrams)
    result.skipgrams[s] = (double)freq / total_skipgrams;
  for (auto [s, freq] : trigrams)
    result.trigrams[s] = (double)freq / total_trigrams;

  return result;
}

Stats readStats(filesystem::path f_name) {
  ifstream f(f_name);
  json j;
  f >> j;
  return j.template get<Stats>();
}

template <int MAX_SIZE>
struct CompactStats {
  array<double, MAX_SIZE> characters;
  array<array<double, MAX_SIZE>, MAX_SIZE> bigrams, skipgrams;
  array<array<array<double, MAX_SIZE>, MAX_SIZE>, MAX_SIZE> trigrams;

  CompactStats() : characters({}), bigrams({}), skipgrams({}), trigrams({}){};
  CompactStats(Stats const& stats) : CompactStats() {
    for (auto [s, p] : stats.characters) characters[s[0]] += p;
    for (auto [s, p] : stats.bigrams) {
      bigrams[s[0]][s[1]] += p;
      bigrams[s[1]][s[0]] += p;
    }
    for (auto [s, p] : stats.skipgrams) {
      skipgrams[s[0]][s[1]] += p;
      skipgrams[s[1]][s[0]] += p;
    }
    for (auto [s, p] : stats.trigrams) {
      trigrams[s[0]][s[1]][s[2]] += p;
    }
  }
};
