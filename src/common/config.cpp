#include "common/config.h"

#include <parser/node/expression.h>

bool IsKeyword(const std::string_view &lexeme) {
  for (uint32_t i = 0; i < 38; ++i) {
    if (lexeme == kSTRICT_KEYWORDS[i]) {
      return true;
    }
  }
  for (uint32_t i = 0; i < 14; ++i) {
    if (lexeme == kRESERVERD_KEYWORDS[i]) {
      return true;
    }
  }
  return false;
}

bool IsIntegerType(const std::string &name) {
  for (uint32_t i = 0; i < 4; ++i) {
    if (name == kBuiltinType[i]) {
      return true;
    }
  }
  return false;
}
