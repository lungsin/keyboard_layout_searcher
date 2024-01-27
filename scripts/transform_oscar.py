import json

with open("static/stats/oscar/id.raw.json", "r") as f:
    stats: dict[str, dict[str, float]] = json.load(f)

PUNCT = ",./;'[]"
PUNCT_SHIFTED = '<>?:"\{\}'
ALPHA = "etaoinsrhldcumfpgywbvkxjqz"
transform = (
    dict(zip(PUNCT_SHIFTED, PUNCT))
    | dict(zip(PUNCT, PUNCT))
    | dict(zip(ALPHA.upper(), ALPHA.lower()))
    | dict(zip(ALPHA.lower(), ALPHA.lower()))
)

print(transform)

new_stats = dict({})
for key, ngram_stats in stats.items():
    new_ngram_stats = dict({})
    for ngram, freq in ngram_stats.items():
        lower_ngram = ngram.lower()
        new_char_list = []
        is_ok = True
        for char in lower_ngram:
            if char not in transform:
                is_ok = False
                break
            new_char_list.append(transform[char])

        if not is_ok:
            continue

        new_ngram = "".join(new_char_list)
        if new_ngram not in new_ngram_stats:
            new_ngram_stats[new_ngram] = 0
        new_ngram_stats[new_ngram] += freq

    new_sum = sum(new_ngram_stats.values(), 0.0)
    for ngram in new_ngram_stats:
        new_ngram_stats[ngram] /= new_sum
    new_stats[key] = new_ngram_stats


with open("static/stats/oscar/id.json", "w") as f:
    json.dump(new_stats, f, indent=4)
