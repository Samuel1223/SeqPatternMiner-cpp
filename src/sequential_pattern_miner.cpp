#include "spm/sequential_pattern_miner.hpp"

#include <stdexcept>

namespace spm {

namespace {
[[noreturn]] void not_implemented() { throw std::logic_error("not implemented"); }
}  // namespace

SequentialPatternMiner::SequentialPatternMiner(int min_support, int max_length)
    : min_support_(min_support), max_length_(max_length) {
  if (min_support <= 0) throw std::invalid_argument("min_support must be positive");
  if (max_length < 0) throw std::invalid_argument("max_length must be non-negative");
}

void SequentialPatternMiner::fit(const std::vector<Sequence>&) { not_implemented(); }

std::vector<PatternSupport> SequentialPatternMiner::patterns() const { not_implemented(); }

int SequentialPatternMiner::support_of(const Pattern&) const { not_implemented(); }

std::vector<PatternSupport> SequentialPatternMiner::top_k(int) const { not_implemented(); }

int SequentialPatternMiner::num_sequences() const { not_implemented(); }

std::size_t SequentialPatternMiner::num_patterns() const { not_implemented(); }

}  // namespace spm
