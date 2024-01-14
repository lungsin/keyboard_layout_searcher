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
