#pragma once

#include <vector>

using Bucket = std::vector<char>;

Bucket newBucketWithCapacity(int capacity);

struct BucketSpec {
  int capacity, count;
};

class RecursionState {
  const std::vector<BucketSpec> bucketSpecs;

  std::vector<std::vector<Bucket>> bucketsBySpec;
  std::vector<int> numBucketsAddedBySpec;
  std::vector<std::pair<int, int>> activeBucketIdStack;

 public:
  RecursionState(const std::vector<BucketSpec>& bucketSpecs);

  void addKeyToCurrentBucket(char key);

  void undoAddKeyToCurrentBucket();

  void addNewBucket(int bucketSpecId);

  void undoAddNewBucket();

 private:
  Bucket& getCurrentBucket();
};