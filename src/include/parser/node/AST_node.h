#pragma once

#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include "semantic/scope/scope.h"
#include "visitor/base/visitor_base.h"

class ASTNode {
  friend class FirstChecker;
  friend class SecondChecker;
  friend class ThirdChecker;
public:
  ASTNode() = delete;
  explicit ASTNode(const std::string_view &);
  void CheckLength(const uint32_t &, const uint32_t &) const;
  virtual void Accept(VisitorBase *) = 0;
  virtual ~ASTNode() = default;
protected:
  std::shared_ptr<Scope> scope_;
  std::shared_ptr<ConstValue> const_value_;
  std::shared_ptr<Type> type_info_;
  bool need_calculate_ = false;

  StructNode *outside_struct_;
private:
  std::shared_ptr<std::string> name_;
};
