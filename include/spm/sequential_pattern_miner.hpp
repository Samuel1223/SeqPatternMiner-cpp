#ifndef SPM_SEQUENTIAL_PATTERN_MINER_HPP
#define SPM_SEQUENTIAL_PATTERN_MINER_HPP

#include <cstddef>
#include <vector>

#include "spm/types.hpp"

namespace spm {

// Mines frequent sequential patterns from a database of sequences.
//
// A pattern P is "supported" by an input sequence S when P is a subsequence of
// S (see spm::contains). The support of P is the number of input sequences
// that support it. P is "frequent" when its support is at least min_support.
//
// Error model (mirrors the rest of the library):
//   * std::invalid_argument for invalid arguments.
//   * std::logic_error when a result is requested before a successful fit.
class SequentialPatternMiner {
 public:
  // min_support is an absolute count: a pattern is frequent iff it is contained
  // in at least min_support input sequences.
  //
  // max_length caps the size of reported patterns, measured as the total number
  // of items summed over all of a pattern's itemsets. max_length == 0 means no
  // limit.
  //
  // max_gap bounds how far apart consecutive itemsets of a pattern may be
  // matched within a sequence. A pattern is contained in a sequence only if it
  // can be matched at strictly increasing itemset positions j_1 < j_2 < ... <
  // j_m (one per pattern itemset, each a subset of the sequence itemset at that
  // position) such that every adjacent step satisfies j_{k+1} - j_k <= max_gap.
  // max_gap == 0 means no limit (any increasing positions, identical to
  // spm::contains); max_gap == 1 requires consecutive itemsets to be matched at
  // adjacent positions. There is no gap constraint before the first itemset.
  // The bound governs both mining and support_of.
  //
  // max_total_gap bounds the TOTAL number of skipped (unmatched) itemsets
  // summed across the whole match: the sum over consecutive matched pairs of
  // (j_{k+1} - j_k - 1) must be <= max_total_gap. max_total_gap == 0 means no
  // limit. A valid match must satisfy BOTH max_gap (every individual step) and
  // max_total_gap (the match as a whole) simultaneously.
  //
  // Throws std::invalid_argument if min_support <= 0, max_length < 0,
  // max_gap < 0, or max_total_gap < 0.
  explicit SequentialPatternMiner(int min_support, int max_length = 0,
                                  int max_gap = 0, int max_total_gap = 0);

  // Mines all frequent patterns from `database` and stores the result.
  //
  // Every itemset in every sequence must be in canonical form: non-empty, with
  // strictly ascending items. Empty sequences (zero itemsets) and an empty
  // database are allowed. Throws std::invalid_argument if any itemset is empty
  // or not strictly ascending.
  //
  // Calling fit again discards all previous state and replaces it with the
  // results of the new fit. Fitting is fully deterministic.
  void fit(const std::vector<Sequence>& database);

  // All frequent patterns (each with length >= 1) together with their support.
  // The returned collection is a set: every frequent pattern appears exactly
  // once and the order is unspecified. Throws std::logic_error if called before
  // a successful fit.
  std::vector<PatternSupport> patterns() const;

  // The closed frequent patterns. A frequent pattern P is closed iff there is
  // no other frequent pattern Q such that P is a proper subsequence of Q and Q
  // has the same support as P. (The sub-pattern relation here is the ordinary
  // subsequence relation between patterns -- each itemset of P a subset of an
  // itemset of Q at strictly increasing positions -- and does NOT use max_gap;
  // max_gap only affects how support is counted against the database.) The
  // order is unspecified. Throws std::logic_error if called before a successful
  // fit.
  std::vector<PatternSupport> closed_patterns() const;

  // Support of an arbitrary `pattern` under the fitted database: the number of
  // input sequences that contain it. The pattern need not be frequent; an
  // absent pattern has support 0. The empty pattern is contained in every
  // sequence. Every non-empty itemset of `pattern` must be canonical
  // (non-empty, strictly ascending), otherwise std::invalid_argument is thrown.
  // Throws std::logic_error if called before a successful fit.
  int support_of(const Pattern& pattern) const;

  // The k frequent patterns with the greatest support. If k exceeds the number
  // of frequent patterns, all of them are returned. Throws std::invalid_argument
  // if k < 0, and std::logic_error if called before a successful fit.
  std::vector<PatternSupport> top_k(int k) const;

  // True once fit has completed successfully.
  bool is_fitted() const { return fitted_; }

  // The configured minimum support.
  int min_support() const { return min_support_; }

  // The configured maximum pattern length (0 means unlimited).
  int max_length() const { return max_length_; }

  // The configured maximum gap between consecutive matched itemsets (0 means
  // unlimited).
  int max_gap() const { return max_gap_; }

  // The configured maximum total skipped itemsets across a match (0 means
  // unlimited).
  int max_total_gap() const { return max_total_gap_; }

  // Number of sequences in the fitted database. Throws std::logic_error if
  // called before a successful fit.
  int num_sequences() const;

  // Number of frequent patterns found. Throws std::logic_error if called before
  // a successful fit.
  std::size_t num_patterns() const;

 private:
  int min_support_;
  int max_length_;
  int max_gap_;
  int max_total_gap_;
  bool fitted_ = false;
  std::vector<Sequence> database_;
  std::vector<PatternSupport> patterns_;
};

}  // namespace spm

#endif  // SPM_SEQUENTIAL_PATTERN_MINER_HPP
