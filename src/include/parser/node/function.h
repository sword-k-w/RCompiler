#ifndef FUNCTION_H
#define FUNCTION_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/expression.h"

class FunctionParametersNode : public ASTNode {
public:
  FunctionParametersNode() = delete;
  FunctionParametersNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
};

class FunctionReturnTypeNode : public ASTNode {
public:
  FunctionReturnTypeNode() = delete;
  FunctionReturnTypeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
};

class FunctionNode : public ASTNode {
public:
  FunctionNode() = delete;
  FunctionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  FunctionParametersNode *function_parameters_ = nullptr;
  FunctionReturnTypeNode *function_return_type_ = nullptr;
  bool semicolon_ = false;
  BlockExpressionNode *block_expr_ = nullptr;
};

#endif //FUNCTION_H