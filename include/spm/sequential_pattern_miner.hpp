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
  // Throws std::invalid_argument if min_support <= 0 or max_length < 0.
  explicit SequentialPatternMiner(int min_support, int max_length = 0);

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

  // Number of sequences in the fitted database. Throws std::logic_error if
  // called before a successful fit.
  int num_sequences() const;

  // Number of frequent patterns found. Throws std::logic_error if called before
  // a successful fit.
  std::size_t num_patterns() const;

 private:
  int min_support_;
  int max_length_;
  bool fitted_ = false;
  std::vector<Sequence> database_;
  std::vector<PatternSupport> patterns_;
};

}  // namespace spm

#endif  // SPM_SEQUENTIAL_PATTERN_MINER_HPP
