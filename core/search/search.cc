#include "search.h"

#include <iostream>

#include "core/keyset_config_transformator.h"

LayoutWithMetric search(KeysetConfig const& keyset_config,
                        std::vector<BucketSpec> const& bucket_specs,
                        RawCorpusStats const& raw_corpus_stats,
                        Threshold const& threshold) {
  std::cerr << "Filter corpus stats based on keyset config" << std::endl;

  RawCorpusStats keyset_corpus_stats = raw_corpus_stats;
  apply_config(keyset_config, keyset_corpus_stats);
  std::cerr << "Get filtered read corpus stats" << std::endl;
  keyset_corpus_stats.save_as_json("/home/klungs/git/klungsyzer/tmp.json");

  std::cerr << "Constructing fast read corpus stats" << std::endl;
  FastReadCorpusStats fast_read_corpus_stats(keyset_corpus_stats,
                                             keyset_config.keyset);

  std::cerr << "Sorted keyset: "
            << unicode::toNarrow(fast_read_corpus_stats.getKeyset())
            << std::endl;
  SearchState state(fast_read_corpus_stats.getKeysetSize(), bucket_specs);

  ThresholdFreq threshold_freq(
      threshold.sfb * fast_read_corpus_stats.getTotalBigrams(),
      threshold.sfs * fast_read_corpus_stats.getTotalSkipgrams());

  BestBucketMetricSet best_result;
  SearchMetadata metadata;
  search(state, fast_read_corpus_stats, threshold_freq, SearchStats({0LL, 0LL}),
         best_result, metadata);

  std::cerr << "Iteration number: " << metadata.num_iteration << std::endl;
  std::cerr << "Peak best results size: " << metadata.peak_best_results_size
            << std::endl;

  auto raw_result = best_result.getAllData();
  LayoutWithMetric result;
  for (auto [sfb, sfs, buckets] : raw_result) {
    static_vector<std::string> layout_str;
    for (auto const& bucket : buckets) {
      WideString bucket_str;
      for (KeyId const& key_id : bucket) {
        bucket_str += fast_read_corpus_stats.getCharFromId(key_id);
      }
      layout_str.push_back(unicode::toNarrow(bucket_str));
    }
    result.push_back({sfb, sfs, layout_str});
  }
  return result;
}

void search(SearchState& state, FastReadCorpusStats const& corpus_stats,
            ThresholdFreq const& threshold, SearchStats const& search_stats,
            BestBucketMetricSet& best_result, SearchMetadata& metadata) {
  ++metadata.num_iteration;

  // pruning
  if (search_stats.sfb > threshold.sfb || search_stats.sfs > threshold.sfs) {
    return;
  }

  if (best_result.isExistsBetterMetricThan(search_stats.sfb,
                                           search_stats.sfs)) {
    return;
  }

  if (metadata.num_iteration % 100000000 == 0) {
    std::cerr << "Num iteration: " << metadata.num_iteration << std::endl;
    std::cerr << "[Search Stats   ] Sfb: " << search_stats.sfb
              << ", SFS: " << search_stats.sfs << std::endl;
    std::cerr << "[Threshold Stats] Sfb: " << threshold.sfb
              << ", SFS: " << threshold.sfs << std::endl;
  }

  // recursion
  switch (state.getPhase()) {
    case SearchState::Phase::ADD_BUCKET: {
      // TODO: experiment with generator to do this loop
      for (size_t bucket_spec_id = 0;
           bucket_spec_id < state.bucket_specs_.size(); ++bucket_spec_id)
        if (!state.isBucketGroupFull(bucket_spec_id)) {
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
          new_stats.sfs += corpus_stats.getSkipgramFreq(other_key, key);
        }
        state.addKeyToCurrentBucket(key);
        search(state, corpus_stats, threshold, new_stats, best_result,
               metadata);
        state.undoAddKeyToCurrentBucket();
      }
      break;
    }
    case SearchState::Phase::END: {
      std::cout << "Found new layout. Num iterations: "
                << metadata.num_iteration << std::endl;
      best_result.insert(search_stats.sfb, search_stats.sfs,
                         state.getAllBuckets());
      metadata.peak_best_results_size =
          std::max(metadata.peak_best_results_size, best_result.size());
      break;
    }
  }
}