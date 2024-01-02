#pragma once

#include "core/data_structure/best_pair_set.h"
#include "core/keyset_config.h"
#include "core/stats/fast_read_corpus_stats.h"
#include "search_state.h"

struct Threshold {
  double sfb, sfs;
};

struct ThresholdFreq {
  long long sfb, sfs;
};

struct SearchStats {
  long long sfb, sfs;
};

struct SearchMetadata {
  size_t num_iteration = 0;
  size_t peak_best_results_size = 0;
};

using LayoutWithMetric =
    std::vector<std::tuple<long long, long long, static_vector<std::string>>>;

using BestBucketMetricSet =
    BestPairSet<long long, long long, static_vector<Bucket>>;

LayoutWithMetric search(KeysetConfig const& keyset_config,
                        std::vector<BucketSpec> const& bucket_specs,
                        RawCorpusStats const& raw_corpus_stats,
                        Threshold const& threshold);

void search(SearchState& state, FastReadCorpusStats const& corpus_stats,
            ThresholdFreq const& threshold, SearchStats const& search_stats,
            BestBucketMetricSet& best_result, SearchMetadata& metadata);