#ifndef FUNCTION_H
#define FUNCTION_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/generic.h"

class FunctionParametersNode : public ASTNode {

};

class FunctionReturnType : public ASTNode {

};

class FunctionNode : public ASTNode {
public:
  FunctionNode() = delete;
  FunctionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:

};

#endif //FUNCTION_H