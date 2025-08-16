#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>
#include <iostream>
#include <cassert>
#include "common/config.h"

enum TokenType {
  kIDENTIFIER_OR_KEYWORD, kCHAR_LITERAL, kSTRING_LITERAL, kRAW_STRING_LITERAL, kC_STRING_LITERAL,
  kRAW_C_STRING_LITERAL, kINTEGER_LITERAL, kPUNCTUATION
};

constexpr TokenType kTokenTypes[kTokenTypeCount] = {
  kIDENTIFIER_OR_KEYWORD, kCHAR_LITERAL, kSTRING_LITERAL, kRAW_STRING_LITERAL, kC_STRING_LITERAL,
  kRAW_C_STRING_LITERAL, kINTEGER_LITERAL, kPUNCTUATION
};

struct Token {
  TokenType type;
  std::string lexeme;
  void Print(std::ostream &os) const {
    os << "(";
    if (type == kIDENTIFIER_OR_KEYWORD) {
      os << "IDENTIFIER_OR_KEYWORD";
    } else if (type == kCHAR_LITERAL) {
      os << "CHAR_LITERAL";
    } else if (type == kSTRING_LITERAL) {
      os << "STRING_LITERAL";
    } else if (type == kRAW_STRING_LITERAL) {
      os << "RAW_STRING_LITERAL";
    } else if (type == kC_STRING_LITERAL) {
      os << "C_STRING_LITERAL";
    } else if (type == kRAW_C_STRING_LITERAL) {
      os << "RAW_C_STRING_LITERAL";
    } else if (type == kINTEGER_LITERAL) {
      os << "INTEGER_LITERAL";
    } else if (type == kPUNCTUATION) {
      os << "PUNCTUATION";
    } else {
      assert(0);
    }
    os << " " << lexeme << ")";
  }
};

class Lexer {
public:
  Lexer() = delete;
  explicit Lexer(const std::string &);
  auto Run() -> std::vector<Token> const;
private:
  uint32_t length_;
  std::string input_;
  auto CheckWhitespace(const uint32_t &) const -> bool;
  auto CheckLineComment(uint32_t) const -> uint32_t;
  auto CheckBlockComment(uint32_t) const -> uint32_t;
  auto CheckAsciiAlpha(const uint32_t &) const -> bool;
  auto CheckAsciiDigit(const uint32_t &) const -> bool;
  auto CheckQuoteEscape(const uint32_t &) const -> bool;
  auto CheckAsciiEscape(const uint32_t &) const -> uint32_t;
  auto CheckByteEscape(const uint32_t &) const -> uint32_t;
  auto CheckBinDigit(const uint32_t &) const -> bool;
  auto CheckDecDigit(const uint32_t &) const -> bool;
  auto CheckOctDigit(const uint32_t &) const -> bool;
  auto CheckHexDigit(const uint32_t &) const -> bool;
  auto CheckStringContinue(const uint32_t &) const -> bool;
  auto CheckAsciiForChar(const uint32_t &) const -> bool;
  auto CheckAsciiForString(const uint32_t &) const -> bool;
  auto CheckAsciiForRaw(const uint32_t &) const -> bool;
  auto CheckCommentLeft(const uint32_t &) const -> bool;
  auto CheckCommentRight(const uint32_t &) const -> bool;
  auto CheckRawStringContent(uint32_t) const -> uint32_t;
  auto CheckRawCStringContent(uint32_t) const -> uint32_t;
  auto CheckDecLiteral(uint32_t) const -> uint32_t;
  auto CheckBinLiteral(uint32_t) const -> uint32_t;
  auto CheckOctLiteral(uint32_t) const -> uint32_t;
  auto CheckHexLiteral(uint32_t) const -> uint32_t;
  auto CheckSuffixNoE(uint32_t) const -> uint32_t;
  auto CheckReservedGuardedStringLiteral(uint32_t) const -> uint32_t;
  auto CheckReservedNumber(uint32_t) const -> uint32_t;
  auto CheckIdentifierOrKeyWord(uint32_t) const -> uint32_t;
  auto CheckCharLiteral(uint32_t) const -> uint32_t;
  auto CheckStringLiteral(uint32_t) const -> uint32_t;
  auto CheckRawStringLiteral(uint32_t) const -> uint32_t;
  auto CheckCStringLiteral(uint32_t) const -> uint32_t;
  auto CheckRawCStringLiteral(uint32_t) const -> uint32_t;
  auto CheckIntegerLiteral(uint32_t) const -> uint32_t;
  auto CheckPunctuation(uint32_t) const -> uint32_t;

  using CheckTokenFuncType = auto(Lexer::*)(uint32_t) const -> uint32_t;
  const CheckTokenFuncType check_token_func_[kTokenTypeCount] = {
    &Lexer::CheckIdentifierOrKeyWord, &Lexer::CheckCharLiteral, &Lexer::CheckStringLiteral, &Lexer::CheckRawStringLiteral,
    &Lexer::CheckCStringLiteral, &Lexer::CheckRawCStringLiteral, &Lexer::CheckIntegerLiteral, &Lexer::CheckPunctuation
  };
};

#endif //LEXER_H
