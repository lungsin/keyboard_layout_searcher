#include <bits/stdc++.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

constexpr int KEY_SIZE = 30;
constexpr char KEYS[KEY_SIZE + 1] = "etaoinsrhldcumfpgywb.v,k'xjq;z";
constexpr int MAX_ASCII = *max_element(KEYS, KEYS + KEY_SIZE) + 1;

constexpr int NUM_BUCKET = 4;
constexpr int bucket_cap[NUM_BUCKET] = {3, 3, 3, 6};
constexpr int bucket_cnt[NUM_BUCKET] = {2, 2, 2, 2};
constexpr int MAX_BUCKET_CNT =
    *max_element(bucket_cnt, bucket_cnt + NUM_BUCKET);
// constexpr int TOTAL_BUCKET = accumulate(bucket_cnt, bucket_cnt + NUM_BUCKET,
// 0);

const int STATS_NUM = 3;
const string stats_fname[STATS_NUM] = {
    "static/oxeylyzer_stats/english.json",
    "static/oxeylyzer_stats/450k.json",
    "static/oxeylyzer_stats/indonesian.json",
};
const double stats_weight[STATS_NUM] = {
    3,
    2,
    2,
};

constexpr double THRESHOLD_SFB = 0.09;
constexpr double THRESHOLD_SFS = 0.080;

constexpr bool ENABLE_FINGER_THRESHOLD = true;
constexpr array<double, NUM_BUCKET> THRESHOLD_FINGER_USAGE{0.12, 0.14, 0.5,
                                                           0.5};

constexpr double WEIGHT_SFB = 1;
constexpr double WEIGHT_SFS = 1.5;
constexpr double WEIGHT_FINGER_USAGE_OVERALL = 0;
constexpr double WEIGHT_SFB_PER_FINGER_OVERALL = 1;
constexpr array<double, NUM_BUCKET> WEIGHT_FINGER_USAGE = {3, 2, 2, 1};
constexpr array<double, NUM_BUCKET> WEIGHT_SFB_PER_FINGER = {4, 3, 2, 1};

// [bucket_id][bucket_cnt_id] => string
using Buckets = array<vector<vector<char>>, NUM_BUCKET>;

int num_bucket;
Buckets buckets, best_buckets;

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

pair<CompactStats, vector<Stats>> readAllStats() {
  Stats ans;
  double total = 0;
  vector<Stats> stats_list;
  for (int i = 0; i < STATS_NUM; i++) {
    Stats cur = readStats(stats_fname[i]);
    stats_list.push_back(cur);
    ans = ans + cur * stats_weight[i];
    total += stats_weight[i];
  }
  return {CompactStats(ans / total), stats_list};
}

double sfb, sfs;
array<array<double, MAX_BUCKET_CNT>, NUM_BUCKET> sfb_bucket = {{}},
                                                 finger_usage = {{}};
double score = 0, best_score = 1e18;

// Bruteforce metadata
int recursion_depth = 0;
long long num_iterations = 0;

void done() {
  assert(best_score >= score);
  best_score = score;
  best_buckets = buckets;
}

inline bool isPrunable() {
  ++num_iterations;
  if (sfb > THRESHOLD_SFB || sfs > THRESHOLD_SFS) {
    return true;
  }

  if (ENABLE_FINGER_THRESHOLD) {
    for (int i = 0; i < NUM_BUCKET; ++i) {
      for (size_t j = 0; j < buckets[i].size(); ++j) {
        if (finger_usage[i][j] > THRESHOLD_FINGER_USAGE[i]) {
          return true;
        }
      }
    }
  }

  if (score > best_score) return true;

  return false;
}

double fingerUsageScore(double const one_finger_usage, int const bucket_id) {
  return max(one_finger_usage - THRESHOLD_FINGER_USAGE[bucket_id], 0.0) *
         WEIGHT_FINGER_USAGE[bucket_id] * WEIGHT_FINGER_USAGE_OVERALL;
}

inline void updateStatsGlobal(int const bucket_id, int const bucket_cnt_id,
                              char const key, int const modifier) {
  double const delta_finger_usage = compact_stats.characters[key] * modifier;
  score -= fingerUsageScore(finger_usage[bucket_id][bucket_cnt_id], bucket_id);
  finger_usage[bucket_id][bucket_cnt_id] += delta_finger_usage;
  score += fingerUsageScore(finger_usage[bucket_id][bucket_cnt_id], bucket_id);

  for (char const key_2 : buckets[bucket_id][bucket_cnt_id]) {
    double const delta_sfb = compact_stats.bigrams[key][key_2] * modifier;
    double const delta_sfs = compact_stats.skipgrams[key][key_2] * modifier;
    sfb += delta_sfb;
    sfs += delta_sfs;
    sfb_bucket[bucket_id][bucket_cnt_id] += delta_sfb;

    score += delta_sfb * WEIGHT_SFB + delta_sfs * WEIGHT_SFS;
    score += delta_sfb * WEIGHT_SFB_PER_FINGER[bucket_id] *
             WEIGHT_SFB_PER_FINGER_OVERALL;
  }
}

inline void pushKey(int bucket_id, int bucket_cnt_id, char key) {
  updateStatsGlobal(bucket_id, bucket_cnt_id, key, 1);
  buckets[bucket_id][bucket_cnt_id].push_back({key});
  ++recursion_depth;
}

inline void popKey(int bucket_id, int bucket_cnt_id, char key) {
  buckets[bucket_id][bucket_cnt_id].pop_back();
  updateStatsGlobal(bucket_id, bucket_cnt_id, key, -1);
  --recursion_depth;
}

inline void pushBucket(int bucket_id, char key) {
  buckets[bucket_id].push_back({});
  ++num_bucket;
  pushKey(bucket_id, buckets[bucket_id].size() - 1, key);
}

inline void popBucket(int bucket_id, char key) {
  popKey(bucket_id, buckets[bucket_id].size() - 1, key);
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

inline void printProgressAddKey(int bucket_id, int bucket_cnt_id, char key) {
  if (recursion_depth <= 3) {
    string indent(recursion_depth * 2, ' ');
    fprintf(stderr, "%s ADD KEY %d %d : %c\n", indent.c_str(), bucket_id,
            bucket_cnt_id, key);
  }
}

void brute(int key_id) {
  if (isPrunable()) {
    return;
  }

  if (key_id == KEY_SIZE) {
    done();
    return;
  }

  char const key = KEYS[key_id];
  // new buckets
  for (int i = 0; i < NUM_BUCKET; i++) {
    if (buckets[i].size() == bucket_cnt[i]) continue;
    printProgressAddBucket(i, key);
    pushBucket(i, key);
    brute(key_id + 1);
    popBucket(i, key);
  }

  for (int i = 0; i < NUM_BUCKET; i++) {
    for (int j = 0; j < buckets[i].size(); j++) {
      if (buckets[i][j].size() == bucket_cap[i]) continue;
      printProgressAddKey(i, j, key);
      pushKey(i, j, key);
      brute(key_id + 1);
      popKey(i, j, key);
    }
  }
}

void printBucketStats(Buckets const& buckets, CompactStats const& stats) {
  double sfb_ = 0, sfs_ = 0;
  array<array<double, MAX_BUCKET_CNT>, NUM_BUCKET> sfb_bucket_ = {{}},
                                                   finger_usage_ = {{}};
  for (size_t bucket_id = 0; bucket_id < NUM_BUCKET; bucket_id++) {
    for (size_t bucket_cnt_id = 0; bucket_cnt_id < bucket_cnt[bucket_id];
         bucket_cnt_id++) {
      for (char const key : buckets[bucket_id][bucket_cnt_id]) {
        double const delta_finger_usage = stats.characters[key];
        finger_usage_[bucket_id][bucket_cnt_id] += delta_finger_usage;

        for (char const key_2 : buckets[bucket_id][bucket_cnt_id]) {
          if (key_2 <= key) continue;
          double const delta_sfb = stats.bigrams[key][key_2];
          double const delta_sfs = stats.skipgrams[key][key_2];
          sfb_ += delta_sfb;
          sfs_ += delta_sfs;
          sfb_bucket_[bucket_id][bucket_cnt_id] += delta_sfb;
        }
      }
    }
  }

  printf("SFB: %2.3lf | SFS: %2.3lf | Bucket: | ", sfb_ * 100, sfs_ * 100);

  for (int i = 0; i < NUM_BUCKET; ++i) {
    int j = 0;
    printf("%s ", string(buckets[i][j].begin(), buckets[i][j].end()).c_str());
  }
  printf(" ");
  for (int i = NUM_BUCKET - 1; i >= 0; --i) {
    int j = 1;
    printf("%s ", string(buckets[i][j].begin(), buckets[i][j].end()).c_str());
  }
  printf("|\n");

  printf("Finger Usage:\n");
  for (int i = 0; i < NUM_BUCKET; ++i) {
    printf("   ");
    for (int j = 0; j < bucket_cnt[i]; j++)
      printf("%2.3lf, ", finger_usage_[i][j] * 100);
    printf("\n");
  }

  printf("SFB per bucket Usage:\n");
  for (int i = 0; i < NUM_BUCKET; ++i) {
    printf("   ");
    for (int j = 0; j < bucket_cnt[i]; j++)
      printf("%2.3lf, ", sfb_bucket_[i][j] * 100);
    printf("\n");
  }
}

int main() {
  auto [aggregated_stats, stats_list] = readAllStats();
  compact_stats = aggregated_stats;
  brute(0);

  for (int i = 0; i < (int)stats_list.size(); i++) {
    cout << "==== Stats " << stats_fname[i] << endl;
    printBucketStats(best_buckets, CompactStats(stats_list[i]));
    cout << endl;
  }
}