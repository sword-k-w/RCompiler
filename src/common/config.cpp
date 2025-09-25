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
  if (name == "$" || name == "@") {
    return true;
  }
  return false;
}

bool IsSignedIntegerType(const std::string &name) {
  return name == "$" || name == "@" || name == "i32" || name == "isize";
}

std::string MergeLeafType(const std::string &name1, const std::string &name2) {
  if (name1 == "$") {
    if (IsIntegerType(name2)) {
      return name2;
    }
  } else if (name2 == "$") {
    if (IsIntegerType(name1)) {
      return name1;
    }
  } else if (name1 == "@") {
    if (IsSignedIntegerType(name2)) {
      return name2;
    }
  } else if (name2 == "@") {
    if (!IsSignedIntegerType(name1)) {
      return name2;
    }
  } else if (name1 == name2) {
    return name1;
  }
  return "";
}

bool IsBuiltinFunction(const std::string &name) {
  for (uint32_t i = 0; i < 7; ++i) {
    if (kBuiltinFunction[i] == name) {
      return true;
    }
  }
  return false;
}