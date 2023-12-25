#pragma once

#include <unordered_set>
#include <vector>

#include "src/types.h"

using Bucket = std::vector<char>;

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

  SearchState(std::vector<char> const& keyset,
              std::vector<BucketSpec> const& bucket_specs);

  void addKeyToCurrentBucket(char key);

  void undoAddKeyToCurrentBucket();

  void addNewBucket(int bucketSpecId);

  void undoAddNewBucket();

  // Action helper
  Phase getPhase() const;

  std::vector<int> getUnusedBucketSpecIds() const;

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
  std::vector<int> numBucketsAddedBySpec_;
  std::vector<std::pair<int, int>> activeBucketIdStack_;

  std::unordered_set<char> unused_keys_;
};