#pragma once

#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include "semantic/scope/scope.h"
#include "visitor/base/visitor_base.h"
#include "semantic/symbol/symbol.h"

class ASTNode {
  friend class FirstChecker;
  friend class SecondChecker;
public:
  ASTNode() = delete;
  explicit ASTNode(const std::string_view &);
  void CheckLength(const uint32_t &, const uint32_t &) const;
  virtual void Accept(VisitorBase *) = 0;
  virtual ~ASTNode() = default;
protected:
  std::shared_ptr<Scope> scope_;
  SymbolType symbol_type_;
  std::shared_ptr<TypeValue> type_value_;
  bool need_calculate_ = false;
  std::shared_ptr<std::string> expect_type_;
private:
  std::shared_ptr<std::string> name_;
};
