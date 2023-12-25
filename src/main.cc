#include <bits/stdc++.h>

#include "src/search/search.h"

using namespace std;

const string KEYSET = "abcdefghijklmnopqrstuvwxyz',.;";

int main(int, char**) {
  Keyset keyset(KEYSET.begin(), KEYSET.end());
  std::vector<BucketSpec> bucket_specs = {{3, 6}, {6, 2}};
  std::ifstream corpus_file("static/text/iweb-corpus-samples-cleaned.txt");
  RawCorpusStats raw_corpus_stats(corpus_file);
  Threshold threshold(0.01, 0.065);
  auto result = search(keyset, bucket_specs, raw_corpus_stats, threshold);

  std::ofstream out_file("result.txt");

  for (auto [sfb, sfs, layout] : result) {
    out_file << sfb << " " << sfs << " ";
    for (auto bucket : layout) {
      string s(bucket.begin(), bucket.end());
      out_file << s << " ";
    }
    out_file << endl;
  }
  return 0;
}