#include "raw_corpus_stats.h"

#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(RawCorpusStats, TestAddWord) {
  RawCorpusStats stats;

  WideString s = U"abcdefg";
  stats.addWord(s);

  EXPECT_EQ(stats.getTotalChars(), 7);
  for (size_t i = 0; i < s.size(); ++i) {
    EXPECT_EQ(stats.getCharFreq(s[i]), 1);
  }

  EXPECT_EQ(stats.getTotalBigrams(), 6);
  for (size_t i = 1; i < s.size(); ++i) {
    EXPECT_EQ(stats.getBigramFreq(s[i - 1], s[i]), 1);
  }

  EXPECT_EQ(stats.getTotalSkipgrams(), 5);
  for (size_t i = 2; i < s.size(); ++i) {
    EXPECT_EQ(stats.getSkipgramFreq(s[i - 2], s[i]), 1);
  }

  EXPECT_EQ(stats.getTotalSkipgrams2(), 4);
  for (size_t i = 3; i < s.size(); ++i) {
    EXPECT_EQ(stats.getSkipgram2Freq(s[i - 3], s[i]), 1);
  }

  EXPECT_EQ(stats.getTotalSkipgrams3(), 3);
  for (size_t i = 4; i < s.size(); ++i) {
    EXPECT_EQ(stats.getSkipgram3Freq(s[i - 4], s[i]), 1);
  }

  EXPECT_EQ(stats.getTotalTrigrams(), 5);
  for (size_t i = 2; i < s.size(); ++i) {
    EXPECT_EQ(stats.getTrigramFreq(s[i - 2], s[i - 1], s[i]), 1);
  }
}