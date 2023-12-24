#pragma once

#include <vector>

using namespace std;

using Bucket = vector<char>;

Bucket newBucketWithCapacity(int capacity);

struct BucketSpec {
  int capacity, count;
};

class RecursionState {
  const vector<BucketSpec> bucketSpecs;

  vector<vector<Bucket>> bucketsBySpec;
  vector<int> numBucketsAddedBySpec;
  vector<pair<int, int>> activeBucketIdStack;

 public:
  RecursionState(const vector<BucketSpec>& bucketSpecs);

  void addKeyToCurrentBucket(char key);

  void undoAddKeyToCurrentBucket();

  void addNewBucket(int bucketSpecId);

  void undoAddNewBucket();

 private:
  Bucket& getCurrentBucket();
};