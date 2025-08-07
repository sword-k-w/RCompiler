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

auto Lexer::CheckRawCStringContent(uint32_t pos) const -> uint32_t {
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
  while (CheckAsciiForRaw(end) && input_[end] != '\0') {
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

auto Lexer::CheckDecLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (CheckDecDigit(pos)) {
    ++pos;
  } else {
    return start;
  }
  while (CheckDecDigit(pos) || input_[pos] == '_') {
    ++pos;
  }
  return pos;
}

auto Lexer::CheckBinLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos + 1 < length_ && input_[pos] == '0' && input_[pos + 1] == 'b') {
    pos += 2;
  } else {
    return start;
  }
  bool flag = false;
  while (CheckBinDigit(pos) || input_[pos] == '_') {
    flag |= CheckBinDigit(pos);
    ++pos;
  }
  if (flag) {
    return pos;
  }
  return start;
}

auto Lexer::CheckOctLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos + 1 < length_ && input_[pos] == '0' && input_[pos + 1] == 'o') {
    pos += 2;
  } else {
    return start;
  }
  bool flag = false;
  while (CheckOctDigit(pos) || input_[pos] == '_') {
    flag |= CheckOctDigit(pos);
    ++pos;
  }
  if (flag) {
    return pos;
  }
  return start;
}

auto Lexer::CheckHexLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos + 1 < length_ && input_[pos] == '0' && input_[pos + 1] == 'x') {
    pos += 2;
  } else {
    return start;
  }
  bool flag = false;
  while (CheckHexDigit(pos) || input_[pos] == '_') {
    flag |= CheckHexDigit(pos);
    ++pos;
  }
  if (flag) {
    return pos;
  }
  return start;
}

auto Lexer::CheckSuffixNoE(uint32_t pos) const -> uint32_t {
  if (pos >= length_ || !CheckAsciiAlpha(pos) || input_[pos] == 'e' || input_[pos] == 'E') {
    return pos;
  }
  ++pos;
  while (pos < length_) {
    if (CheckAsciiAlpha(pos) || CheckAsciiDigit(pos) || input_[pos] == '_') {
      ++pos;
    }
  }
  return pos;
}

auto Lexer::CheckReservedGuardedStringLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  while (pos < length_ && input_[pos] == '#') {
    ++pos;
  }
  if (pos == start) {
    return pos;
  }
  uint32_t tmp = pos;
  pos = CheckStringLiteral(pos);
  if (tmp == pos) {
    return start;
  }
  return pos;
}

auto Lexer::CheckReservedNumber(uint32_t pos) const -> uint32_t {
  uint32_t tmp = CheckBinLiteral(pos);
  if (tmp != pos && tmp < length_ && input_[pos] >= '2' && input_[pos] <= '9') {
    return pos + 1;
  }
  tmp = CheckOctLiteral(pos);
  if (tmp != pos && tmp < length_ && input_[pos] >= '8' && input_[pos] <= '9') {
    return pos + 1;
  }
  tmp = std::max({CheckBinLiteral(pos), CheckOctLiteral(pos), CheckHexLiteral(pos)});
  if (tmp != pos && tmp < length_ && input_[tmp] == '.') {
    ++tmp;
    if (tmp >= length_ || (input_[tmp] != '.' && input_[tmp] != '_' && !CheckAsciiAlpha(tmp))) {
      return tmp;
    }
  }
  tmp = pos;
  if (tmp < length_ && input_[tmp] == '0') {
    ++tmp;
    if (tmp + 1 < length_ && input_[tmp] == 'b') {
      ++tmp;
      while (tmp < length_ && input_[tmp] == '_') {
        ++tmp;
      }
      if (tmp == length_ || !CheckBinDigit(tmp)) {
        return tmp;
      }
    }
    if (tmp + 1 < length_ && input_[tmp] == 'o') {
      ++tmp;
      while (tmp < length_ && input_[tmp] == '_') {
        ++tmp;
      }
      if (tmp == length_ || !CheckOctDigit(tmp)) {
        return tmp;
      }
    }
    if (tmp + 1 < length_ && input_[tmp] == 'x') {
      ++tmp;
      while (tmp < length_ && input_[tmp] == '_') {
        ++tmp;
      }
      if (tmp == length_ || !CheckHexDigit(tmp)) {
        return tmp;
      }
    }
  }
  return pos;
}