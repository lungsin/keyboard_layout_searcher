import json
from repl.corpus import load_raw_corpus_stats, new_keyset_corpus_stats
from repl.language_config.english import EnglishKeysetConfig

if __name__ == "__main__":
    print("Loading raw corpus")
    raw_stats = load_raw_corpus_stats(
        "static/text/shai/iweb-corpus-samples-cleaned.txt"
    )

    print("Preparing keyset corpus stats")
    keyset_config = EnglishKeysetConfig
    keyset_stats = new_keyset_corpus_stats(keyset_config, raw_stats)
    print("Saving the keyset corpus stats to json")
    with open("static/stats/shai.json") as f:
        print(keyset_stats.to_dict())
        json.dump(keyset_stats.to_dict(), f)
