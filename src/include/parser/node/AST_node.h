#pragma once

#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include "semantic/scope/scope.h"
#include "visitor/base/visitor_base.h"

class ASTNode {
public:
  ASTNode() = delete;
  explicit ASTNode(const std::string_view &);
  void CheckLength(const uint32_t &, const uint32_t &) const;
  virtual void Accept(VisitorBase *) = 0;
private:
  std::shared_ptr<Scope> scope_;
};
