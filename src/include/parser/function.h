#ifndef FUNCTION_H
#define FUNCTION_H

#include "lexer/lexer.h"

class FunctionQualifiers : public ASTNode {

};

class FunctionParametersNode : public ASTNode {

};

class FunctionReturnType : public ASTNode {

};

class FunctionNode : public ASTNode {
public:
  FunctionNode() = delete;
  FunctionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);

};

#endif //FUNCTION_H