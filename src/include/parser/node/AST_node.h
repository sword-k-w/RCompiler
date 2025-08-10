#ifndef AST_NODE_H
#define AST_NODE_H

#include "parser/error.h"

class ExpressionNode;

class ItemNode;

class ASTNode {
public:
  ASTNode() = delete;
  ASTNode(const std::string_view &);
  void CheckLength(const uint32_t &, const uint32_t &) const;
  virtual ~ASTNode() = 0;
  // virtual void accept(Visitor *) = 0;
private:
  const std::string name_;
};

#endif //AST_NODE_H
