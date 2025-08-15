#ifndef FUNCTION_H
#define FUNCTION_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/generic.h"
#include "parser/node/expression.h"

class ShorthandSelfNode : public ASTNode {
public:
  ShorthandSelfNode() = delete;
  ShorthandSelfNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool quote_ = false;
  bool mut_ = false;
  SelfLowerNode *self_lower_ = nullptr;
};

class TypedSelfNode : public ASTNode {
public:
  TypedSelfNode() = delete;
  TypedSelfNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool mut_ = false;
  SelfLowerNode *self_lower_ = nullptr;
  TypeNode *type_ = nullptr;
};

class SelfParamNode : public ASTNode {
public:
  SelfParamNode() = delete;
  SelfParamNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  ShorthandSelfNode *shorthand_self_ = nullptr;
  TypedSelfNode *typed_self_ = nullptr;
};


class FunctionParamNode : public ASTNode {
public:
  FunctionParamNode() = delete;
  FunctionParamNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  PatternNoTopAltNode *pattern_no_top_alt_ = nullptr;
  TypeNode *type_ = nullptr;
};

class FunctionParametersNode : public ASTNode {
public:
  FunctionParametersNode() = delete;
  FunctionParametersNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  SelfParamNode *self_param_ = nullptr;
  uint32_t comma_cnt_ = 0;
  std::vector<FunctionParamNode *> function_params_;
};

class FunctionReturnTypeNode : public ASTNode {
public:
  FunctionReturnTypeNode() = delete;
  FunctionReturnTypeNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeNode *type_;
};

class FunctionNode : public ASTNode {
public:
  FunctionNode() = delete;
  FunctionNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  bool const_ = false;
  IdentifierNode *identifier_ = nullptr;
  FunctionParametersNode *function_parameters_ = nullptr;
  FunctionReturnTypeNode *function_return_type_ = nullptr;
  bool semicolon_ = false;
  BlockExpressionNode *block_expr_ = nullptr;
};

#endif //FUNCTION_H