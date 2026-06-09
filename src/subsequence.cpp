#include "spm/subsequence.hpp"

namespace spm {

namespace {

// True iff `needle` is a subset of `haystack`. Both are canonical (strictly
// ascending), so this is a linear merge.
bool is_subset(const Itemset& needle, const Itemset& haystack) {
  std::size_t i = 0;
  std::size_t j = 0;
  while (i < needle.size() && j < haystack.size()) {
    if (needle[i] == haystack[j]) {
      ++i;
      ++j;
    } else if (needle[i] > haystack[j]) {
      ++j;
    } else {
      // needle[i] < haystack[j]: this needle item can never be matched.
      return false;
    }
  }
  return i == needle.size();
}

}  // namespace

bool contains(const Sequence& sequence, const Pattern& pattern) {
  // Greedy left-to-right matching: advance through the pattern itemsets,
  // matching each against the earliest possible sequence itemset.
  std::size_t k = 0;
  if (k == pattern.size()) return true;
  for (const Itemset& element : sequence) {
    if (is_subset(pattern[k], element)) {
      ++k;
      if (k == pattern.size()) return true;
    }
  }
  return false;
}

}  // namespace spm
