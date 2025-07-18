import numpy as np


def process(
    text: str,
    characters_cnt: np.ndarray,
    bigrams_cnt: np.ndarray,
    skipgrams_cnt: np.ndarray,
    skipgrams2_cnt: np.ndarray,
    skipgrams3_cnt: np.ndarray,
    trigrams_cnt: np.ndarray,
):
    byte_arr = np.frombuffer(text.encode(), dtype=np.int8)
    arr = np.array(byte_arr)

    # replace non ascii with space
    arr[arr < 0] = ord(" ")

    # get ngrams
    bigrams = np.stack((arr[:-1], arr[1:]), axis=-1)
    skipgrams = np.stack((arr[:-2], arr[2:]), axis=-1)
    skipgrams2 = np.stack((arr[:-3], arr[3:]), axis=-1)
    skipgrams3 = np.stack((arr[:-4], arr[4:]), axis=-1)
    trigrams = np.stack((arr[:-2], arr[1:-1], arr[2:]), axis=-1)

    # update cnt
    unique_chars, char_cnt = np.unique(arr, return_counts=True)
    characters_cnt[unique_chars] += char_cnt

    def update_multigram(cnt: np.ndarray, ngrams: np.ndarray):
        unique_ngrams, ngram_cnt = np.unique(ngrams, return_counts=True, axis=0)
        cnt[tuple(unique_ngrams.transpose())] += ngram_cnt

    update_multigram(bigrams_cnt, bigrams)
    update_multigram(skipgrams_cnt, skipgrams)
    update_multigram(skipgrams2_cnt, skipgrams2)
    update_multigram(skipgrams3_cnt, skipgrams3)
    update_multigram(trigrams_cnt, trigrams)


def get_ngram_freq_dict(cnt: np.ndarray):
    freq = cnt / cnt.sum()
    ngram_idxs = cnt.nonzero()
    obj = dict({})
    for ngram_idx in zip(*ngram_idxs):
        ngram_freq = freq[ngram_idx]
        ngram = "".join(map(chr, ngram_idx))
        obj[ngram] = ngram_freq
    return obj
