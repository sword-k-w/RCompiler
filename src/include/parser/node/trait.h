#ifndef TRAIT_H
#define TRAIT_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/item.h"

class TypeParamBoundsNode : public ASTNode {
public:
  TypeParamBoundsNode() = delete;
  TypeParamBoundsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

class TraitNode : public ASTNode {
public:
  TraitNode() = delete;
  TraitNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  bool colon_ = false;
  TypeParamBoundsNode *type_param_bounds_ = nullptr;
  std::vector<AsscociatedItemNode *> asscociated_items_;
};

#endif //TRAIT_H
