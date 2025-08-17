#ifndef ITEM_H
#define ITEM_H

#include "lexer/lexer.h"
#include "parser/node/terminal.h"
#include "parser/node/AST_node.h"
#include "parser/node/function.h"
#include "parser/node/struct.h"
#include "parser/node/enumeration.h"
#include "parser/node/trait.h"
#include "parser/node/implementation.h"

class ConstantItemNode : public ASTNode {
public:
  ConstantItemNode() = delete;
  ConstantItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  TypeNode *type_ = nullptr;
  ExpressionNode *expr_ = nullptr;
};

class AsscociatedItemNode : public ASTNode {
public:
  AsscociatedItemNode() = delete;
  AsscociatedItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ConstantItemNode *constant_item_ = nullptr;
  FunctionNode *function_ = nullptr;
};

class ItemNode : public ASTNode {
public:
  ItemNode() = delete;
  ItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);

private:
  FunctionNode *function_ = nullptr;
  StructNode *struct_ = nullptr;
  EnumerationNode *enumeration_ = nullptr;
  ConstantItemNode *constant_item_ = nullptr;
  TraitNode *trait_ = nullptr;
  ImplementationNode *implementation_ = nullptr;
};

#endif //ITEM_H
