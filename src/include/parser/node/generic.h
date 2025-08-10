#ifndef GENERIC_H
#define GENERIC_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"

class GenericParamsNode : public ASTNode {
public:
  GenericParamsNode();
  GenericParamsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

class WhereClauseNode : public ASTNode {
public:
  WhereClauseNode();
  WhereClauseNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

#endif //GENERIC_H
