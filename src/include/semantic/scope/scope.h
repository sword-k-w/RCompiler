#pragma once

#include "semantic/type/type.h"
#include "semantic/symbol/symbol.h"

class Scope {
public:
  Scope() = delete;
  explicit Scope(std::shared_ptr<Scope>);
private:
  std::shared_ptr<Scope> parent_;
  std::map<std::string, SymbolInfo> type_namespace_;
  std::map<std::string, SymbolInfo> value_namespace_;
};
