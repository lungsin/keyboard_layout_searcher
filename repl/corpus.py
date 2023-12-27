from dataclasses import dataclass, field
import dataclasses
from collections import Counter
from typing import Self
import json

from dataclasses_json import dataclass_json

from repl.keyset_config import (
    KeysetConfig,
    KeysetOneToOneReplacement,
    KeysetReplacementConfig,
    LettersToLowercaseReplacement,
)


@dataclass_json
@dataclass
class CorpusStats:
    total_chars: int = 0
    total_bigrams: int = 0
    total_skipgrams: int = 0
    total_skipgrams2: int = 0
    total_skipgrams3: int = 0
    total_trigrams: int = 0

    num_chars: Counter[str] = field(default_factory=Counter)
    num_bigrams: Counter[str] = field(default_factory=Counter)
    num_skipgrams: Counter[str] = field(default_factory=Counter)
    num_skipgrams2: Counter[str] = field(default_factory=Counter)
    num_skipgrams3: Counter[str] = field(default_factory=Counter)
    num_trigrams: Counter[str] = field(default_factory=Counter)

    def replace(
        self: Self,
        config: KeysetOneToOneReplacement
        | LettersToLowercaseReplacement
        | KeysetReplacementConfig,
    ) -> Self:
        if isinstance(config, KeysetOneToOneReplacement):
            return self.replace_one_to_one(config)
        elif isinstance(config, LettersToLowercaseReplacement):
            return self.replace_letters_to_lowercase(config)
        elif isinstance(config, KeysetReplacementConfig):
            return (
                self.replace(config.one_to_one)
                .replace(config.letters_to_lowercase)
                .replace(config.punct_unshifted_replacement)
            )

        raise TypeError(f"Invalid config type", type(config))

    def replace_one_to_one(self: Self, config: KeysetOneToOneReplacement) -> Self:
        stats = CorpusStats(
            total_chars=self.total_chars,
            total_bigrams=self.total_bigrams,
            total_skipgrams=self.total_skipgrams,
            total_skipgrams2=self.total_skipgrams2,
            total_skipgrams3=self.total_skipgrams3,
            total_trigrams=self.total_trigrams,
        )

        replacement_chars = zip(config.before, config.after)

        def replace_stats(stats_before: Counter[str], stats_after: Counter[str]):
            for ngram, num in stats_before.items():
                for before, after in replacement_chars:
                    new_ngram = (
                        ngram.replace(before, after) if before in ngram else ngram
                    )
                    stats_after[new_ngram] += num

        replace_stats(self.num_chars, stats.num_chars)
        replace_stats(self.num_bigrams, stats.num_bigrams)
        replace_stats(self.num_skipgrams, stats.num_skipgrams)
        replace_stats(self.num_skipgrams2, stats.num_skipgrams2)
        replace_stats(self.num_skipgrams3, stats.num_skipgrams3)
        replace_stats(self.num_trigrams, stats.num_trigrams)

        return stats

    def replace_letters_to_lowercase(
        self: Self, letters_to_lowercase: LettersToLowercaseReplacement
    ) -> Self:
        return self.replace_one_to_one(
            KeysetOneToOneReplacement(
                before=letters_to_lowercase.list,
                after=letters_to_lowercase.list.lower(),
            )
        )

    def filter(self: Self, keyset: str):
        keyset_set = set(keyset)
        stats = CorpusStats()

        def filter_stats(stats_before: Counter[str], stats_after: Counter[str]) -> int:
            total = 0
            for ngram, num in stats_before.items():
                is_valid_ngram = keyset_set.issuperset(ngram)
                if is_valid_ngram:
                    stats_after[ngram] += num
                    total += num
            return total

        stats.total_chars = filter_stats(self.num_chars, stats.num_chars)
        stats.total_bigrams = filter_stats(self.num_bigrams, stats.num_bigrams)
        stats.total_skipgrams = filter_stats(self.num_skipgrams, stats.num_skipgrams)
        stats.total_skipgrams2 = filter_stats(self.num_skipgrams2, stats.num_skipgrams2)
        stats.total_skipgrams3 = filter_stats(self.num_skipgrams3, stats.num_skipgrams3)
        stats.total_trigrams = filter_stats(self.num_trigrams, stats.num_trigrams)

        return stats


@dataclass_json
@dataclass
class KeysetCorpusStats:
    keyset_config: KeysetConfig
    stats: CorpusStats

    # @staticmethod
    # def load_json(file_name: str) -> Self:
    #     with open(file_name, "r") as f:
    #         return CorpusStats(**json.load(f))

    # def save_json(self: Self, file_name: str):
    #     with open(file_name, "w") as f:
    #         print(dataclasses.asdict(self))
    #         json.dump(dataclasses.asdict(self), f, indent=4)


def load_raw_corpus_stats(file_name: str) -> CorpusStats:
    stats = CorpusStats()

    def processWord(word: str):
        for i in range(len(word)):
            c = word[i]

            stats.total_chars += 1
            stats.num_chars[c] += 1

            if i >= 1:
                bigram = word[i - 1 : i + 1]
                stats.total_bigrams += 1
                stats.num_bigrams[bigram] += 1

            if i >= 2:
                stats.total_skipgrams += 1
                stats.num_skipgrams["".join([word[i - 2], c])] += 1

                trigram = word[i - 2 : i + 1]
                stats.total_trigrams += 1
                stats.num_trigrams[trigram] += 1

            if i >= 3:
                stats.total_skipgrams2 += 1
                stats.num_skipgrams2["".join([word[i - 3] + c])] += 1

            if i >= 4:
                stats.total_skipgrams3 += 1
                stats.num_skipgrams3["".join([word[i - 4] + c])] += 1

    with open(file_name) as f:
        for i, line in enumerate(f):
            if i % 1000 == 0:
                print(f"processing {i}-th line")
            if i > 10000:
                break
            for word in line.split():
                processWord(word)

    return stats


def new_keyset_corpus_stats(
    keyset_config: KeysetConfig, raw_corpus_stats: CorpusStats
) -> KeysetCorpusStats:
    return KeysetCorpusStats(
        keyset_config=keyset_config,
        stats=raw_corpus_stats.replace(keyset_config.replacement),
    )
