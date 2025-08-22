/*
#ifndef GENERIC_H
#define GENERIC_H

#include "parser/class_declaration.h"
#include "lexer/lexer.h"
#include "parser/node/AST_node.h"
#include "parser/node/terminal.h"
#include "parser/node/type.h"
#include "parser/node/expression.h"
#include "parser/node/trait.h"
#include "parser/node/path.h"

class TypeParamNode : public ASTNode {
  friend class Printer;
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
  friend class Printer;
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
  friend class Printer;
public:
  GenericParamNode() = delete;
  GenericParamNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeParamNode *type_param_ = nullptr;
  ConstParamNode *const_param_ = nullptr;
};

class GenericParamsNode : public ASTNode {
  friend class Printer;
public:
  GenericParamsNode() = delete;
  GenericParamsNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<GenericParamNode *> generic_param_s_;
  uint32_t comma_cnt_ = 0;
};

class GenericArgsConstNode : public ASTNode {
  friend class Printer;
public:
  GenericArgsConstNode() = delete;
  GenericArgsConstNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  BlockExpressionNode *block_expr_ = nullptr;
  LiteralExpressionNode *literal_expr = nullptr;
  bool hyphen_ = false;
  SimplePathSegmentNode *simple_path_segment_ = nullptr;
};

class GenericArgsBindingNode : public ASTNode {
  friend class Printer;
public:
  GenericArgsBindingNode() = delete;
  GenericArgsBindingNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  IdentifierNode *identifier_ = nullptr;
  GenericArgsNode *generic_args_ = nullptr;
  TypeNode *type_ = nullptr;
};

class GenericArgsBoundsNode : public ASTNode {
  friend class Printer;
public:
  GenericArgsBoundsNode() = delete;
  GenericArgsBoundsNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  IdentifierNode *identifier_ = nullptr;
  GenericArgsNode *generic_args_ = nullptr;
  TypeParamBoundsNode *type_param_bounds_ = nullptr;
};

class GenericArgNode : public ASTNode {
  friend class Printer;
public:
  GenericArgNode() = delete;
  GenericArgNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  TypeNode *type_ = nullptr;
  GenericArgsConstNode *generic_args_const_ = nullptr;
  GenericArgsBindingNode *generic_args_binding_ = nullptr;
  GenericArgsBoundsNode *generic_args_bounds_ = nullptr;
};

class GenericArgsNode : public ASTNode {
  friend class Printer;
public:
  GenericArgsNode() = delete;
  GenericArgsNode(const std::vector<Token>&, uint32_t&, const uint32_t&);
private:
  std::vector<GenericArgNode *> generic_arg_s_;
  uint32_t comma_cnt_ = 0;
};

class WhereClauseItemNode : public ASTNode {
  friend class Printer;
public:
  WhereClauseItemNode() = delete;
  WhereClauseItemNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  TypeNode *type_ = nullptr;
  TypeParamBoundsNode *type_param_bounds_ = nullptr;
};

class WhereClauseNode : public ASTNode {
  friend class Printer;
public:
  WhereClauseNode();
  WhereClauseNode(const std::vector<Token> &, uint32_t &, const uint32_t &);
private:
  std::vector<WhereClauseItemNode *> where_clause_items_;
  uint32_t comma_cnt_ = 0;
};

#endif //GENERIC_H
*/