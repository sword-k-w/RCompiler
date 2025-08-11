#ifndef ITEM_H
#define ITEM_H

#include "lexer/lexer.h"
#include "parser/node/terminal.h"
#include "parser/node/AST_node.h"
#include "parser/node/module.h"
#include "parser/node/function.h"
#include "parser/node/struct.h"
#include "parser/node/enumeration.h"
#include "parser/node/trait.h"
#include "parser/node/implementation.h"

class ConstantItemNode : ASTNode {
public:
  ConstantItemNode() = delete;
  ConstantItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  bool underscore_ = false;
  TypeNode *type_ = nullptr;
  ExpressionNode *expr_ = nullptr;
};

class ItemNode : ASTNode {
public:
  ItemNode() = delete;
  ItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);

private:
  ModuleNode *module_ = nullptr;
  FunctionNode *function_ = nullptr;
  StructNode *struct_ = nullptr;
  EnumerationNode *enumeration_ = nullptr;
  ConstantItemNode *constant_item_ = nullptr;
  TraitNode *trait_ = nullptr;
  ImplementationNode *implementation_ = nullptr;
};

#endif //ITEM_H
