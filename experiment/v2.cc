#include <bits/stdc++.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

constexpr int KEY_SIZE = 30;
constexpr char KEYS[KEY_SIZE + 1] = "etaoinsrhldcumfpgywb.v,k'xjq;z";
// constexpr char KEYS[KEY_SIZE + 1] = "abcdefghijklmnopqrstuvwxyz',.;";
constexpr int MAX_ASCII = *max_element(KEYS, KEYS + KEY_SIZE) + 1;

constexpr int NUM_BUCKET = 4;
constexpr int bucket_cap[NUM_BUCKET] = {3, 3, 3, 6};
constexpr int bucket_cnt[NUM_BUCKET] = {2, 2, 2, 2};
constexpr int MAX_BUCKET_CNT =
    *max_element(bucket_cnt, bucket_cnt + NUM_BUCKET);
constexpr int TOTAL_BUCKET = accumulate(bucket_cnt, bucket_cnt + NUM_BUCKET, 0);

const int STATS_NUM = 1;
const string stats_fname[STATS_NUM] = {
    "static/oxeylyzer_stats/english.json",
    //  "static/oxeylyzer_stats/indonesian.json",
    //  "static/oxeylyzer_stats/450k.json",
};
const double stats_weight[STATS_NUM] = {
    1,
    // 1,
    // 1,
};

constexpr double THRESHOLD_SFB = 0.01;
constexpr double THRESHOLD_SFS = 0.065;
constexpr array<double, NUM_BUCKET> THRESHOLD_FINGER_USAGE{0.12, 0.14, 0.16,
                                                           0.18};

constexpr double WEIGHT_SFB = 1;
constexpr double WEIGHT_SFS = WEIGHT_SFB;
constexpr array<double, NUM_BUCKET> WEIGHT_FINGER_USAGE = {4, 3, 2, 1};
constexpr array<double, NUM_BUCKET> WEIGHT_SFB_PER_FINGER = {8, 4, 2, 1};

// [bucket_id][bucket_cnt_id] => string
using Buckets = array<vector<vector<char>>, NUM_BUCKET>;

int num_bucket;
Buckets buckets;

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

struct CompactStats {
  array<double, MAX_ASCII> characters;
  array<array<double, MAX_ASCII>, MAX_ASCII> bigrams, skipgrams;

  CompactStats() : characters({}), bigrams({}), skipgrams({}){};
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
  }
} compact_stats;

Stats readStats(filesystem::path f_name) {
  ifstream f(f_name);
  json j;
  f >> j;
  return j.template get<Stats>();
}

CompactStats readAllStats() {
  Stats ans;
  double total = 0;
  for (int i = 0; i < STATS_NUM; i++) {
    Stats cur = readStats(stats_fname[i]);
    Stats mult = cur * stats_weight[i];
    ans = ans + cur * stats_weight[i];
    total += stats_weight[i];
  }
  return CompactStats(ans / total);
}

double sfb, sfs;
array<array<double, MAX_BUCKET_CNT>, NUM_BUCKET> sfb_bucket = {{}},
                                                 finger_usage = {{}};
double score = 0, best_score = 1e18;

// Bruteforce metadata
int recursion_depth = 0;
long long num_iterations = 0;

void done() {
  best_score = min(best_score, score);
  printf("New layout | Score: %4.6lf | SFB: %2.3lf | SFS: %2.3lf | Bucket: | ",
         score * 100, sfb * 100, sfs * 100);

  for (int i = 0; i < NUM_BUCKET; ++i) {
    for (int j = 0; j < bucket_cnt[i]; j++) {
      printf("%s ", string(buckets[i][j].begin(), buckets[i][j].end()).c_str());
    }
    printf(" ");
  }
  printf("|\n");

  printf("Finger Usage:\n");
  for (int i = 0; i < NUM_BUCKET; ++i) {
    printf("   ");
    for (int j = 0; j < bucket_cnt[i]; j++)
      printf("%2.3lf, ", finger_usage[i][j] * 100);
    printf("\n");
  }

  printf("SFB per bucket Usage:\n");
  for (int i = 0; i < NUM_BUCKET; ++i) {
    printf("   ");
    for (int j = 0; j < bucket_cnt[i]; j++)
      printf("%2.3lf, ", sfb_bucket[i][j] * 100);
    printf("\n");
  }
}

inline bool isPrunable() {
  ++num_iterations;
  if (sfb > THRESHOLD_SFB || sfs > THRESHOLD_SFS) {
    return true;
  }
  for (int i = 0; i < NUM_BUCKET; ++i) {
    for (size_t j = 0; j < buckets[i].size(); ++j) {
      if (finger_usage[i][j] > THRESHOLD_FINGER_USAGE[i]) {
        return true;
      }
    }
  }

  if (score > best_score) return true;

  return false;
}

int is_char_unused = (1 << KEY_SIZE) - 1;
inline void toogleCharUsed(int id) { is_char_unused ^= 1 << id; }
inline int getFirstUnusedKey() {
  assert(is_char_unused > 0);
  return __builtin_ctz(is_char_unused);
}

inline int getAllHigherThanIdx(int id) {
  return is_char_unused & ~((1 << (id + 1)) - 1);
}

inline void updateStats(int const bucket_id, char const key,
                        int const modifier) {
  double const delta_finger_usage = compact_stats.characters[key] * modifier;
  int const bucket_cnt_id = buckets[bucket_id].size() - 1;
  finger_usage[bucket_id][bucket_cnt_id] += delta_finger_usage;
  score += delta_finger_usage * WEIGHT_FINGER_USAGE[bucket_id];

  for (char const assigned_key : buckets[bucket_id].back()) {
    double const delta_sfb =
        compact_stats.bigrams[key][assigned_key] * modifier;
    double const delta_sfs =
        compact_stats.bigrams[key][assigned_key] * modifier;
    sfb += delta_sfb;
    sfs += delta_sfs;
    sfb_bucket[bucket_id][bucket_cnt_id] += delta_sfb;

    score += delta_sfb * WEIGHT_SFB + delta_sfs * WEIGHT_SFS +
             delta_sfb * WEIGHT_SFB_PER_FINGER[bucket_id];
  }
}

inline void pushKey(int bucket_id, char key) {
  updateStats(bucket_id, key, 1);
  buckets[bucket_id].back().push_back({key});
  ++recursion_depth;
}

inline void popKey(int bucket_id, char key) {
  buckets[bucket_id].back().pop_back();
  updateStats(bucket_id, key, -1);
  --recursion_depth;
}

inline void pushBucket(int bucket_id, char key) {
  buckets[bucket_id].push_back({});
  ++num_bucket;
  pushKey(bucket_id, key);
}

inline void popBucket(int bucket_id, char key) {
  popKey(bucket_id, key);
  buckets[bucket_id].pop_back();
  --num_bucket;
}

inline void printProgressAddBucket(int bucket_id, char key) {
  if (recursion_depth <= 3) {
    string indent(recursion_depth * 2, ' ');
    fprintf(stderr, "%s ADD BUCKET %d KEY %c\n", indent.c_str(), bucket_id,
            key);
  }
}

inline void printProgressAddKey(char key) {
  if (recursion_depth <= 3) {
    string indent(recursion_depth * 2, ' ');
    fprintf(stderr, "%s ADD KEY %c\n", indent.c_str(), key);
  }
}

void brute(int const bucket_id, int const num_key_in_bucket,
           int const last_key_id) {
  if (isPrunable()) {
    return;
  }

  int const remaining_key_cap = bucket_cap[bucket_id] - num_key_in_bucket;
  bool const is_bucket_full = remaining_key_cap == 0;
  if (num_bucket == TOTAL_BUCKET && is_bucket_full) {
    done();
    return;
  }

  if (is_bucket_full) {
    int const unused_idx = getFirstUnusedKey();
    char const key = KEYS[unused_idx];
    toogleCharUsed(unused_idx);
    // setup new bucket
    for (int i = 0; i < NUM_BUCKET; i++)
      if ((int)buckets[i].size() != bucket_cnt[i]) {
        printProgressAddBucket(i, key);
        pushBucket(i, key);
        brute(i, 1, unused_idx);
        popBucket(i, key);
      }
    toogleCharUsed(unused_idx);
    return;
  }

  for (int mask = getAllHigherThanIdx(last_key_id);
       __builtin_popcount(mask) >= remaining_key_cap; mask ^= mask & -mask) {
    int const i = __builtin_ctz(mask);
    char const key = KEYS[i];
    printProgressAddKey(key);

    pushKey(bucket_id, key);
    toogleCharUsed(i);
    brute(bucket_id, num_key_in_bucket + 1, i);
    toogleCharUsed(i);
    popKey(bucket_id, key);
  }
}

int main() {
  compact_stats = readAllStats();
  brute(0, bucket_cap[0], 0);
}