#include "search_state.h"

#include <numeric>

int getCount(const BucketSpec& bucket_spec) { return bucket_spec.count; }
int getCapacity(const BucketSpec& bucket_spec) { return bucket_spec.capacity; }

int getTargetTotalBuckets(const std::vector<BucketSpec>& bucket_specs) {
  return std::transform_reduce(bucket_specs.begin(), bucket_specs.end(), 0,
                               std::plus{}, getCount);
}

BucketContainer::BucketContainer(const std::vector<BucketSpec>& bucket_specs)
    : bucket_specs_(bucket_specs),
      group_size_(bucket_specs.size(), 0),
      group_capacity_(bucket_specs.size()),
      group_first_flatten_id_(bucket_specs.size()),
      bucket_spec_id_stack_() {
  // init prealloc_buckets_
  for (size_t i = 0; i < bucket_specs.size(); ++i) {
    const auto& bucket_spec = bucket_specs[i];
    for (size_t j = 0; j < bucket_spec.count; ++j) {
      prealloc_buckets_.emplace_back();
      prealloc_buckets_.back().reserve(bucket_spec.capacity);
    }
  }

  // init group_capacity_
  std::transform(bucket_specs.begin(), bucket_specs.end(),
                 group_capacity_.begin(), getCount);

  // init group_first_flatten_id_
  std::exclusive_scan(group_capacity_.begin(), group_capacity_.end(),
                      group_first_flatten_id_.begin(), 0, std::plus{});
}

void BucketContainer::appendBucket(const BucketSpecId& bucket_spec_id) {
  ++group_size_[bucket_spec_id];
  bucket_spec_id_stack_.push_back(bucket_spec_id);
}

void BucketContainer::popBucket() {
  --group_size_[bucket_spec_id_stack_.back()];
  bucket_spec_id_stack_.pop_back();
}

Bucket& BucketContainer::getLatestBucket() {
  return prealloc_buckets_[getLatestFlattenId()];
}

Bucket const& BucketContainer::getLatestBucket() const {
  return prealloc_buckets_[getLatestFlattenId()];
}

bool BucketContainer::empty() const { return bucket_spec_id_stack_.empty(); }

bool BucketContainer::isLatestBucketFull() const {
  return getLatestBucket().size() ==
         bucket_specs_[bucket_spec_id_stack_.back()].capacity;
}

bool BucketContainer::isGroupFull(size_t const& group_id) const {
  return group_size_[group_id] == group_capacity_[group_id];
}

bool BucketContainer::isFull() const {
  return bucket_spec_id_stack_.size() == prealloc_buckets_.size();
}

static_vector<Bucket> BucketContainer::getAllBuckets() const {
  return prealloc_buckets_;
}

BucketContainer::FlattenId BucketContainer::getLatestFlattenId() const {
  const auto& bucket_spec_id = bucket_spec_id_stack_.back();
  return group_first_flatten_id_[bucket_spec_id] + group_size_[bucket_spec_id] -
         1;
}

SearchState::SearchState(const size_t& keyset_size,
                         const std::vector<BucketSpec>& bucket_specs)
    : bucket_specs_(bucket_specs),
      keyset_size_(keyset_size),
      bucket_container_(bucket_specs),
      unused_keys_bitset_(boost::dynamic_bitset<>(keyset_size).flip()) {}

void SearchState::addKeyToCurrentBucket(const size_t& key_id) {
  bucket_container_.getLatestBucket().push_back(key_id);
  unused_keys_bitset_.flip(key_id);
}

void SearchState::undoAddKeyToCurrentBucket() {
  auto& current_bucket = bucket_container_.getLatestBucket();
  unused_keys_bitset_.flip(current_bucket.back());
  current_bucket.pop_back();
}

void SearchState::addNewBucket(const BucketSpecId& bucket_spec_id) {
  bucket_container_.appendBucket(bucket_spec_id);
}

void SearchState::undoAddNewBucket() { bucket_container_.popBucket(); }

Bucket const& SearchState::getCurrentBucket() const {
  return bucket_container_.getLatestBucket();
}

static_vector<Bucket> SearchState::getAllBuckets() const {
  return bucket_container_.getAllBuckets();
}

SearchState::Phase SearchState::getPhase() const {
  if (bucket_container_.empty()) return Phase::ADD_BUCKET;
  if (!bucket_container_.isLatestBucketFull()) return Phase::ADD_KEY;
  if (!bucket_container_.isFull()) return Phase::ADD_BUCKET;
  return Phase::END;
}
bool SearchState::isBucketGroupFull(BucketSpecId const& id) const {
  return bucket_container_.isGroupFull(id);
}

SearchState::KeysetIds SearchState::getUnusedKeys() const {
  KeysetIds result;
  result.reserve(unused_keys_bitset_.count());
  for (size_t pos = unused_keys_bitset_.find_first();
       pos != boost::dynamic_bitset<>::npos;
       pos = unused_keys_bitset_.find_next(pos)) {
    result.push_back(pos);
  }
  return result;
}