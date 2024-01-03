#include "search.h"

#include <fstream>
#include <iostream>

#include "core/keyset_config_transformator.h"

constexpr size_t kMaxPrintProgressRecursionDepth = 3;

std::string getIndentationPrefix(size_t const& level,
                                 char const& prefix_char = ' ') {
  return std::string(level * 2, prefix_char);
}

WideString bucketToWideString(Bucket const& bucket,
                              FastReadCorpusStats const& corpus_stats) {
  WideString bucket_str;
  for (KeyId const& key_id : bucket) {
    bucket_str += corpus_stats.getCharFromId(key_id);
  }
  return bucket_str;
}

LayoutWithMetric search(KeysetConfig const& keyset_config,
                        std::vector<BucketSpec> const& bucket_specs,
                        RawCorpusStats const& raw_corpus_stats,
                        Threshold const& threshold,
                        std::string const& best_result_filename) {
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
         best_result, metadata, best_result_filename);

  std::cerr << "Iteration number: " << metadata.num_iteration << std::endl;
  std::cerr << "Peak best results size: " << metadata.peak_best_results_size
            << std::endl;

  auto raw_result = best_result.getAllData();
  LayoutWithMetric result;
  for (auto [sfb, sfs, buckets] : raw_result) {
    static_vector<std::string> layout_str;
    for (auto const& bucket : buckets) {
      WideString bucket_str =
          bucketToWideString(bucket, fast_read_corpus_stats);
      layout_str.push_back(unicode::toNarrow(bucket_str));
    }
    result.push_back({sfb, sfs, layout_str});
  }
  return result;
}

void printLayoutInKeyIds(static_vector<Bucket> const& layout,
                         FastReadCorpusStats const& corpus_stats) {
  for (Bucket const& bucket : layout) {
    for (KeyId const& key_id : bucket) {
      std::cout << " " << key_id;
    }
    std::cout << " |";
  }
  std::cout << std::endl;
}

void printLayoutInString(std::ostream& stream,
                         static_vector<Bucket> const& layout,
                         FastReadCorpusStats const& corpus_stats) {
  for (Bucket const& bucket : layout) {
    std::string bucket_str =
        unicode::toNarrow(bucketToWideString(bucket, corpus_stats));
    stream << " " << bucket_str;
  }
  stream << std::endl;
}

void printCheckpoint(SearchState const& state,
                     FastReadCorpusStats const& corpus_stats,
                     SearchStats const& search_stats,
                     BestBucketMetricSet& best_result,
                     SearchMetadata const& metadata,
                     std::string const& best_result_filename) {
  auto calcSfb = [&](long long sfb) {
    return (double)sfb / corpus_stats.getTotalBigrams();
  };
  auto calcSfs = [&](long long sfs) {
    return (double)sfs / corpus_stats.getTotalBigrams();
  };

  std::cout << "Found new layout. Printing info and checkpoint: " << std::endl;

  std::cout << "[Metadata]:" << std::endl;
  std::cout << "Num solutions found: " << metadata.num_solutions_found
            << ", Num iterations: " << metadata.num_iteration
            << ", Best results size: " << best_result.size()
            << ", Peak best results size: " << metadata.peak_best_results_size
            << std::endl;

  std::cout << "== Pruning count from best result by depth: ";
  for (size_t i = 0; i < state.getRecursionDepth(); ++i) {
    std::cout << " " << metadata.depth_to_prune_best_result_count[i];
  }
  std::cout << std::endl;

  std::cout << "== Pruning count from threshold by depth: ";
  for (size_t i = 0; i < state.getRecursionDepth(); ++i) {
    std::cout << " " << metadata.depth_to_prune_threshold_count[i];
  }
  std::cout << std::endl;

  std::ofstream best_result_f(best_result_filename);
  best_result_f << "[Best results]:" << std::endl;
  best_result_f << "Keyset order: "
                << unicode::toNarrow(corpus_stats.getKeyset()) << std::endl;
  auto raw_result = best_result.getAllData();
  for (auto [sfb, sfs, layout] : raw_result) {
    best_result_f << "SFB: " << calcSfb(sfb) << ", SFS: " << calcSfs(sfs)
                  << ", Layout: ";
    printLayoutInString(best_result_f, layout, corpus_stats);
  }

  std::cout << "[Checkpoint]:" << std::endl;
  auto const& layout = state.getAllBuckets();
  std::cout << "SFB: " << calcSfb(search_stats.sfb) << ", "
            << calcSfs(search_stats.sfs) << std::endl;

  std::cout << "Buckets [Key Ids]:";
  printLayoutInKeyIds(layout, corpus_stats);
  std::cout << "Buckets [Chars]:";
  printLayoutInString(std::cout, layout, corpus_stats);

  std::cout << std::endl;
}

void search(SearchState& state, FastReadCorpusStats const& corpus_stats,
            ThresholdFreq const& threshold, SearchStats const& search_stats,
            BestBucketMetricSet& best_result, SearchMetadata& metadata,
            std::string const& best_result_filename) {
  ++metadata.num_iteration;

  // pruning
  if (search_stats.sfb > threshold.sfb || search_stats.sfs > threshold.sfs) {
    ++metadata.depth_to_prune_threshold_count[state.getRecursionDepth()];
    return;
  }

  if (best_result.isExistsBetterMetricThan(search_stats.sfb,
                                           search_stats.sfs)) {
    ++metadata.depth_to_prune_best_result_count[state.getRecursionDepth()];
    return;
  }

  // recursion
  switch (state.getPhase()) {
    case SearchState::Phase::ADD_BUCKET: {
      // TODO: experiment with generator to do this loop
      KeyId const first_unused_key = state.getFirstUnusedKey();
      for (size_t bucket_spec_id = 0;
           bucket_spec_id < state.bucket_specs_.size(); ++bucket_spec_id) {
        if (state.isBucketGroupFull(bucket_spec_id)) continue;

        // Print progress
        if (state.getRecursionDepth() < kMaxPrintProgressRecursionDepth) {
          std::cerr << getIndentationPrefix(state.getRecursionDepth())
                    << "[ADD_BUCKET] size: "
                    << state.bucket_specs_[bucket_spec_id].capacity
                    << ", char: "
                    << unicode::toNarrow(WideString(
                           {corpus_stats.getCharFromId(first_unused_key)}))
                    << std::endl;
        }

        state.addNewBucket(bucket_spec_id);
        state.addKeyToCurrentBucket(first_unused_key);
        search(state, corpus_stats, threshold, search_stats, best_result,
               metadata, best_result_filename);
        state.undoAddKeyToCurrentBucket();
        state.undoAddNewBucket();
      }
      break;
    }
    case SearchState::Phase::ADD_KEY: {
      KeyId const latest_key = state.getCurrentBucket().back();
      size_t const remaining_bucket_capacity =
          state.getLatestBucketRemainingCapacity();
      auto unused_keys = state.getUnusedKeysAfterPos(latest_key);

      for (size_t i = 0; i + remaining_bucket_capacity <= unused_keys.size();
           ++i) {
        KeyId const& key = unused_keys[i];

        // Print progress
        if (state.getRecursionDepth() < kMaxPrintProgressRecursionDepth) {
          std::cerr << getIndentationPrefix(state.getRecursionDepth())
                    << "[ADD_KEY] key: "
                    << unicode::toNarrow(
                           WideString({corpus_stats.getCharFromId(key)}))
                    << std::endl;
        }

        SearchStats new_stats = search_stats;
        for (auto const other_key : state.getCurrentBucket()) {
          new_stats.sfb += corpus_stats.getBigramFreq(other_key, key) +
                           corpus_stats.getBigramFreq(key, other_key);
          new_stats.sfs += corpus_stats.getSkipgramFreq(other_key, key) +
                           corpus_stats.getSkipgramFreq(key, other_key);
        }
        state.addKeyToCurrentBucket(key);
        search(state, corpus_stats, threshold, new_stats, best_result, metadata,
               best_result_filename);
        state.undoAddKeyToCurrentBucket();
      }
      break;
    }
    case SearchState::Phase::END: {
      best_result.insert(search_stats.sfb, search_stats.sfs,
                         state.getAllBuckets());
      metadata.peak_best_results_size =
          std::max(metadata.peak_best_results_size, best_result.size());
      ++metadata.num_solutions_found;
      printCheckpoint(state, corpus_stats, search_stats, best_result, metadata,
                      best_result_filename);
      break;
    }
  }
}