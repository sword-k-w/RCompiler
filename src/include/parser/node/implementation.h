#ifndef IMPLEMENTATION_H
#define IMPLEMENTATION_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"

class ImplementationNode : public ASTNode {
public:
  ImplementationNode();
  ImplementationNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:

};

#endif //IMPLEMENTATION_H
