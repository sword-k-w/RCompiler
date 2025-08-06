#include "lexer/lexer.h"

auto Lexer::CheckIdentifierOrKeyWord(uint32_t pos) const -> uint32_t {
  if (pos >= length_ || !CheckAsciiAlpha(pos)) {
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

auto Lexer::CheckCharLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos < length_ && input_[pos] == '\'') {
    ++pos;
  } else {
    return start;
  }
  if (CheckAsciiForChar(pos)) {
    ++pos;
  } else if (CheckQuoteEscape(pos)) {
    pos += 2;
  } else {
    uint32_t tmp = CheckAsciiEscape(pos);
    if (tmp != pos) {
      pos = tmp;
    } else {
      return start;
    }
  }
  if (pos < length_ && input_[pos] == '\'') {
    ++pos;
  } else {
    return start;
  }
  return CheckIdentifierOrKeyWord(pos);
}

auto Lexer::CheckStringLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos < length_ && input_[pos] == '\"') {
    ++pos;
  } else {
    return start;
  }
  while (pos < length_) {
    if (CheckAsciiForString(pos)) {
      ++pos;
    } else {
      if (CheckQuoteEscape(pos)) {
        pos += 2;
      } else {
        uint32_t tmp = CheckAsciiEscape(pos);
        if (tmp != pos) {
          pos = tmp;
        } else if (CheckStringContinue(pos)) {
          ++pos;
        } else {
          break;
        }
      }
    }
  }
  if (pos < length_ && input_[pos] == '\"') {
    ++pos;
  } else {
    return start;
  }
  return CheckIdentifierOrKeyWord(pos);
}

auto Lexer::CheckRawStringLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos < length_ && input_[pos] == 'r') {
    ++pos;
  } else {
    return start;
  }
  uint32_t tmp = CheckRawStringContent(pos);
  if (tmp != pos) {
    pos = tmp;
  }
  return CheckIdentifierOrKeyWord(pos);
}

auto Lexer::CheckByteLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos + 1 < length_ && input_[pos] == 'b' && input_[pos + 1] == '\'') {
    pos += 2;
  } else {
    return start;
  }
  if (CheckAsciiForChar(pos)) {
    ++pos;
  } else {
    uint32_t tmp = CheckByteEscape(pos);
    if (tmp != pos) {
      pos = tmp;
    } else {
      return start;
    }
  }
  return CheckIdentifierOrKeyWord(pos);
}

auto Lexer::CheckByteStringLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos + 1 < length_ && input_[pos] == 'b' && input_[pos + 1] == '\"') {
    ++pos;
  } else {
    return start;
  }
  while (pos < length_) {
    if (CheckAsciiForString(pos)) {
      ++pos;
    } else {
      uint32_t tmp = CheckByteEscape(pos);
      if (tmp != pos) {
        pos = tmp;
      } else if (CheckStringContinue(pos)) {
        ++pos;
      } else {
        break;
      }
    }
  }
  if (pos < length_ && input_[pos] == '\"') {
    ++pos;
  } else {
    return start;
  }
  return CheckIdentifierOrKeyWord(pos);
}

auto Lexer::CheckRawByteStringLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos + 1 < length_ && input_[pos] == 'b' && input_[pos + 1] == 'r') {
    pos += 2;
  } else {
    return start;
  }
  uint32_t tmp = CheckRawStringContent(pos);
  if (tmp != pos) {
    pos = tmp;
  }
  return CheckIdentifierOrKeyWord(pos);
}
