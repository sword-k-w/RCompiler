#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

// Lightweight bitset backed by std::vector<uint64_t>. Avoids the proxy
// and performance issues of std::vector<bool>.
class BitSet {
public:
  BitSet() = default;

  void Resize(uint32_t n) {
    bits_.resize((static_cast<uint64_t>(n) + 63) >> 6, 0);
  }

  void Set(uint32_t idx) {
    bits_[idx >> 6] |= (1ULL << (idx & 63));
  }

  void ClearBit(uint32_t idx) {
    bits_[idx >> 6] &= ~(1ULL << (idx & 63));
  }

  bool Test(uint32_t idx) const {
    uint32_t word = idx >> 6;
    if (word >= bits_.size()) return false;
    return bits_[word] & (1ULL << (idx & 63));
  }

  void Clear() {
    std::fill(bits_.begin(), bits_.end(), 0);
  }

  bool operator==(const BitSet &o) const {
    return bits_ == o.bits_;
  }
  bool operator!=(const BitSet &o) const {
    return bits_ != o.bits_;
  }

  // this |= o
  void Union(const BitSet &o) {
    for (size_t i = 0; i < bits_.size(); ++i) {
      bits_[i] |= o.bits_[i];
    }
  }

  // this = o & ~except
  void CopyAndNot(const BitSet &o, const BitSet &except) {
    for (size_t i = 0; i < bits_.size(); ++i) {
      bits_[i] = o.bits_[i] & ~except.bits_[i];
    }
  }

  // Full copy
  void Copy(const BitSet &o) {
    bits_ = o.bits_;
  }

  template <typename F>
  void ForEach(F &&f) const {
    for (size_t i = 0; i < bits_.size(); ++i) {
      if (bits_[i] == 0) continue;
      uint64_t word = bits_[i];
      uint32_t base = i << 6;
      while (word) {
        int tz = __builtin_ctzll(word);
        f(base + tz);
        word &= word - 1;
      }
    }
  }

  std::vector<uint64_t> bits_;
};
