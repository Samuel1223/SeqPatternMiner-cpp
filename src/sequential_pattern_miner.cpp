#include "spm/sequential_pattern_miner.hpp"

#include <algorithm>
#include <deque>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "spm/subsequence.hpp"

namespace spm {

namespace {

// Total number of items in a pattern (summed over its itemsets).
int total_items(const Pattern& p) {
  int n = 0;
  for (const Itemset& e : p) n += static_cast<int>(e.size());
  return n;
}

// Validates that every itemset is non-empty and strictly ascending.
bool is_canonical(const Pattern& p) {
  for (const Itemset& e : p) {
    if (e.empty()) return false;
    for (std::size_t i = 1; i < e.size(); ++i) {
      if (e[i - 1] >= e[i]) return false;
    }
  }
  return true;
}

// True iff `needle` is a subset of `haystack`; both are canonical (strictly
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
      return false;
    }
  }
  return i == needle.size();
}

// Gap-bounded containment honouring BOTH the per-step bound (max_gap: every
// adjacent step j_{k+1} - j_k <= max_gap) AND the total-skipped budget
// (max_total_gap: sum of (j_{k+1} - j_k - 1) <= max_total_gap). 0 means
// unbounded for each.
//
// Two traps live here. (1) A greedy earliest-match (as spm::contains does) is
// wrong under a per-step bound. (2) A per-step-only reachability check is wrong
// under the TOTAL budget: a match can satisfy every step yet blow the overall
// budget. We therefore carry, for every feasible end position, the MINIMUM
// total skipped itemsets so far, and prune on both bounds.
bool contains_within_gaps(const Sequence& seq, const Pattern& pattern, int max_gap,
                          int max_total_gap) {
  if (pattern.empty()) return true;
  const bool step_unbounded = (max_gap <= 0);
  const bool total_unbounded = (max_total_gap <= 0);
  if (step_unbounded && total_unbounded) return contains(seq, pattern);

  const int n = static_cast<int>(seq.size());
  const int INF = std::numeric_limits<int>::max();
  // best[j] = minimum total skipped itemsets to match pattern[0..k] ending at j.
  std::vector<int> best(n, INF);
  for (int j = 0; j < n; ++j) {
    if (is_subset(pattern[0], seq[j])) best[j] = 0;  // no gap before the first itemset
  }

  for (std::size_t k = 1; k < pattern.size(); ++k) {
    std::vector<int> next(n, INF);
    for (int j = 0; j < n; ++j) {
      if (!is_subset(pattern[k], seq[j])) continue;
      for (int prev = 0; prev < j; ++prev) {
        if (best[prev] == INF) continue;
        const int dist = j - prev;
        if (!step_unbounded && dist > max_gap) continue;
        const int total = best[prev] + (dist - 1);
        if (!total_unbounded && total > max_total_gap) continue;
        if (total < next[j]) next[j] = total;
      }
    }
    best.swap(next);
  }

  for (int j = 0; j < n; ++j) {
    if (best[j] != INF) return true;
  }
  return false;
}


// True iff pattern itemset `pe` matches sequence itemset `se`: a wildcard
// itemset {WILDCARD} matches anything, otherwise pe must be a subset of se.
bool itemset_matches(const Itemset& pe, const Itemset& se) {
  if (pe.size() == 1 && pe[0] == SequentialPatternMiner::WILDCARD) return true;
  return is_subset(pe, se);
}

// Distinct embeddings of `pattern` in `seq` (mod `mod`); consecutive matched
// positions differ by at most max_gap (0 = unbounded). DP over end positions.
long long count_in_sequence(const Sequence& seq, const Pattern& pattern, int max_gap,
                            long long mod) {
  const int n = static_cast<int>(seq.size());
  const int m = static_cast<int>(pattern.size());
  std::vector<long long> dp(n, 0);
  for (int j = 0; j < n; ++j) {
    if (itemset_matches(pattern[0], seq[j])) dp[j] = 1;
  }
  for (int k = 1; k < m; ++k) {
    std::vector<long long> nd(n, 0);
    for (int j = 0; j < n; ++j) {
      if (!itemset_matches(pattern[k], seq[j])) continue;
      const int lo = (max_gap > 0) ? std::max(0, j - max_gap) : 0;
      long long s = 0;
      for (int prev = lo; prev < j; ++prev) s = (s + dp[prev]) % mod;
      nd[j] = s;
    }
    dp.swap(nd);
  }
  long long total = 0;
  for (int j = 0; j < n; ++j) total = (total + dp[j]) % mod;
  return total;
}

}  // namespace

SequentialPatternMiner::SequentialPatternMiner(int min_support, int max_length, int max_gap,
                                               int max_total_gap)
    : min_support_(min_support),
      max_length_(max_length),
      max_gap_(max_gap),
      max_total_gap_(max_total_gap) {
  if (min_support <= 0) throw std::invalid_argument("min_support must be positive");
  if (max_length < 0) throw std::invalid_argument("max_length must be non-negative");
  if (max_gap < 0) throw std::invalid_argument("max_gap must be non-negative");
  if (max_total_gap < 0) throw std::invalid_argument("max_total_gap must be non-negative");
}

void SequentialPatternMiner::fit(const std::vector<Sequence>& database) {
  for (const Sequence& seq : database) {
    if (!is_canonical(seq)) {
      throw std::invalid_argument("every itemset must be non-empty and strictly ascending");
    }
  }

  database_ = database;
  patterns_.clear();
  fitted_ = false;

  // Support of a pattern under the current database and gap bound.
  auto support = [this](const Pattern& p) -> int {
    int s = 0;
    for (const Sequence& seq : database_) {
      if (contains_within_gaps(seq, p, max_gap_, max_total_gap_)) ++s;
    }
    return s;
  };

  // All distinct items in the database, ascending.
  std::set<Item> item_set;
  for (const Sequence& seq : database_) {
    for (const Itemset& e : seq) {
      for (Item it : e) item_set.insert(it);
    }
  }
  const std::vector<Item> items(item_set.begin(), item_set.end());

  const bool unbounded = (max_length_ == 0);

  // Level-wise canonical growth. Each frequent pattern is generated exactly
  // once because patterns only ever grow at the end: an i-extension appends an
  // item strictly larger than the last item of the last itemset, and an
  // s-extension appends a brand new itemset. Support (and hence frequency) is
  // anti-monotone under the gap bound, so only frequent patterns are extended.
  std::deque<Pattern> frontier;

  // Frequent 1-patterns.
  for (Item it : items) {
    Pattern p{Itemset{it}};
    int s = support(p);
    if (s >= min_support_) {
      patterns_.push_back(PatternSupport{p, s});
      if (unbounded || total_items(p) < max_length_) frontier.push_back(p);
    }
  }

  while (!frontier.empty()) {
    Pattern base = frontier.front();
    frontier.pop_front();
    const Item last_item = base.back().back();

    // i-extension: grow the last itemset with an item greater than its current
    // maximum (keeps the itemset canonical and avoids duplicate generation).
    for (Item it : items) {
      if (it <= last_item) continue;
      Pattern cand = base;
      cand.back().push_back(it);
      if (!unbounded && total_items(cand) > max_length_) continue;
      int s = support(cand);
      if (s >= min_support_) {
        patterns_.push_back(PatternSupport{cand, s});
        if (unbounded || total_items(cand) < max_length_) frontier.push_back(cand);
      }
    }

    // s-extension: append a new singleton itemset.
    for (Item it : items) {
      Pattern cand = base;
      cand.push_back(Itemset{it});
      if (!unbounded && total_items(cand) > max_length_) continue;
      int s = support(cand);
      if (s >= min_support_) {
        patterns_.push_back(PatternSupport{cand, s});
        if (unbounded || total_items(cand) < max_length_) frontier.push_back(cand);
      }
    }
  }

  fitted_ = true;
}

std::vector<PatternSupport> SequentialPatternMiner::patterns() const {
  if (!fitted_) throw std::logic_error("miner is not fitted");
  return patterns_;
}

std::vector<PatternSupport> SequentialPatternMiner::closed_patterns() const {
  if (!fitted_) throw std::logic_error("miner is not fitted");
  // P is closed iff no *other* frequent pattern Q with the same support has P as
  // a proper sub-pattern. contains(Q, P) treats Q as a sequence and tests the
  // ordinary (gap-free) subsequence relation P subseteq Q; support is
  // anti-monotone, so any absorbing Q necessarily has equal (not greater)
  // support. Compared against the full frequent set, not just extensions.
  std::vector<PatternSupport> result;
  for (const PatternSupport& ps : patterns_) {
    bool closed = true;
    for (const PatternSupport& other : patterns_) {
      if (other.support == ps.support && other.pattern != ps.pattern &&
          contains(other.pattern, ps.pattern)) {
        closed = false;
        break;
      }
    }
    if (closed) result.push_back(ps);
  }
  return result;
}

int SequentialPatternMiner::support_of(const Pattern& pattern) const {
  if (!fitted_) throw std::logic_error("miner is not fitted");
  if (!is_canonical(pattern)) {
    throw std::invalid_argument("every itemset must be non-empty and strictly ascending");
  }
  int s = 0;
  for (const Sequence& seq : database_) {
    if (contains_within_gaps(seq, pattern, max_gap_, max_total_gap_)) ++s;
  }
  return s;
}

std::vector<PatternSupport> SequentialPatternMiner::top_k(int k) const {
  if (!fitted_) throw std::logic_error("miner is not fitted");
  if (k < 0) throw std::invalid_argument("k must be non-negative");
  std::vector<PatternSupport> sorted = patterns_;
  std::sort(sorted.begin(), sorted.end(),
            [](const PatternSupport& a, const PatternSupport& b) {
              if (a.support != b.support) return a.support > b.support;
              return a.pattern < b.pattern;
            });
  if (static_cast<std::size_t>(k) < sorted.size()) sorted.resize(k);
  return sorted;
}

int SequentialPatternMiner::num_sequences() const {
  if (!fitted_) throw std::logic_error("miner is not fitted");
  return static_cast<int>(database_.size());
}

std::size_t SequentialPatternMiner::num_patterns() const {
  if (!fitted_) throw std::logic_error("miner is not fitted");
  return patterns_.size();
}


long long SequentialPatternMiner::count_matches(const Pattern& pattern) const {
  if (!fitted_) throw std::logic_error("miner is not fitted");
  constexpr long long kMod = 1000000007LL;
  for (const Itemset& e : pattern) {
    if (e.size() == 1 && e[0] == WILDCARD) continue;  // wildcard itemset allowed
    if (e.empty()) throw std::invalid_argument("itemset must be non-empty");
    for (std::size_t i = 1; i < e.size(); ++i) {
      if (e[i - 1] >= e[i]) {
        throw std::invalid_argument("itemset must be strictly ascending");
      }
    }
  }
  if (pattern.empty()) return static_cast<long long>(database_.size() % kMod);
  long long total = 0;
  for (const Sequence& seq : database_) {
    total = (total + count_in_sequence(seq, pattern, max_gap_, kMod)) % kMod;
  }
  return total;
}


Sequence tokenize(const std::string&) {
  throw std::logic_error("not implemented");
}

}  // namespace spm
