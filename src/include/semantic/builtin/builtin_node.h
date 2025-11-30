#pragma once

#include "parser/class_declaration.h"
#include "parser/node/AST_node.h"

class BuiltinFunctionNode : public ASTNode {
  friend class ThirdChecker;
  friend class IRGenerator;
public:
  BuiltinFunctionNode() = delete;
  explicit BuiltinFunctionNode(const std::string &);
  void Accept(VisitorBase *) override;
private:
  std::string function_name_;
};