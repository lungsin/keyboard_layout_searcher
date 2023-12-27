#pragma once

#include <unordered_set>
#include <vector>

#include "core/types.h"

using Bucket = std::vector<CorpusChar>;
using BucketSpecCountId = int;
using BucketSpecId = int;

struct BucketSpec {
  int capacity, count;
};

class SearchState {
 public:
  enum Phase {
    ADD_BUCKET,
    ADD_KEY,
    END,
  };

  SearchState(Keyset const& keyset,
              std::vector<BucketSpec> const& bucket_specs);

  void addKeyToCurrentBucket(CorpusChar key);

  void undoAddKeyToCurrentBucket();

  void addNewBucket(int bucketSpecId);

  void undoAddNewBucket();

  // Action helper
  Phase getPhase() const;

  std::vector<BucketSpecId> getUnusedBucketSpecIds() const;

  Keyset getUnusedKeys() const;

  // Helper
  Bucket const& getCurrentBucket() const;

  std::vector<Bucket> getAllBuckets() const;

 private:
  Bucket& getCurrentBucketInternal();

  const std::vector<BucketSpec> bucket_specs_;
  const Keyset keyset_;
  size_t target_total_buckets_;

  std::vector<std::vector<Bucket>> bucketsBySpec_;
  std::vector<BucketSpecId> numBucketsAddedBySpec_;
  std::vector<std::pair<BucketSpecId, BucketSpecCountId>> activeBucketIdStack_;

  std::unordered_set<CorpusChar> unused_keys_;
};