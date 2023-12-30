#pragma once

#include <nlohmann/json.hpp>

#include "types.h"

using json = nlohmann::json;

struct KeysetOneToOneReplacement {
  WideChar from;
  WideChar to;
};

void to_json(json& j, const KeysetOneToOneReplacement& c);

void from_json(const json& j, KeysetOneToOneReplacement& c);

struct KeysetOneToOneReplacements {
  std::vector<KeysetOneToOneReplacement> replacements;
};

void to_json(json& j, const KeysetOneToOneReplacements& c);

void from_json(const json& j, KeysetOneToOneReplacements& c);

struct KeysetOneToManyReplacement {
  WideChar from;
  WideString to;
};

void to_json(json& j, const KeysetOneToManyReplacement& c);

void from_json(const json& j, KeysetOneToManyReplacement& c);

struct KeysetOneToManyReplacements {
  std::vector<KeysetOneToManyReplacement> replacements;
};

void to_json(json& j, const KeysetOneToManyReplacements& c);

void from_json(const json& j, KeysetOneToManyReplacements& c);

struct LettersToLowercaseReplacement {
  WideString letters;
};

void to_json(json& j, const LettersToLowercaseReplacement& c);

void from_json(const json& j, LettersToLowercaseReplacement& c);

struct KeysetReplacementConfig {
  KeysetOneToOneReplacements one_to_one;
  KeysetOneToOneReplacements punct_unshifted_replacement;
  LettersToLowercaseReplacement letters_to_lowercase;
  KeysetOneToManyReplacements one_to_many;
};

void to_json(json& j, const KeysetReplacementConfig& c);

void from_json(const json& j, KeysetReplacementConfig& c);

struct KeysetConfig {
  WideString keyset;
  KeysetReplacementConfig replacement;
};

void to_json(json& j, const KeysetConfig& c);

void from_json(const json& j, KeysetConfig& c);
