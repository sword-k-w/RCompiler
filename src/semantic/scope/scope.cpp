#include "semantic/scope/scope.h"
#include "common/error.h"
#include "common/config.h"
#include <cassert>
#include <parser/node/pattern.h>

Scope::Scope(std::shared_ptr<Scope> parent, const std::string &name)
  : parent_(std::move(parent)), name_(std::make_shared<std::string>(name)) {}

void Scope::AddTypeName(const std::string &name, ASTNode *source) {
  if (type_namespace_.find(name) != type_namespace_.end()) {
    throw Error("FirstChecker : same type name " + name);
  }
  for (uint32_t i = 0; i < 7; ++i) {
    if (name == kBuiltinType[i]) {
      throw Error("FirstChecker : type name" + name + " leading to conflict");
    }
  }
  type_namespace_[name] = source;
}

void Scope::AddValueName(const std::string &name, ASTNode *source, bool shadow) {
  if (value_namespace_.find(name) != value_namespace_.end()) {
    if (!shadow) {
      throw Error("FirstChecker : same value name " + name);
    }
    assert(dynamic_cast<IdentifierPatternNode *>(source) != nullptr);
  }
  value_namespace_[name] = source;
}

ASTNode *Scope::FindTypeName(const std::string &name) {
  auto it = type_namespace_.find(name);
  if (it == type_namespace_.end()) {
    if (parent_ == nullptr) {
      return nullptr;
    }
    return parent_->FindTypeName(name);
  }
  return it->second;
}

ASTNode *Scope::FindValueName(const std::string &name) {
  auto it = value_namespace_.find(name);
  if (it == value_namespace_.end()) {
    if (parent_ == nullptr) {
      return nullptr;
    }
    return parent_->FindValueName(name);
  }
  return it->second;
}


