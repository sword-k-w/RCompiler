#include "semantic/scope/scope.h"
#include "common/error.h"

Scope::Scope(std::shared_ptr<Scope> parent) : parent_(std::move(parent)) {}

void Scope::AddTypeName(const std::string &name, const SymbolInfo &symbol_info) {
  if (type_namespace_.find(name) != type_namespace_.end()) {
    throw Error("FirstChecker : same type name " + name);
  }
  type_namespace_[name] = symbol_info;
}

void Scope::AddValueName(const std::string &name, const SymbolInfo &symbol_info) {
  if (value_namespace_.find(name) != value_namespace_.end()) {
    throw Error("FirstChecker : same value name " + name);
  }
  value_namespace_[name] = symbol_info;
}

