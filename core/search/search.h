#pragma once

#include "core/corpus_stats.h"
#include "core/data_structure/best_pair_set.h"
#include "search_state.h"

struct Threshold {
  double sfb, sfs;
};

struct ThresholdOcc {
  long long sfb, sfs;
};

struct SearchStats {
  long long sfb, sfs;
};

struct SearchMetadata {
  int num_iteration;
};

using LayoutWithMetric =
    std::vector<std::tuple<long long, long long, std::vector<Bucket>>>;

LayoutWithMetric search(Keyset const& keyset,
                        std::vector<BucketSpec> const& bucket_specs,
                        RawCorpusStats const& raw_corpus_stats,
                        Threshold const& threshold);

void search(SearchState& state, CorpusStats const& corpus_stats,
            ThresholdOcc const& threshold, SearchStats const& search_stats,
            BestPairSet<long long, long long, std::vector<Bucket>>& best_result,
            SearchMetadata& metadata);