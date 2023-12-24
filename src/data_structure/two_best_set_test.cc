#include "two_best_set.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::WhenSorted;

// Demonstrate some basic assertions.
TEST(TwoBestSet, TestSimpleOperations) {
  TwoBestSet<int, int, int> bestInt;

  EXPECT_THAT(bestInt.insert(5, 5, 0), IsEmpty());
  EXPECT_THAT(bestInt.insert(4, 6, 1), IsEmpty());
  EXPECT_THAT(bestInt.insert(1, 2, 2), WhenSorted(ElementsAre(0, 1)));
  EXPECT_THAT(bestInt.insert(2, 1, 3), IsEmpty());
  EXPECT_THAT(bestInt.getAllData(), WhenSorted(ElementsAre(2, 3)));
}