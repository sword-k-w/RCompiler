#ifndef LEXER_H
#define LEXER_H

#include <boost/xpressive/xpressive.hpp>
#include <string>
#include "common/config.h"

using namespace boost::xpressive;

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
  uint32_t pos_;
  uint32_t length_;
  std::string input_;

};

#endif //LEXER_H
