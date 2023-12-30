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
struct KeysetConfigTransformator {
  T data;

  KeysetConfigTransformator apply_config(const KeysetConfig& config) const {
    return replace(config.replacement).whitelistChars(config.keyset);
  }

  KeysetConfigTransformator replace(
      const KeysetReplacementConfig& config) const {
    return replace(config.one_to_one)
        .replace(config.letters_to_lowercase)
        .replace(config.punct_unshifted_replacement);
  }

  KeysetConfigTransformator replace(
      const KeysetOneToOneReplacements& config) const {
    T new_stats = data;
    for (const auto& [from, to] : config.replacements) {
      new_stats.replaceChar(from, to);
    }
    return KeysetConfigTransformator({new_stats});
  }

  KeysetConfigTransformator replace(
      const KeysetOneToManyReplacements& config) const {
    // TODO: support one to many replacements
    return *this;
  }

  KeysetConfigTransformator replace(
      const LettersToLowercaseReplacement& config) const {
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
    return replace(one_to_one).replace(one_to_many);
  }

  KeysetConfigTransformator whitelistChars(const WideString& whitelist) const {
    T new_stats = data;
    new_stats.whitelistChars(whitelist);
    return KeysetConfigTransformator({new_stats});
  }

  T get() const { return data; }
};
