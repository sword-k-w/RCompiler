#ifndef TYPE_H
#define TYPE_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"

class TypeNode : public ASTNode {
public:
  TypeNode() = delete;
  TypeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);

};

#endif //TYPE_H
