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
constexpr int NUM_FINGERS = NUM_BUCKET * NUM_HANDS;

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

const string RECURVA_PATH = "static/kb/recurva.kb";

constexpr int NUM_ROWS = 3;
constexpr int TOP_ROW = 0;
constexpr int MIDDLE_ROW = 1;
constexpr int BOTTOM_ROW = 2;

// Thresholds and weights config
constexpr double PERCENT = 0.01;
using ThresholdPerLang = array<double, NUM_STATS>;
using ThresholdFingerUsage = array<array<double, NUM_BUCKET>, NUM_STATS>;

constexpr double AGGREGATED_THRESHOLD_SFB = 1 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_SFB_PER_LANG = {0.0090, 0.0095, 0.0100};
constexpr double AGGREGATED_THRESHOLD_SFS = 8.5 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_SFS_PER_LANG = {0.065, 0.075, 0.085};

constexpr double AGGREGATED_THRESHOLD_SFB_2U = 0.2 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_SFB_2U_PER_LANG = {
    0.2 * PERCENT, 0.2 * PERCENT, 0.2 * PERCENT};
constexpr double AGGREGATED_THRESHOLD_SFS_2U = 1 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_SFS_2U_PER_LANG = {
    0.5 * PERCENT, 0.5 * PERCENT, 1.5 * PERCENT};

constexpr bool ENABLE_FINGER_THRESHOLD = true;
constexpr ThresholdFingerUsage THRESHOLD_FINGER_USAGE = {{
    {0.12, 0.14, 0.25, 0.30},
    {0.14, 0.15, 0.25, 0.30},
    {0.14, 0.15, 0.25, 0.30},
}};

constexpr bool ENABLE_SFB_PER_FINGER_THRESHOLD = true;
constexpr ThresholdFingerUsage THRESHOLD_SFB_PER_FINGER = {{
    {0.001, 0.02, 0.10, 0.10},
    {0.1, 0.1, 0.1, 0.1},
    {0.1, 0.1, 0.1, 0.1},
}};

constexpr ThresholdPerLang THRESHOLD_ALTERNATES_PER_LANG = {0.30, 0.30, 0.30};
constexpr ThresholdPerLang THRESHOLD_ROLLS_PER_LANG = {0.30, 0.30, 0.30};
constexpr ThresholdPerLang THRESHOLD_REDIRECTS_PER_LANG = {0.08, 0.15, 0.15};

constexpr double WEIGHT_SFB = 1;
constexpr double WEIGHT_SFS = 1;
constexpr double WEIGHT_SFB_2U = 1;
constexpr double WEIGHT_SFS_2U = 1;

constexpr double WEIGHT_FINGER_USAGE_OVERALL = 0.01 * 0;
constexpr array<double, NUM_BUCKET> WEIGHT_FINGER_USAGE = {4, 3, 2, 1};

constexpr double WEIGHT_SFB_PER_FINGER_OVERALL = 1 * 0;
constexpr array<double, NUM_BUCKET> WEIGHT_SFB_PER_FINGER = {4, 3, 2, 1};

constexpr double WEIGHT_ALTERNATES = 0;
constexpr double WEIGHT_ROLLS = 0;
constexpr double WEIGHT_REDIRECT = 0;

using Bucket = vector<char>;
// [bucket_id][bucket_cnt_id] => string
using Buckets = array<vector<Bucket>, NUM_BUCKET>;
using BucketToHand = array<vector<int>, NUM_BUCKET>;
using RowToBucketKeyId = array<vector<int>, NUM_ROWS>;
using BucketRowAssignment =
    array<array<RowToBucketKeyId, NUM_HANDS>, NUM_BUCKET>;
using FlatBucketId = int;

using FastStats = CompactStats<MAX_ASCII>;

Buckets readKb(filesystem::path path) {
  Buckets result = {{{{}, {}}, {{}, {}}, {{}, {}}, {{}, {}}}};
  ifstream in(path);
  for (int r = 0; r < NUM_ROWS; r++) {
    for (int i = 0; i < NUM_BUCKET; i++) {
      int offset = BUCKET_CAP[i] / NUM_ROWS;
      for (int j = 0; j < offset; j++) {
        char key;
        in >> key;
        result[i][0].push_back(key);
      }
    }

    for (int i = NUM_BUCKET - 1; i >= 0; i--) {
      int offset = BUCKET_CAP[i] / NUM_ROWS;
      for (int j = 0; j < offset; j++) {
        char key;
        in >> key;
        result[i][1].push_back(key);
      }
    }
  }
  return result;
}

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

// Sum of trigrams stats for keys in buckets, i.e.
// value[bucket_1][bucket_2][bucket_3] =
//    sum {key_1: bucket_1, key_2: bucket_2, key_3: bucket_3}
//      trigram[key_1][key_2][key_3]
using TrigramBucketStats =
    array<array<array<double, NUM_FINGERS>, NUM_FINGERS>, NUM_FINGERS>;

struct LayoutStats {
  double sfb = 0, sfs = 0;

  array<array<double, NUM_HANDS>, NUM_BUCKET> sfb_bucket = {{}},
                                              finger_usage = {{}};

  double sfb_2u = 0, sfs_2u = 0;

  // trigram stats
  double alternates = 0, inverse_alternates = 0;
  double rolls = 0, inverse_rolls = 0;
  double redirects = 0;

  // weighted score
  double score = 0;

  FastStats corpus_stats;

  TrigramBucketStats trigram_bucket_stats;

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

  inline void precomputeTrigramBucketStats(Buckets const& buckets) {
    for (int bucket_flat_id_1 = 0; bucket_flat_id_1 < NUM_FINGERS;
         ++bucket_flat_id_1) {
      for (int bucket_flat_id_2 = 0; bucket_flat_id_2 < NUM_FINGERS;
           ++bucket_flat_id_2) {
        for (int bucket_flat_id_3 = 0; bucket_flat_id_3 < NUM_FINGERS;
             ++bucket_flat_id_3) {
          double& stats =
              trigram_bucket_stats[bucket_flat_id_1][bucket_flat_id_2]
                                  [bucket_flat_id_3];
          auto const [bucket_id_1, bucket_cnt_id_1] =
              toBucketIdBucketCntId(bucket_flat_id_1);
          auto const [bucket_id_2, bucket_cnt_id_2] =
              toBucketIdBucketCntId(bucket_flat_id_2);
          auto const [bucket_id_3, bucket_cnt_id_3] =
              toBucketIdBucketCntId(bucket_flat_id_3);

          // calculate trigram sum for bucket combination
          stats = 0;

          for (char const key_1 : buckets[bucket_id_1][bucket_cnt_id_1])
            for (char const key_2 : buckets[bucket_id_2][bucket_cnt_id_2])
              for (char const key_3 : buckets[bucket_id_3][bucket_cnt_id_3])
                stats += corpus_stats.trigrams[key_1][key_2][key_3];
        }
      }
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

    double const delta_trigrams =
        trigram_bucket_stats[bucket_flat_id_1][bucket_flat_id_2]
                            [bucket_flat_id_3] *
        modifier;

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
    int const num_assigned_buckets = [&]() {
      int total_size = 0;
      for (auto const& hand_assignment : bucket_to_hand)
        total_size += hand_assignment.size();
      return total_size;
    }();

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

  inline void updateStatsBucketRowAssignment(
      Bucket const& bucket, RowToBucketKeyId const& row_to_bucket_key_id,
      int const modifier) {
    for (auto const top_key_id : row_to_bucket_key_id[TOP_ROW]) {
      char const top_key = bucket[top_key_id];
      for (auto const bottom_key_id : row_to_bucket_key_id[BOTTOM_ROW]) {
        char const bottom_key = bucket[bottom_key_id];
        double const delta_sfb_2u =
            corpus_stats.bigrams[top_key][bottom_key] * modifier;
        double const delta_sfs_2u =
            corpus_stats.skipgrams[top_key][bottom_key] * modifier;

        sfb_2u += delta_sfb_2u;
        sfs_2u += delta_sfs_2u;

        score += delta_sfb_2u * WEIGHT_SFB_2U;
        score += delta_sfs_2u * WEIGHT_SFS_2U;
      }
    }
  }
};

struct PartialLayout {
  Buckets buckets;

  LayoutStats aggregated_stats;
  array<LayoutStats, NUM_STATS> stats_per_language;

  BucketToHand bucket_to_hand = {};

  BucketRowAssignment bucket_row_assignment = {};

  PartialLayout() = default;

  inline void setStats(FastStats const& aggregated_stats,
                       array<FastStats, NUM_STATS> const& stats_list) {
    this->aggregated_stats = aggregated_stats;
    for (int i = 0; i < NUM_STATS; i++) {
      this->stats_per_language[i] = stats_list[i];
    }
  }

  inline void setLayout(Buckets const& layout) {
    for (int bucket_id = 0; bucket_id < NUM_BUCKET; ++bucket_id) {
      for (int bucket_cnt_id = 0; bucket_cnt_id < NUM_HANDS; ++bucket_cnt_id) {
        auto const& bucket = layout[bucket_id][bucket_cnt_id];
        pushBucket(bucket_id, *bucket.begin());
        for (int i = 1; i < bucket.size(); ++i) {
          pushKey(bucket_id, bucket_cnt_id, bucket[i]);
        }
      }
    }

    for (int bucket_id = 0; bucket_id < NUM_BUCKET; ++bucket_id) {
      for (int bucket_cnt_id = 0; bucket_cnt_id < NUM_HANDS; ++bucket_cnt_id) {
        assignBucketToHand(bucket_id, bucket_cnt_id, bucket_cnt_id);
      }
    }

    for (int bucket_id = 0; bucket_id < NUM_BUCKET; ++bucket_id) {
      for (int bucket_cnt_id = 0; bucket_cnt_id < NUM_HANDS; ++bucket_cnt_id) {
        auto const& bucket = layout[bucket_id][bucket_cnt_id];
        RowToBucketKeyId row_to_bucket_id;
        for (int i = 0; i < bucket.size(); i++) {
          row_to_bucket_id[i / (bucket.size() / NUM_ROWS)].push_back(i);
        }
        setBucketRowPermut(bucket_id, bucket_cnt_id, row_to_bucket_id);
      }
    }
  }

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

  inline void precomputeTrigramBucketStats() {
    aggregated_stats.precomputeTrigramBucketStats(buckets);
    for (auto& stats : stats_per_language)
      stats.precomputeTrigramBucketStats(buckets);
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

  inline void updateStatsBucketRowPermut(int const bucket_id,
                                         int const bucket_cnt_id,
                                         int const modifier) {
    aggregated_stats.updateStatsBucketRowAssignment(
        buckets[bucket_id][bucket_cnt_id],
        bucket_row_assignment[bucket_id][bucket_cnt_id], modifier);
    for (auto& stats : stats_per_language)
      stats.updateStatsBucketRowAssignment(
          buckets[bucket_id][bucket_cnt_id],
          bucket_row_assignment[bucket_id][bucket_cnt_id], modifier);
  }

  inline void setBucketRowPermut(int const bucket_id, int const bucket_cnt_id,
                                 RowToBucketKeyId const& row_to_bucket_id) {
    bucket_row_assignment[bucket_id][bucket_cnt_id] = row_to_bucket_id;
    updateStatsBucketRowPermut(bucket_id, bucket_cnt_id, 1);
  }

  inline void unsetBucketRowPermut(int const bucket_id, int const bucket_cnt_id,
                                   RowToBucketKeyId const& row_to_bucket_id) {
    updateStatsBucketRowPermut(bucket_id, bucket_cnt_id, -1);
  }
};

PartialLayout partial_layout, best_partial_layout;

// Bruteforce metadata
int recursion_depth = 0;
long long num_iterations = 0;
long long num_done = 0, num_bucket_row_assignment_done = 0,
          num_shuffle_done = 0;

inline void doneShuffle();
void shuffleBucketToHand(int bucket_id);
inline void doneBruteBucketRowAssignment();
void bruteRowKeysInBucket(int const bucket_id, int const bucket_cnt_id);

inline void doneShuffle() {
  assert(best_partial_layout.aggregated_stats.score >=
         partial_layout.aggregated_stats.score);

  // DONE ORDERING
  bruteRowKeysInBucket(0, 0);
  // best_partial_layout = partial_layout;
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

void shuffleBucketToHand(int bucket_id) {
  if (isShufflePrunable()) {
    return;
  }

  if (bucket_id == NUM_BUCKET) {
    doneShuffle();
    return;
  }

  for (int hand_id = 0; hand_id < NUM_HANDS; ++hand_id) {
    assignBucketToHand(bucket_id, hand_id);
    shuffleBucketToHand(bucket_id + 1);
    unassignBucketToHand(bucket_id, hand_id);
  }
}

inline bool isBruteBucketRowAssignmentPruneable() {
  ++num_iterations;
  LayoutStats const& aggregated_layout_stats = partial_layout.aggregated_stats;

  if (aggregated_layout_stats.score >
      best_partial_layout.aggregated_stats.score)
    return true;

  if (aggregated_layout_stats.sfb_2u > AGGREGATED_THRESHOLD_SFB_2U ||
      aggregated_layout_stats.sfs_2u > AGGREGATED_THRESHOLD_SFS_2U)
    return true;

  for (int i = 0; i < NUM_STATS; ++i) {
    auto const& stats = partial_layout.stats_per_language[i];
    if (stats.sfb_2u > THRESHOLD_SFB_2U_PER_LANG[i] ||
        stats.sfs_2u > THRESHOLD_SFS_2U_PER_LANG[i])
      return true;
  }

  return false;
}

inline void doneBruteBucketRowAssignment() {
  assert(best_partial_layout.aggregated_stats.score >=
         partial_layout.aggregated_stats.score);
  // DONE ORDERING
  best_partial_layout = partial_layout;
  // shuffleBucketToHand(0);
  ++num_bucket_row_assignment_done;
}

inline void setBucketRowPermut(int const bucket_id, int const bucket_cnt_id,
                               RowToBucketKeyId const& row_to_bucket_id) {
  partial_layout.setBucketRowPermut(bucket_id, bucket_cnt_id, row_to_bucket_id);
}

inline void unsetBucketRowPermut(int const bucket_id, int const bucket_cnt_id,
                                 RowToBucketKeyId const& row_to_bucket_id) {
  partial_layout.unsetBucketRowPermut(bucket_id, bucket_cnt_id,
                                      row_to_bucket_id);
}

template <int num_keys, int num_results>
constexpr array<array<int, num_keys>, num_results> getRowPermutationFor2U() {
  array<int, num_keys> permut;
  for (int i = 0; i < num_keys; i++) {
    permut[i] = i / (num_keys / NUM_ROWS);
  }

  auto isOk = [](array<int, num_keys> const& permut) {
    for (int elem : permut) {
      if (elem == MIDDLE_ROW)
        continue;
      else if (elem == TOP_ROW)
        return true;
      else if (elem == BOTTOM_ROW)
        return false;
    }
    assert(false);
  };

  array<array<int, num_keys>, num_results> results;
  int i = 0;
  do {
    if (isOk(permut)) {
      assert(i < num_results);
      results[i++] = permut;
    }
  } while (next_permutation(permut.begin(), permut.end()));
  return results;
}

const array<array<int, 3>, 3> ROW_PERMUT_FOR_2U_3_KEYS =
    getRowPermutationFor2U<3, 3>();
const array<array<int, 6>, 45> ROW_PERMUT_FOR_2U_6_KEYS =
    getRowPermutationFor2U<6, 45>();

void bruteRowKeysInBucket(int const bucket_id, int const bucket_cnt_id) {
  if (isBruteBucketRowAssignmentPruneable()) {
    return;
  }

  if (bucket_id == NUM_BUCKET) {
    doneBruteBucketRowAssignment();
    return;
  }

  if (bucket_cnt_id == NUM_HANDS) {
    bruteRowKeysInBucket(bucket_id + 1, 0);
    return;
  }

  auto bruteNext = [&](vector<int> const& permut) {
    RowToBucketKeyId row_to_bucket_key_id;
    for (int bucket_key_id = 0; bucket_key_id < permut.size();
         ++bucket_key_id) {
      row_to_bucket_key_id[permut[bucket_key_id]].push_back(bucket_key_id);
    }
    setBucketRowPermut(bucket_id, bucket_cnt_id, row_to_bucket_key_id);
    bruteRowKeysInBucket(bucket_id, bucket_cnt_id + 1);
    unsetBucketRowPermut(bucket_id, bucket_cnt_id, row_to_bucket_key_id);
  };

  auto const& buckets = partial_layout.buckets[bucket_id][bucket_cnt_id];
  if (buckets.size() == 3) {
    for (auto const& permut : ROW_PERMUT_FOR_2U_3_KEYS) {
      bruteNext(vector<int>(permut.begin(), permut.end()));
    }
  } else if (buckets.size() == 6) {
    for (auto const& permut : ROW_PERMUT_FOR_2U_6_KEYS) {
      bruteNext(vector<int>(permut.begin(), permut.end()));
    }
  } else {
    // Not supported
    assert(false);
  }
}

inline void done() {
  assert(best_partial_layout.aggregated_stats.score >=
         partial_layout.aggregated_stats.score);
  // DONE ORDERING
  partial_layout.precomputeTrigramBucketStats();
  // bruteRowKeysInBucket(0, 0);
  shuffleBucketToHand(0);
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

void printBuckets(Buckets const& buckets, BucketToHand const& bucket_to_hand,
                  BucketRowAssignment const& bucket_row_assignment) {
  printf("==========================\n");
  printf("Bucket:\n");

  Buckets layout = buckets;
  // row, hand, bucket
  for (int i = 0; i < NUM_BUCKET; ++i) {
    for (int j = 0; j < NUM_HANDS; ++j) {
      auto const& row_to_bucket_key_id = bucket_row_assignment[i][j];
      Bucket const& bucket = buckets[i][j];
      Bucket result;
      for (int row_id = 0; row_id < NUM_ROWS; ++row_id) {
        for (auto const bucket_key_id : row_to_bucket_key_id[row_id])
          result.push_back(bucket[bucket_key_id]);
      }
      layout[i][j] = result;
      assert(result.size() == buckets[i][j].size());
      assert(result.size() == BUCKET_CAP[i]);
    }
  }

  for (int i = 0; i < NUM_BUCKET; i++) {
    assert(layout[i].size() == NUM_HANDS &&
           bucket_to_hand[i].size() == NUM_HANDS);
    if (bucket_to_hand[i][0]) swap(layout[i][0], layout[i][1]);
  }

  for (int r = 0; r < NUM_ROWS; r++) {
    for (int i = 0; i < NUM_BUCKET; ++i) {
      int const offset = layout[i][0].size() / NUM_ROWS;
      for (int j = 0; j < offset; j++) {
        printf("%c ", layout[i][0][r * offset + j]);
      }
    }
    printf(" ");
    for (int i = NUM_BUCKET - 1; i >= 0; --i) {
      int const offset = layout[i][1].size() / NUM_ROWS;
      for (int j = 0; j < offset; j++) {
        printf("%c ", layout[i][1][r * offset + j]);
      }
    }
    printf("\n");
  }

  printf("\n");
}

void printBucketStats(LayoutStats const& layout_stats) {
  printf("SFB: %2.3lf | SFS: %2.3lf\n", layout_stats.sfb * 100,
         layout_stats.sfs * 100);

  printf("SFB (2U): %2.3lf | SFS (2U): %2.3lf\n", layout_stats.sfb_2u * 100,
         layout_stats.sfs_2u * 100);

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

  partial_layout.setStats(aggregated_stats, stats_list);
  best_partial_layout.setStats(aggregated_stats, stats_list);

  best_partial_layout.setLayout(readKb(RECURVA_PATH));
  cout << "Baseline layout stats from " << RECURVA_PATH << endl;
  printBuckets(best_partial_layout.buckets, best_partial_layout.bucket_to_hand,
               best_partial_layout.bucket_row_assignment);
  cout << "Score: " << best_partial_layout.aggregated_stats.score << endl;
  for (int i = 0; i < NUM_STATS; i++) {
    cout << "==== Stats " << STATS_FNAME[i] << endl;
    printBucketStats(best_partial_layout.stats_per_language[i]);
    cout << endl;
  }
  cout << endl;

  brute(0);

  cerr << "==== Metadata ====\nNum Iterations: " << num_iterations << endl;
  cerr << "Num done: " << num_done << endl;
  cerr << "Num row assign done: " << num_bucket_row_assignment_done << endl;
  cerr << "Num shuffle done: " << num_shuffle_done << endl;
  cerr << "Best score: " << best_partial_layout.aggregated_stats.score << endl;

  if (num_shuffle_done == 0) return 0;

  printBuckets(best_partial_layout.buckets, best_partial_layout.bucket_to_hand,
               best_partial_layout.bucket_row_assignment);
  for (int i = 0; i < NUM_STATS; i++) {
    cout << "==== Stats " << STATS_FNAME[i] << endl;
    printBucketStats(best_partial_layout.stats_per_language[i]);
    cout << endl;
  }
}