#ifndef ITEM_H
#define ITEM_H

#include "lexer/lexer.h"
#include "parser/AST_node.h"
#include "parser/module.h"
#include "parser/function.h"
#include "parser/struct.h"
#include "parser/enumeration.h"
#include "parser/trait.h"
#include "parser/implementation.h"

class ConstantItemNode : ASTNode {
public:
  ConstantItemNode() = delete;
  ConstantItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);

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
