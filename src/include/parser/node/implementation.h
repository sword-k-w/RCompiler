#pragma once

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include <cstdint>

class ImplementationNode : public ASTNode {
  friend class Printer;
public:
  ImplementationNode();
  ImplementationNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeNode *type_ = nullptr;
  IdentifierNode *identifier_ = nullptr;
  std::vector<AssociatedItemNode *> associated_items_;
};
