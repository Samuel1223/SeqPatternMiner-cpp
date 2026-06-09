#ifndef SPM_TYPES_HPP
#define SPM_TYPES_HPP

#include <vector>

namespace spm {

// An item is a small integer symbol (e.g. an encoded game action).
using Item = int;

// An itemset (a.k.a. an "element" or "event") is a group of items that occur
// together at a single step. Throughout this library an itemset is kept in
// canonical form: it is non-empty and its items are strictly ascending
// (sorted and distinct).
using Itemset = std::vector<Item>;

// A sequence is an ordered list of itemsets, e.g. a single player's session
// where each itemset is the set of actions taken at one time step.
using Sequence = std::vector<Itemset>;

// A sequential pattern has the same shape as a sequence.
using Pattern = std::vector<Itemset>;

// A frequent pattern together with the number of input sequences that contain
// it (its support).
struct PatternSupport {
  Pattern pattern;
  int support;
};

}  // namespace spm

#endif  // SPM_TYPES_HPP
