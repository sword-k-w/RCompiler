#include "lexer/lexer.h"

auto Lexer::CheckWhitespace(const uint32_t &pos) const -> bool {
  return pos < length_ && (input_[pos] == '\t' || input_[pos] == '\n' || input_[pos] == '\xB' || input_[pos] == '\xC'
    || input_[pos] == '\r' || input_[pos] == ' ');
}

auto Lexer::CheckLineComment(uint32_t pos) const -> uint32_t {
  if (pos + 1 < length_ && input_[pos] == '/' && input_[pos + 1] == '/') {
    pos += 2;
    while (pos < length_ && input_[pos] != '\n') {
      ++pos;
    }
  }
  return pos;
}

auto Lexer::CheckBlockComment(uint32_t pos) const -> uint32_t {
  uint32_t cnt = 0;
  uint32_t start = pos;
  if (CheckCommentLeft(pos)) {
    ++cnt;
  }
  if (cnt) {
    pos += 2;
    while (cnt && pos < length_) {
      if (CheckCommentLeft(pos)) {
        ++cnt;
        pos += 2;
      } else if (CheckCommentRight(pos)) {
        --cnt;
        pos += 2;
      } else {
        ++pos;
      }
    }
    if (cnt) {
      return start;
    }
  }
  return pos;
}
