#include "search_state.h"

#include <ranges>

SearchState::SearchState(Keyset const& keyset,
                         std::vector<BucketSpec> const& bucket_specs)
    : bucket_specs_(bucket_specs),
      keyset_(keyset),
      bucketsBySpec_(bucket_specs.size()),
      numBucketsAddedBySpec_(bucket_specs.size(), 0),
      unused_keys_(keyset.begin(), keyset.end()) {
  target_total_buckets_ = 0;
  for (int i = 0; i < (int)bucket_specs.size(); i++) {
    const auto bucket_spec = bucket_specs[i];
    target_total_buckets_ += bucket_spec.count;
    bucketsBySpec_[i].resize(bucket_spec.count);
    for (auto& bucket : bucketsBySpec_[i]) bucket.reserve(bucket_spec.capacity);
  }

  activeBucketIdStack_.reserve(target_total_buckets_);
}

void SearchState::addKeyToCurrentBucket(CorpusChar key) {
  getCurrentBucketInternal().push_back(key);
  unused_keys_.erase(key);
}

void SearchState::undoAddKeyToCurrentBucket() {
  auto& current_bucket = getCurrentBucketInternal();
  unused_keys_.insert(current_bucket.back());
  current_bucket.pop_back();
}

void SearchState::addNewBucket(int bucketSpecId) {
  const int bucketId = numBucketsAddedBySpec_[bucketSpecId];
  numBucketsAddedBySpec_[bucketSpecId]++;
  activeBucketIdStack_.emplace_back(bucketSpecId, bucketId);
}

void SearchState::undoAddNewBucket() {
  const auto [bucketSpecId, bucketId] = activeBucketIdStack_.back();
  bucketsBySpec_[bucketSpecId][bucketId].clear();
  numBucketsAddedBySpec_[bucketSpecId]--;
  activeBucketIdStack_.pop_back();
}

Bucket const& SearchState::getCurrentBucket() const {
  const auto& [bucket_spec_id, bucket_id] = activeBucketIdStack_.back();
  return bucketsBySpec_[bucket_spec_id][bucket_id];
}

Bucket& SearchState::getCurrentBucketInternal() {
  const auto& [bucket_spec_id, bucket_id] = activeBucketIdStack_.back();
  return bucketsBySpec_[bucket_spec_id][bucket_id];
}

std::vector<Bucket> SearchState::getAllBuckets() const {
  auto r = activeBucketIdStack_ | std::views::transform([&](auto idx) {
             return bucketsBySpec_[idx.first][idx.second];
           });
  return std::vector<Bucket>(r.begin(), r.end());
}

SearchState::Phase SearchState::getPhase() const {
  if (activeBucketIdStack_.empty()) return Phase::ADD_BUCKET;
  const auto& [bucket_spec_id, bucket_id] = activeBucketIdStack_.back();
  auto const& current_bucket = getCurrentBucket();
  auto const& current_bucket_spec = bucket_specs_[bucket_spec_id];
  if ((int)current_bucket.size() < current_bucket_spec.capacity) {
    return Phase::ADD_KEY;
  }
  if (activeBucketIdStack_.size() < target_total_buckets_)
    return Phase::ADD_BUCKET;
  return Phase::END;
}

std::vector<BucketSpecId> SearchState::getUnusedBucketSpecIds() const {
  auto r = std::views::iota(0, (int)bucket_specs_.size()) |
           std::views::filter([&](int i) {
             return numBucketsAddedBySpec_[i] < bucket_specs_[i].count;
           });
  return std::vector<BucketSpecId>(r.begin(), r.end());
}

Keyset SearchState::getUnusedKeys() const {
  return Keyset(unused_keys_.begin(), unused_keys_.end());
}