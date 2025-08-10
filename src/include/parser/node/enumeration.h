#ifndef ENUMERATION_H
#define ENUMERATION_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"

class EnumerationNode : public ASTNode {
public:
  EnumerationNode() = delete;
  EnumerationNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

#endif //ENUMERATION_H
