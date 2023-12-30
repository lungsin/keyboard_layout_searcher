#include "search.h"

#include <iostream>

#include "core/keyset_config_transformator.h"

LayoutWithMetric search(KeysetConfig const& keyset_config,
                        std::vector<BucketSpec> const& bucket_specs,
                        RawCorpusStats const& raw_corpus_stats,
                        Threshold const& threshold) {
  for (auto x : bucket_specs) std::cerr << x.capacity << std::endl;

  KeysetConfigTransformator<RawCorpusStats> transformator;
  FastReadCorpusStats fast_read_corpus_stats(
      transformator.apply_config(keyset_config).get());

  SearchState state(fast_read_corpus_stats.getKeysetSize(), bucket_specs);

  ThresholdFreq threshold_freq(
      threshold.sfb * fast_read_corpus_stats.getTotalBigrams(),
      threshold.sfs * fast_read_corpus_stats.getTotalSkipgrams());

  BestBucketMetricSet best_result;
  SearchMetadata metadata;
  search(state, fast_read_corpus_stats, threshold_freq, SearchStats({0LL, 0LL}),
         best_result, metadata);

  std::cerr << "Iteration number: " << metadata.num_iteration << std::endl;
  return best_result.getAllData();
}

void search(SearchState& state, FastReadCorpusStats const& corpus_stats,
            ThresholdFreq const& threshold, SearchStats const& search_stats,
            BestBucketMetricSet& best_result, SearchMetadata& metadata) {
  ++metadata.num_iteration;

  // pruning
  if (search_stats.sfb > threshold.sfb || search_stats.sfs > threshold.sfs) {
    std::cerr << "threshold prune" << std::endl;
    return;
  }

  if (best_result.isExistsBetterMetricThan(search_stats.sfb,
                                           search_stats.sfs)) {
    std::cerr << "here" << std::endl;
    return;
  }

  // recursion
  switch (state.getPhase()) {
    case SearchState::Phase::ADD_BUCKET: {
      // TODO: experiment with generator to do this loop
      for (size_t bucket_spec_id = 0;
           bucket_spec_id < state.bucket_specs_.size(); ++bucket_spec_id)
        if (state.isBucketGroupFull(bucket_spec_id)) {
          state.addNewBucket(bucket_spec_id);
          search(state, corpus_stats, threshold, search_stats, best_result,
                 metadata);
          state.undoAddNewBucket();
        }
      break;
    }
    case SearchState::Phase::ADD_KEY: {
      auto unused_keys = state.getUnusedKeys();

      for (auto const& key : unused_keys) {
        SearchStats new_stats = search_stats;
        for (auto const other_key : state.getCurrentBucket()) {
          new_stats.sfb += corpus_stats.getBigramFreq(other_key, key);
          new_stats.sfs += corpus_stats.getBigramFreq(other_key, key);
        }
        state.addKeyToCurrentBucket(key);
        search(state, corpus_stats, threshold, new_stats, best_result,
               metadata);
        state.undoAddKeyToCurrentBucket();
      }
      break;
    }
    case SearchState::Phase::END: {
      std::cerr << "END" << std::endl;
      best_result.insert(search_stats.sfb, search_stats.sfs,
                         state.getAllBuckets());
      break;
    }
  }
}