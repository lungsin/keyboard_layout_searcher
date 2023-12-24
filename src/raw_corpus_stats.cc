#include "raw_corpus_stats.h"

RawCorpusStats::RawCorpusStats(istream& textStream) {
  for (string word; textStream >> word;) {
    addWord(word);
  }
}

void RawCorpusStats::addWord(const string& word) {
  const int n = word.size();
  for (const char& c : word) {
    characters[c]++;
  }
  for (int i = 1; i < n; i++) {
    bigrams[{word[i - 1], word[i]}]++;
  }
  for (int i = 2; i < n; i++) {
    trigrams[{word[i - 2], word[i - 1], word[i]}];
    skipgrams[{word[i - 2], word[i]}]++;
  }
  for (int i = 3; i < n; i++) {
    skipgrams2[{word[i - 3], word[i]}]++;
  }
  for (int i = 4; i < n; i++) {
    skipgrams3[{word[i - 4], word[i]}]++;
  }

  totalChars += n;
  totalBigrams += n - 1;
  totalTrigrams += n - 2;
  totalSkipgrams += n - 2;
  totalSkipgrams2 += n - 3;
  totalSkipgrams3 += n - 4;
}