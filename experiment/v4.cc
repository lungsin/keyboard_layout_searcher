#include <bits/stdc++.h>

#include <nlohmann/json.hpp>

#include "utils.hpp"

using json = nlohmann::json;
using namespace std;

constexpr int KEY_SIZE = 30;
constexpr char KEYS[KEY_SIZE + 1] = "etaoinsrhldcumfpgywb.v,k'xjq;z";
constexpr int MAX_ASCII = *max_element(KEYS, KEYS + KEY_SIZE) + 1;

// Each bucket represents a finger.
constexpr int NUM_BUCKET = 4;
// alternating left-right from pinky to index finger
constexpr int BUCKET_CAP[NUM_BUCKET] = {3, 3, 3, 6};
constexpr int BUCKET_CNT[NUM_BUCKET] = {2, 2, 2, 2};
constexpr int MAX_BUCKET_CNT =
    *max_element(BUCKET_CNT, BUCKET_CNT + NUM_BUCKET);

const int NUM_STATS = 3;
const string STATS_FNAME[NUM_STATS] = {
    "static/oxeylyzer_stats/english.json",
    "static/oxeylyzer_stats/450k.json",
    "static/oxeylyzer_stats/indonesian.json",
};
const double STATS_WEIGHT[NUM_STATS] = {
    4,
    1,
    0,
};

constexpr double AGGREGATED_THRESHOLD_SFB = 0.020;
constexpr array<double, NUM_STATS> THRESHOLD_SFB_PER_LANG = {0.0085, 0.0095,
                                                             0.0100};
constexpr double AGGREGATED_THRESHOLD_SFS = 0.085;
constexpr array<double, NUM_STATS> THRESHOLD_SFS_PER_LANG = {0.060, 0.065,
                                                             0.065};

constexpr bool ENABLE_FINGER_THRESHOLD = false;
constexpr array<double, NUM_BUCKET> THRESHOLD_FINGER_USAGE = {0.12, 0.14, 0.16,
                                                              0.18};

constexpr double WEIGHT_SFB = 1;
constexpr double WEIGHT_SFS = 1;
constexpr double WEIGHT_FINGER_USAGE_OVERALL = 1;
constexpr double WEIGHT_SFB_PER_FINGER_OVERALL = 1;
constexpr array<double, NUM_BUCKET> WEIGHT_FINGER_USAGE = {4, 2, 2, 1};
constexpr array<double, NUM_BUCKET> WEIGHT_SFB_PER_FINGER = {4, 3, 2, 1};

// [bucket_id][bucket_cnt_id] => string
using Buckets = array<vector<vector<char>>, NUM_BUCKET>;

using FastStats = CompactStats<MAX_ASCII>;

pair<FastStats, array<FastStats, NUM_STATS>> readAllStats() {
  Stats ans;
  double total = 0;
  array<FastStats, NUM_STATS> stats_list;
  for (int i = 0; i < NUM_STATS; i++) {
    Stats cur = readStats(STATS_FNAME[i]);
    stats_list[i] = cur;
    ans = ans + cur * STATS_WEIGHT[i];
    total += STATS_WEIGHT[i];
  }
  return {FastStats(ans / total), stats_list};
}

inline double fingerUsageScore(double const one_finger_usage,
                               int const bucket_id) {
  return max(one_finger_usage - THRESHOLD_FINGER_USAGE[bucket_id], 0.0) *
         WEIGHT_FINGER_USAGE[bucket_id] * WEIGHT_FINGER_USAGE_OVERALL;
}

struct LayoutStats {
  double sfb = 0, sfs = 0;

  array<array<double, MAX_BUCKET_CNT>, NUM_BUCKET> sfb_bucket = {{}},
                                                   finger_usage = {{}};

  // trigram stats
  double alternates = 0, redirects = 0, rolls = 0;

  // weighted score
  double score = 0;

  FastStats corpus_stats;

  LayoutStats() = default;
  LayoutStats(FastStats const& corpus_stats) : corpus_stats(corpus_stats) {}

  inline void updateStats(Buckets const& buckets, int const bucket_id,
                          int const bucket_cnt_id, char const key,
                          int const modifier) {
    double const delta_finger_usage = corpus_stats.characters[key] * modifier;
    double& bucket_finger_usage = finger_usage[bucket_id][bucket_cnt_id];
    score -= fingerUsageScore(bucket_finger_usage, bucket_id);
    bucket_finger_usage += delta_finger_usage;
    score += fingerUsageScore(bucket_finger_usage, bucket_id);

    for (char const key_2 : buckets[bucket_id][bucket_cnt_id]) {
      double const delta_sfb = corpus_stats.bigrams[key][key_2] * modifier;
      double const delta_sfs = corpus_stats.skipgrams[key][key_2] * modifier;
      sfb += delta_sfb;
      sfs += delta_sfs;
      sfb_bucket[bucket_id][bucket_cnt_id] += delta_sfb;

      score += delta_sfb * WEIGHT_SFB + delta_sfs * WEIGHT_SFS;
      score += delta_sfb * WEIGHT_SFB_PER_FINGER[bucket_id] *
               WEIGHT_SFB_PER_FINGER_OVERALL;
    }

    // TODO: update the trigram stats
  }
};

struct PartialLayout {
  Buckets buckets;

  LayoutStats aggregated_stats;
  array<LayoutStats, NUM_STATS> stats_per_language;

  inline void updateStats(int const bucket_id, int const bucket_cnt_id,
                          char const key, int const modifier) {
    aggregated_stats.updateStats(buckets, bucket_id, bucket_cnt_id, key,
                                 modifier);
    for (auto& stats : stats_per_language)
      stats.updateStats(buckets, bucket_id, bucket_cnt_id, key, modifier);
  }

  inline void pushKey(int bucket_id, int const bucket_cnt_id, char key) {
    updateStats(bucket_id, bucket_cnt_id, key, 1);
    buckets[bucket_id][bucket_cnt_id].push_back({key});
  }

  inline void popKey(int bucket_id, int const bucket_cnt_id, char key) {
    buckets[bucket_id][bucket_cnt_id].pop_back();
    updateStats(bucket_id, bucket_cnt_id, key, -1);
  }

  inline void pushBucket(int bucket_id, char key) {
    buckets[bucket_id].push_back({});
    pushKey(bucket_id, buckets[bucket_id].size() - 1, key);
  }

  inline void popBucket(int bucket_id, char key) {
    popKey(bucket_id, buckets[bucket_id].size() - 1, key);
    buckets[bucket_id].pop_back();
  }
};

PartialLayout partial_layout, best_partial_layout;

// Bruteforce metadata
int recursion_depth = 0;
long long num_iterations = 0, num_done = 0;

inline void done() {
  assert(best_partial_layout.aggregated_stats.score >=
         partial_layout.aggregated_stats.score);
  best_partial_layout = partial_layout;
  ++num_done;
}

inline bool isPrunable() {
  ++num_iterations;
  LayoutStats const& aggregated_layout_stats = partial_layout.aggregated_stats;
  if (aggregated_layout_stats.sfb > AGGREGATED_THRESHOLD_SFB ||
      aggregated_layout_stats.sfs > AGGREGATED_THRESHOLD_SFS)
    return true;

  if (aggregated_layout_stats.score >
      best_partial_layout.aggregated_stats.score)
    return true;

  for (int i = 0; i < NUM_STATS; ++i) {
    auto const& stats = partial_layout.stats_per_language[i];
    if (stats.sfb > THRESHOLD_SFB_PER_LANG[i] ||
        stats.sfs > THRESHOLD_SFS_PER_LANG[i])
      return true;
  }

  if (ENABLE_FINGER_THRESHOLD) {
    for (int i = 0; i < NUM_BUCKET; ++i)
      for (size_t j = 0; j < partial_layout.buckets[i].size(); ++j)
        if (aggregated_layout_stats.finger_usage[i][j] >
            THRESHOLD_FINGER_USAGE[i])
          return true;
  }

  return false;
}

inline void pushKey(int bucket_id, int const bucket_cnt_id, char key) {
  ++recursion_depth;
  partial_layout.pushKey(bucket_id, bucket_cnt_id, key);
}

inline void popKey(int bucket_id, int const bucket_cnt_id, char key) {
  --recursion_depth;
  partial_layout.popKey(bucket_id, bucket_cnt_id, key);
}

inline void pushBucket(int bucket_id, char key) {
  ++recursion_depth;
  partial_layout.pushBucket(bucket_id, key);
}

inline void popBucket(int bucket_id, char key) {
  --recursion_depth;
  partial_layout.popBucket(bucket_id, key);
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
    if (partial_layout.buckets[i].size() == BUCKET_CNT[i]) continue;
    printProgressAddBucket(i, key);
    pushBucket(i, key);
    brute(key_id + 1);
    popBucket(i, key);
  }

  for (int i = 0; i < NUM_BUCKET; i++) {
    for (int j = 0; j < partial_layout.buckets[i].size(); j++) {
      if (partial_layout.buckets[i][j].size() == BUCKET_CAP[i]) continue;
      printProgressAddKey(i, j, key);
      pushKey(i, j, key);
      brute(key_id + 1);
      popKey(i, j, key);
    }
  }
}

void printBuckets(Buckets const& buckets) {
  printf("Bucket: ");

  for (int i = 0; i < NUM_BUCKET; ++i) {
    printf("%s ", string(buckets[i][0].begin(), buckets[i][0].end()).c_str());
  }
  printf(" ");
  for (int i = NUM_BUCKET - 1; i >= 0; --i) {
    printf("%s ", string(buckets[i][1].begin(), buckets[i][1].end()).c_str());
  }
  printf("\n");
}

void printBucketStats(LayoutStats const& layout_stats) {
  printf("SFB: %2.3lf | SFS: %2.3lf\n", layout_stats.sfb * 100,
         layout_stats.sfs * 100);

  printf("Finger Usage:\n");
  for (int i = 0; i < NUM_BUCKET; ++i) {
    printf("   ");
    for (int j = 0; j < BUCKET_CNT[i]; j++)
      printf("%2.3lf, ", layout_stats.finger_usage[i][j] * 100);
    printf("\n");
  }

  printf("SFB per bucket Usage:\n");
  for (int i = 0; i < NUM_BUCKET; ++i) {
    printf("   ");
    for (int j = 0; j < BUCKET_CNT[i]; j++)
      printf("%2.3lf, ", layout_stats.sfb_bucket[i][j] * 100);
    printf("\n");
  }
}

int main() {
  auto [aggregated_stats, stats_list] = readAllStats();

  partial_layout.aggregated_stats = aggregated_stats;
  best_partial_layout.aggregated_stats.score = 1e18;
  for (int i = 0; i < NUM_STATS; i++) {
    partial_layout.stats_per_language[i] = stats_list[i];
  }

  brute(0);

  cerr << "==== Metadata ====\nNum Iterations: " << num_iterations << endl;
  cerr << "Num done: " << num_done << endl;

  printBuckets(best_partial_layout.buckets);
  for (int i = 0; i < NUM_STATS; i++) {
    cout << "==== Stats " << STATS_FNAME[i] << endl;
    printBucketStats(best_partial_layout.stats_per_language[i]);
    cout << endl;
  }
}