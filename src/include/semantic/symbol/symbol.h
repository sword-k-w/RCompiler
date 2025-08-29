#pragma once

#include "semantic/type/type.h"
#include "parser/class_declaration.h"

enum SymbolType {
  kFunction, kType, kTrait, kConst, kVariable
};

struct SymbolInfo {
  SymbolType symbol_type_;
  std::shared_ptr<TypeValue> type_value_;
  ASTNode *source_;
};