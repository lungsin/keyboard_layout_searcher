#include "raw_corpus_stats.h"

#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(RawCorpusStats, TestAddWord) {
  RawCorpusStats stats;

  std::string s = "abcdefg";
  stats.addWord("abcdefg");

  EXPECT_EQ(stats.characters.size(), 7);
  for (int i = 0; i < (int)s.size(); i++) {
    EXPECT_EQ(stats.characters[s[i]], 1);
  }

  EXPECT_EQ(stats.bigrams.size(), 6);
  EXPECT_EQ(stats.bigrams["ab"], 1);
  EXPECT_EQ(stats.bigrams["bc"], 1);
  EXPECT_EQ(stats.bigrams["cd"], 1);
  EXPECT_EQ(stats.bigrams["de"], 1);
  EXPECT_EQ(stats.bigrams["ef"], 1);
  EXPECT_EQ(stats.bigrams["fg"], 1);

  EXPECT_EQ(stats.skipgrams.size(), 5);
  EXPECT_EQ(stats.skipgrams["ac"], 1);
  EXPECT_EQ(stats.skipgrams["bd"], 1);
  EXPECT_EQ(stats.skipgrams["ce"], 1);
  EXPECT_EQ(stats.skipgrams["df"], 1);
  EXPECT_EQ(stats.skipgrams["eg"], 1);
}