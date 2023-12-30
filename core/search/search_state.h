#pragma once

#include <boost/container/static_vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <vector>

using KeyId = size_t;
using Bucket = std::vector<KeyId>;
using BucketSpecCountId = size_t;
using BucketSpecId = size_t;
using BucketId = std::pair<BucketSpecId, BucketSpecCountId>;

struct BucketSpec {
  int capacity, count;
};

// We only have 10 fingers.
constexpr size_t kMaxBucketNumber = 10;
constexpr size_t kMaxKeysetSize = 64;

template <typename T>
using static_vector = boost::container::static_vector<T, kMaxBucketNumber>;

class BucketContainer {
 public:
  BucketContainer(const std::vector<BucketSpec>& bucket_specs);

  void appendBucket(const BucketSpecId& bucket_spec_id);
  void popBucket();

  Bucket& getLatestBucket();

  const Bucket& getLatestBucket() const;

  bool empty() const;

  bool isLatestBucketFull() const;

  bool isGroupFull(size_t const& group_id) const;

  bool isFull() const;

  static_vector<Bucket> getAllBuckets() const;

 private:
  using FlattenId = size_t;

  FlattenId getLatestFlattenId() const;

  std::vector<BucketSpec> const bucket_specs_;

  static_vector<Bucket> prealloc_buckets_;

  static_vector<size_t> group_size_;
  static_vector<size_t> group_capacity_;
  static_vector<FlattenId> group_first_flatten_id_;
  static_vector<BucketSpecId> bucket_spec_id_stack_;
};

class SearchState {
 public:
  enum Phase {
    ADD_BUCKET,
    ADD_KEY,
    END,
  };

  using KeysetIds = std::vector<KeyId>;

  SearchState(const size_t& keyset_size,
              const std::vector<BucketSpec>& bucket_specs);

  void addKeyToCurrentBucket(const KeyId& keyId);

  void undoAddKeyToCurrentBucket();

  void addNewBucket(const BucketSpecId& bucketSpecId);

  void undoAddNewBucket();

  // Action helper
  Phase getPhase() const;

  bool isBucketGroupFull(BucketSpecId const& id) const;

  KeysetIds getUnusedKeys() const;

  // Helper
  Bucket const& getCurrentBucket() const;

  static_vector<Bucket> getAllBuckets() const;

  const std::vector<BucketSpec> bucket_specs_;
  const size_t keyset_size_;

 private:
  BucketContainer bucket_container_;

  boost::dynamic_bitset<> unused_keys_bitset_;
};