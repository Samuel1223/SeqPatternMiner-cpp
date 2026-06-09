#ifndef SPM_SUBSEQUENCE_HPP
#define SPM_SUBSEQUENCE_HPP

#include "spm/types.hpp"

namespace spm {

// Containment test for sequential patterns (already implemented in
// src/subsequence.cpp -- use it, do not re-implement it).
//
// Returns true iff `pattern` is a subsequence of `sequence`: there exist
// indices j_1 < j_2 < ... < j_m (one per pattern itemset, strictly increasing)
// such that pattern[k] is a subset of sequence[j_k] for every k. Matching is
// order-preserving across itemsets but the matched sequence itemsets need not
// be contiguous.
//
// The empty pattern is contained in every sequence. Both arguments are assumed
// to be in canonical form (every itemset non-empty with strictly ascending
// items); behaviour is unspecified otherwise.
bool contains(const Sequence& sequence, const Pattern& pattern);

}  // namespace spm

#endif  // SPM_SUBSEQUENCE_HPP
