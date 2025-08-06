#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>
#include "common/config.h"

enum TokenType {
  kIDENTIFIER_OR_KEYWORD, kCHAR_LITERAL, kSTRING_LITERAL, kRAW_STRING_LITERAL, kBYTE_LITERAL,
  kBYTE_STRING_LITERAL, kRAW_BYTE_STRING_LITERAL, kC_STRING_LITERAL, kRAW_C_STRING_LITERAL,
  kINTEGER_LITERAL, kFLOAT_LITERAL, kPUNCTUATION, kRESERVED_TOKEN
};

struct Token {
  TokenType type;
  std::string lexeme;
};

class Lexer {
public:
  Lexer() = delete;
  explicit Lexer(const std::string &);
  auto Run() -> std::vector<Token> const;
private:
  uint32_t length_;
  std::string input_;
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
  auto CheckRawStringContent(uint32_t) const -> uint32_t;
  auto CheckIdentifierOrKeyWord(uint32_t) const -> uint32_t;
  auto CheckCharLiteral(uint32_t) const -> uint32_t;
  auto CheckStringLiteral(uint32_t) const -> uint32_t;
  auto CheckRawStringLiteral(uint32_t) const -> uint32_t;
  auto CheckByteLiteral(uint32_t) const -> uint32_t;
  auto CheckByteStringLiteral(uint32_t) const -> uint32_t;
  auto CheckRawByteStringLiteral(uint32_t) const -> uint32_t;
};

#endif //LEXER_H
