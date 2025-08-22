#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>

class ConstantItemNode : public ASTNode {
  friend class Printer;
public:
  ConstantItemNode() = delete;
  ConstantItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  TypeNode *type_ = nullptr;
  ExpressionNode *expr_ = nullptr;
};

class AssociatedItemNode : public ASTNode {
  friend class Printer;
public:
  AssociatedItemNode() = delete;
  AssociatedItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ConstantItemNode *constant_item_ = nullptr;
  FunctionNode *function_ = nullptr;
};

class ItemNode : public ASTNode {
  friend class Printer;
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
