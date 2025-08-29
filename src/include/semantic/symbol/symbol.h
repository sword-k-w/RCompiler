#pragma once

#include "semantic/type/type.h"
#include "parser/class_declaration.h"

enum SymbolType {
  kFunction, kType, kTrait, kConst, kVariable
};

struct SymbolInfo {
  SymbolType symbol_type_;
  std::shared_ptr<Type> type_;
  ASTNode *source_;
};