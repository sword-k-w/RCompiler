#ifndef CRATE_H
#define CRATE_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/item.h"

class CrateNode : public ASTNode {
public:
  CrateNode() = delete;
  CrateNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<ItemNode *> items_;
};

#endif //CRATE_H
