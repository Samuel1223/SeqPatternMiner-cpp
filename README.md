# SeqPatternMiner-cpp

A small, dependency-free C++17 library for **sequential pattern mining** — the
problem of discovering ordered patterns that recur across many sequences. It is
the kind of analysis used on player activity logs to find common progressions of
in-game actions (e.g. *players who do A, then later B, then later C*).

## Data model

| Type | Meaning |
|------|---------|
| `spm::Item` | an integer symbol (e.g. an encoded action) |
| `spm::Itemset` | a group of items that occur together at one step; kept canonical (non-empty, strictly ascending) |
| `spm::Sequence` | an ordered list of itemsets (one entity's timeline) |
| `spm::Pattern` | a sequential pattern (same shape as a sequence) |

A pattern is *contained* in a sequence when its itemsets appear, in order, each as
a subset of a distinct (not necessarily adjacent) itemset of the sequence. The
**support** of a pattern is the number of database sequences that contain it; a
pattern is **frequent** when its support meets a minimum threshold.

## Layout

```
include/spm/types.hpp                  # Item / Itemset / Sequence / Pattern / PatternSupport
include/spm/subsequence.hpp            # contains(): the containment primitive
include/spm/sequential_pattern_miner.hpp
src/subsequence.cpp                    # contains() — implemented
src/sequential_pattern_miner.cpp       # SequentialPatternMiner — the miner
```

`contains()` is provided. `SequentialPatternMiner` (in
`src/sequential_pattern_miner.cpp`) is the mining engine: it consumes a database
of sequences and exposes the frequent patterns, per-pattern support, and the
highest-support patterns. See the doc comments in
`include/spm/sequential_pattern_miner.hpp` for the full contract.

## Building

```bash
cmake -S . -B build && cmake --build build
```

This builds the `spm` static library. The library targets C++17 and has no
third-party dependencies.
