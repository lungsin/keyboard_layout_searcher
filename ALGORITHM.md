# Algorithm Writeup

Currently, the code to run the brute-force is quite messy. It's unreadable and probably doesn't run in your machine.
This is mostly because this project is an experiment-heavy project.
And so, I wrote the code with the mindset of having fast iterations, not broad usability like what other
keyboard layout analyzers do.

The goal of this document is to describe the algorithm in detail so that the community can learn from this project,
and in turn, the community can hopefully create the proper tools designed for broad usage based on the algorithm found
in this project.

## High Level Overview

The goal of the algorithm is to place 30 keys to 30 possible location in the keyboard.
The locations in the keyboard are assumed to be 3 rows each having 10 keys.
These numbers are selected because most keyboard has this geometry (3x10 keys).
The algorithm described here can be modified to accommodate other type of keyboards with more keys.
For the purpose of ease of explanation, we'll assume that there are 30 keys here.

We must define a score for a layout, then the goal of the algorithm is to find layout with the minimum score.
This project uses a weighted sum of various common KB layout statistics such as:

- SFB 2U: Same Finger Bigram 2 unit
- SFS 2U: Same Finger Skipgram 2 unit
- FSB: Full Scissors Bigram
- FSS: Full Scissors Skipgram
- Total Redirect

All of other common statistics can also be used. In fact, this project implements the calculation of other common statistics,
but it has zero weight because during my experimentation, I didn't really see a significant benefit of having those statistics as part of the score.
Such statistics are:

- SFB & SFS
- HSB & HSS: Half Scissors Bigram and Skipgram.
- Total Alternate
- Total Rolls
- Per-Finger Usage
- Per-Finger SFB

The basic idea of the algorithm is to do a brute-force search. On each step of the brute-force, we have a partially done layout and take one step closer to the full layout. All kind of statistics of the partially-done layout are calculated on-the-fly. These statistics are then used to prune the search space. For example, we can set a threshold for SFB and we should not continue the search with the current partial layout when its SFB has cross the threshold. The search itself is done in stages, where each stage focus on calculating some specific statistics.

## Search Strategy

As mentioned from the previous section, the search strategy here is divided into stages, where each stage focus on some specific statistics. Such stages are:

1. To set **which keys are in the same finger**. Note: This is the most complicated stage of the entire stages! Note that we don't care whether the keys are on the left hand or right hand.
2. To set which hand for each key: left or right hand.
3. To set which row for each key: top, middle, or bottom row.
4. To set which column of the index finger for each key: inner or outer index finger column.

### Stage 1: To set which key are in the same finger

The goal of this stage is to calculate SFB and SFS, hence why we need to know whether two keys are belong in the same finger. This stage is quite complicated, because assigning a finger for each key is NOT the most efficient way for this stage. Specifically, we shouldn't differentiate whether a key belongs to a left or a right hand, because it's simply irrelevant to know whether two keys are in the same finger.

To do this, I want to introduce a terminology called _bucket_. A _bucket_ represents the set of keys for a finger type. There are four bucket types, one for each finger type (index, middle, ring, pinky), where bucket represent index finger has 6 keys and the rest has 3 keys.

Then, the algorithm goes as follow:

Suppose we have the list of the keys. For each key in order of that list, do one of the following:

- Append the key to an existing bucket (assuming the bucket has enough capacity)
- Create a new bucket with a particular bucket type with the key as its first element (assuming the number of the buckets with the bucket type doesn't exceed the number of hands: 2)

In this way, the keys in each bucket are sorted in increasing order, and the buckets for each type are also sorted in increasing order by the first key in the bucket. So, we eliminate the symmetry of the differentiation between left hand and right hand.

### Stage 2: To set which hand a key belong

For each finger type, we simply allocate which bucket belong to which hand: left or right hand.

### Stage 3: To set which row a key belong

For each bucket, we simply permutate which key belong to which row: top, middle, or bottom row.

### Stage 4: To set

For each bucket, we simply permutate remaining configuration of the keys in the index finger: whether the key belongs in the inner or outer column.

##
