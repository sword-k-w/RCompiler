#include "semantic/scope/scope.h"
#include "common/error.h"
#include "common/config.h"

Scope::Scope(std::shared_ptr<Scope> parent, const std::string &name)
  : parent_(std::move(parent)), name_(std::make_shared<std::string>(name)) {}

void Scope::AddTypeName(const std::string &name, const SymbolInfo &symbol_info) {
  if (type_namespace_.find(name) != type_namespace_.end()) {
    throw Error("FirstChecker : same type name " + name);
  }
  for (uint32_t i = 0; i < 6; ++i) {
    if (name == kBuiltinType[i]) {
      throw Error("FirstChecker : type name" + name + " leading to conflict");
    }
  }
  type_namespace_[name] = symbol_info;
}

void Scope::AddValueName(const std::string &name, const SymbolInfo &symbol_info) {
  if (value_namespace_.find(name) != value_namespace_.end()) {
    throw Error("FirstChecker : same value name " + name);
  }
  value_namespace_[name] = symbol_info;
}

