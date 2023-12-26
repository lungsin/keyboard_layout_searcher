#include "corpus_stats.h"

#include <gtest/gtest.h>

#include "src/raw_corpus_stats.h"

// Demonstrate some basic assertions.
TEST(CorpusStats, TestConstructor) {
  Keyset keyset{'a', 'b', 'c', 'd'};
  RawCorpusStats raw_stats;
  raw_stats.addWord("abcdefg");
  CorpusStats stats(keyset, raw_stats);

  EXPECT_EQ(stats.getBigramOccurance('a', 'b'), 1);
  EXPECT_EQ(stats.getBigramOccurance('b', 'c'), 1);
  EXPECT_EQ(stats.getBigramOccurance('c', 'd'), 1);
  EXPECT_EQ(stats.getBigramOccurance('d', 'e'), 0);
  EXPECT_EQ(stats.getBigramOccurance('e', 'f'), 0);
  EXPECT_EQ(stats.getBigramOccurance('f', 'g'), 0);

  EXPECT_EQ(stats.getSkipgramOccurance('a', 'c'), 1);
  EXPECT_EQ(stats.getSkipgramOccurance('b', 'd'), 1);
  EXPECT_EQ(stats.getSkipgramOccurance('c', 'e'), 0);
  EXPECT_EQ(stats.getSkipgramOccurance('d', 'f'), 0);
  EXPECT_EQ(stats.getSkipgramOccurance('e', 'g'), 0);
}