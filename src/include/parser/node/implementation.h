#ifndef IMPLEMENTATION_H
#define IMPLEMENTATION_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/generic.h"
#include "parser/node/type.h"
#include "parser/node/item.h"
#include "parser/node/path.h"

class ImplementationNode : public ASTNode {
public:
  ImplementationNode();
  ImplementationNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeNode *type_ = nullptr;
  IdentifierNode *identifier_ = nullptr;
  std::vector<AsscociatedItemNode *> asscociated_items_;
};

#endif //IMPLEMENTATION_H
