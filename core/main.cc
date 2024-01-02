
#include <boost/locale.hpp>
#include <fstream>
#include <iostream>
#include <string>

#include "core/search/search.h"
#include "core/stats/raw_corpus_stats.h"

using namespace std;

const string KEYSET = "abcdefghijklmnopqrstuvwxyz',.;";

const std::filesystem::path kWorkingDirectoryPath =
    std::getenv("BUILD_WORKING_DIRECTORY");

std::string toWorkingDirectory(std::string const& relative_path) {
  return kWorkingDirectoryPath / relative_path;
}

RawCorpusStats loadRawCorpusStats(std::filesystem::path text_path,
                                  std::filesystem::path out_path) {
  std::cerr << "Loading Raw Corpus Stats" << std::endl;
  if (std::filesystem::exists(out_path)) {
    std::cerr << "Found a cache, loading the raw corpus file directly: "
              << out_path << std::endl;
    return RawCorpusStats::fromJsonFile(out_path);
  } else {
    std::cerr << "Loading the raw corpus file from the text file: " << text_path
              << std::endl;
    std::ifstream corpus_file(text_path);
    RawCorpusStats raw_corpus_stats(corpus_file);
    raw_corpus_stats.save_as_json(out_path);
    return raw_corpus_stats;
  }
}

KeysetConfig loadKeysetConfig(std::filesystem::path const& config_path) {
  json j;
  std::ifstream config_stream(config_path);
  config_stream >> j;
  return j.template get<KeysetConfig>();
}

int main(int, char**) {
  // Create system default locale
  boost::locale::generator gen;
  locale loc = gen("");
  locale::global(loc);

  std::filesystem::path raw_shai_path =
      toWorkingDirectory("static/stats/shai/shai.raw.json");

  std::filesystem::path text_shai_path =
      "static/text/shai/iweb-corpus-samples-cleaned.txt";

  RawCorpusStats raw_corpus_stats =
      loadRawCorpusStats(text_shai_path, raw_shai_path);

  KeysetConfig keyset_config =
      loadKeysetConfig("static/keyset_config/default.json");
  std::vector<BucketSpec> bucket_specs = {{3, 6}, {6, 2}};
  Threshold threshold(0.01, 0.065);
  auto result =
      search(keyset_config, bucket_specs, raw_corpus_stats, threshold);

  std::ofstream out_file(toWorkingDirectory("result.txt"));

  for (auto [sfb, sfs, layout] : result) {
    out_file << sfb << " " << sfs << " ";
    for (auto bucket : layout) {
      out_file << bucket << " ";
    }
    out_file << endl;
  }
  return 0;
}