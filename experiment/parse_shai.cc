#include <nlohmann/json.hpp>

#include "utils.hpp"

using namespace std;
using json = nlohmann::json;

int main() {
  map<char, char> mapper;
  for (char c = 'a'; c <= 'z'; c++) mapper[c] = c, mapper[toupper(c)] = c;
  string punct = ",./;'";
  string punct_shifted = "<>?:\"";
  for (size_t i = 0; i < punct.size(); ++i)
    mapper[punct[i]] = punct[i], mapper[punct_shifted[i]] = punct[i];

  Stats stats = calculateStatsFromText(
      "static/text/shai/iweb-corpus-samples-cleaned.txt", mapper);

  json j = stats;
  cout << std::setw(4) << j << endl;
}