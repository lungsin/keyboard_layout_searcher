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
// finger type => bucket_id
enum FingerType {
  PINKY_FINGER = 0,
  MIDDLE_FINGER = 1,
  RING_FINGER = 2,
  INDEX_FINGER = 3,
};

enum HandType {
  LEFT_HAND = 0,
  RIGHT_HAND = 1,
};

const int NUM_STATS = 3;
const string STATS_FNAME[NUM_STATS] = {
    // "static/stats/shai/shai.json",

    // "static/oxeylyzer_stats/english.json",
    // "static/oxeylyzer_stats/450k.json",
    // "static/oxeylyzer_stats/indonesian.json",

    "static/playground_stats/english.json",
    "static/playground_stats/450k.json",
    "static/playground_stats/indonesian.json",
};
const double STATS_WEIGHT[NUM_STATS] = {
    1,
    1,
    1,
};

const string RECURVA_PATH = "static/kb/recurva.kb";
const string RECURVA_STATS_PATH = "result/recurva.txt";

const string MAYA_PATH = "static/kb/maya.kb";
const string MAYA_STATS_PATH = "result/maya.txt";

constexpr int NUM_ROWS = 3;
enum RowType { TOP_ROW = 0, MIDDLE_ROW = 1, BOTTOM_ROW = 2 };
constexpr int NUM_KEYS_PER_ROW[NUM_BUCKET] = {1, 1, 1, 2};

// Thresholds and weights config
constexpr double PERCENT = 0.01;
using ThresholdPerLang = array<double, NUM_STATS>;
using ThresholdFingerUsage = array<array<double, NUM_BUCKET>, NUM_STATS>;

constexpr double AGGREGATED_THRESHOLD_SFB = 1 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_SFB_PER_LANG = {0.9 * PERCENT, 1 * PERCENT,
                                                     1 * PERCENT};
constexpr double AGGREGATED_THRESHOLD_SFS = 8.5 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_SFS_PER_LANG = {0.065, 0.075, 0.085};

constexpr double AGGREGATED_THRESHOLD_SFB_2U = 0.1 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_SFB_2U_PER_LANG = {
    0.1 * PERCENT, 0.1 * PERCENT, 0.1 * PERCENT};
constexpr double AGGREGATED_THRESHOLD_SFS_2U = 1.5 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_SFS_2U_PER_LANG = {
    0.5 * PERCENT, 0.5 * PERCENT, 0.5 * PERCENT};

// Scissors
constexpr bool ENABLE_SCISSORS_THRESHOLD = true;
constexpr double AGGREGATED_THRESHOLD_HSB = 6 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_HSB_PER_LANG = {
    5 * PERCENT,
    5 * PERCENT,
    6 * PERCENT,
};
constexpr double AGGREGATED_THRESHOLD_HSS = 12 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_HSS_PER_LANG = {
    8 * PERCENT,
    8 * PERCENT,
    12 * PERCENT,
};

constexpr double AGGREGATED_THRESHOLD_FSB = 0.6 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_FSB_PER_LANG = {
    0.3 * PERCENT,
    0.3 * PERCENT,
    0.6 * PERCENT,
};
constexpr double AGGREGATED_THRESHOLD_FSS = 1.5 * PERCENT;
constexpr ThresholdPerLang THRESHOLD_FSS_PER_LANG = {
    0.3 * PERCENT,
    0.3 * PERCENT,
    1.5 * PERCENT,
};

// Finger-wise threshold
constexpr bool ENABLE_FINGER_THRESHOLD = true;
constexpr ThresholdFingerUsage THRESHOLD_FINGER_USAGE = {{
    {0.12, 0.16, 0.25, 0.30},
    {0.14, 0.16, 0.25, 0.30},
    {0.14, 0.22, 0.25, 0.30},
}};

constexpr bool ENABLE_SFB_PER_FINGER_THRESHOLD = true;
constexpr ThresholdFingerUsage THRESHOLD_SFB_PER_FINGER = {{
    {0.15 * PERCENT, 1 * PERCENT, 0.10, 0.10},
    {0.1, 0.1, 0.1, 0.1},
    {0.15 * PERCENT, 0.1, 0.1, 0.1},
}};

// Trigram threshold
constexpr ThresholdPerLang THRESHOLD_ALTERNATES_PER_LANG = {0.30, 0.30, 0.30};
constexpr ThresholdPerLang THRESHOLD_ROLLS_PER_LANG = {0.30, 0.30, 0.30};
constexpr ThresholdPerLang THRESHOLD_REDIRECTS_PER_LANG = {0.08, 0.10, 0.10};

constexpr double WEIGHT_SFB = 1 * 0;
constexpr double WEIGHT_SFS = 1 * 0;
constexpr double WEIGHT_SFB_2U = 1;
constexpr double WEIGHT_SFS_2U = 1;

// Scissors
constexpr double WEIGHT_HSB = 0.05;
constexpr double WEIGHT_HSS = 0.05;
constexpr double WEIGHT_FSB = 1;
constexpr double WEIGHT_FSS = 1;

constexpr double WEIGHT_FINGER_USAGE_OVERALL = 0.01 * 0;
constexpr array<double, NUM_BUCKET> WEIGHT_FINGER_USAGE = {4, 3, 2, 1};

constexpr double WEIGHT_SFB_PER_FINGER_OVERALL = 1 * 0;
constexpr array<double, NUM_BUCKET> WEIGHT_SFB_PER_FINGER = {4, 3, 2, 1};

constexpr double WEIGHT_ALTERNATES = 0;
constexpr double WEIGHT_ROLLS = 0;
constexpr double WEIGHT_REDIRECT = 0;

// Quality of life restrictions

// Sometimes the result of this searcher puts the vowels at right hand.
// Since a lot of layouts prefer to put vowels at right hand, this config is
// here so that the letter E is at right hand.
constexpr bool MUST_PUT_E_AT_RIGHT_HAND = true;

constexpr bool MUST_PUT_SHORTCUT_KEYS_AT_LEFT_HAND = false;
constexpr string SHORTCUT_KEYS = "zxcvws";

using Bucket = vector<char>;
// [bucket_id][bucket_cnt_id] => string
using Buckets = array<vector<Bucket>, NUM_BUCKET>;
using BucketToHand = array<vector<int>, NUM_BUCKET>;
using RowToBucketKeyId = array<vector<int>, NUM_ROWS>;
using BucketRowAssignment =
    array<array<RowToBucketKeyId, NUM_HANDS>, NUM_BUCKET>;
using FlatBucketId = int;

using Row = vector<char>;
using Rows = array<Row, NUM_ROWS>;
using Layout = array<array<Rows, NUM_HANDS>, NUM_BUCKET>;

inline Row getRowFromBucket(Bucket const& bucket, RowType row_type) {
  int const num_keys_per_row = bucket.size() / NUM_ROWS;
  int const offset = num_keys_per_row * row_type;
  return Row(bucket.begin() + offset,
             bucket.begin() + offset + num_keys_per_row);
}

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
  // bigram stats
  double sfb = 0, sfs = 0;

  array<array<double, NUM_HANDS>, NUM_BUCKET> sfb_bucket = {{}},
                                              finger_usage = {{}};

  double sfb_2u = 0, sfs_2u = 0;

  // Half & full scisors stats
  double hsb = 0, fsb = 0;
  double hss = 0, fss = 0;

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

  inline void update2UStats(Layout const& layout, int const bucket_id,
                            int const hand_id, int const modifier) {
    auto const& rows = layout[bucket_id][hand_id];

    for (char const top_key : rows[RowType::TOP_ROW]) {
      for (char const bottom_key : rows[RowType::BOTTOM_ROW]) {
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

  inline void updateScissorsStats(Layout const& layout, int const bucket_id,
                                  int const hand_id, int const modifier) {
    auto const& rows = layout[bucket_id][hand_id];

    auto const& top_row = rows[RowType::TOP_ROW];
    auto const& middle_row = rows[RowType::MIDDLE_ROW];
    auto const& bottom_row = rows[RowType::BOTTOM_ROW];

    // Update scissors
    auto updateHalfScissors = [&](Row const& row1, Row const& row2) {
      for (char const key1 : row1)
        for (char const key2 : row2) {
          double const delta_hsb = corpus_stats.bigrams[key1][key2] * modifier;
          double const delta_hss =
              corpus_stats.skipgrams[key1][key2] * modifier;

          hsb += delta_hsb;
          hss += delta_hss;
          score += delta_hsb * WEIGHT_HSB + delta_hss * WEIGHT_HSS;
        }
    };

    auto updateFullScissors = [&](Row const& row1, Row const& row2) {
      for (char const key1 : row1)
        for (char const key2 : row2) {
          double const delta_fsb = corpus_stats.bigrams[key1][key2] * modifier;
          double const delta_fss =
              corpus_stats.skipgrams[key1][key2] * modifier;

          fsb += delta_fsb;
          fss += delta_fss;
          score += delta_fsb * WEIGHT_FSB + delta_fss * WEIGHT_FSS;
        }
    };

    bool const is_current_finger_middle_or_ring =
        bucket_id == MIDDLE_FINGER || bucket_id == RING_FINGER;
    bool const is_current_finger_middle_or_ring_or_pinky =
        is_current_finger_middle_or_ring || bucket_id == PINKY_FINGER;
    for (int other_bucket_id = 0; other_bucket_id < bucket_id;
         ++other_bucket_id) {
      bool const is_other_finger_middle_or_ring =
          other_bucket_id == MIDDLE_FINGER || other_bucket_id == RING_FINGER;
      if (!is_current_finger_middle_or_ring && !is_other_finger_middle_or_ring)
        continue;

      auto const& other_rows = layout[other_bucket_id][hand_id];
      auto const& other_top_row = other_rows[RowType::TOP_ROW];
      auto const& other_middle_row = other_rows[RowType::MIDDLE_ROW];
      auto const& other_bottom_row = other_rows[RowType::BOTTOM_ROW];

      // [Half scissors] current finger hits the lower row
      if (is_current_finger_middle_or_ring) {
        updateHalfScissors(bottom_row, other_middle_row);
        updateHalfScissors(middle_row, other_top_row);
      }

      // [Half scissors] Other finger hits the lower row
      if (is_other_finger_middle_or_ring) {
        updateHalfScissors(other_bottom_row, middle_row);
        updateHalfScissors(other_middle_row, top_row);
      }

      bool const is_other_finger_middle_or_ring_or_pinky =
          is_other_finger_middle_or_ring || other_bucket_id == PINKY_FINGER;
      // [Full scissors] current finger hits the lower row
      if (is_current_finger_middle_or_ring_or_pinky) {
        updateFullScissors(bottom_row, other_top_row);
      }

      // [Full scissors] Other finger hits the lower row
      if (is_other_finger_middle_or_ring_or_pinky) {
        updateFullScissors(other_bottom_row, top_row);
      }
    }
  }
};

struct PartialLayout {
  Buckets buckets;

  LayoutStats aggregated_stats;
  array<LayoutStats, NUM_STATS> stats_per_language;

  BucketToHand bucket_to_hand = {};
  array<optional<int>, MAX_ASCII> letter_to_hand = {};

  BucketRowAssignment bucket_row_assignment = {};

  Layout final_layout;

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

    precomputeTrigramBucketStats();

    for (int bucket_id = 0; bucket_id < NUM_BUCKET; ++bucket_id)
      for (int bucket_cnt_id = 0; bucket_cnt_id < NUM_HANDS; ++bucket_cnt_id)
        assignBucketToHand(bucket_id, bucket_cnt_id, bucket_cnt_id);

    for (int bucket_id = 0; bucket_id < NUM_BUCKET; ++bucket_id) {
      for (int bucket_cnt_id = 0; bucket_cnt_id < NUM_HANDS; ++bucket_cnt_id) {
        vector<int> identity_permut(BUCKET_CAP[bucket_id]);
        iota(identity_permut.begin(), identity_permut.end(), 0);
        setRowPermutFor2U(bucket_id, bucket_cnt_id, identity_permut);
      }
    }

    for (int bucket_id = 0; bucket_id < NUM_BUCKET; ++bucket_id)
      for (int hand_id = 0; hand_id < NUM_HANDS; ++hand_id)
        setSwapTopBottomRow(bucket_id, hand_id, false);
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
    for (char const key : buckets[bucket_id][bucket_cnt_id])
      letter_to_hand[key] = hand_id;
    updateStatsShuffle(bucket_id, bucket_cnt_id, hand_id, 1);
  }

  inline void unassignBucketToHand(int const bucket_id, int const bucket_cnt_id,
                                   int const hand_id) {
    updateStatsShuffle(bucket_id, bucket_cnt_id, hand_id, -1);
    bucket_to_hand[bucket_id].pop_back();
    for (char const key : buckets[bucket_id][bucket_cnt_id])
      letter_to_hand[key] = nullopt;
  }

  inline void update2UStats(int const bucket_id, int const hand_id,
                            int const modifier) {
    aggregated_stats.update2UStats(final_layout, bucket_id, hand_id, modifier);
    for (auto& stats : stats_per_language)
      stats.update2UStats(final_layout, bucket_id, hand_id, modifier);
  }

  inline void setRowPermutFor2U(int const bucket_id, int const bucket_cnt_id,
                                vector<int> const& keys_permut) {
    int const hand_id = bucket_to_hand[bucket_id][bucket_cnt_id];
    auto const& bucket = buckets[bucket_id][bucket_cnt_id];

    auto& finger_stack = final_layout[bucket_id][hand_id];
    for (int i = 0; i < NUM_ROWS; ++i) finger_stack[i].clear();

    int const num_keys_per_row = NUM_KEYS_PER_ROW[bucket_id];
    for (int i = 0; i < bucket.size(); ++i) {
      finger_stack[i / num_keys_per_row].push_back(bucket[keys_permut[i]]);
    }
    update2UStats(bucket_id, hand_id, 1);
  }

  inline void unsetRowPermutFor2U(int const bucket_id, int const bucket_cnt_id,
                                  vector<int> const& finger_keys_permut) {
    int const hand_id = bucket_to_hand[bucket_id][bucket_cnt_id];
    update2UStats(bucket_id, hand_id, -1);
  }

  inline void updateScissorsStats(int const bucket_id, int const hand_id,
                                  int const modifier) {
    aggregated_stats.updateScissorsStats(final_layout, bucket_id, hand_id,
                                         modifier);
    for (auto& stats : stats_per_language)
      stats.updateScissorsStats(final_layout, bucket_id, hand_id, modifier);
  }

  inline void swapTopBottomRow(int const bucket_id, int const hand_id,
                               bool const is_swap) {
    if (!is_swap) return;
    auto& finger_stack = final_layout[bucket_id][hand_id];
    finger_stack[RowType::TOP_ROW].swap(finger_stack[RowType::BOTTOM_ROW]);
  }

  inline void setSwapTopBottomRow(int const bucket_id, int const hand_id,
                                  bool const is_swap) {
    swapTopBottomRow(bucket_id, hand_id, is_swap);
    updateScissorsStats(bucket_id, hand_id, 1);
  }

  inline void unsetSwapTopBottomRow(int const bucket_id, int const hand_id,
                                    bool const is_swap) {
    updateScissorsStats(bucket_id, hand_id, -1);
    swapTopBottomRow(bucket_id, hand_id, is_swap);
  }
};

PartialLayout partial_layout, best_partial_layout;

// Bruteforce metadata
int recursion_depth = 0;
long long num_iterations = 0;
long long num_brute_bucket_done = 0;
long long num_shuffle_done = 0;
long long num_row_permut_2u_done = 0;
long long num_brute_scissors_done = 0;

inline void doneBruteScissors() {
  assert(best_partial_layout.aggregated_stats.score >=
         partial_layout.aggregated_stats.score);
  best_partial_layout = partial_layout;

  if (num_brute_scissors_done % 10 == 0) {
    cerr << "Found new layout. Iter: " << num_brute_scissors_done
         << ",Score: " << best_partial_layout.aggregated_stats.score << endl;
  }
  ++num_brute_scissors_done;
}

inline bool isPruneableBruteScissors() {
  ++num_iterations;
  LayoutStats const& aggregated_layout_stats = partial_layout.aggregated_stats;

  if (aggregated_layout_stats.score >
      best_partial_layout.aggregated_stats.score)
    return true;

  // Scissors
  if (ENABLE_SCISSORS_THRESHOLD) {
    if (aggregated_layout_stats.hsb > AGGREGATED_THRESHOLD_HSB ||
        aggregated_layout_stats.hss > AGGREGATED_THRESHOLD_HSS ||
        aggregated_layout_stats.fsb > AGGREGATED_THRESHOLD_FSB ||
        aggregated_layout_stats.fss > AGGREGATED_THRESHOLD_FSS)
      return true;

    for (int i = 0; i < NUM_STATS; ++i) {
      auto const& stats = partial_layout.stats_per_language[i];
      if (stats.hsb > THRESHOLD_HSB_PER_LANG[i] ||
          stats.hss > THRESHOLD_HSS_PER_LANG[i] ||
          stats.fsb > THRESHOLD_FSB_PER_LANG[i] ||
          stats.fss > THRESHOLD_FSS_PER_LANG[i])
        return true;
    }
  }

  return false;
}

void bruteScissors(int bucket_id, int hand_id) {
  if (isPruneableBruteScissors()) {
    return;
  }

  if (bucket_id == NUM_BUCKET) {
    doneBruteScissors();
    return;
  }

  if (hand_id == NUM_HANDS) {
    bruteScissors(bucket_id + 1, 0);
    return;
  }

  auto bruteNext = [&](bool const& is_swap_top_bottom_row) {
    partial_layout.setSwapTopBottomRow(bucket_id, hand_id,
                                       is_swap_top_bottom_row);
    bruteScissors(bucket_id, hand_id + 1);
    partial_layout.unsetSwapTopBottomRow(bucket_id, hand_id,
                                         is_swap_top_bottom_row);
  };
  bruteNext(false);
  bruteNext(true);
}

inline void doneBruteRowPermutFor2U() {
  assert(best_partial_layout.aggregated_stats.score >=
         partial_layout.aggregated_stats.score);
  bruteScissors(0, 0);
  ++num_row_permut_2u_done;
}

inline bool isPrunableBruteRowPermutFor2U() {
  ++num_iterations;
  LayoutStats const& aggregated_layout_stats = partial_layout.aggregated_stats;

  if (aggregated_layout_stats.score >
      best_partial_layout.aggregated_stats.score)
    return true;

  // SFB & SFS 2U
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

template <int num_keys, int num_results>
constexpr array<array<int, num_keys>, num_results> getRowPermutFor2U() {
  array<int, num_keys> permut;
  int const num_keys_per_row = num_keys / NUM_ROWS;
  for (int i = 0; i < num_keys; i++) {
    permut[i] = i / num_keys_per_row;
  }

  auto isPermut = [](array<int, num_keys> const& arr) {
    array<int, num_keys> arr2 = arr;
    sort(arr2.begin(), arr2.end());
    for (int i = 0; i < arr2.size(); ++i)
      if (arr2[i] != i) return false;
    return true;
  };

  auto rowPermutToKeyPermut = [&](array<int, num_keys> const& row_permut) {
    array<int, NUM_ROWS> freq = {{}};
    array<int, num_keys> result;
    for (int i = 0; i < num_keys; ++i) {
      int const row = row_permut[i];
      result[freq[row] + row * num_keys_per_row] = i;
      ++freq[row];
    }
    assert(isPermut(result));
    return result;
  };

  // Remove top and bottom row symmetry
  auto isTopFirst = [&](array<int, num_keys> const& result) {
    for (int elem : result) {
      if (elem == RowType::MIDDLE_ROW)
        continue;
      else if (elem == RowType::TOP_ROW)
        return true;
      else if (elem == RowType::BOTTOM_ROW)
        return false;
    }
    assert(false);
  };

  array<array<int, num_keys>, num_results> results;
  int i = 0;
  do {
    if (!isTopFirst(permut)) continue;
    assert(i < num_results);
    results[i++] = rowPermutToKeyPermut(permut);
  } while (next_permutation(permut.begin(), permut.end()));
  assert(i == num_results);
  return results;
}

const auto ROW_PERMUT_FOR_2U_3_KEYS = getRowPermutFor2U<3, 3>();
const auto ROW_PERMUT_FOR_2U_6_KEYS = getRowPermutFor2U<6, 45>();

void bruteRowPermutFor2U(int const bucket_id, int const hand_id) {
  if (isPrunableBruteRowPermutFor2U()) {
    return;
  }

  if (bucket_id == NUM_BUCKET) {
    doneBruteRowPermutFor2U();
    return;
  }

  if (hand_id == NUM_HANDS) {
    bruteRowPermutFor2U(bucket_id + 1, 0);
    return;
  }

  auto bruteNext = [&](vector<int> const& permut) {
    partial_layout.setRowPermutFor2U(bucket_id, hand_id, permut);
    bruteRowPermutFor2U(bucket_id, hand_id + 1);
    partial_layout.unsetRowPermutFor2U(bucket_id, hand_id, permut);
  };

  auto const& buckets = partial_layout.buckets[bucket_id][hand_id];
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

// [Stage] Shuffle bucket to hand

inline void doneShuffle() {
  assert(best_partial_layout.aggregated_stats.score >=
         partial_layout.aggregated_stats.score);
  bruteRowPermutFor2U(0, 0);
  ++num_shuffle_done;
}

inline bool isPrunableShuffle() {
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

  if (MUST_PUT_E_AT_RIGHT_HAND) {
    if (partial_layout.letter_to_hand['e'].value_or(-1) == HandType::LEFT_HAND)
      return true;
  }

  if (MUST_PUT_SHORTCUT_KEYS_AT_LEFT_HAND) {
    for (char const key : SHORTCUT_KEYS) {
      if (partial_layout.letter_to_hand[key].value_or(-1) == RIGHT_HAND)
        return true;
    }
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
  if (isPrunableShuffle()) {
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

// [Stage] Brute force bucket

inline void done() {
  assert(best_partial_layout.aggregated_stats.score >=
         partial_layout.aggregated_stats.score);
  partial_layout.precomputeTrigramBucketStats();
  shuffleBucketToHand(0);
  ++num_brute_bucket_done;
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

void printFinalLayout(ostream& out, Layout const& layout) {
  out << "==========================\n";
  out << "Layout:\n";

  for (int r = 0; r < NUM_ROWS; r++) {
    for (int i = 0; i < NUM_BUCKET; ++i)
      for (char const key : layout[i][0][r]) out << key << " ";

    out << " ";
    for (int i = NUM_BUCKET - 1; i >= 0; --i)
      for (char const key : layout[i][1][r]) out << key << " ";

    out << endl;
  }
  out << endl;
}

void printBucketStats(ostream& out, LayoutStats const& layout_stats,
                      BucketToHand const& bucket_to_hand) {
  out << format("SFB: {:2.3f} | SFS: {:2.3f}\n", layout_stats.sfb * 100,
                layout_stats.sfs * 100);
  out << format("SFB (2U): {:2.3f} | SFS (2U): {:2.3f}\n",
                layout_stats.sfb_2u * 100, layout_stats.sfs_2u * 100);

  out << format("HSB: {:2.3f} | HSS: {:2.3f}\n", layout_stats.hsb * 100,
                layout_stats.hss * 100);
  out << format("FSB: {:2.3f} | FSS: {:2.3f}\n", layout_stats.fsb * 100,
                layout_stats.fss * 100);

  out << format("Finger Usage:\n");
  for (int i = 0; i < NUM_BUCKET; ++i) {
    out << format("   ");
    for (int j = 0; j < NUM_HANDS; j++) {
      int const hand_id = bucket_to_hand[i][j];
      out << format("{:2.3f}, ", layout_stats.finger_usage[i][hand_id] * 100);
    }
    out << format("\n");
  }

  out << format("SFB per finger:\n");
  for (int i = 0; i < NUM_BUCKET; ++i) {
    out << format("   ");
    for (int j = 0; j < NUM_HANDS; j++) {
      int const hand_id = bucket_to_hand[i][j];
      out << format("{:2.3f}, ", layout_stats.sfb_bucket[i][hand_id] * 100);
    }
    out << format("\n");
  }

  out << format("Total alternates: {:2.3f}\n", layout_stats.alternates * 100);

  out << format("Total rolls: {:2.3f}\n", layout_stats.rolls * 100);

  out << format("Total redirect: {:2.3f}\n", layout_stats.redirects * 100);
}

void printBaseline(ostream& out, PartialLayout const& layout) {
  printFinalLayout(out, layout.final_layout);
  out << "Score: " << layout.aggregated_stats.score << endl;
  for (int i = 0; i < NUM_STATS; i++) {
    out << "==== Stats " << STATS_FNAME[i] << endl;
    printBucketStats(out, layout.stats_per_language[i], layout.bucket_to_hand);
    out << endl;
  }
  out << endl;
}

void setBaseline(string const& kb_path) {
  best_partial_layout.setLayout(readKb(kb_path));
  best_partial_layout.aggregated_stats.score += 1e-9;
}

void dumpBaseline(string const& out_fname, string const& kb_path,
                  FastStats const& aggregated_stats,
                  array<FastStats, NUM_STATS> const& stats_list) {
  ofstream out(out_fname);
  PartialLayout layout;
  layout.setStats(aggregated_stats, stats_list);
  layout.setLayout(readKb(kb_path));
  printBaseline(out, layout);
}

int main() {
  auto [aggregated_stats, stats_list] = readAllStats();

  partial_layout.setStats(aggregated_stats, stats_list);
  best_partial_layout.setStats(aggregated_stats, stats_list);

  setBaseline(RECURVA_PATH);
  dumpBaseline(toWorkingDirectory(RECURVA_STATS_PATH), RECURVA_PATH,
               aggregated_stats, stats_list);
  dumpBaseline(toWorkingDirectory(MAYA_STATS_PATH), MAYA_PATH, aggregated_stats,
               stats_list);

  brute(0);

  cerr << "==== Metadata ====\nNum Iterations: " << num_iterations << endl;
  cerr << "Num brute buckets done: " << num_brute_bucket_done << endl;
  cerr << "Num shuffle done: " << num_shuffle_done << endl;
  cerr << "Num row 2u assign done: " << num_row_permut_2u_done << endl;
  cerr << "Num brute scissors done: " << num_brute_scissors_done << endl;
  cerr << "Best score: " << best_partial_layout.aggregated_stats.score << endl;

  if (num_shuffle_done == 0) return 0;

  printFinalLayout(cout, best_partial_layout.final_layout);
  for (int i = 0; i < NUM_STATS; i++) {
    cout << "==== Stats " << STATS_FNAME[i] << endl;
    printBucketStats(cout, best_partial_layout.stats_per_language[i],
                     best_partial_layout.bucket_to_hand);
    cout << endl;
  }
}