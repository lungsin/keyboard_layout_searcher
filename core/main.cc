#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <fstream>
#include <iostream>
#include <string>

#include "core/stats/raw_corpus_stats.h"

using namespace std;

const string KEYSET = "abcdefghijklmnopqrstuvwxyz',.;";

void test() {
  bitset<128> bs;
  bs._Find_next(3);

  boost::dynamic_bitset<> dbs;
  dbs.find_first();
}
int main(int, char**) {
  // Keyset keyset(KEYSET.begin(), KEYSET.end());
  // std::vector<BucketSpec> bucket_specs = {{3, 6}, {6, 2}};
  std::ifstream corpus_file("static/text/shai/iweb-corpus-samples-cleaned.txt");
  RawCorpusStats raw_corpus_stats(corpus_file);

  raw_corpus_stats.save_as_json("static/stats/shai.json");

  // Threshold threshold(0.01, 0.065);
  // auto result = search(keyset, bucket_specs, raw_corpus_stats, threshold);

  // std::ofstream out_file("result.txt");

  // for (auto [sfb, sfs, layout] : result) {
  //   out_file << sfb << " " << sfs << " ";
  //   for (auto bucket : layout) {
  //     string s(bucket.begin(), bucket.end());
  //     out_file << s << " ";
  //   }
  //   out_file << endl;
  // }
  return 0;
}