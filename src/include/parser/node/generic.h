#ifndef GENERIC_H
#define GENERIC_H

#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/type.h"
#include "parser/node/expression.h"
#include "parser/node/trait.h"

class TypeParamNode : public ASTNode {
public:
  TypeParamNode() = delete;
  TypeParamNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier_ = nullptr;
  bool colon_ = false;
  TypeParamBoundsNode *type_param_bounds_ = nullptr;
  TypeNode *type_ = nullptr;
};

class ConstParamNode : public ASTNode {
public:
  ConstParamNode() = delete;
  ConstParamNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  IdentifierNode *identifier1_ = nullptr;
  TypeNode *type_ = nullptr;
  BlockExpressionNode *block_expr_ = nullptr;
  IdentifierNode *identifier2_ = nullptr;
  bool hyphen_ = false;
  LiteralExpressionNode *litral_expr_ = nullptr;
};

class GenericParamNode : public ASTNode {
public:
  GenericParamNode();
  GenericParamNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeParamNode *type_param_ = nullptr;
  ConstParamNode *const_param_ = nullptr;
};

class GenericParamsNode : public ASTNode {
public:
  GenericParamsNode();
  GenericParamsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<GenericParamNode *> generic_param_s_;
  uint32_t comma_cnt_ = 0;
};

class GenericArgsNode : public ASTNode {
public:
  GenericArgsNode();
  GenericArgsNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
};

class WhereClauseNode : public ASTNode {
public:
  WhereClauseNode();
  WhereClauseNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
};

#endif //GENERIC_H