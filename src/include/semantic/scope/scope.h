#pragma once

#include "semantic/type/type.h"
#include "semantic/symbol/symbol.h"

class Scope {
public:
  Scope() = delete;
  Scope(std::shared_ptr<Scope>, const std::string &);
  void AddTypeName(const std::string &, const SymbolInfo &);
  void AddValueName(const std::string &, const SymbolInfo &);
  std::shared_ptr<std::string> name_;
private:
  std::shared_ptr<Scope> parent_;
  std::map<std::string, SymbolInfo> type_namespace_;
  std::map<std::string, SymbolInfo> value_namespace_;
};
