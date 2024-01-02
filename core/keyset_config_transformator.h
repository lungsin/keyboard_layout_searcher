#pragma once

#include "core/keyset_config.h"
#include "core/unicode/unicode.h"

template <typename T>
concept KeysetConfigTransformable =
    requires(T&& data, const WideChar& wc, const WideString& ws) {
      { data.replaceChar(wc, wc) } -> std::same_as<void>;
      { data.whitelistChars(ws) } -> std::same_as<void>;
    };

template <KeysetConfigTransformable T>
void replace(const KeysetOneToOneReplacements& config, T& data) {
  for (const auto& [from, to] : config.replacements) {
    data.replaceChar(from, to);
  }
}

template <KeysetConfigTransformable T>
void replace(const KeysetOneToManyReplacements& config, T& data) {
  // TODO: support one to many replacements
}

template <KeysetConfigTransformable T>
void replace(const LettersToLowercaseReplacement& config, T& data) {
  KeysetOneToOneReplacements one_to_one;
  KeysetOneToManyReplacements one_to_many;
  for (const auto& letter : config.letters) {
    const WideString lowercase = unicode::toLowercase(letter);
    if (lowercase.size() == 1) {
      one_to_one.replacements.push_back(
          KeysetOneToOneReplacement({letter, lowercase[0]}));
    } else {
      one_to_many.replacements.push_back(
          KeysetOneToManyReplacement({letter, lowercase}));
    }
  }
  replace(one_to_one, data);
  replace(one_to_many, data);
}

template <KeysetConfigTransformable T>
void whitelistChars(const WideString& whitelist, T& data) {
  data.whitelistChars(whitelist);
}

template <KeysetConfigTransformable T>
void replace(const KeysetReplacementConfig& config, T& data) {
  replace(config.one_to_one, data);
  replace(config.letters_to_lowercase, data);
  replace(config.punct_unshifted_replacement, data);
}

template <KeysetConfigTransformable T>
void apply_config(const KeysetConfig& config, T& data) {
  replace(config.replacement, data);
  whitelistChars(config.keyset, data);
}
