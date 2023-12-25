#include "search_state.h"

#include <functional>
#include <ranges>

Bucket newBucketWithCapacity(int capacity) {
  Bucket b;
  b.reserve(capacity);
  return b;
}

SearchState::SearchState(std::vector<char> const& keyset,
                         std::vector<BucketSpec> const& bucket_specs)
    : bucket_specs_(bucket_specs),
      keyset_(keyset),
      bucketsBySpec_(bucket_specs.size()),
      numBucketsAddedBySpec_(bucket_specs.size(), 0) {
  int totalBuckets = 0;
  for (int i = 0; i < (int)bucket_specs.size(); i++) {
    const auto bucketSpec = bucket_specs[i];
    totalBuckets += bucketSpec.count;
    auto empty_bucket = Bucket();

    bucketsBySpec_[i].resize(bucketSpec.count,
                             newBucketWithCapacity(bucketSpec.capacity));
  }

  activeBucketIdStack_.reserve(totalBuckets);
}

void SearchState::addKeyToCurrentBucket(char key) {
  activeBucketRefStack_.back().get().push_back(key);
  unused_keys_.erase(key);
}

void SearchState::undoAddKeyToCurrentBucket() {
  auto& current_bucket = activeBucketRefStack_.back().get();
  unused_keys_.insert(current_bucket.back());
  current_bucket.pop_back();
}

void SearchState::addNewBucket(int bucketSpecId) {
  const int bucketId = numBucketsAddedBySpec_[bucketSpecId];
  numBucketsAddedBySpec_[bucketSpecId]++;
  activeBucketIdStack_.emplace_back(bucketSpecId, bucketId);
  activeBucketRefStack_.push_back(
      std::ref(bucketsBySpec_[bucketSpecId][bucketId]));
}

void SearchState::undoAddNewBucket() {
  const auto [bucketSpecId, bucketId] = activeBucketIdStack_.back();
  bucketsBySpec_[bucketSpecId][bucketId].clear();
  numBucketsAddedBySpec_[bucketSpecId]--;
  activeBucketIdStack_.pop_back();
  activeBucketRefStack_.pop_back();
}

Bucket const& SearchState::getCurrentBucket() const {
  return activeBucketRefStack_.back();
}

std::vector<Bucket> SearchState::getAllBuckets() const {
  return std::vector<Bucket>(activeBucketRefStack_.begin(),
                             activeBucketRefStack_.end());
}

SearchState::Phase SearchState::getPhase() const {
  if (activeBucketRefStack_.empty()) return Phase::ADD_BUCKET;
  auto const& current_bucket = getCurrentBucket();
  if (current_bucket.size() == current_bucket.capacity())
    return Phase::ADD_BUCKET;
  return Phase::ADD_KEY;
}

std::vector<int> SearchState::getUnusedBucketSpecIds() const {
  auto r = std::views::iota(0, (int)bucket_specs_.size()) |
           std::views::filter([&](int i) {
             return numBucketsAddedBySpec_[i] < bucket_specs_[i].count;
           });
  return std::vector<int>(r.begin(), r.end());
}

Keyset SearchState::getUnusedKeys() const {
  return Keyset(unused_keys_.begin(), unused_keys_.end());
}