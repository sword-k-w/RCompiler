#include "lexer/lexer.h"

auto Lexer::CheckRawStringContent(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  uint32_t count = 0; // count of #
  while (pos < length_ && input_[pos] != '\"') {
    if (input_[pos] != '#') {
      return start;
    }
    ++pos;
    ++count;
  }
  if (pos >= length_) {
    return start;
  }
  uint32_t end = pos;
  while (CheckAsciiForRaw(end)) {
    ++end;
  }
  for (uint32_t i = end - 1; i > pos + count; --i) {
    bool flag = true;
    for (uint32_t j = i - count + 1; j <= i; ++j) {
      flag &= (input_[pos] == '#');
    }
    flag &= (input_[i - count] == '\"');
    if (flag) {
      return i + 1;
    }
  }
  return start;
}

