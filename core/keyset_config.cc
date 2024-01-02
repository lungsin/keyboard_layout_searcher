#include "keyset_config.h"

#include "unicode/unicode.h"

void to_json(json& j, const KeysetOneToOneReplacement& c) {
  j = json{
      {"from", c.from},
      {"to", c.to},
  };
}

void from_json(const json& j, KeysetOneToOneReplacement& c) {
  j.at("from").get_to(c.from);
  j.at("to").get_to(c.to);
}

void to_json(json& j, const KeysetOneToOneReplacements& c) {
  WideString from, to;
  for (const auto& replacement : c.replacements) {
    from += replacement.from;
    to += replacement.to;
  }
  j = json{
      {"from", unicode::toNarrow(from)},
      {"to", unicode::toNarrow(to)},
  };
}

void from_json(const json& j, KeysetOneToOneReplacements& c) {
  std::string from_narrow, to_narrow;
  j.at("from").get_to(from_narrow);
  j.at("to").get_to(to_narrow);
  WideString from = unicode::toWide(from_narrow),
             to = unicode::toWide(to_narrow);
  c.replacements.clear();
  for (size_t i = 0; i < std::min(from_narrow.size(), to_narrow.size()); ++i) {
    c.replacements.emplace_back(from_narrow[i], to_narrow[i]);
  }
}

void to_json(json& j, const KeysetOneToManyReplacement& c) {
  j = json{
      {"from", unicode::toNarrow(WideString({c.from}))},
      {"to", unicode::toNarrow(c.to)},
  };
}

void from_json(const json& j, KeysetOneToManyReplacement& c) {
  std::string from, to;
  j.at("from").get_to(from);
  j.at("to").get_to(to);
  c.from = unicode::toWide(from)[0];
  c.to = unicode::toWide(to);
}

void to_json(json& j, const KeysetOneToManyReplacements& c) {
  j = json{
      {"replacements", c.replacements},
  };
}

void from_json(const json& j, KeysetOneToManyReplacements& c) {
  j.at("replacements").get_to(c.replacements);
}

void to_json(json& j, const LettersToLowercaseReplacement& c) {
  j = json{
      {"letters", unicode::toNarrow(c.letters)},
  };
}

void from_json(const json& j, LettersToLowercaseReplacement& c) {
  std::string letters;
  j.at("letters").get_to(letters);
  c.letters = unicode::toWide(letters);
}

void to_json(json& j, const KeysetReplacementConfig& c) {
  j = json{
      {"one_to_one", c.one_to_one},
      {"punct_unshifted_replacement", c.punct_unshifted_replacement},
      {"letters_to_lowercase", c.letters_to_lowercase},
      {"one_to_many", c.one_to_many},
  };
}

void from_json(const json& j, KeysetReplacementConfig& c) {
  j.at("one_to_one").get_to(c.one_to_one);
  j.at("punct_unshifted_replacement").get_to(c.punct_unshifted_replacement);
  j.at("letters_to_lowercase").get_to(c.letters_to_lowercase);
  j.at("one_to_many").get_to(c.one_to_many);
}

void to_json(json& j, const KeysetConfig& c) {
  j = json{
      {"keyset", unicode::toNarrow(c.keyset)},
      {"replacement", c.replacement},
  };
}

void from_json(const json& j, KeysetConfig& c) {
  std::string keyset_narrow;
  j.at("keyset").get_to(keyset_narrow);
  c.keyset = unicode::toWide(keyset_narrow);
  j.at("replacement").get_to(c.replacement);
}
