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
constexpr int NUM_HANDS = 2;

const int NUM_STATS = 3;
const string STATS_FNAME[NUM_STATS] = {
    "static/oxeylyzer_stats/english.json",
    "static/oxeylyzer_stats/450k.json",
    "static/oxeylyzer_stats/indonesian.json",
};
const double STATS_WEIGHT[NUM_STATS] = {
    4,
    1,
    4,
};

using ThresholdPerLang = array<double, NUM_STATS>;
using ThresholdFingerUsage = array<array<double, NUM_BUCKET>, NUM_STATS>;

constexpr double AGGREGATED_THRESHOLD_SFB = 0.020;
constexpr ThresholdPerLang THRESHOLD_SFB_PER_LANG = {0.0090, 0.0095, 0.0100};
constexpr double AGGREGATED_THRESHOLD_SFS = 0.085;
constexpr ThresholdPerLang THRESHOLD_SFS_PER_LANG = {0.065, 0.075, 0.085};

constexpr bool ENABLE_FINGER_THRESHOLD = true;
constexpr ThresholdFingerUsage THRESHOLD_FINGER_USAGE = {
    {{0.12, 0.14, 0.30, 0.30},
     {0.15, 0.15, 0.30, 0.30},
     {0.15, 0.20, 0.30, 0.30}}};

constexpr bool ENABLE_SFB_PER_FINGER_THRESHOLD = true;
constexpr ThresholdFingerUsage THRESHOLD_SFB_PER_FINGER = {
    {{0.001, 0.02, 0.10, 0.10}, {0.1, 0.1, 0.1, 0.1}, {0.1, 0.1, 0.1, 0.1}}};

constexpr ThresholdPerLang THRESHOLD_ALTERNATES_PER_LANG = {0.35, 0.40, 0.40};
constexpr ThresholdPerLang THRESHOLD_ROLLS_PER_LANG = {0.35, 0.35, 0.35};
constexpr ThresholdPerLang THRESHOLD_REDIRECTS_PER_LANG = {0.06, 0.08, 0.11};

constexpr double WEIGHT_SFB = 1;
constexpr double WEIGHT_SFS = 1;
constexpr double WEIGHT_FINGER_USAGE_OVERALL = 0.01;
constexpr double WEIGHT_SFB_PER_FINGER_OVERALL = 1;
constexpr array<double, NUM_BUCKET> WEIGHT_FINGER_USAGE = {8, 4, 2, 1};
constexpr array<double, NUM_BUCKET> WEIGHT_SFB_PER_FINGER = {8, 4, 2, 1};

constexpr double WEIGHT_ALTERNATES = 0.01;
constexpr double WEIGHT_ROLLS = 0.01;
constexpr double WEIGHT_REDIRECT = 0.02;

// [bucket_id][bucket_cnt_id] => string
using Buckets = array<vector<vector<char>>, NUM_BUCKET>;
using BucketToHand = array<vector<int>, NUM_BUCKET>;
using FlatBucketId = int;

using FastStats = CompactStats<MAX_ASCII>;

pair<FastStats, array<FastStats, NUM_STATS>> readAllStats() {
  cerr << "Reading all stats" << endl;
  Stats ans;
  double total = 0;
  array<FastStats, NUM_STATS> stats_list;
  for (int i = 0; i < NUM_STATS; i++) {
    cerr << "  Loading stats file: " << STATS_FNAME[i] << endl;
    Stats cur = readStats(STATS_FNAME[i]);
    stats_list[i] = cur;
    ans = ans + cur * STATS_WEIGHT[i];
    total += STATS_WEIGHT[i];
  }
  return {FastStats(ans / total), stats_list};
}

inline double fingerUsageScore(double const one_finger_usage,
                               int const bucket_id) {
  return one_finger_usage * WEIGHT_FINGER_USAGE[bucket_id] *
         WEIGHT_FINGER_USAGE_OVERALL;
}

inline FlatBucketId toFlatBucketId(int const bucket_id,
                                   int const bucket_cnt_id) {
  return bucket_id * NUM_HANDS + bucket_cnt_id;
}

inline pair<int, int> toBucketIdBucketCntId(FlatBucketId const bucket_flat_id) {
  return {bucket_flat_id >> 1, bucket_flat_id & 1};
}

struct LayoutStats {
  double sfb = 0, sfs = 0;

  array<array<double, NUM_HANDS>, NUM_BUCKET> sfb_bucket = {{}},
                                              finger_usage = {{}};

  // trigram stats
  double alternates = 0, inverse_alternates = 0;
  double rolls = 0, inverse_rolls = 0;
  double redirects = 0;

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
  }

  inline void updateTrigramStats(FlatBucketId const bucket_flat_id_1,
                                 FlatBucketId const bucket_flat_id_2,
                                 FlatBucketId const bucket_flat_id_3,
                                 Buckets const& buckets,
                                 BucketToHand const& bucket_to_hand,
                                 int const modifier) {
    auto const [bucket_id_1, bucket_cnt_id_1] =
        toBucketIdBucketCntId(bucket_flat_id_1);
    auto const [bucket_id_2, bucket_cnt_id_2] =
        toBucketIdBucketCntId(bucket_flat_id_2);
    auto const [bucket_id_3, bucket_cnt_id_3] =
        toBucketIdBucketCntId(bucket_flat_id_3);

    int const hand_id_1 = bucket_to_hand[bucket_id_1][bucket_cnt_id_1];
    int const hand_id_2 = bucket_to_hand[bucket_id_2][bucket_cnt_id_2];
    int const hand_id_3 = bucket_to_hand[bucket_id_3][bucket_cnt_id_3];

    double const delta_trigrams = [&]() {
      double delta_trigrams = 0;
      for (char const key_1 : buckets[bucket_id_1][bucket_cnt_id_1])
        for (char const key_2 : buckets[bucket_id_2][bucket_cnt_id_2])
          for (char const key_3 : buckets[bucket_id_3][bucket_cnt_id_3])
            delta_trigrams += corpus_stats.trigrams[key_1][key_2][key_3];

      return delta_trigrams * modifier;
    }();

    // alternates
    if (hand_id_1 != hand_id_2 && hand_id_1 == hand_id_3) {
      alternates += delta_trigrams;
    } else {
      inverse_alternates += delta_trigrams;
      score += delta_trigrams * WEIGHT_ALTERNATES;
    }

    // rolls
    bool const is_roll_2_1 = hand_id_1 == hand_id_2 && hand_id_2 != hand_id_3 &&
                             bucket_id_1 != bucket_id_2;
    bool const is_roll_1_2 = hand_id_2 == hand_id_3 && hand_id_1 != hand_id_2 &&
                             bucket_id_2 != bucket_id_3;
    if (is_roll_1_2 || is_roll_2_1) {
      rolls += delta_trigrams;
    } else {
      inverse_rolls += delta_trigrams;
      score += delta_trigrams * WEIGHT_ROLLS;
    }

    // redirect
    bool const is_one_hand_trigram =
        hand_id_1 == hand_id_2 && hand_id_2 == hand_id_3;
    bool const is_direction_change =
        (bucket_id_1 < bucket_id_2 && bucket_id_2 > bucket_id_3) ||
        (bucket_id_1 > bucket_id_2 && bucket_id_2 < bucket_id_3);
    if (is_one_hand_trigram && is_direction_change) {
      redirects += delta_trigrams;
      score += delta_trigrams * WEIGHT_REDIRECT;
    }
  }

  inline void updateStatsShuffle(Buckets const& buckets,
                                 BucketToHand const& bucket_to_hand,
                                 int const bucket_id, int const bucket_cnt_id,
                                 int const hand_id, int const modifier) {
    int const num_assigned_buckets =
        transform_reduce(bucket_to_hand.begin(), bucket_to_hand.end(), 0,
                         plus<int>(), [&](vector<int> const& hand_assignment) {
                           return hand_assignment.size();
                         });
    int const current_bucket_flat_id = toFlatBucketId(bucket_id, bucket_cnt_id);
    for (int i = 0; i < num_assigned_buckets; ++i) {
      for (int j = 0; j < num_assigned_buckets; ++j) {
        if (i != current_bucket_flat_id && j != current_bucket_flat_id) {
          int k = current_bucket_flat_id;
          updateTrigramStats(i, j, k, buckets, bucket_to_hand, modifier);
          continue;
        }

        for (int k = 0; k < num_assigned_buckets; ++k) {
          updateTrigramStats(i, j, k, buckets, bucket_to_hand, modifier);
        }
      }
    }
  }
};

struct PartialLayout {
  Buckets buckets;

  LayoutStats aggregated_stats;
  array<LayoutStats, NUM_STATS> stats_per_language;

  BucketToHand bucket_to_hand = {};

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

  inline void updateStatsShuffle(int const bucket_id, int const bucket_cnt_id,
                                 int const hand_id, int const modifier) {
    aggregated_stats.updateStatsShuffle(buckets, bucket_to_hand, bucket_id,
                                        bucket_cnt_id, hand_id, modifier);
    for (auto& stats : stats_per_language)
      stats.updateStatsShuffle(buckets, bucket_to_hand, bucket_id,
                               bucket_cnt_id, hand_id, modifier);
  }

  inline void assignBucketToHand(int const bucket_id, int const bucket_cnt_id,
                                 int const hand_id) {
    bucket_to_hand[bucket_id].push_back(hand_id);
    updateStatsShuffle(bucket_id, bucket_cnt_id, hand_id, 1);
  }

  inline void unassignBucketToHand(int const bucket_id, int const bucket_cnt_id,
                                   int const hand_id) {
    updateStatsShuffle(bucket_id, bucket_cnt_id, hand_id, -1);
    bucket_to_hand[bucket_id].pop_back();
  }
};

PartialLayout partial_layout, best_partial_layout;

// Bruteforce metadata
int recursion_depth = 0;
long long num_iterations = 0, num_done = 0, num_shuffle_done = 0;

inline void doneShuffle() {
  assert(best_partial_layout.aggregated_stats.score >=
         partial_layout.aggregated_stats.score);
  best_partial_layout = partial_layout;
  ++num_shuffle_done;
}

inline bool isShufflePrunable() {
  ++num_iterations;
  LayoutStats const& aggregated_layout_stats = partial_layout.aggregated_stats;

  if (aggregated_layout_stats.score >
      best_partial_layout.aggregated_stats.score)
    return true;

  for (int i = 0; i < NUM_STATS; ++i) {
    auto const& stats = partial_layout.stats_per_language[i];
    if (stats.inverse_alternates > 1.0 - THRESHOLD_ALTERNATES_PER_LANG[i] ||
        stats.inverse_rolls > 1.0 - THRESHOLD_ROLLS_PER_LANG[i] ||
        stats.redirects > THRESHOLD_REDIRECTS_PER_LANG[i])
      return true;
  }

  return false;
}

inline void assignBucketToHand(int const bucket_id, int const hand_id) {
  partial_layout.assignBucketToHand(bucket_id, 0, hand_id);
  partial_layout.assignBucketToHand(bucket_id, 1, hand_id ^ 1);
}

inline void unassignBucketToHand(int const bucket_id, int const hand_id) {
  partial_layout.unassignBucketToHand(bucket_id, 1, hand_id ^ 1);
  partial_layout.unassignBucketToHand(bucket_id, 0, hand_id);
}

void shuffle(int bucket_id) {
  if (isShufflePrunable()) {
    return;
  }

  if (bucket_id == NUM_BUCKET) {
    doneShuffle();
    return;
  }

  for (int hand_id = 0; hand_id < NUM_HANDS; ++hand_id) {
    assignBucketToHand(bucket_id, hand_id);
    shuffle(bucket_id + 1);
    unassignBucketToHand(bucket_id, hand_id);
  }
}

inline void done() {
  assert(best_partial_layout.aggregated_stats.score >=
         partial_layout.aggregated_stats.score);
  shuffle(0);
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
    for (int stats_i = 0; stats_i < NUM_STATS; ++stats_i) {
      auto const& stats = partial_layout.stats_per_language[stats_i];
      for (int i = 0; i < NUM_BUCKET; ++i)
        for (size_t j = 0; j < partial_layout.buckets[i].size(); ++j)
          if (stats.finger_usage[i][j] > THRESHOLD_FINGER_USAGE[stats_i][i])
            return true;
    }
  }

  if (ENABLE_SFB_PER_FINGER_THRESHOLD) {
    for (int stats_i = 0; stats_i < NUM_STATS; ++stats_i) {
      auto const& stats = partial_layout.stats_per_language[stats_i];
      for (int i = 0; i < NUM_BUCKET; ++i)
        for (size_t j = 0; j < partial_layout.buckets[i].size(); ++j)
          if (stats.sfb_bucket[i][j] > THRESHOLD_SFB_PER_FINGER[stats_i][i])
            return true;
    }
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
    if (partial_layout.buckets[i].size() == NUM_HANDS) continue;
    printProgressAddBucket(i, key);
    pushBucket(i, key);
    brute(key_id + 1);
    popBucket(i, key);
  }

  for (int i = 0; i < NUM_BUCKET; i++) {
    for (size_t j = 0; j < partial_layout.buckets[i].size(); j++) {
      if (partial_layout.buckets[i][j].size() == BUCKET_CAP[i]) continue;
      printProgressAddKey(i, j, key);
      pushKey(i, j, key);
      brute(key_id + 1);
      popKey(i, j, key);
    }
  }
}

void printBuckets(Buckets const& buckets, BucketToHand const& bucket_to_hand) {
  printf("Bucket:\n");

  Buckets layout = buckets;
  for (int i = 0; i < NUM_BUCKET; i++) {
    assert(layout[i].size() == NUM_HANDS &&
           bucket_to_hand[i].size() == NUM_HANDS);
    if (bucket_to_hand[i][0]) swap(layout[i][0], layout[i][1]);
  }

  for (int i = 0; i < NUM_BUCKET; ++i) {
    printf("%s ", string(layout[i][0].begin(), layout[i][0].end()).c_str());
  }
  printf(" ");
  for (int i = NUM_BUCKET - 1; i >= 0; --i) {
    printf("%s ", string(layout[i][1].begin(), layout[i][1].end()).c_str());
  }
  printf("\n");
}

void printBucketStats(LayoutStats const& layout_stats) {
  printf("SFB: %2.3lf | SFS: %2.3lf\n", layout_stats.sfb * 100,
         layout_stats.sfs * 100);

  printf("Finger Usage:\n");
  for (int i = 0; i < NUM_BUCKET; ++i) {
    printf("   ");
    for (int j = 0; j < NUM_HANDS; j++)
      printf("%2.3lf, ", layout_stats.finger_usage[i][j] * 100);
    printf("\n");
  }

  printf("SFB per bucket Usage:\n");
  for (int i = 0; i < NUM_BUCKET; ++i) {
    printf("   ");
    for (int j = 0; j < NUM_HANDS; j++)
      printf("%2.3lf, ", layout_stats.sfb_bucket[i][j] * 100);
    printf("\n");
  }

  printf("Total alternates: %2.3lf\n", layout_stats.alternates * 100);

  printf("Total rolls: %2.3lf\n", layout_stats.rolls * 100);

  printf("Total redirect: %2.3lf\n", layout_stats.redirects * 100);
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
  cerr << "Num shuffle done: " << num_shuffle_done << endl;

  if (num_shuffle_done == 0) return 0;

  printBuckets(best_partial_layout.buckets, best_partial_layout.bucket_to_hand);
  for (int i = 0; i < NUM_STATS; i++) {
    cout << "==== Stats " << STATS_FNAME[i] << endl;
    printBucketStats(best_partial_layout.stats_per_language[i]);
    cout << endl;
  }
}