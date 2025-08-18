#ifndef TRAIT_H
#define TRAIT_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/generic.h"
#include "parser/node/item.h"

class TraitNode : public ASTNode {
  friend class Printer;
public:
  TraitNode() = delete;
  TraitNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  std::vector<AssociatedItemNode *> asscociated_items_;
};

#endif //TRAIT_H
