#ifndef AST_NODE_H
#define AST_NODE_H

class ExpressionNode;

class ASTNode {
public:
  virtual ~ASTNode() = 0;
  // virtual void accept(Visitor *) = 0;
};

#endif //AST_NODE_H
