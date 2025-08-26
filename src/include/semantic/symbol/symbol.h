#pragma once

#include "semantic/type/type.h"

enum SymbolType {
  kFunction, kType, kTrait, kConst, kVariable
};

struct SymbolInfo {
  SymbolType symbol_type_;
  std::shared_ptr<Type> type_;
};