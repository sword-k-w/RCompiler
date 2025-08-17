#include "lexer/lexer.h"

auto Lexer::CheckIdentifierOrKeyWord(uint32_t pos) const -> uint32_t {
  if (pos >= length_ || !CheckAsciiAlpha(pos)) {
    return pos;
  }
  ++pos;
  while (pos < length_ && (CheckAsciiAlpha(pos) || CheckAsciiDigit(pos) || input_[pos] == '_')) {
    ++pos;
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

auto Lexer::CheckCStringLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos + 1 < length_ && input_[pos] == 'c' && input_[pos + 1] == '\"') {
    ++pos;
  } else {
    return start;
  }
  while (pos < length_) {
    if (input_[pos] != '\"' && input_[pos] != '\\' && input_[pos] != '\r' && input_[pos] != '\0') {
      ++pos;
    } else {
      uint32_t tmp = CheckByteEscape(pos);
      if (tmp != pos && (tmp != pos + 2 || input_[pos + 1] != '0') &&
        (tmp != pos + 4 || input_[pos + 2] != '0' || input_[pos + 3] != '0')) {
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

auto Lexer::CheckRawCStringLiteral(uint32_t pos) const -> uint32_t {
  uint32_t start = pos;
  if (pos + 1 < length_ && input_[pos] == 'c' && input_[pos + 1] == 'r') {
    pos += 2;
  } else {
    return start;
  }
  uint32_t tmp = CheckRawCStringContent(pos);
  if (tmp != pos) {
    pos = tmp;
  }
  return CheckIdentifierOrKeyWord(pos);
}

auto Lexer::CheckIntegerLiteral(uint32_t pos) const -> uint32_t {
  uint32_t tmp = std::max({CheckBinLiteral(pos), CheckOctLiteral(pos), CheckHexLiteral(pos)});
  if (tmp != pos) {
    return CheckSuffixNoE(tmp);
  }
  tmp = CheckDecLiteral(pos);
  if (tmp != pos) {
    return CheckSuffixNoE(tmp);
  }
  return pos;
}

auto Lexer::CheckPunctuation(uint32_t pos) const -> uint32_t {
  if (pos + 2 < length_) {
    if (input_[pos] == '<' && input_[pos + 1] == '<' && input_[pos + 2] == '=') {
      return pos + 3;
    }
    if (input_[pos] == '>' && input_[pos + 1] == '>' && input_[pos + 2] == '=') {
      return pos + 3;
    }
    if (input_[pos] == '.' && input_[pos + 1] == '.' && input_[pos + 2] == '=') {
      return pos + 3;
    }
    if (input_[pos] == '.' && input_[pos + 1] == '.' && input_[pos + 2] == '.') {
      return pos + 3;
    }
  }
  if (pos + 1 < length_) {
    if (input_[pos] == '<' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '<' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '=' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '!' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '>' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '&' && input_[pos + 1] == '&') {
      return pos + 2;
    }
    if (input_[pos] == '|' && input_[pos + 1] == '|') {
      return pos + 2;
    }
    if (input_[pos] == '<' && input_[pos + 1] == '<') {
      return pos + 2;
    }
    if (input_[pos] == '>' && input_[pos + 1] == '>') {
      return pos + 2;
    }
    if (input_[pos] == '+' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '-' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '*' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '/' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '%' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '^' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '&' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '|' && input_[pos + 1] == '=') {
      return pos + 2;
    }
    if (input_[pos] == '.' && input_[pos + 1] == '.') {
      return pos + 2;
    }
    if (input_[pos] == ':' && input_[pos + 1] == ':') {
      return pos + 2;
    }
    if (input_[pos] == '-' && input_[pos + 1] == '>') {
      return pos + 2;
    }
    if (input_[pos] == '<' && input_[pos + 1] == '-') {
      return pos + 2;
    }
    if (input_[pos] == '=' && input_[pos + 1] == '>') {
      return pos + 2;
    }
  }
  if (pos < length_) {
    if (input_[pos] == '=' || input_[pos] == '<' || input_[pos] == '>' || input_[pos] == '!' || input_[pos] == '~'
      || input_[pos] == '+' || input_[pos] == '-' || input_[pos] == '*' || input_[pos] == '/' || input_[pos] == '%'
      || input_[pos] == '^' || input_[pos] == '&' || input_[pos] == '|' || input_[pos] == '@' || input_[pos] == '.'
      || input_[pos] == ',' || input_[pos] == ';' || input_[pos] == ':' || input_[pos] == '#' || input_[pos] == '$'
      || input_[pos] == '?' || input_[pos] == '_' || input_[pos] == '{' || input_[pos] == '}' || input_[pos] == '['
      || input_[pos] == ']' || input_[pos] == '(' || input_[pos] == ')') {
      return pos + 1;
    }
  }
  return pos;
}