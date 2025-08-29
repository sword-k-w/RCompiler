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
  SymbolInfo symbol_info_;
  bool need_calculate_ = false;
private:
  std::shared_ptr<std::string> name_;
};
