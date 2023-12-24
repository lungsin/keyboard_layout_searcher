#include "search_state.h"

Bucket newBucketWithCapacity(int capacity) {
  Bucket b;
  b.reserve(capacity);
  return b;
}

RecursionState::RecursionState(const std::vector<BucketSpec>& bucketSpecs)
    : bucketSpecs(bucketSpecs),
      bucketsBySpec(bucketSpecs.size()),
      numBucketsAddedBySpec(bucketSpecs.size(), 0) {
  int totalBuckets = 0;
  for (int i = 0; i < (int)bucketSpecs.size(); i++) {
    const auto bucketSpec = bucketSpecs[i];
    totalBuckets += bucketSpec.count;
    auto empty_bucket = Bucket();

    bucketsBySpec[i].resize(bucketSpec.count,
                            newBucketWithCapacity(bucketSpec.capacity));
  }

  activeBucketIdStack.reserve(totalBuckets);
}

void RecursionState::addKeyToCurrentBucket(char key) {
  getCurrentBucket().push_back(key);
}

void RecursionState::undoAddKeyToCurrentBucket() {
  getCurrentBucket().pop_back();
}

void RecursionState::addNewBucket(int bucketSpecId) {
  const int bucketId = numBucketsAddedBySpec[bucketSpecId];
  numBucketsAddedBySpec[bucketSpecId]++;
  activeBucketIdStack.emplace_back(bucketSpecId, bucketId);
}

void RecursionState::undoAddNewBucket() {
  const auto [bucketSpecId, bucketId] = activeBucketIdStack.back();
  bucketsBySpec[bucketSpecId][bucketId].clear();
  numBucketsAddedBySpec[bucketSpecId]--;
  activeBucketIdStack.pop_back();
}

Bucket& RecursionState::getCurrentBucket() {
  const auto [bucketSpecId, bucketId] = activeBucketIdStack.back();
  return bucketsBySpec[bucketSpecId][bucketId];
}