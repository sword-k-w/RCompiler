#include "lexer/lexer.h"

auto Lexer::CheckAsciiAlpha(const uint32_t &pos) const -> bool {
  return pos < length_ && (input_[pos] >= 'a' && input_[pos] <= 'z') || (input_[pos] >= 'A' && input_[pos] <= 'Z');
}

auto Lexer::CheckAsciiDigit(const uint32_t &pos) const -> bool {
  return pos < length_ && input_[pos] >= '0' && input_[pos] <= '9';
}

auto Lexer::CheckQuoteEscape(const uint32_t &pos) const -> bool {
  return pos + 1 < length_ && input_[pos] == '\\' && (input_[pos + 1] == '\'' || input_[pos + 1] == '\"');
}

auto Lexer::CheckAsciiEscape(const uint32_t &pos) const -> uint32_t {
  if (pos + 1 < length_ && input_[pos] == '\\' && (input_[pos + 1] == 'n' || input_[pos + 1] == 'r'
    || input_[pos + 1] == 't' || input_[pos + 1] == '\\' || input_[pos + 1] == '0')) {
    return pos + 2;
    }
  if (pos + 3 < length_ && input_[pos] == '\\' && input_[pos + 1] == 'x'
    && CheckOctDigit(pos + 2) && CheckHexDigit(pos + 3)) {
    return pos + 4;
    }
  return pos;
}

auto Lexer::CheckByteEscape(const uint32_t &pos) const -> uint32_t {
  if (pos + 1 < length_ && input_[pos] == '\\' && (input_[pos + 1] == 'n' || input_[pos + 1] == 'r'
    || input_[pos + 1] == 't' || input_[pos + 1] == '\\' || input_[pos + 1] == '0' ||
    input_[pos + 1] == '\'' || input_[pos + 1] == '\"')) {
    return pos + 2;
    }
  if (pos + 3 < length_ && input_[pos] == '\\' && input_[pos + 1] == 'x'
    && CheckOctDigit(pos + 2) && CheckHexDigit(pos + 3)) {
    return pos + 4;
    }
  return pos;
}

auto Lexer::CheckBinDigit(const uint32_t &pos) const -> bool {
  return pos < length_ && (input_[pos] == '0' || input_[pos] == '1');
}

auto Lexer::CheckOctDigit(const uint32_t &pos) const -> bool {
  return pos < length_ && input_[pos] >= '0' && input_[pos] < '8';
}

auto Lexer::CheckDecDigit(const uint32_t &pos) const -> bool {
  return pos < length_ && input_[pos] <= '0' && input_[pos] <= '9';
}

auto Lexer::CheckHexDigit(const uint32_t &pos) const -> bool {
  return pos < length_ && ((input_[pos] >= '0' && input_[pos] <= '9') || (input_[pos] >= 'a' && input_[pos] <= 'f')
    || (input_[pos] >= 'A' && input_[pos] <= 'F'));
}

auto Lexer::CheckStringContinue(const uint32_t &pos) const -> bool {
  return input_[pos] == '\\' || input_[pos] == '\n';
}

auto Lexer::CheckAsciiForChar(const uint32_t &pos) const -> bool {
  return pos < length_ && input_[pos] != '\'' && input_[pos] != '\\' && input_[pos] != '\n' && input_[pos] != '\r'
    && input_[pos] != '\t';
}

auto Lexer::CheckAsciiForString(const uint32_t &pos) const -> bool {
  return pos < length_ && input_[pos] != '\"' && input_[pos] != '\\' && input_[pos] != '\r';
}

auto Lexer::CheckAsciiForRaw(const uint32_t &pos) const -> bool {
  return pos < length_ && input_[pos] != '\r';
}

auto Lexer::CheckCommentLeft(const uint32_t &pos) const -> bool {
  return pos + 1 < length_ && input_[pos] == '/' && input_[pos + 1] == '*';
}

auto Lexer::CheckCommentRight(const uint32_t &pos) const -> bool {
  return pos + 1 < length_ && input_[pos] == '*' && input_[pos + 1] == '/';
}

