#include "search.h"

void search(Keyset const& keyset, std::vector<BucketSpec> const& bucket_specs,
            RawCorpusStats raw_corpus_stats, Threshold const& threshold) {
  // TODO: sort keyset by occurance
  SearchState state(keyset, bucket_specs);
  CorpusStats corpus_stats(keyset, raw_corpus_stats);
  ThresholdOcc threshold_occ(threshold.sfb * corpus_stats.total_bigrams_,
                             threshold.sfs * corpus_stats.total_skipgrams_);
  BestPairSet<long long, long long, std::vector<Bucket>> best_result;
  SearchMetadata metadata;
  search(state, corpus_stats, threshold_occ, SearchStats(0LL, 0LL), best_result,
         metadata);
}

void search(SearchState& state, CorpusStats const& corpus_stats,
            ThresholdOcc const& threshold, SearchStats const& search_stats,
            BestPairSet<long long, long long, std::vector<Bucket>>& best_result,
            SearchMetadata& metadata) {
  metadata.num_iteration++;

  // pruning
  if (search_stats.sfb > threshold.sfb || search_stats.sfs > threshold.sfs)
    return;

  if (best_result.isExistsBetterMetricThan(search_stats.sfb, search_stats.sfs))
    return;

  // recursion
  switch (state.getPhase()) {
    case SearchState::Phase::ADD_BUCKET: {
      for (auto const& bucket_spec_id : state.getUnusedBucketSpecIds()) {
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
          new_stats.sfb += corpus_stats.getBigramOccurance(other_key, key);
          new_stats.sfs += corpus_stats.getSkipgramOccurance(other_key, key);
        }
        state.addKeyToCurrentBucket(key);
        search(state, corpus_stats, threshold, new_stats, best_result,
               metadata);
        state.undoAddKeyToCurrentBucket();
      }
      break;
    }
    case SearchState::Phase::END: {
      best_result.insert(search_stats.sfb, search_stats.sfs,
                         state.getAllBuckets());
      break;
    }
  }
}